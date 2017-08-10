// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// server utility functions that refer to the DB

#ifndef BOINC_SCHED_UTIL_H
#define BOINC_SCHED_UTIL_H

#include "boinc_db_types.h"
#include "util.h"

#include "sched_util_basic.h"

extern void compute_avg_turnaround(HOST& host, double turnaround);

struct PERF_INFO {
    double host_fpops_mean;
    double host_fpops_stddev;
    double host_fpops_50_percentile;
    double host_fpops_95_percentile;

    int get_from_db();
};

// Return a value for host_app_version.app_version_id.
// if the app version is anonymous platform,
// make a "pseudo ID" that combines the app ID and the resource type
// else just used the app_version ID
//
inline DB_ID_TYPE generalized_app_version_id(
    DB_ID_TYPE avid, DB_ID_TYPE appid
) {
    if (avid < 0) {
        return appid*1000000 - avid;
    }
    return avid;
}

extern int count_workunits(long&, const char* query);
extern int count_unsent_results(long&, DB_ID_TYPE appid, int size_class=-1);
extern int restrict_wu_to_user(WORKUNIT& wu, DB_ID_TYPE userid);
extern int restrict_wu_to_host(WORKUNIT& wu, DB_ID_TYPE hostid);
extern int min_transition_time(double&);

#endif
