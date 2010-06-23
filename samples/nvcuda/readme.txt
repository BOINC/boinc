Tuan Le
University of California, Berkeley
Berkeley Space Sciences Lab
tuanle86@berkeley.edu


----------------------- Linux Makefile ----------------------

Makefile needs to be edited on your machine before running. Please follow these steps:

1) Open "boinc/samples/nvcuda/common.mk" with gedit
2) Ctrl+f and search for "tuanle". You will find the following:

LIBDIR     := /home/tuanle/NVIDIA_GPU_Computing_SDK/C/lib

COMMONDIR  := /home/tuanle/NVIDIA_GPU_Computing_SDK/C/common

SHAREDDIR  := /home/tuanle/NVIDIA_GPU_Computing_SDK/shared
...
...
NVCUVIDLIB := -L/home/tuanle/NVIDIA_GPU_Computing_SDK/C/common/lib/darwin -lnvcuvid

3) Replace these above paths by appropriate paths on your machine.
4) Done!


----------------------- Linux Troubleshooting ----------------

Error: ./example_app_nvcuda: error while loading shared libraries: libcudart.so.3: cannot open
       shared object file: No such file or directory
Read: http://developer.download.nvidia.com/compute/cuda/3_0/docs/GettingStartedLinux.pdf  (on top of page 6)
Solution: export PATH=/usr/local/cuda/bin:$PATH
          export LD_LIBRARY_PATH=/usr/local/cuda/lib:$LD_LIBRARY_PATH


----------------------- Run Executable file ------------------

"make" command will create an executable file in "boinc/samples/nvcuda/linux/release/".
If your machine doesn't have CUDA-enabled GPU, then the executable file for this sample app in 
the release directory will stop execution after the statement "Start at inversion #1" is printed out on
the terminal. In this case, it's best to run in emurelease mode. To generate an executable file in
emurelease mode, type "make emu=1". The executable file is then created in "boinc/samples/nvcuda/linux/emurelease/".
