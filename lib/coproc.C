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

#include "coproc.h"

void COPROCS::get() {
    COPROC_CUDA::get(*this);
    COPROC_CELL_SPE::get(*this);
}

void COPROC_CUDA::get(COPROCS& coprocs) {
   void (*__cudaGetDeviceCount)( int * );
   void (*__cudaGetDeviceProperties) ( cudaDeviceProp*, int );
   int count;
   cudaDeviceProp prop;

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
        "   <maxThreadsDim>%d</maxThreadsDim>\n"
        "   <maxGridSize>%d</maxGridSize>\n"
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
        prop.maxThreadsDim,
        prop.maxGridSize,
        prop.totalConstMem,
        prop.major,
        prop.minor,
        prop.clockRate,
        prop.textureAlignment
    );
}

void COPROC_CELL_SPE::get(COPROCS&) {
}
