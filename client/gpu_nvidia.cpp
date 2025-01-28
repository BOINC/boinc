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
/* get annotation macros from sal.h */
/* define the ones that don't exist */
#include "sal.h"
/* These are just an annotations.  They don't do anything */
#ifndef __success
#define __success(x)
#endif
#ifndef __in
#define __in
#endif
#ifndef __out
#define __out
#endif
#ifndef __in_ecount
#define __in_ecount(x)
#endif
#ifndef __out_ecount
#define __out_ecount(x)
#endif
#ifndef __in_opt
#define __in_opt
#endif
#ifndef __out_opt
#define __out_opt
#endif
#ifndef __inout
#define __inout
#endif
#ifndef __inout_opt
#define __inout_opt
#endif
#ifndef __inout_ecount
#define __inout_ecount(x)
#endif
#ifndef __inout_ecount_full
#define __inout_ecount_full(x)
#endif
#ifndef __inout_ecount_part_opt
#define __inout_ecount_part_opt(x,y)
#endif
#ifndef __inout_ecount_full_opt
#define __inout_ecount_full_opt(x,y)
#endif
#ifndef __out_ecount_full_opt
#define __out_ecount_full_opt(x)
#endif

#include "nvapi.h"

#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#include <Carbon/Carbon.h>
#include "hostinfo.h"
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

static void get_available_nvidia_ram(COPROC_NVIDIA &cc, vector<string>& warnings);

#ifndef SIM
#if !(defined(_WIN32) || defined(__APPLE__))

static int nvidia_driver_version() {
    int (*nvml_init)()  = NULL;
    int (*nvml_finish)()  = NULL;
    int (*nvml_driver)(char *f, unsigned int len) = NULL;
    int dri_ver = 0;
    int major=0, minor=0;
    void *handle = NULL;
    char driver_string[81];

    handle  = dlopen("libnvidia-ml.so.1", RTLD_NOW);
    if (!handle) {
        handle  = dlopen("libnvidia-ml.so", RTLD_NOW);
        if (!handle) {
            goto end;
        }
    }

    nvml_driver = (int(*)(char *, unsigned int)) dlsym(handle,  "nvmlSystemGetDriverVersion");
    nvml_init = (int(*)(void)) dlsym(handle,  "nvmlInit");
    nvml_finish = (int(*)(void)) dlsym(handle,  "nvmlShutdown");
    if (!nvml_driver || !nvml_init || !nvml_finish) goto end;

    if (nvml_init()) goto end;
    if (nvml_driver(driver_string, 80)) goto end;
    sscanf(driver_string, "%d.%d", &major, &minor);
    dri_ver = major*100 + std::min(minor, 99);
        // minor can in fact be > 99, at least on Linux
        // encoding as MMnn doesn't work.
        // this is a temporary workaround.
end:
    if (nvml_finish) nvml_finish();
    if (handle) dlclose(handle);
    return dri_ver;
}

#endif
#endif // SIM

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

CUDA_GDC p_cuDeviceGetCount = NULL;
CUDA_GDV p_cuDriverGetVersion = NULL;
CUDA_GDI p_cuInit = NULL;
CUDA_GDG p_cuDeviceGet = NULL;
CUDA_GDA p_cuDeviceGetAttribute = NULL;
CUDA_GDN p_cuDeviceGetName = NULL;
CUDA_GDM p_cuDeviceTotalMem = NULL;
CUDA_GDM p_cuDeviceTotalMem_v2 = NULL;
CUDA_GDCC p_cuDeviceComputeCapability = NULL;
CUDA_CC p_cuCtxCreate = NULL;
CUDA_CD p_cuCtxDestroy = NULL;
CUDA_MA p_cuMemAlloc = NULL;
CUDA_MF p_cuMemFree = NULL;
CUDA_MGI p_cuMemGetInfo = NULL;
CUDA_MGI p_cuMemGetInfo_v2 = NULL;
#else
int (*p_cuInit)(unsigned int);
int (*p_cuDeviceGetCount)(int*);
int (*p_cuDriverGetVersion)(int*);
int (*p_cuDeviceGet)(int*, int);
int (*p_cuDeviceGetAttribute)(int*, int, int);
int (*p_cuDeviceGetName)(char*, int, int);
int (*p_cuDeviceTotalMem)(size_t*, int);
int (*p_cuDeviceTotalMem_v2)(size_t*, int);
int (*p_cuDeviceComputeCapability)(int*, int*, int);
int (*p_cuCtxCreate)(void**, unsigned int, unsigned int);
int (*p_cuCtxDestroy)(void*);
int (*p_cuMemAlloc)(unsigned int*, size_t);
int (*p_cuMemFree)(unsigned int);
int (*p_cuMemGetInfo)(size_t*, size_t*);
int (*p_cuMemGetInfo_v2)(size_t*, size_t*);
#endif

// NVIDIA interfaces are documented here:
// http://developer.download.nvidia.com/compute/cuda/2_3/toolkit/docs/online/index.html

void COPROC_NVIDIA::get(
    vector<string>& warnings
) {
    int cuda_ndevs, retval;
    char buf[2048];
    int j, itemp;
    size_t global_mem = 0;
    string s;
    COPROC_NVIDIA cc;

#ifdef _WIN32
    HMODULE cudalib = LoadLibrary("nvcuda.dll");
    if (!cudalib) {
        gpu_warning(warnings, "No NVIDIA library found");
        return;
    }
    p_cuDeviceGetCount = (CUDA_GDC)GetProcAddress( cudalib, "cuDeviceGetCount" );
    p_cuDriverGetVersion = (CUDA_GDV)GetProcAddress( cudalib, "cuDriverGetVersion" );
    p_cuInit = (CUDA_GDI)GetProcAddress( cudalib, "cuInit" );
    p_cuDeviceGet = (CUDA_GDG)GetProcAddress( cudalib, "cuDeviceGet" );
    p_cuDeviceGetAttribute = (CUDA_GDA)GetProcAddress( cudalib, "cuDeviceGetAttribute" );
    p_cuDeviceGetName = (CUDA_GDN)GetProcAddress( cudalib, "cuDeviceGetName" );
    p_cuDeviceTotalMem = (CUDA_GDM)GetProcAddress( cudalib, "cuDeviceTotalMem" );
    p_cuDeviceTotalMem_v2 = (CUDA_GDM)GetProcAddress(cudalib, "cuDeviceTotalMem_v2");
    p_cuDeviceComputeCapability = (CUDA_GDCC)GetProcAddress( cudalib, "cuDeviceComputeCapability" );
    p_cuCtxCreate = (CUDA_CC)GetProcAddress( cudalib, "cuCtxCreate" );
    p_cuCtxDestroy = (CUDA_CD)GetProcAddress( cudalib, "cuCtxDestroy" );
    p_cuMemAlloc = (CUDA_MA)GetProcAddress( cudalib, "cuMemAlloc" );
    p_cuMemFree = (CUDA_MF)GetProcAddress( cudalib, "cuMemFree" );
    p_cuMemGetInfo = (CUDA_MGI)GetProcAddress(cudalib, "cuMemGetInfo");
    p_cuMemGetInfo_v2 = (CUDA_MGI)GetProcAddress(cudalib, "cuMemGetInfo_v2");

#ifndef SIM
    NvAPI_Initialize();
    NvAPI_ShortString ss;
    NvU32 Version = 0;
    NvAPI_SYS_GetDriverAndBranchVersion(&Version, ss);

#if 0
    // NvAPI now provides an API for getting #cores :-)
    // But not FLOPs per clock cycle :-(
    // Anyway, don't use this for now because server code estimates FLOPS
    // based on compute capability, so we may as well do the same
    // See http://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/
    //
    NvPhysicalGpuHandle GPUHandle[NVAPI_MAX_PHYSICAL_GPUS];
    NvU32 GpuCount, nc;
    NvAPI_EnumPhysicalGPUs(GPUHandle, &GpuCount);
    for (unsigned int i=0; i<GpuCount; i++) {
        NvAPI_GPU_GetGpuCoreCount(GPUHandle[i], &nc);
    }
#endif
#endif
#else

void* cudalib = NULL;

#ifdef __APPLE__
    cudalib = dlopen("/usr/local/cuda/lib/libcuda.dylib", RTLD_NOW);
#else
    cudalib = dlopen("libcuda.so", RTLD_NOW);
#endif
    if (!cudalib) {
        snprintf(buf, sizeof(buf), "NVIDIA: %s", dlerror());
        gpu_warning(warnings, buf);
        return;
    }
    p_cuDeviceGetCount = (int(*)(int*)) dlsym(cudalib, "cuDeviceGetCount");
    p_cuDriverGetVersion = (int(*)(int*)) dlsym( cudalib, "cuDriverGetVersion" );
    p_cuInit = (int(*)(unsigned int)) dlsym( cudalib, "cuInit" );
    p_cuDeviceGet = (int(*)(int*, int)) dlsym( cudalib, "cuDeviceGet" );
    p_cuDeviceGetAttribute = (int(*)(int*, int, int)) dlsym( cudalib, "cuDeviceGetAttribute" );
    p_cuDeviceGetName = (int(*)(char*, int, int)) dlsym( cudalib, "cuDeviceGetName" );
    p_cuDeviceTotalMem = (int(*)(size_t*, int)) dlsym( cudalib, "cuDeviceTotalMem" );
    p_cuDeviceTotalMem_v2 = (int(*)(size_t*, int)) dlsym(cudalib, "cuDeviceTotalMem_v2");
    p_cuDeviceComputeCapability = (int(*)(int*, int*, int)) dlsym( cudalib, "cuDeviceComputeCapability" );
    p_cuCtxCreate = (int(*)(void**, unsigned int, unsigned int)) dlsym( cudalib, "cuCtxCreate" );
    p_cuCtxDestroy = (int(*)(void*)) dlsym( cudalib, "cuCtxDestroy" );
    p_cuMemAlloc = (int(*)(unsigned int*, size_t)) dlsym( cudalib, "cuMemAlloc" );
    p_cuMemFree = (int(*)(unsigned int)) dlsym( cudalib, "cuMemFree" );
    p_cuMemGetInfo = (int(*)(size_t*, size_t*)) dlsym( cudalib, "cuMemGetInfo" );
    p_cuMemGetInfo_v2 = (int(*)(size_t*, size_t*)) dlsym(cudalib, "cuMemGetInfo_v2");
#endif

    if (!p_cuDriverGetVersion) {
        gpu_warning(warnings, "cuDriverGetVersion() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuInit) {
        gpu_warning(warnings, "cuInit() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuDeviceGetCount) {
        gpu_warning(warnings, "cuDeviceGetCount() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuDeviceGet) {
        gpu_warning(warnings, "cuDeviceGet() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuDeviceGetAttribute) {
        gpu_warning(warnings, "cuDeviceGetAttribute() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuDeviceTotalMem && !p_cuDeviceTotalMem_v2) {
        gpu_warning(warnings, "cuDeviceTotalMem() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuDeviceComputeCapability) {
        gpu_warning(warnings, "cuDeviceComputeCapability() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuMemAlloc) {
        gpu_warning(warnings, "cuMemAlloc() missing from NVIDIA library");
        goto leave;
    }
    if (!p_cuMemFree) {
        gpu_warning(warnings, "cuMemFree() missing from NVIDIA library");
        goto leave;
    }

    retval = (*p_cuInit)(0);
#ifdef __APPLE__
    // If system is just booting, CUDA driver may not be ready yet
    if (retval) {
        if (get_system_uptime() < 300) {   // Retry only if system has been up for under 5 minutes
            for (int retryCount=0; retryCount<120; retryCount++) {
                retval = (*p_cuInit)(0);
                if (!retval) break;
                boinc_sleep(1.);
                continue;
            }
        }
    }
#endif

    if (retval) {
        snprintf(buf, sizeof(buf), "NVIDIA drivers present but no GPUs found");
        gpu_warning(warnings, buf);
        goto leave;
    }

    retval = (*p_cuDriverGetVersion)(&cuda_version);
    if (retval) {
        snprintf(buf, sizeof(buf), "cuDriverGetVersion() returned %d", retval);
        gpu_warning(warnings, buf);
        goto leave;
    }

    have_cuda = true;

    retval = (*p_cuDeviceGetCount)(&cuda_ndevs);
    if (retval) {
        snprintf(buf, sizeof(buf), "cuDeviceGetCount() returned %d", retval);
        gpu_warning(warnings, buf);
        goto leave;
    }
    snprintf(buf, sizeof(buf), "NVIDIA library reports %d GPU%s", cuda_ndevs, (cuda_ndevs==1)?"":"s");
    gpu_warning(warnings, buf);

    for (j=0; j<cuda_ndevs; j++) {
        cc.prop.clear();
        CUdevice device;
        retval = (*p_cuDeviceGet)(&device, j);
        if (retval) {
            snprintf(buf, sizeof(buf), "cuDeviceGet(%d) returned %d", j, retval);
            gpu_warning(warnings, buf);
            goto leave;
        }
        retval = (*p_cuDeviceGetName)(cc.prop.name, 256, device);
        if (retval) {
            snprintf(buf, sizeof(buf), "cuDeviceGetName(%d) returned %d", j, retval);
            gpu_warning(warnings, buf);
            goto leave;
        }
        (*p_cuDeviceComputeCapability)(&cc.prop.major, &cc.prop.minor, device);
        if (p_cuDeviceTotalMem_v2) {
            (*p_cuDeviceTotalMem_v2)(&global_mem, device);
        } else {
            (*p_cuDeviceTotalMem)(&global_mem, device);
        }
        cc.prop.totalGlobalMem = (double) global_mem;
        (*p_cuDeviceGetAttribute)(&itemp, CU_DEVICE_ATTRIBUTE_SHARED_MEMORY_PER_BLOCK, device);
        cc.prop.sharedMemPerBlock = (double) itemp;
        (*p_cuDeviceGetAttribute)(&cc.prop.regsPerBlock, CU_DEVICE_ATTRIBUTE_REGISTERS_PER_BLOCK, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.warpSize, CU_DEVICE_ATTRIBUTE_WARP_SIZE, device);
        (*p_cuDeviceGetAttribute)(&itemp, CU_DEVICE_ATTRIBUTE_MAX_PITCH, device);
        cc.prop.memPitch = (double) itemp;
        retval = (*p_cuDeviceGetAttribute)(&cc.prop.maxThreadsPerBlock, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, device);
        retval = (*p_cuDeviceGetAttribute)(&cc.prop.maxThreadsDim[0], CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.maxThreadsDim[1], CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.maxThreadsDim[2], CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.maxGridSize[0], CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.maxGridSize[1], CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.maxGridSize[2], CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.clockRate, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, device);
        (*p_cuDeviceGetAttribute)(&itemp, CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY, device);
        cc.prop.totalConstMem = (double) itemp;
        (*p_cuDeviceGetAttribute)(&itemp, CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT, device);
        cc.prop.textureAlignment = (double) itemp;
        (*p_cuDeviceGetAttribute)(&cc.prop.deviceOverlap, CU_DEVICE_ATTRIBUTE_GPU_OVERLAP, device);
        (*p_cuDeviceGetAttribute)(&cc.prop.multiProcessorCount, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, device);
        (*p_cuDeviceGetAttribute)(&cc.pci_info.bus_id, CU_DEVICE_ATTRIBUTE_PCI_BUS_ID, device);
        (*p_cuDeviceGetAttribute)(&cc.pci_info.device_id, CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID, device);
        (*p_cuDeviceGetAttribute)(&cc.pci_info.domain_id, CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID, device);
        if (cc.prop.major <= 0) continue;  // major == 0 means emulation
        if (cc.prop.major > 100) continue;  // e.g. 9999 is an error
#ifdef SIM
        cc.display_driver_version = 0;
#elif defined(_WIN32)
        cc.display_driver_version = Version;
#elif defined(__APPLE__)
        cc.display_driver_version = NSVersionOfRunTimeLibrary("cuda");
#else
        cc.display_driver_version = nvidia_driver_version();
#endif
        cc.have_cuda = true;
        cc.cuda_version = cuda_version;
        cc.device_num = j;
        cc.set_peak_flops();
        if (cc.bad_gpu_peak_flops("CUDA", s)) {
            gpu_warning(warnings, s.c_str());
        }
        get_available_nvidia_ram(cc, warnings);
        nvidia_gpus.push_back(cc);
    }
    if (!nvidia_gpus.size()) {
        gpu_warning(warnings, "No CUDA-capable NVIDIA GPUs found");
    }

leave:
#ifdef _WIN32
    if (cudalib) FreeLibrary(cudalib);
#else
    if (cudalib) dlclose(cudalib);
#endif
}

// Find the most capable instance; copy to *this.
// set is_used (USED, UNUSED, IGNORED) for each instance.
// Don't use less-capable instances (unless use_all is set)
//
void COPROC_NVIDIA::correlate(
    bool use_all,    // if false, use only those equivalent to most capable
    vector<int>& ignore_devs
) {
    unsigned int i;

    if (!nvidia_gpus.size()) return;

    // identify the most capable non-ignored instance
    //
    bool first = true;
    for (i=0; i<nvidia_gpus.size(); i++) {
        nvidia_gpus[i].is_used = COPROC_IGNORED;
        if (in_vector(nvidia_gpus[i].device_num, ignore_devs)) continue;
#ifdef __APPLE__
        if ((nvidia_gpus[i].cuda_version >= 6050) && nvidia_gpus[i].prop.major < 2) {
            // Can't use GPUs with compute capability < 2 with CUDA drivers >= 6.5.x
            nvidia_gpus[i].is_used = COPROC_UNUSED;
            continue;
        }
#endif
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
        } else if (this->have_opencl && !nvidia_gpus[i].have_opencl) {
            nvidia_gpus[i].is_used = COPROC_UNUSED;
        } else if (this->have_cuda && !nvidia_gpus[i].have_cuda) {
            nvidia_gpus[i].is_used = COPROC_UNUSED;
#ifdef __APPLE__
        } else if (nvidia_gpus[i].is_used == COPROC_UNUSED) {
            // Can't use GPUs with compute capability < 2 with CUDA drivers >= 6.5.x
            continue;
#endif
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
// CAUTION: as currently written, this method should be
// called only from COPROC_NVIDIA::get().  If in the
// future you wish to call it from additional places:
// * It must be called from a separate child process on
//   dual-GPU laptops (e.g., Macbook Pros) with the results
//   communicated to the main client process via IPC or a
//   temp file.  See the comments about dual-GPU laptops
//   in gpu_detect.cpp and main.cpp for more details.
// * The CUDA library must be loaded and cuInit() called
//   first.
// * See client/coproc_detect.cpp and cpu_sched.cpp in
//   BOINC 6.12.36 for an earlier attempt to call this
//   from the scheduler.  Note that it was abandoned
//   due to repeated calls crashing the driver.
//
static void get_available_nvidia_ram(COPROC_NVIDIA &cc, vector<string>& warnings) {
    int retval;
    size_t memfree = 0, memtotal = 0;
    int device;
    void* ctx;
    char buf[256];

    cc.available_ram = cc.prop.totalGlobalMem;
    if (!p_cuDeviceGet) {
        gpu_warning(warnings, "cuDeviceGet() missing from NVIDIA library");
        return;
    }
    if (!p_cuCtxCreate) {
        gpu_warning(warnings, "cuCtxCreate() missing from NVIDIA library");
        return;
    }
    if (!p_cuCtxDestroy) {
        gpu_warning(warnings, "cuCtxDestroy() missing from NVIDIA library");
        return;
    }
    if (!p_cuMemGetInfo && !p_cuMemGetInfo_v2) {
        gpu_warning(warnings, "cuMemGetInfo() missing from NVIDIA library");
        return;
    }

    retval = (*p_cuDeviceGet)(&device, cc.device_num);
    if (retval) {
        snprintf(buf, sizeof(buf),
            "[coproc] cuDeviceGet(%d) returned %d", cc.device_num, retval
        );
        gpu_warning(warnings, buf);
        return;
    }
    retval = (*p_cuCtxCreate)(&ctx, 0, device);
    if (retval) {
        snprintf(buf, sizeof(buf),
            "[coproc] cuCtxCreate(%d) returned %d", cc.device_num, retval
        );
        gpu_warning(warnings, buf);
        return;
    }
    if (p_cuMemGetInfo_v2) {
        retval = (*p_cuMemGetInfo_v2)(&memfree, &memtotal);
    }
    else {
        retval = (*p_cuMemGetInfo)(&memfree, &memtotal);
    }
    if (retval) {
        snprintf(buf, sizeof(buf),
            "[coproc] cuMemGetInfo(%d) returned %d", cc.device_num, retval
        );
        gpu_warning(warnings, buf);
        (*p_cuCtxDestroy)(ctx);
        return;
    }
    (*p_cuCtxDestroy)(ctx);
    cc.available_ram = (double) memfree;
}

// check whether each GPU is running a graphics app (assume yes)
// return true if there's been a change since last time
//
// CAUTION: this method is not currently used.  If you wish
// to call it in the future:
// * It must be called from a separate child process on
//   dual-GPU laptops (e.g., Macbook Pros) with the results
//   communicated to the main client process via IPC or a
//   temp file.  See the comments about dual-GPU laptops
//   in gpu_detect.cpp and main.cpp for more details.
// * The CUDA library must be loaded and cuInit() called
//   first.
//
#if 0
bool COPROC_NVIDIA::check_running_graphics_app() {
    int retval, j;
    bool change = false;
    if (!p_cuDeviceGet) {
        gpu_warning(warnings, "cuDeviceGet() missing from NVIDIA library");
        return;
    }
    if (!p_cuDeviceGetAttribute) {
        gpu_warning(warnings, "cuDeviceGetAttribute() missing from NVIDIA library");
        return;
    }

    for (j=0; j<count; j++) {
        bool new_val = true;
        int device, kernel_timeout;
        retval = (*p_cuDeviceGet)(&device, j);
        if (!retval) {
            retval = (*p_cuDeviceGetAttribute)(&kernel_timeout, CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT, device);
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
#endif
