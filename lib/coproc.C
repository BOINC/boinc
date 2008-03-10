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
#include <dlfcn.h>
#endif

#include "error_numbers.h"
#include "parse.h"

#include "coproc.h"

void COPROCS::get() {
    COPROC_CUDA::get(*this);
    COPROC_CELL_SPE::get(*this);
}

int COPROCS::parse(FILE* fin) {
    char buf[1024];

    while (fgets(buf, sizeof(buf), fin)) {
        if (strstr(buf, "</coprocs>")) return 0;
        if (strstr(buf, "<coproc_cuda>")) {
            COPROC_CUDA cc;
            int retval = cc.parse(fin);
            if (!retval) {
                coprocs.push_back(cc);
            }
        }
    }
    return ERR_XML_PARSE;
}

void COPROC_CUDA::get(COPROCS& coprocs) {
   void (*__cudaGetDeviceCount)( int * );
   void (*__cudaGetDeviceProperties) ( cudaDeviceProp*, int );
   int count;

#ifdef _WIN32
   HMODULE cudalib = GetModuleHandle("cudart.dll");
   __cudaGetDeviceCount = (void(*)(int*)) GetProcAddress(cudalib, "cudaGetDeviceCount");
   if(!__cudaGetDeviceCount) return;
   __cudaGetDeviceProperties = (void(*)(cudaDeviceProp*, int)) GetProcAddress( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) return;

#else
   void *cudalib = dlopen ("libcudart.so", RTLD_NOW );
   if(!cudalib) return;
   __cudaGetDeviceCount = (void(*)(int*)) dlsym(cudalib, "cudaGetDeviceCount");
   if(!__cudaGetDeviceCount) return;
   __cudaGetDeviceProperties = (void(*)(cudaDeviceProp*, int)) dlsym( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) return;
#endif
   (*__cudaGetDeviceCount)(&count);
   if (count < 1) return;

   for (int i=0; i<count; i++) {
       COPROC_CUDA cc;
       (*__cudaGetDeviceProperties)(&cc.prop, i);
       cc.count = 1;
       strcpy(cc.name, cc.prop.name);
       coprocs.coprocs.push_back(cc);
    }
}

void COPROC_CUDA::write_xml(FILE* f) {
    fprintf(f,
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
        "   <textureAlignment>%u</textureAlignment>\n",
        count,
        prop.name,
        prop.totalGlobalMem,
        prop.sharedMemPerBlock,
        prop.regsPerBlock,
        prop.warpSize,
        prop.memPitch,
        prop.maxThreadsPerBlock,
        prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2],
        prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2],
        prop.totalConstMem,
        prop.major,
        prop.minor,
        prop.clockRate,
        prop.textureAlignment
    );
}

void COPROC_CUDA::clear() {
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
        if (parse_str(buf, "<name>", name, sizeof(name))) {
            strcpy(prop.name, name);
            continue;
        }
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

void COPROC_CELL_SPE::get(COPROCS&) {
}
