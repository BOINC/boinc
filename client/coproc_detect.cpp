// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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


// client-specific GPU code.  Mostly GPU detection

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#ifndef SIM
#include <nvapi.h>
#endif
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#endif
#include "config.h"
#include <dlfcn.h>
#include <setjmp.h>
#include <signal.h>
#endif

#include "coproc.h"
#include "str_util.h"
#include "util.h"

#include "client_state.h"
#include "client_msgs.h"

using std::string;
using std::vector;

//#define MEASURE_AVAILABLE_RAM

static bool in_vector(int n, vector<int>& v) {
    for (unsigned int i=0; i<v.size(); i++) {
        if (v[i] == n) return true;
    }
    return false;
}

#ifndef _WIN32
jmp_buf resume;

void segv_handler(int) {
    longjmp(resume, 1);
}
#endif

void COPROC::print_available_ram() {
#ifdef MEASURE_AVAILABLE_RAM
    if (gstate.now - last_print_time < 60) return;
    last_print_time = gstate.now;

    for (int i=0; i<count; i++) {
        if (available_ram_unknown[i]) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] %s device %d: available RAM unknown",
                    type, device_nums[i]
                );
            }
        } else {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] %s device %d: available RAM %d MB",
                    type, device_nums[i],
                    (int)(available_ram[i]/MEGA)
                );
            }
        }
    }
#endif
}

void COPROCS::get(
    bool use_all, vector<string>&descs, vector<string>&warnings,
    vector<int>& ignore_cuda_dev,
    vector<int>& ignore_ati_dev
) {

#ifdef _WIN32
    COPROC_CUDA::get(*this, use_all, descs, warnings, ignore_cuda_dev);
    COPROC_ATI::get(*this, descs, warnings, ignore_ati_dev);
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        COPROC_CUDA::get(*this, use_all, descs, warnings, ignore_cuda_dev);
    }
#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    } else {
        COPROC_ATI::get(*this, descs, warnings, ignore_ati_dev);
    }
#endif
    signal(SIGSEGV, old_sig);
#endif
}

// return 1/-1/0 if device 1 is more/less/same capable than device 2.
// If "loose", ignore FLOPS and tolerate small memory diff
//
int cuda_compare(COPROC_CUDA& c1, COPROC_CUDA& c2, bool loose) {
    if (c1.prop.major > c2.prop.major) return 1;
    if (c1.prop.major < c2.prop.major) return -1;
    if (c1.prop.minor > c2.prop.minor) return 1;
    if (c1.prop.minor < c2.prop.minor) return -1;
    if (c1.cuda_version > c2.cuda_version) return 1;
    if (c1.cuda_version < c2.cuda_version) return -1;
    if (loose) {
        if (c1.prop.totalGlobalMem > 1.4*c2.prop.totalGlobalMem) return 1;
        if (c1.prop.totalGlobalMem < .7* c2.prop.totalGlobalMem) return -1;
        return 0;
    }
    if (c1.prop.totalGlobalMem > c2.prop.totalGlobalMem) return 1;
    if (c1.prop.totalGlobalMem < c2.prop.totalGlobalMem) return -1;
    double s1 = c1.peak_flops();
    double s2 = c2.peak_flops();
    if (s1 > s2) return 1;
    if (s1 < s2) return -1;
    return 0;
}

#ifdef _WIN32
typedef int (__stdcall *CUDA_GDC)(int *count);
typedef int (__stdcall *CUDA_GDV)(int* version);
typedef int (__stdcall *CUDA_GDI)(int);
typedef int (__stdcall *CUDA_GDG)(int*, int);
typedef int (__stdcall *CUDA_GDA)(int*, int, int);
typedef int (__stdcall *CUDA_GDN)(char*, int, int);
typedef int (__stdcall *CUDA_GDM)(unsigned int*, int);
typedef int (__stdcall *CUDA_GDCC)(int*, int*, int);
typedef int (__stdcall *CUDA_CC)(unsigned int*, unsigned int, unsigned int);
typedef int (__stdcall *CUDA_CD)(unsigned int);
typedef int (__stdcall *CUDA_MA)(unsigned int*, unsigned int);
typedef int (__stdcall *CUDA_MF)(unsigned int);
typedef int (__stdcall *CUDA_MGI)(unsigned int*, unsigned int*);

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
int (*__cuInit)(int);
int (*__cuDeviceGetCount)(int*);
int (*__cuDriverGetVersion)(int*);
int (*__cuDeviceGet)(int*, int);
int (*__cuDeviceGetAttribute)(int*, int, int);
int (*__cuDeviceGetName)(char*, int, int);
int (*__cuDeviceTotalMem)(unsigned int*, int);
int (*__cuDeviceComputeCapability)(int*, int*, int);
int (*__cuCtxCreate)(unsigned int*, unsigned int, unsigned int);
int (*__cuCtxDestroy)(unsigned int);
int (*__cuMemAlloc)(unsigned int*, unsigned int);
int (*__cuMemFree)(unsigned int);
int (*__cuMemGetInfo)(unsigned int*, unsigned int*);
#endif

// NVIDIA interfaces are documented here:
// http://developer.download.nvidia.com/compute/cuda/2_3/toolkit/docs/online/index.html

void COPROC_CUDA::get(
    COPROCS& coprocs,
    bool use_all,    // if false, use only those equivalent to most capable
    vector<string>& descs,
    vector<string>& warnings,
    vector<int>& ignore_devs
) {
    int count, retval;
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
    NvDisplayHandle hDisplay;
    NV_DISPLAY_DRIVER_VERSION Version;
    memset(&Version, 0, sizeof(Version));
    Version.version = NV_DISPLAY_DRIVER_VERSION_VER;

    NvAPI_Initialize();
    for (int i=0; ; i++) {
        nvapiStatus = NvAPI_EnumNvidiaDisplayHandle(i, &hDisplay);
        if (nvapiStatus != NVAPI_OK) break;
        nvapiStatus = NvAPI_GetDisplayDriverVersion(hDisplay, &Version);
        if (nvapiStatus == NVAPI_OK) break;
    }
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
    __cuInit = (int(*)(int)) dlsym( cudalib, "cuInit" );
    __cuDeviceGet = (int(*)(int*, int)) dlsym( cudalib, "cuDeviceGet" );
    __cuDeviceGetAttribute = (int(*)(int*, int, int)) dlsym( cudalib, "cuDeviceGetAttribute" );
    __cuDeviceGetName = (int(*)(char*, int, int)) dlsym( cudalib, "cuDeviceGetName" );
    __cuDeviceTotalMem = (int(*)(unsigned int*, int)) dlsym( cudalib, "cuDeviceTotalMem" );
    __cuDeviceComputeCapability = (int(*)(int*, int*, int)) dlsym( cudalib, "cuDeviceComputeCapability" );
    __cuCtxCreate = (int(*)(unsigned int*, unsigned int, unsigned int)) dlsym( cudalib, "cuCtxCreate" );
    __cuCtxDestroy = (int(*)(unsigned int)) dlsym( cudalib, "cuCtxDestroy" );
    __cuMemAlloc = (int(*)(unsigned int*, unsigned int)) dlsym( cudalib, "cuMemAlloc" );
    __cuMemFree = (int(*)(unsigned int)) dlsym( cudalib, "cuMemFree" );
    __cuMemGetInfo = (int(*)(unsigned int*, unsigned int*)) dlsym( cudalib, "cuMemGetInfo" );
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

    int cuda_version;
    retval = (*__cuDriverGetVersion)(&cuda_version);
    if (retval) {
        sprintf(buf, "cuDriverGetVersion() returned %d", retval);
        warnings.push_back(buf);
        return;
    }

    vector<COPROC_CUDA> gpus;
    retval = (*__cuDeviceGetCount)(&count);
    if (retval) {
        sprintf(buf, "cuDeviceGetCount() returned %d", retval);
        warnings.push_back(buf);
        return;
    }
    sprintf(buf, "NVIDIA library reports %d GPU%s", count, (count==1)?"":"s");
    warnings.push_back(buf);

    int j;
    unsigned int i;
    COPROC_CUDA cc;
    string s;
    for (j=0; j<count; j++) {
        memset(&cc.prop, 0, sizeof(cc.prop));
        int device;
        retval = (*__cuDeviceGet)(&device, j);
        if (retval) {
            sprintf(buf, "cuDeviceGet(%d) returned %d", j, retval);
            warnings.push_back(buf);
            return;
        }
        (*__cuDeviceGetName)(cc.prop.name, 256, device);
        if (retval) {
            sprintf(buf, "cuDeviceGetName(%d) returned %d", j, retval);
            warnings.push_back(buf);
            return;
        }
        (*__cuDeviceComputeCapability)(&cc.prop.major, &cc.prop.minor, device);
        (*__cuDeviceTotalMem)(&cc.prop.totalGlobalMem, device);
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
        retval = (*__cuDeviceGetAttribute)(&cc.prop.multiProcessorCount, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, device);
        //retval = (*__cuDeviceGetProperties)(&cc.prop, device);
        if (cc.prop.major <= 0) continue;  // major == 0 means emulation
        if (cc.prop.major > 100) continue;  // e.g. 9999 is an error
#if defined(_WIN32) && !defined(SIM)
        cc.display_driver_version = Version.drvVersion;
#else
        cc.display_driver_version = 0;
#endif
        cc.cuda_version = cuda_version;
        cc.device_num = j;
        gpus.push_back(cc);
    }

    if (!gpus.size()) {
        warnings.push_back("No CUDA-capable NVIDIA GPUs found");
        return;
    }

    // identify the most capable non-ignored instance
    //
    COPROC_CUDA best;
    bool first = true;
    for (i=0; i<gpus.size(); i++) {
        if (in_vector(gpus[i].device_num, ignore_devs)) continue;
        if (first) {
            best = gpus[i];
            first = false;
        } else if (cuda_compare(gpus[i], best, false) > 0) {
            best = gpus[i];
        }
    }

    // see which other instances are equivalent,
    // and set the "count" and "device_nums" fields
    //
    best.count = 0;
    for (i=0; i<gpus.size(); i++) {
        char buf2[256];
        gpus[i].description(buf);
        if (in_vector(gpus[i].device_num, ignore_devs)) {
            sprintf(buf2, "NVIDIA GPU %d (ignored by config): %s", gpus[i].device_num, buf);
        } else if (use_all || !cuda_compare(gpus[i], best, true)) {
            best.device_nums[best.count] = gpus[i].device_num;
            best.count++;
            sprintf(buf2, "NVIDIA GPU %d: %s", gpus[i].device_num, buf);
        } else {
            sprintf(buf2, "NVIDIA GPU %d (not used): %s", gpus[i].device_num, buf);
        }
        descs.push_back(string(buf2));
    }

    if (best.count) {
        COPROC_CUDA* ccp = new COPROC_CUDA;
        *ccp = best;
        coprocs.coprocs.push_back(ccp);
    }
}

// fake a NVIDIA GPU (for debugging)
//
COPROC_CUDA* fake_cuda(COPROCS& coprocs, double ram, int count) {
   COPROC_CUDA* cc = new COPROC_CUDA;
   strcpy(cc->type, "CUDA");
   cc->count = count;
   for (int i=0; i<count; i++) {
       cc->device_nums[i] = i;
   }
   cc->display_driver_version = 18000;
   cc->cuda_version = 2020;
   strcpy(cc->prop.name, "Fake NVIDIA GPU");
   cc->prop.totalGlobalMem = (unsigned int)ram;
   cc->prop.sharedMemPerBlock = 100;
   cc->prop.regsPerBlock = 8;
   cc->prop.warpSize = 10;
   cc->prop.memPitch = 10;
   cc->prop.maxThreadsPerBlock = 20;
   cc->prop.maxThreadsDim[0] = 2;
   cc->prop.maxThreadsDim[1] = 2;
   cc->prop.maxThreadsDim[2] = 2;
   cc->prop.maxGridSize[0] = 10;
   cc->prop.maxGridSize[1] = 10;
   cc->prop.maxGridSize[2] = 10;
   cc->prop.totalConstMem = 10;
   cc->prop.major = 1;
   cc->prop.minor = 2;
   cc->prop.clockRate = 1250000;
   cc->prop.textureAlignment = 1000;
   cc->prop.multiProcessorCount = 14;
   coprocs.coprocs.push_back(cc);
   return cc;
}

// See how much RAM is available on each GPU.
// If this fails, set "available_ram_unknown"
//
void COPROC_CUDA::get_available_ram() {
#ifdef MEASURE_AVAILABLE_RAM
    int device, i, retval;
    unsigned int memfree, memtotal;
    unsigned int ctx;
    
    // avoid crash if faked GPU
    //
    if (!__cuDeviceGet) {
        for (i=0; i<count; i++) {
            available_ram[i] = available_ram_fake[i];
            available_ram_unknown[i] = false;
        }
        return;
    }
    for (i=0; i<count; i++) {
        int devnum = device_nums[i];
        available_ram[i] = 0;
        available_ram_unknown[i] = true;
        retval = (*__cuDeviceGet)(&device, devnum);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] cuDeviceGet(%d) returned %d", devnum, retval
                );
            }
            continue;
        }
        retval = (*__cuCtxCreate)(&ctx, 0, device);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] cuCtxCreate(%d) returned %d", devnum, retval
                );
            }
            continue;
        }
        retval = (*__cuMemGetInfo)(&memfree, &memtotal);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] cuMemGetInfo(%d) returned %d", devnum, retval
                );
            }
            (*__cuCtxDestroy)(ctx);
            continue;
        }
        (*__cuCtxDestroy)(ctx);
        available_ram[i] = (double) memfree;
        available_ram_unknown[i] = false;
    }
#else
    for (int i=0; i<count; i++) {
        available_ram[i] = prop.totalGlobalMem;
    }
#endif
}

// check whether each GPU is running a graphics app (assume yes)
// return true if there's been a change since last time
//
bool COPROC_CUDA::check_running_graphics_app() {
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

////////////////// ATI STARTS HERE /////////////////
//
// Docs:
// http://developer.amd.com/gpu_assets/Stream_Computing_User_Guide.pdf
// ?? why don't they have HTML docs??

#ifdef _WIN32
typedef int (__stdcall *ATI_ATTRIBS) (CALdeviceattribs *attribs, CALuint ordinal);
typedef int (__stdcall *ATI_CLOSE)(void);
typedef int (__stdcall *ATI_GDC)(CALuint *numDevices);
typedef int (__stdcall *ATI_GDI)(void);
typedef int (__stdcall *ATI_INFO) (CALdeviceinfo *info, CALuint ordinal);
typedef int (__stdcall *ATI_VER) (CALuint *cal_major, CALuint *cal_minor, CALuint *cal_imp);
typedef int (__stdcall *ATI_STATUS) (CALdevicestatus*, CALdevice);
typedef int (__stdcall *ATI_DEVICEOPEN) (CALdevice*, CALuint);
typedef int (__stdcall *ATI_DEVICECLOSE) (CALdevice);

ATI_ATTRIBS __calDeviceGetAttribs = NULL;
ATI_CLOSE   __calShutdown = NULL;
ATI_GDC     __calDeviceGetCount = NULL;
ATI_GDI     __calInit = NULL;
ATI_INFO    __calDeviceGetInfo = NULL;
ATI_VER     __calGetVersion = NULL;
ATI_STATUS  __calDeviceGetStatus = NULL;
ATI_DEVICEOPEN  __calDeviceOpen = NULL;
ATI_DEVICECLOSE  __calDeviceClose = NULL;

#else

int (*__calInit)();
int (*__calGetVersion)(CALuint*, CALuint*, CALuint*);
int (*__calDeviceGetCount)(CALuint*);
int (*__calDeviceGetAttribs)(CALdeviceattribs*, CALuint);
int (*__calShutdown)();
int (*__calDeviceGetInfo)(CALdeviceinfo*, CALuint);
int (*__calDeviceGetStatus)(CALdevicestatus*, CALdevice);
int (*__calDeviceOpen)(CALdevice*, CALuint);
int (*__calDeviceClose)(CALdevice);

#endif

void COPROC_ATI::get(COPROCS& coprocs,
    vector<string>& descs, vector<string>& warnings, vector<int>& ignore_devs
) {
    CALuint numDevices, cal_major, cal_minor, cal_imp;
    CALdevice device;
    CALdeviceinfo info;
    CALdeviceattribs attribs;
    char buf[256];
    bool amdrt_detected = false;
    bool atirt_detected = false;
    int retval;

    attribs.struct_size = sizeof(CALdeviceattribs);
    device = 0;
    numDevices =0;

#ifdef _WIN32

#if defined _M_X64
    const char* atilib_name = "aticalrt64.dll";
    const char* amdlib_name = "amdcalrt64.dll";
#else
    const char* atilib_name = "aticalrt.dll";
    const char* amdlib_name = "amdcalrt.dll";
#endif

    HINSTANCE callib = LoadLibrary(atilib_name);
    if (callib) {
        atirt_detected = true;
    } else {
        callib = LoadLibrary(amdlib_name);
        if (callib) {
            amdrt_detected = true;
        }
    }

    if (!callib) {
        warnings.push_back("No ATI library found.");
        return;
    }

    __calInit = (ATI_GDI)GetProcAddress(callib, "calInit" );
    __calGetVersion = (ATI_VER)GetProcAddress(callib, "calGetVersion" );
    __calDeviceGetCount = (ATI_GDC)GetProcAddress(callib, "calDeviceGetCount" );
    __calDeviceGetAttribs =(ATI_ATTRIBS)GetProcAddress(callib, "calDeviceGetAttribs" );
    __calShutdown = (ATI_CLOSE)GetProcAddress(callib, "calShutdown" );
    __calDeviceGetInfo = (ATI_INFO)GetProcAddress(callib, "calDeviceGetInfo" );
    __calDeviceGetStatus = (ATI_STATUS)GetProcAddress(callib, "calDeviceGetStatus" );
    __calDeviceOpen = (ATI_DEVICEOPEN)GetProcAddress(callib, "calDeviceOpen" );
    __calDeviceClose = (ATI_DEVICECLOSE)GetProcAddress(callib, "calDeviceClose" );

#else

    void* callib;

    callib = dlopen("libaticalrt.so", RTLD_NOW);
    if (!callib) {
        warnings.push_back("No ATI library found");
        return;
    }

    atirt_detected = true;

    __calInit = (int(*)()) dlsym(callib, "calInit");
    __calGetVersion = (int(*)(CALuint*, CALuint*, CALuint*)) dlsym(callib, "calGetVersion");
    __calDeviceGetCount = (int(*)(CALuint*)) dlsym(callib, "calDeviceGetCount");
    __calDeviceGetAttribs = (int(*)(CALdeviceattribs*, CALuint)) dlsym(callib, "calDeviceGetAttribs");
    __calShutdown = (int(*)()) dlsym(callib, "calShutdown");
    __calDeviceGetInfo = (int(*)(CALdeviceinfo*, CALuint)) dlsym(callib, "calDeviceGetInfo");
    __calDeviceGetStatus = (int(*)(CALdevicestatus*, CALdevice)) dlsym(callib, "calDeviceGetStatus");
    __calDeviceOpen = (int(*)(CALdevice*, CALuint)) dlsym(callib, "calDeviceOpen");
    __calDeviceClose = (int(*)(CALdevice)) dlsym(callib, "calDeviceClose");

#endif

    if (!__calInit) {
        warnings.push_back("calInit() missing from CAL library");
        return;
    }
    if (!__calGetVersion) {
        warnings.push_back("calGetVersion() missing from CAL library");
        return;
    }
    if (!__calDeviceGetCount) {
        warnings.push_back("calDeviceGetCount() missing from CAL library");
        return;
    }
    if (!__calDeviceGetAttribs) {
        warnings.push_back("calDeviceGetAttribs() missing from CAL library");
        return;
    }
    if (!__calDeviceGetInfo) {
        warnings.push_back("calDeviceGetInfo() missing from CAL library");
        return;
    }
    if (!__calDeviceGetStatus) {
        warnings.push_back("calDeviceGetStatus() missing from CAL library");
        return;
    }
    if (!__calDeviceOpen) {
        warnings.push_back("calDeviceOpen() missing from CAL library");
        return;
    }
    if (!__calDeviceClose) {
        warnings.push_back("calDeviceClose() missing from CAL library");
        return;
    }

    retval = (*__calInit)();
    if (retval != CAL_RESULT_OK) {
        sprintf(buf, "calInit() returned %d", retval);
        warnings.push_back(buf);
        return;
    }
    retval = (*__calDeviceGetCount)(&numDevices);
    if (retval != CAL_RESULT_OK) {
        sprintf(buf, "calDeviceGetCount() returned %d", retval);
        warnings.push_back(buf);
        return;
    }
    retval = (*__calGetVersion)(&cal_major, &cal_minor, &cal_imp);
    if (retval != CAL_RESULT_OK) {
        sprintf(buf, "calGetVersion() returned %d", retval);
        warnings.push_back(buf);
        return;
    }

    if (!numDevices) {
        warnings.push_back("No usable CAL devices found");
        return;
    }

    COPROC_ATI cc, cc2;
    string s, gpu_name;
    vector<COPROC_ATI> gpus;
    for (CALuint i=0; i<numDevices; i++) {
        retval = (*__calDeviceGetInfo)(&info, i);
        if (retval != CAL_RESULT_OK) {
            sprintf(buf, "calDeviceGetInfo() returned %d", retval);
            warnings.push_back(buf);
            return;
        }
        retval = (*__calDeviceGetAttribs)(&attribs, i);
        if (retval != CAL_RESULT_OK) {
            sprintf(buf, "calDeviceGetAttribs() returned %d", retval);
            warnings.push_back(buf);
            return;
        }
        switch ((int)attribs.target) {
        case CAL_TARGET_600:
            gpu_name="ATI Radeon HD 2900 (RV600)";
            break;
        case CAL_TARGET_610:
            gpu_name="ATI Radeon HD 2300/2400/3200 (RV610)";
            attribs.numberOfSIMD=1;        // set correct values (reported wrong by driver)
            attribs.wavefrontSize=32;
            break;
        case CAL_TARGET_630:
            gpu_name="ATI Radeon HD 2600 (RV630)";
            // set correct values (reported wrong by driver)
            attribs.numberOfSIMD=3;
            attribs.wavefrontSize=32;
            break;
        case CAL_TARGET_670:
            gpu_name="ATI Radeon HD 3800 (RV670)";
            break;
        case CAL_TARGET_710:
            gpu_name="ATI Radeon HD 4350/4550 (R710)";
            break;
        case CAL_TARGET_730:
            gpu_name="ATI Radeon HD 4600 series (R730)";
            break;
        case CAL_TARGET_7XX:
            gpu_name="ATI Radeon (RV700 class)";
            break;
        case CAL_TARGET_770:
            gpu_name="ATI Radeon HD 4700/4800 (RV740/RV770)";
            break;
        case 8:
            gpu_name="ATI Radeon HD5800 series (Cypress)";
            break;
        case 9:
            gpu_name="ATI Radeon HD5700 series (Juniper)";
            break;
        case 10:
            gpu_name="ATI Radeon HD5x00 series (Redwood)";
            break;
        case 11:
            gpu_name="ATI Radeon HD5x00 series (Cedar)";
            break;
        default:
            gpu_name="ATI unknown";
            break;
        }
        cc.attribs = attribs;
        cc.info = info;
        strcpy(cc.name, gpu_name.c_str());
        sprintf(cc.version, "%d.%d.%d", cal_major, cal_minor, cal_imp);
        cc.amdrt_detected = amdrt_detected;
        cc.atirt_detected = atirt_detected;
        cc.device_num = i;
        gpus.push_back(cc);
    }

    // TODO: count only GPUs with as much memory as fastest one,
    // same as for NVIDIA

    COPROC_ATI best;
    bool first = true;
    for (unsigned int i=0; i<gpus.size(); i++) {
        char buf[256], buf2[256];
        gpus[i].description(buf);
        if (in_vector(gpus[i].device_num, ignore_devs)) {
            sprintf(buf2, "ATI GPU %d (ignored by config): %s", gpus[i].device_num, buf);
        } else {
            if (first) {
                best = gpus[i];
                first = false;
            } else if (gpus[i].peak_flops() > best.peak_flops()) {
                best = gpus[i];
            }
            sprintf(buf2, "ATI GPU %d: %s", gpus[i].device_num, buf);
        }
        descs.push_back(buf2);
    }
    best.count = 0;
    for (unsigned int i=0; i<gpus.size(); i++) {
        if (in_vector(gpus[i].device_num, ignore_devs)) continue;
        best.device_nums[best.count] = i;
        best.count++;
    }

    COPROC_ATI* ccp = new COPROC_ATI;
    *ccp = best;
    strcpy(ccp->type, "ATI");
    coprocs.coprocs.push_back(ccp);

    // shut down, otherwise Lenovo won't be able to switch to low-power GPU
    //
    retval = (*__calShutdown)();
}

COPROC_ATI* fake_ati(COPROCS& coprocs, double ram, int count) {
    COPROC_ATI* cc = new COPROC_ATI;
    strcpy(cc->type, "ATI");
    strcpy(cc->version, "1.4.3");
    strcpy(cc->name, "foobar");
    cc->count = count;
    memset(&cc->attribs, 0, sizeof(cc->attribs));
    memset(&cc->info, 0, sizeof(cc->info));
    cc->attribs.localRAM = (int)(ram/MEGA);
    cc->attribs.numberOfSIMD = 32;
    cc->attribs.wavefrontSize = 32;
    cc->attribs.engineClock = 50;
    for (int i=0; i<count; i++) {
        cc->device_nums[i] = i;
    }
    coprocs.coprocs.push_back(cc);
    return cc;
}

void COPROC_ATI::get_available_ram() {
#ifdef MEASURE_AVAILABLE_RAM
    CALdevicestatus st;
    CALdevice dev;
    int i, retval;

    st.struct_size = sizeof(CALdevicestatus);

    // avoid crash if faked GPU
    if (!__calInit) {
        for (i=0; i<count; i++) {
            available_ram[i] = available_ram_fake[i];
            available_ram_unknown[i] = false;
        }
        return;
    }
    for (i=0; i<count; i++) {
        available_ram[i] = 0;
        available_ram_unknown[i] = true;
    }
    retval = (*__calInit)();
    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] calInit() returned %d", retval
            );
        }
        return;
    }

    for (i=0; i<count; i++) {
        int devnum = device_nums[i];
        retval = (*__calDeviceOpen)(&dev, devnum);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] calDeviceOpen(%d) returned %d", devnum, retval
                );
            }
            continue;
        }
        retval = (*__calDeviceGetStatus)(&st, dev);
        if (retval) {
            if (log_flags.coproc_debug) {
                msg_printf(0, MSG_INFO,
                    "[coproc] calDeviceGetStatus(%d) returned %d",
                    devnum, retval
                );
            }
            (*__calDeviceClose)(dev);
            continue;
        }
        available_ram[i] = st.availLocalRAM*MEGA;
        available_ram_unknown[i] = false;
        (*__calDeviceClose)(dev);
    }
    (*__calShutdown)();
#else
    for (int i=0; i<count; i++) {
        available_ram[i] = attribs.localRAM*MEGA;
    }
#endif
}
