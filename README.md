# Extra Credit: Implement Dense Matrix-Matrix Multiplication as Fast as You Can #

**Due Fri Dec 3rd, 5:00pm PT (no late submission allowed)**

If you complete this assignment, you will receive up to 10 bonus points on one of the regular programming assignments (PA1-PA4). 

## Overview ##

In this assignment you will implement general dense matrix-matrix multiplication (GEMM).

## Environment Setup ##

__You will need to run code on the myth machines for this assignment.__
(Hostnames for these machines are `myth[51-66].stanford.edu`)
These machines contain four-core 4.2 GHz Intel Core i7 processors (although dynamic frequency scaling can take them to 4.5 GHz when the chip decides it is useful and possible to do so). Each core in the processor can execute AVX2 vector instructions which describe
simultaneous execution of the same operation on multiple single-precision data
values. For the curious, a complete specification for this CPU can be found at 
<https://ark.intel.com/products/97129/Intel-Core-i7-7700K-Processor-8M-Cache-up-to-4-50-GHz->.

Note: For grading purposes, we expect you to report on the performance of code run on the Stanford myth machines, however
for kicks, you may also want to run the programs in this assignment on your own machine or on AWS machines if you have credits left over.
Feel free to include your findings from running code on other machines in your report as well, just be very clear what machine you were running on. 

To get started:

1. The Intel Math Kernel Library (MKL) is a highly optimized linear algebra library which we'll be testing your matrix-multiply routine against. It may be easily installed on the myth machines through the following steps:  

Visit https://software.intel.com/en-us/mkl/choose-download/linux in your browser.

Register. Under "Choose Product to Download", select Intel Math Kernel Library for Linux, 2020 Update 4. The full installer ("Full Package", 438MB) is more than adequate for our purposes.

Right-click on the "Full Package" button and select "Copy link address". On your myth machine, run `wget <paste the copied link address>` to download it directly on the myth machine. (Alternatively, you may download it to your local machine and use scp to transfer it to a myth machine.)

Untar the downloaded file: `tar xzvf l_mkl_2020.4.304.tgz`  (You may need to change the version number in the filename accordingly.)

Enter the extracted directory and run the install script: `./install.sh`  (Accept the EULA, accept the default installation settings, and wait a few minutes for installation to complete.)

Run this: `source intel/mkl/bin/mklvars.sh intel64` (To avoid having to do this every time you log in, you may wish to add this to your .bashrc)

2. The assignment starter code is available on [github]. Please clone the extra credit starter code using:

    `git clone git@github.com:stanford-cs149/extracredit_gemm`

3. Run the starter code. It accepts an argument `N` that specifies the sizes of all dimensions of the matrices. If N= You should see the following output by default, which documents the performance of three different implementations (Intel MKL library's performance, a staff reference implementation written in ISPC, and your solution).

4. Note how much faster MKL is than the starter code we give you, which is just a simple triple for loop (the best MKL run is ~190 times faster than the best starter code run!!!). 

```
(base) foo@myth55:~/starter/gemm$ ./gemm 1024
Running Intel MKL... 16.10ms
Running your ispc GEMM... 4174.45ms
Running ref ispc GEMM... 89.08ms
Running Intel MKL... 20.59ms
Running your ispc GEMM... 3050.68ms
Running ref ispc GEMM... 83.26ms
Running Intel MKL... 20.66ms
Running your ispc GEMM... 3045.31ms
Running ref ispc GEMM... 84.50ms
[Intel MKL]:            [16.097] ms     [1.941] GB/s    [1.15e+12] GFLOPS
[Your ISPC GEMM]:               [3045.309] ms   [0.010] GB/s    [6.06e+09] GFLOPS
[Ref ISPC GEMM]:                [83.262] ms     [0.375] GB/s    [2.22e+11] GFLOPS
Total squared error user: 0.000000
Total squared error ispc: 0.000000
```
### What you need to do:

Implement GEMM on square (NxN) matrices. __(Your solution can assume all matrices are square with powers of two dimensions.)__
Place your implementation in the function `gemm_ispc` in the file `gemm/gemm.ispc`.
To use this implementation, please replace the naive solution we've include in `gemm/gemm.cpp`.

To perform this replacement, modify `main.cpp` by uncommenting `ispc::gemm_ispc(m, n, k, A2, B2, C2, alpha, beta);` and commenting out `gemm(m, n, k, A2, B2, C2, alpha, beta);`

### Grading

__This extra credit will be graded on a case-by-case basis.__  There is no grading harness, however we are looking for documentation of a serious effort to obtain improved performance using ideas like creating parallel work for all cores, blocking to fit in cache (and perhaps even registers), SIMD vector processing (e.g. using ISPC). To give you a ballpark estimate of expectations, we are looking for student work to be approaching that of the reference ISPC implementation to gain this extra credit (although that reference ISPC implementation is pretty good, so don't necessarily expect to beat it to get points).

## Hand-in Instructions ##

Handin will be performed via [Gradescope](https://www.gradescope.com/). Please place the following files in your handin:

1. Your implementation of `gemm.ispc`
2. A writeup with:
  1. A graph of GFlops for MKL, your implementation, and the reference ISPC implementation with input argument N of 256, 512, 1024, 2048, and 4096 
  2. A detailed description of the steps you performed in the optimzation process. In particular, please let us know which steps had the biggest benefit.
