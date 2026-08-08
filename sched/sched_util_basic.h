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

// general back-end utility functions (not scheduler-specific)
// No database-related stuff here; put that in sched_util.h

#ifndef BOINC_SCHED_UTIL_BASIC_H
#define BOINC_SCHED_UTIL_BASIC_H

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
extern int get_log_path(char*, const char*);

// check the status of a file with the given path and its .md5 in download hier
//
extern int check_download_file(const char* file_path, const char* dl_hier_path);

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

// returns zero if we get lock on file with file descriptor fd.
// returns < 0 if error
// returns PID > 0 if another process has lock
//
extern int mylockf(int fd);

// returns zero if there is no write lock on file with file descriptor fd.
// returns < 0 if error
// returns PID > 0 of the process that has the lock
//
extern int checklockf(int fd);

// return true if x is -y or --y (for argv processing)
//
extern bool is_arg(const char*, const char*);

extern int plan_class_to_proc_type(const char* plan_class);

#ifdef GCL_SIMULATOR
extern void simulator_signal_handler(int signum);
extern void continue_simulation(const char *daemonname);
#endif

#endif
