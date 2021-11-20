# Extra Credit: Implement Dense Matrix-Matrix Multiplication as Fast as You Can #

**Due Fri Dec 3rd, 5:00pm PT (no late submission allowed)**

If you complete this assignment, you will __receive up__ to 10 bonus points on one of the regular programming assignments (PA1-PA4).   

## Overview ##

In this assignment you will implement general dense matrix-matrix multiplication (GEMM).  GEMM is a critical computational primitives used in many applications: ranging from linear algebra, solving systems of equations, scientific computing, and most recently in [deep learning](https://petewarden.com/2015/04/20/why-gemm-is-at-the-heart-of-deep-learning/).  

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

1. The Intel Math Kernel Library (MKL) is a highly optimized linear algebra library which you'll testing your matrix-multiply routine against. You can install it on the myth machines using the following steps.  __Note: that if you have problems installing MKL you can skip this step and still do the assignment, you just won't be able to compare your performance to a professional implementation. __

You can install MKL using the following commands:

```bash
wget https://registrationcenter-download.intel.com/akdlm/irc_nas/18236/l_BaseKit_p_2021.4.0.3422.sh

bash l_BaseKit_p_2021.4.0.3422.sh
```

You'll likely want to deselect everything except for the Intel Math Kernel Library, and your final installation should take up 7.7GB of space on Myth.

Or, for a one-line installation once you've downloaded you can use this command. Note that you will accept the EULA by running this command.
```
bash l_BaseKit_p_2021.4.0.3422.sh -a --action install --components intel.oneapi.lin.mkl.devel -s --eula accept
```

If that fails, you can manually download MKL as part of the Intel oneAPI Base Toolkit here: https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html

Once you've installed MKL successfully, you'll want to define the `MKLROOT` environment variable to point it at your MKL installation. Once you've done so, the Makefile should detect it and automatically include MKL in any further builds. Note that you will need to `make clean` first. If you've installed MKL to the default directory, you can use the following command:

```bash
export MKLROOT=${HOME}/intel/oneapi/mkl/latest/
```

2. The assignment starter code is available on [github]. Please clone the extra credit starter code using:

    `git clone git@github.com:stanford-cs149/extracredit_gemm`

3. Run the starter code in the `/gemm` directory using the command `./gemm N`.  The parameter `N` specifies the sizes of all dimensions of the matrices.  In other words, if N=256, then the code will create 256x256 matrices A, B, and C, and compute `C = alpha * A * B + beta C` (where alpha and beta are scalars). If you have MKL installed, you should see the following output, which documents the performance of three different implementations (Intel MKL library's performance, a staff reference implementation written in ISPC, and your solution).

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

Implement GEMM on square (NxN) matrices. __(Your solution can assume all matrices are square with power-of-two dimensions.)__. You can implement your solution in plain C (perhaps with vector instrinsics), or use ISPC.  

* If you want to implement the solution completing from scratch, place your solution in `gemm/gemm.cpp`.  You'll find a naive solution already given to you.
* If you want to implement your solution in ISPC, please place it in `gemm/gemm.ispc`.  __You'll need to modify `main.cpp` to call `ispc::gemm_ispc(m, n, k, A2, B2, C2, alpha, beta);`__ instead of `gemm(m, n, k, A2, B2, C2, alpha, beta);` so that the harness runs your ISPC code instead of the C solution.

That's it!  It's an "going further" extra credit, so you are on your own!  However we do first recommend ["blocking" (aka tiling)](https://cs149.stanford.edu/fall21/lecture/perfopt2/slide_54) the loops so that you dramatically improve the cache behavior of the algorithm for larger N.  Specifically, how can you decompose a large matrix multiplication of NxN matrices, written using 3 for loops, into a sequence of smaller KxK matrix multiplications where KxK submatrices of A, B, and C are sized to fit in cache?  Once you get your tiling scheme working, then I'd recommend considering multi-core and SIMD parallelization.

For a better understanding of blocking, I recommend you [take a look at this article](https://csapp.cs.cmu.edu/public/waside/waside-blocking.pdf).


### Grading

__This extra credit will be graded on a case-by-case basis.__  There is no grading harness, however we are looking for documentation of a serious effort to obtain improved performance using ideas like creating parallel work for all cores, blocking to fit in cache (and perhaps even registers), SIMD vector processing (e.g. using ISPC). To give you a ballpark estimate of expectations, we are looking for student work to be approaching that of the reference ISPC implementation to gain this extra credit (although that reference ISPC implementation is pretty good, so don't necessarily expect to beat it to get points).  Although it's on a complete Recently your instructor was impressed by this 

## Hand-in Instructions ##

Handin will be performed via [Gradescope](https://www.gradescope.com/). Please place the following files in your handin:

1. Your implementation of `gemm.ispc`
2. A writeup with:
  1. A graph of GFlops for MKL, your implementation, and the reference ISPC implementation with input argument N of 256, 512, 1024, 2048, and 4096 
  2. A detailed description of the steps you performed in the optimzation process. In particular, please let us know which steps had the biggest benefit.
