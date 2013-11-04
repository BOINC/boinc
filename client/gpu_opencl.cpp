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
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
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
#include "str_replace.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
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

static bool is_AMD(char *vendor) {
    if (strstr(vendor, "ATI")) return true;
    if (strstr(vendor, "AMD")) return true;
    if (strstr(vendor, "Advanced Micro Devices, Inc.")) return true;
    return false;
}

static bool is_NVIDIA(char* vendor) {
    if (strstr(vendor, "NVIDIA")) return true;
    return false;
}

static bool is_intel(char* vendor) {
    if (strcasestr(vendor, "intel")) return true;
    return false;
}

// If "loose", tolerate small diff
//
static int opencl_compare(OPENCL_DEVICE_PROP& c1, OPENCL_DEVICE_PROP& c2, bool loose) {
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

#ifdef __APPLE__
static bool compare_pci_slots(int NVIDIA_GPU_Index1, int NVIDIA_GPU_Index2) {
    if (NVIDIA_GPU_Index1 >= (int)nvidia_gpus.size()) return false;  // Should never happen
    if (NVIDIA_GPU_Index2 >= (int)nvidia_gpus.size()) return false;  // Should never happen
    return (
        nvidia_gpus[NVIDIA_GPU_Index1].pci_info.bus_id <
                nvidia_gpus[NVIDIA_GPU_Index2].pci_info.bus_id
    );
}
#endif


// OpenCL interfaces are documented here:
// http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/ and
// http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/

void COPROCS::get_opencl(
    vector<string>& warnings
) {
    cl_int ciErrNum;
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices, device_index;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    char platform_version[256];
    char platform_vendor[256];
    char buf[256];
    OPENCL_DEVICE_PROP prop;
    int current_CUDA_index;
    int current_CAL_index;
    int min_CAL_target;
    int num_CAL_devices = (int)ati_gpus.size();
    vector<int>devnums_pci_slot_sort;
    vector<OPENCL_DEVICE_PROP>::iterator it;

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

    if (nvidia_gpus.size()) {
        for (int i=0; i<(int)nvidia_gpus.size(); ++i) {
            devnums_pci_slot_sort.push_back(i);
        }
#ifdef __APPLE__
        std::stable_sort(
            devnums_pci_slot_sort.begin(),
            devnums_pci_slot_sort.end(),
            compare_pci_slots
        );
#endif
    }

    for (platform_index=0; platform_index<num_platforms; ++platform_index) {
        ciErrNum = (*__clGetPlatformInfo)(
            platforms[platform_index], CL_PLATFORM_VERSION,
            sizeof(platform_version), &platform_version, NULL
        );
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "Couldn't get PLATFORM_VERSION for platform #%d; error %d",
                platform_index, ciErrNum
            );
            warnings.push_back(buf);
            continue;
        }

        ciErrNum = (*__clGetPlatformInfo)(
            platforms[platform_index], CL_PLATFORM_VENDOR,
            sizeof(platform_vendor), &platform_vendor, NULL
        );
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "Couldn't get PLATFORM_VENDOR for platform #%d; error %d",
                platform_index, ciErrNum
            );
            warnings.push_back(buf);
        }

        //////////// CPU //////////////

        ciErrNum = (*__clGetDeviceIDs)(
            platforms[platform_index], (CL_DEVICE_TYPE_CPU),
            MAX_COPROC_INSTANCES, devices, &num_devices
        );

        if ((ciErrNum != CL_SUCCESS) && (num_devices != 0)) {
            num_devices = 0;                 // No devices
            if (ciErrNum != CL_DEVICE_NOT_FOUND) {
                snprintf(buf, sizeof(buf),
                    "Couldn't get CPU Device IDs for platform #%d: error %d",
                    platform_index, ciErrNum
                );
                warnings.push_back(buf);
            }
        }

        for (device_index=0; device_index<num_devices; ++device_index) {
            memset(&prop, 0, sizeof(prop));
            prop.device_id = devices[device_index];
            strncpy(
                prop.opencl_platform_version, platform_version,
                sizeof(prop.opencl_platform_version)-1
            );

            ciErrNum = get_opencl_info(prop, device_index, warnings);
            if (ciErrNum != CL_SUCCESS) continue;

            prop.is_used = COPROC_UNUSED;
            prop.get_device_version_int();

            OPENCL_CPU_PROP c;
            strlcpy(c.platform_vendor, platform_vendor, sizeof(c.platform_vendor));
            c.opencl_prop = prop;
            cpu_opencls.push_back(c);
        }

        //////////// GPUs //////////////
        
        ciErrNum = (*__clGetDeviceIDs)(
            platforms[platform_index], (CL_DEVICE_TYPE_GPU),
            MAX_COPROC_INSTANCES, devices, &num_devices
        );

        if (ciErrNum == CL_DEVICE_NOT_FOUND) continue;  // No devices
        if (num_devices == 0) continue;                 // No devices

        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "Couldn't get Device IDs for platform #%d: error %d",
                platform_index, ciErrNum
            );
            warnings.push_back(buf);
            continue;
        }

        // Mac OpenCL does not recognize all NVIDIA GPUs returned by CUDA
        // Fortunately, CUDA and OpenCL return the same GPU model name on
        // the Mac, so we can use this to match OpenCL devices with CUDA.
        //
        current_CUDA_index = 0;

        // ATI/AMD OpenCL does not always recognize all GPUs returned by CAL.
        // This is complicated for several reasons:
        // * CAL returns only an enum (CALtargetEnum) for the GPU's family,
        //   not specific model information.
        // * OpenCL return only the GPU family name
        // * Which GPUs support OpenCL varies with different versions of the
        //   AMD Catalyst drivers.
        //
        // To deal with this, we make some (probably imperfect) assumptions:
        // * AMD drivers eliminate OpenCL support for older GPU families first.
        // * Lower values of CALtargetEnum represent older GPU families.
        // * All ATI/AMD GPUs reported by OpenCL are also reported by CAL (on
        //   systems where CAL is available) though the converse may not be true.
        //
        current_CAL_index = 0;
        min_CAL_target = 0;
        if (is_AMD(platform_vendor) && (num_CAL_devices > 0)) {
            while (1) {
                int numToMatch = 0;
                for (int i=0; i<num_CAL_devices; ++i) {
                    if ((int)ati_gpus[i].attribs.target >= min_CAL_target) {
                        ++numToMatch;
                    }
                }
                if (numToMatch == (int)num_devices) break;
                if (numToMatch < (int)num_devices) {
                    warnings.push_back(
                        "Could not match ATI OpenCL and CAL GPUs: ignoring CAL."
                    );
                    // If we can't match ATI OpenCL and CAL GPUs, ignore CAL
                    // and keep OpenCL because AMD has deprecated CAL.
                    ati_gpus.clear();
                    ati.have_cal = false;
                    num_CAL_devices = 0;
                    break;
                }
                ++min_CAL_target;
            }
        }

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
            if (is_NVIDIA(prop.vendor)) {
                if (nvidia.have_cuda) {
                    // Mac OpenCL does not recognize all NVIDIA GPUs returned by
                    // CUDA but we assume that OpenCL and CUDA return devices 
                    // with identical model name strings and that OpenCL returns
                    // devices in order of acending PCI slot.
                    //
                    // On other systems, assume OpenCL and CUDA return devices 
                    // in the same order.
                    //
                    while (1) {
                        if (current_CUDA_index >= (int)(nvidia_gpus.size())) {
                            snprintf(buf, sizeof(buf),
                                "OpenCL NVIDIA index #%d does not match any CUDA device",
                                device_index
                            );
                            warnings.push_back(buf);
                            return; // Should never happen
                        }
                        if (!strcmp(prop.name,
                            nvidia_gpus[devnums_pci_slot_sort[current_CUDA_index]].prop.name)
                            ) {
                            break;  // We have a match
                        }
                        // This CUDA GPU is not recognized by OpenCL,
                        // so try the next
                        //
                        ++current_CUDA_index;
                    }
                    prop.device_num = devnums_pci_slot_sort[current_CUDA_index];
                } else {
                    prop.device_num = (int)(nvidia_opencls.size());
                }
                prop.opencl_device_index = device_index;

                if (nvidia.have_cuda) {
                    prop.peak_flops = nvidia_gpus[prop.device_num].peak_flops;
                } else {
                    COPROC_NVIDIA c;
                    c.opencl_prop = prop;
                    c.set_peak_flops();
                    prop.peak_flops = c.peak_flops;
                }
                if (nvidia_gpus.size()) {
                    // Assumes OpenCL device_num and CUDA device_num now match
                    //
                    prop.opencl_available_ram = nvidia_gpus[prop.device_num].available_ram;
                } else {
                    prop.opencl_available_ram = prop.global_mem_size;
                }
                
                // Build nvidia_opencls vector in device_num order
                for (it=nvidia_opencls.begin(); it<nvidia_opencls.end(); it++) {
                    if (it->device_num > prop.device_num) break;
                }
                nvidia_opencls.insert(it, prop);
                
                ++current_CUDA_index;
            }
            
            //////////// AMD / ATI //////////////
            if (is_AMD(prop.vendor)) {
                prop.opencl_device_index = device_index;

                if (ati.have_cal) {
                    // AMD OpenCL does not recognize all AMD GPUs returned by
                    // CAL but we assume that OpenCL and CAL return devices in
                    // the same order.  See additional comments earlier in
                    // this source file for more details.
                    //
                    while (1) {
                        if (current_CAL_index >= num_CAL_devices) {
                            snprintf(buf, sizeof(buf),
                                "OpenCL ATI device #%d does not match any CAL device",
                                device_index
                            );
                            warnings.push_back(buf);
                            return; // Should never happen
                        }
                        if ((int)ati_gpus[current_CAL_index].attribs.target >= min_CAL_target) {
                            break;  // We have a match
                        }
                        // This CAL GPU is not recognized by OpenCL,
                        // so try the next
                        //
                        ++current_CAL_index;
                    }
                    prop.device_num = current_CAL_index++;

                    // Always use GPU model name from CAL if
                    // available for ATI / AMD  GPUs because
                    // (we believe) it is more user-friendly.
                    //
                    safe_strcpy(prop.name, ati_gpus[prop.device_num].name);

                    // Work around a bug in OpenCL which returns only
                    // 1/2 of total global RAM size: use the value from CAL.
                    // This bug applies only to ATI GPUs, not to NVIDIA
                    // See also further workaround code for Macs.
                    //
                    prop.global_mem_size = ati_gpus[prop.device_num].attribs.localRAM * MEGA;
                    prop.peak_flops = ati_gpus[prop.device_num].peak_flops;
                } else {            // ! ati.have_cal
                    prop.device_num = (int)(ati_opencls.size());
                    COPROC_ATI c;
                    c.opencl_prop = prop;
                    c.set_peak_flops();
                    prop.peak_flops = c.peak_flops;
                }

                if (ati_gpus.size()) {
                    prop.opencl_available_ram = ati_gpus[prop.device_num].available_ram;
                } else {
                    prop.opencl_available_ram = prop.global_mem_size;
                }
                ati_opencls.push_back(prop);
            }

            //////////// INTEL GPU //////////////
            //
            if (is_intel(prop.vendor)) {
                prop.device_num = (int)(intel_gpu_opencls.size());
                prop.opencl_device_index = device_index;

                COPROC_INTEL c;
                c.opencl_prop = prop;
                c.is_used = COPROC_UNUSED;
                c.available_ram = prop.global_mem_size;
                safe_strcpy(c.name, prop.name);
                safe_strcpy(c.version, prop.opencl_driver_version);

                c.set_peak_flops();
                prop.peak_flops = c.peak_flops;
                prop.opencl_available_ram = prop.global_mem_size;

                intel_gpu_opencls.push_back(prop);

                // At present Intel GPUs only support OpenCL
                // and do not have a native GPGPU framework,
                // so treat each detected Intel OpenCL GPU device as
                // a native device.
                //
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
        opencl_get_ati_mem_size_from_opengl(warnings);
    }
#endif

    if ((nvidia_opencls.size() == 0) &&
        (ati_opencls.size() == 0) &&
        (intel_gpu_opencls.size() == 0)
    ) {
        warnings.push_back(
            "OpenCL library present but no OpenCL-capable GPUs found"
        );
    }
}

void COPROCS::correlate_opencl(
    bool use_all,
    IGNORE_GPU_INSTANCE& ignore_gpu_instance
) {
    if (nvidia_opencls.size() > 0) {
        if (nvidia.have_cuda) { // If CUDA already found the "best" NVIDIA GPU
            nvidia.merge_opencl(
                nvidia_opencls, ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU]
            );
        } else {
            nvidia.find_best_opencls(
                use_all, nvidia_opencls, ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU]
            );
            nvidia.prop.totalGlobalMem = nvidia.opencl_prop.global_mem_size;
            nvidia.available_ram = nvidia.opencl_prop.global_mem_size;
            nvidia.prop.clockRate = nvidia.opencl_prop.max_clock_frequency * 1000;
            safe_strcpy(nvidia.prop.name, nvidia.opencl_prop.name);
        }
    }
    
    if (ati_opencls.size() > 0) {
        if (ati.have_cal) { // If CAL already found the "best" CAL GPU
            ati.merge_opencl(ati_opencls, ignore_gpu_instance[PROC_TYPE_AMD_GPU]);
        } else {
            ati.find_best_opencls(use_all, ati_opencls, ignore_gpu_instance[PROC_TYPE_AMD_GPU]);
            ati.attribs.localRAM = ati.opencl_prop.global_mem_size/MEGA;
            ati.available_ram = ati.opencl_prop.global_mem_size;
            ati.attribs.engineClock = ati.opencl_prop.max_clock_frequency;
            safe_strcpy(ati.name, ati.opencl_prop.name);
        }
    }
    
    if (intel_gpu_opencls.size() > 0) {
        intel_gpu.find_best_opencls(use_all, intel_gpu_opencls, ignore_gpu_instance[PROC_TYPE_INTEL_GPU]);
        intel_gpu.available_ram = intel_gpu.opencl_prop.global_mem_size;
        safe_strcpy(intel_gpu.name, intel_gpu.opencl_prop.name);
    }
    
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
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get name for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_VENDOR, sizeof(prop.vendor), prop.vendor, NULL);
    if ((ciErrNum != CL_SUCCESS) || (prop.vendor[0] == 0)) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get vendor for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_VENDOR_ID, sizeof(prop.vendor_id), &prop.vendor_id, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get vendor ID for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_AVAILABLE, sizeof(prop.available), &prop.available, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get availability for device %d",
            (int)device_index
        );
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
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get half-precision floating point capabilities for device %d",
                (int)device_index
            );
            warnings.push_back(buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_SINGLE_FP_CONFIG,
        sizeof(prop.single_fp_config), &prop.single_fp_config, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get single-precision floating point capabilities for device %d",
            (int)device_index
        );
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
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get double-precision floating point capabilities for device %d",
                (int)device_index
            );
            warnings.push_back(buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_ENDIAN_LITTLE, sizeof(prop.endian_little),
        &prop.endian_little, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get little or big endian for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_EXECUTION_CAPABILITIES,
        sizeof(prop.execution_capabilities), &prop.execution_capabilities, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get execution capabilities for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_EXTENSIONS, sizeof(prop.extensions),
        prop.extensions, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get device extensions for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_GLOBAL_MEM_SIZE,
        sizeof(prop.global_mem_size), &prop.global_mem_size, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get global memory size for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_LOCAL_MEM_SIZE,
        sizeof(prop.local_mem_size), &prop.local_mem_size, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get local memory size for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY,
        sizeof(prop.max_clock_frequency), &prop.max_clock_frequency, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get max clock frequency for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_MAX_COMPUTE_UNITS,
        sizeof(prop.max_compute_units), &prop.max_compute_units, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get max compute units for device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DEVICE_VERSION, sizeof(prop.opencl_device_version), prop.opencl_device_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get OpenCL version supported by device %d",
            (int)device_index
        );
        warnings.push_back(buf);
        return ciErrNum;
    }

    ciErrNum = (*__clGetDeviceInfo)(prop.device_id, CL_DRIVER_VERSION, sizeof(prop.opencl_driver_version), prop.opencl_driver_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get OpenCL driver version for device %d",
            (int)device_index
        );
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

void COPROCS::opencl_get_ati_mem_size_from_opengl(vector<string>& warnings) {
    CGLRendererInfoObj info;
    long i, j;
    GLint numRenderers = 0, rv = 0, deviceVRAM, rendererID;
    CGLError theErr2 = kCGLNoError;
    CGLContextObj curr_ctx = CGLGetCurrentContext (); // save current CGL context
    int ati_gpu_index = 0;
    GLint rendererIDs[32];
    CFDataRef modelName[32];
    char opencl_name[256], iokit_name[256], buf[256];
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
                        if (is_AMD((char *)strVend)) {
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
                                        snprintf(buf, sizeof(buf),
                                            "opencl_get_ati_mem_size_from_opengl model name mismatch: %s vs %s\n",
                                            ati_opencls[ati_gpu_index].name, (char *)CFDataGetBytePtr(modelName[j])
                                        );
                                        warnings.push_back(buf);
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
                                            snprintf(buf, sizeof(buf),
                                            "opencl_get_ati_mem_size_from_opengl model name to renderer mismatch: %s vs %s\n",
                                            strRend, ati_opencls[ati_gpu_index].name
                                        );
                                        warnings.push_back(buf);
                                    }
                                }
                            }   // End if (log_flags.coproc_debug) {

                            ati_gpu_index++;
                        } // End if ATI / AMD GPU

                        CGLDestroyContext (cglContext);
                    } else {
                        warnings.push_back(
                            "opencl_get_ati_mem_size_from_opengl failed to create context\n"
                        );
                    }
                } else {
                    warnings.push_back(
                        "opencl_get_ati_mem_size_from_opengl failed to create PixelFormat\n"
                    );
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
