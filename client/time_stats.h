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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _TIME_STATS_
#define _TIME_STATS_

#include "miofile.h"
#include <vector>

class TIME_STATS {
    bool first;
    int previous_connected_state;
public:
    double last_update;
// we maintain an exponentially weighted average of these quantities:
    double on_frac;
        // the fraction of total time this host runs the core client
    double connected_frac;
        // of the time this host runs the core client,
        // the fraction it is connected to the Internet,
        // or -1 if not known
    double active_frac;
        // of the time this host runs the core client,
        // the fraction it is enabled to work
        // (as determined by preferences, manual suspend/resume, etc.)
    double cpu_efficiency;
        // The ratio between CPU time accumulated by BOINC apps
        // and the wall time those apps are scheduled at the OS level.
        // May be less than one if
        // 1) apps page or do I/O
        // 2) other CPU-intensive apps run

    FILE* time_stats_log;
    double inactive_start;

    void update(int suspend_reason);
    void update_cpu_efficiency(double cpu_wall_time, double cpu_time);

    TIME_STATS();
    int write(MIOFILE&, bool to_server);
    int parse(MIOFILE&);

    void log_append(const char*, double);
    void log_append_net(int);
    void trim_stats_log();
    void get_log_after(double, MIOFILE&);
    void start();
    void quit();
};

#endif
