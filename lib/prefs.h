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

// Global preferences are edited and stored on BOINC servers.
// The native representation of preferences is XML.
// The client maintains the preferences (in XML form)
// and mod time in the state file and in memory.
// It includes these items in each scheduler request message.
// A scheduler reply message may contain a more recent set of preferences.
//

// The following structure is a parsed version of the prefs file
//
struct GLOBAL_PREFS {
    int mod_time;
    bool run_on_batteries;
    bool run_if_user_active;
    int start_hour;     // 0..23; no restriction if start==end
    int end_hour;
    int net_start_hour;     // 0..23; no restriction if start==end
    int net_end_hour;
    bool leave_apps_in_memory;
    bool confirm_before_connecting;
    bool run_minimized;
    bool run_on_startup;
    bool hangup_if_dialed;
    bool dont_verify_images;
    double work_buf_min_days;
    int max_cpus;
    double cpu_scheduling_period_minutes;
    double disk_interval;
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;
    double vm_max_used_pct;
    double idle_time_to_run;
    double max_bytes_sec_up;
    double max_bytes_sec_down;
    //int max_memory_mbytes;
    int proc_priority;
    int cpu_affinity;
    double cpu_usage_limit;
    char source_project[256];
    char source_scheduler[256];

    GLOBAL_PREFS();
    void defaults();
    void clear_bools();
    int parse(MIOFILE&, const char* venue, bool& found_venue);
    int parse_override(MIOFILE&, const char* venue, bool& found_venue);
    int parse_file(const char* filename, const char* venue, bool& found_venue);
    int write(MIOFILE&);
};

#endif
