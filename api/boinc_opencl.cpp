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

// The BOINC client calls the project application with the arguments:
//   --gpu_type TYPE --device N
// where TYPE is ATI or NVIDIA, and N is the GPU number of that type
// For example, for ATI GPU number 0, the arguments will be:
//   --gpu_type ATI --device 0
//
// To get the cl_device_id and cl_platform_id for the OpenCL GPU 
// asigned to your application call this function:
// int boinc_get_opencl_ids(int argc, char** argv, cl_device_id*, cl_platform_id*);
//
// NOTE: You should compile and link this function as part of your 
// application; it is not included in the standard BOINC libraries.
//

#ifdef _WIN32
#include "win_util.h"
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

#include "error_numbers.h"
#include "util.h"
#include "str_replace.h"

#include "coproc.h"

#include "boinc_opencl.h"

#ifndef _WIN32
static jmp_buf resume;
static void segv_handler(int) {
    longjmp(resume, 1);
}
#endif

int boinc_get_opencl_ids_aux(
    char *type, int device_num, cl_device_id* device, cl_platform_id* platform
) {
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    char vendor[256];                 // Device vendor (NVIDIA, ATI, AMD, etc.)
    int retval = 0;

    retval = clGetPlatformIDs(MAX_OPENCL_PLATFORMS, platforms, &num_platforms);
    if (num_platforms == 0) return CL_DEVICE_NOT_FOUND;
    if (retval) return retval;
    
    for (platform_index=0; platform_index<num_platforms; ++platform_index) {
        retval = clGetDeviceIDs(
            platforms[platform_index], CL_DEVICE_TYPE_GPU, MAX_COPROC_INSTANCES, devices, &num_devices
        );

        if (num_devices > (cl_uint)(device_num + 1)) continue;
    
        cl_device_id device_id = devices[device_num];

        retval = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
        if (retval || strlen(vendor)==0) continue;
            
        if ((strstr(vendor, "AMD")) ||  
            (strstr(vendor, "Advanced Micro Devices, Inc."))
        ) {
            strcpy(vendor, GPU_TYPE_ATI);
        }
        
        if (!strcmp(vendor, type)) {
            *device = device_id;
            *platform = platforms[platform_index];
            break;
        }
    }

    if (device == NULL) return CL_DEVICE_NOT_FOUND;
    return 0;
}

int boinc_get_opencl_ids(
    int argc, char** argv, cl_device_id* device, cl_platform_id* platform
) {
    char type[256];
    int device_num, retval=0;
    
    strcpy(type, "");
    device_num = -1;
    
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--gpu_type")) {
            strlcpy(type, argv[++i], sizeof(type));
        }
        if (!strcmp(argv[i], "--device")) {
            device_num = atoi(argv[++i]);
        }
    }

    if (!strlen(type)) {
        return CL_INVALID_DEVICE_TYPE;
    }
    
    if (device_num < 0) {
        return CL_INVALID_DEVICE;
    }

#ifdef _WIN32
    try {
        retval = boinc_get_opencl_ids_aux(type, device_num, device, platform);
    }
    catch (...) {
        return ERR_SIGNAL_CATCH;
    }
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        return ERR_SIGNAL_CATCH;
    } else {
        retval = boinc_get_opencl_ids_aux(type, device_num, device, platform);
    }

    signal(SIGSEGV, old_sig);
#endif
    
    return retval;
}
