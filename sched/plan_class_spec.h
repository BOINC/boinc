// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// Plan class specifications in XML
// See https://github.com/BOINC/boinc/wiki/AppPlanSpec

#include <string>
#include <vector>
#include <regex.h>

// Represents a plan class, as specified in XML
// if you add anything here, initialize it in the constructor
//
struct PLAN_CLASS_SPEC {
    char name[256];
    char gpu_type[256];
    bool cuda;
    bool cal;
    bool opencl;
    bool virtualbox;
    bool wsl;
    bool docker;
    bool is64bit;
    std::vector<std::string> cpu_features;
    double min_ncpus;
    int max_threads;
    double mem_usage_base;
    double mem_usage_per_cpu;
    bool nthreads_cmdline;
    double projected_flops_scale;
    bool have_os_regex;
    regex_t os_regex;
    bool have_cpu_vendor_regex;
    regex_t cpu_vendor_regex;
    bool have_cpu_model_regex;
    regex_t cpu_model_regex;
    double min_os_version;
        // Win versions can be 9 digits; may as well be safe
    double max_os_version;
    int min_android_version;
    int max_android_version;
    int min_libc_version;
        // if WSL: applies to WSL distro
    char project_prefs_tag[256];
    bool have_project_prefs_regex;
    regex_t project_prefs_regex;
    bool project_prefs_default_true;
    double avg_ncpus;
    int min_core_client_version;
    int max_core_client_version;
        // for non-compute-intensive, or override for GPU apps
    int user_id;
    double infeasible_random;
    long min_wu_id;
    long max_wu_id;
    long min_batch;
    long max_batch;

    // GPU apps
    //
    double cpu_frac;
    double min_gpu_ram_mb;
        // for older clients that don't report available RAM
    double gpu_ram_used_mb;
    double gpu_peak_flops_scale;
    double ngpus;
    int min_driver_version;
    int max_driver_version;
    char gpu_utilization_tag[256];
        // the project prefs tag for user-supplied gpu_utilization factor
    double min_gpu_peak_flops;
    double max_gpu_peak_flops;

    // AMD/ATI apps
    //
    bool need_ati_libs;
    bool need_amd_libs;
        // need DLLs w/ ATI or AMD name (default: neither)
    int min_cal_target;
    int max_cal_target;
    bool without_opencl; // restrict to CAL only GPUs

    // NVIDIA apps
    //
    int min_nvidia_compcap;
    int max_nvidia_compcap;

    // CUDA apps
    //
    int min_cuda_version;
    int max_cuda_version;

    // OpenCL apps
    //
    int min_opencl_version;
    int max_opencl_version;
    int min_opencl_driver_revision;
    int max_opencl_driver_revision;
    bool double_precision_fp;

    // Apple GPU
    int min_metal_support;

    // VirtualBox apps
    //
    int min_vbox_version;
    int max_vbox_version;
    vector<int> exclude_vbox_version;
    bool vm_accel_required;

    int parse(XML_PARSER&);
    bool opencl_check(OPENCL_DEVICE_PROP&);
    bool check(SCHEDULER_REQUEST& sreq, HOST_USAGE& hu, const WORKUNIT* wu);
    PLAN_CLASS_SPEC();
};

struct PLAN_CLASS_SPECS {
    std::vector<PLAN_CLASS_SPEC> classes;
    int parse_file(const char*);
    int parse_specs(FILE*);
    bool check(SCHEDULER_REQUEST& sreq, const char* plan_class,
        HOST_USAGE& hu, const WORKUNIT* wu
    );
    bool wu_is_infeasible(const char* plan_class, const WORKUNIT* wu);
    PLAN_CLASS_SPECS(){};
};
