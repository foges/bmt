Description of files:

+ `bmtload`: Load a file stored in `.bmt` format. Type `help bmtload` for more information.
+ `bmtsave`: Save a matrix to a `.bmt` formatted file. Type `help bmtsave` for more information.
+ `test_bmt.m`: Script tests the implementation. 
+ `csr2sparse.m`: Converts CSR matrix to a sparse MATLAB matrix (can also be used for CSC matrices). This is used by `bmtload.m`.
+ `sparse2csr.m`: Converts a MATLAB sparse matrix to a CSR matrix (can also be used for CSC matrices). This is used by `bmtsave.m`.