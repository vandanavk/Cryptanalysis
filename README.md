# BCR code

Install GMP
  1. Download GMP from https://gmplib.org
  2. In Terminal, cd to the gmp folder
  3. ./configure
  4. make
  5. sudo make install
  
Install GMP-ECM
  1. Download GMP ECM from http://gforge.inria.fr/projects/ecm/
  2. In Terminal, cd to the ecm folder
  3. ./configure
  4. make ecm-params
  5. make
  6. sudo make install
  
In the Terminal session, where you will build and execute the BCR code,
>export C_INCLUDE_PATH=/usr/local/include
>export LIBRARY_PATH=/usr/local/lib
>export LD_LIBRARY_PATH=/usr/local/lib
>export LD_RUN_PATH=/usr/local/lib

>cd <path>/Project_1
>gcc bcr.c -lgmp 
>./a.out

The code takes 30 minutes to execute because of the prime factorization step.
The final message is printed on the output screen.
