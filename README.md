Binary Matrix (BMT) 
==================

BMT is a binary (as opposed to ASCII) format for storing sparse matrices. It supports

+ Sparse and dense matrices.
+ Row major (resp. CSR) and column major (resp. CSC) matrices.
+ Double or single precision entries.
+ 32- or 64-bit integer indices.

We provide a MATLAB interface, making it easy to export matrices from MATLAB to C++ and vice versa.

Examples
========

Sparse matrix example
---------------------

To export a sparse matrix from MATLAB, type
```
 bmtsave(A, 'A_sparse.bmt')
```
Then to load it in C++

```
char order;
char fname[] = "A_sparse.bmt";
int m, n, nnz;
double *val;
int  *col_ind, *row_ptr;
ReadBmtSparse(fname, &order, &m, &n, &nnz, &val, &col_ind, &row_ptr);
```
Upon exiting, variables take the following values
+ `order`: The character `'c'`, indicating that the matrix was stored in column major ordering.
+ `m, n`: Dimensions of matrix `A`.
+ `nnz`: Number of non-zero entries in `A`.
+ `col_ind`: Indicies of of columns for each element.
+ `row_ptr`: Pointer to beginning of each rows.

Dense matrix example
--------------------

The syntax in MATLAB is the same for sparse and dense matrices:
```
 bmtsave(A, 'A_dense.bmt')
```
Then, assuming `A` is dense, we can load it in C++

```
char order;
char fname[] = "A_dense.bmt";
int m, n;
double *A;
ReadBmtDense(fname, &order, &m, &n, &A);
```
Upon exiting, variables take the following values
+ `order`: The character `'c'`, indicating that the matrix was stored in column major ordering.
+ `m, n`: Dimensions of matrix `A`.
+ `A`: Entries in A.

