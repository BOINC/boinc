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

#ifndef _EDF_SIM_H
#define _EDF_SIM_H

#include <cstring>
#include <vector>

struct IP_RESULT {
    char name[256];
    double computation_deadline;
    double report_deadline;
    double cpu_time_remaining;
    int parse(FILE*);
        /// Whether or not the result would have missed its deadline,
        /// independent of any newly scheduled result
        /// Used to determine if late results will complete even later
    bool misses_deadline;
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
