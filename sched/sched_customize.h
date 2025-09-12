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

#ifndef BOINC_SCHED_CUSTOMIZE_H
#define BOINC_SCHED_CUSTOMIZE_H

#include "boinc_db.h"
#include "sched_types.h"

// Scheduler parameters that may need to modified by the project
// and their default values.
//
// ------ Shared Memory Parameters -----------
// #define MAX_PLATFORMS        50
// #define MAX_APPS             10
// #define MAX_APP_VERSIONS     50
// #define MAX_ASSIGNMENTS      10
// #define MAX_WU_RESULTS       100
//
// ------ Default Plan Class Parameters --------------
// #define ATI_MIN_RAM                  256*MEGA
// #define OPENCL_ATI_MIN_RAM           256*MEGA
// #define CUDA_MIN_RAM                 256*MEGA
// #define CUDAFERMI_MIN_RAM            384*MEGA
// #define OPENCL_NVIDIA_MIN_RAM        384*MEGA
// #define OPENCL_INTEL_GPU_MIN_RAM     256*MEGA
// #define OPENCL_APPLE_GPU_MIN_RAM     256*MEGA
//
// #define CUDA_MIN_DRIVER_VERSION              17700
// #define CUDA23_MIN_CUDA_VERSION              2030
// #define CUDA23_MIN_DRIVER_VERSION            19038
// #define CUDA3_MIN_CUDA_VERSION               3000
// #define CUDA3_MIN_DRIVER_VERSION             19500
// #define CUDA_OPENCL_MIN_DRIVER_VERSION       19713
// #define CUDA_OPENCL_101_MIN_DRIVER_VERSION   28013
//

// this keeps track of the project's lowest and highest requirements
// for a GPU type, so that we can send users an appropriate message
//
struct GPU_REQUIREMENTS {
    double min_ram;
    double opt_ram;
    int min_driver_version;
    int opt_driver_version;

    void clear() {
        min_ram = opt_ram = 0;
        min_driver_version = opt_driver_version = 0;
    }
    void update(int version, double ram) {
        if (min_driver_version) {
            if (version < min_driver_version) {
                min_driver_version = version;
            }
        } else {
            min_driver_version = version;
        }
        if (version > opt_driver_version) {
            opt_driver_version = version;
        }
        if (min_ram) {
            if (ram < min_ram) {
                min_ram = ram;
            }
        } else {
            min_ram = ram;
        }
        if (ram > opt_ram) {
            opt_ram = ram;
        }
    }
};

extern GPU_REQUIREMENTS gpu_requirements[NPROC_TYPES];

extern bool wu_is_infeasible_custom(WORKUNIT&, APP&, BEST_APP_VERSION&);
extern bool app_plan(
    SCHEDULER_REQUEST&, const char* plan_class, HOST_USAGE&, const WORKUNIT* wu
);
extern void handle_file_xfer_results();
extern bool wu_restricted_plan_class;

// Suppose we have a computation that uses two devices alternately.
// The devices have speeds s1 and s2.
// The fraction of work done on device 1 is frac.
//
// This function returns:
// 1) the overall speed
// 2) the utilization of device 1, which is always in (0, 1).
//
static inline void coproc_perf(
    double s1, double s2, double frac,
    double& speed, double& u1
) {
    double y = (frac*s2 + (1-frac)*s1);
    speed = s1*s2/y;
        // do the math
    u1 = frac*s2/y;
}

#endif
