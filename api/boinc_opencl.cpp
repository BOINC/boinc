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
// int boinc_get_opencl_ids(int argc, char** argv, cl_device_id*, cl_platform_id*);
//
// To use this function, link your application with libboinc_opencl.a
//

#ifdef _WIN32
#include "win_util.h"
#endif
#include <string>

#include "error_numbers.h"
#include "boinc_api.h"
#include "coproc.h"
#include "boinc_opencl.h"

// returns an OpenCL error num or zero
//
int boinc_get_opencl_ids_aux(
    char *type, int device_num, cl_device_id* device, cl_platform_id* platform
) {
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    char vendor[256];                 // Device vendor (NVIDIA, ATI, AMD, etc.)
    int retval = 0;
    bool found = false;

    retval = clGetPlatformIDs(MAX_OPENCL_PLATFORMS, platforms, &num_platforms);
    if (num_platforms == 0) return CL_DEVICE_NOT_FOUND;
    if (retval != CL_SUCCESS) return retval;
    
    for (platform_index=0; platform_index<num_platforms; ++platform_index) {
        retval = clGetDeviceIDs(
            platforms[platform_index], CL_DEVICE_TYPE_GPU,
            MAX_COPROC_INSTANCES, devices, &num_devices
        );
        if (retval != CL_SUCCESS) continue;

        if (device_num >= (int)num_devices) continue;
    
        cl_device_id device_id = devices[device_num];

        retval = clGetDeviceInfo(
            device_id, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL
        );
        if ((retval != CL_SUCCESS) || (strlen(vendor)==0)) continue;
            
        if ((strstr(vendor, "AMD")) ||  
            (strstr(vendor, "Advanced Micro Devices, Inc."))
        ) {
            strcpy(vendor, GPU_TYPE_ATI);
        }
        
	if (strcasestr(vendor, "nvidia"))
	{
            strcpy(vendor, GPU_TYPE_NVIDIA);
	}
        if (!strcmp(vendor, type)) {
            *device = device_id;
            *platform = platforms[platform_index];
            found = true;
            break;
        }
    }

    if (!found) return CL_DEVICE_NOT_FOUND;
    return 0;
}

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
    int opencl_device_index;

    retval = boinc_parse_init_data_file();
    if (retval) return retval;
    boinc_get_init_data(aid);
    
    if (!strlen(aid.gpu_type)) {
        fprintf(stderr, "GPU type not found in %s\n", INIT_DATA_FILE);
        return ERR_NOT_FOUND;
    }
    
    opencl_device_index = aid.gpu_opencl_dev_index;
    if (opencl_device_index < 0) {
        // Older versions of init_data.xml don't have gpu_opencl_dev_index field
        opencl_device_index = aid.gpu_device_num;
    }

    if (opencl_device_index < 0) {
        fprintf(stderr, "GPU device # not found in %s\n", INIT_DATA_FILE);
        return ERR_NOT_FOUND;
    }

    retval = boinc_get_opencl_ids_aux(
        aid.gpu_type, opencl_device_index, device, platform
    );

    return retval;
}
