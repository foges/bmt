// BMT = Binary Matrix.

#ifndef BMT_H_
#define BMT_H_

#include <stdint.h>

#include <cstdio>
#include <typeinfo>

bool IsSparse(const char *fname) {
  FILE *fid = fopen(fname, "rb");
  char buf[4];
  fread(buf, sizeof(char), sizeof(buf), fid);
  fclose(fid);
  return buf[3] == 2 || buf[3] == 3;
}

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

  // Read val.
  if (!flag && kFPrec == 32) {
    float *val_ = new float[nnz_];
    count = fread(val_, nnz_ * sizeof(float), 1, fid);
    if (count == 0)
      flag = 2;
    if (typeid(float) == typeid(T)) {
      *val = reinterpret_cast<T*>(val_);
    } else {
      *val = new T[nnz_];
      for (size_t i = 0; i < nnz_; ++i)
        (*val)[i] = static_cast<T>(val_[i]);
      delete [] val_;
    }
  } else if (!flag) {
    double *val_ = new double[nnz_];
    fread(val_, nnz_ * sizeof(double), 1, fid);
    if (count == 0)
      flag = 2;
    if (typeid(double) == typeid(T)) {
      *val = reinterpret_cast<T*>(val_);
    } else {
      *val = new T[nnz_];
      for (size_t i = 0; i < nnz_; ++i)
        (*val)[i] = static_cast<T>(val_[i]);
      delete [] val_;
    }
  }

  // Read ind and ptr.
  size_t numel_int = nnz_ + 1;
  if (kMatTyp == 2)
    numel_int += n_;
  else
    numel_int += m_;
  if (!flag && kIPrec == 4) {
    int32_t *ind_ptr = new int32_t[numel_int];
    count = fread(ind_ptr, numel_int * sizeof(int32_t), 1, fid);
    if (count == 0)
      flag = 2;
    if (typeid(I) == typeid(int32_t)) {
      *ind = reinterpret_cast<I*>(ind_ptr);
      *ptr = *ind + nnz_;
    } else {
      *ind = new I[numel_int];
      for (size_t i = 0; i < numel_int; ++i)
        (*ind)[i] = static_cast<I>(ind_ptr[i]);
      *ptr = *ind + nnz_;
      delete [] ind_ptr;
    }
  } else if (!flag) {
    int64_t *ind_ptr = new int64_t[numel_int];
    count = fread(ind_ptr, numel_int * sizeof(int64_t), 1, fid);
    if (count == 0)
      flag = 2;
    if (typeid(I) == typeid(int64_t)) {
      *ind = reinterpret_cast<I*>(ind_ptr);
      *ptr = *ind + nnz_;
    } else {
      *ind = new I[numel_int];
      for (size_t i = 0; i < numel_int; ++i)
        (*ind)[i] = static_cast<I>(ind_ptr[i]);
      *ptr = *ind + nnz_;
      delete [] ind_ptr;
    }
  }

  // Close and finish.
  fclose(fid);
  return flag;
}

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
    if (typeid(float) == typeid(T)) {
      *data = reinterpret_cast<T*>(data_);
    } else {
      *data = new T[m_ * n_];
      for (size_t i = 0; i < m_ * n_; ++i)
        (*data)[i] = static_cast<T>(data_[i]);
      delete [] data_;
    }
  } else if (!flag) {
    double *data_ = new double[m_ * n_];
    count = fread(data_, m_ * n_ * sizeof(double), 1, fid);
    if (count == 0)
      flag = 2;
    if (typeid(double) == typeid(T)) {
      *data = reinterpret_cast<T*>(data_);
    } else {
      *data = new T[m_ * n_];
      for (size_t i = 0; i < m_ * n_; ++i)
        (*data)[i] = static_cast<T>(data_[i]);
      delete [] data_;
    }
  }
 
  // Close and return.
  fclose(fid);
  return flag;
}

// TODO(chris): Write BMT functions.

#endif  // BMT_H_

