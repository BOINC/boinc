// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// Detect the host's GPUs, and populate the global 'coprocs' object
//
// Theory of operation:
// There are two ways of detecting GPUs:
//  - vendor-specific libraries like CUDA and CAL,
//      which detect only that vendor's GPUs
//  - OpenCL, which can detect multiple types of GPUs:
//      nvidia, amd, intel, apple and new types
//
// We call these libraries in a separate process (boinc --detect_gpus)
// for two reasons:
// 1) These libraries sometimes crash,
//    and we've been unable to trap these via signal and exception handlers.
// 2) Some dual-GPU laptops (e.g., Macbook Pro) don't power down
//    the more powerful GPU until all applications which used them exit.
//    Doing GPU detection in a second, short-lived process
//    saves battery life on these laptops.

// This process writes an XML file "coproc_info.xml" containing
//  - lists of GPUs detected via vendor-specific APIs
//      e.g. <coproc_cuda>
//  - lists of nvidia/amd/intel/apple GPUs detected via OpenCL
//      e.g. <nvidia_opencl>
//  - a list of other GPUs detected via OpenCL
//      e.g. <other_opencl>
//
// Data structures (lib/coproc.h):
// OPENCL_DEVICE_PROP: OpenCL info for a GPU
// COPROC: base class for a GPU, including OpenCL info
// COPROC_ATI etc.: derived classes including vendor-specific info
// COPROCS: a vector of COPROC objects,
//      together with an instance of each vendor-specific type
//
// BOINC assumes that all GPUs of a given vendor are equivalent.
// We make this true by identifying the 'most capable' GPU of each type,
// and ignoring GPUs of that type that are not equivalent to it
// (in terms of memory size and capabilities).
// In the final COPROCS object, each COPROC has a 'count' field
// and a list of IDs (native, OpenCL, PCI) for the qualifying GPU instances.
//
// The client parses coproc_info.xml into a bunch of vectors:
//      ati_gpus, etc: COPROC_* objects
//      api_opencls, etc: OPENCL_DEVICE_PROPs for ATI GPUs
//      other_opencls: OPENCL_DEVICE_PROPs for other GPUs
//
// Then (COPROCS::correlate_gpus):
// for each vendor (e.g. COPROC_ATI::correlate()):
//   scan ati_gpus and find the most capable GPU.
//   Copy its object to COPROCS.ati_gpu.
//   Scan the list again, identifying equivalent instances,
//   incrementing count, getting IDs
// Then (COPROCS::correlate_opencl())
// For each vendor (e.g. ATI)
//   if we detected a GPU with CAL
//      merge_opencl(): copy OpenCL info to COPROCS.ati_gpu
//   else
//      find_best_opencls(): find best OpenCL instance,
//      populate COPROCS.ati_gpu
//
//
// Finally (CLIENT_STATE::init()):
// For each vendor (e.g. ATI)
//  if COPROC_ATI is present (count>0)
//     append it to the COPROCS.coprocs vector
// Append COPROCs for other OpenCL devices found

// GPUs can also be explicitly described in cc_config.xml

// comment out to do GPU detection in same process (for debugging)
//
#define USE_CHILD_PROCESS_TO_DETECT_GPUS 1

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include "config.h"
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "coproc.h"
#include "file_names.h"
#include "util.h"
#include "str_replace.h"

#include "client_msgs.h"
#include "client_state.h"
#include "gpu_detect.h"

using std::string;
using std::vector;

#ifndef _WIN32
jmp_buf resume;

void segv_handler(int) {
    longjmp(resume, 1);
}
#endif

// the following vectors store the low-level info from coproc_info.xml
// - For integrated GPUs (Intel, Apple)
//  there is at most one instance on current computers.
//  But who knows, this might change.
// - Intel GPUs can currently only be used via OpenCL.
//  But this too could change.
//
vector<COPROC_ATI> ati_gpus;
vector<COPROC_NVIDIA> nvidia_gpus;
vector<COPROC_INTEL> intel_gpus;
vector<COPROC_APPLE> apple_gpus;
vector<OPENCL_DEVICE_PROP> ati_opencls;
vector<OPENCL_DEVICE_PROP> nvidia_opencls;
vector<OPENCL_DEVICE_PROP> intel_gpu_opencls;
vector<OPENCL_DEVICE_PROP> apple_gpu_opencls;
vector<OPENCL_DEVICE_PROP> other_opencls;
vector<OPENCL_CPU_PROP> cpu_opencls;

static char* client_path;
    // argv[0] from the command used to run client.
    // May be absolute or relative.
static char client_dir[MAXPATHLEN];
    // current directory at start of client

// find GPU instances, then correlate (merge) them
//
void COPROCS::get(
    bool use_all, vector<string>&descs, vector<string>&warnings,
    IGNORE_GPU_INSTANCE& ignore_gpu_instance
) {
#if USE_CHILD_PROCESS_TO_DETECT_GPUS
    // detect_gpus() can cause crashes even with try/catch,
    // so do it in a separate process that writes a file
    //
    int retval = 0;
    char buf[256];

    retval = launch_child_process_to_detect_gpus();
    if (retval) {
        snprintf(buf, sizeof(buf),
            "launch_child_process_to_detect_gpus() returned error %d",
            retval
        );
        gpu_warning(warnings, buf);
    }
    retval = read_coproc_info_file(warnings);
    if (retval) {
        snprintf(buf, sizeof(buf),
            "read_coproc_info_file() returned error %d",
            retval
        );
        gpu_warning(warnings, buf);
    }
#else
    detect_gpus(warnings);
#endif
    correlate_gpus(use_all, descs, ignore_gpu_instance);
}

// populate the global vectors
// ati_gpus, nvidia_gpus, intel_gpus, nvidia_opencls, etc.
//
void COPROCS::detect_gpus(vector<string> &warnings) {
#ifdef _WIN32
#if !defined(_M_ARM) && !defined(_M_ARM64)
    try {
        nvidia.get(warnings);
    }
    catch (...) {
        gpu_warning(warnings, "Caught SIGSEGV in NVIDIA GPU detection");
    }
#endif
    try {
        ati.get(warnings);
    }
    catch (...) {
        gpu_warning(warnings, "Caught SIGSEGV in ATI GPU detection");
    }
    try {
        intel_gpu.get(warnings);
    }
    catch (...) {
        gpu_warning(warnings, "Caught SIGSEGV in INTEL GPU detection");
    }
    try {
        // OpenCL detection must come last
        get_opencl(warnings);
    }
    catch (...) {
        gpu_warning(warnings, "Caught SIGSEGV in OpenCL detection");
    }
#else // non-Windows

    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        gpu_warning(warnings, "Caught SIGSEGV in NVIDIA GPU detection");
    } else {
        nvidia.get(warnings);
    }


#ifndef __APPLE__       // ATI does not yet support CAL on Macs
    if (setjmp(resume)) {
        gpu_warning(warnings, "Caught SIGSEGV in ATI GPU detection");
    } else {
        ati.get(warnings);
    }
#else
    apple_gpu.get(warnings);
#endif
    if (setjmp(resume)) {
        gpu_warning(warnings, "Caught SIGSEGV in INTEL GPU detection");
    } else {
        intel_gpu.get(warnings);
    }
    if (setjmp(resume)) {
        gpu_warning(warnings, "Caught SIGSEGV in OpenCL detection");
    } else {
        // OpenCL detection must come last
        get_opencl(warnings);
    }
    signal(SIGSEGV, old_sig);
#endif
}

// for each GPU type, scan the GPUs we detected
// (e.g. the vector nvidia_gpus).
// Find the most capable one, and the ones equivalent to it.
// Also correlate OpenCL GPUs with CUDA/CAL GPUs.
// Then create a single COPROC (with appropriate count)
// Also return a list of strings describing the GPUs.
//
void COPROCS::correlate_gpus(
    bool use_all,
    vector<string> &descs,
    IGNORE_GPU_INSTANCE &ignore_gpu_instance
) {
    unsigned int i;
    char buf[256], buf2[1024];

#if !defined(_M_ARM) && !defined(_M_ARM64)
    nvidia.correlate(use_all, ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU]);
#endif
    ati.correlate(use_all, ignore_gpu_instance[PROC_TYPE_AMD_GPU]);
    intel_gpu.correlate(use_all, ignore_gpu_instance[PROC_TYPE_INTEL_GPU]);
#ifdef __APPLE__
    apple_gpu.correlate(use_all, ignore_gpu_instance[PROC_TYPE_APPLE_GPU]);
#endif
    correlate_opencl(use_all, ignore_gpu_instance);

    for (i=0; i<cpu_opencls.size(); i++) {
        gstate.host_info.opencl_cpu_prop[gstate.host_info.num_opencl_cpu_platforms++] = cpu_opencls[i];
    }

    // make list of GPU descriptions

    for (i=0; i<nvidia_gpus.size(); i++) {
        // This is really CUDA description
        nvidia_gpus[i].description(buf, sizeof(buf));
        switch(nvidia_gpus[i].is_used) {
        case COPROC_IGNORED:
            snprintf(buf2, sizeof(buf2),
                "CUDA: NVIDIA GPU %d (ignored by config): %s",
                nvidia_gpus[i].device_num, buf
            );
            break;
        case COPROC_USED:
            snprintf(buf2, sizeof(buf2),
                "CUDA: NVIDIA GPU %d: %s",
                nvidia_gpus[i].device_num, buf
            );
            break;
        case COPROC_UNUSED:
        default:
            snprintf(buf2, sizeof(buf2),
                "CUDA: NVIDIA GPU %d (not used): %s",
                nvidia_gpus[i].device_num, buf
            );

#ifdef __APPLE__
            if ((nvidia_gpus[i].cuda_version >= 6050) &&
                            nvidia_gpus[i].prop.major < 2) {
                // This will be called only if CUDA recognized and reported the GPU
                msg_printf(NULL, MSG_USER_ALERT, "NVIDIA GPU %d: %s %s",
                    nvidia_gpus[i].device_num, nvidia_gpus[i].prop.name,
                    _("cannot be used for CUDA or OpenCL computation with CUDA driver 6.5 or later")
                );
            }
#endif
            break;
        }
        descs.push_back(string(buf2));
    }

    for (i=0; i<ati_gpus.size(); i++) {
        // This is really CAL description
        ati_gpus[i].description(buf, sizeof(buf));
        switch(ati_gpus[i].is_used) {
        case COPROC_IGNORED:
            snprintf(buf2, sizeof(buf2),
                "CAL: ATI GPU %d (ignored by config): %s",
                ati_gpus[i].device_num, buf
            );
            break;
        case COPROC_USED:
            snprintf(buf2, sizeof(buf2),
                "CAL: ATI GPU %d: %s",
                ati_gpus[i].device_num, buf
            );
            break;
        case COPROC_UNUSED:
        default:
            snprintf(buf2, sizeof(buf2),
                "CAL: ATI GPU %d: (not used) %s",
                ati_gpus[i].device_num, buf
            );
            break;
        }
        descs.push_back(string(buf2));
    }

    // Create descriptions for OpenCL NVIDIA GPUs
    //
    for (i=0; i<nvidia_opencls.size(); i++) {
        if (nvidia_opencls[i].warn_bad_cuda) {
            // This will be called only if CUDA did _not_ recognize and report the GPU
            msg_printf(NULL, MSG_USER_ALERT, "NVIDIA GPU %d: %s %s",
                nvidia_opencls[i].device_num, nvidia_opencls[i].name,
                _("cannot be used for CUDA or OpenCL computation with CUDA driver 6.5 or later")
            );
        }
        nvidia_opencls[i].description(buf, sizeof(buf), proc_type_name(PROC_TYPE_NVIDIA_GPU));
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL ATI GPUs
    //
    for (i=0; i<ati_opencls.size(); i++) {
        ati_opencls[i].description(buf, sizeof(buf), proc_type_name(PROC_TYPE_AMD_GPU));
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL Intel GPUs
    //
    for (i=0; i<intel_gpu_opencls.size(); i++) {
        intel_gpu_opencls[i].description(buf, sizeof(buf), proc_type_name(PROC_TYPE_INTEL_GPU));
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL Apple GPUs
    //
    for (i=0; i<apple_gpu_opencls.size(); i++) {
        apple_gpu_opencls[i].description(buf, sizeof(buf), proc_type_name(PROC_TYPE_APPLE_GPU));
        descs.push_back(string(buf));
    }

    // Create descriptions for other OpenCL GPUs
    //
    int max_other_coprocs = MAX_RSC-1;  // coprocs[0] is reserved for CPU
    if (have_nvidia()) max_other_coprocs--;
    if (have_ati()) max_other_coprocs--;
    if (have_intel_gpu()) max_other_coprocs--;
    if (have_apple_gpu()) max_other_coprocs--;

    // TODO: Should we implement cc_config ignore vectors
    // for other (future) OpenCL coprocessors?

    for (i=0; i<other_opencls.size(); i++) {
        if ((int)i > max_other_coprocs) {
            other_opencls[i].is_used = COPROC_UNUSED;
        }
        other_opencls[i].description(buf, sizeof(buf), other_opencls[i].name);
        descs.push_back(string(buf));
    }

    // Create descriptions for OpenCL CPUs
    //
    for (i=0; i<cpu_opencls.size(); i++) {
        cpu_opencls[i].description(buf, sizeof(buf));
        descs.push_back(string(buf));
    }

    ati_gpus.clear();
    nvidia_gpus.clear();
    intel_gpus.clear();
    apple_gpus.clear();
    ati_opencls.clear();
    nvidia_opencls.clear();
    intel_gpu_opencls.clear();
    apple_gpu_opencls.clear();
    cpu_opencls.clear();
}

// This is called from CLIENT_STATE::init()
// after adding NVIDIA/ATI/Intel/Apple GPUs
// If we don't care about the order of GPUs in COPROCS::coprocs[],
// this code could be included at the end of COPROCS::correlate_gpus().
//
int COPROCS::add_other_coproc_types() {
    int retval = 0;

    for (unsigned int i=0; i<other_opencls.size(); i++) {
        if (other_opencls[i].is_used != COPROC_USED) continue;
        if (n_rsc >= MAX_RSC) {
            retval = ERR_BUFFER_OVERFLOW;
            break;
        }

        COPROC c;
        // For device types other than NVIDIA, ATI or Intel GPU.
        // we put each instance into a separate other_opencls element,
        // so count=1.
        //
        c.count = 1;
        c.opencl_device_count = 1;
        c.opencl_prop = other_opencls[i];
        c.available_ram = c.opencl_prop.global_mem_size;
        c.device_num = c.opencl_prop.device_num;
        c.peak_flops = c.opencl_prop.peak_flops;
        c.have_opencl = true;
        c.opencl_device_indexes[0] = c.opencl_prop.opencl_device_index;
        c.opencl_device_ids[0] = c.opencl_prop.device_id;
        c.instance_has_opencl[0] = true;
        c.clear_usage();
        safe_strcpy(c.type, other_opencls[i].name);

        // Don't call COPROCS::add() because duplicate type is legal here
        coprocs[n_rsc++] = c;

    }

    other_opencls.clear();
    return retval;
}

void COPROCS::set_path_to_client(char *path) {
    client_path = path;
    // The path may be relative to the current directory
     boinc_getcwd(client_dir);
}

int COPROCS::write_coproc_info_file(vector<string> &warnings) {
    MIOFILE mf;
    unsigned int i, temp;
    FILE* f;

    f = boinc_fopen(COPROC_INFO_FILENAME, "wb");
    if (!f) return ERR_FOPEN;
    mf.init_file(f);

    mf.printf("    <coprocs>\n");

    if (nvidia.have_cuda) {
        mf.printf("    <have_cuda>1</have_cuda>\n");
        mf.printf("    <cuda_version>%d</cuda_version>\n", nvidia.cuda_version);
    }

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
    for (i=0; i<apple_gpus.size(); ++i) {
        apple_gpus[i].write_xml(mf, false);
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
    for (i=0; i<apple_gpu_opencls.size(); ++i) {
        apple_gpu_opencls[i].write_xml(mf, "apple_gpu_opencl", true);
    }
    for (i=0; i<other_opencls.size(); i++) {
        other_opencls[i].write_xml(mf, "other_opencl", true);
    }
    for (i=0; i<cpu_opencls.size(); i++) {
        cpu_opencls[i].write_xml(mf);
    }
    for (i=0; i<warnings.size(); ++i) {
        mf.printf("<warning>%s</warning>\n", warnings[i].c_str());
    }

    mf.printf("    </coprocs>\n");
    fclose(f);
    return 0;
}

// if we're using a separate process to find GPUs,
// read its output file and create data structures
//
int COPROCS::read_coproc_info_file(vector<string> &warnings) {
    MIOFILE mf;
    int retval;
    FILE* f;
    string s;

    COPROC_ATI my_ati_gpu;
    COPROC_NVIDIA my_nvidia_gpu;
    COPROC_INTEL my_intel_gpu;
    COPROC_APPLE my_apple_gpu;
    OPENCL_DEVICE_PROP ati_opencl;
    OPENCL_DEVICE_PROP nvidia_opencl;
    OPENCL_DEVICE_PROP intel_gpu_opencl;
    OPENCL_DEVICE_PROP apple_gpu_opencl;
    OPENCL_DEVICE_PROP other_opencl;
    OPENCL_CPU_PROP cpu_opencl;

    ati_gpus.clear();
    nvidia_gpus.clear();
    intel_gpus.clear();
    apple_gpus.clear();
    ati_opencls.clear();
    nvidia_opencls.clear();
    intel_gpu_opencls.clear();
    apple_gpu_opencls.clear();
    other_opencls.clear();
    cpu_opencls.clear();

    f = boinc_fopen(COPROC_INFO_FILENAME, "r");
    if (!f) return ERR_FOPEN;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    if (!xp.parse_start("coprocs")) {
        fclose(f);
        return ERR_XML_PARSE;
    }

    while (!xp.get_tag()) {
        if (xp.match_tag("/coprocs")) {
            fclose(f);
            return 0;
        }

        if (xp.parse_bool("have_cuda", nvidia.have_cuda)) continue;
        if (xp.parse_int("cuda_version", nvidia.cuda_version)) {
             continue;
        }

        if (xp.match_tag("coproc_ati")) {
            retval = my_ati_gpu.parse(xp);
            if (retval) {
                my_ati_gpu.clear();
            } else {
                my_ati_gpu.device_num = (int)ati_gpus.size();
                ati_gpus.push_back(my_ati_gpu);
            }
            continue;
        }
        if (xp.match_tag("coproc_cuda")) {
            retval = my_nvidia_gpu.parse(xp);
            if (retval) {
                my_nvidia_gpu.clear();
            } else {
                my_nvidia_gpu.device_num = (int)nvidia_gpus.size();
                my_nvidia_gpu.pci_info = my_nvidia_gpu.pci_infos[0];
                my_nvidia_gpu.pci_infos[0].clear();
                nvidia_gpus.push_back(my_nvidia_gpu);
            }
            continue;
        }
        if (xp.match_tag("coproc_intel_gpu")) {
            retval = my_intel_gpu.parse(xp);
            if (retval) {
                my_intel_gpu.clear();
            } else {
                my_intel_gpu.device_num = (int)intel_gpus.size();
                intel_gpus.push_back(my_intel_gpu);
            }
            continue;
        }
        if (xp.match_tag("coproc_apple_gpu")) {
            retval = my_apple_gpu.parse(xp);
            if (retval) {
                my_apple_gpu.clear();
            } else {
                my_apple_gpu.device_num = (int)apple_gpus.size();
                apple_gpus.push_back(my_apple_gpu);
            }
            continue;
        }

        if (xp.match_tag("ati_opencl")) {
            ati_opencl.clear();
            retval = ati_opencl.parse(xp, "/ati_opencl");
            if (retval) {
                ati_opencl.clear();
            } else {
                ati_opencl.is_used = COPROC_IGNORED;
                ati_opencls.push_back(ati_opencl);
            }
            continue;
        }

        if (xp.match_tag("nvidia_opencl")) {
            nvidia_opencl.clear();
            retval = nvidia_opencl.parse(xp, "/nvidia_opencl");
            if (retval) {
                nvidia_opencl.clear();
            } else {
                nvidia_opencl.is_used = COPROC_IGNORED;
                nvidia_opencls.push_back(nvidia_opencl);
            }
            continue;
        }

        if (xp.match_tag("intel_gpu_opencl")) {
            intel_gpu_opencl.clear();
            retval = intel_gpu_opencl.parse(xp, "/intel_gpu_opencl");
            if (retval) {
                intel_gpu_opencl.clear();
            } else {
                intel_gpu_opencl.is_used = COPROC_IGNORED;
                intel_gpu_opencls.push_back(intel_gpu_opencl);
            }
            continue;
        }
        if (xp.match_tag("apple_gpu_opencl")) {
            apple_gpu_opencl.clear();
            retval = apple_gpu_opencl.parse(xp, "/apple_gpu_opencl");
            if (retval) {
                apple_gpu_opencl.clear();
            } else {
                apple_gpu_opencl.is_used = COPROC_IGNORED;
                apple_gpu_opencls.push_back(apple_gpu_opencl);
            }
            continue;
        }

        if (xp.match_tag("other_opencl")) {
            other_opencl.clear();
            retval = other_opencl.parse(xp, "/other_opencl");
            if (retval) {
                other_opencl.clear();
            } else {
                other_opencl.is_used = COPROC_USED;
                other_opencls.push_back(other_opencl);
            }
            continue;
        }

        if (xp.match_tag("opencl_cpu_prop")) {
            cpu_opencl.clear();
            retval = cpu_opencl.parse(xp);
            if (retval) {
                cpu_opencl.clear();
            } else {
                cpu_opencl.opencl_prop.is_used = COPROC_IGNORED;
                cpu_opencls.push_back(cpu_opencl);
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
    char quoted_data_dir[MAXPATHLEN+2];
    char data_dir[MAXPATHLEN];
    int retval = 0;

    retval = boinc_delete_file(COPROC_INFO_FILENAME);
    if (retval) {
        msg_printf(0, MSG_INFO,
            "Failed to delete old %s. error code %d",
            COPROC_INFO_FILENAME, retval
        );
    } else {
        for (;;) {
            if (!boinc_file_exists(COPROC_INFO_FILENAME)) break;
            boinc_sleep(0.01);
        }
    }

    // use full path to exe if possible, otherwise keep using argv[0]
    char execpath[MAXPATHLEN];
    if (!get_real_executable_path(execpath, sizeof(execpath))) {
        client_path = execpath;
    }

    boinc_getcwd(data_dir);

#ifdef _WIN32
    strlcpy(quoted_data_dir, "\"", sizeof(quoted_data_dir));
    strlcat(quoted_data_dir, data_dir, sizeof(quoted_data_dir));
    strlcat(quoted_data_dir, "\"", sizeof(quoted_data_dir));
#else
    strlcpy(quoted_data_dir, data_dir, sizeof(quoted_data_dir));
#endif

    if (log_flags.coproc_debug) {
        msg_printf(0, MSG_INFO,
            "[coproc] launching child process at %s",
            client_path
        );
        if (!is_path_absolute(client_path)) {
            msg_printf(0, MSG_INFO,
                "[coproc] relative to directory %s",
                client_dir
            );
        }
        msg_printf(0, MSG_INFO,
            "[coproc] with data directory %s",
            quoted_data_dir
        );
    }

    int argc = 4;
    char* const argv[5] = {
         const_cast<char *>(client_path),
         const_cast<char *>("--detect_gpus"),
         const_cast<char *>("--dir"),
         const_cast<char *>(quoted_data_dir),
         NULL
    };

    retval = run_program(
        client_dir,
        client_path,
        argc,
        argv,
        prog
    );

    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[coproc] run_program of child process returned error %d",
                retval
            );
        }
        return retval;
    }

    int status = 0;
    retval = get_exit_status(prog, status, 10);
    if (retval) {
        char buf[200];
#ifdef _WIN32
        snprintf(buf, sizeof(buf),
            "process exited after the time-out interval elapsed");
#else
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            snprintf(buf, sizeof(buf),
                "process exited with status %d: %s", code, strerror(code)
            );
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            snprintf(buf, sizeof(buf),
                "process was terminated by signal %d", sig
            );
        } else {
            snprintf(buf, sizeof(buf), "unknown status %d", status);
        }
#endif
        msg_printf(0, MSG_INFO, "GPU detection failed: %s", buf);
    }

    return 0;
}

// print descriptions of coprocs specified in cc_config.xml,
// and make sure counts are <= 64
//
void COPROCS::bound_counts() {
    for (int j=1; j<n_rsc; j++) {
        msg_printf(NULL, MSG_INFO, "Coprocessor specified in cc_config.xml. Type %s (%s); count %d",
            coprocs[j].type,
            coprocs[j].non_gpu?"non-GPU":"GPU",
            coprocs[j].count
        );
        if (coprocs[j].count > MAX_COPROC_INSTANCES) {
            msg_printf(NULL, MSG_USER_ALERT,
                "%d instances of %s specified in cc_config.xml; max is %d",
                coprocs[j].count,
                coprocs[j].type,
                MAX_COPROC_INSTANCES
            );
            coprocs[j].count = MAX_COPROC_INSTANCES;
        }
    }
}

void gpu_warning(vector<string> &warnings, const char* msg) {
    fprintf(stderr, "%s\n", msg);
    warnings.push_back(msg);
}

#ifdef __APPLE__
#include "mac/mac_spawn.h"
void COPROC_APPLE::get(vector<string>&) {
    int retval = callPosixSpawn(
        "sh -c '/usr/sbin/system_profiler SPDisplaysDataType > temp'"
    );
    if (retval) return;
    FILE* f = fopen("temp", "r");
    if (!f) return;
    char buf[256], chipset_model[256];
    int n, metalv;
    bool have_model=false, have_ncores=false, have_metalv=false;
    while (fgets(buf, sizeof(buf), f)) {
        if (sscanf(buf, "%*[ ]Chipset Model: %[^\n]", chipset_model) == 1) {
            have_model = true;
        } else if (sscanf(buf, "%*[ ]Total Number of Cores: %d", &n) == 1) {
            have_ncores = true;
        } else if (sscanf(buf, "%*[ ]Metal Support: Metal %d", &metalv) == 1) {
            have_metalv = true;
        }
    }
    fclose(f);

    if (!have_model || !have_ncores || !have_metalv) return;

    // system_profiler reports Intel integrated GPUs on Intel Macs.
    // Ignore them.
    //
    if (!strstr(chipset_model, "Apple")) return;

    COPROC_APPLE ca;
    ca.count = 1;
    safe_strcpy(ca.model, chipset_model);
    ca.ncores = n;
    ca.metal_support = metalv;
    ca.have_metal = true;
    apple_gpus.push_back(ca);
}

void COPROC_APPLE::correlate(bool, vector<int> &ignore_devs) {
    if (!ignore_devs.empty() && ignore_devs[0]==0) return;
    if (apple_gpus.empty()) return;
    *this = apple_gpus[0];
}
#endif
