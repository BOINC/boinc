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
#include "win_util.h"
#endif
#include <string>

#include "error_numbers.h"
#include "coproc.h"
#include "str_replace.h"

#include "boinc_api.h"

#include "boinc_opencl.h"

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
int get_vendor(cl_device_id device_id, char* vendor) {
    int retval = 0;

    retval = clGetDeviceInfo(
        device_id, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL
    );
    if ((retval != CL_SUCCESS) || (strlen(vendor)==0)) return retval;
        
    if ((strstr(vendor, "AMD")) ||  
        (strstr(vendor, "Advanced Micro Devices, Inc."))
    ) {
        strcpy(vendor, GPU_TYPE_ATI);
    }
    
    if (strcasestr(vendor, "nvidia")) {
        strcpy(vendor, GPU_TYPE_NVIDIA);
    }

    if (!strlen(vendor)) return CL_INVALID_DEVICE_TYPE;
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

    if ((!type) || (!strlen(type))) return CL_DEVICE_NOT_FOUND;

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
                retval = get_vendor(device_id, vendor);
                if (retval != CL_SUCCESS) continue;
            
                if (!strcmp(vendor, type)) {
                    *device = device_id;
                    *platform = platforms[platform_index];
                    return 0;
                }
            }
            
            continue;
        }
        
        // Older versions of init_data.xml don't have gpu_opencl_dev_index field
        // NOTE: This may return the wrong device on older versions of BOINC if
        //  OpenCL does not recognize all GPU models detected by CUDA
        for (device_index=0; device_index<(int)num_devices; ++device_index) {
            device_id = devices[device_index];

            retval = get_vendor(device_id, vendor);
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

    return CL_DEVICE_NOT_FOUND;
}


// This version is compatible with older clients.
// Usage:
// Pass the argc and argv received from the BOINC client
// type: may be NULL, empty string, "NVIDIA", "nvidia", "ATI", "AMD",
//       "Advanced Micro Devices, Inc.", "intel_gpu" or "INTEL_GPU"
//       (if NULL or empty string then it will fail on older clients.)
//
// returns
// - 0 if success
// - ERR_FOPEN if init_data.xml missing
// - ERR_XML_PARSE if can't parse init_data.xml
// - ERR_NOT_FOUND if unable to get gpu_type information
// - ERR_NOT_FOUND if unable to get opencl_device_index or gpu device_num
// - an OpenCL error number if OpenCL error
//
int boinc_get_opencl_ids(
    int argc, char** argv, char* type,
    cl_device_id* device, cl_platform_id* platform
){
    int retval;
    APP_INIT_DATA aid;
    char *gpu_type;
    int gpu_device_num = -1;
    int i;

    retval = boinc_parse_init_data_file();
    if (retval) return retval;
    boinc_get_init_data(aid);
    
    if (strlen(aid.gpu_type)) {
        gpu_type = aid.gpu_type;
    } else {
        gpu_type = type;
    }
    
    if ((!gpu_type) || !strlen(gpu_type)) {
        fprintf(stderr, "GPU type not found in %s\n", INIT_DATA_FILE);
        return ERR_NOT_FOUND;
    }
    
    if (aid.gpu_opencl_dev_index < 0) {
        // Older versions of init_data.xml don't have gpu_opencl_dev_index field
        //
        gpu_device_num = aid.gpu_device_num;
        if (gpu_device_num < 0) {
            // Even older versions of init_data.xml don't have gpu_device_num field
            for (i=0; i<argc-1; i++) {
                if ((!strcmp(argv[i], "--device")) || (!strcmp(argv[i], "-device"))) {
                    gpu_device_num = atoi(argv[i+1]);
                    break;
                }
            }
        }
    }

    if ((aid.gpu_opencl_dev_index < 0) && (gpu_device_num < 0)) {
        fprintf(stderr, "GPU device # not found in %s\n", INIT_DATA_FILE);
        return ERR_NOT_FOUND;
    }

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
// - ERR_NOT_FOUND if missing <gpu_type> or <gpu_device_num> fields
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
        return ERR_NOT_FOUND;
    }
    
    if ((aid.gpu_opencl_dev_index < 0) && (aid.gpu_device_num < 0)) {
        fprintf(stderr, "GPU device # not found in %s\n", INIT_DATA_FILE);
        return ERR_NOT_FOUND;
    }

    retval = boinc_get_opencl_ids_aux(
        aid.gpu_type, aid.gpu_opencl_dev_index, aid.gpu_device_num, device, platform
    );

    return retval;
}
