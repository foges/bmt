function bmtsave(A, fname, order, base, float_prec, int_prec)
%BMTSAVE Save file in Binary Matrix format.
%  The bmt format stores both sparse and dense matrices in in a binary
%  format (as opposed to ASCII). The data is not compressed, but does store
%  information such as dimensions, sparsity, and integer and floating point
%  precision.
%
%  For sparse matrices the layout is
%    (int_prec, float_prec, endian, mattype, m, n, nnz, val, ind, ptr),
%  and for dense matricies it is
%    (int_prec, float_prec, endian, mattype, m, n, data).
%
%  BMTSAVE(A, fname)
%  BMTSAVE(A, fname, order, base, float_prec, int_prec)
%
%  Optional Inputs: order, base, float_prec, int_prec
%
%  Inputs:
%  A          - Either dense or sparse MATLAB matrix.
%  
%  fname      - Filename to which data in A will be written.
%
%  order      - Specify 'r' for row major (resp. CSR) or 'c' for column
%               major (resp. CSC). Default: 'c'.
%
%  base       - Must be either 0 or 1, for 0- or 1-based indexing.
%               Default: 0.
%
%  float_prec - Floating point precision, must be either 32 or 64. 
%               Default 64.
%
%  int_prec   - Integer precision, must be either 32 or 64. Default 64.
%
%  References:
%  http://github.com/foges/bmt
%
%  See also BMTLOAD.
%

% Check input.
if nargin < 2
  error('Must specify matrix A and filename')
end
if nargin < 3 || isempty(order)
  order = 'c';
end
if nargin < 4 || isempty(base)
  base = 0;
end
if nargin < 5 || isempty(float_prec)
  float_prec = 64;
end
if nargin < 6 || isempty(int_prec)
  int_prec = 64;
end

if int_prec ~= 64 && int_prec ~= 32
  error('int_prec must be 32 or 64')
elseif float_prec ~= 64 && float_prec ~= 32
  error('float_prec must be 32 or 64')
end

if ~strcmp(order, 'r') && ~strcmp(order, 'c')
  error('Order must be ''r'' (row) or ''c'' (column)')
end

if ~ismatrix(A)
  error('A must be matrix.')
end

% Handle input.
[m, n] = size(A);

if strcmp(order, 'c') && ~issparse(A)
  mattype = 0;
elseif strcmp(order, 'r') && ~issparse(A)
  mattype = 1;
  A = A';
elseif strcmp(order, 'c') && issparse(A)
  mattype = 2;
  A = A';
else
  mattype = 3;
end
  
% Set up flags.
f_prec = sprintf('float%d', float_prec);
f_flag = uint8(float_prec);

i_prec = sprintf('int%d', int_prec);
i_flag = uint8(int_prec);

[~, ~, endian] = computer();
if strcmp(endian, 'L')
  endian = 0;
else
  endian = 1;
end

% Open file.
fid = fopen(fname, 'w');

% Write precision flags, base flag and dimensions.
fwrite(fid, i_flag, 'uint8');
fwrite(fid, f_flag, 'uint8');
fwrite(fid, endian, 'uint8');
fwrite(fid, mattype, 'uint8');
fwrite(fid, m, i_prec);
fwrite(fid, n, i_prec);

if issparse(A)
  % Convert A to CSR.
  [val, row_ptr, col_ind] = sparse2csr(A, base);

  % Write sparse data.
  fwrite(fid, nnz(A), i_prec);
  fwrite(fid, val, f_prec);
  fwrite(fid, col_ind, i_prec);
  fwrite(fid, row_ptr, i_prec);
else
  % Write dense data.
  fwrite(fid, A, f_prec);
end

% Close
fclose(fid);

end
