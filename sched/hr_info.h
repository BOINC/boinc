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

#ifndef _HR_INFO_
#define _HR_INFO_

#include "hr.h"

// statistics about the host population,
// and about the set of jobs in shared mem.
//
struct PERF_INFO {
    double host_fpops_mean;
    double host_fpops_stdev;
    double wu_fpops_mean;
    double wu_fpops_stdev;

    int write_file();
    int read_file();
};

struct HR_INFO {
    double *rac_per_class[HR_NTYPES];
        // how much RAC per class
    int *max_slots[HR_NTYPES];
        // max # of work array slots per class
    int *cur_slots[HR_NTYPES];
        // estimate of current # of slots used for per class
    double type_weights[HR_NTYPES];
        // total app weight per type
    int slots_per_type[HR_NTYPES];
        // # of slots per type (fixed at start)
    bool type_being_used[HR_NTYPES];
        // whether any app is actually using this HR type
    PERF_INFO perf_info;

    int write_file();
    int read_file();
    void scan_db();
    void allot();
    void init();
    void allocate(int);
    bool accept(int, int);
    void show(FILE*);
};

#define HR_INFO_FILENAME "../hr_info.txt"
#define PERF_INFO_FILENAME "../perf_info.txt"

#endif
