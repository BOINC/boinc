Tuan Le
University of California, Berkeley
Berkeley Space Sciences Lab
tuanle86@berkeley.edu

----------------------- Linux Makefile ----------------------

Makefile is written with the assumption that the "ati-stream-sdk-v2.1-lnx32"
is used and is installed at the same location as boinc.
If you are using different ATIStream SDK, i.e. ati-stream-sdk-v2.1-lnx64, or
if boinc and ATIStream SDK are not installed at the same location, then Makefile
needs to be edited. There are two places that you need to look at:

1) At "CXFLAGS2"
		-L ../../../ati-stream-sdk-v.2.1-lnx32/lib/x86 \
		-L ../../../ati-stream-sdk-v2.1-lnx32/TempSDKUtil/lib/x86 \

2) At "atiopencl.o: atiopencl.cpp atiopencl.hpp atiopencl_kernels.cl"
		-I ../../../ati-stream-sdk-v2.1-lnx32/include \
		-I ../../../ati-stream-sdk-v2.1-lnx32/samples/opencl/SDKUtil

Modify these command lines so that it refers to the correct SDK directory name
and its path.

----------------------- Linux Troubleshooting ----------------

Most problems related to running ATIStream SDK executable files on Linux can be
found in ATI_Stream_SDK_Installation_Notes.pdf section 2.2

1) ./atiopencl: error while loading shared libraries: libOpenCL.so: cannot open
   shared object file: No such file or directory
   
   Solution: Set the environment variable ATISTREAMSDKROOT
                  -> export ATISTREAMSDKROOT=<location where Stream SDK is extracted>

             Set the library path LD_LIBRARY_PATH
             - For 32-bit systems:
                  -> export LD_LIBRARY_PATH=$ATISTREAMSDKROOT/lib/x86:$LD_LIBRARY_PATH
             - For 64-bit systems:
                  -> export LD_LIBRARY_PATH=$ATISTREAMSDKROOT/lib/x86_64:$LD_LIBRARY_PATH
             - For the 32-bit SDK to run on 64-bit systems:
                  -> export LD_LIBRARY_PATH=$ATISTREAMSDKROOT/lib/x86:$ATISTREAMSDKROOT/lib/x86_64:$LD_LIBRARY_PATH

2) ./atiopencl: error CL_PLATFORM_NOT_FOUND

   Solution: Need to register the OpenCL ICD by following these steps:
             * Download icd-registration.tgz at http://developer.amd.com/Downloads/icd-registration.tgz
             * In the Shell, type "cd /"
             * Type "ls", you will see that there is a directory named "etc". The icd-registration.tgz
               needs to be unzipped to that "etc directory"
	     * Now copy icd-registration.tgz to cd / by typing "sudo cp -r /home/tuanle/Downloads/icd-registration.tgz ."
               You will need to replace the specified path above with the appropriate path on your computer, and
               don't forget "sudo". Otherwise you will get a "Permission denied".
             * Unzip by typing "sudo tar xzf icd-registration.tgz"
	     * Now you should be able to find an "OpenCL" directory in "etc". Check etc/OpenCL/vendors/, make sure there
               are "atiocl32.icd" and "atiocl64.icd" there.
             * DONE!