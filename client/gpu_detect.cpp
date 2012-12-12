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
#include <mach-o/dyld.h>
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

#ifndef _WIN32
jmp_buf resume;

void segv_handler(int) {
    longjmp(resume, 1);
}
#endif

vector<COPROC_ATI> ati_gpus;
vector<COPROC_NVIDIA> nvidia_gpus;
vector<COPROC_INTEL> intel_gpus;
vector<OPENCL_DEVICE_PROP> ati_opencls;
vector<OPENCL_DEVICE_PROP> nvidia_opencls;
vector<OPENCL_DEVICE_PROP> intel_gpu_opencls;

void COPROCS::get(
    bool use_all, vector<string>&descs, vector<string>&warnings,
    IGNORE_GPU_INSTANCE& ignore_gpu_instance
) {
    unsigned int i;
    char buf[256], buf2[256];

#ifdef _WIN32
    try {
        nvidia.get(use_all, warnings, ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU]);
    }
    catch (...) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    }
    try {
        ati.get(use_all, warnings, ignore_gpu_instance[PROC_TYPE_AMD_GPU]);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    }
    try {
        intel_gpu.get(use_all, warnings, ignore_gpu_instance[PROC_TYPE_INTEL_GPU]);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in INTEL GPU detection");
    }
    try {
        get_opencl(use_all, warnings, ignore_gpu_instance);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    }
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        nvidia.get(use_all, warnings, ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU]);
    }
#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    } else {
        ati.get(use_all, warnings, ignore_gpu_instance[PROC_TYPE_AMD_GPU]);
    }
#endif
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in INTEL GPU detection");
    } else {
        intel_gpu.get(use_all, warnings, ignore_gpu_instance[PROC_TYPE_INTEL_GPU]);
    }
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    } else {
        get_opencl(use_all, warnings, ignore_gpu_instance);
    }
    signal(SIGSEGV, old_sig);
#endif

    for (i=0; i<nvidia_gpus.size(); i++) {
        nvidia_gpus[i].description(buf);
        switch(nvidia_gpus[i].is_used) {
        case COPROC_IGNORED:
            sprintf(buf2, "NVIDIA GPU %d (ignored by config): %s", nvidia_gpus[i].device_num, buf);
            break;
        case COPROC_USED:
            sprintf(buf2, "NVIDIA GPU %d: %s", nvidia_gpus[i].device_num, buf);
            break;
        case COPROC_UNUSED:
        default:
            if (nvidia_gpus[i].opencl_prop.is_used) {
                sprintf(buf2, "NVIDIA GPU %d (OpenCL only): %s", nvidia_gpus[i].device_num, buf);
            } else {
                sprintf(buf2, "NVIDIA GPU %d (not used): %s", nvidia_gpus[i].device_num, buf);
            }
            break;
        }
        descs.push_back(string(buf2));
    }

    for (i=0; i<ati_gpus.size(); i++) {
        ati_gpus[i].description(buf);
        switch(ati_gpus[i].is_used) {
        case COPROC_IGNORED:
            sprintf(buf2, "ATI GPU %d (ignored by config): %s", ati_gpus[i].device_num, buf);
            break;
        case COPROC_USED:
            sprintf(buf2, "ATI GPU %d: %s", ati_gpus[i].device_num, buf);
            break;
        case COPROC_UNUSED:
        default:
            if (ati_gpus[i].opencl_prop.is_used) {
                sprintf(buf2, "ATI GPU %d: (OpenCL only) %s", ati_gpus[i].device_num, buf);
            } else {
                sprintf(buf2, "ATI GPU %d: (not used) %s", ati_gpus[i].device_num, buf);
            }
            break;
        }
        descs.push_back(string(buf2));
    }

    for (i=0; i<intel_gpus.size(); i++) {
        intel_gpus[i].description(buf);
        switch(intel_gpus[i].is_used) {
        case COPROC_IGNORED:
            sprintf(buf2, "INTEL GPU %d (ignored by config): %s", intel_gpus[i].device_num, buf);
            break;
        case COPROC_USED:
            sprintf(buf2, "INTEL GPU %d: %s", intel_gpus[i].device_num, buf);
            break;
        case COPROC_UNUSED:
        default:
            if (intel_gpus[i].opencl_prop.is_used) {
                sprintf(buf2, "INTEL GPU %d: (OpenCL only) %s", intel_gpus[i].device_num, buf);
            } else {
                sprintf(buf2, "INTEL GPU %d: (not used) %s", intel_gpus[i].device_num, buf);
            }
            break;
        }
        descs.push_back(string(buf2));
    }

    // Create descriptions for OpenCL NVIDIA GPUs
    //
    for (i=0; i<nvidia_opencls.size(); i++) {
        nvidia_opencls[i].description(buf, GPU_TYPE_NVIDIA);
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL ATI GPUs
    //
    for (i=0; i<ati_opencls.size(); i++) {
        ati_opencls[i].description(buf, GPU_TYPE_ATI);
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL Intel GPUs
    //
    for (i=0; i<intel_gpu_opencls.size(); i++) {
        intel_gpu_opencls[i].description(buf, GPU_TYPE_INTEL);
        descs.push_back(string(buf));
    }

    ati_gpus.clear();
    nvidia_gpus.clear();
    intel_gpus.clear();
    ati_opencls.clear();
    nvidia_opencls.clear();
    intel_gpu_opencls.clear();
}
