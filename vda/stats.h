// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// keeps track of a time-varying property of a file
// (server disk usage, up/download rate, fault tolerance level)
//

#ifndef _STATS_
#define _STATS_

#include <stdio.h>

typedef enum {DISK, NETWORK, FAULT_TOLERANCE} STATS_KIND;

struct STATS_ITEM {
    STATS_KIND kind;
    double value;
    double integral;
    double extreme_val;
    double extreme_val_time;
    double prev_t;
    double start_time;
    bool first;
    char name[256];
    FILE* f;
    bool log_changes;

    void init(const char* n, const char* filename, STATS_KIND k);
    void sample(double v, bool collecting_stats, double now);
    void sample_inc(double inc, bool collecting_stats, double now, const char* reason=NULL);
    void print(double now);
    void print_summary(FILE* f, double now);
};

#endif
