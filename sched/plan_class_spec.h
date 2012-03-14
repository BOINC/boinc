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
#include <cstring>
#include <vector>
#include <regex.h>

#include "str_util.h"
#include "parse.h"
#include "util.h"

using std::vector;
using std::string;

struct PLAN_CLASS_SPEC {
    char   name[256];
    int    type; // 0 = CPU, 1 = CUDA, 2 = ATI

    int    min_cuda_compcap; // CUDA-specific
    int    max_cuda_compcap;
    int    min_cuda_version;
    int    max_cuda_version;

    int    min_opencl_version; // OpenCL specific

    int    min_driver_version; // GPU only
    int    max_driver_version; // WARNING: for CUDA the display driver version is only reported on Windows!!
    double min_gpu_ram_mb;
    double gpu_ram_used_mb;
    double gpu_flops;
    double cpu_flops;

    char   project_prefs_tag[256]; // name of a tag from the project specific prefs
    char   gpu_utilization_tag[256]; // the project prefs tag for user-supplied gpu_utilization factor
    double project_prefs_min; // min value this tag can have to allow this plan-class
    double project_prefs_max; // max value this tag can have to allow this plan-class

    std::vector<string> cpu_features;
        // list of CPU features required for this plan class
        // each feature in a separate tag <cpu_feature> ..  </cpu_feature>
        // all features must be lowercase(!)
    int    min_macos_version;
        // min OS version required for this plan class, 0 = no check
    int    max_macos_version;
        // max OS version allowed for this plan class, 0 = no limit
    regex_t os_version_regex;
        // specifying a regexp should work for all OS
    char   os_version_string[256];

    double speedup;
        // speedup over standard "sequential" App for this platform
    double peak_flops_factor;
    double avg_ncpus;
    double max_ncpus;
    double ngpus;
        // defaults to CPU plan classes for type 0, 1 otherwise

    bool check(SCHEDULER_REQUEST& sreq, HOST_USAGE& hu);
    void print(void);
    PLAN_CLASS_SPEC();
};

struct PLAN_CLASS_SPECS {
    std::vector<PLAN_CLASS_SPEC> classes;
    int parse_file(char*);
    int parse_specs(FILE*);
    bool check(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu);
    void print(void);
    PLAN_CLASS_SPECS(){};
};
