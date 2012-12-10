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

bool in_vector(int n, vector<int>& v) {
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

vector<COPROC_ATI> ati_gpus;
vector<COPROC_NVIDIA> nvidia_gpus;
vector<OPENCL_DEVICE_PROP> ati_opencls;
vector<OPENCL_DEVICE_PROP> nvidia_opencls;
vector<OPENCL_DEVICE_PROP> intel_gpu_opencls;

void COPROCS::get(
    bool use_all, vector<string>&descs, vector<string>&warnings,
    vector<int>& ignore_nvidia_dev,
    vector<int>& ignore_ati_dev,
    vector<int>& ignore_intel_gpu_dev
) {
    unsigned int i;
    char buf[256], buf2[256];

#ifdef _WIN32
    try {
        nvidia.get(use_all, warnings, ignore_nvidia_dev);
    }
    catch (...) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    }
    try {
        ati.get(use_all, warnings, ignore_ati_dev);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    }
    try {
        get_opencl(use_all, warnings, ignore_ati_dev, ignore_nvidia_dev, ignore_intel_gpu_dev);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    }
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        nvidia.get(use_all, warnings, ignore_nvidia_dev);
    }
#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    } else {
        ati.get(use_all, warnings, ignore_ati_dev);
    }
#endif
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    } else {
        get_opencl(use_all, warnings, ignore_ati_dev, ignore_nvidia_dev, ignore_intel_gpu_dev);
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
            sprintf(buf2, "NVIDIA GPU %d (not used): %s", nvidia_gpus[i].device_num, buf);
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
            sprintf(buf2, "ATI GPU %d: (not used) %s", ati_gpus[i].device_num, buf);
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
    ati_opencls.clear();
    nvidia_opencls.clear();
    intel_gpu_opencls.clear();
}
