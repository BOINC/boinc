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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#endif

#include "parse.h"

#include "error_numbers.h"
#include "prefs.h"

// The following values determine how the client behaves
// if there are no global prefs (e.g. on our very first RPC).
// These should impose minimal restrictions,
// so that the client can do the RPC and get the global prefs from the server
//
void GLOBAL_PREFS::defaults() {
    run_on_batteries = true;
    run_if_user_active = true;
    start_hour = 0;
    end_hour = 0;
    net_start_hour = 0;
    net_end_hour = 0;
    run_minimized = false;
    run_on_startup = false;
    leave_apps_in_memory = false;
    confirm_before_connecting = true;
    hangup_if_dialed = false;
    dont_verify_images = false;
    work_buf_min_days = 0.1;
    max_cpus = 16;
    cpu_scheduling_period_minutes = 60;
    disk_interval = 60;
    disk_max_used_gb = 10;
    disk_max_used_pct = 50;
    disk_min_free_gb = 0.1;
    vm_max_used_pct = 75;
    idle_time_to_run = 3;
    max_bytes_sec_up = 1e9;
    max_bytes_sec_down = 1e9;
    //max_memory_mbytes = 128;
    proc_priority = 1;
    cpu_affinity = -1;
    cpu_usage_limit = 1;

    // don't initialize source_project, source_scheduler here
    // since they are outside of <venue> elements
}

// before parsing
void GLOBAL_PREFS::clear_bools() {
    run_on_batteries = false;
    run_if_user_active = false;
    leave_apps_in_memory = false;
    confirm_before_connecting = false;
    run_minimized = false;
    run_on_startup = false;
    hangup_if_dialed = false;
    dont_verify_images = false;
}

GLOBAL_PREFS::GLOBAL_PREFS() {
    defaults();
}

// Parse XML global prefs, setting defaults first.
//
int GLOBAL_PREFS::parse(
    MIOFILE& in, const char* host_venue, bool& found_venue
) {
    defaults();
    clear_bools();

    strcpy(source_project, "");
    strcpy(source_scheduler, "");
    return parse_override(in, host_venue, found_venue);
}

// Parse global prefs, overriding whatever is currently in the structure.
//
// If host_venue is nonempty and we find an element of the form
// <venue name="X">
//   ...
// </venue>
// where X==host_venue, then parse that and ignore the rest.
// Otherwise ignore <venue> elements.
//
int GLOBAL_PREFS::parse_override(
    MIOFILE& in, const char* host_venue, bool& found_venue
) {
    char buf[256], buf2[256];
    bool in_venue = false, in_correct_venue=false;

    found_venue = false;
    while (in.fgets(buf, 256)) {
        if (in_venue) {
            if (match_tag(buf, "</venue>")) {
                if (in_correct_venue) {
                    break;
                } else {
                    in_venue = false;
                    continue;
                }
            } else {
                if (!in_correct_venue) continue;
            }
        } else {
            if (match_tag(buf, "<venue")) {
                in_venue = true;
                parse_attr(buf, "name", buf2, sizeof(buf2));
                if (!strcmp(buf2, host_venue)) {
                    defaults();
                    clear_bools();
                    in_correct_venue = true;
                    found_venue = true;
                } else {
                    in_correct_venue = false;
                }
                continue;
            }
        }
        if (match_tag(buf, "<global_preferences>")) {
            continue;
        } else if (parse_str(buf, "<source_project>", source_project, sizeof(source_project))) {
            continue;
        } else if (parse_str(buf, "<source_scheduler>", source_scheduler, sizeof(source_scheduler))) {
            continue;
        } else if (parse_int(buf, "<mod_time>", mod_time)) {
            continue;
        } else if (match_tag(buf, "</global_preferences>")) {
            break;
        } else if (parse_bool(buf, "run_on_batteries", run_on_batteries)) {
            continue;
        } else if (parse_bool(buf, "run_if_user_active", run_if_user_active)) {
            continue;
        } else if (parse_int(buf, "<start_hour>", start_hour)) {
            continue;
        } else if (parse_int(buf, "<end_hour>", end_hour)) {
            continue;
        } else if (parse_int(buf, "<net_start_hour>", net_start_hour)) {
            continue;
        } else if (parse_int(buf, "<net_end_hour>", net_end_hour)) {
            continue;
        } else if (parse_bool(buf, "leave_apps_in_memory", leave_apps_in_memory)) {
            continue;
        } else if (parse_bool(buf, "confirm_before_connecting", confirm_before_connecting)) {
            continue;
        } else if (parse_bool(buf, "hangup_if_dialed", hangup_if_dialed)) {
            continue;
        } else if (parse_bool(buf, "run_minimized", run_minimized)) {
            continue;
        } else if (parse_bool(buf, "run_on_startup", run_on_startup)) {
            continue;
        } else if (parse_bool(buf, "dont_verify_images", dont_verify_images)) {
            continue;
        } else if (parse_double(buf, "<work_buf_min_days>", work_buf_min_days)) {
            continue;
        } else if (parse_int(buf, "<max_cpus>", max_cpus)) {
            if (max_cpus < 1) max_cpus = 1;
            continue;
        } else if (parse_double(buf, "<disk_interval>", disk_interval)) {
            if (disk_interval<0) disk_interval = 0;
            continue;
        } else if (parse_double(buf, "<cpu_scheduling_period_minutes>", cpu_scheduling_period_minutes)) {
            if (cpu_scheduling_period_minutes < 0.0001) cpu_scheduling_period_minutes = 60;
            continue;
        } else if (parse_double(buf, "<disk_max_used_gb>", disk_max_used_gb)) {
            continue;
        } else if (parse_double(buf, "<disk_max_used_pct>", disk_max_used_pct)) {
            continue;
        } else if (parse_double(buf, "<disk_min_free_gb>", disk_min_free_gb)) {
            continue;
        } else if (parse_double(buf, "<vm_max_used_pct>", vm_max_used_pct)) {
            continue;
        } else if (parse_double(buf, "<idle_time_to_run>", idle_time_to_run)) {
            continue;
        } else if (parse_double(buf, "<max_bytes_sec_up>", max_bytes_sec_up)) {
            if (max_bytes_sec_up <= 0) max_bytes_sec_up = 1e12;
            continue;
        } else if (parse_double(buf, "<max_bytes_sec_down>", max_bytes_sec_down)) {
            if (max_bytes_sec_down <= 0) max_bytes_sec_down = 1e12;
            continue;
#if 0
        } else if (parse_int(buf, "<max_memory_mbytes>", max_memory_mbytes)) {
            continue;
#endif
        } else if (parse_int(buf, "<cpu_affinity>", cpu_affinity)) {
            continue;
        } else if (parse_double(buf, "<cpu_usage_limit>", cpu_usage_limit)) {
            continue;
        }
    }
    return 0;
}

// Parse global prefs file
//
int GLOBAL_PREFS::parse_file(
    const char* filename, const char* host_venue, bool& found_venue
) {
    FILE* f;
    int retval;

    f = fopen(filename, "r");
    if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    retval = parse(mf, host_venue, found_venue);
    fclose(f);
    return retval;
}

// this is used only to write the app init data file
//
int GLOBAL_PREFS::write(MIOFILE& f) {
    f.printf(
        "<global_preferences>\n"
        "   <mod_time>%d</mod_time>\n"
        "%s%s"
        "   <start_hour>%d</start_hour>\n"
        "   <end_hour>%d</end_hour>\n"
        "   <net_start_hour>%d</net_start_hour>\n"
        "   <net_end_hour>%d</net_end_hour>\n"
        "%s%s%s%s%s%s"
        "   <work_buf_min_days>%f</work_buf_min_days>\n"
        "   <max_cpus>%d</max_cpus>\n"
        "   <cpu_scheduling_period_minutes>%f</cpu_scheduling_period_minutes>\n"
        "   <disk_interval>%f</disk_interval>\n"
        "   <disk_max_used_gb>%f</disk_max_used_gb>\n"
        "   <disk_max_used_pct>%f</disk_max_used_pct>\n"
        "   <disk_min_free_gb>%f</disk_min_free_gb>\n"
        "   <vm_max_used_pct>%f</vm_max_used_pct>\n"
        "   <idle_time_to_run>%f</idle_time_to_run>\n"
        "   <max_bytes_sec_up>%f</max_bytes_sec_up>\n"
        "   <max_bytes_sec_down>%f</max_bytes_sec_down>\n"
        "</global_preferences>\n",
        mod_time,
        run_on_batteries?"   <run_on_batteries/>\n":"",
        run_if_user_active?"   <run_if_user_active/>\n":"",
        start_hour,
        end_hour,
        net_start_hour,
        net_end_hour,
        leave_apps_in_memory?"   <leave_apps_in_memory/>\n":"",
        confirm_before_connecting?"   <confirm_before_connecting/>\n":"",
        run_minimized?"   <run_minimized/>\n":"",
        run_on_startup?"   <run_on_startup/>\n":"",
        hangup_if_dialed?"   <hangup_if_dialed/>\n":"",
        dont_verify_images?"   <dont_verify_images/>\n":"",
        work_buf_min_days,
        max_cpus,
        cpu_scheduling_period_minutes,
        disk_interval,
        disk_max_used_gb,
        disk_max_used_pct,
        disk_min_free_gb,
        vm_max_used_pct,
        idle_time_to_run,
        max_bytes_sec_up,
        max_bytes_sec_down
    );
    return 0;
}


const char *BOINC_RCSID_3fb442bb02 = "$Id$";
