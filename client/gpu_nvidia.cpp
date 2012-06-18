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

// Detection of NVIDIA GPUs

#ifdef _WIN32
#include "boinc_win.h"
#include "nvapi.h"
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

#include "client_msgs.h"
#include "gpu_detect.h"

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
    CU_DEVICE_ATTRIBUTE_PCI_BUS_ID = 33,
    CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID = 34,
    CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID = 50
};

#ifdef _WIN32
typedef int (__stdcall *CUDA_GDC)(int *count);
typedef int (__stdcall *CUDA_GDV)(int* version);
typedef int (__stdcall *CUDA_GDI)(unsigned int);
typedef int (__stdcall *CUDA_GDG)(int*, int);
typedef int (__stdcall *CUDA_GDA)(int*, int, int);
typedef int (__stdcall *CUDA_GDN)(char*, int, int);
typedef int (__stdcall *CUDA_GDM)(size_t*, int);
typedef int (__stdcall *CUDA_GDCC)(int*, int*, int);
typedef int (__stdcall *CUDA_CC)(void**, unsigned int, unsigned int);
typedef int (__stdcall *CUDA_CD)(void*);
typedef int (__stdcall *CUDA_MA)(unsigned int*, size_t);
typedef int (__stdcall *CUDA_MF)(unsigned int);
typedef int (__stdcall *CUDA_MGI)(size_t*, size_t*);

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
int (*__cuInit)(unsigned int);
int (*__cuDeviceGetCount)(int*);
int (*__cuDriverGetVersion)(int*);
int (*__cuDeviceGet)(int*, int);
int (*__cuDeviceGetAttribute)(int*, int, int);
int (*__cuDeviceGetName)(char*, int, int);
int (*__cuDeviceTotalMem)(size_t*, int);
int (*__cuDeviceComputeCapability)(int*, int*, int);
int (*__cuCtxCreate)(void**, unsigned int, unsigned int);
int (*__cuCtxDestroy)(void*);
int (*__cuMemAlloc)(unsigned int*, size_t);
int (*__cuMemFree)(unsigned int);
int (*__cuMemGetInfo)(size_t*, size_t*);
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
    __cuInit = (int(*)(unsigned int)) dlsym( cudalib, "cuInit" );
    __cuDeviceGet = (int(*)(int*, int)) dlsym( cudalib, "cuDeviceGet" );
    __cuDeviceGetAttribute = (int(*)(int*, int, int)) dlsym( cudalib, "cuDeviceGetAttribute" );
    __cuDeviceGetName = (int(*)(char*, int, int)) dlsym( cudalib, "cuDeviceGetName" );
    __cuDeviceTotalMem = (int(*)(size_t*, int)) dlsym( cudalib, "cuDeviceTotalMem" );
    __cuDeviceComputeCapability = (int(*)(int*, int*, int)) dlsym( cudalib, "cuDeviceComputeCapability" );
    __cuCtxCreate = (int(*)(void**, unsigned int, unsigned int)) dlsym( cudalib, "cuCtxCreate" );
    __cuCtxDestroy = (int(*)(void*)) dlsym( cudalib, "cuCtxDestroy" );
    __cuMemAlloc = (int(*)(unsigned int*, size_t)) dlsym( cudalib, "cuMemAlloc" );
    __cuMemFree = (int(*)(unsigned int)) dlsym( cudalib, "cuMemFree" );
    __cuMemGetInfo = (int(*)(size_t*, size_t*)) dlsym( cudalib, "cuMemGetInfo" );
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
    size_t global_mem;
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
        (*__cuDeviceTotalMem)(&global_mem, device);
        cc.prop.totalGlobalMem = (double) global_mem;
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
        (*__cuDeviceGetAttribute)(&cc.prop.multiProcessorCount, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, device);
        (*__cuDeviceGetAttribute)(&cc.pci_info.bus_id, CU_DEVICE_ATTRIBUTE_PCI_BUS_ID, device);
        (*__cuDeviceGetAttribute)(&cc.pci_info.device_id, CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID, device);
        (*__cuDeviceGetAttribute)(&cc.pci_info.domain_id, CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID, device);
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
    // and set "count", "device_nums", and "pci_infos"
    //
    count = 0;
    for (i=0; i<nvidia_gpus.size(); i++) {
        if (in_vector(nvidia_gpus[i].device_num, ignore_devs)) {
            nvidia_gpus[i].is_used = COPROC_IGNORED;
        } else if (use_all || !nvidia_compare(nvidia_gpus[i], *this, true)) {
            device_nums[count] = nvidia_gpus[i].device_num;
            pci_infos[count] = nvidia_gpus[i].pci_info;
            count++;
            nvidia_gpus[i].is_used = COPROC_USED;
        } else {
            nvidia_gpus[i].is_used = COPROC_UNUSED;
        }
    }
}

// See how much RAM is available on this GPU.
//
void COPROC_NVIDIA::get_available_ram() {
    int retval;
    size_t memfree, memtotal;
    int device;
    void* ctx;
    
    available_ram = prop.totalGlobalMem;
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

