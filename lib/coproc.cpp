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
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "cal.h"

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
        if (parse_int(buf, "<req_instances>", req_instances)) continue;
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
        } else if (!strcmp(cp->type, "CAL")){
		    COPROC_CAL* cp2 =(COPROC_CAL*) cp;
		    sprintf(buf2,"[CAL|%s|%d|%dMB|%s]",
                cp2->prop.name,cp2->count,cp2->prop.localRAM,cp2->prop.version
            );
		    strcat(bigbuf,buf2); 
		}
    }
    bigbuf[len-1] = 0;
    strcpy(buf, bigbuf);
}

vector<string> COPROCS::get(bool use_all) {
    vector<string> strings;
    COPROC_CUDA::get(*this, strings, use_all);

    string s = COPROC_CAL::get(*this);
    if (s.size()) strings.push_back(s);

    return strings;
}

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
        if (strstr(buf, "<coproc_cal>")) {
            COPROC_CAL* cc = new COPROC_CAL;
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
    double s1 = c1.flops_estimate();
    double s2 = c2.flops_estimate();
    if (s1 > s2) return 1;
    if (s1 < s2) return -1;
    return 0;
}

void COPROC_CUDA::get(
    COPROCS& coprocs, vector<string>& strings,
    bool use_all    // if false, use only those equivalent to most capable
) {
    int count, retval;

#ifdef _WIN32

    typedef int (__stdcall *PCGDC)(int *count);
    typedef int (__stdcall *PCGDP)(struct cudaDeviceProp *prop, int device);
    typedef int (__stdcall *PCGDV)(int* version);
    typedef int (__stdcall *PCGDI)(int);
    typedef int (__stdcall *PCGDG)(int*, int);
    typedef int (__stdcall *PCGDA)(int*, int, int);
    typedef int (__stdcall *PCGDN)(char*, int, int);
    typedef int (__stdcall *PCGDM)(unsigned int*, int);
    typedef int (__stdcall *PCGDCC)(int*, int*, int);

    PCGDC __cuDeviceGetCount = NULL;
    PCGDP __cuDeviceGetProperties = NULL;
    PCGDV __cuDriverGetVersion = NULL;
    PCGDI __cuInit = NULL;
    PCGDG __cuDeviceGet = NULL;
    PCGDA __cuDeviceGetAttribute = NULL;
    PCGDN __cuDeviceGetName = NULL;
    PCGDM __cuDeviceTotalMem = NULL;
    PCGDCC __cuDeviceComputeCapability = NULL;

    HMODULE cudalib = LoadLibrary("nvcuda.dll");
    if (!cudalib) {
        strings.push_back("Can't load library nvcuda.dll");
        return;
    }

    __cuDeviceGetCount = (PCGDC)GetProcAddress(cudalib, "cuDeviceGetCount");
    __cuDeviceGetProperties = (PCGDP)GetProcAddress(cudalib, "cuDeviceGetProperties");
    __cuDriverGetVersion = (PCGDV)GetProcAddress(cudalib, "cuDriverGetVersion" );
    __cuInit = (PCGDI)GetProcAddress(cudalib, "cuInit" );
    __cuDeviceGet = (PCGDG)GetProcAddress(cudalib, "cuDeviceGet" );
    __cuDeviceGetAttribute = (PCGDA)GetProcAddress(cudalib, "cuDeviceGetAttribute" );
    __cuDeviceGetName = (PCGDN)GetProcAddress(cudalib, "cuDeviceGetName" );
    __cuDeviceTotalMem = (PCGDM)GetProcAddress(cudalib, "cuDeviceTotalMem" );
    __cuDeviceComputeCapability = (PCGDCC)GetProcAddress(cudalib, "cuDeviceComputeCapability" );

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
    void* cudalib;
    int (*__cuInit)(int);
    int (*__cuDeviceGetCount)(int*);
    int (*__cuDeviceGetProperties)(cudaDeviceProp*, int);
    int (*__cuDriverGetVersion)(int*);
    int (*__cuDeviceGet)(int*, int);
    int (*__cuDeviceGetAttribute)(int*, int, int);
    int (*__cuDeviceGetName)(char*, int, int);
    int (*__cuDeviceTotalMem)(unsigned int*, int);
    int (*__cuDeviceComputeCapability)(int*, int*, int);

#ifdef __APPLE__
    cudalib = dlopen("/usr/local/cuda/lib/libcuda.dylib", RTLD_NOW);
#else
    cudalib = dlopen("libcuda.so", RTLD_NOW);
#endif
    if (!cudalib) {
        strings.push_back("Can't load library libcuda");
        return;
    }
    __cuDeviceGetCount = (int(*)(int*)) dlsym(cudalib, "cuDeviceGetCount");
    __cuDeviceGetProperties = (int(*)(cudaDeviceProp*, int)) dlsym( cudalib, "cuDeviceGetProperties" );
    __cuDriverGetVersion = (int(*)(int*)) dlsym( cudalib, "cuDriverGetVersion" );
    __cuInit = (int(*)(int)) dlsym( cudalib, "cuInit" );
    __cuDeviceGet = (int(*)(int*, int)) dlsym( cudalib, "cuDeviceGet" );
    __cuDeviceGetAttribute = (int(*)(int*, int, int)) dlsym( cudalib, "cuDeviceGetAttribute" );
    __cuDeviceGetName = (int(*)(char*, int, int)) dlsym( cudalib, "cuDeviceGetName" );
    __cuDeviceTotalMem = (int(*)(unsigned int*, int)) dlsym( cudalib, "cuDeviceTotalMem" );
    __cuDeviceComputeCapability = (int(*)(int*, int*, int)) dlsym( cudalib, "cuDeviceComputeCapability" );
#endif

#ifdef __APPLE__
    if (!__cuDriverGetVersion) {
        strings.push_back("CUDA driver is out of date.  Please install CUDA driver 2.3 or later.");
        return;
    }
#endif

    retval = (*__cuInit)(0);

    int cuda_version;
    retval = (*__cuDriverGetVersion)(&cuda_version);

    vector<COPROC_CUDA> gpus;
    retval = (*__cuDeviceGetCount)(&count);
    int j;
    unsigned int i;
    COPROC_CUDA cc;
    string s;
    for (j=0; j<count; j++) {
        memset(&cc.prop, 0, sizeof(cc.prop));
        int device;
        retval = (*__cuDeviceGet)(&device, j);
        (*__cuDeviceGetName)(cc.prop.name, 256, device);
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
        strings.push_back("No CUDA-capable NVIDIA GPUs found");
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
        char buf[256], buf2[256];
        cc.description(buf);
        if (use_all || !cuda_compare(gpus[i], best, true)) {
            best.device_nums[best.count] = gpus[i].device_num;
            best.count++;
            sprintf(buf2, "NVIDIA GPU %d: %s", gpus[i].device_num, buf);
        } else {
            sprintf(buf2, "NVIDIA GPU %d (not used): %s", gpus[i].device_num, buf);
        }
        strings.push_back(string(buf2));
    }

    COPROC_CUDA* ccp = new COPROC_CUDA;
    *ccp = best;
    coprocs.coprocs.push_back(ccp);
}

void COPROC_CUDA::description(char* buf) {
    sprintf(buf, "%s (driver version %d, CUDA version %d, compute capability %d.%d, %.0fMB, est. %.0fGFLOPS)",
        prop.name, display_driver_version, cuda_version, prop.major, prop.minor, prop.totalGlobalMem/(1024.*1024.), flops_estimate()/1e9
    );
}

// add a non-existent CUDA coproc (for debugging)
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
   strcpy(cc->prop.name, "CUDA NVIDIA chip");
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
        "   <req_instances>%d</req_instances>\n"
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
        if (parse_int(buf, "<req_instances>", req_instances)) continue;
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

string COPROC_CAL::get(COPROCS& coprocs) {

CALuint numDevices,cal_major,cal_minor,cal_imp;
CALdevice device;
CALdeviceinfo info;
CALdeviceattribs attribs;
attribs.struct_size = sizeof(CALdeviceattribs);
device = 0;
numDevices =0;

typedef int (__stdcall *PCGDC)(CALuint *numDevices);
typedef int (__stdcall *ATTRIBS) (CALdeviceattribs *attribs, CALuint ordinal);
typedef int (__stdcall *INFO) (CALdeviceinfo *info, CALuint ordinal);
typedef int (__stdcall *VER) (CALuint *cal_major, CALuint *cal_minor, CALuint *cal_imp);
typedef int (__stdcall *PCGDI)(void);
typedef int (__stdcall *CLOSE)(void);

	PCGDI	__calInit = NULL;
	VER		__calGetVersion = NULL;
    PCGDC	__calDeviceGetCount = NULL;
	ATTRIBS __calDeviceGetAttribs = NULL;
	INFO    __calDeviceGetInfo = NULL;
	CLOSE	__calShutdown = NULL;

#if defined _M_X64


HINSTANCE callib = LoadLibrary("aticalrt64.dll");
HINSTANCE callib2 = LoadLibrary("amdcalrt64.dll");
if (!callib==NULL) // TRY CAL 1.4 first driver > 9.2
{
__calInit = (PCGDI)GetProcAddress(callib, "calInit" );
__calGetVersion = (VER)GetProcAddress(callib, "calGetVersion" );
__calDeviceGetInfo = (INFO)GetProcAddress(callib, "calDeviceGetInfo" );
__calDeviceGetCount = (PCGDC)GetProcAddress(callib, "calDeviceGetCount" );
__calDeviceGetAttribs =(ATTRIBS)GetProcAddress(callib, "calDeviceGetAttribs" );
__calShutdown = (CLOSE)GetProcAddress(callib, "calShutdown" );
(*__calInit)(); // INIT CAL
(*__calDeviceGetCount)(&numDevices); //QUERY NUM DEVICES
(*__calGetVersion)(&cal_major,&cal_minor,&cal_imp); // CAL VERSION, IMPORTANT TO KNOW FOR PROJECTS!  
}
else
{
if (!callib2==NULL)
{
__calInit = (PCGDI)GetProcAddress(callib2, "calInit" );
__calDeviceGetCount = (PCGDC)GetProcAddress(callib2, "calDeviceGetCount" );
__calGetVersion = (VER)GetProcAddress(callib2, "calGetVersion" );
__calDeviceGetInfo = (INFO)GetProcAddress(callib2, "calDeviceGetInfo" );
__calDeviceGetAttribs =(ATTRIBS)GetProcAddress(callib2, "calDeviceGetAttribs" );
__calShutdown = (CLOSE)GetProcAddress(callib2, "calShutdown" );
(*__calInit)(); // INIT CAL 
(*__calDeviceGetCount)(&numDevices); //QUERY NUM DEVICES
(*__calGetVersion)(&cal_major,&cal_minor,&cal_imp); // VERSION  
}
else
{
return "No CAL Runtime Libraries installed."; 
}
} 

#else
HINSTANCE callib = LoadLibrary("aticalrt.dll");
HINSTANCE callib2 = LoadLibrary("amdcalrt.dll");
if (!callib==NULL) // TRY CAL 1.4 first driver > 9.2
{
__calInit = (PCGDI)GetProcAddress(callib, "calInit" );
__calGetVersion = (VER)GetProcAddress(callib, "calGetVersion" );
__calDeviceGetInfo = (INFO)GetProcAddress(callib, "calDeviceGetInfo" );
__calDeviceGetCount = (PCGDC)GetProcAddress(callib, "calDeviceGetCount" );
__calDeviceGetAttribs =(ATTRIBS)GetProcAddress(callib, "calDeviceGetAttribs" );
__calShutdown = (CLOSE)GetProcAddress(callib, "calShutdown" );
(*__calInit)(); // INIT CAL
(*__calDeviceGetCount)(&numDevices); //QUERY NUM DEVICES
(*__calGetVersion)(&cal_major,&cal_minor,&cal_imp); // VERSION  
}
else
{
if (!callib2==NULL)
{
__calInit = (PCGDI)GetProcAddress(callib2, "calInit" );
__calDeviceGetCount = (PCGDC)GetProcAddress(callib2, "calDeviceGetCount" );
__calGetVersion = (VER)GetProcAddress(callib2, "calGetVersion" );
__calDeviceGetInfo = (INFO)GetProcAddress(callib2, "calDeviceGetInfo" );
__calDeviceGetAttribs =(ATTRIBS)GetProcAddress(callib2, "calDeviceGetAttribs" );
__calShutdown = (CLOSE)GetProcAddress(callib2, "calShutdown" );
(*__calInit)(); // INIT CAL 
(*__calDeviceGetCount)(&numDevices); //QUERY NUM DEVICES
(*__calGetVersion)(&cal_major,&cal_minor,&cal_imp); // VERSION  
}
else
{
return "No CAL Runtime Libraries installed."; 
}
} 

#endif
  
int real_count = 0; // == numDevices
COPROC_CAL cc, cc2;
string s,CALDEVNAME;
if(!numDevices)
	{
return "No useable CAL devices found"; //NO USABLE CAL DEVICES, USUALLY TO OLD DRIVERS
  	}
else
{
if (numDevices == 1) // ONLY ONE GPU
{
(*__calDeviceGetInfo)(&info, 0); //DEVICE: GPU0
(*__calDeviceGetAttribs)(&attribs, 0);	
 cc2 = cc;
switch (info.target) //attribs.target would work too
   {
    case CAL_TARGET_600:
	CALDEVNAME="ATI GPU RV600";
	break;
    case CAL_TARGET_610:
	CALDEVNAME="ATI GPU RV610";
	break;
	case CAL_TARGET_630:
	CALDEVNAME="ATI GPU RV630";
	break;
    case CAL_TARGET_670:
	CALDEVNAME="ATI GPU RV670";
	break;
   case CAL_TARGET_710:
    CALDEVNAME="ATI GPU R710";
    break;
   case CAL_TARGET_730:
    CALDEVNAME="ATI GPU R730";
    break;
   case CAL_TARGET_7XX:
    CALDEVNAME="ATI GPU R7XX";
    break;
   case CAL_TARGET_770:
    CALDEVNAME="ATI GPU RV770";
    break;
   default:
    CALDEVNAME="ATI unknown";
    break;
	}
s+="";
s+=CALDEVNAME;
} //IF ==1
else 
{
for (CALuint i = 0;i <numDevices; i++)
{
(*__calDeviceGetInfo)(&info, i);	
(*__calDeviceGetAttribs)(&attribs, i);	
if (cc.flops() > cc2.flops()) {
    cc2 = cc;
   }
switch (info.target)
   {
    case CAL_TARGET_600:
	CALDEVNAME="ATI GPU RV600";
	break;
    case CAL_TARGET_610:
	CALDEVNAME="ATI GPU RV610";
	break;
	case CAL_TARGET_630:
	CALDEVNAME="ATI GPU RV630";
	break;
    case CAL_TARGET_670:
	CALDEVNAME="ATI GPU RV670";
	break;
   case CAL_TARGET_710:
    CALDEVNAME="ATI GPU R710";
    break;
   case CAL_TARGET_730:
    CALDEVNAME="ATI GPU R730";
    break;
   case CAL_TARGET_7XX:
    CALDEVNAME="ATI GPU R7XX";
    break;
   case CAL_TARGET_770:
    CALDEVNAME="ATI GPU RV770";
    break;
   default:
    CALDEVNAME="ATI unknown";
    break;
	}
s += ", ";
s+=CALDEVNAME;
}// FOR
} //ELSE
} // ELSE
COPROC_CAL* ccp = new COPROC_CAL;
*ccp = cc2;
real_count = numDevices;
char buffer[50];
sprintf(buffer,"%d.%d.%d",cal_major,cal_minor,cal_imp);
string calversion;
calversion = reinterpret_cast<char *>(buffer);
strcpy(ccp->prop.version,calversion.c_str());
strcpy(ccp->prop.name,CALDEVNAME.c_str());
strcpy(ccp->type, "CAL");
ccp->count = numDevices;
ccp->prop.engineClock=attribs.engineClock;
ccp->prop.localRAM=attribs.localRAM; 
ccp->prop.uncachedRemoteRAM=attribs.uncachedRemoteRAM;
ccp->prop.cachedRemoteRAM=attribs.cachedRemoteRAM;
ccp->prop.memoryClock=attribs.memoryClock;
ccp->prop.wavefrontSize=attribs.wavefrontSize;
ccp->prop.numberOfSIMD=attribs.numberOfSIMD;
ccp->prop.doublePrecision=attribs.doublePrecision;
ccp->prop.pitch_alignment=attribs.pitch_alignment;
ccp->prop.surface_alignment=attribs.surface_alignment;
coprocs.coprocs.push_back(ccp);
 if (numDevices == 1) {
return "CAL device: "+s;
} else {
return "CAL devices: "+s;
}
__calShutdown(); // CLOSE CAL
}//CAL_DEVICE

#ifndef _USING_FCGI_
void COPROC_CAL::write_xml(MIOFILE& f) {
	f.printf(
  "<coproc_cal>\n"
  "   <count>%d</count>\n"
  "   <name>%s</name>\n"
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
  "   <CALVersion>%s</CALVersion>\n"
  "</coproc_cal>\n",
count,
prop.name,
prop.localRAM, 
prop.uncachedRemoteRAM,
prop.cachedRemoteRAM,
prop.engineClock,
prop.memoryClock,
prop.wavefrontSize,
prop.numberOfSIMD,
prop.doublePrecision,
prop.pitch_alignment,
prop.surface_alignment,
prop.version
   );
};
#endif

void COPROC_CAL::clear() {
	count = 0;
	strcpy(prop.name, "");
	strcpy(prop.version, "");
	prop.localRAM = 0;
	prop.uncachedRemoteRAM = 0;
	prop.cachedRemoteRAM = 0;
	prop.engineClock = 0;
	prop.memoryClock = 0;
	prop.wavefrontSize = 0;
	prop.numberOfSIMD = 0;
	prop.doublePrecision = 0;
	prop.pitch_alignment = 0;
	prop.surface_alignment = 0;
	}

int COPROC_CAL::parse(FILE* fin) {
    char buf[1024];//, buf2[256];

    clear();
    while (fgets(buf, sizeof(buf), fin)) {
        if (strstr(buf, "</coproc_cal>")) return 0;
        if (parse_int(buf, "<count>", count)) continue;
		if (parse_str(buf, "<name>", prop.name, sizeof(prop.name))) continue;
		if (parse_int(buf, "<localRAM>",prop.localRAM))continue;
		if (parse_int(buf, "<uncachedRemoteRAM>",prop.uncachedRemoteRAM))  continue;
		if (parse_int(buf, "<cachedRemoteRAM>",prop.cachedRemoteRAM))continue;
		if (parse_int(buf, "<engineClock>",prop.engineClock))continue;
		if (parse_int(buf, "<memoryClock>",prop.memoryClock))continue;
		if (parse_int(buf, "<wavefrontSize>",prop.wavefrontSize))continue;
		if (parse_int(buf, "<numberOfSIMD>"  ,prop.numberOfSIMD))continue;
		if (parse_int(buf, "<doublePrecision>",prop.doublePrecision))continue;
		if (parse_int(buf, "<pitch_alignment>",prop.pitch_alignment))continue;
		if (parse_int(buf, "<surface_alignment>",prop.surface_alignment))continue;
		if (parse_str(buf, "<CALVersion>", prop.version, sizeof(prop.version))) continue;

    }
    return ERR_XML_PARSE;
}