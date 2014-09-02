%TEST_BMT Test script for BMTLOAD and BMTSAVE.

% Matrix dimensions, density and status.
m = 5;
n = 3;
r = 0.3;
status = {'Fail', 'Pass'};

% Begin testing.
A_sp_csr = sprand(m, n, r);
bmtsave(A_sp_csr, 'A_sp_csr.bmt','r');
A_sp_csr_ = bmtload('A_sp_csr.bmt');
fprintf('CSR Status: %s\n', status{full(all(all(A_sp_csr_ == A_sp_csr))) + 1});

A_sp_csc = sprand(m, n, r);
bmtsave(A_sp_csc, 'A_sp_csc.bmt','c');
A_sp_csc_ = bmtload('A_sp_csc.bmt');
fprintf('CSC Status: %s\n', status{full(all(all(A_sp_csc_ == A_sp_csc))) + 1});

A_de_row = rand(m, n);
bmtsave(A_de_row, 'A_de_row.bmt','r');
A_de_row_ = bmtload('A_de_row.bmt');
fprintf('DRW Status: %s\n', status{full(all(all(A_de_row_ == A_de_row))) + 1});

A_de_col = rand(m, n);
bmtsave(A_de_col, 'A_de_col.bmt','c');
A_de_col_ = bmtload('A_de_col.bmt');
fprintf('DCL Status: %s\n', status{full(all(all(A_de_col_ == A_de_col))) + 1});
