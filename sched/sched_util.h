// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef SCHED_UTIL_H
#define SCHED_UTIL_H

#include "boinc_db.h"

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
extern bool check_stop_sched();
extern void install_stop_signal_handler();
extern int try_fopen(const char* path, FILE*& f, const char* mode);
extern void get_log_path(char*, const char*);

// convert filename to path in a hierarchical directory system
//
extern int dir_hier_path(
    const char* filename, const char* root, int fanout, bool new_hash,
	char* result, bool create=false
);

// convert filename to URL in a hierarchical directory system
//
extern int dir_hier_url(
    const char* filename, const char* root, int fanout, bool new_hash,
	char* result
);

// extract filename from result name, needed for locality scheduling.
//
extern int extract_filename(char* in, char* out);

extern void compute_avg_turnaround(HOST& host, double turnaround);

// used to track execution time of cgi scripts
extern int elapsed_time();


#endif
