#include <cmath>

#include "bmt.h"

typedef double real_t;

int TestDenseCol() {
  char ord_ = 'c';
  int m_ = 5, n_ = 3;
  real_t A_[] =
      { 7.647335395229897e-01,  9.023050655863230e-01,  9.187559514684829e-01,
        7.147636157215225e-01,  9.222284756353700e-01,  2.069495500510403e-01,
        3.183278527554755e-01,  1.611478384786107e-01,  1.619819158928901e-01,
        2.633090813600850e-01,  3.018824400863795e-02,  8.208822841300821e-01,
        4.744996037051061e-01,  1.396057084270367e-01,  1.819799083073432e-01};

  char ord;
  char fname[] = "A_de_col.bmt";
  int m, n;
  real_t *A;
  int flag = ReadBmtDense(fname, &ord, &m, &n, &A);

  if (flag)
    return flag;

  if (m_ != m || n_ != n)
    return 9;

  if (ord_ != ord)
    return 10;
 
  real_t err = 0;
  for (size_t i = 0; i < m_ * n_; ++i)
    err += fabs(A[i] - A_[i]);
  if (err / (m_ * n_) > 1e-14)
    return 11;
  
  return 0;
}

int TestDenseRow() {
  char ord_ = 'r';
  int m_ = 5, n_ = 3;
  real_t A_[] =
      { 4.446608463357279e-01,  1.520070107334148e-01,  6.886459923006888e-01,
        7.639469846221241e-01,  6.010880025121487e-01,  1.721978761545961e-02,
        4.637546750018795e-02,  4.981336972691891e-01,  2.138519849816289e-02,
        8.474672243900893e-01,  3.562458175330537e-01,  3.512099090984306e-01,
        9.439343241523100e-01,  3.554308490889824e-01,  2.845264573634944e-01};
       
  char ord;
  char fname[] = "A_de_row.bmt";
  int m, n;
  real_t *A;
  int flag = ReadBmtDense(fname, &ord, &m, &n, &A);

  if (flag)
    return flag;

  if (m_ != m || n_ != n)
    return 9;

  if (ord_ != ord)
    return 10;
 
  real_t err = 0;
  for (size_t i = 0; i < m_ * n_; ++i)
    err += fabs(A[i] - A_[i]);
  if (err / (m_ * n_) > 1e-14)
    return 11;
  
  return 0;
}

int TestSparseCol() {
  char ord_ = 'c';
  int m_ = 5, n_ = 3, nnz_ = 4;
  real_t val_[] =
      { 0.605543559817451, 0.384665659769348, 0.096554229287261,
        0.230983526252695};
  int col_ptr_[] = {0, 2, 4, 4};
  int row_ind_[] = {1, 3, 0, 4};

  char ord;
  char fname[] = "A_sp_csc.bmt";
  int m, n, nnz;
  real_t *val;
  int *col_ptr, *row_ind;
  int flag = ReadBmtSparse(fname, &ord, &m, &n, &nnz, &val, &row_ind, &col_ptr);

  if (flag)
    return flag;

  if (m_ != m || n_ != n || nnz_ != nnz)
    return 9;

  if (ord_ != ord)
    return 10;
 
  real_t err = 0;
  for (size_t i = 0; i < nnz_; ++i)
    err += fabs(val[i] - val_[i]);
  for (size_t i = 0; i < n_ + 1; ++i)
    err += fabs(col_ptr[i] - col_ptr_[i]);
  for (size_t i = 0; i < nnz_; ++i)
    err += fabs(row_ind[i] - row_ind_[i]);

  if (err / (2 * nnz_ + m_ + 1) > 1e-14)
    return 11;
  
  return 0;
}

int TestSparseRow() {
  char ord_ = 'r';
  int m_ = 5, n_ = 3, nnz_ = 4;
  real_t val_[] =
      { 0.854866202883940, 0.601513688757442, 0.802494583435725,
        0.681237425784736};
  int row_ptr_[] = {0, 0, 1, 4, 4, 4};
  int col_ind_[] = {2, 0, 1, 2};

  char ord;
  char fname[] = "A_sp_csr.bmt";
  int m, n, nnz;
  real_t *val;
  int *row_ptr, *col_ind;
  int flag = ReadBmtSparse(fname, &ord, &m, &n, &nnz, &val, &col_ind, &row_ptr);

  if (flag)
    return flag;

  if (m_ != m || n_ != n || nnz_ != nnz)
    return 9;

  if (ord_ != ord)
    return 10;
 
  real_t err = 0;
  for (size_t i = 0; i < nnz_; ++i)
    err += fabs(val[i] - val_[i]);
  for (size_t i = 0; i < m_ + 1; ++i)
    err += fabs(row_ptr[i] - row_ptr_[i]);
  for (size_t i = 0; i < nnz_; ++i)
    err += fabs(col_ind[i] - col_ind_[i]);

  if (err / (2 * nnz_ + m_ + 1) > 1e-14)
    return 11;
  
  return 0;
}

int main() {
  const char *kStatus[] = {
      "Pass", 
      "Fail, could not open file.", 
      "Fail, file length incorrect",
      "Fail, integer precision invalid.",
      "Fail, floating point precision invalid.",
      "Fail, endianness invalid.",
      "Fail, matrix type invald.",
      "Fail, unsupported system, sizeof(float) != 4.",
      "Fail, unsupported system, sizeof(double) != 8.",
      "Fail, did not read m, n, or nnz correctly.",
      "Fail, did not read order correctly.",
      "Fail, data incorrect."};
  int flag_dense_col = TestDenseCol();
  printf("Dense Col Flag: %s\n", kStatus[flag_dense_col]);
  int flag_dense_row = TestDenseRow();
  printf("Dense Row Flag: %s\n", kStatus[flag_dense_row]);
  int flag_sparse_col = TestSparseCol();
  printf("Sparse Col Flag: %s\n", kStatus[flag_sparse_col]);
  int flag_sparse_row = TestSparseRow();
  printf("Sparse Row Flag: %s\n", kStatus[flag_sparse_row]);
}

