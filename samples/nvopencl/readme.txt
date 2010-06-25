Tuan Le
University of California, Berkeley
Berkeley Space Sciences Lab
tuanle86@berkeley.edu


----------------------- Linux Makefile ----------------------

Makefile needs to be edited on your machine before running. Please follow these steps:

1) Open "boinc/samples/nvopencl/common_opencl.mk" with gedit
2) Ctrl+f and search for "tuanle". You will find the following:

ROOTDIR   ?= /home/tuanle/NVIDIA_GPU_Computing_SDK/

3) Replace this path by appropriate path on your machine.
4) Done!


----------------------- Linux Troubleshooting ----------------

Error: make: /usr/bin/ld: cannot find -lOpenCL
Cause: file "libOpenCL.so" is missing
Solution: It appears that NVIDIA SDK 3.0 misses this library file. You can instead download
          the ati-stream-sdk-v2.1-lnx32 and copy file libOpenCL.so from /ati-stream-sdk-v2.1-lnx32/lib/x86/
          to NVIDIA_GPU_Computing_SDK/OpenCL/shared/lib/ . For your convenience, I have put a copy of
          libOpenCL.so in boinc/samples/nvopencl/. You can use this file instead and follow the previous step.

Error: ./example_app_nvopencl -> error while loading shared libraries: libOpenCL.so: cannot open shared object file: No such file
or director
Solution: export LD_LIBRARY_PATH=/home/tuanle/NVIDIA_GPU_Computing_SDK/OpenCL/shared/lib:$LD_LIBRARY_PATH
          This is the directory that you just put in file libOpenCL.so from the previous step. Of course,
          you need to use the correct path on your machine where NVIDIA_GPU_Computing_SDK directory is located.

Error: ./example_app_nvopencl -> Error: Getting Platforms. (clGetPlatformsIDs)
Solution: Unfortunately, unlike NVIDIA cuda, NVIDIA OpenCL will require you to have an OpenCL-enabled NVIDIA GPU
          for it to works. Once you have a machine with working GPU, you will also need to install the appropriate
          NVIDIA driver.

----------------------- Run Executable file ------------------

"make" command will create an executable file in "boinc/samples/nvopencl/linux/release/".
If your machine doesn't have CUDA-enabled GPU, then the executable file for this sample app in 
the release directory will stop execution after the statement "Start at inversion #1" with the error message
"Error: Getting Platforms. (clGetPlatformsIDs)"
