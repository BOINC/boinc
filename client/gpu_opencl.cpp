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

#define TEST_OTHER_COPROC_LOGIC 0

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

CL_PLATFORMIDS  p_clGetPlatformIDs = NULL;
CL_PLATFORMINFO p_clGetPlatformInfo = NULL;
CL_DEVICEIDS    p_clGetDeviceIDs = NULL;
CL_INFO         p_clGetDeviceInfo = NULL;

#else

void* opencl_lib = NULL;

cl_int (*p_clGetPlatformIDs)(
    cl_uint,         // num_entries,
    cl_platform_id*, // platforms
    cl_uint *        // num_platforms
);
cl_int (*p_clGetPlatformInfo)(
    cl_platform_id,  // platform
    cl_platform_info, // param_name
    size_t,          // param_value_size
    void*,           // param_value
    size_t*          // param_value_size_ret
);
cl_int (*p_clGetDeviceIDs)(
    cl_platform_id,  // platform
    cl_device_type,  // device_type
    cl_uint,         // num_entries
    cl_device_id*,   // devices
    cl_uint*         // num_devices
);
cl_int (*p_clGetDeviceInfo)(
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

#ifdef __APPLE__
static bool is_apple(char* vendor) {
    if (strcasestr(vendor, "apple")) return true;
    return false;
}
#endif

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


// Test OS version number on all versions of OS X without using deprecated Gestalt
// compareOSVersionTo(x, y) returns:
// -1 if the OS version we are running on is less than x.y
//  0 if the OS version we are running on is equal to x.y
// +1 if the OS version we are running on is lgreater than x.y
int compareOSVersionTo(int toMajor, int toMinor) {
    static SInt32 major = -1;
    static SInt32 minor = -1;

    if (major < 0) {
        char vers[100], *p1 = NULL;
        FILE *f;
        vers[0] = '\0';
        f = popen("sw_vers -productVersion", "r");
        if (f) {
            fscanf(f, "%s", vers);
            pclose(f);
        }
        if (vers[0] == '\0') {
            fprintf(stderr, "popen(\"sw_vers -productVersion\" failed\n");
            fflush(stderr);
            return 0;
        }
        // Extract the major system version number
        major = atoi(vers);
        // Extract the minor system version number
        p1 = strchr(vers, '.');
        minor = atoi(p1+1);
    }

    if (major < toMajor) return -1;
    if (major > toMajor) return 1;
    // if (major == toMajor) compare minor version numbers
    if (minor < toMinor) return -1;
    if (minor > toMinor) return 1;
    return 0;
}
#endif

#ifdef ANDROID
#include <android/dlext.h>
void* (*p_android_dlopen_ext)(const char*, int, const android_dlextinfo*);
struct android_namespace_t* (*p_android_create_namespace)(const char*, const char*, const char*, uint64_t, const char*, struct android_namespace_t*);
struct android_namespace_t* (*p_android_get_exported_namespace)(const char*);

struct android_namespace_t* get_android_namespace(vector<string>& warnings) {
    p_android_get_exported_namespace = (struct android_namespace_t*(*)(const char*)) dlsym(RTLD_DEFAULT, "android_get_exported_namespace");
    if (!p_android_get_exported_namespace) {
        gpu_warning(warnings, "No android_get_exported_namespace()");
    }
    if (!p_android_get_exported_namespace) {
        p_android_get_exported_namespace = (struct android_namespace_t*(*)(const char*)) dlsym(RTLD_DEFAULT, "__loader_android_get_exported_namespace");
        if (!p_android_get_exported_namespace) {
            gpu_warning(warnings, "No __loader_android_get_exported_namespace()");
        }
    }
    if (p_android_get_exported_namespace) {
        return (*p_android_get_exported_namespace)("vndk");
    }

    p_android_create_namespace = (struct android_namespace_t*(*)(const char*, const char*, const char*, uint64_t, const char*, struct android_namespace_t*)) dlsym(RTLD_DEFAULT, "android_create_namespace");
    if (!p_android_create_namespace) {
        gpu_warning(warnings, "No android_create_namespace()");
        return NULL;
    }
    string lib_path;
    if (sizeof(void*) == 8) {
        lib_path = "/system/lib64/";
    }
    else {
        lib_path = "/system/lib/";
    }
#define ANDROID_NAMESPACE_TYPE_ISOLATED 1
#define ANDROID_NAMESPACE_TYPE_SHARED 2
    return (*p_android_create_namespace)("trustme", lib_path.c_str(), lib_path.c_str(), ANDROID_NAMESPACE_TYPE_SHARED | ANDROID_NAMESPACE_TYPE_ISOLATED, "/system/:/data/:/vendor/", NULL);
}

void* android_dlopen(const char* filename, vector<string>& warnings) {
    char buf[256];
    gpu_warning(warnings, "Trying dlopen()");
    void* handle = dlopen(filename, RTLD_NOW);
    if (handle) {
        return handle;
    }

    p_android_dlopen_ext = (void*(*)(const char*, int, const android_dlextinfo*)) dlsym(RTLD_DEFAULT, "android_dlopen_ext");
    if (!p_android_dlopen_ext) {
        gpu_warning(warnings, "No android_dlopen_ext()");
        return NULL;
    }

    struct android_namespace_t* ns = get_android_namespace(warnings);
    if (!ns) {
        gpu_warning(warnings, "No namespace");
        return NULL;
    }

    const android_dlextinfo dlextinfo = {
        .flags = ANDROID_DLEXT_USE_NAMESPACE,
        .library_namespace = ns,
    };
    gpu_warning(warnings, "Trying android_dlopen_ext()");
    return (*p_android_dlopen_ext)(filename, RTLD_NOW, &dlextinfo);
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
    int max_other_coprocs = MAX_RSC-1;  // coprocs[0] is reserved for CPU
    string s;

    if (cc_config.no_opencl) {
        return;
    }

#ifdef _WIN32
    opencl_lib = LoadLibrary("OpenCL.dll");
    if (!opencl_lib) {
        gpu_warning(warnings, "No OpenCL library found");
        return;
    }

    p_clGetPlatformIDs = (CL_PLATFORMIDS)GetProcAddress( opencl_lib, "clGetPlatformIDs" );
    p_clGetPlatformInfo = (CL_PLATFORMINFO)GetProcAddress( opencl_lib, "clGetPlatformInfo" );
    p_clGetDeviceIDs = (CL_DEVICEIDS)GetProcAddress( opencl_lib, "clGetDeviceIDs" );
    p_clGetDeviceInfo = (CL_INFO)GetProcAddress( opencl_lib, "clGetDeviceInfo" );
#else
#ifdef __APPLE__
    opencl_lib = dlopen("/System/Library/Frameworks/OpenCL.framework/Versions/Current/OpenCL", RTLD_NOW);
#elif defined ANDROID
    opencl_lib = android_dlopen("libOpenCL.so", warnings);
#else
    opencl_lib = dlopen("libOpenCL.so", RTLD_NOW);
    if (!opencl_lib) {
        opencl_lib = dlopen("libOpenCL.so.1", RTLD_NOW);
    }
#endif
    if (!opencl_lib) {
        snprintf(buf, sizeof(buf), "OpenCL: %s", dlerror());
        gpu_warning(warnings, buf);
        return;
    }
    p_clGetPlatformIDs = (cl_int(*)(cl_uint, cl_platform_id*, cl_uint*)) dlsym( opencl_lib, "clGetPlatformIDs" );
    p_clGetPlatformInfo = (cl_int(*)(cl_platform_id, cl_platform_info, size_t, void*, size_t*)) dlsym( opencl_lib, "clGetPlatformInfo" );
    p_clGetDeviceIDs = (cl_int(*)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*)) dlsym( opencl_lib, "clGetDeviceIDs" );
    p_clGetDeviceInfo = (cl_int(*)(cl_device_id, cl_device_info, size_t, void*, size_t*)) dlsym( opencl_lib, "clGetDeviceInfo" );
#endif

    if (!p_clGetPlatformIDs) {
        gpu_warning(warnings, "clGetPlatformIDs() missing from OpenCL library");
        goto leave;
    }
    if (!p_clGetPlatformInfo) {
        gpu_warning(warnings, "clGetPlatformInfo() missing from OpenCL library");
        goto leave;
    }
    if (!p_clGetDeviceIDs) {
        gpu_warning(warnings, "clGetDeviceIDs() missing from OpenCL library");
        goto leave;
    }
    if (!p_clGetDeviceInfo) {
        gpu_warning(warnings, "clGetDeviceInfo() missing from OpenCL library");
        goto leave;
    }

    ciErrNum = (*p_clGetPlatformIDs)(MAX_OPENCL_PLATFORMS, platforms, &num_platforms);
    if ((ciErrNum != CL_SUCCESS) || (num_platforms == 0)) {
        gpu_warning(warnings, "clGetPlatformIDs() failed to return any OpenCL platforms");
        goto leave;
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
        ciErrNum = (*p_clGetPlatformInfo)(
            platforms[platform_index], CL_PLATFORM_VERSION,
            sizeof(platform_version), &platform_version, NULL
        );
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "Couldn't get PLATFORM_VERSION for platform #%u; error %d",
                platform_index, ciErrNum
            );
            gpu_warning(warnings, buf);
            continue;
        }

        ciErrNum = (*p_clGetPlatformInfo)(
            platforms[platform_index], CL_PLATFORM_VENDOR,
            sizeof(platform_vendor), &platform_vendor, NULL
        );
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "Couldn't get PLATFORM_VENDOR for platform #%u; error %d",
                platform_index, ciErrNum
            );
            gpu_warning(warnings, buf);
        }

        //////////// CPU //////////////

        ciErrNum = (*p_clGetDeviceIDs)(
            platforms[platform_index], (CL_DEVICE_TYPE_CPU),
            MAX_COPROC_INSTANCES, devices, &num_devices
        );

        if ((ciErrNum != CL_SUCCESS) && (num_devices != 0)) {
            num_devices = 0;                 // No devices
            if (ciErrNum != CL_DEVICE_NOT_FOUND) {
                snprintf(buf, sizeof(buf),
                    "Couldn't get CPU Device IDs for platform #%u: error %d",
                    platform_index, ciErrNum
                );
                gpu_warning(warnings, buf);
            }
        }

        for (device_index=0; device_index<num_devices; ++device_index) {
            prop.clear();
            prop.device_id = devices[device_index];
            strlcpy(
                prop.opencl_platform_version, platform_version,
                sizeof(prop.opencl_platform_version)
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

        //////////// GPUs and Accelerators //////////////

// Looks like implementation of Qualcomm has some problems with clGetDeviceIDs
// It returns CL_DEVICE_NOT_FOUND for CL_DEVICE_TYPE_GPU and CL_DEVICE_TYPE_ACCELERATOR combined
// But it returns CL_SUCCESS when asking separately for CL_DEVICE_TYPE_GPU or CL_DEVICE_TYPE_ACCELERATOR
// So we will ask for CL_DEVICE_TYPE_GPU and CL_DEVICE_TYPE_ACCELERATOR separately
#ifdef ANDROID
        cl_device_id android_gpu[MAX_COPROC_INSTANCES];
        cl_uint num_android_gpu = 0;
        ciErrNum = (*p_clGetDeviceIDs)(
            platforms[platform_index],
            (CL_DEVICE_TYPE_GPU),
            MAX_COPROC_INSTANCES, android_gpu, &num_android_gpu
        );
        if (ciErrNum == CL_SUCCESS && num_android_gpu > 0) {
            for (int i=0; i<num_android_gpu; ++i) {
                devices[i] = android_gpu[i];
            }
            num_devices = num_android_gpu;
        }

        cl_device_id android_acc[MAX_COPROC_INSTANCES];
        cl_uint num_android_acc = 0;
        ciErrNum = (*p_clGetDeviceIDs)(
            platforms[platform_index],
            (CL_DEVICE_TYPE_ACCELERATOR),
            MAX_COPROC_INSTANCES - num_devices, android_acc, &num_android_acc
        );
        if (ciErrNum == CL_SUCCESS && num_android_acc > 0) {
            for (int i=0; i<num_android_acc; ++i) {
                devices[num_devices+i] = android_acc[i];
            }
            num_devices += num_android_acc;
        }
#else
        ciErrNum = (*p_clGetDeviceIDs)(
            platforms[platform_index],
            (CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR),
            MAX_COPROC_INSTANCES, devices, &num_devices
        );

        if (ciErrNum == CL_DEVICE_NOT_FOUND) {
            gpu_warning(warnings, "No OpenCL GPUs or Accelerators found");
            continue;  // No devices
        }

        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "Couldn't get Device IDs for platform #%u: error %d",
                platform_index, ciErrNum
            );
            gpu_warning(warnings, buf);
            continue;
        }
#endif
        if (num_devices == 0) continue;                 // No devices

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
                    gpu_warning(warnings, "Could not match ATI OpenCL and CAL GPUs: ignoring CAL.");
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
            prop.clear();
            prop.device_id = devices[device_index];
            strlcpy(
                prop.opencl_platform_version, platform_version,
                sizeof(prop.opencl_platform_version)
            );

//TODO: Should we store the platform(s) for each GPU found?
//TODO: Must we check if multiple platforms found the same GPU and merge the records?
            ciErrNum = get_opencl_info(prop, device_index, warnings);
            if (ciErrNum != CL_SUCCESS) continue;

// TODO: Eliminate this, or improve it
#if TEST_OTHER_COPROC_LOGIC
            if (is_NVIDIA(prop.vendor)) {
                safe_strcpy(prop.vendor, "FAKE VENDOR X");
            } else if (is_AMD(prop.vendor)) {
                safe_strcpy(prop.vendor, "FAKE VENDOR Y");
            } else {
                safe_strcpy(prop.vendor, "FAKE VENDOR Z");
            }
#endif

            prop.is_used = COPROC_UNUSED;
            prop.get_device_version_int();

            if (is_NVIDIA(prop.vendor)) {
                //////////// NVIDIA //////////////
                bool cuda_match_found = false;
                if (nvidia.have_cuda) {
                    // Mac OpenCL does not recognize all NVIDIA GPUs returned by
                    // CUDA but we assume that OpenCL and CUDA return devices
                    // with identical model name strings and that OpenCL returns
                    // devices in order of ascending PCI slot.
                    //
                    // On other systems, assume OpenCL and CUDA return devices
                    // in the same order.
                    //
                    int saved_CUDA_index = current_CUDA_index;

                    while (1) {
                        if (current_CUDA_index >= (int)(nvidia_gpus.size())) {
                            snprintf(buf, sizeof(buf),
                                "OpenCL NVIDIA index #%u does not match any CUDA device",
                                device_index
                            );
                            gpu_warning(warnings, buf);
                            // Newer versions of CUDA driver don't support older NVIDIA GPUs
                            if (nvidia.cuda_version >= 6050) {
                                prop.device_num = (int)(nvidia_opencls.size());
                                current_CUDA_index = saved_CUDA_index;
                                prop.warn_bad_cuda = true;
                                break;
                            } else {
                                // Older CUDA drivers should report all NVIDIA GPUs reported by OpenCL
                                goto leave; // Should never happen
                            }
                        }
                        if (!strcmp(prop.name,
                            nvidia_gpus[devnums_pci_slot_sort[current_CUDA_index]].prop.name)
                            ) {
                            cuda_match_found = true;
                            prop.device_num = devnums_pci_slot_sort[current_CUDA_index];
                            break;  // We have a match
                        }
                        // This CUDA GPU is not recognized by OpenCL,
                        // so try the next
                        //
                        ++current_CUDA_index;
                    }
                } else {
                    prop.device_num = (int)(nvidia_opencls.size());
                }
                prop.opencl_device_index = device_index;

                if (cuda_match_found) {
                    prop.peak_flops = nvidia_gpus[prop.device_num].peak_flops;
                } else {
                    COPROC_NVIDIA c;
                    c.opencl_prop = prop;
                    c.set_peak_flops();
                    if (c.bad_gpu_peak_flops("NVIDIA OpenCL", s)) {
                        gpu_warning(warnings, s.c_str());
                    }
                    prop.peak_flops = c.peak_flops;
                }
                if (cuda_match_found) {
                    // Assumes OpenCL device_num and CUDA device_num now match
                    //
                    prop.opencl_available_ram = nvidia_gpus[prop.device_num].available_ram;
                } else {
                    prop.opencl_available_ram = prop.global_mem_size;
                }

                // Build nvidia_opencls vector in device_num order
                for (it=nvidia_opencls.begin(); it != nvidia_opencls.end(); ++it) {
                    if (it->device_num > prop.device_num) break;
                }
                nvidia_opencls.insert(it, prop);

                if (cuda_match_found) ++current_CUDA_index;
            } else if (is_AMD(prop.vendor)) {
                //////////// AMD / ATI //////////////
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
                                "OpenCL ATI device #%u does not match any CAL device",
                                device_index
                            );
                            gpu_warning(warnings, buf);
                            goto leave; // Should never happen
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
                    if (c.bad_gpu_peak_flops("AMD OpenCL", s)) {
                        gpu_warning(warnings, s.c_str());
                    }
                    prop.peak_flops = c.peak_flops;
                }

                if (ati_gpus.size()) {
                    prop.opencl_available_ram = ati_gpus[prop.device_num].available_ram;
                } else {
                    prop.opencl_available_ram = prop.global_mem_size;
                }
                ati_opencls.push_back(prop);
            } else if (is_intel(prop.vendor)) {
                //////////// INTEL GPU //////////////
                prop.device_num = (int)(intel_gpu_opencls.size());
                prop.opencl_device_index = device_index;

                COPROC_INTEL c;
                c.opencl_prop = prop;
                c.is_used = COPROC_UNUSED;
                c.available_ram = prop.global_mem_size;
                safe_strcpy(c.name, prop.name);
                safe_strcpy(c.version, prop.opencl_driver_version);

                c.set_peak_flops();
                if (c.bad_gpu_peak_flops("Intel OpenCL", s)) {
                    gpu_warning(warnings, s.c_str());
                }
                prop.peak_flops = c.peak_flops;
                prop.opencl_available_ram = prop.global_mem_size;

                intel_gpu_opencls.push_back(prop);

                // At present Intel GPUs only support OpenCL
                // and do not have a native GPGPU framework,
                // so treat each detected GPU as a native device.
                //
                intel_gpus.push_back(c);
#ifdef __APPLE__
            } else if (is_apple(prop.vendor)) {
                //////////// APPLE GPU //////////////
                prop.device_num = (int)(apple_gpu_opencls.size());
                prop.opencl_device_index = device_index;

                COPROC_APPLE c;
                c.opencl_prop = prop;
                c.is_used = COPROC_UNUSED;
                c.available_ram = prop.global_mem_size;

                c.set_peak_flops();
                if (c.bad_gpu_peak_flops("Apple OpenCL", s)) {
                    gpu_warning(warnings, s.c_str());
                }
                prop.peak_flops = c.peak_flops;
                prop.opencl_available_ram = prop.global_mem_size;

                apple_gpu_opencls.push_back(prop);
#endif
            } else {
                //////////// OTHER GPU OR ACCELERATOR //////////////
                // Put each coprocessor instance into a separate other_opencls element

                // opencl_device_index is passed to project apps via init_data.xml
                // to differentiate among OpenCL devices from the same vendor. It is
                // used by boinc_get_opencl_ids() to select the correct OpenCL device.
                int opencl_device_index = 0;
                for (unsigned int coproc_index=0; coproc_index<other_opencls.size(); coproc_index++) {
                    if (!strcmp(other_opencls[coproc_index].vendor, prop.vendor)) {
                        opencl_device_index++;  // Another OpenCL device from same vendor
                    }
                }

                prop.device_num = 0;    // Each vector entry has only one device
                prop.opencl_device_index = opencl_device_index;
                prop.opencl_available_ram = prop.global_mem_size;
                prop.is_used = COPROC_USED;

                // TODO: is there a better way to estimate peak_flops?
                //
                prop.peak_flops = 0;
                if (prop.max_compute_units) {
                    double freq = ((double)prop.max_clock_frequency) * MEGA;
                    prop.peak_flops = ((double)prop.max_compute_units) * freq;
                }
                if (prop.peak_flops <= 0 || prop.peak_flops > GPU_MAX_PEAK_FLOPS) {
                    char buf2[256];
                    snprintf(buf2, sizeof(buf2),
                        "OpenCL generic: bad peak FLOPS; Max units %u, max freq %u MHz",
                        prop.max_compute_units, prop.max_clock_frequency
                    );
                    gpu_warning(warnings, buf2);
                    prop.peak_flops = GPU_DEFAULT_PEAK_FLOPS;
                }

                other_opencls.push_back(prop);
            }
        }
    }

    // nvidia.count etc. haven't been set yet,
    // so we can't use have_nvidia() etc.
    //
    if ((nvidia_opencls.size() > 0) || nvidia.have_cuda) max_other_coprocs--;
    if ((ati_opencls.size() > 0) || ati.have_cal) max_other_coprocs--;
    if (intel_gpu_opencls.size() > 0) max_other_coprocs--;
    if (apple_gpu_opencls.size() > 0) max_other_coprocs--;
    if ((int)other_opencls.size() > max_other_coprocs) {
        gpu_warning(warnings, "Too many OpenCL device types found");
    }


#ifdef __APPLE__
    // Work around a bug in OpenCL which returns only
    // 1/2 of total global RAM size.
    // This bug applies only to ATI GPUs, not to NVIDIA
    // This has already been fixed on latest Catalyst
    // drivers, but Mac does not use Catalyst drivers.
    if (ati_opencls.size() > 0) {
        // This problem seems to be fixed in OS 10.7
        if (compareOSVersionTo(10, 7) < 0) {
            opencl_get_ati_mem_size_from_opengl(warnings);
        }
    }
#endif

    if ((nvidia_opencls.size() == 0) &&
        (ati_opencls.size() == 0) &&
        (intel_gpu_opencls.size() == 0) &&
        (apple_gpu_opencls.size() == 0) &&
        (cpu_opencls.size() == 0) &&
        (other_opencls.size() == 0)
    ) {
        gpu_warning(warnings, "OpenCL library present but no OpenCL-capable devices found");
    }
leave:
#ifdef _WIN32
    if (opencl_lib) FreeLibrary(opencl_lib);
#else
    if (opencl_lib) dlclose(opencl_lib);
#endif
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
        intel_gpu.find_best_opencls(
            use_all, intel_gpu_opencls,
            ignore_gpu_instance[PROC_TYPE_INTEL_GPU]
        );
        intel_gpu.available_ram = intel_gpu.opencl_prop.global_mem_size;
        safe_strcpy(intel_gpu.name, intel_gpu.opencl_prop.name);
    }
#ifdef __APPLE__
    if (apple_gpu_opencls.size() > 0) {
        if (apple_gpu.have_metal) {
            apple_gpu.merge_opencl(
                apple_gpu_opencls, ignore_gpu_instance[PROC_TYPE_APPLE_GPU]
            );
        } else {
            apple_gpu.find_best_opencls(
                use_all, apple_gpu_opencls,
                ignore_gpu_instance[PROC_TYPE_APPLE_GPU]
            );
            safe_strcpy(apple_gpu.model, apple_gpu.opencl_prop.name);
        }
        apple_gpu.set_peak_flops();
        if (!apple_gpu.available_ram) {
            apple_gpu.available_ram = apple_gpu.opencl_prop.global_mem_size;
        }
    }
#endif
}

cl_int COPROCS::get_opencl_info(
    OPENCL_DEVICE_PROP& prop,
    cl_uint device_index,
    vector<string>&warnings
) {
    cl_int ciErrNum;
    char buf[256];

    ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_NAME, sizeof(prop.name), prop.name, NULL);
    if ((ciErrNum != CL_SUCCESS) || (prop.name[0] == 0)) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get name for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_VENDOR, sizeof(prop.vendor), prop.vendor, NULL);
    if ((ciErrNum != CL_SUCCESS) || (prop.vendor[0] == 0)) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get vendor for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_VENDOR_ID, sizeof(prop.vendor_id), &prop.vendor_id, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get vendor ID for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_AVAILABLE, sizeof(prop.available), &prop.available, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get availability for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
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
            gpu_warning(warnings, buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_SINGLE_FP_CONFIG,
        sizeof(prop.single_fp_config), &prop.single_fp_config, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get single-precision floating point capabilities for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
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
            gpu_warning(warnings, buf);
            return ciErrNum;
        }
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_ENDIAN_LITTLE, sizeof(prop.endian_little),
        &prop.endian_little, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get little or big endian for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_EXECUTION_CAPABILITIES,
        sizeof(prop.execution_capabilities), &prop.execution_capabilities, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get execution capabilities for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_EXTENSIONS, sizeof(prop.extensions),
        prop.extensions, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get device extensions for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_GLOBAL_MEM_SIZE,
        sizeof(prop.global_mem_size), &prop.global_mem_size, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get global memory size for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_LOCAL_MEM_SIZE,
        sizeof(prop.local_mem_size), &prop.local_mem_size, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get local memory size for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY,
        sizeof(prop.max_clock_frequency), &prop.max_clock_frequency, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get max clock frequency for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(
        prop.device_id, CL_DEVICE_MAX_COMPUTE_UNITS,
        sizeof(prop.max_compute_units), &prop.max_compute_units, NULL
    );
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get max compute units for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_VERSION, sizeof(prop.opencl_device_version), prop.opencl_device_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get OpenCL version supported by device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DRIVER_VERSION, sizeof(prop.opencl_driver_version), prop.opencl_driver_version, NULL);
    if (ciErrNum != CL_SUCCESS) {
        snprintf(buf, sizeof(buf),
            "clGetDeviceInfo failed to get OpenCL driver version for device %d",
            (int)device_index
        );
        gpu_warning(warnings, buf);
        return ciErrNum;
    }

    // Nvidia Specific Extensions
    if (strstr(prop.extensions, "cl_nv_device_attribute_query") != NULL) {

        ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof(prop.nv_compute_capability_major), &prop.nv_compute_capability_major, NULL);
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV for device %d",
                (int)device_index
            );
            gpu_warning(warnings, buf);
            return ciErrNum;
        }

        ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof(prop.nv_compute_capability_minor), &prop.nv_compute_capability_minor, NULL);
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV for device %d",
                (int)device_index
            );
            gpu_warning(warnings, buf);
            return ciErrNum;
        }

    }

    // AMD Specific Extensions
    if (strstr(prop.extensions, "cl_amd_device_attribute_query") != NULL) {

        ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_BOARD_NAME_AMD, sizeof(buf), buf, NULL);
        if (strlen(buf) && ciErrNum == CL_SUCCESS) {
            safe_strcpy(prop.name, buf);
        } else if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get AMD Board Name for device %d",
                (int)device_index
            );
            gpu_warning(warnings, buf);
            return ciErrNum;
        }

        ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD, sizeof(prop.amd_simd_per_compute_unit), &prop.amd_simd_per_compute_unit, NULL);
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD for device %d",
                (int)device_index
            );
            gpu_warning(warnings, buf);
            return ciErrNum;
        }

        ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_SIMD_WIDTH_AMD, sizeof(prop.amd_simd_width), &prop.amd_simd_width, NULL);
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get CL_DEVICE_SIMD_WIDTH_AMD for device %d",
                (int)device_index
            );
            gpu_warning(warnings, buf);
            return ciErrNum;
        }

        ciErrNum = (*p_clGetDeviceInfo)(prop.device_id, CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD, sizeof(prop.amd_simd_instruction_width), &prop.amd_simd_instruction_width, NULL);
        if (ciErrNum != CL_SUCCESS) {
            snprintf(buf, sizeof(buf),
                "clGetDeviceInfo failed to get CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD for device %d",
                (int)device_index
            );
            gpu_warning(warnings, buf);
            return ciErrNum;
        }

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

    // find OpenCL info for the 'best' instance,
    // and copy it into this object
    //
    for (i=0; i<opencls.size(); i++) {
        opencls[i].is_used = COPROC_UNUSED;

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

    // Fill in OpenCL ID info for instances
    // which CAL or CUDA found equivalent to best instance
    //
    opencl_device_count = 0;
    for (i=0; i<(unsigned int)count; ++i) {
        for (j=0; j<opencls.size(); j++) {
            if (device_nums[i] == opencls[j].device_num) {
                opencls[j].is_used = COPROC_USED;
                opencl_device_indexes[opencl_device_count] = opencls[j].opencl_device_index;
                opencl_device_ids[opencl_device_count++] = opencls[j].device_id;
                instance_has_opencl[i] = true;
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

    // identify the most capable ATI, NVIDIA, Intel, or Apple OpenCL GPU
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
            instance_has_opencl[count] = true;
            device_nums[count++] = opencls[i].device_num;
            opencl_device_indexes[opencl_device_count] = opencls[i].opencl_device_index;
            opencl_device_ids[opencl_device_count++] = opencls[i].device_id;
            opencls[i].is_used = COPROC_USED;
        }
    }
}

void fake_opencl_gpu(char* type) {
    OPENCL_DEVICE_PROP op;
    op.clear();
    strcpy(op.name, type);
    strcpy(op.vendor, "ARM");
    op.vendor_id = 102760464;
    op.available = 1;
    op.half_fp_config = 63;
    op.single_fp_config = 63;
    op.double_fp_config = 63;
    op.endian_little = 1;
    op.execution_capabilities = 1;
    strcpy(op.extensions, "cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store cl_khr_3d_image_writes cl_khr_fp64 cl_khr_int64_base_atomics cl_khr_int64_extended_atomics cl_khr_fp16 cl_khr_gl_sharing cl_khr_icd cl_khr_egl_event cl_khr_egl_image cl_khr_image2d_from_buffer cl_arm_core_id cl_arm_printf cl_arm_thread_limit_hint cl_arm_non_uniform_work_group_size cl_arm_import_memory");
    op.global_mem_size = 2086998016;
    op.local_mem_size = 32768;
    op.max_clock_frequency = 600;
    op.max_compute_units = 2;
    strcpy(op.opencl_platform_version, "OpenCL 1.2 v1.r14p0-01rel0.0fe2d25ca074016740f8ab3fb451b151");
    strcpy(op.opencl_device_version,   "OpenCL 1.2 v1.r14p0-01rel0.0fe2d25ca074016740f8ab3fb451b151");
    strcpy(op.opencl_driver_version, "1.2");
    op.is_used = COPROC_USED;
    other_opencls.push_back(op);
}

#ifdef __APPLE__
// OpenCL returns incorrect total RAM size for some
// ATI GPUs so we get that info from OpenGL on Macs

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <Carbon/Carbon.h>
#include <IOKit/graphics/IOGraphicsLib.h>

static io_service_t IOServicePortFromCGDisplayID(CGDirectDisplayID displayID);

void COPROCS::opencl_get_ati_mem_size_from_opengl(vector<string>& warnings) {
    CGLRendererInfoObj info;
    long i, j;
    GLint numRenderers = 0, rv = 0, deviceVRAM, rendererID;
    cl_ulong deviceMemSize;
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
//                io_registry_entry_t dspPort = CGDisplayIOServicePort(displayID);  // Deprecated in OS 10.9
                io_registry_entry_t dspPort = IOServicePortFromCGDisplayID(displayID);

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
                CGLError notAvail = CGLDescribeRenderer (info, i, kCGLRPVideoMemoryMegabytes, &deviceVRAM);
                if (notAvail == kCGLNoError) {
                    deviceMemSize = ((cl_ulong)deviceVRAM) * (1024L*1024L);
                } else {	// kCGLRPVideoMemoryMegabytes is not available before OS 10.7
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                    // kCGLRPVideoMemory=120 is deprecated in OS 10.7 and may not be
                    // defined in later SDKs, so use a literal value here instead
                    // CGLDescribeRenderer (info, i, kCGLRPVideoMemory, &deviceVRAM);
                    CGLDescribeRenderer (info, i, (CGLRendererProperty)120, &deviceVRAM);
                    deviceMemSize = deviceVRAM;
#pragma clang diagnostic pop
                }

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
                            ati_opencls[ati_gpu_index].global_mem_size = deviceMemSize;
                            ati_opencls[ati_gpu_index].opencl_available_ram = deviceMemSize;

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
                                        gpu_warning(warnings, buf);
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
                                        gpu_warning(warnings, buf);
                                    }
                                }
                            }   // End if (log_flags.coproc_debug) {

                            ati_gpu_index++;
                        } // End if ATI / AMD GPU

                        CGLDestroyContext (cglContext);
                    } else {
                        gpu_warning(warnings,
                            "opencl_get_ati_mem_size_from_opengl failed to create context"
                        );
                    }
                } else {
                    gpu_warning(warnings,
                        "opencl_get_ati_mem_size_from_opengl failed to create PixelFormat"
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



// The following replaces CGDisplayIOServicePort which is deprecated in OS 10.9
//
//========================================================================
// GLFW 3.1 OS X - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

// Returns the io_service_t corresponding to a CG display ID, or 0 on failure.
// The io_service_t should be released with IOObjectRelease when not needed.
//

static io_service_t IOServicePortFromCGDisplayID(CGDirectDisplayID displayID)
{
    io_iterator_t iter;
    io_service_t serv, servicePort = 0;

    CFMutableDictionaryRef matching = IOServiceMatching("IODisplayConnect");

    // releases matching for us
    kern_return_t err = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                     matching,
                                                     &iter);
    if (err)
        return 0;

    while ((serv = IOIteratorNext(iter)) != 0)
    {
        CFDictionaryRef info;
        CFIndex vendorID, productID, serialNumber;
        CFNumberRef vendorIDRef, productIDRef, serialNumberRef;
        Boolean success;

        info = IODisplayCreateInfoDictionary(serv,
                                             kIODisplayOnlyPreferredName);

        vendorIDRef = (CFNumberRef)CFDictionaryGetValue(info,
                                           CFSTR(kDisplayVendorID));
        productIDRef = (CFNumberRef)CFDictionaryGetValue(info,
                                            CFSTR(kDisplayProductID));
        serialNumberRef = (CFNumberRef)CFDictionaryGetValue(info,
                                               CFSTR(kDisplaySerialNumber));

        success = CFNumberGetValue(vendorIDRef, kCFNumberCFIndexType,
                                   &vendorID);
        success &= CFNumberGetValue(productIDRef, kCFNumberCFIndexType,
                                    &productID);
        success &= CFNumberGetValue(serialNumberRef, kCFNumberCFIndexType,
                                    &serialNumber);

        if (!success)
        {
            CFRelease(info);
            continue;
        }
        // If the vendor and product id along with the serial don't match
        // then we are not looking at the correct monitor.
        // NOTE: The serial number is important in cases where two monitors
        //       are the exact same.
        if (CGDisplayVendorNumber(displayID) != vendorID  ||
            CGDisplayModelNumber(displayID) != productID  ||
            CGDisplaySerialNumber(displayID) != serialNumber)
        {
            CFRelease(info);
            continue;
        }

        // The VendorID, Product ID, and the Serial Number all Match Up!
        // Therefore we have found the appropriate display io_service
        servicePort = serv;
        CFRelease(info);
        break;
    }

    IOObjectRelease(iter);
    return servicePort;
}
#endif// __APPLE__
