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

#include <vector>

#include "boinc_db.h"

#define ERROR_RATE_INIT 0.1
    // the initial error rate of a host or app version

#define MIN_HOST_SAMPLES  10
    // use host scaling only if have this many samples for host
#define MIN_VERSION_SAMPLES   100
    // update a version's scale only if it has this many samples

// parameters for maintaining averages.
// per-host averages respond faster to change

#define HAV_AVG_THRESH  20
#define HAV_AVG_WEIGHT  .01
#define HAV_AVG_LIMIT   10

#define AV_AVG_THRESH   100
#define AV_AVG_WEIGHT   .001
#define AV_AVG_LIMIT    10

extern double fpops_to_credit(double fpops);
    // credit that should be granted for a given number of
    // floating-point ops
extern double cpu_time_to_credit(double cpu_time, double cpu_flops_sec);
extern int grant_credit(DB_HOST& host, double start_time, double credit);

extern int update_av_scales(struct SCHED_SHMEM*);
extern int assign_credit_set(
    WORKUNIT&, std::vector<RESULT>&, DB_APP&, std::vector<DB_APP_VERSION>&,
    std::vector<DB_HOST_APP_VERSION>&,
    double max_granted_credit, double& credit
);

extern void got_error(DB_HOST_APP_VERSION&);

extern int hav_lookup(DB_HOST_APP_VERSION& hav, int hostid, int avid);

extern int write_modified_app_versions(
    std::vector<DB_APP_VERSION>& app_versions
);
