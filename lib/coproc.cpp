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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#include "coproc.h"

using std::string;
using std::vector;

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
                cp2->prop.name, cp2->count, mem, cp2->drvVersion
            );
            strcat(bigbuf, buf2);
        }
    }
    bigbuf[len-1] = 0;
    strcpy(buf, bigbuf);
}

vector<string> COPROCS::get(bool use_all) {
    vector<string> strings;
    COPROC_CUDA::get(*this, strings, use_all);
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

// return 1/-1/0 if device 1 is more/less/same capable than device 2
//
int cuda_compare(COPROC_CUDA& c1, COPROC_CUDA& c2, bool ignore_flops) {
    if (c1.prop.major > c2.prop.major) return 1;
    if (c1.prop.major < c2.prop.major) return -1;
    if (c1.prop.minor > c2.prop.minor) return 1;
    if (c1.prop.minor < c2.prop.minor) return -1;
    if (c1.drvVersion > c2.drvVersion) return 1; 
    if (c1.drvVersion < c2.drvVersion) return -1; 
    if (c1.prop.totalGlobalMem > c2.prop.totalGlobalMem) return 1;
    if (c1.prop.totalGlobalMem < c2.prop.totalGlobalMem) return -1;
    if (ignore_flops) return 0;
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
    int count;

#ifdef _WIN32

    typedef int (__stdcall *PCGDC)(int *count);
    typedef int (__stdcall *PCGDP)(struct cudaDeviceProp *prop, int device);

    PCGDC __cudaGetDeviceCount = NULL;
    PCGDP __cudaGetDeviceProperties = NULL;

    HMODULE cudalib = LoadLibrary("cudart.dll");
    if (!cudalib) {
        strings.push_back("Can't load library cudart.dll");
        return;
    }

    __cudaGetDeviceCount = (PCGDC)GetProcAddress( cudalib, "cudaGetDeviceCount" );
    if (!__cudaGetDeviceCount) {
        strings.push_back("Library doesn't have cudaGetDeviceCount()");
        return;
    }

    __cudaGetDeviceProperties = (PCGDP)GetProcAddress( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) {
        strings.push_back("Library doesn't have cudaGetDeviceProperties()");
        return;
    }

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
    void (*__cudaGetDeviceCount)(int*);
    void (*__cudaGetDeviceProperties)(cudaDeviceProp*, int);

#ifdef __APPLE__
    cudalib = dlopen("/usr/local/cuda/lib/libcudart.dylib", RTLD_NOW);
#else
    // libcudart.so is included with the BOINC install for linux,
    // so look for it in the current dir.
    //
    cudalib = dlopen("./libcudart.so", RTLD_NOW);
    if (!cudalib) {
        // If that fails, look for it in the library search path
        //
        cudalib = dlopen("libcudart.so", RTLD_NOW);
    }
#endif
    if (!cudalib) {
        strings.push_back("Can't load library libcudart");
        return;
    }
    __cudaGetDeviceCount = (void(*)(int*)) dlsym(cudalib, "cudaGetDeviceCount");
    if(!__cudaGetDeviceCount) {
        strings.push_back("Library doesn't have cudaGetDeviceCount()");
        return;
    }
    __cudaGetDeviceProperties = (void(*)(cudaDeviceProp*, int)) dlsym( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) {
        strings.push_back("Library doesn't have cudaGetDeviceProperties()");
        return;
    }
#endif

    vector<COPROC_CUDA> gpus;
    (*__cudaGetDeviceCount)(&count);
    int j;
    unsigned int i;
    COPROC_CUDA cc;
    string s;
    for (j=0; j<count; j++) {
        (*__cudaGetDeviceProperties)(&cc.prop, j);
        if (cc.prop.major <= 0) continue;  // major == 0 means emulation
        if (cc.prop.major > 100) continue;  // e.g. 9999 is an error
#if defined(_WIN32) && !defined(SIM)
        cc.drvVersion = Version.drvVersion;
#else
        cc.drvVersion = 0;
#endif
        cc.device_num = j;
        gpus.push_back(cc);
    }

    if (!gpus.size()) {
        strings.push_back("No CUDA devices found");
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
        char buf[256];
		cc.description(buf);
        if (use_all || !cuda_compare(gpus[i], best, true)) {
            best.device_nums[best.count] = gpus[i].device_num;
            best.count++;
            strings.push_back("CUDA device: "+string(buf));
        } else {
            strings.push_back("CUDA device (not used): "+string(buf));
        }
    }

    COPROC_CUDA* ccp = new COPROC_CUDA;
    *ccp = best;
    coprocs.coprocs.push_back(ccp);
}

void COPROC_CUDA::description(char* buf) {
	sprintf(buf, "%s (driver version %d, compute capability %d.%d, %.0fMB, est. %.0fGFLOPS)",
		prop.name, drvVersion, prop.major, prop.minor, prop.totalGlobalMem/(1024.*1024.), flops_estimate()/1e9
	);
}

// add a non-existent CUDA coproc (for debugging)
//
void fake_cuda(COPROCS& coprocs, int count) {
   COPROC_CUDA* cc = new COPROC_CUDA;
   strcpy(cc->type, "CUDA");
   cc->count = count;
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
        drvVersion,
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
        if (parse_int(buf, "<drvVersion>", drvVersion)) continue;
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
