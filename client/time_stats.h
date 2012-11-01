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

#ifndef _TIME_STATS_
#define _TIME_STATS_

#include <vector>

#include "miofile.h"
#include "common_defs.h"

struct CLIENT_TIME_STATS : TIME_STATS {
    bool first;
    int previous_connected_state;

    double last_update;

    FILE* time_stats_log;
    double inactive_start;

    void update(int suspend_reason, int gpu_suspend_reason);

    void init();
    int write(MIOFILE&, bool to_remote);
    int parse(XML_PARSER&);

    double availability_frac(int rsc_type) {
        double x;
        if (rsc_type == 0) {
            x = on_frac*active_frac;
        } else {
            x = on_frac*gpu_active_frac;
        }
        return x>0?x:1;
    }
    void log_append(const char*, double);
    void log_append_net(int);
    void trim_stats_log();
    void get_log_after(double, MIOFILE&);
    void start();
    void quit();
};

#endif
