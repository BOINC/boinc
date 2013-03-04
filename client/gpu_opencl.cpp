// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// Detection of GPUs using OpenCL 

#ifdef _WIN32
#include "boinc_win.h"
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#endif
#include "config.h"
#include <dlfcn.h>
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;

#include "coproc.h"
#include "util.h"
#include "str_replace.h"

#include "client_msgs.h"
#include "gpu_detect.h"

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

cl_int (*__clGetPlatformIDs)(
    cl_uint,         // num_entries,
    cl_platform_id*, // platforms
    cl_uint *        // num_platforms
);
cl_int (*__clGetPlatformInfo)(
    cl_platform_id,  // platform
    cl_platform_info, // param_name
    size_t,          // param_value_size
    void*,           // param_value
    size_t*          // param_value_size_ret
);
cl_int (*__clGetDeviceIDs)(
    cl_platform_id,  // platform
    cl_device_type,  // device_type
    cl_uint,         // num_entries
    cl_device_id*,   // devices
    cl_uint*         // num_devices
);
cl_int (*__clGetDeviceInfo)(
    cl_device_id,    // device
    cl_device_info,  // param_name
    size_t,          // param_value_size
    void*,           // param_value
    size_t*          // param_value_size_ret
);

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
    vector<int>& ignore_nvidia_dev,
    vector<int>& ignore_intel_dev
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
            platforms[platform_index], CL_PLATFORM_VERSION,
            sizeof(platform_version), &platform_version, NULL
        );
        if (ciErrNum != CL_SUCCESS) {
            msg_printf(0, MSG_INFO,
                "Couldn't get PLATFORM_VERSION for platform #%d; error %d", platform_index, ciErrNum
            );
            continue;
        }

        ciErrNum = (*__clGetDeviceIDs)(
            platforms[platform_index], CL_DEVICE_TYPE_GPU,
            MAX_COPROC_INSTANCES, devices, &num_devices
        );
        if (ciErrNum != CL_SUCCESS) {
            msg_printf(0, MSG_INFO,
                "Couldn't get Device IDs for platform #%d: error %d", platform_index, ciErrNum
            );
            continue;
        }

        // Mac OpenCL does not recognize all NVIDIA GPUs returned by CUDA
        //
        current_CUDA_index = 0;

        for (device_index=0; device_index<num_devices; ++device_index) {
            memset(&prop, 0, sizeof(prop));
            prop.device_id = devices[device_index];
            strncpy(
                prop.opencl_platform_version, platform_version,
                sizeof(prop.opencl_platform_version)-1
            );
            
//TODO: Should we store the platform(s) for each GPU found?
//TODO: Must we check if multiple platforms found the same GPU and merge the records?
            ciErrNum = get_opencl_info(prop, device_index, warnings);
            if (ciErrNum != CL_SUCCESS) continue;
            
            prop.is_used = COPROC_UNUSED;
            prop.get_device_version_int();

            //////////// NVIDIA //////////////
            if (strstr(prop.vendor, GPU_TYPE_NVIDIA)) {
                if (nvidia.have_cuda) {
                    // Mac OpenCL does not recognize all NVIDIA GPUs returned by
                    // CUDA but we assume that OpenCL and CUDA return devices in
                    // the same order and with identical model name strings
                    //
                    while (1) {
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
                        // This CUDA GPU is not recognized by OpenCL,
                        // so try the next
                        //
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
                    // Assumes OpenCL and CUDA return the devices
                    // in the same order
                    //
                    prop.opencl_available_ram = nvidia_gpus[prop.device_num].available_ram;
                } else {
                    prop.opencl_available_ram = prop.global_mem_size;
                }
                nvidia_opencls.push_back(prop);
                ++current_CUDA_index;
            }
            
            //////////// AMD / ATI //////////////
            if ((strstr(prop.vendor, GPU_TYPE_ATI)) || 
                (strstr(prop.vendor, "AMD")) ||  
                (strstr(prop.vendor, "Advanced Micro Devices, Inc."))
            ) {
                prop.device_num = (int)(ati_opencls.size());
                prop.opencl_device_index = device_index;
                
                if (ati.have_cal) {
                    if (prop.device_num < (int)(ati_gpus.size())) {
                        // Always use GPU model name from CAL if available
                        // for ATI / AMD  GPUs because
                        // (we believe) it is more user-friendly.
                        // Assumes OpenCL and CAL return the devices
                        // in the same order
                        //
                        strcpy(prop.name, ati_gpus[prop.device_num].name);

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
            
            //////////// INTEL GPU //////////////
            if (strcasestr(prop.vendor, "intel")) {
                cl_device_type device_type;
                
                ciErrNum = (*__clGetDeviceInfo)(
                    prop.device_id, CL_DEVICE_TYPE,
                    sizeof(device_type), &device_type, NULL
                );
                if (ciErrNum != CL_SUCCESS) {
                    warnings.push_back("clGetDeviceInfo failed to get device type for Intel device");
                    continue;
                }
                if (device_type == CL_DEVICE_TYPE_CPU) continue;

                prop.device_num = (int)(intel_gpu_opencls.size());
                prop.opencl_device_index = device_index;
                
                COPROC_INTEL c;
                c.opencl_prop = prop;
                c.is_used = COPROC_UNUSED;
                c.available_ram = prop.global_mem_size;
                strcpy(c.name, prop.name);
                strcpy(c.version, prop.opencl_driver_version);

                c.set_peak_flops();
                prop.peak_flops = c.peak_flops;
                prop.opencl_available_ram = prop.global_mem_size;

                intel_gpu_opencls.push_back(prop);

                // At present Intel GPUs only support OpenCL and do not have a native
                // GPGPU framework, so treat each detected Intel OpenCL GPU device as 
                // a native device.
                intel_gpus.push_back(c);
            }
        }
    }


#ifdef __APPLE__
    // Work around a bug in OpenCL which returns only 
    // 1/2 of total global RAM size. 
    // This bug applies only to ATI GPUs, not to NVIDIA
    // This has already been fixed on latest Catalyst 
    // drivers, but Mac does not use Catalyst drivers.
    if (ati_opencls.size() > 0) {
        opencl_get_ati_mem_size_from_opengl();
    }
#endif

    if ((nvidia_opencls.size() == 0) &&
        (ati_opencls.size() == 0) &&
        (intel_gpu_opencls.size() == 0)
    ) {
        warnings.push_back(
            "OpenCL library present but no OpenCL-capable GPUs found"
        );
        return;
    }
        
    if (nvidia.have_cuda) { // If CUDA already found the "best" NVIDIA GPU
        nvidia.merge_opencl(nvidia_opencls, ignore_nvidia_dev);
    } else {
        nvidia.find_best_opencls(use_all, nvidia_opencls, ignore_nvidia_dev);
        nvidia.prop.totalGlobalMem = nvidia.opencl_prop.global_mem_size;
        nvidia.available_ram = nvidia.opencl_prop.global_mem_size;
        nvidia.prop.clockRate = nvidia.opencl_prop.max_clock_frequency * 1000;
        strcpy(nvidia.prop.name, nvidia.opencl_prop.name);
    }

    if (ati.have_cal) { // If CAL already found the "best" CAL GPU
        ati.merge_opencl(ati_opencls, ignore_ati_dev);
    } else {
        ati.find_best_opencls(use_all, ati_opencls, ignore_ati_dev);
        ati.attribs.localRAM = ati.opencl_prop.global_mem_size/MEGA;
        ati.available_ram = ati.opencl_prop.global_mem_size;
        ati.attribs.engineClock = ati.opencl_prop.max_clock_frequency;
        strcpy(ati.name, ati.opencl_prop.name);
    }

    intel_gpu.find_best_opencls(use_all, intel_gpu_opencls, ignore_intel_dev);
    intel_gpu.available_ram = intel_gpu.opencl_prop.global_mem_size;
    strcpy(intel_gpu.name, intel_gpu.opencl_prop.name);

// TODO: Add code to allow adding other GPU vendors
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
//
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
    //
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
//
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


#ifdef __APPLE__
// OpenCL returns incorrect total RAM size for some 
// ATI GPUs so we get that info from OpenGL on Macs

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <Carbon/Carbon.h>

void COPROCS::opencl_get_ati_mem_size_from_opengl() {
    CGLRendererInfoObj info;
    long i, j;
    GLint numRenderers = 0, rv = 0, deviceVRAM, rendererID;
    CGLError theErr2 = kCGLNoError;
    CGLContextObj curr_ctx = CGLGetCurrentContext (); // save current CGL context
    int ati_gpu_index = 0;
    GLint rendererIDs[32];
    CFDataRef modelName[32];
    char opencl_name[256], iokit_name[256];
    char *p;

    if (log_flags.coproc_debug) {

        for (i=0; i<32; ++i) {
            rendererIDs[i] = 0;
            modelName[i] = NULL;
            
            CGOpenGLDisplayMask myMask = 1 << i;
            CGDirectDisplayID displayID = CGOpenGLDisplayMaskToDisplayID(myMask);
            theErr2 = CGLQueryRendererInfo(myMask, &info, &numRenderers); 
            if ((displayID != kCGNullDirectDisplay) && (theErr2 == kCGLNoError)) {
                // Get the I/O Kit service port for the display
                io_registry_entry_t dspPort = CGDisplayIOServicePort(displayID);
                for (j = 0; j < numRenderers; j++) {
                    // find accelerated renderer (assume only one)
                    CGLDescribeRenderer (info, j, kCGLRPAcceleratedCompute, &rv); 
                    if (true == rv) { // if openCL-capable
                        // what is the renderer ID
                        CGLDescribeRenderer (info, j, kCGLRPRendererID, &rendererIDs[i]);
                        modelName[i] = (CFDataRef)IORegistryEntrySearchCFProperty(
                            dspPort,
                            kIOServicePlane, CFSTR("model"), kCFAllocatorDefault,
                            kIORegistryIterateRecursively | kIORegistryIterateParents
                        );
                    }
                    if (modelName[i] != NULL) break;
                }
            }
        }
    }   // End if (log_flags.coproc_debug) {

    theErr2 = CGLQueryRendererInfo( 0xffffffff, &info, &numRenderers);
    if (theErr2 == kCGLNoError) {
        CGLDescribeRenderer (info, 0, kCGLRPRendererCount, &numRenderers);
        for (i = 0; i < numRenderers; i++) {
            if (ati_gpu_index >= (int)ati_opencls.size()) {
                break;
            }
            
            CGLDescribeRenderer (info, i, kCGLRPAcceleratedCompute, &rv); 
            if (true == rv) { // if openCL-capable
                // what is the renderer ID
                CGLDescribeRenderer (info, i, kCGLRPRendererID, &rendererID);
               // what is the VRAM?
                CGLDescribeRenderer (info, i, kCGLRPVideoMemory, &deviceVRAM);

                // build context and context specific info
                CGLPixelFormatAttribute attribs[] = {
                    kCGLPFARendererID,
                    (CGLPixelFormatAttribute)rendererID, 
                    kCGLPFAAllowOfflineRenderers,
                    (CGLPixelFormatAttribute)0
                };
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
                            ati_opencls[ati_gpu_index].opencl_available_ram = deviceVRAM;

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
                                            "[coproc] opencl_get_ati_mem_size_from_opengl model name mismatch: %s vs %s\n",
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
                                            "[coproc] opencl_get_ati_mem_size_from_opengl model name to renderer mismatch: %s vs %s\n",
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
                                "[coproc] opencl_get_ati_mem_size_from_opengl failed to create context\n"
                            );
                        }
                    }
                } else {
                    if (log_flags.coproc_debug) {
                        msg_printf(0, MSG_INFO,
                            "[coproc] opencl_get_ati_mem_size_from_opengl failed to create PixelFormat\n"
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
