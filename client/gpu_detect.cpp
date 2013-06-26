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

#ifdef __APPLE__
#define USE_CHILD_PROCESS_TO_DETECT_GPUS 1
#endif

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <setjmp.h>
#include <signal.h>
#endif

#include "coproc.h"
#include "file_names.h"
#include "util.h"
#include "util.h"
#include "str_replace.h"

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

static char client_path[MAXPATHLEN];

void COPROCS::get(
    bool use_all, vector<string>&descs, vector<string>&warnings,
    IGNORE_GPU_INSTANCE& ignore_gpu_instance
) {
#if USE_CHILD_PROCESS_TO_DETECT_GPUS
    launch_child_process_to_detect_gpus();
    read_coproc_info_file(warnings);
#else
    detect_gpus(warnings);
#endif
    correlate_gpus(use_all, descs, ignore_gpu_instance);
}


void COPROCS::detect_gpus(std::vector<std::string> &warnings) {
#ifdef _WIN32
    try {
        nvidia.get(warnings);
    }
    catch (...) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    }
    try {
        ati.get(warnings);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    }
    try {
        intel_gpu.get(warnings);
    } 
    catch (...) {
        warnings.push_back("Caught SIGSEGV in INTEL GPU detection");
    }
    try {
        // OpenCL detection must come last
        get_opencl(warnings);
    }
    catch (...) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    }
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        nvidia.get(warnings);
    }
    

#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in ATI GPU detection");
    } else {
        ati.get(warnings);
    }
#endif
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in INTEL GPU detection");
    } else {
        intel_gpu.get(warnings);
    }
    if (setjmp(resume)) {
        warnings.push_back("Caught SIGSEGV in OpenCL detection");
    } else {
        // OpenCL detection must come last
        get_opencl(warnings);
    }
    signal(SIGSEGV, old_sig);
#endif
}


void COPROCS::correlate_gpus(
    bool use_all,
    std::vector<std::string> &descs,
    IGNORE_GPU_INSTANCE &ignore_gpu_instance
) {
    unsigned int i;
    char buf[256], buf2[256];

    nvidia.correlate(use_all, ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU]);
    ati.correlate(use_all, ignore_gpu_instance[PROC_TYPE_AMD_GPU]);
    intel_gpu.correlate(use_all, ignore_gpu_instance[PROC_TYPE_AMD_GPU]);
    correlate_opencl(use_all, ignore_gpu_instance);

    for (i=0; i<nvidia_gpus.size(); i++) {
        // This is really CUDA description
        nvidia_gpus[i].description(buf);
        switch(nvidia_gpus[i].is_used) {
        case COPROC_IGNORED:
            sprintf(buf2, "CUDA: NVIDIA GPU %d (ignored by config): %s", nvidia_gpus[i].device_num, buf);
            break;
        case COPROC_USED:
            sprintf(buf2, "CUDA: NVIDIA GPU %d: %s", nvidia_gpus[i].device_num, buf);
            break;
        case COPROC_UNUSED:
        default:
            sprintf(buf2, "CUDA: NVIDIA GPU %d (not used): %s", nvidia_gpus[i].device_num, buf);
            break;
        }
        descs.push_back(string(buf2));
    }

    for (i=0; i<ati_gpus.size(); i++) {
        // This is really CAL description
        ati_gpus[i].description(buf);
        switch(ati_gpus[i].is_used) {
        case COPROC_IGNORED:
            sprintf(buf2, "CAL: ATI GPU %d (ignored by config): %s", ati_gpus[i].device_num, buf);
            break;
        case COPROC_USED:
            sprintf(buf2, "CAL: ATI GPU %d: %s", ati_gpus[i].device_num, buf);
            break;
        case COPROC_UNUSED:
        default:
            sprintf(buf2, "CAL: ATI GPU %d: (not used) %s", ati_gpus[i].device_num, buf);
            break;
        }
        descs.push_back(string(buf2));
    }

    // Create descriptions for OpenCL NVIDIA GPUs
    //
    for (i=0; i<nvidia_opencls.size(); i++) {
        nvidia_opencls[i].description(buf, proc_type_name(PROC_TYPE_NVIDIA_GPU));
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL ATI GPUs
    //
    for (i=0; i<ati_opencls.size(); i++) {
        ati_opencls[i].description(buf, proc_type_name(PROC_TYPE_AMD_GPU));
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL Intel GPUs
    //
    for (i=0; i<intel_gpu_opencls.size(); i++) {
        intel_gpu_opencls[i].description(buf, proc_type_name(PROC_TYPE_INTEL_GPU));
        descs.push_back(string(buf));
    }

    ati_gpus.clear();
    nvidia_gpus.clear();
    intel_gpus.clear();
    ati_opencls.clear();
    nvidia_opencls.clear();
    intel_gpu_opencls.clear();
}

// Some dual-GPU laptops (e.g., Macbook Pro) don't 
// power down the more powerful GPU until all
// applications which used them exit.  To save
// battery life, the client launches a second
// instance of the client as a child process to 
// detect and get information about the GPUs.
// The child process writes the info to a temp
// file which our main client then reads.
//
void COPROCS::set_path_to_client(char *path) {
    strlcpy(client_path, path, sizeof(client_path));
}

int COPROCS::write_coproc_info_file(vector<string> &warnings) {
    MIOFILE mf;
    unsigned int i, temp;
    FILE* f;
    
    f = boinc_fopen(COPROC_INFO_FILENAME, "wb");
    if (!f) return ERR_FOPEN;
    mf.init_file(f);
    
    mf.printf("    <coprocs>\n");

    for (i=0; i<ati_gpus.size(); ++i) {
       ati_gpus[i].write_xml(mf, false);
    }
    for (i=0; i<nvidia_gpus.size(); ++i) {
        temp = nvidia_gpus[i].count;
        nvidia_gpus[i].count = 1;
        nvidia_gpus[i].pci_infos[0] = nvidia_gpus[i].pci_info;
        nvidia_gpus[i].write_xml(mf, false);
        nvidia_gpus[i].count = temp;
    }
    for (i=0; i<intel_gpus.size(); ++i) {
        intel_gpus[i].write_xml(mf, false);
    }
    for (i=0; i<ati_opencls.size(); ++i) {
        ati_opencls[i].write_xml(mf, "ati_opencl", true);
    }
    for (i=0; i<nvidia_opencls.size(); ++i) {
        nvidia_opencls[i].write_xml(mf, "nvidia_opencl", true);
    }
    for (i=0; i<intel_gpu_opencls.size(); ++i) {
        intel_gpu_opencls[i].write_xml(mf, "intel_gpu_opencl", true);
    }
    for (i=0; i<warnings.size(); ++i) {
        mf.printf("<warning>%s</warning>\n", warnings[i].c_str());
    }

// TODO: write OpenCL info for CPU when implemented:
//  gstate.host_info.have_cpu_opencl
//  gstate.host_info.cpu_opencl_prop

    mf.printf("    </coprocs>\n");
    fclose(f);
    return 0;
}

int COPROCS::read_coproc_info_file(vector<string> &warnings) {
    MIOFILE mf;
    int retval;
    FILE* f;
    string s;

    COPROC_ATI ati_gpu;
    COPROC_NVIDIA nvidia_gpu;
    COPROC_INTEL intel_gpu;
    OPENCL_DEVICE_PROP ati_opencl;
    OPENCL_DEVICE_PROP nvidia_opencl;
    OPENCL_DEVICE_PROP intel_gpu_opencl;

    ati_gpus.clear();
    nvidia_gpus.clear();
    intel_gpus.clear();
    ati_opencls.clear();
    nvidia_opencls.clear();
    intel_gpu_opencls.clear();

    f = fopen(COPROC_INFO_FILENAME, "r");
    if (!f) return ERR_FOPEN;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    
    while (!xp.get_tag()) {
        if (xp.match_tag("coprocs")) {
            if (xp.match_tag("/coprocs")) {
                fclose(f);
                return 0;
            }
        }

        if (xp.match_tag("coproc_ati")) {
            retval = ati_gpu.parse(xp);
            if (retval) {
                ati_gpu.clear();
            } else {
                ati_gpu.device_num = ati_gpus.size();
                ati_gpus.push_back(ati_gpu);
            }
            continue;
        }
        if (xp.match_tag("coproc_cuda")) {
            retval = nvidia_gpu.parse(xp);
            if (retval) {
                nvidia_gpu.clear();
            } else {
                nvidia_gpu.device_num = nvidia_gpus.size();
                nvidia_gpu.pci_info = nvidia_gpu.pci_infos[0];
                memset(&nvidia_gpu.pci_infos[0], 0, sizeof(struct PCI_INFO));
                nvidia_gpus.push_back(nvidia_gpu);
            }
            continue;
        }
        if (xp.match_tag("coproc_intel_gpu")) {
            retval = intel_gpu.parse(xp);
            if (retval) {
                intel_gpu.clear();
            } else {
                intel_gpu.device_num = nvidia_gpus.size();
                intel_gpus.push_back(intel_gpu);
            }
            continue;
        }
        
        if (xp.match_tag("ati_opencl")) {
            memset(&ati_opencl, 0, sizeof(ati_opencl));
            retval = ati_opencl.parse(xp, "/ati_opencl");
            if (retval) {
                memset(&ati_opencl, 0, sizeof(ati_opencl));
            } else {
                ati_opencl.is_used = COPROC_IGNORED;
                ati_opencls.push_back(ati_opencl);
            }
            continue;
        }

        if (xp.match_tag("nvidia_opencl")) {
            memset(&nvidia_opencl, 0, sizeof(nvidia_opencl));
            retval = nvidia_opencl.parse(xp, "/nvidia_opencl");
            if (retval) {
                memset(&nvidia_opencl, 0, sizeof(nvidia_opencl));
            } else {
                nvidia_opencl.is_used = COPROC_IGNORED;
                nvidia_opencls.push_back(nvidia_opencl);
            }
            continue;
        }

        if (xp.match_tag("intel_gpu_opencl")) {
            memset(&intel_gpu_opencl, 0, sizeof(intel_gpu_opencl));
            retval = intel_gpu_opencl.parse(xp, "/intel_gpu_opencl");
            if (retval) {
                memset(&intel_gpu_opencl, 0, sizeof(intel_gpu_opencl));
            } else {
                intel_gpu_opencl.is_used = COPROC_IGNORED;
                intel_gpu_opencls.push_back(intel_gpu_opencl);
            }
            continue;
        }
        if (xp.parse_string("warning", s)) {
            warnings.push_back(s);
            continue;
        }

    // TODO: parse OpenCL info for CPU when implemented:
    //  gstate.host_info.have_cpu_opencl
    //  gstate.host_info.cpu_opencl_prop
    }
    
    fclose(f);
    return ERR_XML_PARSE;
}

int COPROCS::launch_child_process_to_detect_gpus() {
#ifdef _WIN32
    HANDLE prog;
#else
    int prog;
#endif
    char dir[MAXPATHLEN];
    int i;
    int retval = 0;
    
    boinc_delete_file(COPROC_INFO_FILENAME);
    boinc_getcwd(dir);
    
    int argc = 3;
    char* const argv[3] = { 
         const_cast<char *>("boinc"), 
         const_cast<char *>("--detect_gpus"), 
         const_cast<char *>("") 
    }; 

    retval = run_program(
        dir,
        client_path,
        argc,
        argv, 
        0,
        prog
    );
    
    if (retval) return retval;
    
    // Wait for child to run and exit
    for (i=0; i<50; ++i) {
        boinc_sleep(0.1);
        if (process_exists(prog)) break;
        if (boinc_file_exists(COPROC_INFO_FILENAME)) break;
    }
    
    for (i=0; i<50; ++i) {
        boinc_sleep(0.1);
        if (!process_exists(prog)) break;
    }
    
    return 0;
}
