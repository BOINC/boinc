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
#include "str_replace.h"

#include "coproc.h"

#include "boinc_opencl.h"

#ifdef _WIN32
static HMODULE opencl_lib = NULL;

typedef cl_int (__stdcall *CL_PLATFORMIDS) (cl_uint, cl_platform_id*, cl_uint*);
typedef cl_int (__stdcall *CL_DEVICEIDS)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
typedef cl_int (__stdcall *CL_INFO) (cl_device_id, cl_device_info, size_t, void*, size_t*);

static CL_PLATFORMIDS  __clGetPlatformIDs = NULL;
static CL_DEVICEIDS    __clGetDeviceIDs = NULL;
static CL_INFO         __clGetDeviceInfo = NULL;

#else

static jmp_buf resume;

static void segv_handler(int) {
    longjmp(resume, 1);
}

static void* opencl_lib = NULL;

static cl_int (*__clGetPlatformIDs)(cl_uint    /* num_entries */,
                 cl_platform_id * /* platforms */,
                 cl_uint *        /* num_platforms */);
static cl_int (*__clGetDeviceIDs)(cl_platform_id   /* platform */,
               cl_device_type   /* device_type */,
               cl_uint          /* num_entries */,
               cl_device_id *   /* devices */,
               cl_uint *        /* num_devices */);
static cl_int (*__clGetDeviceInfo)(cl_device_id    /* device */,
                cl_device_info  /* param_name */,
                size_t          /* param_value_size */,
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */);
#endif

int boinc_get_opencl_ids_aux(
    char *type, int device_num, cl_device_id* device, cl_platform_id* platform
) {
    cl_int errnum;
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    char vendor[256];                 // Device vendor (NVIDIA, ATI, AMD, etc.)
    int retval = 0;

#ifdef _WIN32
    opencl_lib = LoadLibrary("OpenCL.dll");
    if (!opencl_lib) {
        return ERR_NOT_IMPLEMENTED;
    }
    __clGetPlatformIDs = (CL_PLATFORMIDS)GetProcAddress( opencl_lib, "clGetPlatformIDs" );
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
        return ERR_NOT_IMPLEMENTED;
    }
    
    __clGetPlatformIDs = (cl_int(*)(cl_uint, cl_platform_id*, cl_uint*)) dlsym( opencl_lib, "clGetPlatformIDs" );
    __clGetDeviceIDs = (cl_int(*)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*)) dlsym( opencl_lib, "clGetDeviceIDs" );
    __clGetDeviceInfo = (cl_int(*)(cl_device_id, cl_device_info, size_t, void*, size_t*)) dlsym( opencl_lib, "clGetDeviceInfo" );
#endif
    if ((!__clGetPlatformIDs) || (!__clGetDeviceIDs) || (!__clGetDeviceInfo)) {
        retval = ERR_NOT_IMPLEMENTED;
        goto bail;
    }

    errnum = (*__clGetPlatformIDs)(MAX_OPENCL_PLATFORMS, platforms, &num_platforms);
    if (num_platforms == 0) errnum = CL_DEVICE_NOT_FOUND;
    if (errnum != CL_SUCCESS) {
        retval = errnum;
        goto bail;
    }
    
    for (platform_index=0; platform_index<num_platforms; ++platform_index) {
        errnum = (*__clGetDeviceIDs)(
            platforms[platform_index], CL_DEVICE_TYPE_GPU, MAX_COPROC_INSTANCES, devices, &num_devices
        );

        if (num_devices > (cl_uint)(device_num + 1)) continue;
    
        cl_device_id device_id = devices[device_num];

        errnum = (*__clGetDeviceInfo)(device_id, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
        if ((errnum != CL_SUCCESS) || (vendor[0] == 0)) continue;
            
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

bail:
#ifndef _WIN32
    dlclose(opencl_lib);
#endif

    if (!retval && device == NULL) {
        retval = CL_DEVICE_NOT_FOUND;
    }
    return retval;
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
        retval = boinc_get_opencl_ids_aux(type, device_num, &ref);
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
