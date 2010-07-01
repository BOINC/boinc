Tuan Le
University of California, Berkeley
Berkeley Space Sciences Lab
tuanle86@berkeley.edu

----------------------- Mac OS Makefile ----------------------
By default, the NVIDIA Cuda SDK for MacOS is installed at $ROOT/Developer/ under the directory name "GPU Computing".
If it is installed somewhere else, then common_mac.mk file needs to be edited by the following steps:

1) Open "boinc/samples/nvcuda/common_mac.mk"
2) Look for "ROOTDIR  ?= /DeveloperGPU\ Computing
3) Replace this path by appropriate path on your machine.
4) Done!

----------------------- Mac Troubleshooting ----------------

Error: ./example_app_nvcuda_mac: library not found for -lcutil_i386. 
Solution: run make from "CPU Computing" directory to build the cutil_i386 library.

Note: It appears that nvcc compiler has trouble compiling .cu file that contains both BOINC and 
      Cuda code. Any attempt to compile such .cu files will result in errors like the following:

	/usr/lib/gcc/i686-apple-darwin9/4.0.1/include/mmintrin.h(55): error: identifier "__builtin_ia32_emms" is undefined

	/usr/lib/gcc/i686-apple-darwin9/4.0.1/include/mmintrin.h(68): error: identifier "__builtin_ia32_vec_init_v2si" is undefined

	/usr/lib/gcc/i686-apple-darwin9/4.0.1/include/mmintrin.h(111): error: identifier "__builtin_ia32_vec_ext_v2si" is undefined

	/usr/lib/gcc/i686-apple-darwin9/4.0.1/include/mmintrin.h(150): error: identifier "__builtin_ia32_packsswb" is undefined

	/usr/lib/gcc/i686-apple-darwin9/4.0.1/include/mmintrin.h(165): error: identifier "__builtin_ia32_packssdw" is undefined
      
        ...........
      
      The solution that I came up with is to compile BOINC and Cuda seperately. Put BOINC code in .c file and CUDA code in .cu file.
      Then, write an external function in .cu file that will make kernel calls, do computation and return a result that could be used
      in .c file.     

----------------------- Run Executable file ------------------

"make -f Makefile_mac" command will create an executable file in "boinc/samples/nvcuda/darwin/release/".
If your machine doesn't have CUDA-enabled GPU, then the executable file for this sample app in 
the release directory will stop execution after some output statements on the terminal. In this case,
it's best to run in emurelease mode. To generate an executable file in emurelease mode, type 
"make -f Makefile_mac emu=1". The executable file is then created in "boinc/samples/nvcuda/darwin/emurelease/".
Note that NVIDIA Cuda SDK version older than version 3.0 will no longer support emulation mode.

Also, you need to define some environment variables before running the executable file.

export PATH=/usr/local/cuda/bin:$PATH
export DYLD_LIBRARY_PATH=/usr/local/cuda/lib:$DYLD_LIBRARY_PATH