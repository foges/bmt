function A = bmtload(fname)
%BMTLOAD Load file in Binary Matrix format.
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
%  A = BMTLOAD(fname)
%
%  Inputs:
%  fname    - Filename of the bmt file.
%
%  Outputs:
%  A        - MATLAB matrix.
%
%  References:
%  http://github.com/foges/bmt
%
%  See also BMTSAVE.
%

% Attempt to open file.
fid = fopen(fname);
if fid == -1
  fid = fopen([fname '.bmt']);
end
if fid == -1
  error('Could not open file.')
end

% Read info.
int_prec    = fread(fid, 1, 'uint8');
float_prec  = fread(fid, 1, 'uint8');
endian      = fread(fid, 1, 'uint8');
mattype     = fread(fid, 1, 'uint8');

i_prec = sprintf('int%d', int_prec);
f_prec = sprintf('float%d', float_prec);

if endian == 0
  endian = 'l';
else
  endian = 'b';
end

% Read size.
m = fread(fid, 1, i_prec, 0, endian);
n = fread(fid, 1, i_prec, 0, endian);

% Load matrix depending on type
if mattype == 0
  A = fread(fid, [m, n], f_prec, 0, endian);
elseif mattype == 1
  A = fread(fid, [n, m], f_prec, 0, endian)';
elseif mattype == 2
  nnz = fread(fid, 1, i_prec, 0, endian);
  val = fread(fid, nnz, f_prec, 0, endian);
  col_ind = fread(fid, nnz, i_prec, 0, endian);
  row_ptr = fread(fid, n + 1, i_prec, 0, endian);
  A = csr2sparse(val, row_ptr, col_ind, m)';
else
  nnz = fread(fid, 1, i_prec, 0, endian);
  val = fread(fid, nnz, f_prec, 0, endian);
  col_ind = fread(fid, nnz, i_prec, 0, endian);
  row_ptr = fread(fid, m + 1, i_prec, 0, endian);
  A = csr2sparse(val, row_ptr, col_ind, n);
end

fclose(fid);

end
