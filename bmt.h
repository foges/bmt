////////////////////////////////////////////////////////////////////////////////
// Copyright 2014 Chris Fougner.                                              //
//                                                                            //
// This program is free software: you can redistribute it and/or modify       //
// it under the terms of the GNU General Public License as published by       //
// the Free Software Foundation, either version 3 of the License, or          //
// (at your option) any later version.                                        //
//                                                                            //
// This program is distributed in the hope that it will be useful,            //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              //
// GNU General Public License for more details.                               //
//                                                                            //
// You should have received a copy of the GNU General Public License          //
// along with this program.  If not, see http://www.gnu.org/licenses/.        //
////////////////////////////////////////////////////////////////////////////////

// Binary Matrix (BMT) is a binary file format storing matrices.
// It supports the following matrix types
//
//   - Sparse and dense matrices.
//   - Row major (resp. CSR) and column major (resp. CSC) matrices.
//   - Double or single precision entries.
//   - 32- or 64-bit integer indices.
//
// This file format is much more efficient than MatrixMarket files and similar
// at storing matrices. The layout of the file is
//
//   Sparse: (int_prec, float_prec, endian, mattype, m, n, nnz, val, ind, ptr)
//   Dense: (int_prec, float_prec, endian, mattype, m, n, data)
//
//       - int_prec    : Integer precision (either 32 or 64).
//       - float_prec  : Floatint point precision (either 32 or 64).
//       - endian      : Endianness (either big or small).
//       - mattype     : Matrix type (either sparse or dense).
//       - (m, n)      : Matrix dimensions.
//       - nnz         : [sparse only] Number of non-zeros in matrix.
//       - val         : [sparse only] Numeric entries in matrix.
//       - ind         : [sparse only] Column, resp. row indices (for CSR resp.
//                       CSC sparse matrices).
//       - ptr         : [sparse only] Row, resp. column pointers (for CSR resp.
//                       CSC sparse matrices).
//       - data        : [dense only] Matrix data.
//

#ifndef BMT_H_
#define BMT_H_

#include <stdint.h>

#include <cstdio>
#include <typeinfo>

// Checks if a specified matrix is sparse or dense.
//
// Input arguments:
//   - fname : Filename of matrix to check.
//
// Returns:
//   0 : The matrix is not sparse.
//   1 : The matrix is sparse.
//   2 : The matrix could not be found.
//   3 : The matrix could not be read.
int IsSparse(const char fname[]) {
  FILE *fid = fopen(fname, "rb");
  if (fid != 0)
    return 2;
  char buf[4];
  int count = fread(buf, sizeof(buf), 1, fid);
  if (count != 0)
    return 3;
  fclose(fid);
  return buf[3] == 2 || buf[3] == 3;
}

// Reads a specified sparse matrix.
//
// Template arguments:
//   - T : Floating point type of matrix.
//   - I : Integer type of matrix indices.
//
// Input arguments:
//   - fname : Filename of matrix to load.
//
// Output arguments
//   - order  : 'r' for CSR format or 'c' for CSC.
//   - (m, n) : Dimensions of matrix.
//   - nnz    : Number of non-zeros in matrix.
//   - val    : Pointer to array of numeric data.
//   - ind    : Pointer to array of column (CSR) or row (CSC) indices.
//   - ptr    : Pointer to array of row (CSR) or column (CSC) pointers.
//
// Returns:
//   0 : Matrix successfully read.
//   1 : Could not open file.
//   2 : File length incorrect (or header corrupt).
//   3 : Integer precision invalid.
//   4 : Floating point precision invalid.
//   5 : Endianness invalid.
//   6 : Matrix type invald.
//   7 : Unsupported system, sizeof(float) != 4.
//   8 : Unsupported system, sizeof(double) != 8.
template <typename T, typename I>
int ReadBmtSparse(const char *fname, char *order, I *m, I *n, I *nnz, T **val,
                  I **ind, I **ptr) {
  int flag = 0;
  // Read info (int precision, float precision, endianness, matrix type).
  FILE *fid = fopen(fname, "rb");
  if (!fid)
    return 1;
  char info_buf[4];
  size_t count = fread(info_buf, sizeof(info_buf), 1, fid);
  if (count == 0)
    flag = 2;

  const int kIPrec  = info_buf[0];
  const int kFPrec  = info_buf[1];
  const int kEndian = info_buf[2];
  const int kMatTyp = info_buf[3];

  // Check info is valid.
  if (!flag) {
    if (kIPrec != 32 && kIPrec != 64)
      flag = 3;
    else if (kFPrec != 32 && kFPrec != 64)
      flag = 4;
    else if (kEndian != 0 && kEndian != 1)
      flag = 5;
    else if (kMatTyp != 2 && kMatTyp != 3)
      flag = 6;
    else if (kFPrec == 32 && sizeof(float) != 4)
      flag = 7;
    else if (kFPrec == 64 && sizeof(double) != 8)
      flag = 8;
  }

  // Read m, n and nnz.
  size_t m_, n_, nnz_;
  if (!flag && kIPrec == 32) {
    int32_t size_buf[3];
    count = fread(size_buf, sizeof(size_buf), 1, fid);
    if (count == 0)
      flag = 2;
    m_ = static_cast<size_t>(size_buf[0]);
    n_ = static_cast<size_t>(size_buf[1]);
    nnz_ = static_cast<size_t>(size_buf[2]);
  } else if (!flag) {
    int64_t size_buf[3];
    count = fread(size_buf, sizeof(size_buf), 1, fid);
    if (count == 0)
      flag = 2;
    m_ = static_cast<size_t>(size_buf[0]);
    n_ = static_cast<size_t>(size_buf[1]);
    nnz_ = static_cast<size_t>(size_buf[2]);
  }

  // Set output info
  *m = static_cast<I>(m_);
  *n = static_cast<I>(n_);
  *nnz = static_cast<I>(nnz_);
  *order = kMatTyp == 2 ? 'c' : 'r';
  *val = 0;
  *ptr = *ind = 0;

  // Read val.
  if (!flag && kFPrec == 32) {
    float *val_ = new float[nnz_];
    count = fread(val_, nnz_ * sizeof(float), 1, fid);
    if (count == 0)
      flag = 2;
    if (!flag && typeid(float) == typeid(T)) {
      *val = reinterpret_cast<T*>(val_);
    } else if (!flag) {
      *val = new T[nnz_];
      for (size_t i = 0; i < nnz_; ++i)
        (*val)[i] = static_cast<T>(val_[i]);
      delete [] val_;
    } else {
      delete [] val_;
    }
  } else if (!flag) {
    double *val_ = new double[nnz_];
    count = fread(val_, nnz_ * sizeof(double), 1, fid);
    if (count == 0)
      flag = 2;
    if (!flag && typeid(double) == typeid(T)) {
      *val = reinterpret_cast<T*>(val_);
    } else if (!flag) {
      *val = new T[nnz_];
      for (size_t i = 0; i < nnz_; ++i)
        (*val)[i] = static_cast<T>(val_[i]);
      delete [] val_;
    } else {
      delete [] val_;
    }
  }

  // Read ind and ptr.
  size_t num_ptr_;
  if (kMatTyp == 2)
    num_ptr_ = n_ + 1;
  else
    num_ptr_ = m_ + 1;
  if (!flag && kIPrec == 4) {
    int32_t *ind_ = new int32_t[nnz_];
    int32_t *ptr_ = new int32_t[num_ptr_];
    count = fread(ind_, nnz_ * sizeof(int32_t), 1, fid);
    if (count == 0)
      flag = 2;
    count = fread(ptr_, num_ptr_ * sizeof(int32_t), 1, fid);
    if (!flag && count == 0)
      flag = 2;
    if (!flag && typeid(I) == typeid(int32_t)) {
      *ind = reinterpret_cast<I*>(ind_);
      *ptr = reinterpret_cast<I*>(ptr_);
    } else if (!flag) {
      *ind = new I[nnz_];
      *ptr = new I[num_ptr_];
      for (size_t i = 0; i < nnz_; ++i)
        (*ind)[i] = static_cast<I>(ind_[i]);
      for (size_t i = 0; i < num_ptr_; ++i)
        (*ptr)[i] = static_cast<I>(ptr_[i]);
      delete [] ind_;
      delete [] ptr_;
    } else {
      delete [] ind_;
      delete [] ptr_;
    }
  } else if (!flag) {
    int64_t *ind_ = new int64_t[nnz_];
    int64_t *ptr_ = new int64_t[num_ptr_];
    count = fread(ind_, nnz_ * sizeof(int64_t), 1, fid);
    if (count == 0)
      flag = 2;
    count = fread(ptr_, num_ptr_ * sizeof(int64_t), 1, fid);
    if (!flag && count == 0)
      flag = 2;
    if (!flag && typeid(I) == typeid(int64_t)) {
      *ind = reinterpret_cast<I*>(ind_);
      *ptr = reinterpret_cast<I*>(ptr_);
    } else if (!flag) {
      *ind = new I[nnz_];
      *ptr = new I[num_ptr_];
      for (size_t i = 0; i < nnz_; ++i)
        (*ind)[i] = static_cast<I>(ind_[i]);
      for (size_t i = 0; i < num_ptr_; ++i)
        (*ptr)[i] = static_cast<I>(ptr_[i]);
      delete [] ind_;
      delete [] ptr_;
    } else {
      delete [] ind_;
      delete [] ptr_;
    }
  }

  // Delete all data if there was an error.
  if (flag) {
    if (*val != 0)
      delete [] val;
    *val = 0;
    *ind = *ptr = 0;
  }

  // Close and finish.
  fclose(fid);
  return flag;
}

// Reads a specified dense matrix.
//
// Template arguments:
//   - T : Floating point type of matrix.
//   - I : Integer type of matrix dimensions.
//
// Input arguments:
//   - fname : Filename of matrix to load.
//
// Output arguments
//   - order  : 'r' for row major format or 'c' for column major format.
//   - (m, n) : Dimensions of matrix.
//   - data   : Pointer to array of numeric data.
//
// Returns:
//   0 : Matrix successfully read.
//   1 : Could not open file (or header corrupt).
//   2 : File length incorrect.
//   3 : Integer precision invalid.
//   4 : Floating point precision invalid.
//   5 : Endianness invalid.
//   6 : Matrix type invald.
//   7 : Unsupported system, sizeof(float) != 4.
//   8 : Unsupported system, sizeof(double) != 8.
template <typename T, typename I>
int ReadBmtDense(const char *fname, char *order, I *m, I *n, T **data) {
  int flag = 0;
  // Read info (int precision, float precision, endianness, matrix type).
  FILE *fid = fopen(fname, "rb");
  if (!fid)
    return 1;
  char info_buf[4];
  size_t count = fread(info_buf, sizeof(info_buf), 1, fid);
  if (count == 0)
    flag = 2;

  const int kIPrec  = info_buf[0];
  const int kFPrec  = info_buf[1];
  const int kEndian = info_buf[2];
  const int kMatTyp = info_buf[3];

  // Check info is valid.
  if (!flag) {
    if (kIPrec != 32 && kIPrec != 64)
      flag = 3;
    else if (kFPrec != 32 && kFPrec != 64)
      flag = 4;
    else if (kEndian != 0 && kEndian != 1)
      flag = 5;
    else if (kMatTyp != 0 && kMatTyp != 1)
      flag = 6;
    else if (kFPrec == 32 && sizeof(float) != 4)
      flag = 7;
    else if (kFPrec == 64 && sizeof(double) != 8)
      flag = 8;
  }

  // Read m, n and nnz.
  size_t m_, n_;
  if (!flag && kIPrec == 32) {
    int32_t size_buf[2];
    count = fread(size_buf, sizeof(size_buf), 1, fid);
    if (count == 0)
      flag = 2;
    m_ = static_cast<size_t>(size_buf[0]);
    n_ = static_cast<size_t>(size_buf[1]);
  } else if (!flag) {
    int64_t size_buf[2];
    count = fread(size_buf, sizeof(size_buf), 1, fid);
    if (count == 0)
      flag = 2;
    m_ = static_cast<size_t>(size_buf[0]);
    n_ = static_cast<size_t>(size_buf[1]);
  }

  // Set output info
  *m = static_cast<I>(m_);
  *n = static_cast<I>(n_);
  *order = kMatTyp == 0 ? 'c' : 'r';

  // Read val.
  if (!flag && kFPrec == 32) {
    float *data_ = new float[m_ * n_];
    count = fread(data_, m_ * n_ * sizeof(float), 1, fid);
    if (count == 0)
      flag = 2;
    if (!flag && typeid(float) == typeid(T)) {
      *data = reinterpret_cast<T*>(data_);
    } else if (!flag) {
      *data = new T[m_ * n_];
      for (size_t i = 0; i < m_ * n_; ++i)
        (*data)[i] = static_cast<T>(data_[i]);
      delete [] data_;
    } else {
      delete [] data_;
    }
  } else if (!flag) {
    double *data_ = new double[m_ * n_];
    count = fread(data_, m_ * n_ * sizeof(double), 1, fid);
    if (count == 0)
      flag = 2;
    if (!flag && typeid(double) == typeid(T)) {
      *data = reinterpret_cast<T*>(data_);
    } else if (!flag) {
      *data = new T[m_ * n_];
      for (size_t i = 0; i < m_ * n_; ++i)
        (*data)[i] = static_cast<T>(data_[i]);
      delete [] data_;
    } else {
      delete [] data_;
    }
  }

  if (flag)
    *data = 0;

  // Close and return.
  fclose(fid);
  return flag;
}

// TODO(chris): BMT-write functions.

#endif  // BMT_H_

