// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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

#ifndef _EDF_SIM_H
#define _EDF_SIM_H

#include <vector>

struct IP_RESULT {
    char name[256];
    double computation_deadline;
    double report_deadline;
    double cpu_time_remaining;
    int parse(FILE*);
    bool misses_deadline;
        // Whether or not the result would have missed its deadline,
        // independent of any newly scheduled result
        // Used to determine if late results will complete even later
    double estimated_completion_time;

    IP_RESULT() {}
    IP_RESULT(const char* n, double d, double c) {
        strcpy(name, n);
        report_deadline = d;
        computation_deadline = d;
        cpu_time_remaining = c;
        misses_deadline = false;
        estimated_completion_time = 0;
    }
};

extern void init_ip_results(double, int, std::vector<IP_RESULT>&);
extern bool check_candidate(IP_RESULT&, int, std::vector<IP_RESULT>);

#endif
