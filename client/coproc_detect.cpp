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

//#define MEASURE_AVAILABLE_RAM

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

void COPROC::print_available_ram() {
#ifdef MEASURE_AVAILABLE_RAM
    if (gstate.now - last_print_time < 60) return;
    last_print_time = gstate.now;

    for (int i=0; i<count; i++) {
        if (available_ram_unknown[i]) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] %s device %d: available RAM unknown",
                    type, device_nums[i]
                );
            }
        } else {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] %s device %d: available RAM %d MB",
                    type, device_nums[i],
                    (int)(available_ram[i]/MEGA)
                );
            }
        }
    }
#endif
}


//TODO: Determine how we want to compare OpenCL devices - this is only a placeholder
// return 1/-1/0 if device 1 is more/less/same capable than device 2.
// factors (decreasing priority):
// - global memory
// - local memory
// - number of cores
// - speed
//
// If "loose", tolerate small memory diff
//
int opencl_compare(OPENCL_DEVICE_PROP& c1, OPENCL_DEVICE_PROP& c2, bool loose) {
    if (loose) {
        if (c1.global_RAM > 1.4*c2.global_RAM) return 1;
        if (c1.global_RAM < .7* c2.global_RAM) return -1;
        return 0;
    }
    if (c1.global_RAM > c2.global_RAM) return 1;
    if (c1.global_RAM < c2.global_RAM) return -1;

    if (loose) {
        if (c1.local_RAM > 1.4*c2.local_RAM) return 1;
        if (c1.local_RAM < .7* c2.local_RAM) return -1;
        return 0;
    }
    if (c1.local_RAM > c2.local_RAM) return 1;
    if (c1.local_RAM < c2.local_RAM) return -1;

    if (c1.max_cores > c2.max_cores) return 1;
    if (c1.max_cores < c2.max_cores) return -1;

    if (c1.max_clock_freq > c2.max_clock_freq) return 1;
    if (c1.max_clock_freq < c2.max_clock_freq) return -1;

    return 0;
}

void COPROCS::get_opencl(bool use_all, vector<string>&warnings) {
    cl_int ciErrNum;
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices, device_index;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    OPENCL_DEVICE_PROP prop;
    vector<OPENCL_DEVICE_PROP> nvidia_opencls;
    vector<OPENCL_DEVICE_PROP> ati_opencls;
    unsigned int i;

#ifdef _WIN32
    opencl_lib = LoadLibrary("OpenCL.dll");
    if (!opencl_lib) {
        warnings.push_back("No OpenCL library found");
        return;
    }

    __clGetPlatformIDs = (CL_PLATFORMIDS)GetProcAddress( opencl_lib, "clGetPlatformIDs" );
    __clGetPlatformInfo = (CL_PLATFORMINFO)( opencl_lib, "clGetPlatformInfo" );
    __clGetDeviceIDs = (CL_DEVICEIDS)GetProcAddress( opencl_lib, "clGetDeviceIDs" );
    __clGetDeviceInfo = (CL_INFO)GetProcAddress( opencl_lib, "clGetDeviceInfo" );
#else
#ifdef __APPLE__
    opencl_lib = dlopen("/System/Library/Frameworks/OpenCL.framework/Versions/Current/OpenCL", RTLD_NOW);
#else
// TODO: Is this correct?
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
        ciErrNum = (*__clGetDeviceIDs)(
            platforms[platform_index], CL_DEVICE_TYPE_GPU, MAX_COPROC_INSTANCES, devices, &num_devices
        );
        if (ciErrNum != CL_SUCCESS) {
            char buf[256];
            sprintf(buf, "clGetDeviceIDs for platform #%d returned error %d", platform_index, ciErrNum);
            warnings.push_back(buf);
            return;
        }

        for (device_index=0; device_index<num_devices; ++device_index) {
            memset(&prop, 0, sizeof(prop));
            prop.device_id = devices[device_index];
            
//TODO: Should we store the platform(s) for each GPU found?
//TODO: Must we check if multiple platforms found the same GPU and merge the records?

            ciErrNum = get_opencl_info(prop, device_index, warnings);
            if (ciErrNum != CL_SUCCESS) break;
            
            if (strstr(prop.vendor, "NVIDIA")) {
                 nvidia_opencls.push_back(prop);
            }
            if ((strstr(prop.vendor, "ATI")) || 
                (strstr(prop.vendor, "AMD")) ||  
                (strstr(prop.vendor, "Advanced Micro Devices, Inc."))
            ) {
                 ati_opencls.push_back(prop);
            }
        }
    }

    if ((nvidia_opencls.size() == 0) && (ati_opencls.size() == 0)) {
        warnings.push_back("OpenCL library present but no OpenCL-capable GPUs found");
        return;
    }

    if (nvidia.have_cuda) { // If CUDA already found the "best" NVIDIA GPU
        for (i=0; i<nvidia_opencls.size(); i++) {
            if (nvidia.matches(nvidia_opencls[i])) {
                // TODO: how do we exclude those listed by config.ignore_nvidia_dev ??
                nvidia.opencl_prop = nvidia_opencls[i];
                nvidia.opencl_device_ids[nvidia.opencl_device_count++] = nvidia_opencls[i].device_id;
                nvidia.have_opencl = true;
            }
        }
    } else {
        // identify the most capable NVIDIA OpenCL GPU
        //
        bool first = true;
        for (i=0; i<nvidia_opencls.size(); i++) {
//TODO: Temporary code for testing
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc_debug] COPROC_NVIDIA [no CUDA]: nvidia_opencls[%d].name = '%s'; nvidia_opencls[%d].device_id = %p",
                    i, nvidia_opencls[i].name, i, nvidia_opencls[i].device_id);
                msg_printf(0, MSG_INFO,
                    "[coproc_debug] COPROC_NVIDIA [no CUDA]: nvidia_opencls[%d].global_RAM = %lu; nvidia_opencls[%d].local_RAM = %lu",
                    i, nvidia_opencls[i].global_RAM, i, nvidia_opencls[i].local_RAM);
            }
//          if (in_vector(nvidia_opencls[i].device_num, ignore_devs)) continue;
            bool is_best = false;
            if (first) {
                is_best = true;
                first = false;
            } else if (opencl_compare(nvidia_opencls[i], nvidia.opencl_prop, false) > 0) {
                is_best = true;
            }
            if (is_best) {
                nvidia.opencl_prop = nvidia_opencls[i];     // fill in what info we have
                strcpy(nvidia.prop.name, nvidia_opencls[i].name);
                nvidia.prop.totalGlobalMem = nvidia_opencls[i].global_RAM;
                nvidia.prop.clockRate = nvidia_opencls[i].max_clock_freq * 1000;
                nvidia.have_opencl = true;
            }
        }

        // see which other instances are equivalent,
        // and set the "opencl_device_count" and "opencl_device_ids" fields
        //
        nvidia.opencl_device_count = 0;
        for (i=0; i<nvidia_opencls.size(); i++) {
#if 0
// TODO: should we implement config.ignore_opencl_dev?
            char buf2[256];
//TODO: What should description() be for OpenCL?
            nvidia_opencls[i].description(buf);
            if (in_vector(nvidia_opencls[i].device_num, ignore_devs)) {
                sprintf(buf2, "NVIDIA GPU %d (ignored by config): %s", nvidia_opencls[i].device_num, buf);
            } else 
#endif
            if (use_all || !opencl_compare(nvidia_opencls[i], nvidia.opencl_prop, true)) {
                nvidia.opencl_device_ids[nvidia.opencl_device_count++] = nvidia_opencls[i].device_id;
//                sprintf(buf2, "NVIDIA GPU %d: %s", nvidia_opencls[i].device_num, buf);
//            } else {
//                sprintf(buf2, "NVIDIA GPU %d (not used): %s", nvidia_opencls[i].device_num, buf);
            }
//            descs.push_back(string(buf2));
        }
    }           // End if (! nvidia.have_cuda)

    if (ati.have_cal) { // If CAL already found the "best" CAL GPU
        for (i=0; i<ati_opencls.size(); i++) {
            if (ati.matches(ati_opencls[i])) {
                // TODO: how do we exclude those listed by config.ignore_ati_dev ??
                ati.opencl_prop = ati_opencls[i];
                ati.opencl_device_ids[ati.opencl_device_count++] = ati_opencls[i].device_id;
                ati.have_opencl = true;
            }
        }
    } else {
        // identify the most capable ATI OpenCL GPU
        //
        bool first = true;
        for (i=0; i<ati_opencls.size(); i++) {
//TODO: Temporary code for testing
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc_debug] COPROC_ATI [no CAL]: ati_opencls[%d].name = '%s'; ati_opencls[%d].device_id = %p",
                    i, ati_opencls[i].name, i, ati_opencls[i].device_id);
                msg_printf(0, MSG_INFO,
                    "[coproc_debug] COPROC_ATI [no CAL]: ati_opencls[%d].global_RAM = %lu; ati_opencls[%d].local_RAM = %lu",
                    i, ati_opencls[i].global_RAM, i, ati_opencls[i].local_RAM);
            }
//          if (in_vector(ati_opencls[i].device_num, ignore_devs)) continue;
            bool is_best = false;
            if (first) {
                is_best = true;
                first = false;
            } else if (opencl_compare(ati_opencls[i], ati.opencl_prop, false) > 0) {
                is_best = true;
            }
            if (is_best) {
                ati.opencl_prop = ati_opencls[i];     // fill in what info we have
                strcpy(ati.name, ati_opencls[i].name);
                ati.attribs.localRAM = ati_opencls[i].local_RAM;
                ati.attribs.engineClock = ati_opencls[i].max_clock_freq;
                ati.have_opencl = true;
            }
        }

        // see which other instances are equivalent,
        // and set the "opencl_device_count" and "opencl_device_ids" fields
        //
        ati.opencl_device_count = 0;
        for (i=0; i<ati_opencls.size(); i++) {
#if 0
// TODO: should we implement config.ignore_opencl_dev?
            char buf2[256];
            ati_opencls[i].description(buf);
            if (in_vector(ati_opencls[i].device_num, ignore_devs)) {
                sprintf(buf2, "ATI GPU %d (ignored by config): %s", ati_opencls[i].device_num, buf);
            } else 
#endif
            if (use_all || !opencl_compare(ati_opencls[i], ati.opencl_prop, true)) {
                ati.opencl_device_ids[ati.opencl_device_count++] = ati_opencls[i].device_id;
//                sprintf(buf2, "ATI GPU %d: %s", ati_opencls[i].device_num, buf);
//            } else {
//                sprintf(buf2, "ATI GPU %d (not used): %s", ati_opencls[i].device_num, buf);
            }
//            descs.push_back(string(buf2));
        }
    }           // End if (! ati.have_cuda)
//TODO: Add code to add for other GPU vendors
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

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_HALF_FP_CONFIG, sizeof(prop.hp_fp_config), &prop.hp_fp_config, NULL);
    if (ciErrNum != CL_SUCCESS) {
        if ((ciErrNum == CL_INVALID_VALUE) || (ciErrNum == CL_INVALID_OPERATION)) {
            prop.hp_fp_config = 0;  // Not supported by OpenCL 1.0
        } else {
            sprintf(buf, "clGetDeviceInfo failed to get half-precision floating point capabilities for GPU %d", (int)device_index);
            warnings.push_back(buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(prop.sp_fp_config), &prop.sp_fp_config, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get single-precision floating point capabilities for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(prop.dp_fp_config), &prop.dp_fp_config, NULL);
    if (ciErrNum != CL_SUCCESS) {
        if ((ciErrNum == CL_INVALID_VALUE) || (ciErrNum == CL_INVALID_OPERATION)) {
            prop.dp_fp_config = 0;  // Not supported by OpenCL 1.0
        } else {
            sprintf(buf, "clGetDeviceInfo failed to get double-precision floating point capabilities for GPU %d", (int)device_index);
            warnings.push_back(buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_ENDIAN_LITTLE, sizeof(prop.little_endian), &prop.little_endian, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get little or big endian for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(prop.exec_capab), &prop.exec_capab, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get execution capabilities for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_EXTENSIONS, sizeof(prop.extensions), prop.extensions, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get device extensions for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(prop.global_RAM), &prop.global_RAM, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get global RAM size for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(prop.local_RAM), &prop.local_RAM, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get local RAM size for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(prop.max_clock_freq), &prop.max_clock_freq, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get max number of cores for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(prop.max_cores), &prop.max_cores, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get local RAM size for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_VERSION, sizeof(prop.openCL_device_version), prop.openCL_device_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get OpenCL version supported by GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DRIVER_VERSION, sizeof(prop.openCL_driver_version), prop.openCL_driver_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        sprintf(buf, "clGetDeviceInfo failed to get OpenCL driver version for GPU %d", (int)device_index);
        warnings.push_back(buf);
        return ciErrNum;
    }

    return CL_SUCCESS;
}

void COPROCS::get(
    bool use_all, vector<string>&descs, vector<string>&warnings,
    vector<int>& ignore_nvidia_dev,
    vector<int>& ignore_ati_dev
) {

#ifdef _WIN32
    try {
        nvidia.get(use_all, descs, warnings, ignore_nvidia_dev);
    }
    catch (...) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    }
    try {
        ati.get(use_all, descs, warnings, ignore_ati_dev);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    }
    try {
        get_opencl(use_all, warnings);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    }
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        nvidia.get(use_all, descs, warnings, ignore_nvidia_dev);
    }
#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    } else {
        ati.get(use_all, descs, warnings, ignore_ati_dev);
    }
#endif
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    } else {
        get_opencl(use_all, warnings);
    }
    signal(SIGSEGV, old_sig);
#endif
}


////////////////// NVIDIA STARTS HERE /////////////////
//

// return 1/-1/0 if device 1 is more/less/same capable than device 2.
// factors (decreasing priority):
// - compute capability
// - software version
// - memory
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
        if (c1.prop.totalGlobalMem > 1.4*c2.prop.totalGlobalMem) return 1;
        if (c1.prop.totalGlobalMem < .7* c2.prop.totalGlobalMem) return -1;
        return 0;
    }
    if (c1.prop.totalGlobalMem > c2.prop.totalGlobalMem) return 1;
    if (c1.prop.totalGlobalMem < c2.prop.totalGlobalMem) return -1;
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
typedef int (__stdcall *CUDA_CC)(unsigned int*, unsigned int, unsigned int);
typedef int (__stdcall *CUDA_CD)(unsigned int);
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
int (*__cuCtxCreate)(unsigned int*, unsigned int, unsigned int);
int (*__cuCtxDestroy)(unsigned int);
int (*__cuMemAlloc)(unsigned int*, unsigned int);
int (*__cuMemFree)(unsigned int);
int (*__cuMemGetInfo)(unsigned int*, unsigned int*);
#endif

// NVIDIA interfaces are documented here:
// http://developer.download.nvidia.com/compute/cuda/2_3/toolkit/docs/online/index.html

void COPROC_NVIDIA::get(
    bool use_all,    // if false, use only those equivalent to most capable
    vector<string>& descs,
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
    NvDisplayHandle hDisplay;
    NV_DISPLAY_DRIVER_VERSION Version;
    memset(&Version, 0, sizeof(Version));
    Version.version = NV_DISPLAY_DRIVER_VERSION_VER;

    NvAPI_Initialize();
    for (int i=0; ; i++) {
        nvapiStatus = NvAPI_EnumNvidiaDisplayHandle(i, &hDisplay);
        if (nvapiStatus != NVAPI_OK) break;
        nvapiStatus = NvAPI_GetDisplayDriverVersion(hDisplay, &Version);
        if (nvapiStatus == NVAPI_OK) break;
    }
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
    __cuCtxCreate = (int(*)(unsigned int*, unsigned int, unsigned int)) dlsym( cudalib, "cuCtxCreate" );
    __cuCtxDestroy = (int(*)(unsigned int)) dlsym( cudalib, "cuCtxDestroy" );
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

    vector<COPROC_NVIDIA> gpus;
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
#else
        cc.display_driver_version = 0;
#endif
        cc.have_cuda = true;
        cc.cuda_version = cuda_version;
        cc.device_num = j;
		cc.set_peak_flops();
        gpus.push_back(cc);
    }

    if (!gpus.size()) {
        warnings.push_back("No CUDA-capable NVIDIA GPUs found");
        return;
    }

    // identify the most capable non-ignored instance
    //
    bool first = true;
    for (i=0; i<gpus.size(); i++) {
        if (in_vector(gpus[i].device_num, ignore_devs)) continue;
        if (first) {
            *this = gpus[i];
            first = false;
        } else if (nvidia_compare(gpus[i], *this, false) > 0) {
            *this = gpus[i];
        }
    }

    // see which other instances are equivalent,
    // and set the "count" and "device_nums" fields
    //
    count = 0;
    for (i=0; i<gpus.size(); i++) {
        char buf2[256];
        gpus[i].description(buf);
        if (in_vector(gpus[i].device_num, ignore_devs)) {
            sprintf(buf2, "NVIDIA GPU %d (ignored by config): %s", gpus[i].device_num, buf);
        } else if (use_all || !nvidia_compare(gpus[i], *this, true)) {
            device_nums[count] = gpus[i].device_num;
            count++;
            sprintf(buf2, "NVIDIA GPU %d: %s", gpus[i].device_num, buf);
        } else {
            sprintf(buf2, "NVIDIA GPU %d (not used): %s", gpus[i].device_num, buf);
        }
        descs.push_back(string(buf2));
    }
}

// fake a NVIDIA GPU (for debugging)
//
void COPROC_NVIDIA::fake(int driver_version, double ram, int n) {
   strcpy(type, "NVIDIA");
   count = n;
   for (int i=0; i<count; i++) {
       device_nums[i] = i;
   }
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
   prop.clockRate = 1250000;
   prop.textureAlignment = 1000;
   prop.multiProcessorCount = 14;
   set_peak_flops();
}

// See how much RAM is available on each GPU.
// If this fails, set "available_ram_unknown"
//
void COPROC_NVIDIA::get_available_ram() {
#ifdef MEASURE_AVAILABLE_RAM
    int device, i, retval;
    unsigned int memfree, memtotal;
    unsigned int ctx;
    
    // avoid crash if faked GPU
    //
    if (!__cuDeviceGet) {
        for (i=0; i<count; i++) {
            available_ram[i] = available_ram_fake[i];
            available_ram_unknown[i] = false;
        }
        return;
    }
    for (i=0; i<count; i++) {
        int devnum = device_nums[i];
        available_ram[i] = 0;
        available_ram_unknown[i] = true;
        retval = (*__cuDeviceGet)(&device, devnum);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] cuDeviceGet(%d) returned %d", devnum, retval
                );
            }
            continue;
        }
        retval = (*__cuCtxCreate)(&ctx, 0, device);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] cuCtxCreate(%d) returned %d", devnum, retval
                );
            }
            continue;
        }
        retval = (*__cuMemGetInfo)(&memfree, &memtotal);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] cuMemGetInfo(%d) returned %d", devnum, retval
                );
            }
            (*__cuCtxDestroy)(ctx);
            continue;
        }
        (*__cuCtxDestroy)(ctx);
        available_ram[i] = (double) memfree;
        available_ram_unknown[i] = false;
    }
#else
    for (int i=0; i<count; i++) {
        available_ram_unknown[i] = false;
        available_ram[i] = prop.totalGlobalMem;
    }
#endif
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

bool COPROC_NVIDIA::matches(OPENCL_DEVICE_PROP& OpenCLprop) {
    bool retval = true;

//TODO: Temporary code for testing
    if (log_flags.coproc_debug) {
        msg_printf(0, MSG_INFO,
            "[coproc_debug] COPROC_NVIDIA [in matches()]: prop.name = '%s'; OpenCLprop.name = '%s'",
            prop.name, OpenCLprop.name);
        msg_printf(0, MSG_INFO,
            "[coproc_debug] COPROC_NVIDIA [in matches()]: device_num = %d, prop.deviceHandle = %d; OpenCLprop.device_id = %p",
            device_num, prop.deviceHandle, OpenCLprop.device_id);
        msg_printf(0, MSG_INFO,
            "[coproc_debug] COPROC_NVIDIA [in matches()]: prop.totalGlobalMem = %u; OpenCLprop.global_RAM = %lu; OpenCLprop.local_RAM = %lu",
            prop.totalGlobalMem, OpenCLprop.global_RAM, OpenCLprop.local_RAM);
    }

#if 0//def _WIN32
//TODO: Verify this test is correct
    if (prop.deviceHandle != OpenCLprop.device_id) return false;
#else
    if (strcmp(prop.name, OpenCLprop.name)) retval = false;
//TODO: Figure out why these don't match
//TODO: Should there be "loose" comparisons here?
//    if (prop.totalGlobalMem != OpenCLprop.global_RAM) return false;
//    if ((prop.clockRate / 1000) != (int)OpenCLprop.max_clock_freq) retval = false;
#endif

//TODO: Temporary code for testing
    if (log_flags.coproc_debug) {
        if (retval) {
            msg_printf(0, MSG_INFO, "[coproc_debug] COPROC_NVIDIA: Match Found!");
        } else {
            msg_printf(0, MSG_INFO, "[coproc_debug] COPROC_NVIDIA: Match NOT Found!");
        }
    }
    return retval;
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
    vector<string>& descs, vector<string>& warnings, vector<int>& ignore_devs
) {
    CALuint numDevices, cal_major, cal_minor, cal_imp;
    CALdevice device;
    char buf[256];
    int retval;

    attribs.struct_size = sizeof(CALdeviceattribs);
    device = 0;
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
    vector<COPROC_ATI> gpus;
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
// based on AMD's Stream SDK 2.3 shipped with AMD Catalyst 10.12 APP
//
// and by comments of Dr. Andreas Przystawik aka Gipsel at http://www.planet3dnow.de/vbulletin/showthread.php?p=4335830#post4335830
//
//
// added new/current/coming AMD RADEON GPUs/IGPs/APUs
        case 12:
            gpu_name="AMD SUMO";
            break;
        case 13:
            gpu_name="AMD SUPERSUMO";
            break;
        case 14:
            gpu_name="AMD Radeon HD 6250/6310 (Wrestler)";
            break;
        case 15:
            gpu_name="AMD Radeon HD 6900 series (Cayman)";
            break;
        case 16:
            gpu_name="AMD RESERVED2";
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
        // there arent any other target ids inside the Shadercompiler (YET !!! )
        default:
            gpu_name="ATI unknown";
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
        gpus.push_back(cc);
    }

    // shut down, otherwise Lenovo won't be able to switch to low-power GPU
    //
    retval = (*__calShutdown)();

    if (!gpus.size()) {
        warnings.push_back("No ATI GPUs found");
        return;
    }

    bool first = true;
    unsigned int i;
    for (i=0; i<gpus.size(); i++) {
        if (in_vector(gpus[i].device_num, ignore_devs)) continue;
        if (first) {
            *this = gpus[i];
            first = false;
        } else if (ati_compare(gpus[i], *this, false) > 0) {
            *this = gpus[i];
        }
    }

    count = 0;
    for (i=0; i<gpus.size(); i++) {
        char buf2[256];
        gpus[i].description(buf);
        if (in_vector(gpus[i].device_num, ignore_devs)) {
            sprintf(buf2, "ATI GPU %d (ignored by config): %s", gpus[i].device_num, buf);
        } else if (use_all || !ati_compare(gpus[i], *this, true)) {
            device_nums[count] = gpus[i].device_num;
            count++;
            sprintf(buf2, "ATI GPU %d: %s", gpus[i].device_num, buf);
        } else {
            sprintf(buf2, "ATI GPU %d: (not used) %s", gpus[i].device_num, buf);
        }
        descs.push_back(string(buf2));
    }
}

void COPROC_ATI::fake(double ram, int n) {
    strcpy(type, "ATI");
    strcpy(version, "1.4.3");
    strcpy(name, "foobar");
    count = n;
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

void COPROC_ATI::get_available_ram() {
#ifdef MEASURE_AVAILABLE_RAM
    CALdevicestatus st;
    CALdevice dev;
    int i, retval;

    st.struct_size = sizeof(CALdevicestatus);

    // avoid crash if faked GPU
    if (!__calInit) {
        for (i=0; i<count; i++) {
            available_ram[i] = available_ram_fake[i];
            available_ram_unknown[i] = false;
        }
        return;
    }
    for (i=0; i<count; i++) {
        available_ram[i] = 0;
        available_ram_unknown[i] = true;
    }
    retval = (*__calInit)();
    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] calInit() returned %d", retval
            );
        }
        return;
    }

    for (i=0; i<count; i++) {
        int devnum = device_nums[i];
        retval = (*__calDeviceOpen)(&dev, devnum);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] calDeviceOpen(%d) returned %d", devnum, retval
                );
            }
            continue;
        }
        retval = (*__calDeviceGetStatus)(&st, dev);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] calDeviceGetStatus(%d) returned %d",
                    devnum, retval
                );
            }
            (*__calDeviceClose)(dev);
            continue;
        }
        available_ram[i] = st.availLocalRAM*MEGA;
        available_ram_unknown[i] = false;
        (*__calDeviceClose)(dev);
    }
    (*__calShutdown)();
#else
    for (int i=0; i<count; i++) {
        available_ram_unknown[i] = false;
        available_ram[i] = attribs.localRAM*MEGA;
    }
#endif
}

bool COPROC_ATI::matches(OPENCL_DEVICE_PROP& OpenCLprop) {
    bool retval = true;

//TODO: Temporary code for testing
    if (log_flags.coproc_debug) {
        msg_printf(0, MSG_INFO,
            "[coproc_debug] COPROC_ATI [in matches()]: prop.name = '%s'; OpenCLprop.name = '%s'",
            name, OpenCLprop.name);
        msg_printf(0, MSG_INFO,
            "[coproc_debug] COPROC_ATI [in matches()]: device_num = %d; OpenCLprop.device_id = %p",
            device_num, OpenCLprop.device_id);
        msg_printf(0, MSG_INFO,
            "[coproc_debug] COPROC_ATI [in matches()]: attribs.localRAM = %u; OpenCLprop.global_RAM = %lu; OpenCLprop.local_RAM = %lu",
            attribs.localRAM, OpenCLprop.global_RAM, OpenCLprop.local_RAM);
    }

#if 0//def _WIN32
//TODO: Verify this test is correct
    if (prop.deviceHandle != OpenCLprop.device_id) return false;
#else
    if (strcmp(name, OpenCLprop.name)) retval = false;
//TODO: Figure out why these don't match
//TODO: Should there be "loose" comparisons here?
//    if (attribs.localRAM != OpenCLprop.local_RAM) retval = false;
//    if (attribs.engineClock != OpenCLprop.max_clock_freq) retval = false;
#endif

//TODO: Temporary code for testing
    if (log_flags.coproc_debug) {
        if (retval) {
            msg_printf(0, MSG_INFO, "[coproc_debug] COPROC_ATI: Match Found!");
        } else {
            msg_printf(0, MSG_INFO, "[coproc_debug] COPROC_ATI: Match NOT Found!");
        }
    }
    return retval;
}
