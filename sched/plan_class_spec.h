// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
//

/*

Possible tags:

  <name>                Name of the plan class, string without spaces, exact match
  <type>                type of the plan class, currently understood are 0 = CPU and 1 = CUDA
                        2 = ATI is suggested, but not supported in current code.
                        May be specified numerically (0, 1, 2) or as token (CPU, CUDA, ATI)
                        3 = NVidia OpenCL, 4 = ATI OpenCL.

  <min_cuda_compcap>    CUDA only: minimum compute capability
  <max_cuda_compcap>    CUDA only: maximum compute capability, set to 9999 to exclude emulation device
  <min_cuda_version>    CUDA only: minimum CUDA version
  <max_cuda_version>    CUDA only: maximum CUDA version
  <min_opencl_version>  OpenCL only: minimum required device version
        IMPORTANT NOTE: the NVidia display driver version is only reported by Windows
                        core clients. Mac and Linux clients only report the CUDA version. The display
                        driver version will allways be 0 (zero) for these clients, hence this should
                        NEVER be used to restrict a plan class for platforms other than Windows!
                NOTE#2: Reporting driver version has been added to recent Mac Clients (6.13.x)
                NOTE#3: the driver_version may be specified negative. In this case its _absolute_
                        value is compared to the client's driver version, but only if this is reported.
                        If the Client doesn't report a driver version, this check is skipped.
  <min_driver_version>  GPU only: minimum display driver version
  <max_driver_version>  GPU only: minimum display driver version
  <min_gpu_ram_mb>      GPU only: minimum required amount of video RAM (in MB)
  <gpu_ram_used_mb>     GPU only: video RAM a task will actually use (in MB)

  <project_prefs_tag>   name of a tag from the project specific preferences that can be used to enable
                        or disable this plan-class (scanned as double, 0.0 if not present)
  <project_prefs_min>   min value this tag can have to allow this plan-class
  <project_prefs_max>   max value this tag can have to allow this plan-class
  <gpu_utilization_tag> name of a tag from the project specific preferences which values is a custom
                        gpu utilization factor supplied by the user. The 'ngpus' setting of the plan class
                        will be multiplied by this when present. 
  <cpu_feature>         CPU features required for this plan class. Multiple tags allowed. All features
                        lowercase (e.g. sse2, altivec). Both host.p_features and host.p_model are checked

  <min_macos_version>   Deprecated: min Darwin version required for this plan class, 0 = no check,
                        numeric: 1000 * major version + 100 * minor version + patchlevel
  <max_macos_version>   Deprecated: max Darwin version allowed for this plan class, 0 = no limit

  <os_version>          regexp specifying an OS version (should work for all OS that way)

  <speedup>             speedup over standard "sequential" App for this platform
  <peak_flops_factor>   correct the (theoretical) peak flops by that factor (assumed efficency)
  <avg_ncpus>
  <max_ncpus>
  <ngpus>               GPU only: number / fraction of GPUs used, defaults 0 for CPU plan classes, 1 otherwise
                        if ngpus < 0, set ncudas by the fraction of the total video RAM a tasks would take
  <gpu_flops>
  <cpu_flops>           GPU only: if both gpu_flops and cpu_flops are set, compuute
                        hu.avg_ncpus = avg_ncpus * sreq.host.p_fpops / cpu_flops and
                        projected_flops = cp.peak_flops / gpu_flops * speedup + 1.0 / hu.avg_ncpus;

Here is an example of three plan class specifications, two CPU, one CUDA.

<plan_classes>

  <plan_class>
    <name>                CUDA32          </name>
    <min_cuda_compcap>    100             </min_cuda_compcap>
    <max_cuda_compcap>    9999            </max_cuda_compcap>
    <min_cuda_version>    3020            </min_cuda_version>
    <min_driver_version>  26000           </min_driver_version>
    <max_driver_version>  99999           </max_driver_version>
    <min_gpu_ram_mb>      300             </min_gpu_ram_mb>
    <gpu_ram_used_mb>     300             </gpu_ram_used_mb>
    <speedup>             10.0            </speedup>
    <avgncpus>            0.2             </avgncpus>
  </plan_class>

  <plan_class>
    <name>                BRP3SSE         </name>
    <cpu_feature>         sse             </cpu_feature>
    <project_prefs_tag>   also_run_cpu    </project_prefs_tag>
    <project_prefs_max>   0               </project_prefs_max>
  </plan_class>

  <plan_class>
    <name>                ALTIVEC         </name>
    <cpu_feature>         altivec         </cpu_feature>
    <min_macos_version>   80000           </min_macos_version>
    <speedup>             1.4             </speedup>
  </plan_class>

</plan_classes>

*/

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
  int    type; /* 0 = CPU, 1 = CUDA, 2 = ATI */

  int    min_cuda_compcap; /* CUDA-specific */
  int    max_cuda_compcap;
  int    min_cuda_version;
  int    max_cuda_version;

  int    min_opencl_version; /* OpnCL specific */

  int    min_driver_version; /* GPU only */
  int    max_driver_version; /* WARNING: for CUDA the display driver version is only reported on Windows!! */
  double min_gpu_ram_mb;
  double gpu_ram_used_mb;
  double gpu_flops;
  double cpu_flops;

  char   project_prefs_tag[256]; /* name of a tag from the project specific prefs */
  char   gpu_utilization_tag[256]; /* the project prefs tag for user-supplied gpu_utilization factor */
  double project_prefs_min; /* min value this tag can have to allow this plan-class */
  double project_prefs_max; /* max value this tag can have to allow this plan-class */

  std::vector<string> cpu_features; /* list of CPU features required for this plan class
				       each feature in a separate tag <cpu_feature> ..  </cpu_feature>
				       all features must be lowercase(!) */
  int    min_macos_version; /* min OS version required for this plan class, 0 = no check */
  int    max_macos_version; /* max OS version allowed for this plan class, 0 = no limit */
  regex_t os_version_regex; /* specifying a regexp should work for all OS */
  char   os_version_string[256];

  double speedup; /* speedup over standard "sequential" App for this platform */
  double peak_flops_factor;
  double avg_ncpus;
  double max_ncpus;
  double ngpus; /* defaults to CPU plan classes for type 0, 1 otherwise */

  bool check(SCHEDULER_REQUEST& sreq, HOST_USAGE& hu);
  void print(void);
  PLAN_CLASS_SPEC();
};

struct PLAN_CLASS_SPECS {
  std::vector<PLAN_CLASS_SPEC> classes;
  bool parsed;
  int parse_file(char*);
  int parse_specs(FILE*);
  bool check(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu);
  void print(void);
  PLAN_CLASS_SPECS();
};
