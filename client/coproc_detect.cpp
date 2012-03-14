// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


// client-specific GPU code.  Mostly GPU detection

#define FAKENVIDIACUDA0 0
#define FAKE2NVIDIAOPENCLS 0

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#ifndef SIM
#include <nvapi.h>
#endif
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#endif
#include "config.h"
#include <dlfcn.h>
#include <setjmp.h>
#include <signal.h>
#endif

#include "coproc.h"
#include "str_util.h"
#include "util.h"

#include "client_state.h"
#include "client_msgs.h"

using std::string;
using std::vector;

static bool in_vector(int n, vector<int>& v) {
    for (unsigned int i=0; i<v.size(); i++) {
        if (v[i] == n) return true;
    }
    return false;
}

#ifndef _WIN32
jmp_buf resume;

void segv_handler(int) {
    longjmp(resume, 1);
}
#endif

vector<COPROC_ATI> ati_gpus;
vector<COPROC_NVIDIA> nvidia_gpus;
vector<OPENCL_DEVICE_PROP> nvidia_opencls;
vector<OPENCL_DEVICE_PROP> ati_opencls;


void COPROCS::get(
    bool use_all, vector<string>&descs, vector<string>&warnings,
    vector<int>& ignore_nvidia_dev,
    vector<int>& ignore_ati_dev
) {
    unsigned int i;
    char buf[256], buf2[256];

#ifdef _WIN32
    try {
        nvidia.get(use_all, warnings, ignore_nvidia_dev);
    }
    catch (...) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    }
    try {
        ati.get(use_all, warnings, ignore_ati_dev);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    }
    try {
        get_opencl(use_all, warnings, ignore_ati_dev, ignore_nvidia_dev);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    }
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        nvidia.get(use_all, warnings, ignore_nvidia_dev);
    }
#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    } else {
        ati.get(use_all, warnings, ignore_ati_dev);
    }
#endif
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    } else {
        get_opencl(use_all, warnings, ignore_ati_dev, ignore_nvidia_dev);
    }
    signal(SIGSEGV, old_sig);
#endif

    for (i=0; i<nvidia_gpus.size(); i++) {
        nvidia_gpus[i].description(buf);
        switch(nvidia_gpus[i].is_used) {
        case COPROC_IGNORED:
            sprintf(buf2, "NVIDIA GPU %d (ignored by config): %s", nvidia_gpus[i].device_num, buf);
            break;
        case COPROC_USED:
            sprintf(buf2, "NVIDIA GPU %d: %s", nvidia_gpus[i].device_num, buf);
            break;
        case COPROC_UNUSED:
        default:
            sprintf(buf2, "NVIDIA GPU %d (not used): %s", nvidia_gpus[i].device_num, buf);
            break;
        }
        descs.push_back(string(buf2));
    }

    for (i=0; i<ati_gpus.size(); i++) {
        ati_gpus[i].description(buf);
        switch(ati_gpus[i].is_used) {
        case COPROC_IGNORED:
            sprintf(buf2, "ATI GPU %d (ignored by config): %s", ati_gpus[i].device_num, buf);
            break;
        case COPROC_USED:
            sprintf(buf2, "ATI GPU %d: %s", ati_gpus[i].device_num, buf);
            break;
        case COPROC_UNUSED:
        default:
            sprintf(buf2, "ATI GPU %d: (not used) %s", ati_gpus[i].device_num, buf);
            break;
        }
        descs.push_back(string(buf2));
    }

    // Create descriptions for OpenCL NVIDIA GPUs
    //
    for (i=0; i<nvidia_opencls.size(); i++) {
        nvidia_opencls[i].description(buf, GPU_TYPE_NVIDIA);
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL ATI GPUs
    //
    for (i=0; i<ati_opencls.size(); i++) {
        ati_opencls[i].description(buf, GPU_TYPE_ATI);
        descs.push_back(string(buf));
    }

    ati_gpus.clear();
    nvidia_gpus.clear();
    nvidia_opencls.clear();
    ati_opencls.clear();
}


////////////////// OPENCL STARTS HERE /////////////////
//

#ifdef _WIN32

HMODULE opencl_lib = NULL;

typedef cl_int (__stdcall *CL_PLATFORMIDS) (cl_uint, cl_platform_id*, cl_uint*);
typedef cl_int (__stdcall *CL_PLATFORMINFO) (cl_platform_id, cl_platform_info, size_t, void*, size_t*);
typedef cl_int (__stdcall *CL_DEVICEIDS)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
typedef cl_int (__stdcall *CL_INFO) (cl_device_id, cl_device_info, size_t, void*, size_t*);

CL_PLATFORMIDS  __clGetPlatformIDs = NULL;
CL_PLATFORMINFO __clGetPlatformInfo = NULL;
CL_DEVICEIDS    __clGetDeviceIDs = NULL;
CL_INFO         __clGetDeviceInfo = NULL;

#else

void* opencl_lib = NULL;

cl_int (*__clGetPlatformIDs)(cl_uint    /* num_entries */,
                 cl_platform_id * /* platforms */,
                 cl_uint *        /* num_platforms */);
cl_int (*__clGetPlatformInfo)(cl_platform_id   /* platform */,
                  cl_platform_info /* param_name */,
                  size_t           /* param_value_size */,
                  void *           /* param_value */,
                  size_t *         /* param_value_size_ret */);
cl_int (*__clGetDeviceIDs)(cl_platform_id   /* platform */,
               cl_device_type   /* device_type */,
               cl_uint          /* num_entries */,
               cl_device_id *   /* devices */,
               cl_uint *        /* num_devices */);
cl_int (*__clGetDeviceInfo)(cl_device_id    /* device */,
                cl_device_info  /* param_name */,
                size_t          /* param_value_size */,
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */);

#endif

// If "loose", tolerate small diff
//
int opencl_compare(OPENCL_DEVICE_PROP& c1, OPENCL_DEVICE_PROP& c2, bool loose) {
    if (c1.opencl_device_version_int > c2.opencl_device_version_int) return 1;
    if (c1.opencl_device_version_int < c2.opencl_device_version_int) return -1;
    if (loose) {
        if (c1.global_mem_size > 1.4*c2.global_mem_size) return 1;
        if (c1.global_mem_size < .7*c2.global_mem_size) return -1;
        return 0;
    }
    if (c1.global_mem_size > c2.global_mem_size) return 1;
    if (c1.global_mem_size < c2.global_mem_size) return -1;
    if (c1.peak_flops > c2.peak_flops) return 1;
    if (c1.peak_flops < c2.peak_flops) return -1;
    return 0;
}

// OpenCL interfaces are documented here:
// http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/ and 
// http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/

void COPROCS::get_opencl(
    bool use_all,
    vector<string>& warnings, 
    vector<int>& ignore_ati_dev,
    vector<int>& ignore_nvidia_dev
) {
    cl_int ciErrNum;
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices, device_index;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    char platform_version[256];
    OPENCL_DEVICE_PROP prop;
    COPROC_NVIDIA nvidia_temp;
    COPROC_ATI ati_temp;
    int current_CUDA_index;
    char buf[256];

#ifdef _WIN32
    opencl_lib = LoadLibrary("OpenCL.dll");
    if (!opencl_lib) {
        warnings.push_back("No OpenCL library found");
        return;
    }

    __clGetPlatformIDs = (CL_PLATFORMIDS)GetProcAddress( opencl_lib, "clGetPlatformIDs" );
    __clGetPlatformInfo = (CL_PLATFORMINFO)GetProcAddress( opencl_lib, "clGetPlatformInfo" );
    __clGetDeviceIDs = (CL_DEVICEIDS)GetProcAddress( opencl_lib, "clGetDeviceIDs" );
    __clGetDeviceInfo = (CL_INFO)GetProcAddress( opencl_lib, "clGetDeviceInfo" );
#else
#ifdef __APPLE__
    opencl_lib = dlopen("/System/Library/Frameworks/OpenCL.framework/Versions/Current/OpenCL", RTLD_NOW);
#else
//TODO: Is this correct?
    opencl_lib = dlopen("libOpenCL.so", RTLD_NOW);
#endif
    if (!opencl_lib) {
        warnings.push_back("No OpenCL library found");
        return;
    }
    __clGetPlatformIDs = (cl_int(*)(cl_uint, cl_platform_id*, cl_uint*)) dlsym( opencl_lib, "clGetPlatformIDs" );
    __clGetPlatformInfo = (cl_int(*)(cl_platform_id, cl_platform_info, size_t, void*, size_t*)) dlsym( opencl_lib, "clGetPlatformInfo" );
    __clGetDeviceIDs = (cl_int(*)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*)) dlsym( opencl_lib, "clGetDeviceIDs" );
    __clGetDeviceInfo = (cl_int(*)(cl_device_id, cl_device_info, size_t, void*, size_t*)) dlsym( opencl_lib, "clGetDeviceInfo" );
#endif

    if (!__clGetPlatformIDs) {
        warnings.push_back("clGetPlatformIDs() missing from OpenCL library");
        return;
    }
    if (!__clGetPlatformInfo) {
        warnings.push_back("clGetPlatformInfo() missing from OpenCL library");
        return;
    }
    if (!__clGetDeviceIDs) {
        warnings.push_back("clGetDeviceIDs() missing from OpenCL library");
        return;
    }
    if (!__clGetDeviceInfo) {
        warnings.push_back("clGetDeviceInfo() missing from OpenCL library");
        return;
    }

    ciErrNum = (*__clGetPlatformIDs)(MAX_OPENCL_PLATFORMS, platforms, &num_platforms);
    if ((ciErrNum != CL_SUCCESS) || (num_platforms == 0)) {
        warnings.push_back("clGetPlatformIDs() failed to return any OpenCL platforms");
        return;
    }
    
    for (platform_index=0; platform_index<num_platforms; ++platform_index) {
        ciErrNum = (*__clGetPlatformInfo)(
            platforms[platform_index], CL_PLATFORM_VERSION, sizeof(platform_version), &platform_version, NULL
        );
        if (ciErrNum != CL_SUCCESS) {
            sprintf(buf, "clGetPlatformInfo CL_PLATFORM_VERSION for platform #%d returned error %d", platform_index, ciErrNum);
            warnings.push_back(buf);
            return;
        }

        ciErrNum = (*__clGetDeviceIDs)(
            platforms[platform_index], CL_DEVICE_TYPE_GPU, MAX_COPROC_INSTANCES, devices, &num_devices
        );
        if (ciErrNum != CL_SUCCESS) {
            sprintf(buf, "clGetDeviceIDs for platform #%d returned error %d", platform_index, ciErrNum);
            warnings.push_back(buf);
            return;
        }

#if FAKE2NVIDIAOPENCLS
num_devices = 3;
devices[2] = devices[1];
#endif

        // Mac OpenCL does not recognize all NVIDIA GPUs returned by CUDA
        current_CUDA_index = 0;

        for (device_index=0; device_index<num_devices; ++device_index) {
            memset(&prop, 0, sizeof(prop));
            prop.device_id = devices[device_index];
            strncpy(prop.opencl_platform_version, platform_version, sizeof(prop.opencl_platform_version)-1);
            
//TODO: Should we store the platform(s) for each GPU found?
//TODO: Must we check if multiple platforms found the same GPU and merge the records?
#if FAKE2NVIDIAOPENCLS
if (device_index == 2) {
strcpy(prop.name, "GEForce 120 GT");
strcpy(prop.vendor, "NVIDIA");
prop.vendor_id = 16918016;
prop.available = 1;
prop.half_fp_config = 0;
prop.single_fp_config = 30;
prop.double_fp_config = 63;
prop.endian_little = 1;
prop.execution_capabilities = 1;
strcpy(prop.extensions, "cl_APPLE_SetMemObjectDestructor cl_APPLE_ContextLoggingFunctions cl_APPLE_clut cl_APPLE_query_kernel_names cl_APPLE_gl_sharing cl_khr_gl_event cl_khr_byte_addressable_store cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics ");
prop.global_mem_size = 268435456;
prop.local_mem_size = 16384;
prop.max_clock_frequency = 1000;
prop.max_compute_units = 10;
strcpy(prop.opencl_device_version, "OpenCL 1.0 ");
strcpy(prop.opencl_driver_version, "CLH 1.0");
} else
#endif
            ciErrNum = get_opencl_info(prop, device_index, warnings);
            if (ciErrNum != CL_SUCCESS) break;
            
            prop.is_used = COPROC_UNUSED;
            prop.get_device_version_int();

            if (strstr(prop.vendor, GPU_TYPE_NVIDIA)) {
                if (nvidia.have_cuda) {
                    // Mac OpenCL does not recognize all NVIDIA GPUs returned by 
                    // CUDA but we assume that OpenCL and CUDA return devices in 
                    // the same order and with identical model name strings
                    while(1) {
                        if (current_CUDA_index >= (int)(nvidia_gpus.size())) {
                            if (log_flags.coproc_debug) {
                                msg_printf(0, MSG_INFO,
                                "[coproc] OpenCL NVIDIA index #%d does not match any CUDA device", 
                                device_index
                                );
                            }
                            return; // Should never happen
                        }
                        if (!strcmp(prop.name, nvidia_gpus[current_CUDA_index].prop.name)) {
                            break;  // We have a match
                        }
                        // This CUDA GPU is not recognized by OpenCL, so try the next
                        ++current_CUDA_index;
                    }
                    prop.device_num = current_CUDA_index;
                } else {
                    prop.device_num = (int)(nvidia_opencls.size());
                }
                prop.opencl_device_index = device_index;

                if (!nvidia.have_cuda) {
                    COPROC_NVIDIA c;
                    c.opencl_prop = prop;
                    c.set_peak_flops();
                    prop.peak_flops = c.peak_flops;
                }
                if (nvidia_gpus.size()) {
                    // Assumes OpenCL and CUDA return the devices in the same order
                    prop.opencl_available_ram = nvidia_gpus[prop.device_num].available_ram;
                } else {
                    prop.opencl_available_ram = prop.global_mem_size;
                }
                nvidia_opencls.push_back(prop);
                ++current_CUDA_index;
            }
            if ((strstr(prop.vendor, GPU_TYPE_ATI)) || 
                (strstr(prop.vendor, "AMD")) ||  
                (strstr(prop.vendor, "Advanced Micro Devices, Inc."))
            ) {
                prop.device_num = (int)(ati_opencls.size());
                prop.opencl_device_index = device_index;
                
                if (ati.have_cal) {
                    if (prop.device_num < (int)(ati_gpus.size())) {
                        // Always use GPU model name from OpenCL if available for ATI / AMD 
                        // GPUs because (we believe) it is more reliable and user-friendly.
                        // Assumes OpenCL and CAL return the devices in the same order
                        strcpy(ati_gpus[prop.device_num].name, prop.name);

                        // Work around a bug in OpenCL which returns only 
                        // 1/2 of total global RAM size: use the value from CAL. 
                        // This bug applies only to ATI GPUs, not to NVIDIA
                        // See also further workaround code for Macs.
                        //
                        prop.global_mem_size = ati_gpus[prop.device_num].attribs.localRAM * MEGA;
                    } else {
                        if (log_flags.coproc_debug) {
                            msg_printf(0, MSG_INFO,
                            "[coproc] OpenCL ATI device #%d does not match any CAL device", 
                            device_index
                            );
                        }
                    }
                } else {            // ! ati.have_cal
                    COPROC_ATI c;
                    c.opencl_prop = prop;
                    c.set_peak_flops();
                    prop.peak_flops = c.peak_flops;
                }
                
                if (ati_gpus.size()) {
                    // Assumes OpenCL and CAL return the same device with the same index
                    prop.opencl_available_ram = ati_gpus[prop.device_num].available_ram;
                } else {
                    prop.opencl_available_ram = prop.global_mem_size;
                }
                ati_opencls.push_back(prop);
            }
        }
    }


#ifdef __APPLE__
                // Work around a bug in OpenCL which returns only 
                // 1/2 of total global RAM size. 
                // This bug applies only to ATI GPUs, not to NVIDIA
                // This has already been fixed on latest Catalyst 
                // drivers, but Mac does not use Catalyst drivers.

    get_ati_mem_size_from_opengl();

#endif

    if ((nvidia_opencls.size() == 0) && (ati_opencls.size() == 0)) {
        warnings.push_back("OpenCL library present but no OpenCL-capable GPUs found");
        return;
    }
        
    if (nvidia.have_cuda) { // If CUDA already found the "best" NVIDIA GPU
        nvidia.merge_opencl(nvidia_opencls, ignore_nvidia_dev);
    } else {
        nvidia.find_best_opencls(use_all, nvidia_opencls, ignore_nvidia_dev);
        nvidia.prop.totalGlobalMem = nvidia.opencl_prop.global_mem_size;
        nvidia.available_ram = nvidia.opencl_prop.global_mem_size;
        nvidia.prop.clockRate = nvidia.opencl_prop.max_clock_frequency * 1000;
        strcpy(nvidia.prop.name, prop.name);
    }

    if (ati.have_cal) { // If CAL already found the "best" CAL GPU
        ati.merge_opencl(ati_opencls, ignore_ati_dev);
    } else {
        ati.find_best_opencls(use_all, ati_opencls, ignore_ati_dev);
        ati.attribs.localRAM = ati.opencl_prop.global_mem_size/MEGA;
        ati.available_ram = ati.opencl_prop.global_mem_size;
        ati.attribs.engineClock = ati.opencl_prop.max_clock_frequency;
        strcpy(ati.name, prop.name);
    }           // End if (! ati.have_cal)

//TODO: Add code to allow adding other GPU vendors
}

cl_int COPROCS::get_opencl_info(
    OPENCL_DEVICE_PROP& prop, 
    cl_uint device_index, 
    vector<string>&warnings
) {
    cl_int ciErrNum;
    char buf[256];
    
    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_NAME, sizeof(prop.name), prop.name, NULL);
    if ((ciErrNum != CL_SUCCESS) || (prop.name[0] == 0)) {
        sprintf(buf, "clGetDeviceInfo failed to get name for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }


    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_VENDOR, sizeof(prop.vendor), prop.vendor, NULL);
    if ((ciErrNum != CL_SUCCESS) || (prop.vendor[0] == 0)) {
        sprintf(buf, "clGetDeviceInfo failed to get vendor for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }


    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_VENDOR_ID, sizeof(prop.vendor_id), &prop.vendor_id, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get vendor ID for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_AVAILABLE, sizeof(prop.available), &prop.available, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get availability for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_HALF_FP_CONFIG,
        sizeof(prop.half_fp_config), &prop.half_fp_config, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        if ((ciErrNum == CL_INVALID_VALUE) || (ciErrNum == CL_INVALID_OPERATION)) {
            prop.half_fp_config = 0;  // Not supported by OpenCL 1.0
        } else {
            sprintf(buf, "clGetDeviceInfo failed to get half-precision floating point capabilities for GPU %d", (int)device_index);
            warnings.push_back(buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_SINGLE_FP_CONFIG,
        sizeof(prop.single_fp_config), &prop.single_fp_config, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get single-precision floating point capabilities for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_DOUBLE_FP_CONFIG,
        sizeof(prop.double_fp_config), &prop.double_fp_config, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        if ((ciErrNum == CL_INVALID_VALUE) || (ciErrNum == CL_INVALID_OPERATION)) {
            prop.double_fp_config = 0;  // Not supported by OpenCL 1.0
        } else {
            sprintf(buf, "clGetDeviceInfo failed to get double-precision floating point capabilities for GPU %d", (int)device_index);
            warnings.push_back(buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_ENDIAN_LITTLE, sizeof(prop.endian_little),
        &prop.endian_little, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get little or big endian for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_EXECUTION_CAPABILITIES,
        sizeof(prop.execution_capabilities), &prop.execution_capabilities, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get execution capabilities for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_EXTENSIONS, sizeof(prop.extensions),
        prop.extensions, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get device extensions for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_GLOBAL_MEM_SIZE,
        sizeof(prop.global_mem_size), &prop.global_mem_size, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get global memory size for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }


    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_LOCAL_MEM_SIZE,
        sizeof(prop.local_mem_size), &prop.local_mem_size, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get local memory size for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY,
        sizeof(prop.max_clock_frequency), &prop.max_clock_frequency, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get max clock frequency for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_MAX_COMPUTE_UNITS,
        sizeof(prop.max_compute_units), &prop.max_compute_units, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get max compute units for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_VERSION, sizeof(prop.opencl_device_version), prop.opencl_device_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get OpenCL version supported by GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DRIVER_VERSION, sizeof(prop.opencl_driver_version), prop.opencl_driver_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get OpenCL driver version for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    return CL_SUCCESS;
}

// This is called for ATI GPUs with CAL or NVIDIA GPUs with CUDA, to merge 
// the OpenCL info into the CAL or CUDA data for the "best" CAL or CUDA GPU.
// This assumes that, for each GPU, we have previously correlated its CAL 
// or CUDA device_num with its opencl_device_index.
void COPROC::merge_opencl(
    vector<OPENCL_DEVICE_PROP> &opencls, 
    vector<int>& ignore_dev
) {
    unsigned int i, j;
    
    for (i=0; i<opencls.size(); i++) {
        if (in_vector(opencls[i].device_num, ignore_dev)) {
            opencls[i].is_used = COPROC_IGNORED;
            continue;
        }
        if (device_num == opencls[i].device_num) {
            opencl_prop = opencls[i];
            opencl_device_ids[0] = opencls[i].device_id;
            have_opencl = true;
            break;
        }
    }
    
    opencl_device_count = 0;
    // Fill in info for other GPUs which CAL or CUDA found equivalent to best
    for (i=0; i<(unsigned int)count; ++i) {
        for (j=0; j<opencls.size(); j++) {
            if (device_nums[i] == opencls[j].device_num) {
                opencls[j].is_used = COPROC_USED;
                opencl_device_indexes[opencl_device_count] = opencls[j].opencl_device_index;
                opencl_device_ids[opencl_device_count++] = opencls[j].device_id;
            }
        }
    }
}

// This is called for ATI GPUs without CAL or NVIDIA GPUs without CUDA
void COPROC::find_best_opencls(
    bool use_all,
    vector<OPENCL_DEVICE_PROP> &opencls, 
    vector<int>& ignore_dev
) {
    unsigned int i;
    
    // identify the most capable ATI or NVIDIA OpenCL GPU
    //
    bool first = true;
    for (i=0; i<opencls.size(); i++) {
        if (in_vector(opencls[i].device_num, ignore_dev)) {
            opencls[i].is_used = COPROC_IGNORED;
            continue;
        }
        bool is_best = false;
        if (first) {
            is_best = true;
            first = false;
        } else if (opencl_compare(opencls[i], opencl_prop, false) > 0) {
            is_best = true;
        }
        if (is_best) {
            // fill in what info we have
            opencl_prop = opencls[i];
            device_num = opencls[i].device_num;
            peak_flops = opencls[i].peak_flops;
            have_opencl = true;
        }
    }

    // see which other instances are equivalent, and set the count, 
    // device_nums, opencl_device_count and opencl_device_ids fields
    //
    count = 0;
    opencl_device_count = 0;
    for (i=0; i<opencls.size(); i++) {
        if (in_vector(opencls[i].device_num, ignore_dev)) {
            opencls[i].is_used = COPROC_IGNORED;
            continue;
        }
        if (use_all || !opencl_compare(opencls[i], opencl_prop, true)) {
            device_nums[count++] = opencls[i].device_num;
            opencl_device_indexes[opencl_device_count] = opencls[i].opencl_device_index;
            opencl_device_ids[opencl_device_count++] = opencls[i].device_id;
            opencls[i].is_used = COPROC_USED;
        }
    }
}

////////////////// NVIDIA STARTS HERE /////////////////
//

// return 1/-1/0 if device 1 is more/less/same capable than device 2.
// factors (decreasing priority):
// - compute capability
// - software version
// - available memory
// - speed
//
// If "loose", ignore FLOPS and tolerate small memory diff
//
int nvidia_compare(COPROC_NVIDIA& c1, COPROC_NVIDIA& c2, bool loose) {
    if (c1.prop.major > c2.prop.major) return 1;
    if (c1.prop.major < c2.prop.major) return -1;
    if (c1.prop.minor > c2.prop.minor) return 1;
    if (c1.prop.minor < c2.prop.minor) return -1;
    if (c1.cuda_version > c2.cuda_version) return 1;
    if (c1.cuda_version < c2.cuda_version) return -1;
    if (loose) {
        if (c1.available_ram> 1.4*c2.available_ram) return 1;
        if (c1.available_ram < .7* c2.available_ram) return -1;
        return 0;
    }
    if (c1.available_ram > c2.available_ram) return 1;
    if (c1.available_ram < c2.available_ram) return -1;
    double s1 = c1.peak_flops;
    double s2 = c2.peak_flops;
    if (s1 > s2) return 1;
    if (s1 < s2) return -1;
    return 0;
}

enum CUdevice_attribute_enum {
  CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK = 1,
  CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X = 2,
  CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y = 3,
  CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z = 4,
  CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X = 5,
  CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y = 6,
  CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z = 7,
  CU_DEVICE_ATTRIBUTE_SHARED_MEMORY_PER_BLOCK = 8,
  CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY = 9,
  CU_DEVICE_ATTRIBUTE_WARP_SIZE = 10,
  CU_DEVICE_ATTRIBUTE_MAX_PITCH = 11,
  CU_DEVICE_ATTRIBUTE_REGISTERS_PER_BLOCK = 12,
  CU_DEVICE_ATTRIBUTE_CLOCK_RATE = 13,
  CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT = 14,
  CU_DEVICE_ATTRIBUTE_GPU_OVERLAP = 15,
  CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT = 16,
  CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT = 17,
  CU_DEVICE_ATTRIBUTE_INTEGRATED = 18,
  CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY = 19,
  CU_DEVICE_ATTRIBUTE_COMPUTE_MODE = 20
};

#ifdef _WIN32
typedef int (__stdcall *CUDA_GDC)(int *count);
typedef int (__stdcall *CUDA_GDV)(int* version);
typedef int (__stdcall *CUDA_GDI)(int);
typedef int (__stdcall *CUDA_GDG)(int*, int);
typedef int (__stdcall *CUDA_GDA)(int*, int, int);
typedef int (__stdcall *CUDA_GDN)(char*, int, int);
typedef int (__stdcall *CUDA_GDM)(unsigned int*, int);
typedef int (__stdcall *CUDA_GDCC)(int*, int*, int);
typedef int (__stdcall *CUDA_CC)(void**, unsigned int, unsigned int);
typedef int (__stdcall *CUDA_CD)(void*);
typedef int (__stdcall *CUDA_MA)(unsigned int*, unsigned int);
typedef int (__stdcall *CUDA_MF)(unsigned int);
typedef int (__stdcall *CUDA_MGI)(unsigned int*, unsigned int*);

CUDA_GDC __cuDeviceGetCount = NULL;
CUDA_GDV __cuDriverGetVersion = NULL;
CUDA_GDI __cuInit = NULL;
CUDA_GDG __cuDeviceGet = NULL;
CUDA_GDA __cuDeviceGetAttribute = NULL;
CUDA_GDN __cuDeviceGetName = NULL;
CUDA_GDM __cuDeviceTotalMem = NULL;
CUDA_GDCC __cuDeviceComputeCapability = NULL;
CUDA_CC __cuCtxCreate = NULL;
CUDA_CD __cuCtxDestroy = NULL;
CUDA_MA __cuMemAlloc = NULL;
CUDA_MF __cuMemFree = NULL;
CUDA_MGI __cuMemGetInfo = NULL;
#else
void* cudalib;
int (*__cuInit)(int);
int (*__cuDeviceGetCount)(int*);
int (*__cuDriverGetVersion)(int*);
int (*__cuDeviceGet)(int*, int);
int (*__cuDeviceGetAttribute)(int*, int, int);
int (*__cuDeviceGetName)(char*, int, int);
int (*__cuDeviceTotalMem)(unsigned int*, int);
int (*__cuDeviceComputeCapability)(int*, int*, int);
int (*__cuCtxCreate)(void**, unsigned int, unsigned int);
int (*__cuCtxDestroy)(void*);
int (*__cuMemAlloc)(unsigned int*, unsigned int);
int (*__cuMemFree)(unsigned int);
int (*__cuMemGetInfo)(unsigned int*, unsigned int*);
#endif

// NVIDIA interfaces are documented here:
// http://developer.download.nvidia.com/compute/cuda/2_3/toolkit/docs/online/index.html

void COPROC_NVIDIA::get(
    bool use_all,    // if false, use only those equivalent to most capable
    vector<string>& warnings,
    vector<int>& ignore_devs
) {
    int cuda_ndevs, retval;
    char buf[256];

#ifdef _WIN32
    HMODULE cudalib = LoadLibrary("nvcuda.dll");
    if (!cudalib) {
        warnings.push_back("No NVIDIA library found");
        return;
    }
    __cuDeviceGetCount = (CUDA_GDC)GetProcAddress( cudalib, "cuDeviceGetCount" );
    __cuDriverGetVersion = (CUDA_GDV)GetProcAddress( cudalib, "cuDriverGetVersion" );
    __cuInit = (CUDA_GDI)GetProcAddress( cudalib, "cuInit" );
    __cuDeviceGet = (CUDA_GDG)GetProcAddress( cudalib, "cuDeviceGet" );
    __cuDeviceGetAttribute = (CUDA_GDA)GetProcAddress( cudalib, "cuDeviceGetAttribute" );
    __cuDeviceGetName = (CUDA_GDN)GetProcAddress( cudalib, "cuDeviceGetName" );
    __cuDeviceTotalMem = (CUDA_GDM)GetProcAddress( cudalib, "cuDeviceTotalMem" );
    __cuDeviceComputeCapability = (CUDA_GDCC)GetProcAddress( cudalib, "cuDeviceComputeCapability" );
    __cuCtxCreate = (CUDA_CC)GetProcAddress( cudalib, "cuCtxCreate" );
    __cuCtxDestroy = (CUDA_CD)GetProcAddress( cudalib, "cuCtxDestroy" );
    __cuMemAlloc = (CUDA_MA)GetProcAddress( cudalib, "cuMemAlloc" );
    __cuMemFree = (CUDA_MF)GetProcAddress( cudalib, "cuMemFree" );
    __cuMemGetInfo = (CUDA_MGI)GetProcAddress( cudalib, "cuMemGetInfo" );

#ifndef SIM
    NvAPI_Status nvapiStatus;
    NV_DISPLAY_DRIVER_VERSION Version;
    memset(&Version, 0, sizeof(Version));
    Version.version = NV_DISPLAY_DRIVER_VERSION_VER;

    NvAPI_Initialize();
    nvapiStatus = NvAPI_GetDisplayDriverVersion(NULL, &Version);
#endif
#else

#ifdef __APPLE__
    cudalib = dlopen("/usr/local/cuda/lib/libcuda.dylib", RTLD_NOW);
#else
    cudalib = dlopen("libcuda.so", RTLD_NOW);
#endif
    if (!cudalib) {
        warnings.push_back("No NVIDIA library found");
        return;
    }
    __cuDeviceGetCount = (int(*)(int*)) dlsym(cudalib, "cuDeviceGetCount");
    __cuDriverGetVersion = (int(*)(int*)) dlsym( cudalib, "cuDriverGetVersion" );
    __cuInit = (int(*)(int)) dlsym( cudalib, "cuInit" );
    __cuDeviceGet = (int(*)(int*, int)) dlsym( cudalib, "cuDeviceGet" );
    __cuDeviceGetAttribute = (int(*)(int*, int, int)) dlsym( cudalib, "cuDeviceGetAttribute" );
    __cuDeviceGetName = (int(*)(char*, int, int)) dlsym( cudalib, "cuDeviceGetName" );
    __cuDeviceTotalMem = (int(*)(unsigned int*, int)) dlsym( cudalib, "cuDeviceTotalMem" );
    __cuDeviceComputeCapability = (int(*)(int*, int*, int)) dlsym( cudalib, "cuDeviceComputeCapability" );
    __cuCtxCreate = (int(*)(void**, unsigned int, unsigned int)) dlsym( cudalib, "cuCtxCreate" );
    __cuCtxDestroy = (int(*)(void*)) dlsym( cudalib, "cuCtxDestroy" );
    __cuMemAlloc = (int(*)(unsigned int*, unsigned int)) dlsym( cudalib, "cuMemAlloc" );
    __cuMemFree = (int(*)(unsigned int)) dlsym( cudalib, "cuMemFree" );
    __cuMemGetInfo = (int(*)(unsigned int*, unsigned int*)) dlsym( cudalib, "cuMemGetInfo" );
#endif

    if (!__cuDriverGetVersion) {
        warnings.push_back("cuDriverGetVersion() missing from NVIDIA library");
        return;
    }
    if (!__cuInit) {
        warnings.push_back("cuInit() missing from NVIDIA library");
        return;
    }
    if (!__cuDeviceGetCount) {
        warnings.push_back("cuDeviceGetCount() missing from NVIDIA library");
        return;
    }
    if (!__cuDeviceGet) {
        warnings.push_back("cuDeviceGet() missing from NVIDIA library");
        return;
    }
    if (!__cuDeviceGetAttribute) {
        warnings.push_back("cuDeviceGetAttribute() missing from NVIDIA library");
        return;
    }
    if (!__cuDeviceTotalMem) {
        warnings.push_back("cuDeviceTotalMem() missing from NVIDIA library");
        return;
    }
    if (!__cuDeviceComputeCapability) {
        warnings.push_back("cuDeviceComputeCapability() missing from NVIDIA library");
        return;
    }
    if (!__cuCtxCreate) {
        warnings.push_back("cuCtxCreate() missing from NVIDIA library");
        return;
    }
    if (!__cuCtxDestroy) {
        warnings.push_back("cuCtxDestroy() missing from NVIDIA library");
        return;
    }
    if (!__cuMemAlloc) {
        warnings.push_back("cuMemAlloc() missing from NVIDIA library");
        return;
    }
    if (!__cuMemFree) {
        warnings.push_back("cuMemFree() missing from NVIDIA library");
        return;
    }
    if (!__cuMemGetInfo) {
        warnings.push_back("cuMemGetInfo() missing from NVIDIA library");
        return;
    }

    retval = (*__cuInit)(0);
    if (retval) {
        sprintf(buf, "NVIDIA drivers present but no GPUs found");
        warnings.push_back(buf);
        return;
    }

    retval = (*__cuDriverGetVersion)(&cuda_version);
    if (retval) {
        sprintf(buf, "cuDriverGetVersion() returned %d", retval);
        warnings.push_back(buf);
        return;
    }

    retval = (*__cuDeviceGetCount)(&cuda_ndevs);
    if (retval) {
        sprintf(buf, "cuDeviceGetCount() returned %d", retval);
        warnings.push_back(buf);
        return;
    }
    sprintf(buf, "NVIDIA library reports %d GPU%s", cuda_ndevs, (cuda_ndevs==1)?"":"s");
    warnings.push_back(buf);

    int j;
    unsigned int i;
    COPROC_NVIDIA cc;
    string s;
    for (j=0; j<cuda_ndevs; j++) {
        memset(&cc.prop, 0, sizeof(cc.prop));
        int device;
#if FAKENVIDIACUDA0
if (j == 0) {
 cc.fake(0x40032, 256*MEGA, 64*MEGA, 1);
 cc.device_num = 0;
 nvidia_gpus.push_back(cc);
}
#endif
        retval = (*__cuDeviceGet)(&device, j);
        if (retval) {
            sprintf(buf, "cuDeviceGet(%d) returned %d", j, retval);
            warnings.push_back(buf);
            return;
        }
        cc.prop.deviceHandle = device;
        (*__cuDeviceGetName)(cc.prop.name, 256, device);
        if (retval) {
            sprintf(buf, "cuDeviceGetName(%d) returned %d", j, retval);
            warnings.push_back(buf);
            return;
        }
        (*__cuDeviceComputeCapability)(&cc.prop.major, &cc.prop.minor, device);
        (*__cuDeviceTotalMem)(&cc.prop.totalGlobalMem, device);
        (*__cuDeviceGetAttribute)(&cc.prop.sharedMemPerBlock, CU_DEVICE_ATTRIBUTE_SHARED_MEMORY_PER_BLOCK, device);
        (*__cuDeviceGetAttribute)(&cc.prop.regsPerBlock, CU_DEVICE_ATTRIBUTE_REGISTERS_PER_BLOCK, device);
        (*__cuDeviceGetAttribute)(&cc.prop.warpSize, CU_DEVICE_ATTRIBUTE_WARP_SIZE, device);
        (*__cuDeviceGetAttribute)(&cc.prop.memPitch, CU_DEVICE_ATTRIBUTE_MAX_PITCH, device);
        retval = (*__cuDeviceGetAttribute)(&cc.prop.maxThreadsPerBlock, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, device);
        retval = (*__cuDeviceGetAttribute)(&cc.prop.maxThreadsDim[0], CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X, device);
        (*__cuDeviceGetAttribute)(&cc.prop.maxThreadsDim[1], CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y, device);
        (*__cuDeviceGetAttribute)(&cc.prop.maxThreadsDim[2], CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z, device);
        (*__cuDeviceGetAttribute)(&cc.prop.maxGridSize[0], CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X, device);
        (*__cuDeviceGetAttribute)(&cc.prop.maxGridSize[1], CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y, device);
        (*__cuDeviceGetAttribute)(&cc.prop.maxGridSize[2], CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z, device);
        (*__cuDeviceGetAttribute)(&cc.prop.clockRate, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, device);
        (*__cuDeviceGetAttribute)(&cc.prop.totalConstMem, CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY, device);
        (*__cuDeviceGetAttribute)(&cc.prop.textureAlignment, CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT, device);
        (*__cuDeviceGetAttribute)(&cc.prop.deviceOverlap, CU_DEVICE_ATTRIBUTE_GPU_OVERLAP, device);
        retval = (*__cuDeviceGetAttribute)(&cc.prop.multiProcessorCount, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, device);
        //retval = (*__cuDeviceGetProperties)(&cc.prop, device);
        if (cc.prop.major <= 0) continue;  // major == 0 means emulation
        if (cc.prop.major > 100) continue;  // e.g. 9999 is an error
#if defined(_WIN32) && !defined(SIM)
        cc.display_driver_version = Version.drvVersion;
#elif defined(__APPLE__)
        cc.display_driver_version = NSVersionOfRunTimeLibrary("cuda");
#else
        cc.display_driver_version = 0;
#endif
        cc.have_cuda = true;
        cc.cuda_version = cuda_version;
        cc.device_num = j;
        cc.set_peak_flops();
        cc.get_available_ram();
#if FAKENVIDIACUDA0
cc.device_num = j+1;
#endif
        nvidia_gpus.push_back(cc);
    }
    if (!nvidia_gpus.size()) {
        warnings.push_back("No CUDA-capable NVIDIA GPUs found");
        return;
    }

    // identify the most capable non-ignored instance
    //
    bool first = true;
    for (i=0; i<nvidia_gpus.size(); i++) {
        if (in_vector(nvidia_gpus[i].device_num, ignore_devs)) continue;
        if (first) {
            *this = nvidia_gpus[i];
            first = false;
        } else if (nvidia_compare(nvidia_gpus[i], *this, false) > 0) {
            *this = nvidia_gpus[i];
        }
    }

    // see which other instances are equivalent,
    // and set the "count" and "device_nums" fields
    //
    count = 0;
    for (i=0; i<nvidia_gpus.size(); i++) {
        if (in_vector(nvidia_gpus[i].device_num, ignore_devs)) {
            nvidia_gpus[i].is_used = COPROC_IGNORED;
        } else if (use_all || !nvidia_compare(nvidia_gpus[i], *this, true)) {
            device_nums[count] = nvidia_gpus[i].device_num;
            count++;
            nvidia_gpus[i].is_used = COPROC_USED;
        } else {
            nvidia_gpus[i].is_used = COPROC_UNUSED;
        }
    }
}

// fake a NVIDIA GPU (for debugging)
//
void COPROC_NVIDIA::fake(
    int driver_version, double ram, double avail_ram, int n
) {
   strcpy(type, GPU_TYPE_NVIDIA);
   count = n;
   for (int i=0; i<count; i++) {
       device_nums[i] = i;
   }
   available_ram = avail_ram;
   display_driver_version = driver_version;
   cuda_version = 2020;
   strcpy(prop.name, "Fake NVIDIA GPU");
   prop.totalGlobalMem = (unsigned int)ram;
   prop.sharedMemPerBlock = 100;
   prop.regsPerBlock = 8;
   prop.warpSize = 10;
   prop.memPitch = 10;
   prop.maxThreadsPerBlock = 20;
   prop.maxThreadsDim[0] = 2;
   prop.maxThreadsDim[1] = 2;
   prop.maxThreadsDim[2] = 2;
   prop.maxGridSize[0] = 10;
   prop.maxGridSize[1] = 10;
   prop.maxGridSize[2] = 10;
   prop.totalConstMem = 10;
   prop.major = 1;
   prop.minor = 2;
#if FAKENVIDIACUDA0
   prop.minor = 0;
#endif
   prop.clockRate = 1250000;
   prop.textureAlignment = 1000;
   prop.multiProcessorCount = 14;
   set_peak_flops();
}

// See how much RAM is available on this GPU.
//
void COPROC_NVIDIA::get_available_ram() {
    int retval;
    unsigned int memfree, memtotal;
	int device;
    void* ctx;
    
    available_ram = prop.dtotalGlobalMem;
    retval = (*__cuDeviceGet)(&device, device_num);
    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] cuDeviceGet(%d) returned %d", device_num, retval
            );
        }
        return;
    }
    retval = (*__cuCtxCreate)(&ctx, 0, device);
    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] cuCtxCreate(%d) returned %d", device_num, retval
            );
        }
        return;
    }
    retval = (*__cuMemGetInfo)(&memfree, &memtotal);
    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] cuMemGetInfo(%d) returned %d", device_num, retval
            );
        }
        (*__cuCtxDestroy)(ctx);
        return;
    }
    (*__cuCtxDestroy)(ctx);
    available_ram = (double) memfree;
}

// check whether each GPU is running a graphics app (assume yes)
// return true if there's been a change since last time
//
bool COPROC_NVIDIA::check_running_graphics_app() {
    int retval, j;
    bool change = false;
    for (j=0; j<count; j++) {
        bool new_val = true;
        int device, kernel_timeout;
        retval = (*__cuDeviceGet)(&device, j);
        if (!retval) {
            retval = (*__cuDeviceGetAttribute)(&kernel_timeout, CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT, device);
            if (!retval && !kernel_timeout) {
                new_val = false;
            }
        }
        if (new_val != running_graphics_app[j]) {
            change = true;
        }
        running_graphics_app[j] = new_val;
    }
    return change;
}


////////////////// ATI STARTS HERE /////////////////
//
// Docs:
// http://developer.amd.com/gpu/ATIStreamSDK/assets/ATI_Stream_SDK_CAL_Programming_Guide_v2.0%5B1%5D.pdf
// ?? why don't they have HTML docs??

// criteria:
//
// - double precision support
// - local RAM
// - speed
//
int ati_compare(COPROC_ATI& c1, COPROC_ATI& c2, bool loose) {
    if (c1.attribs.doublePrecision && !c2.attribs.doublePrecision) return 1;
    if (!c1.attribs.doublePrecision && c2.attribs.doublePrecision) return -1;
    if (loose) {
        if (c1.attribs.localRAM> 1.4*c2.attribs.localRAM) return 1;
        if (c1.attribs.localRAM< .7* c2.attribs.localRAM) return -1;
        return 0;
    }
    if (c1.attribs.localRAM > c2.attribs.localRAM) return 1;
    if (c1.attribs.localRAM < c2.attribs.localRAM) return -1;
    double s1 = c1.peak_flops;
    double s2 = c2.peak_flops;
    if (s1 > s2) return 1;
    if (s1 < s2) return -1;
    return 0;
}

#ifdef _WIN32
typedef int (__stdcall *ATI_ATTRIBS) (CALdeviceattribs *attribs, CALuint ordinal);
typedef int (__stdcall *ATI_CLOSE)(void);
typedef int (__stdcall *ATI_GDC)(CALuint *numDevices);
typedef int (__stdcall *ATI_GDI)(void);
typedef int (__stdcall *ATI_INFO) (CALdeviceinfo *info, CALuint ordinal);
typedef int (__stdcall *ATI_VER) (CALuint *cal_major, CALuint *cal_minor, CALuint *cal_imp);
typedef int (__stdcall *ATI_STATUS) (CALdevicestatus*, CALdevice);
typedef int (__stdcall *ATI_DEVICEOPEN) (CALdevice*, CALuint);
typedef int (__stdcall *ATI_DEVICECLOSE) (CALdevice);

ATI_ATTRIBS __calDeviceGetAttribs = NULL;
ATI_CLOSE   __calShutdown = NULL;
ATI_GDC     __calDeviceGetCount = NULL;
ATI_GDI     __calInit = NULL;
ATI_INFO    __calDeviceGetInfo = NULL;
ATI_VER     __calGetVersion = NULL;
ATI_STATUS  __calDeviceGetStatus = NULL;
ATI_DEVICEOPEN  __calDeviceOpen = NULL;
ATI_DEVICECLOSE  __calDeviceClose = NULL;

#else

int (*__calInit)();
int (*__calGetVersion)(CALuint*, CALuint*, CALuint*);
int (*__calDeviceGetCount)(CALuint*);
int (*__calDeviceGetAttribs)(CALdeviceattribs*, CALuint);
int (*__calShutdown)();
int (*__calDeviceGetInfo)(CALdeviceinfo*, CALuint);
int (*__calDeviceGetStatus)(CALdevicestatus*, CALdevice);
int (*__calDeviceOpen)(CALdevice*, CALuint);
int (*__calDeviceClose)(CALdevice);

#endif

void COPROC_ATI::get(
    bool use_all,
    vector<string>& warnings,
	vector<int>& ignore_devs
) {
    CALuint numDevices, cal_major, cal_minor, cal_imp;
    char buf[256];
    int retval;

    attribs.struct_size = sizeof(CALdeviceattribs);
    numDevices =0;

#ifdef _WIN32

#if defined _M_X64
    const char* atilib_name = "aticalrt64.dll";
    const char* amdlib_name = "amdcalrt64.dll";
#else
    const char* atilib_name = "aticalrt.dll";
    const char* amdlib_name = "amdcalrt.dll";
#endif

    HINSTANCE callib = LoadLibrary(atilib_name);
    if (callib) {
        atirt_detected = true;
    } else {
        callib = LoadLibrary(amdlib_name);
        if (callib) {
            amdrt_detected = true;
        }
    }

    if (!callib) {
        warnings.push_back("No ATI library found.");
        return;
    }

    __calInit = (ATI_GDI)GetProcAddress(callib, "calInit" );
    __calGetVersion = (ATI_VER)GetProcAddress(callib, "calGetVersion" );
    __calDeviceGetCount = (ATI_GDC)GetProcAddress(callib, "calDeviceGetCount" );
    __calDeviceGetAttribs =(ATI_ATTRIBS)GetProcAddress(callib, "calDeviceGetAttribs" );
    __calShutdown = (ATI_CLOSE)GetProcAddress(callib, "calShutdown" );
    __calDeviceGetInfo = (ATI_INFO)GetProcAddress(callib, "calDeviceGetInfo" );
    __calDeviceGetStatus = (ATI_STATUS)GetProcAddress(callib, "calDeviceGetStatus" );
    __calDeviceOpen = (ATI_DEVICEOPEN)GetProcAddress(callib, "calDeviceOpen" );
    __calDeviceClose = (ATI_DEVICECLOSE)GetProcAddress(callib, "calDeviceClose" );

#else

    void* callib;

    callib = dlopen("libaticalrt.so", RTLD_NOW);
    if (!callib) {
        warnings.push_back("No ATI library found");
        return;
    }

    atirt_detected = true;

    __calInit = (int(*)()) dlsym(callib, "calInit");
    __calGetVersion = (int(*)(CALuint*, CALuint*, CALuint*)) dlsym(callib, "calGetVersion");
    __calDeviceGetCount = (int(*)(CALuint*)) dlsym(callib, "calDeviceGetCount");
    __calDeviceGetAttribs = (int(*)(CALdeviceattribs*, CALuint)) dlsym(callib, "calDeviceGetAttribs");
    __calShutdown = (int(*)()) dlsym(callib, "calShutdown");
    __calDeviceGetInfo = (int(*)(CALdeviceinfo*, CALuint)) dlsym(callib, "calDeviceGetInfo");
    __calDeviceGetStatus = (int(*)(CALdevicestatus*, CALdevice)) dlsym(callib, "calDeviceGetStatus");
    __calDeviceOpen = (int(*)(CALdevice*, CALuint)) dlsym(callib, "calDeviceOpen");
    __calDeviceClose = (int(*)(CALdevice)) dlsym(callib, "calDeviceClose");

#endif

    if (!__calInit) {
        warnings.push_back("calInit() missing from CAL library");
        return;
    }
    if (!__calGetVersion) {
        warnings.push_back("calGetVersion() missing from CAL library");
        return;
    }
    if (!__calDeviceGetCount) {
        warnings.push_back("calDeviceGetCount() missing from CAL library");
        return;
    }
    if (!__calDeviceGetAttribs) {
        warnings.push_back("calDeviceGetAttribs() missing from CAL library");
        return;
    }
    if (!__calDeviceGetInfo) {
        warnings.push_back("calDeviceGetInfo() missing from CAL library");
        return;
    }
    if (!__calDeviceGetStatus) {
        warnings.push_back("calDeviceGetStatus() missing from CAL library");
        return;
    }
    if (!__calDeviceOpen) {
        warnings.push_back("calDeviceOpen() missing from CAL library");
        return;
    }
    if (!__calDeviceClose) {
        warnings.push_back("calDeviceClose() missing from CAL library");
        return;
    }

    retval = (*__calInit)();
    if (retval != CAL_RESULT_OK) {
        sprintf(buf, "calInit() returned %d", retval);
        warnings.push_back(buf);
        return;
    }
    retval = (*__calDeviceGetCount)(&numDevices);
    if (retval != CAL_RESULT_OK) {
        sprintf(buf, "calDeviceGetCount() returned %d", retval);
        warnings.push_back(buf);
        return;
    }
    retval = (*__calGetVersion)(&cal_major, &cal_minor, &cal_imp);
    if (retval != CAL_RESULT_OK) {
        sprintf(buf, "calGetVersion() returned %d", retval);
        warnings.push_back(buf);
        return;
    }

    if (!numDevices) {
        warnings.push_back("No usable CAL devices found");
        return;
    }

    COPROC_ATI cc, cc2;
    string s, gpu_name;
    for (CALuint i=0; i<numDevices; i++) {
        retval = (*__calDeviceGetInfo)(&info, i);
        if (retval != CAL_RESULT_OK) {
            sprintf(buf, "calDeviceGetInfo() returned %d", retval);
            warnings.push_back(buf);
            return;
        }
        retval = (*__calDeviceGetAttribs)(&attribs, i);
        if (retval != CAL_RESULT_OK) {
            sprintf(buf, "calDeviceGetAttribs() returned %d", retval);
            warnings.push_back(buf);
            return;
        }
        switch ((int)attribs.target) {
        case CAL_TARGET_600:
            gpu_name="ATI Radeon HD 2900 (RV600)";
            break;
        case CAL_TARGET_610:
            gpu_name="ATI Radeon HD 2300/2400/3200 (RV610)";
            attribs.numberOfSIMD=1;        // set correct values (reported wrong by driver)
            attribs.wavefrontSize=32;
            break;
        case CAL_TARGET_630:
            gpu_name="ATI Radeon HD 2600 (RV630)";
            // set correct values (reported wrong by driver)
            attribs.numberOfSIMD=3;
            attribs.wavefrontSize=32;
            break;
        case CAL_TARGET_670:
            gpu_name="ATI Radeon HD 3800 (RV670)";
            break;
        case CAL_TARGET_710:
            gpu_name="ATI Radeon HD 4350/4550 (R710)";
            break;
        case CAL_TARGET_730:
            gpu_name="ATI Radeon HD 4600 series (R730)";
            break;
        case CAL_TARGET_7XX:
            gpu_name="ATI Radeon (RV700 class)";
            break;
        case CAL_TARGET_770:
            gpu_name="ATI Radeon HD 4700/4800 (RV740/RV770)";
            break;
        case 8:
            gpu_name="ATI Radeon HD 5800 series (Cypress)";
            break;
        case 9:
            gpu_name="ATI Radeon HD 5700 series (Juniper)";
            break;
        case 10:
            gpu_name="ATI Radeon HD 5x00 series (Redwood)";
            break;
        case 11:
            gpu_name="ATI Radeon HD 5x00 series (Cedar)";
            break;
//
// looks like we mixed the CAL TargetID because all other tools identify CAL_TARGETID 13 as Sumo (not SuperSumo) so
// we have to fix this and some other strings here too
// CAL_TARGETID 12 is still unknown .. maybe this is SuperSumo inside AMDs upcoming Trinity
// 
// 
        case 12:
            gpu_name="AMD Radeon HD (unknown)";
            break;
        case 13:
            gpu_name="AMD Radeon HD 6x00 series (Sumo)";
            break;
// AMD released some more Wrestler so we have at the moment : 6250/6290/6310/6320/7310/7340 (based on Catalyst 12.2 preview)
        case 14:
            gpu_name="AMD Radeon HD 6200/6300/7300 series (Wrestler)";
            break;
        case 15:
            gpu_name="AMD Radeon HD 6900 series (Cayman)";
            break;
// the last unknown ... AMD Radeon HD (unknown) looks better !
        case 16:
            gpu_name="AMD Radeon HD (unknown)";
            break;
        case 17:
            gpu_name="AMD Radeon HD 6800 series (Barts)";
            break;
        case 18:
            gpu_name="AMD Radeon HD 6x00 series (Turks)";
            break;
        case 19:
            gpu_name="AMD Radeon HD 6300 series (Caicos)";
            break;
        case 20:
            gpu_name = "AMD Radeon HD 79x0 series (Tahiti)";
            break;
// there arent any other target ids inside the Shadercompiler (YET !!! )
// but because of ATI was bought by AMD and is not existing anymore the default should be changed too
        default:
            gpu_name="AMD Radeon HD (unknown)";
            break;
        }
        cc.have_cal = true;
        cc.attribs = attribs;
        cc.info = info;
        strcpy(cc.name, gpu_name.c_str());
        sprintf(cc.version, "%d.%d.%d", cal_major, cal_minor, cal_imp);
        cc.amdrt_detected = amdrt_detected;
        cc.atirt_detected = atirt_detected;
        cc.device_num = i;
        cc.set_peak_flops();
        cc.get_available_ram();
        ati_gpus.push_back(cc);
    }

    // shut down CAL, otherwise Lenovo won't be able to switch to low-power GPU
    //
    retval = (*__calShutdown)();

    if (!ati_gpus.size()) {
        warnings.push_back("No ATI GPUs found");
        return;
    }

    // find the most capable non-ignored instance
    //
    bool first = true;
    unsigned int i;
    for (i=0; i<ati_gpus.size(); i++) {
        if (in_vector(ati_gpus[i].device_num, ignore_devs)) continue;
        if (first) {
            *this = ati_gpus[i];
            first = false;
        } else if (ati_compare(ati_gpus[i], *this, false) > 0) {
            *this = ati_gpus[i];
        }
    }

    // see which other instances are equivalent,
    // and set the "count" and "device_nums" fields
    //
    count = 0;
    for (i=0; i<ati_gpus.size(); i++) {
        ati_gpus[i].description(buf);
        if (in_vector(ati_gpus[i].device_num, ignore_devs)) {
            ati_gpus[i].is_used = COPROC_IGNORED;
        } else if (use_all || !ati_compare(ati_gpus[i], *this, true)) {
            device_nums[count] = ati_gpus[i].device_num;
            count++;
            ati_gpus[i].is_used = COPROC_USED;
        } else {
            ati_gpus[i].is_used = COPROC_UNUSED;
        }
    }
}

void COPROC_ATI::fake(double ram, double avail_ram, int n) {
    strcpy(type, GPU_TYPE_ATI);
    strcpy(version, "1.4.3");
    strcpy(name, "foobar");
    count = n;
    available_ram = avail_ram;
    memset(&attribs, 0, sizeof(attribs));
    memset(&info, 0, sizeof(info));
    attribs.localRAM = (int)(ram/MEGA);
    attribs.numberOfSIMD = 32;
    attribs.wavefrontSize = 32;
    attribs.engineClock = 50;
    for (int i=0; i<count; i++) {
        device_nums[i] = i;
    }
    set_peak_flops();
}

// get available RAM of ATI GPU
//
void COPROC_ATI::get_available_ram() {
    CALdevicestatus st;
    CALdevice dev;
    int retval;

    available_ram = attribs.localRAM*MEGA;

    st.struct_size = sizeof(CALdevicestatus);

    retval = (*__calDeviceOpen)(&dev, device_num);
    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] calDeviceOpen(%d) returned %d", device_num, retval
            );
        }
        return;
    }
    retval = (*__calDeviceGetStatus)(&st, dev);
    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] calDeviceGetStatus(%d) returned %d",
                device_num, retval
            );
        }
        (*__calDeviceClose)(dev);
        return;
    }
    available_ram = st.availLocalRAM*MEGA;
    (*__calDeviceClose)(dev);
}

#ifdef __APPLE__
// OpenCL returns incorrect total RAM size for some 
// ATI GPUs so we get that info from OpenGL on Macs

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <Carbon/Carbon.h>

void COPROCS::get_ati_mem_size_from_opengl() {
    CGLRendererInfoObj info;
    long i, j;
    GLint numRenderers = 0, rv = 0, deviceVRAM, rendererID;
    CGLError theErr2 = kCGLNoError;
    CGLContextObj curr_ctx = CGLGetCurrentContext (); // save current CGL context
    int ati_gpu_index = 0;
    GLint rendererIDs[32];
    CFDataRef modelName[32];
    char opencl_name[1024], iokit_name[1024];
    char *p;

    if (log_flags.coproc_debug) {

        for (i=0; i<32; ++i) {
            rendererIDs[i] = 0;
            modelName[i] = NULL;
            
            CGOpenGLDisplayMask myMask = 1 << i;
            CGDirectDisplayID displayID = CGOpenGLDisplayMaskToDisplayID(myMask);
            theErr2 = CGLQueryRendererInfo (myMask, 
                                      &info, 
                                      &numRenderers);

            if ((displayID != kCGNullDirectDisplay) && (theErr2 == kCGLNoError)) {
                // Get the I/O Kit service port for the display
                io_registry_entry_t dspPort = CGDisplayIOServicePort(displayID);
                for (j = 0; j < numRenderers; j++) {
                  // find accelerated renderer (assume only one)
                    CGLDescribeRenderer (info, j, kCGLRPAcceleratedCompute, &rv); 
                    if (true == rv) { // if openCL-capable
                        // what is the renderer ID
                        CGLDescribeRenderer (info, j, kCGLRPRendererID, &rendererIDs[i]);
                        modelName[i] = (CFDataRef)IORegistryEntrySearchCFProperty(dspPort,
                            kIOServicePlane, CFSTR("model"), kCFAllocatorDefault,
                            kIORegistryIterateRecursively | kIORegistryIterateParents);
                    }
                    if (modelName[i] != NULL) break;
                }
            }
        }
    }   // End if (log_flags.coproc_debug) {

    theErr2 = CGLQueryRendererInfo (0xffffffff, 
                                  &info, 
                                  &numRenderers);
    if (theErr2 == kCGLNoError) {
        CGLDescribeRenderer (info, 0, kCGLRPRendererCount, &numRenderers);
        for (i = 0; i < numRenderers; i++) {
            if (ati_gpu_index >= (int)ati_opencls.size()) {
                break;
            }
            
            CGLDescribeRenderer (info, i, kCGLRPAcceleratedCompute, &rv); 
            if (true == rv) { // if openCL-capable
                // what is the renderer ID
                CGLDescribeRenderer (info, i, kCGLRPRendererID,
                                     &rendererID);
               // what is the VRAM?
                CGLDescribeRenderer (info, i, kCGLRPVideoMemory,
                                     &deviceVRAM);

                // build context and context specific info
                CGLPixelFormatAttribute attribs[] = { kCGLPFARendererID,
                                                    (CGLPixelFormatAttribute)rendererID, 
                                                    kCGLPFAAllowOfflineRenderers,
                                                    (CGLPixelFormatAttribute)0 };
                CGLPixelFormatObj pixelFormat = NULL;
                GLint numPixelFormats = 0;
                CGLContextObj cglContext;

                CGLChoosePixelFormat (attribs, &pixelFormat, &numPixelFormats);
                if (pixelFormat) {
                    CGLCreateContext(pixelFormat, NULL, &cglContext);
                    CGLDestroyPixelFormat (pixelFormat);
                    CGLSetCurrentContext (cglContext);
                    if (cglContext) {
                        // get vendor string from renderer
                        const GLubyte * strVend = glGetString (GL_VENDOR);
                        if (strVend &&
                            ((strstr((char *)strVend, GPU_TYPE_ATI)) || 
                            (strstr((char *)strVend, "AMD")) ||  
                            (strstr((char *)strVend, "Advanced Micro Devices, Inc.")))
                        ) {
                            ati_opencls[ati_gpu_index].global_mem_size = deviceVRAM;

                            if (log_flags.coproc_debug) {
                                // For some GPUs, one API returns "ATI" but the other API returns
                                // "AMD" in the model name, so we normalize both to "AMD"
                                strlcpy(opencl_name, ati_opencls[ati_gpu_index].name, sizeof(opencl_name));
                                if ((p = strstr(opencl_name, "ATI")) != NULL) {
                                    *++p='M';
                                    *++p='D';
                                }
                                
                                for (j=0; j<32; j++) {
                                    if ((rendererID == rendererIDs[j]) && (modelName[j] != NULL)) {
                                        break;
                                    }
                                }
                                if (j < 32) {
                                    strlcpy(iokit_name, (char *)CFDataGetBytePtr(modelName[j]), sizeof(iokit_name));
                                    if ((p = strstr(iokit_name, "ATI")) != NULL) {
                                        *++p='M';
                                        *++p='D';
                                    }
                                    if (strcmp(iokit_name, opencl_name)) {
                                        msg_printf(0, MSG_INFO,
                                        "[coproc] get_ati_mem_size_from_opengl model name mismatch: %s vs %s\n", 
                                        ati_opencls[ati_gpu_index].name, (char *)CFDataGetBytePtr(modelName[j])
                                        );
                                    }
                                } else {
                                    // Could not get model name from IOKit, so use renderer name
                                    const GLubyte * strRend = glGetString (GL_RENDERER);
                                    if (strRend != NULL) {
                                        strlcpy(iokit_name, (char *)strRend, sizeof(iokit_name));
                                        if ((p = strstr(iokit_name, "ATI")) != NULL) {
                                            *++p='M';
                                            *++p='D';
                                        }
                                    }
                                    
                                    if ((strRend == NULL) || 
                                        (!strstr(iokit_name, opencl_name))) {
                                        msg_printf(0, MSG_INFO,
                                        "[coproc] get_ati_mem_size_from_opengl model name to renderer mismatch: %s vs %s\n", 
                                        strRend, ati_opencls[ati_gpu_index].name
                                        );
                                    }
                                }
                            }   // End if (log_flags.coproc_debug) {

                            ati_gpu_index++;
                        } // End if ATI / AMD GPU
                        
                        CGLDestroyContext (cglContext);
                    } else {
                        if (log_flags.coproc_debug) {
                            msg_printf(0, MSG_INFO,
                            "[coproc] get_ati_mem_size_from_opengl failed to create context\n"
                            );
                        }
                    }
                } else {
                        if (log_flags.coproc_debug) {
                            msg_printf(0, MSG_INFO,
                            "[coproc] get_ati_mem_size_from_opengl failed to create PixelFormat\n"
                            );
                        }
                }
            }       // End if kCGLRPAcceleratedCompute attribute
        }   // End loop: for (i = 0; i < numRenderers; i++)
        CGLDestroyRendererInfo (info);
    }
    
    if (log_flags.coproc_debug) {
        for (j=0; j<32; j++) {
            if (modelName[j] != NULL) {
                CFRelease(modelName[j]);
            }
        }
    }    
    CGLSetCurrentContext (curr_ctx); // restore current CGL context
}
#endif

