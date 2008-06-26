// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
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

#include "coproc.h"

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
    while (fin.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc>")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<type>", type, sizeof(type))) continue;
        if (parse_int(buf, "<count>", count)) continue;
    }
    return ERR_XML_PARSE;
}

const char* COPROCS::get() {
    const char* p = COPROC_CUDA::get(*this);
    if (p) return p;
    COPROC_CELL_SPE::get(*this);
    return NULL;
}

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

COPROC* COPROCS::lookup(char* type) {
    for (unsigned int i=0; i<coprocs.size(); i++) {
        COPROC* cp = coprocs[i];
        if (!strcmp(type, cp->type)) return cp;
    }
    return NULL;
}

const char* COPROC_CUDA::get(COPROCS& coprocs) {
    int count;

#ifdef _WIN32
    int retval;
    int (__stdcall* __cudaGetDeviceCount)(int*);
    int (__stdcall* __cudaGetDeviceProperties)(cudaDeviceProp*, int);
    int bufsize=256;
    char buf[256], path[256];
    HMODULE cudalib = LoadLibrary("nvcuda.dll");
    if (!cudalib) {
        return "Can't load library nvcuda.dll";
    }
    __cudaGetDeviceCount = (int(__stdcall*)(int*)) GetProcAddress(
        cudalib, "cudaGetDeviceCount"
    );
    if(!__cudaGetDeviceCount) {
        return "Library doesn't have cudaGetDeviceCount()";
    }
    __cudaGetDeviceProperties = (int(__stdcall*)(cudaDeviceProp*, int)) GetProcAddress( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) {
        return "Library doesn't have cudaGetDeviceProperties()";
    }
#else
    void* cudalib;
    void (*__cudaGetDeviceCount)(int*);
    void (*__cudaGetDeviceProperties)(cudaDeviceProp*, int);

    if (!boinc_file_exists("/usr/lib64/libcuda.so")
        && !boinc_file_exists("/usr/lib/libcuda.so")
    ){
        return "No CUDA driver found";
    }
#ifdef __APPLE__
    cudalib = dlopen ("libcudart.dylib", RTLD_NOW );
#else
    cudalib = dlopen ("libcudart.so", RTLD_NOW );
#endif
    if (!cudalib) {
        return "Can't load library libcudart";
    }
    __cudaGetDeviceCount = (void(*)(int*)) dlsym(cudalib, "cudaGetDeviceCount");
    if(!__cudaGetDeviceCount) {
        return "Library doesn't have cudaGetDeviceCount()";
    }
    __cudaGetDeviceProperties = (void(*)(cudaDeviceProp*, int)) dlsym( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) {
        return "Library doesn't have cudaGetDeviceProperties()";
    }
#endif
    (*__cudaGetDeviceCount)(&count);
    if (count < 1) {
        return "No CUDA devices found";
    }

    for (int i=0; i<count; i++) {
        COPROC_CUDA* cc = new COPROC_CUDA;
        (*__cudaGetDeviceProperties)(&cc->prop, i);
        cc->count = 1;
        strcpy(cc->type, "CUDA");
        coprocs.coprocs.push_back(cc);
    }
    return 0;
}

// add a non-existent CUDA coproc (for debugging)
//
void fake_cuda(COPROCS& coprocs) {
   COPROC_CUDA* cc = new COPROC_CUDA;
   strcpy(cc->type, "CUDA");
   cc->count = 1;
   strcpy(cc->prop.name, "CUDA NVIDIA chip");
   cc->prop.totalGlobalMem = 1000;
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
   cc->prop.minor = 1;
   cc->prop.clockRate = 10000;
   cc->prop.textureAlignment = 1000;
   coprocs.coprocs.push_back(cc);
}

#ifndef _USING_FCGI_
void COPROC_CUDA::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc_cuda>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
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
        "</coproc_cuda>\n",
        count,
        prop.name,
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
        (unsigned int)prop.textureAlignment
    );
}
#endif

void COPROC_CUDA::clear() {
    count = 0;
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
    prop.totalConstMem = 0;
    prop.major = 0;
    prop.minor = 0;
    prop.clockRate = 0;
    prop.textureAlignment = 0;
}

int COPROC_CUDA::parse(FILE* fin) {
    char buf[1024], buf2[256];

    clear();
    while (fgets(buf, sizeof(buf), fin)) {
        if (strstr(buf, "</coproc_cuda>")) return 0;
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_str(buf, "<name>", prop.name, sizeof(prop.name))) continue;
        if (parse_int(buf, "<totalGlobalMem`>", (int&)prop.totalGlobalMem)) continue;
        if (parse_int(buf, "<sharedMemPerBlock`>", (int&)prop.sharedMemPerBlock)) continue;
        if (parse_int(buf, "<regsPerBlock`>", prop.regsPerBlock)) continue;
        if (parse_int(buf, "<warpSize`>", prop.warpSize)) continue;
        if (parse_int(buf, "<memPitch`>", (int&)prop.memPitch)) continue;
        if (parse_int(buf, "<maxThreadsPerBlock`>", prop.maxThreadsPerBlock)) continue;
        if (parse_str(buf, "<maxThreadsDim`>", buf2, sizeof(buf2))) {
            // can't use sscanf here (FCGI)
            //
            prop.maxThreadsDim[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                prop.maxThreadsDim[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    prop.maxThreadsDim[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_str(buf, "<maxGridSize`>", buf2, sizeof(buf2))) {
            prop.maxGridSize[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                prop.maxGridSize[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    prop.maxGridSize[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_int(buf, "<totalConstMem`>", (int&)prop.totalConstMem)) continue;
        if (parse_int(buf, "<major`>", prop.major)) continue;
        if (parse_int(buf, "<minor`>", prop.minor)) continue;
        if (parse_int(buf, "<clockRate`>", prop.clockRate)) continue;
        if (parse_int(buf, "<textureAlignment`>", (int&)prop.textureAlignment)) continue;
    }
    return ERR_XML_PARSE;
}

const char* COPROC_CELL_SPE::get(COPROCS&) {
    return NULL;
}
