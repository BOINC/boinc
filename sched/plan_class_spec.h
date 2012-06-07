// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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


// configurable app plan functions.
// see http://boinc.berkeley.edu/trac/wiki/AppPlanConfig

#include <string>
#include <vector>
#include <regex.h>

struct PLAN_CLASS_SPEC {
    char name[256];
    char gpu_type[256];
    bool cuda;
    bool cal;
    bool opencl;
    bool virtualbox;
    bool is64bit;
    std::vector<std::string> cpu_features;
    double min_ncpus;
    int max_threads;
    double projected_flops_scale;
    bool have_os_regex;
    regex_t os_regex;
    char project_prefs_tag[256];
    bool have_project_prefs_regex;
    regex_t project_prefs_regex;
    double avg_ncpus;
        // for non-compute-intensive, or override for GPU apps

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

    // AMD/ATI apps
    //
    bool need_ati_libs;
        // need DLLs w/ ati name (default: amd)
    int min_cal_target;
    int max_cal_target;

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

    // VirtualBox apps
    //
    int min_vbox_version;
    int max_vbox_version;

    int parse(XML_PARSER&);
    bool check(SCHEDULER_REQUEST& sreq, HOST_USAGE& hu);
    PLAN_CLASS_SPEC();
};

struct PLAN_CLASS_SPECS {
    std::vector<PLAN_CLASS_SPEC> classes;
    int parse_file(const char*);
    int parse_specs(FILE*);
    bool check(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu);
    PLAN_CLASS_SPECS(){};
};
