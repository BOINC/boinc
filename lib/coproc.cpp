// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif

#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#ifndef SIM
#include <nvapi.h>
#endif
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#endif
#include <dlfcn.h>
#include <setjmp.h>
#include <signal.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#ifdef _WIN32
#include "win_util.h"
#endif

#include "coproc.h"

using std::string;
using std::vector;

#ifndef _USING_FCGI_
using std::perror;
#endif

#ifndef _USING_FCGI_
void COPROC::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc>\n"
        "   <type>%s</type>\n"
        "   <count>%d</count>\n"
        "</coproc>\n",
        type, count
    );
}
#endif

bool COPROC::is_usable(){
    return true;
}

int COPROC_REQ::parse(MIOFILE& fin) {
    char buf[1024];
    strcpy(type, "");
    count = 0;
    while (fin.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc>")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<type>", type, sizeof(type))) continue;
        if (parse_double(buf, "<count>", count)) continue;
    }
    return ERR_XML_PARSE;
}

int COPROC::parse(MIOFILE& fin) {
    char buf[1024];
    strcpy(type, "");
    count = 0;
    used = 0;
    req_secs = 0;
    estimated_delay = 0;
    req_instances = 0;
    while (fin.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc>")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<type>", type, sizeof(type))) continue;
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;
    }
    return ERR_XML_PARSE;
}

void COPROCS::summary_string(char* buf, int len) {
    char bigbuf[8192], buf2[1024];

    strcpy(bigbuf, "");
    for (unsigned int i=0; i<coprocs.size(); i++) {
        COPROC* cp = coprocs[i];
        if (!strcmp(cp->type, "CUDA")) {
            COPROC_CUDA* cp2 = (COPROC_CUDA*) cp;
            int mem = (int)(cp2->prop.dtotalGlobalMem/MEGA);
            sprintf(buf2, "[CUDA|%s|%d|%dMB|%d]",
                cp2->prop.name, cp2->count, mem, cp2->display_driver_version
            );
            strcat(bigbuf, buf2);
        } else if (!strcmp(cp->type, "ATI")){
            COPROC_ATI* cp2 =(COPROC_ATI*) cp;
            sprintf(buf2,"[CAL|%s|%d|%dMB|%s]",
                cp2->name, cp2->count, cp2->attribs.localRAM, cp2->version
            );
            strcat(bigbuf,buf2);
        }
    }
    bigbuf[len-1] = 0;
    strcpy(buf, bigbuf);
}


#ifdef _WIN32

void COPROCS::get(bool use_all, vector<string>&descs, vector<string>&warnings) {
    COPROC_CUDA::get(*this, use_all, descs, warnings);
    COPROC_ATI::get(*this, descs, warnings);
}

#else

jmp_buf resume;

void segv_handler(int) {
    longjmp(resume, 1);
}

void COPROCS::get(bool use_all, vector<string>&descs, vector<string>&warnings) {
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        COPROC_CUDA::get(*this, use_all, descs, warnings);
    }
#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    } else {
        COPROC_ATI::get(*this, descs, warnings);
    }
#endif
    signal(SIGSEGV, old_sig);
}

#endif

// used only to parse scheduler request messages
//
int COPROCS::parse(FILE* fin) {
    char buf[1024];

    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</coprocs>")) {
            return 0;
        }
        if (strstr(buf, "<coproc_cuda>")) {
            COPROC_CUDA* cc = new COPROC_CUDA;
            int retval = cc->parse(fin);
            if (!retval) {
                coprocs.push_back(cc);
            }
        }
        if (strstr(buf, "<coproc_ati>")) {
            COPROC_ATI* cc = new COPROC_ATI;
            int retval = cc->parse(fin);
            if (!retval) {
                coprocs.push_back(cc);
            }
        }
    }
    return ERR_XML_PARSE;
}

COPROC* COPROCS::lookup(const char* type) {
    for (unsigned int i=0; i<coprocs.size(); i++) {
        COPROC* cp = coprocs[i];
        if (!strcmp(type, cp->type)) return cp;
    }
    return NULL;
}

#ifdef _WIN32


#endif

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
#endif

// NVIDIA interfaces are documented here:
// http://developer.download.nvidia.com/compute/cuda/2_3/toolkit/docs/online/index.html

void COPROC_CUDA::get(
    COPROCS& coprocs,
    bool use_all,    // if false, use only those equivalent to most capable
    vector<string>& descs,
    vector<string>& warnings
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

    // identify the most capable instance
    //
    COPROC_CUDA best;
    for (i=0; i<gpus.size(); i++) {
        if (i==0) {
            best = gpus[i];
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
        if (use_all || !cuda_compare(gpus[i], best, true)) {
            best.device_nums[best.count] = gpus[i].device_num;
            best.count++;
            sprintf(buf2, "NVIDIA GPU %d: %s", gpus[i].device_num, buf);
        } else {
            sprintf(buf2, "NVIDIA GPU %d (not used): %s", gpus[i].device_num, buf);
        }
        descs.push_back(string(buf2));
    }

    COPROC_CUDA* ccp = new COPROC_CUDA;
    *ccp = best;
    coprocs.coprocs.push_back(ccp);
}

bool COPROC_CUDA::is_usable() {
#ifdef _WIN32
    if (is_remote_desktop()) return false;
#endif
    return true;
}

void COPROC_CUDA::description(char* buf) {
    char vers[256];
    if (display_driver_version) {
        sprintf(vers, "%d", display_driver_version);
    } else {
        strcpy(vers, "unknown");
    }
    sprintf(buf, "%s (driver version %s, CUDA version %d, compute capability %d.%d, %.0fMB, %.0f GFLOPS peak)",
        prop.name, vers, cuda_version, prop.major, prop.minor,
        prop.totalGlobalMem/(1024.*1024.), peak_flops()/1e9
    );
}

// fake a NVIDIA GPU (for debugging)
//
void fake_cuda(COPROCS& coprocs, int count) {
   COPROC_CUDA* cc = new COPROC_CUDA;
   strcpy(cc->type, "CUDA");
   cc->count = count;
   for (int i=0; i<count; i++) {
       cc->device_nums[i] = i;
   }
   cc->display_driver_version = 18000;
   cc->cuda_version = 2020;
   strcpy(cc->prop.name, "Fake NVIDIA GPU");
   cc->prop.totalGlobalMem = 256*1024*1024;
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
}

#ifndef _USING_FCGI_
void COPROC_CUDA::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc_cuda>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <req_secs>%f</req_secs>\n"
        "   <req_instances>%f</req_instances>\n"
        "   <estimated_delay>%f</estimated_delay>\n"
        "   <drvVersion>%d</drvVersion>\n"
        "   <cudaVersion>%d</cudaVersion>\n"
        "   <totalGlobalMem>%u</totalGlobalMem>\n"
        "   <sharedMemPerBlock>%u</sharedMemPerBlock>\n"
        "   <regsPerBlock>%d</regsPerBlock>\n"
        "   <warpSize>%d</warpSize>\n"
        "   <memPitch>%u</memPitch>\n"
        "   <maxThreadsPerBlock>%d</maxThreadsPerBlock>\n"
        "   <maxThreadsDim>%d %d %d</maxThreadsDim>\n"
        "   <maxGridSize>%d %d %d</maxGridSize>\n"
        "   <totalConstMem>%u</totalConstMem>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <clockRate>%d</clockRate>\n"
        "   <textureAlignment>%u</textureAlignment>\n"
        "   <deviceOverlap>%d</deviceOverlap>\n"
        "   <multiProcessorCount>%d</multiProcessorCount>\n"
        "</coproc_cuda>\n",
        count,
        prop.name,
        req_secs,
        req_instances,
        estimated_delay,
        display_driver_version,
        cuda_version,
        (unsigned int)prop.totalGlobalMem,
        (unsigned int)prop.sharedMemPerBlock,
        prop.regsPerBlock,
        prop.warpSize,
        (unsigned int)prop.memPitch,
        prop.maxThreadsPerBlock,
        prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2],
        prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2],
        (unsigned int)prop.totalConstMem,
        prop.major,
        prop.minor,
        prop.clockRate,
        (unsigned int)prop.textureAlignment,
        prop.deviceOverlap,
        prop.multiProcessorCount
    );
}
#endif

void COPROC_CUDA::clear() {
    count = 0;
    used = 0;
    req_secs = 0;
    req_instances = 0;
    estimated_delay = -1;   // mark as absent
    cuda_version = 0;
    display_driver_version = 0;
    strcpy(prop.name, "");
    prop.totalGlobalMem = 0;
    prop.sharedMemPerBlock = 0;
    prop.regsPerBlock = 0;
    prop.warpSize = 0;
    prop.memPitch = 0;
    prop.maxThreadsPerBlock = 0;
    prop.maxThreadsDim[0] = 0;
    prop.maxThreadsDim[1] = 0;
    prop.maxThreadsDim[2] = 0;
    prop.maxGridSize[0] = 0;
    prop.maxGridSize[1] = 0;
    prop.maxGridSize[2] = 0;
    prop.clockRate = 0;
    prop.totalConstMem = 0;
    prop.major = 0;
    prop.minor = 0;
    prop.textureAlignment = 0;
    prop.deviceOverlap = 0;
    prop.multiProcessorCount = 0;
}

int COPROC_CUDA::parse(FILE* fin) {
    char buf[1024], buf2[256];

    clear();
    while (fgets(buf, sizeof(buf), fin)) {
        if (strstr(buf, "</coproc_cuda>")) {
            return 0;
        }
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;
        if (parse_str(buf, "<name>", prop.name, sizeof(prop.name))) continue;
        if (parse_int(buf, "<drvVersion>", display_driver_version)) continue;
        if (parse_int(buf, "<cudaVersion>", cuda_version)) continue;
        if (parse_double(buf, "<totalGlobalMem>", prop.dtotalGlobalMem)) continue;
        if (parse_int(buf, "<sharedMemPerBlock>", (int&)prop.sharedMemPerBlock)) continue;
        if (parse_int(buf, "<regsPerBlock>", prop.regsPerBlock)) continue;
        if (parse_int(buf, "<warpSize>", prop.warpSize)) continue;
        if (parse_int(buf, "<memPitch>", (int&)prop.memPitch)) continue;
        if (parse_int(buf, "<maxThreadsPerBlock>", prop.maxThreadsPerBlock)) continue;
        if (parse_str(buf, "<maxThreadsDim>", buf2, sizeof(buf2))) {
            // can't use sscanf here (FCGI)
            //
            prop.maxThreadsDim[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                p++;
                prop.maxThreadsDim[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    p++;
                    prop.maxThreadsDim[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_str(buf, "<maxGridSize>", buf2, sizeof(buf2))) {
            prop.maxGridSize[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                p++;
                prop.maxGridSize[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    p++;
                    prop.maxGridSize[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_int(buf, "<clockRate>", prop.clockRate)) continue;
        if (parse_int(buf, "<totalConstMem>", (int&)prop.totalConstMem)) continue;
        if (parse_int(buf, "<major>", prop.major)) continue;
        if (parse_int(buf, "<minor>", prop.minor)) continue;
        if (parse_int(buf, "<textureAlignment>", (int&)prop.textureAlignment)) continue;
        if (parse_int(buf, "<deviceOverlap>", prop.deviceOverlap)) continue;
        if (parse_int(buf, "<multiProcessorCount>", prop.multiProcessorCount)) continue;
    }
    return ERR_XML_PARSE;
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

ATI_ATTRIBS __calDeviceGetAttribs = NULL;
ATI_CLOSE   __calShutdown = NULL;
ATI_GDC     __calDeviceGetCount = NULL;
ATI_GDI     __calInit = NULL;
ATI_INFO    __calDeviceGetInfo = NULL;
ATI_VER     __calGetVersion = NULL;
#else
int (*__calInit)();
int (*__calGetVersion)(CALuint*, CALuint*, CALuint*);
int (*__calDeviceGetCount)(CALuint*);
int (*__calDeviceGetAttribs)(CALdeviceattribs*, CALuint);
int (*__calShutdown)();
int (*__calDeviceGetInfo)(CALdeviceinfo*, CALuint);
#endif

void COPROC_ATI::get(COPROCS& coprocs,
    vector<string>& descs, vector<string>& warnings
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
    for (unsigned int i=0; i<gpus.size(); i++) {
        char buf[256], buf2[256];
        if (i == 0) {
            best = gpus[i];
        } else if (gpus[i].peak_flops() > best.peak_flops()) {
            best = gpus[i];
        }
        gpus[i].description(buf);
        sprintf(buf2, "ATI GPU %d: %s", gpus[i].device_num, buf);
        descs.push_back(buf2);
    }
    for (unsigned int i=0; i<gpus.size(); i++) {
        best.device_nums[i] = i;
    }

    COPROC_ATI* ccp = new COPROC_ATI;
    *ccp = best;
    strcpy(ccp->type, "ATI");
    ccp->count = numDevices;
    coprocs.coprocs.push_back(ccp);
}

bool COPROC_ATI::is_usable() {
#ifdef _WIN32
    if (is_remote_desktop()) return false;
#endif
    return true;
}

#ifndef _USING_FCGI_
void COPROC_ATI::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc_ati>\n"
    );

    f.printf(
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <req_secs>%f</req_secs>\n"
        "   <req_instances>%f</req_instances>\n"
        "   <estimated_delay>%f</estimated_delay>\n"
        "   <target>%d</target>\n"
        "   <localRAM>%d</localRAM>\n"
        "   <uncachedRemoteRAM>%d</uncachedRemoteRAM>\n"
        "   <cachedRemoteRAM>%d</cachedRemoteRAM>\n"
        "   <engineClock>%u</engineClock>\n"
        "   <memoryClock>%d</memoryClock>\n"
        "   <wavefrontSize>%d</wavefrontSize>\n"
        "   <numberOfSIMD>%d</numberOfSIMD>\n"
        "   <doublePrecision>%d</doublePrecision>\n"
        "   <pitch_alignment>%d</pitch_alignment>\n"
        "   <surface_alignment>%d</surface_alignment>\n"
        "   <maxResource1DWidth>%d</maxResource1DWidth>\n"
        "   <maxResource2DWidth>%d</maxResource2DWidth>\n"
        "   <maxResource2DHeight>%d</maxResource2DHeight>\n"
        "   <CALVersion>%s</CALVersion>\n",
        count,
        name,
        req_secs,
        req_instances,
        estimated_delay,
        attribs.target,
        attribs.localRAM,
        attribs.uncachedRemoteRAM,
        attribs.cachedRemoteRAM,
        attribs.engineClock,
        attribs.memoryClock,
        attribs.wavefrontSize,
        attribs.numberOfSIMD,
        attribs.doublePrecision,
        attribs.pitch_alignment,
        attribs.surface_alignment,
        info.maxResource1DWidth,
        info.maxResource2DWidth,
        info.maxResource2DHeight,
        version
    );

    if (atirt_detected) {
        f.printf("    <atirt_detected/>\n");
    }

    if (amdrt_detected) {
        f.printf("    <amdrt_detected/>\n");
    }

    f.printf("</coproc_ati>\n");
};
#endif

void COPROC_ATI::clear() {
    count = 0;
    used = 0;
    req_secs = 0;
    req_instances = 0;
    estimated_delay = -1;
    strcpy(name, "");
    strcpy(version, "");
    atirt_detected = false;
    amdrt_detected = false;
    memset(&attribs, 0, sizeof(attribs));
    memset(&info, 0, sizeof(info));
}

int COPROC_ATI::parse(FILE* fin) {
    char buf[1024];
    int n;

    clear();

    while (fgets(buf, sizeof(buf), fin)) {
        if (strstr(buf, "</coproc_ati>")) return 0;
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;

        if (parse_int(buf, "<target>", n)) {
            attribs.target = (CALtarget)n;
            continue;
        }
        if (parse_int(buf, "<localRAM>", n)) {
            attribs.localRAM = n;
            continue;
        }
        if (parse_int(buf, "<uncachedRemoteRAM>", n)) {
            attribs.uncachedRemoteRAM = n;
            continue;
        }
        if (parse_int(buf, "<cachedRemoteRAM>", n)) {
            attribs.cachedRemoteRAM = n;
            continue;
        }
        if (parse_int(buf, "<engineClock>", n)) {
            attribs.engineClock = n;
            continue;
        }
        if (parse_int(buf, "<memoryClock>", n)) {
            attribs.memoryClock = n;
            continue;
        }
        if (parse_int(buf, "<wavefrontSize>", n)) {
            attribs.wavefrontSize = n;
            continue;
        }
        if (parse_int(buf, "<numberOfSIMD>"  , n)) {
            attribs.numberOfSIMD = n;
            continue;
        }
        if (parse_int(buf, "<doublePrecision>", n)) {
            attribs.doublePrecision = n?CAL_TRUE:CAL_FALSE;
            continue;
        }
        if (parse_int(buf, "<pitch_alignment>", n)) {
            attribs.pitch_alignment = n;
            continue;
        }
        if (parse_int(buf, "<surface_alignment>", n)) {
            attribs.surface_alignment = n;
            continue;
        }
        if (parse_int(buf, "<maxResource1DWidth>", n)) {
            info.maxResource1DWidth = n;
            continue;
        }
        if (parse_int(buf, "<maxResource2DWidth>", n)) {
            info.maxResource2DWidth = n;
            continue;
        }
        if (parse_int(buf, "<maxResource2DHeight>", n)) {
            info.maxResource2DHeight = n;
            continue;
        }
        if (parse_bool(buf, "amdrt_detected", amdrt_detected)) continue;
        if (parse_bool(buf, "atirt_detected", atirt_detected)) continue;
        if (parse_str(buf, "<CALVersion>", version, sizeof(version))) continue;
    }
    return ERR_XML_PARSE;
}

void COPROC_ATI::description(char* buf) {
    sprintf(buf, "%s (CAL version %s, %.0fMB, %.0f GFLOPS peak)",
        name, version, attribs.localRAM/1024.*1024., peak_flops()/1.e9
    );
}

void fake_ati(COPROCS& coprocs, int count) {
    COPROC_ATI* cc = new COPROC_ATI;
    strcpy(cc->type, "ATI");
    strcpy(cc->version, "1.4.3");
    cc->count = count;
    cc->attribs.numberOfSIMD = 32;
    cc->attribs.wavefrontSize = 32;
    cc->attribs.engineClock = 500;
    for (int i=0; i<count; i++) {
        cc->device_nums[i] = i;
    }
    coprocs.coprocs.push_back(cc);
}
