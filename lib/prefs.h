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

#ifndef BOINC_PREFS_H
#define BOINC_PREFS_H

#include <cstdio>
#include <map>

#include "miofile.h"
#include "cc_config.h"
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
    bool battery_charge_min_pct;
    bool battery_max_temperature;
    bool confirm_before_connecting;
    bool cpu_scheduling_period_minutes;
    bool cpu_usage_limit;
    bool daily_xfer_limit_mb;
    bool daily_xfer_period_days;
    bool disk_interval;
    bool disk_max_used_gb;
    bool disk_max_used_pct;
    bool disk_min_free_gb;
    bool dont_verify_images;
    bool end_hour;
    bool hangup_if_dialed;
    bool idle_time_to_run;
    bool leave_apps_in_memory;
    bool max_bytes_sec_down;
    bool max_bytes_sec_up;
    bool max_ncpus;
    bool max_ncpus_pct;
    bool net_end_hour;
    bool net_start_hour;
    bool network_wifi_only;
    bool ram_max_used_busy_frac;
    bool ram_max_used_idle_frac;
    bool run_if_user_active;
    bool run_gpu_if_user_active;
    bool run_on_batteries;
    bool start_hour;
    bool suspend_cpu_usage;
    bool suspend_if_no_recent_input;
    bool vm_max_used_frac;
    bool work_buf_additional_days;
    bool work_buf_min_days;

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
    TIME_SPAN() : present(false), start_hour(0), end_hour(0) {}
    TIME_SPAN(double start, double end) : present(false), start_hour(start), end_hour(end) {}

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
    void write(MIOFILE&); 
    int parse(XML_PARSER&);
};


struct GLOBAL_PREFS {
    double mod_time;

    double battery_charge_min_pct;
    double battery_max_temperature;
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
};

////////////////// new prefs starts here /////////////////

// basic idea: the "prefs dictionary" is a map string -> double.
// Entries are created for preference terms.
// Some of the entries have standardized names,
// and their values are set by the client.
//
// Names of the form .X represent the executable file X;
// their value is whether that program is running.

#define PREFS_IDLE_TIME "idle_time"    // time since user input
#define PREFS_ON_BATTERIES "on_batteries"
#define PREFS_TIME "time"
#define PREFS_NON_BOINC_CPU_USAGE "non_boinc_cpu_usage"

// the values of non-standard entries are set via RPC

struct PREFS_DICT_ENTRY {
    double value;
    PREFS_DICT_ENTRY(double v) {
        value = v;
    }
    PREFS_DICT_ENTRY() {}
};

typedef std::pair<std::string, PREFS_DICT_ENTRY> PREFS_DICT_PAIR;

struct PREFS_DICT {
    std::map<std::string, PREFS_DICT_ENTRY> dict;
    bool lookup(std::string s, PREFS_DICT_ENTRY& x) {
        if (dict.count(s) == 0) {
            return false;
        }
        x = dict[s];
        return true;
    }

    void add_item(std::string s) {
        dict.insert(PREFS_DICT_PAIR(s, PREFS_DICT_ENTRY(0)));
    }

    void init();
};

extern PREFS_DICT prefs_dict;

typedef enum {
    TERM_NONE,
    TERM_GT,
    TERM_BOOL,
    TERM_TIME_RANGE,
    TERM_APP_RUNNING
} TERM_TYPE;

// a term of a condition
//
struct PREFS_TERM {
    std::string item;
    bool _not;
    double thresh;
    TIME_PREFS *time_range;
    TERM_TYPE term_type;
    PREFS_TERM(TERM_TYPE t, std::string i) {
        term_type = t;
        item = i;
    }
    PREFS_TERM(TERM_TYPE t, std::string i, double x) {
        term_type = t;
        item = i;
        thresh = x;
    }
    bool holds() {
        PREFS_DICT_ENTRY x;
        if (!prefs_dict.lookup(item, x)) {
            return false;
        }
        switch (term_type) {
        case TERM_NONE: return false;
        case TERM_GT: return x.value > thresh;
        case TERM_BOOL:
        case TERM_APP_RUNNING:
            return x.value != 0;
        case TERM_TIME_RANGE: return time_range->suspended(x.value);
        }
        return false;
    }
    void clear() {
        item.clear();
        _not = false;
        thresh = 0;
        term_type = TERM_NONE;
        time_range = NULL;
    }
    PREFS_TERM() {
        clear();
    }
    void write(MIOFILE&);
    int parse(XML_PARSER&);
};

// A condition is the "and" of some terms, possibly negated.
//
struct PREFS_CONDITION {
    bool _not;
    std::vector<PREFS_TERM> terms;
    bool holds() {
        bool x = true;
        for (unsigned int i=0; i<terms.size(); i++) {
            if (!terms[i].holds()) {
                x = false;
                break;
            }
        }
        if (_not) x = !x;
        return x;
    }
    void clear() {
        _not = false;
        terms.clear();
    }
    PREFS_CONDITION() {
        clear();
    }
    void write(MIOFILE&);
    int parse(XML_PARSER&);
};

// specifies a group of "dynamic parameters": #CPUs, max RAM, etc.
// All are optional.
//
struct PREFS_DYNAMIC_PARAMS {
    OPTIONAL_DOUBLE cpu_usage_limit;
    OPTIONAL_BOOL dont_use_cpu;
    OPTIONAL_BOOL dont_use_gpu;
    OPTIONAL_BOOL dont_do_file_xfer;
    OPTIONAL_DOUBLE max_bytes_sec_down;
    OPTIONAL_DOUBLE max_bytes_sec_up;
    OPTIONAL_INT max_ncpus;
    OPTIONAL_DOUBLE max_ncpus_pct;
    OPTIONAL_DOUBLE ram_max_used_frac;
    void clear() {
        memset(this, 0, sizeof(*this));
    }
    PREFS_DYNAMIC_PARAMS() {
        clear();
    }
    inline bool any_present() {
        return cpu_usage_limit.present
            || dont_use_cpu.present
            || dont_use_gpu.present
            || dont_do_file_xfer.present
            || max_bytes_sec_down.present
            || max_bytes_sec_up.present
            || max_ncpus.present
            || max_ncpus_pct.present
            || ram_max_used_frac.present
        ;
    }
    // add or replace items that are present in s
    //
    void overlay(PREFS_DYNAMIC_PARAMS& s) {
        cpu_usage_limit.overlay(s.cpu_usage_limit);
        dont_use_cpu.overlay(s.dont_use_cpu);
        dont_use_gpu.overlay(s.dont_use_gpu);
        dont_do_file_xfer.overlay(s.dont_do_file_xfer);
        max_bytes_sec_down.overlay(s.max_bytes_sec_down);
        max_bytes_sec_up.overlay(s.max_bytes_sec_up);
        max_ncpus.overlay(s.max_ncpus);
        max_ncpus_pct.overlay(s.max_ncpus_pct);
        ram_max_used_frac.overlay(s.ram_max_used_frac);
    }
    void write(MIOFILE&);
    int parse(XML_PARSER&);
};

// the combination of a condition and a set of params
//
struct PREFS_CLAUSE {
    PREFS_CONDITION condition;
    PREFS_DYNAMIC_PARAMS params;
    void write(MIOFILE&);
    int parse(XML_PARSER&);
    void clear() {
        condition.clear();
        params.clear();
    }
    PREFS_CLAUSE() {
        clear();
    }
};

// static params, i.e. those that don't change over time
//
struct PREFS_STATIC_PARAMS {
    OPTIONAL_DOUBLE battery_charge_min_pct;
    OPTIONAL_DOUBLE battery_max_temperature;
    OPTIONAL_BOOL confirm_before_connecting;
    OPTIONAL_DOUBLE cpu_scheduling_period_minutes;
    OPTIONAL_DOUBLE daily_xfer_limit_mb;
    OPTIONAL_INT daily_xfer_period_days;
    OPTIONAL_DOUBLE disk_max_used_gb;
    OPTIONAL_DOUBLE disk_max_used_pct;
    OPTIONAL_DOUBLE disk_min_free_gb;
    OPTIONAL_BOOL dont_verify_images;
    OPTIONAL_BOOL hangup_if_dialed;
    OPTIONAL_BOOL leave_apps_in_memory;
    OPTIONAL_BOOL network_wifi_only;
    OPTIONAL_DOUBLE work_buf_additional_days;
    OPTIONAL_DOUBLE work_buf_min_days;
    void clear() {
        memset(this, 0, sizeof(*this));
    }
    void write(MIOFILE&);
    int parse(XML_PARSER&);
};

// overall preferences.
// To evaluate dynamic params,
// we start of with a default param set S.
// the clauses are evaluated in order.
// for each one whose condition is true,
// its params overlay S.
//
struct PREFS {
    double mod_time;
    std::vector<PREFS_CLAUSE> clauses;
    PREFS_STATIC_PARAMS static_params;

    void convert(GLOBAL_PREFS&, CC_CONFIG&);
    void get_dynamic_params(PREFS_DYNAMIC_PARAMS&);
    void write(MIOFILE&);
    void write_file(const char* fname);
    int parse(XML_PARSER&);
    void init();
};

extern PREFS prefs2;

#endif
