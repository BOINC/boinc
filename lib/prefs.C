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
    vm_max_used_frac = 0.75;
	ram_max_used_busy_frac = 0.5;
	ram_max_used_idle_frac = 0.9;
    idle_time_to_run = 3;
    max_bytes_sec_up = 0;
    max_bytes_sec_down = 0;
    cpu_usage_limit = 100;

    // don't initialize source_project, source_scheduler,
    // mod_time, host_specific here
    // since they are outside of <venue> elements,
    // and this is called when find the right venue.
    // Also, don't memset to 0
}

// before parsing
void GLOBAL_PREFS::clear_bools() {
    run_on_batteries = false;
    run_if_user_active = false;
    leave_apps_in_memory = false;
    confirm_before_connecting = false;
    hangup_if_dialed = false;
    dont_verify_images = false;
}

GLOBAL_PREFS::GLOBAL_PREFS() {
    defaults();
}

// Parse XML global prefs, setting defaults first.
// The start tag has already been parsed.
//
int GLOBAL_PREFS::parse(
    XML_PARSER& xp, const char* host_venue, bool& found_venue
) {
    defaults();
    clear_bools();

    strcpy(source_project, "");
    strcpy(source_scheduler, "");
    mod_time = 0;
    host_specific = false;

    return parse_override(xp, host_venue, found_venue);
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
// The start tag has already been parsed.
//
int GLOBAL_PREFS::parse_override(
    XML_PARSER& xp, const char* host_venue, bool& found_venue
) {
    char tag[256], buf2[256];
    bool in_venue = false, in_correct_venue=false, is_tag;
    double dtemp;

    found_venue = false;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            printf("unexpected text: %s\n", tag);
            continue;
        }
        if (in_venue) {
            if (!strcmp(tag, "/venue")) {
                if (in_correct_venue) {
                    return 0;
                } else {
                    in_venue = false;
                    continue;
                }
            } else {
                if (!in_correct_venue) continue;
            }
        } else {
            if (strstr(tag, "venue")) {
                in_venue = true;
                parse_attr(tag, "name", buf2, sizeof(buf2));
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
        if (xp.parse_str(tag, "source_project", source_project, sizeof(source_project))) {
            continue;
        } else if (xp.parse_str(tag, "source_scheduler", source_scheduler, sizeof(source_scheduler))) {
            continue;
        } else if (xp.parse_int(tag, "mod_time", mod_time)) {
            continue;
        } else if (!strcmp(tag, "/global_preferences")) {
            return 0;
        } else if (xp.parse_bool(tag, "run_on_batteries", run_on_batteries)) {
            continue;
        } else if (xp.parse_bool(tag, "run_if_user_active", run_if_user_active)) {
            continue;
        } else if (xp.parse_int(tag, "start_hour", start_hour)) {
            continue;
        } else if (xp.parse_int(tag, "end_hour", end_hour)) {
            continue;
        } else if (xp.parse_int(tag, "net_start_hour", net_start_hour)) {
            continue;
        } else if (xp.parse_int(tag, "net_end_hour", net_end_hour)) {
            continue;
        } else if (xp.parse_bool(tag, "leave_apps_in_memory", leave_apps_in_memory)) {
            continue;
        } else if (xp.parse_bool(tag, "confirm_before_connecting", confirm_before_connecting)) {
            continue;
        } else if (xp.parse_bool(tag, "hangup_if_dialed", hangup_if_dialed)) {
            continue;
        } else if (xp.parse_bool(tag, "dont_verify_images", dont_verify_images)) {
            continue;
        } else if (xp.parse_double(tag, "work_buf_min_days", work_buf_min_days)) {
            continue;
        } else if (xp.parse_int(tag, "max_cpus", max_cpus)) {
            if (max_cpus < 1) max_cpus = 1;
            continue;
        } else if (xp.parse_double(tag, "disk_interval", disk_interval)) {
            if (disk_interval<0) disk_interval = 0;
            continue;
        } else if (xp.parse_double(tag, "cpu_scheduling_period_minutes", cpu_scheduling_period_minutes)) {
            if (cpu_scheduling_period_minutes < 0.0001) cpu_scheduling_period_minutes = 60;
            continue;
        } else if (xp.parse_double(tag, "disk_max_used_gb", disk_max_used_gb)) {
            continue;
        } else if (xp.parse_double(tag, "disk_max_used_pct", disk_max_used_pct)) {
            continue;
        } else if (xp.parse_double(tag, "disk_min_free_gb", disk_min_free_gb)) {
            continue;
        } else if (xp.parse_double(tag, "vm_max_used_pct", dtemp)) {
			vm_max_used_frac = dtemp/100;
            continue;
        } else if (xp.parse_double(tag, "ram_max_used_busy_pct", dtemp)) {
			ram_max_used_busy_frac = dtemp/100;
            continue;
        } else if (xp.parse_double(tag, "ram_max_used_idle_pct", dtemp)) {
			ram_max_used_idle_frac = dtemp/100;
            continue;
        } else if (xp.parse_double(tag, "idle_time_to_run", idle_time_to_run)) {
            continue;
        } else if (xp.parse_double(tag, "max_bytes_sec_up", max_bytes_sec_up)) {
            if (max_bytes_sec_up < 0) max_bytes_sec_up = 0;
            continue;
        } else if (xp.parse_double(tag, "max_bytes_sec_down", max_bytes_sec_down)) {
            if (max_bytes_sec_down < 0) max_bytes_sec_down = 0;
            continue;
        } else if (xp.parse_double(tag, "cpu_usage_limit", dtemp)) {
            if (dtemp > 0 && dtemp <= 100) {
                cpu_usage_limit = dtemp;
            }
            continue;
        } else if (xp.parse_bool(tag, "host_specific", host_specific)) {
            continue;
        }
    }
    return ERR_XML_PARSE;
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
    XML_PARSER xp(&mf);
    if (!xp.parse_start("global_preferences")) {
        return ERR_XML_PARSE;
    }
    retval = parse(xp, host_venue, found_venue);
    fclose(f);
    return retval;
}

// Write the global prefs that are actually in force
// (our particular venue, modified by overwrite file).
// This is used to write
// 1) the app init data file
// 2) GUI RPC get_state reply
// Not used for scheduler request; there, we just copy the
// global_prefs.xml file (which includes all venues).
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
        "%s%s%s%s"
        "   <work_buf_min_days>%f</work_buf_min_days>\n"
        "   <max_cpus>%d</max_cpus>\n"
        "   <cpu_scheduling_period_minutes>%f</cpu_scheduling_period_minutes>\n"
        "   <disk_interval>%f</disk_interval>\n"
        "   <disk_max_used_gb>%f</disk_max_used_gb>\n"
        "   <disk_max_used_pct>%f</disk_max_used_pct>\n"
        "   <disk_min_free_gb>%f</disk_min_free_gb>\n"
        "   <vm_max_used_pct>%f</vm_max_used_pct>\n"
        "   <ram_max_used_busy_pct>%f</ram_max_used_busy_pct>\n"
        "   <ram_max_used_idle_pct>%f</ram_max_used_idle_pct>\n"
        "   <idle_time_to_run>%f</idle_time_to_run>\n"
        "   <max_bytes_sec_up>%f</max_bytes_sec_up>\n"
        "   <max_bytes_sec_down>%f</max_bytes_sec_down>\n"
        "   <cpu_usage_limit>%f</cpu_usage_limit>\n"
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
        hangup_if_dialed?"   <hangup_if_dialed/>\n":"",
        dont_verify_images?"   <dont_verify_images/>\n":"",
        work_buf_min_days,
        max_cpus,
        cpu_scheduling_period_minutes,
        disk_interval,
        disk_max_used_gb,
        disk_max_used_pct,
        disk_min_free_gb,
        vm_max_used_frac*100,
		ram_max_used_busy_frac*100,
		ram_max_used_idle_frac*100,
        idle_time_to_run,
        max_bytes_sec_up,
        max_bytes_sec_down,
        cpu_usage_limit
    );
    return 0;
}

const char *BOINC_RCSID_3fb442bb02 = "$Id$";
