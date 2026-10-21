// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#ifndef BOINC_PREFS_H
#define BOINC_PREFS_H

#include <cstdio>

#include "common_defs.h"
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
    bool battery_charge_min_pct = false;
    bool battery_max_temperature = false;
    bool confirm_before_connecting = false;
    bool cpu_scheduling_period_minutes = false;
    bool cpu_usage_limit = false;
    bool daily_xfer_limit_mb = false;
    bool daily_xfer_period_days = false;
    bool disk_interval = false;
    bool disk_max_used_gb = false;
    bool disk_max_used_pct = false;
    bool disk_min_free_gb = false;
    bool dont_verify_images = false;
    bool end_hour = false;
    bool hangup_if_dialed = false;
    bool idle_time_to_run = false;
    bool leave_apps_in_memory = false;
    bool max_bytes_sec_down = false;
    bool max_bytes_sec_up = false;
    bool max_ncpus = false;
    bool max_ncpus_pct = false;
    bool net_end_hour = false;
    bool net_start_hour = false;
    bool network_wifi_only = false;
    bool niu_cpu_usage_limit = false;
    bool niu_max_ncpus_pct = false;
    bool niu_suspend_cpu_usage = false;
    bool ram_max_used_busy_frac = false;
    bool ram_max_used_idle_frac = false;
    bool run_if_user_active = false;
    bool run_gpu_if_user_active = false;
    bool run_on_batteries = false;
    bool start_hour = false;
    bool suspend_cpu_usage = false;
    bool suspend_if_no_recent_input = false;
    bool vm_max_used_frac = false;
    bool work_buf_additional_days = false;
    bool work_buf_min_days = false;

    GLOBAL_PREFS_MASK(DUMMY_TYPE){}
    void clear() {
        static const GLOBAL_PREFS_MASK x(DUMMY);
        *this = x;
    }
    GLOBAL_PREFS_MASK() {
        clear();
    }
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
    TIME_SPAN() : present(false), start_hour(0), end_hour(0) {}
    TIME_SPAN(double start, double end) : present(false), start_hour(start), end_hour(end) {}

    bool suspended(double hour) const;
    TimeMode mode() const;
};


struct WEEK_PREFS {
    TIME_SPAN days[7];

    WEEK_PREFS() {}
    void clear() {
        static const WEEK_PREFS init;
        *this = init;
    }

    void set(int day, double start, double end);
    void set(int day, TIME_SPAN* time);
    void unset(int day);
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

    double battery_charge_min_pct;      // Android
    double battery_max_temperature;     // Android
    bool confirm_before_connecting;
    double cpu_scheduling_period_minutes;
        // length of a time slice.
        // scheduling happens more often.
    TIME_PREFS cpu_times;
    double cpu_usage_limit;
        // for CPU throttling.  This is a percentage 0..100
    double daily_xfer_limit_mb;
    int daily_xfer_period_days;
    double disk_interval;
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;
    bool dont_verify_images;
    bool hangup_if_dialed;
    double idle_time_to_run;
    bool leave_apps_in_memory;
    double max_bytes_sec_down;
    double max_bytes_sec_up;
    int max_ncpus;
    double max_ncpus_pct;
    TIME_PREFS net_times;
    bool network_wifi_only;
        // introduced with Android. Do network communication only when on Wifi,
        // not on public cell networks.
        // CAUTION: this only applies to file transfers.
        // scheduler RPCs are made regardless of this preference.
    double niu_cpu_usage_limit;
    double niu_max_ncpus_pct;
    double niu_suspend_cpu_usage;
    double ram_max_used_busy_frac;
    double ram_max_used_idle_frac;
    bool run_gpu_if_user_active;
    bool run_if_user_active;
    bool run_on_batteries;
        // poorly named; what it really means is:
        // if false, suspend while on batteries
    double suspend_cpu_usage;
    double suspend_if_no_recent_input;
    double vm_max_used_frac;
    double work_buf_additional_days;
    double work_buf_min_days;

    char source_project[256];
    char source_scheduler[256];
    bool host_specific;
        // an account manager can set this; if set, don't propagate
    bool override_file_present;
    bool need_idle_state;
        // whether idle state makes any difference

    GLOBAL_PREFS();
    void defaults();
    void enabled_defaults();
    void init();
    void init_bools();
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
    static double parse_mod_time(const char*);
    bool get_need_idle_state(bool have_gpu) {
        // is any pref set that causes different behavior if user is active?
        //
        if (!run_if_user_active) return true;
        if (have_gpu && !run_gpu_if_user_active) return true;
        if (suspend_if_no_recent_input) return true;
        if (niu_cpu_usage_limit && niu_cpu_usage_limit != cpu_usage_limit) return true;
        if (niu_max_ncpus_pct && niu_max_ncpus_pct != max_ncpus_pct) return true;
        if (niu_suspend_cpu_usage && niu_suspend_cpu_usage != suspend_cpu_usage) return true;
        if (ram_max_used_busy_frac != ram_max_used_idle_frac) return true;
        return false;
    }
};

#endif
