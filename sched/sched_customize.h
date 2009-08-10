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

#include "boinc_db.h"
#include "sched_types.h"

// reasons for the planning function to reject a host

#define PLAN_REJECT_CUDA_NO_DEVICE          2
#define PLAN_REJECT_CUDA_VERSION            3
#define PLAN_REJECT_NVIDIA_DRIVER_VERSION   4
#define PLAN_REJECT_CUDA_MEM                5
#define PLAN_REJECT_CUDA_SPEED              6
#define PLAN_REJECT_UNKNOWN                 7
#define PLAN_REJECT_INSUFFICIENT_CPUS       8
#define PLAN_REJECT_CPU_FEATURE             9
#define PLAN_REJECT_NVIDIA_COMPUTE_CAPABILITY             10

#define PLAN_CUDA_MIN_DRIVER_VERSION        17700
#define PLAN_CUDA23_MIN_DRIVER_VERSION        19038
#define PLAN_CUDA_MIN_RAM                   (254*1024*1024)

extern bool wu_is_infeasible_custom(WORKUNIT&, APP&, BEST_APP_VERSION&);
extern int app_plan(SCHEDULER_REQUEST&, char* plan_class, HOST_USAGE&);
extern bool app_plan_uses_gpu(const char* plan_class);
