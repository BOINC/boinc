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

#ifndef _PREFS_
#define _PREFS_

#include <cstdio>

#include "miofile.h"
#include "parse.h"

// global prefs are maintained as follows:
// 1) a "global_prefs.xml" file, which stores the "network" prefs;
//      it's maintained by communication with scheduling servers
//      or project managers
// 2) a "global_prefs_override.xml" file, which can be edited manually
//      or via a GUI.
//      For the prefs that it specifies, it overrides the network prefs.

// A struct with one bool per pref.
// This is passed in GUI RPCs (get/set_global_prefs_override_struct)
// to indicate which prefs are (or should be) specified in the override file
//
struct GLOBAL_PREFS_MASK {
    bool run_on_batteries;
    bool run_if_user_active;
    bool run_gpu_if_user_active;
    bool idle_time_to_run;
    bool suspend_if_no_recent_input;
    bool suspend_cpu_usage;
    bool start_hour;
    bool end_hour;
    bool net_start_hour;
    bool net_end_hour;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool hangup_if_dialed;
    bool dont_verify_images;
    bool work_buf_min_days;
    bool work_buf_additional_days;
    bool max_ncpus_pct;
    bool max_ncpus;
    bool cpu_scheduling_period_minutes;
    bool disk_interval;
    bool disk_max_used_gb;
    bool disk_max_used_pct;
    bool disk_min_free_gb;
    bool vm_max_used_frac;
	bool ram_max_used_busy_frac;
	bool ram_max_used_idle_frac;
    bool max_bytes_sec_up;
    bool max_bytes_sec_down;
    bool cpu_usage_limit;
    bool daily_xfer_limit_mb;
    bool daily_xfer_period_days;
    bool network_wifi_only;

    GLOBAL_PREFS_MASK();
    void clear();
    bool are_prefs_set();
    bool are_simple_prefs_set();
    void set_all();
};


// 0..24
// run always if start==end or start==0, end=24
// don't run at all if start=24, end=0
//
struct TIME_SPAN {
    bool present;
    double start_hour;
    double end_hour;

    enum TimeMode {
        Always = 7000,
        Never,
        Between
    };
    TIME_SPAN() : start_hour(0), end_hour(0) {}
    TIME_SPAN(double start, double end) : start_hour(start), end_hour(end) {}

    bool suspended(double hour) const;
    TimeMode mode() const;
};


struct WEEK_PREFS {
    TIME_SPAN days[7];

    void clear() {
        memset(this, 0, sizeof(WEEK_PREFS));
    }
    WEEK_PREFS() {
        clear();
    }

    void set(int day, double start, double end);
    void set(int day, TIME_SPAN* time);
    void unset(int day);

protected:
    void copy(const WEEK_PREFS& original);
};


struct TIME_PREFS : public TIME_SPAN {
    WEEK_PREFS week;

    TIME_PREFS() {}
    TIME_PREFS(double start, double end) {
        start_hour = start;
        end_hour = end;
    }
    
    void clear();
    bool suspended(double t);
    
};


struct GLOBAL_PREFS {
    double mod_time;
    bool run_on_batteries;
        // poorly named; what it really means is:
        // if false, suspend while on batteries
    bool run_if_user_active;
    bool run_gpu_if_user_active;
    double idle_time_to_run;
    double suspend_if_no_recent_input;
    double suspend_cpu_usage;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool hangup_if_dialed;
    bool dont_verify_images;
    TIME_PREFS cpu_times;
    TIME_PREFS net_times;
    double work_buf_min_days;
    double work_buf_additional_days;
    double max_ncpus_pct;
    int max_ncpus;
    double cpu_scheduling_period_minutes;
        // length of a time slice.
        // scheduling happens more often.
    double disk_interval;
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;
    double vm_max_used_frac;
	double ram_max_used_busy_frac;
	double ram_max_used_idle_frac;
    double max_bytes_sec_up;
    double max_bytes_sec_down;
    double cpu_usage_limit;
    double daily_xfer_limit_mb;
    int daily_xfer_period_days;
    char source_project[256];
    char source_scheduler[256];
    bool host_specific;
        // an account manager can set this; if set, don't propagate
    bool override_file_present;
    bool network_wifi_only;
        // introduced with Android. Do network communication only when on Wifi,
        // not on public cell networks.
        // CAUTION: this only applies to file transfers.
        // scheduler RPCs are made regardless of this preference.

    GLOBAL_PREFS();
    void defaults();
    void init();
    void clear_bools();
    int parse(XML_PARSER&, const char* venue, bool& found_venue, GLOBAL_PREFS_MASK& mask);
    int parse_day(XML_PARSER&);
    int parse_override(XML_PARSER&, const char* venue, bool& found_venue, GLOBAL_PREFS_MASK& mask);
    int parse_file(const char* filename, const char* venue, bool& found_venue);
    int write(MIOFILE&);
    int write_subset(MIOFILE&, GLOBAL_PREFS_MASK&);
    void write_day_prefs(MIOFILE&);
    inline double cpu_scheduling_period() {
        return cpu_scheduling_period_minutes*60;
    }
};

#endif
