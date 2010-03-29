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

// Historical note: named after Jeff Cobb
//
#define COBBLESTONE_FACTOR 100.0

#define ERROR_RATE_INIT 0.1
    // the initial error rate of a host or app version

extern void compute_credit_rating(HOST&);
extern double credit_multiplier(int, time_t);
extern double fpops_to_credit(double fpops, double intops);
    // credit that should be granted for a given number of
    // floating-point and integer ops
extern int update_credit_per_cpu_sec(
    double credit, double cpu_time, double& credit_per_cpu_sec
);
extern int grant_credit(
    DB_HOST& host, double start_time, double cpu_time, double credit
);

extern int update_av_scales(struct SCHED_SHMEM*);
extern int assign_credit_set(
    WORKUNIT&, std::vector<RESULT>&, DB_APP&, std::vector<DB_APP_VERSION>&
);

extern int update_host_scale_times(
    struct SCHED_SHMEM*, std::vector<RESULT>& results, int hostid
);

// if the result was anonymous platform,
// make a "pseudo ID" that combines the app ID and the resource type
//
inline int generalized_app_version_id(int avid, int appid) {
    if (avid < 0) {
        return appid*1000000 - avid;
    }
    return avid;
}

extern int host_scale_probation(
    DB_HOST& host, int appid, int app_version_id, double latency_bound
);

extern int write_modified_app_versions(std::vector<DB_APP_VERSION>& app_versions);
