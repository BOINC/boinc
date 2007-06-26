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

#ifndef _PREFS_
#define _PREFS_

#include <cstdio>
#include "miofile.h"
#include "parse.h"

// Global preferences are edited and stored on BOINC servers.
// The native representation of preferences is XML.
// The client maintains the preferences (in XML form)
// and mod time in the state file and in memory.
// It includes these items in each scheduler request message.
// A scheduler reply message may contain a more recent set of preferences.
//

// A struct with one bool per GLOBAL_PREFS field
//
struct GLOBAL_PREFS_MASK {
    bool run_on_batteries;
    bool run_if_user_active;
    bool start_hour;     // 0..23; no restriction if start==end
    bool end_hour;
    bool net_start_hour;     // 0..23; no restriction if start==end
    bool net_end_hour;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool hangup_if_dialed;
    bool dont_verify_images;
    bool work_buf_min_days;
    bool work_buf_additional_days;
    bool max_cpus;
    bool cpu_scheduling_period_minutes;
    bool disk_interval;
    bool disk_max_used_gb;
    bool disk_max_used_pct;
    bool disk_min_free_gb;
    bool vm_max_used_frac;
	bool ram_max_used_busy_frac;
	bool ram_max_used_idle_frac;
    bool idle_time_to_run;
    bool max_bytes_sec_up;
    bool max_bytes_sec_down;
    bool cpu_usage_limit;

    GLOBAL_PREFS_MASK();
    void clear();
    bool are_prefs_set();
    bool are_simple_prefs_set();
};

#define PREFS_CPU       0
#define PREFS_NETWORK   1

struct TIME_PREFS {
    double start_hour;     // 0..24
        // run always if start==end or start==0, end=24
        // don't run at all if start=24, end=0
    double end_hour;
    double net_start_hour;     // 0..24; no restriction if start==end
    double net_end_hour;

    void clear();
    bool suspended(double hour, int which);
};

struct DAY_PREFS {
    bool present;
    int day_of_week;
    TIME_PREFS time_prefs;

    int parse(XML_PARSER&);
};

struct WEEK_PREFS {
    bool present;       // at least one day is present
    DAY_PREFS days[7];  // sun..sat
    void clear();
};

// The following structure is a parsed version of the prefs file
//
struct GLOBAL_PREFS {
    int mod_time;
    bool run_on_batteries;
    bool run_if_user_active;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool hangup_if_dialed;
    bool dont_verify_images;
    TIME_PREFS time_prefs;
    double work_buf_min_days;
    double work_buf_additional_days;
    int max_cpus;
    double cpu_scheduling_period_minutes;
    double disk_interval;
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;
    double vm_max_used_frac;
	double ram_max_used_busy_frac;
	double ram_max_used_idle_frac;
    double idle_time_to_run;
    double max_bytes_sec_up;
    double max_bytes_sec_down;
    double cpu_usage_limit;
    char source_project[256];
    char source_scheduler[256];
    bool host_specific;
    WEEK_PREFS week_prefs;

    GLOBAL_PREFS();
    void defaults();
    void clear_bools();
    int parse(XML_PARSER&, const char* venue, bool& found_venue, GLOBAL_PREFS_MASK& mask);
    int parse_override(XML_PARSER&, const char* venue, bool& found_venue, GLOBAL_PREFS_MASK& mask);
    int parse_file(const char* filename, const char* venue, bool& found_venue);
    int write(MIOFILE&);
    int write_subset(MIOFILE&, GLOBAL_PREFS_MASK&);
    bool suspended_time_of_day(int);
    inline double cpu_scheduling_period() {
        return cpu_scheduling_period_minutes*60;
    }
};

#endif
