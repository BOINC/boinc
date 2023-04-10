// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// utility functions that don't belong elsewhere

#ifndef BOINC_UTIL_H
#define BOINC_UTIL_H

#include <stdlib.h>
#include <string>
#include <vector>

extern double dtime();
extern double dday();
extern void boinc_sleep(double);
extern void push_unique(std::string, std::vector<std::string>&);

// NOTE: use #include <functional>   to get max,min

#define SECONDS_PER_DAY 86400
#define KILO (1024.)
#define MEGA (1024.*KILO)
#define GIGA (1024.*MEGA)
#define TERA (1024.*GIGA)

static inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}
extern double rand_normal();
extern bool boinc_is_finite(double);

extern void update_average(double, double, double, double, double&, double&);

inline bool in_vector(int n, std::vector<int>& v) {
    for (unsigned int i=0; i<v.size(); i++) {
        if (n == v[i]) return true;
    }
    return false;
}

// used in sim so put it here instead of proc_control
extern int boinc_calling_thread_cpu_time(double&);

// fake a crash
//
extern void boinc_crash();

#ifdef GCL_SIMULATOR
extern double simtime;
#define time(x) ((int)simtime)
#endif

#endif
