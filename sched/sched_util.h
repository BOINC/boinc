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

// general back-end utility functions (not scheduler-specific)

#ifndef SCHED_UTIL_H
#define SCHED_UTIL_H

#include "boinc_db_types.h"
#include "util.h"

// "average credit" uses an exponential decay so that recent
// activity is weighted more heavily.
// CREDIT_HALF_LIFE is the "half-life" period:
// the average decreases by 1/2 if idle for this period.
//
#define SECONDS_IN_DAY (3600*24)
#define CREDIT_HALF_LIFE  (SECONDS_IN_DAY*7)

extern void write_pid_file(const char* filename);
extern void set_debug_level(int);
extern void check_stop_daemons();
extern void daemon_sleep(int);
extern bool check_stop_sched();
extern void install_stop_signal_handler();
extern int try_fopen(const char* path, FILE*& f, const char* mode);
extern void get_log_path(char*, const char*);

// convert filename to path in a hierarchical directory system
//
extern int dir_hier_path(
    const char* filename, const char* root, int fanout,
    char* result, bool create=false
);

// convert filename to URL in a hierarchical directory system
//
extern int dir_hier_url(
    const char* filename, const char* root, int fanout,
    char* result
);

extern void compute_avg_turnaround(HOST& host, double turnaround);

struct PERF_INFO {
    double host_fpops_mean;
    double host_fpops_stddev;
    double host_fpops_50_percentile;
    double host_fpops_95_percentile;

    int get_from_db();
};

// returns zero if we get lock on file with file descriptor fd.
// returns < 0 if error
// returns PID > 0 if another process has lock
//
extern int mylockf(int fd);

extern int count_workunits(int&, const char* query);
extern int count_unsent_results(int&, int appid);

// Return a value for host_app_version.app_version_id.
// if the app version is anonymous platform,
// make a "pseudo ID" that combines the app ID and the resource type
// else just used the app_version ID
//
inline int generalized_app_version_id(int avid, int appid) {
    if (avid < 0) {
        return appid*1000000 - avid;
    }
    return avid;
}

// return true if x is -y or --y (for argv processing)
//
extern bool is_arg(const char*, const char*);

extern bool app_plan_uses_gpu(const char* plan_class);

extern int restrict_wu_to_user(WORKUNIT& wu, int userid);

#ifdef GCL_SIMULATOR
extern void simulator_signal_handler(int signum);
extern void continue_simulation(const char *daemonname);
#endif

#endif
