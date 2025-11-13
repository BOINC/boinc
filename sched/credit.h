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

#define AVG_CPU_FPOPS   4.3e9
    // use this if host didn't report valid p_fpops

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

// variant of DB_APP_VERSION used by the validator
//
struct DB_APP_VERSION_VAL : DB_APP_VERSION {
    std::vector<double>pfc_samples;
    std::vector<double>credit_samples;
    std::vector<double>credit_times;
};

extern double fpops_to_credit(double fpops);
    // credit that should be granted for a given number of
    // floating-point ops
extern double cpu_time_to_credit(double cpu_time, double cpu_flops_sec);
extern int grant_credit(DB_HOST& host, double start_time, double credit);

extern int update_av_scales(struct SCHED_SHMEM*);
extern int assign_credit_set(
    WORKUNIT&, std::vector<RESULT>&, DB_APP&, std::vector<DB_APP_VERSION_VAL>&,
    std::vector<DB_HOST_APP_VERSION>&,
    double max_granted_credit, double& credit
);

extern void got_error(DB_HOST_APP_VERSION&);

extern int hav_lookup(DB_HOST_APP_VERSION& hav, DB_ID_TYPE hostid, DB_ID_TYPE avid);

extern int write_modified_app_versions(
    std::vector<DB_APP_VERSION_VAL>& app_versions
);

extern int grant_credit_by_app(RESULT& result, double credit);
extern double low_average(std::vector<double>&);
