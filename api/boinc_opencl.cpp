// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// BOINC API for OpenCL
//
// To get the cl_device_id and cl_platform_id for the OpenCL GPU
// assigned to your application call this function:
// int boinc_get_opencl_ids(int argc, char** argv, char *type, cl_device_id* device, cl_platform_id* platform);
//
// To use this function, link your application with libboinc_opencl.a
//

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <string>
#endif

#include "error_numbers.h"
#include "coproc.h"
#include "str_replace.h"
#include "boinc_api.h"

#include "boinc_opencl.h"

static int compareBOINCVersionTo(int toMajor, int toMinor, int toRelease);

// A few complicating factors:
// Windows & Linux have a separate OpenCL platform for each vendor
//  (NVIDIA, AMD, Intel).
// Mac has only one platform (Apple) which reports GPUs from all vendors.
//
// In all systems, opencl_device_indexes start at 0 for each platform
//  and device_nums start at 0 for each vendor.
//
// On Macs, OpenCL does not always recognize all GPU models detected by
//  CUDA, so a device_num may not correspond to its opencl_device_index
//  even if all GPUs are from NVIDIA.
//
int get_vendor(cl_device_id device_id, char* vendor, int len) {
    int retval = 0;
    strlcpy(vendor, "", len);
    retval = clGetDeviceInfo(
        device_id, CL_DEVICE_VENDOR, len, vendor, NULL
    );
    if (retval != CL_SUCCESS) return retval;
    if (!strlen(vendor)) return CL_INVALID_DEVICE_TYPE;
    if ((strstr(vendor, "AMD")) ||
        (strstr(vendor, "Advanced Micro Devices, Inc."))
    ) {
        strlcpy(vendor, GPU_TYPE_ATI, len);       // "ATI"
    } else if (strcasestr(vendor, "nvidia")) {
        strlcpy(vendor, GPU_TYPE_NVIDIA, len);    // "NVIDIA"
    } else if (strcasestr(vendor, "intel")) {
        strlcpy(vendor, GPU_TYPE_INTEL, len);     // "intel_gpu"
    } else if (strcasestr(vendor, "apple")) {
        strlcpy(vendor, GPU_TYPE_APPLE, len);     // "intel_gpu"
    }
    return 0;
}


// returns an OpenCL error num or zero
//
int boinc_get_opencl_ids_aux(
    char* type, int opencl_device_index, int device_num,
    cl_device_id* device, cl_platform_id* platform
) {
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    char vendor[256];               // Device vendor (NVIDIA, ATI, AMD, etc.)
    int retval = 0;
    cl_device_id device_id;
    int device_num_for_type = -1;
    int device_index;

    if ((!type) || (!strlen(type))) return CL_INVALID_DEVICE_TYPE;

    retval = clGetPlatformIDs(MAX_OPENCL_PLATFORMS, platforms, &num_platforms);
    if (num_platforms == 0) return CL_DEVICE_NOT_FOUND;
    if (retval != CL_SUCCESS) return retval;

    for (platform_index=0; platform_index<num_platforms; ++platform_index) {
        retval = clGetDeviceIDs(
            platforms[platform_index], CL_DEVICE_TYPE_GPU,
            MAX_COPROC_INSTANCES, devices, &num_devices
        );
        if (retval != CL_SUCCESS) continue;

        // Use gpu_opencl_dev_index if available
        if (opencl_device_index >= 0) {
            if (opencl_device_index < (int)num_devices) {
                device_id = devices[opencl_device_index];
                retval = get_vendor(device_id, vendor, sizeof(vendor));
                if (retval != CL_SUCCESS) continue;

                if (!strcmp(vendor, type)) {
                    *device = device_id;
                    *platform = platforms[platform_index];
                    return 0;
                }
            }

            continue;
        }

        // Older versions of init_data.xml don't have the gpu_opencl_dev_index
        // field so use the value of gpu_device_num.
        // NOTE: This may return the wrong device on older versions of BOINC if
        // OpenCL does not recognize all GPU models detected by CUDA or CAL
        for (device_index=0; device_index<(int)num_devices; ++device_index) {
            device_id = devices[device_index];

            retval = get_vendor(device_id, vendor, sizeof(vendor));
            if (retval != CL_SUCCESS) continue;

            if (!strcmp(vendor, type)) {
                if(++device_num_for_type == device_num) {
                    *device = device_id;
                    *platform = platforms[platform_index];
                    return 0;
                }
            }
        }
    }

    fprintf(stderr, "GPU not found: type=%s, opencl_device_index=%d, device_num=%d\n",
                        type, opencl_device_index, device_num);

    return CL_DEVICE_NOT_FOUND;
}


// This version is compatible with older clients.
// Usage:
// Pass the argc and argv received from the BOINC client
// type: may be PROC_TYPE_NVIDIA_GPU, PROC_TYPE_AMD_GPU,
//      PROC_TYPE_INTEL_GPU or PROC_TYPE_APPLE_GPU
//      (it may also be 0, but then it will fail on older clients.)
//
// The argc, argv and type arguments are ignored for 6.13.3 or later clients.
//
// returns
// - 0 if success
// - ERR_FOPEN if init_data.xml missing
// - ERR_XML_PARSE if can't parse init_data.xml
// - CL_INVALID_DEVICE_TYPE if unable to get gpu_type information
// - CL_INVALID_DEVICE if unable to get opencl_device_index or gpu device_num
// - CL_DEVICE_NOT_FOUND if the requested device was not found
// - an OpenCL error number if OpenCL error
//
int boinc_get_opencl_ids(
    int argc, char** argv, int type,
    cl_device_id* device, cl_platform_id* platform
){
    int retval;
    APP_INIT_DATA aid;
    char *gpu_type = NULL;
    int gpu_device_num = -1;
    int i;

    retval = boinc_parse_init_data_file();
    if (retval) return retval;
    boinc_get_init_data(aid);

    if (strlen(aid.gpu_type)) {
        gpu_type = aid.gpu_type;
    } else {
        switch (type) {
        case PROC_TYPE_NVIDIA_GPU:
            gpu_type = (char *)GPU_TYPE_NVIDIA;
            break;
        case PROC_TYPE_AMD_GPU:
            gpu_type = (char *)GPU_TYPE_ATI;
            break;
        case PROC_TYPE_INTEL_GPU:
            gpu_type = (char *)GPU_TYPE_INTEL;
            break;
        case PROC_TYPE_APPLE_GPU:
            gpu_type = (char *)GPU_TYPE_APPLE;
            break;
        }
    }

    if ((!gpu_type) || !strlen(gpu_type)) {
        fprintf(stderr, "GPU type not found in %s\n", INIT_DATA_FILE);
        return CL_INVALID_DEVICE_TYPE;
    }

    if (aid.gpu_opencl_dev_index < 0) {
         if (compareBOINCVersionTo(7,0,12) >= 0) {
            // gpu_opencl_dev_index was added in BOINC version 7.0.12.
            // A gpu_opencl_dev_index value of -1 in version 7.0.12 or later
            // means BOINC client did not assign an OpenCL GPU to this task.
            fprintf(stderr, "Illegal value for gpu_opencl_dev_index: %d in BOINC Client %d.%d.%d\n",
                        aid.gpu_opencl_dev_index, aid.major_version, aid.minor_version, aid.release);
            return CL_INVALID_DEVICE;
        }

        // Older versions of init_data.xml don't have the gpu_opencl_dev_index
        // field so use the value of gpu_device_num if available.
        gpu_device_num = aid.gpu_device_num;
        if (gpu_device_num < 0) {
             if (compareBOINCVersionTo(6,13,3) < 0) {
                // gpu_device_num and gpu_type fields were added in BOINC version 6.13.3.
                // Very old versions of init_data.xml don't have gpu_device_num field
                // but instead pass the device number as a command-line argument.
                for (i=0; i<argc-1; i++) {
                    if ((!strcmp(argv[i], "--device")) || (!strcmp(argv[i], "-device"))) {
                        gpu_device_num = atoi(argv[i+1]);
                        break;
                    }
                }
            }

            if (gpu_device_num < 0) {
                // BOINC client apparently did not assign a GPU to this task.
                fprintf(stderr, "Illegal value for gpu_device_num: %d in BOINC Client %d.%d.%d\n",
                        aid.gpu_device_num, aid.major_version, aid.minor_version, aid.release);
                return CL_INVALID_DEVICE;
            }
        }
    }   // End if (aid.gpu_opencl_dev_index < 0)

    retval = boinc_get_opencl_ids_aux(
        gpu_type, aid.gpu_opencl_dev_index, gpu_device_num, device, platform
    );

    return retval;
}


// Deprecated: use the version above instead
//
// returns
// - 0 if success
// - ERR_FOPEN if init_data.xml missing
// - ERR_XML_PARSE if can't parse init_data.xml
// - CL_INVALID_DEVICE_TYPE if unable to get gpu_type information
// - CL_INVALID_DEVICE if unable to get opencl_device_index or gpu device_num
// - CL_DEVICE_NOT_FOUND if the requested device was not found
// - an OpenCL error number if OpenCL error
//
int boinc_get_opencl_ids(cl_device_id* device, cl_platform_id* platform) {
    int retval;
    APP_INIT_DATA aid;

    retval = boinc_parse_init_data_file();
    if (retval) return retval;
    boinc_get_init_data(aid);

    if (!strlen(aid.gpu_type)) {
        fprintf(stderr, "GPU type not found in %s\n", INIT_DATA_FILE);
        return CL_INVALID_DEVICE_TYPE;
    }

    if (aid.gpu_opencl_dev_index < 0) {
        if (compareBOINCVersionTo(7,0,12) >= 0) {
            // gpu_opencl_dev_index was added in BOINC version 7.0.12.
            // A gpu_opencl_dev_index value of -1 in version 7.0.12 or
            // later means BOINC did not assign an OpenCL GPU to this task.
            fprintf(stderr, "Illegal value for gpu_opencl_dev_index: %d in BOINC Client %d.%d.%d\n",
                        aid.gpu_opencl_dev_index, aid.major_version, aid.minor_version, aid.release);
            return CL_INVALID_DEVICE;
        }

        if (aid.gpu_device_num < 0) {
             if (compareBOINCVersionTo(6,13,3) >= 0) {
                // gpu_device_num and gpu_type fields were added in BOINC version 6.13.3.
                // A gpu_device_num value of -1 in version 6.13.3 or later means
                // BOINC did not assign a GPU to this task.
            fprintf(stderr, "Illegal value for gpu_device_num: %d in BOINC Client %d.%d.%d\n",
                        aid.gpu_device_num, aid.major_version, aid.minor_version, aid.release);
                return CL_INVALID_DEVICE;
            }
        }
    }

    retval = boinc_get_opencl_ids_aux(
        aid.gpu_type, aid.gpu_opencl_dev_index, aid.gpu_device_num, device, platform
    );

    return retval;
}

static int compareBOINCVersionTo(int toMajor, int toMinor, int toRelease) {
    APP_INIT_DATA aid;

    boinc_get_init_data(aid);

    if (aid.major_version < toMajor) return -1;
    if (aid.major_version > toMajor) return 1;
    if (aid.minor_version < toMinor) return -1;
    if (aid.minor_version > toMinor) return 1;
    if (aid.release < toRelease) return -1;
    if (aid.release > toRelease) return 1;
    return 0;
}



