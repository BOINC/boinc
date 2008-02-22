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
//      (it should be used for testing only, but older (<6) clients rely on it).
//      For the prefs that it specifies, it overrides the network prefs.


// 0..24
// run always if start==end or start==0, end=24
// don't run at all if start=24, end=0
class TIME_SPAN {
public:
    enum TimeMode {
        Always = 7000,
        Never,
        Between
    };
    TIME_SPAN()
        : start_hour(0), end_hour(0) {}
    TIME_SPAN(double start, double end)
        : start_hour(start), end_hour(end) {}

    bool        suspended(double hour) const;
    TimeMode    mode() const;

    double      start_hour;
    double      end_hour;
    
};


class WEEK_PREFS {
public:
    WEEK_PREFS();
    WEEK_PREFS(const WEEK_PREFS& original);
    ~WEEK_PREFS();

    TIME_SPAN* get(int day) const;
    void set(int day, double start, double end);
    void set(int day, TIME_SPAN* time);
    void unset(int day);
    void clear();
    WEEK_PREFS& operator=(const WEEK_PREFS& rhs);

protected:
    void copy(const WEEK_PREFS& original);
    TIME_SPAN* days[7];

};


class TIME_PREFS : public TIME_SPAN {
public:
    TIME_PREFS() : TIME_SPAN() {}
    TIME_PREFS(double start, double end)
        : TIME_SPAN(start, end) {}
    
    void        clear();
    bool        suspended() const;
    
    WEEK_PREFS  week;
};


class VENUE {
public:
    VENUE(char* name = "", char* description = "");

    char venue_name[32]; // immutable
    char venue_description[256]; // localisable, renamable, UTF-8?

    std::string get_venue_description();
};


class GLOBAL_PREFS : public VENUE {
public:
    GLOBAL_PREFS();

    double mod_time;
    bool run_on_batteries;
        // poorly named; what it really means is:
        // if false, suspend while on batteries
    bool run_if_user_active;
    double idle_time_to_run;
    double suspend_if_no_recent_input;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool hangup_if_dialed;
    bool dont_verify_images;
    TIME_PREFS cpu_times;
    TIME_PREFS net_times;
    double work_buf_min_days;
    double work_buf_additional_days;
    double max_ncpus_pct;
    double cpu_scheduling_period_minutes;
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
    char source_project[256];
    char source_scheduler[256];
    bool host_specific;

    void defaults();
    void clear_bools();

    int parse(XML_PARSER&);
    int parse_day(XML_PARSER&);
    int parse_override(XML_PARSER&);
    int parse_file(const char* filename);
    int parse_preference_tags(XML_PARSER&);
    int write(MIOFILE&);
    inline double cpu_scheduling_period() {
        return cpu_scheduling_period_minutes*60;
    }

    static int parse_file(const char* filename, std::vector<GLOBAL_PREFS*>& venues);
    static int parse_venues(XML_PARSER& xp, std::vector<GLOBAL_PREFS*>& venues);

private:
    static int recursive_parse_venue(XML_PARSER& xp, GLOBAL_PREFS* const prefs, std::vector<GLOBAL_PREFS*>* venues);
};

#endif
