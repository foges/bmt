function A = csr2sparse(val, row_ptr, col_ind, n)
%CSR2SPARSE Convert from CSR to a MATLAB sparse matrix.
%   To convert a CSC matrix instead of CSR matrix to a sparse matrix
%   replace row_ptr with col_ptr, col_ind by row_ind, and n by m. Lastly
%   transpose the matrix A upon exiting. This function supports both 0- and
%   1-based indexing (this is determined by the first entry in row_ptr).
%
%   A = CSR2SPARSE(val, row_ptr, col_ind, n)
%
%   Inputs:
%   val      - Vector of matrix entries.
%
%   row_ptr  - Pointer to index start of each row.
%
%   col_ind  - Index of elements in each colum.
%
%   Outputs:
%   A        - MATLAB sparse matrix, reconstructed from input.
%
%   References:
%   http://en.wikipedia.org/wiki/Sparse_matrix
%
%   Author:
%   Chris Fougner

% Check input.
if nargin < 4
  error('Too few inputs')
end
if length(val) ~= length(col_ind) || length(col_ind) ~= row_ptr(end)
  error('Vector lengths must agree')
end
if row_ptr(1) ~= 0 && row_ptr(1) ~= 1
  error('First element of row_ptr must be 0 or 1')
end

% Determine indexing base.
base = row_ptr(1);

% Compute full row index.
m = length(row_ptr) - 1;
row_ind = zeros(length(col_ind), 1);
for i = 1:m
  row_ind(row_ptr(i) + (1 - base):row_ptr(i + 1) - base) = i;
end

% Construct matrix.
A = sparse(row_ind, col_ind + (1 - base), val, m, n);

end