To build the openclapp sample for Macintosh:

First build the BOINC libraries:
cd [path]/mac_build
source BuildMacBOINC.sh -lib

For more details, see the instructions in that script's comments or at:
[path]/mac_build/HowToBuildBOINC_XCode.rtf

After building the libraries, run the make file in this directory:
cd [path]/samples/openclapp
make -f Makefile_mac

To build for Linux (assuming you have installed the appropriate GPU computing SDK):
For ATI/AMD:
make -f Makefile_AMD

For NVIDIA:
make -f Makefile_NVIDIA

For intel Ivy Bridge: modify one of the above make files for the appropriate paths to the OpenCL headers and libraries.

Adjust the -I and -L arguments for Linux if the OpenCL headers and libraries are in non-standard locations.

To run:
This same sample is designed to run with AMD, NVIDIA and Intel Ivy Bridge GPUs.  It is supplied with 3 minimal init_data.xml files, one for each of these 3 vendors (GPU "types".)  Copy the appropriate init_data.xml file into the directory containing the openclapp executable.  Then run from the Terminal:
$ cd to/the/directory/containing/executable/and/init_data.xml/file
$ ./openclapp [options]

command line options
 -run_slow: sleep 1 second after each character
 -cpu_time N: use about N CPU seconds after copying files
 -early_exit: exit(10) after 30 iterations
 -early_crash: crash after 30 iterations
 -early_sleep: go into infinite sleep after 30 iterations

==============================================================

Important notes about the sample code:

Since a computer can have multiple GPUs, the application must use the GPU assigned by the BOINC client.  To do this, it must call the following API:
int boinc_get_opencl_ids(
    int argc, char** argv, int type,
    cl_device_id* device, cl_platform_id* platform
);

The arguments are as follows:
argc, argv: the argv and argc received by the application's main() from the BOINC client.
type: may be PROC_TYPE_NVIDIA_GPU, PROC_TYPE_AMD_GPU or PROC_TYPE_INTEL_GPU.
device: a pointer to the variable to receive the cl_device_id of the desired GPU.
platform: a pointer to the variable to receive the cl_platform_id of the desired GPU.

Currently, BOINC expects projects to provide separate production applications for each GPU vendor (GPU type), with a separate "plan class" for each.  BOINC currently supports GPUs from the three major vendors: AMD (ATI). NVIDIA or Intel (Ivy Bridge or later).  BOINC refers to the vendors as gpu "types."

Because older clients do not write the <gpu_type> field into the init_data.xml file, your application must pass the appropriate GPU type as the third argument in the boinc_get_opencl_ids() call, or it will not be compatible with older clients.

However, to avoid redundancy, this one sample is designed to work with OpenCl-capable GPUs from any of the three vendors.  To accomplish this, it does not pass a valid type in the boinc_get_opencl_ids() call; it requires init_data.xml file to have a valid <gpu_type> field, and so would not be compatible with older clients.  This shortcut is not acceptable for production OpenCL applications; you _must_ pass in a type of either PROC_TYPE_NVIDIA_GPU, PROC_TYPE_AMD_GPU or PROC_TYPE_INTEL_GPU.

==============================================================

What is the difference between a GPU's gpu_device_num and its gpu_opencl_dev_index?

In most cases, they are identical.  But on Macs which have CUDA installed, Mac OpenCL does not always recognize all NVIDIA GPUs recognized by CUDA.  In that case, the gpu_device_num  is the device's position among all the CUDA-capable GPUs, and the gpu_opencl_dev_index is the device's position among all the OpenCL-capable GPUs.

