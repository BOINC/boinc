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

#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "str_replace.h"

#include "vda_lib.h"
#include "stats.h"

void STATS_ITEM::init(const char* n, const char* filename, STATS_KIND k) {
    f = fopen(filename, "w");
    safe_strcpy(name, n);
    kind = k;
    value = 0;
    integral = 0;
    switch (kind) {
    case DISK:
    case NETWORK:
        extreme_val = 0;
        break;
    case FAULT_TOLERANCE:
        extreme_val = INT_MAX;
        break;
    }
    extreme_val_time = 0;
    first = true;
    log_changes = false;
}

void STATS_ITEM::sample(double v, bool collecting_stats, double now) {
    if (value != v && log_changes) {
        char buf[256];
        switch (kind) {
        case DISK:
            sprintf(buf, "%s: %f GB -> %f GB\n", name, value/1e9, v/1e9);
            show_msg(buf);
            break;
        case NETWORK:
            sprintf(buf, "%s: %f Mbps -> %f Mbps\n", name, value/1e6, v/1e6);
            show_msg(buf);
            break;
        case FAULT_TOLERANCE:
            sprintf(buf, "%s: %.0f -> %.0f\n", name, value, v);
            show_msg(buf);
            break;
        }
    }
    double old_val = value;
    value = v;
    if (!collecting_stats) return;
    if (first) {
        first = false;
        prev_t = now;
        return;
    }
    double dt = now - prev_t;
    prev_t = now;
    integral += dt*old_val;
    switch (kind) {
    case DISK:
    case NETWORK:
        if (v > extreme_val) {
            extreme_val = v;
            extreme_val_time = now;
        }
        break;
    case FAULT_TOLERANCE:
        if (v < extreme_val) {
            extreme_val = v;
            extreme_val_time = now;
        }
        break;
    }

    fprintf(f, "%f %f\n", now, old_val);
    fprintf(f, "%f %f\n", now, v);
}

void STATS_ITEM::sample_inc(double inc, bool collecting_stats, double now, const char* reason) {
    sample(value+inc, collecting_stats, now);
    if (reason) {
        printf("   reason: %s\n", reason);
    }
}

void STATS_ITEM::print(double now) {
    sample_inc(0, true, now);
    double dt = now - start_time;
    switch (kind) {
    case DISK:
        printf("    mean: %f GB.  Max: %f GB at %s\n",
            (integral/dt)/1e9, extreme_val/1e9, time_str(extreme_val_time)
        );
        break;
    case NETWORK:
        printf("    mean: %f Mbps.  Max: %f Mbps at %s\n",
            (integral/dt)/1e6, extreme_val/1e6, time_str(extreme_val_time)
        );
        break;
    case FAULT_TOLERANCE:
        printf("    mean: %.2f.  Min: %.0f at %s\n",
            integral/dt, extreme_val, time_str(extreme_val_time)
        );
        break;
    }
}

void STATS_ITEM::print_summary(FILE* fout, double now) {
    double dt = now - start_time;
    switch (kind) {
    case DISK:
        fprintf(fout, "%f\n", integral/dt);
        break;
    case NETWORK:
        fprintf(fout, "%f\n", integral/dt);
        break;
    case FAULT_TOLERANCE:
        fprintf(fout, "%f\n", extreme_val);
        break;
    }
}

