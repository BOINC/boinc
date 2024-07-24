// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <time.h>
#endif

#include "boinc_stdio.h"
#include "error_numbers.h"
#include "str_replace.h"
#include "parse.h"
#include "util.h"

#include "prefs.h"

void GLOBAL_PREFS_MASK::set_all() {
    battery_charge_min_pct = true;
    battery_max_temperature = true;
    confirm_before_connecting = true;
    cpu_scheduling_period_minutes = true;
    cpu_usage_limit = true;
    daily_xfer_limit_mb = true;
    daily_xfer_period_days = true;
    disk_interval = true;
    disk_max_used_gb = true;
    disk_max_used_pct = true;
    disk_min_free_gb = true;
    dont_verify_images = true;
    end_hour = true;
    hangup_if_dialed = true;
    idle_time_to_run = true;
    leave_apps_in_memory = true;
    max_bytes_sec_down = true;
    max_bytes_sec_up = true;
    max_ncpus= true;
    max_ncpus_pct = true;
    net_end_hour = true;
    net_start_hour = true;
    network_wifi_only = true;
    niu_max_ncpus_pct = true;
    niu_cpu_usage_limit = true;
    niu_suspend_cpu_usage = true;
    ram_max_used_busy_frac = true;
    ram_max_used_idle_frac = true;
    run_gpu_if_user_active = true;
    run_if_user_active = true;
    run_on_batteries = true;
    start_hour = true;
    suspend_cpu_usage = 0;
    suspend_if_no_recent_input = true;
    vm_max_used_frac = true;
    work_buf_additional_days = true;
    work_buf_min_days = true;
}

bool GLOBAL_PREFS_MASK::are_prefs_set() {
    if (battery_charge_min_pct) return true;
    if (battery_max_temperature) return true;
    if (confirm_before_connecting) return true;
    if (cpu_scheduling_period_minutes) return true;
    if (cpu_usage_limit) return true;
    if (daily_xfer_limit_mb) return true;
    if (daily_xfer_period_days) return true;
    if (disk_interval) return true;
    if (disk_max_used_gb) return true;
    if (disk_max_used_pct) return true;
    if (disk_min_free_gb) return true;
    if (dont_verify_images) return true;
    if (end_hour) return true;
    if (hangup_if_dialed) return true;
    if (idle_time_to_run) return true;
    if (leave_apps_in_memory) return true;
    if (max_bytes_sec_down) return true;
    if (max_bytes_sec_up) return true;
    if (max_ncpus) return true;
    if (max_ncpus_pct) return true;
    if (net_start_hour) return true;
    if (network_wifi_only) return true;
    if (net_end_hour) return true;
    if (niu_max_ncpus_pct) return true;
    if (niu_cpu_usage_limit) return true;
    if (niu_suspend_cpu_usage) return true;
    if (ram_max_used_busy_frac) return true;
    if (ram_max_used_idle_frac) return true;
    if (run_gpu_if_user_active) return true;
    if (run_if_user_active) return true;
    if (run_on_batteries) return true;
    if (start_hour) return true;
    if (suspend_if_no_recent_input) return true;
    if (suspend_cpu_usage) return true;
    if (vm_max_used_frac) return true;
    if (work_buf_additional_days) return true;
    if (work_buf_min_days) return true;
    return false;
}

bool GLOBAL_PREFS_MASK::are_simple_prefs_set() {
    if (cpu_usage_limit) return true;
    if (disk_max_used_gb) return true;
    if (end_hour) return true;
    if (idle_time_to_run) return true;
    if (net_end_hour) return true;
    if (net_start_hour) return true;
    if (run_if_user_active) return true;
    if (run_on_batteries) return true;
    if (start_hour) return true;
    return false;
}


// TIME_SPAN implementation

bool TIME_SPAN::suspended(double hour) const {
    if (start_hour == end_hour) return false;
    if (start_hour == 0 && end_hour == 24) return false;
    if (start_hour == 24 && end_hour == 0) return true;
    if (start_hour < end_hour) {
        return (hour < start_hour || hour > end_hour);
    } else {
        return (hour >= end_hour && hour < start_hour);
    }
}

TIME_SPAN::TimeMode TIME_SPAN::mode() const {
    if (end_hour == start_hour || (start_hour == 0 && end_hour == 24)) {
        return Always;
    } else if (start_hour == 24 && end_hour == 0) {
        return Never;
    }
    return Between;
}


// TIME_PREFS implementation

void TIME_PREFS::clear() {
    start_hour = 0;
    end_hour = 0;
    week.clear();
}

bool TIME_PREFS::suspended(double now) {
    time_t t = (time_t)now;
    struct tm* tmp = localtime(&t);
    double hour = (tmp->tm_hour * 3600 + tmp->tm_min * 60 + tmp->tm_sec) / 3600.;
    int day = tmp->tm_wday;

    // Use day-specific settings, if they exist:
    //
    if (day>=0 && day<7 && week.days[day].present) {
        return week.days[day].suspended(hour);
    }
    return TIME_SPAN::suspended(hour);
}


// WEEK_PREFS implementation


void WEEK_PREFS::set(int day, double start, double end) {
    if (day < 0 || day > 6) return;
    if (start == end) return;
    days[day].present = true;
    days[day].start_hour = start;
    days[day].end_hour = end;
}


void WEEK_PREFS::set(int day, TIME_SPAN* time) {
    if (day < 0 || day > 6) return;
    if (time->start_hour == time->end_hour) return;
    days[day].present = true;
    days[day].start_hour = time->start_hour;
    days[day].end_hour = time->end_hour;
}

void WEEK_PREFS::unset(int day) {
    if (day < 0 || day > 6) return;
    days[day].present = false;
}

// The following values determine how the client behaves
// if there are no global prefs (e.g. on our very first RPC).
// These should impose minimal restrictions,
// so that the client can do the RPC and get the global prefs from the server
//
void GLOBAL_PREFS::defaults() {
    battery_charge_min_pct = 90;
    battery_max_temperature = 40;
    confirm_before_connecting = true;
    cpu_scheduling_period_minutes = 60;
    cpu_times.clear();
    cpu_usage_limit = 100;
    daily_xfer_limit_mb = 0;
    daily_xfer_period_days = 0;
    disk_interval = 60;
    disk_max_used_gb = 0;
    disk_max_used_pct = 90;
    disk_min_free_gb = 0.1;
    dont_verify_images = false;
    hangup_if_dialed = false;
    idle_time_to_run = 3;
    leave_apps_in_memory = false;
    max_bytes_sec_down = 0;
    max_bytes_sec_up = 0;
    max_ncpus = 0;
#ifdef ANDROID
    max_ncpus_pct = 50;
#else
    max_ncpus_pct = 0;
#endif
    net_times.clear();
#ifdef ANDROID
    network_wifi_only = true;
#else
    network_wifi_only = false;
#endif
    niu_max_ncpus_pct = 100;
    niu_cpu_usage_limit = 100;
    niu_suspend_cpu_usage = 50;
    ram_max_used_busy_frac = 0.5;
#ifdef ANDROID
    ram_max_used_idle_frac = 0.5;
#else
    ram_max_used_idle_frac = 0.9;
#endif
    run_gpu_if_user_active = false;
    run_if_user_active = true;
    run_on_batteries = false;
#ifdef ANDROID
    suspend_cpu_usage = 50;
#else
    suspend_cpu_usage = 25;
#endif
    suspend_if_no_recent_input = 0;
    vm_max_used_frac = 0.75;
    work_buf_additional_days = 0.5;
    work_buf_min_days = 0.1;

    override_file_present = false;

    // don't initialize source_project, source_scheduler,
    // mod_time, host_specific here
    // since they are outside of <venue> elements,
    // and this is called when find the right venue.
}

// values for fields with an enabling checkbox in the GUI.
// These are the values shown when the checkbox is first checked
// (in cases where this differs from the default).
// These should be consistent with html/inc/prefs.inc
//
void GLOBAL_PREFS::enabled_defaults() {
    defaults();
    suspend_if_no_recent_input = 60;
    disk_max_used_gb = 100;
    disk_min_free_gb = 1.0;
    daily_xfer_limit_mb = 10000;
    daily_xfer_period_days = 30;
    max_bytes_sec_down = 100*KILO;
    max_bytes_sec_up = 100*KILO;
    cpu_times.start_hour = 0;
    cpu_times.end_hour = 24.0;    // 24:00
    net_times.start_hour = 0;
    net_times.end_hour = 24.0;    // 24:00
}

// call before parsing
//
void GLOBAL_PREFS::init_bools() {
    run_on_batteries = false;
    run_if_user_active = false;
    leave_apps_in_memory = false;
    confirm_before_connecting = false;
    hangup_if_dialed = false;
    dont_verify_images = false;
    network_wifi_only = true;
}

void GLOBAL_PREFS::init() {
    defaults();
    safe_strcpy(source_project, "");
    safe_strcpy(source_scheduler, "");
    mod_time = 0;
    host_specific = false;
}

GLOBAL_PREFS::GLOBAL_PREFS() {
    init();
}

// Parse XML global prefs, setting defaults first.
//
int GLOBAL_PREFS::parse(
    XML_PARSER& xp, const char* host_venue, bool& found_venue, GLOBAL_PREFS_MASK& mask
) {
    init();
    init_bools();
    return parse_override(xp, host_venue, found_venue, mask);
}

int GLOBAL_PREFS::parse_day(XML_PARSER& xp) {
    int day_of_week = -1;
    bool has_cpu = false;
    bool has_net = false;
    double start_hour = 0;
    double end_hour = 0;
    double net_start_hour = 0;
    double net_end_hour = 0;

    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/day_prefs")) {
            if (day_of_week < 0 || day_of_week > 6) return ERR_XML_PARSE;
            if (has_cpu) {
                cpu_times.week.set(day_of_week, start_hour, end_hour);
            }
            if (has_net) {
                net_times.week.set(day_of_week, net_start_hour, net_end_hour);
            }
            return 0;
        }
        if (xp.parse_int("day_of_week", day_of_week)) continue;
        if (xp.parse_double("start_hour", start_hour)) {
            has_cpu = true;
            continue;
        }
        if (xp.parse_double("end_hour", end_hour)) {
            has_cpu = true;
            continue;
        }
        if (xp.parse_double("net_start_hour", net_start_hour)) {
            has_net = true;
            continue;
        }
        if (xp.parse_double("net_end_hour", net_end_hour)) {
            has_net = true;
            continue;
        }
        xp.skip_unexpected(true, "GLOBAL_PREFS::parse_day");
    }
    return ERR_XML_PARSE;
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
// The start tag may or may not have already been parsed
//
int GLOBAL_PREFS::parse_override(
    XML_PARSER& xp, const char* host_venue, bool& found_venue, GLOBAL_PREFS_MASK& mask
) {
    char buf2[256], attrs[256];
    bool in_venue = false, in_correct_venue=false;
    double dtemp;
    int itemp;

    found_venue = false;
    mask.clear();

    while (!xp.get_tag(attrs, sizeof(attrs))) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("global_preferences")) continue;
        if (xp.match_tag("/global_preferences")) {
            if (cpu_times.start_hour == cpu_times.end_hour) {
                mask.start_hour = mask.end_hour = false;
            }
            if (net_times.start_hour == net_times.end_hour) {
                mask.net_start_hour = mask.net_end_hour = false;
            }
            // if not-in-use prefs weren't specified, use in-use counterpart
            //
            if (!mask.niu_max_ncpus_pct) {
                niu_max_ncpus_pct = max_ncpus_pct;
            }
            if (!mask.niu_cpu_usage_limit) {
                niu_cpu_usage_limit = cpu_usage_limit;
            }
            if (!mask.niu_suspend_cpu_usage) {
                niu_suspend_cpu_usage = suspend_cpu_usage;
            }
            return 0;
        }
        // parse these first; they're independent of venue
        //
        if (xp.parse_str("source_project", source_project, sizeof(source_project))) {
            continue;
        }
        if (xp.parse_str("source_scheduler", source_scheduler, sizeof(source_scheduler))) {
            continue;
        }
        if (xp.parse_double("mod_time", mod_time)) {
            double now = dtime();
            if (mod_time > now) {
                mod_time = now;
            }
            continue;
        }
        if (in_venue) {
            if (xp.match_tag("/venue")) {
                in_venue = false;
                continue;
            } else {
                // we're in a venue but not the right one; skip tag
                //
                if (!in_correct_venue) continue;
            }
        } else {
            if (strstr(xp.parsed_tag, "venue")) {
                in_venue = true;
                parse_attr(attrs, "name", buf2, sizeof(buf2));
                if (!strcmp(buf2, host_venue)) {
                    defaults();
                    init_bools();
                    mask.clear();
                    in_correct_venue = true;
                    found_venue = true;
                } else {
                    in_correct_venue = false;
                }
                continue;
            }
            if (found_venue) {
                // we already found and parsed the target venue;
                // skip subsequent prefs tags not in a venue
                continue;
            }
        }
        if (xp.parse_double("battery_charge_min_pct", battery_charge_min_pct)) {
            mask.battery_charge_min_pct = true;
            continue;
        }
        if (xp.parse_double("battery_max_temperature", battery_max_temperature)) {
            mask.battery_max_temperature = true;
            continue;
        }
        if (xp.parse_bool("run_on_batteries", run_on_batteries)) {
            mask.run_on_batteries = true;
            continue;
        }
        if (xp.parse_bool("run_if_user_active", run_if_user_active)) {
            mask.run_if_user_active = true;
            continue;
        }
        if (xp.parse_bool("run_gpu_if_user_active", run_gpu_if_user_active)) {
            mask.run_gpu_if_user_active = true;
            continue;
        }
        if (xp.parse_double("idle_time_to_run", idle_time_to_run)) {
            mask.idle_time_to_run = true;
            continue;
        }
        if (xp.parse_double("suspend_if_no_recent_input", suspend_if_no_recent_input)) {
            mask.suspend_if_no_recent_input = true;
            continue;
        }
        if (xp.parse_double("suspend_cpu_usage", suspend_cpu_usage)) {
            mask.suspend_cpu_usage = true;
            continue;
        }
        if (xp.parse_double("niu_suspend_cpu_usage", niu_suspend_cpu_usage)) {
            mask.niu_suspend_cpu_usage = true;
            continue;
        }
        if (xp.parse_double("start_hour", cpu_times.start_hour)) {
            mask.start_hour = true;
            continue;
        }
        if (xp.parse_double("end_hour", cpu_times.end_hour)) {
            mask.end_hour = true;
            continue;
        }
        if (xp.parse_double("net_start_hour", net_times.start_hour)) {
            mask.net_start_hour = true;
            continue;
        }
        if (xp.parse_double("net_end_hour", net_times.end_hour)) {
            mask.net_end_hour = true;
            continue;
        }
        if (xp.match_tag("day_prefs")) {
            parse_day(xp);
            continue;
        }
        if (xp.parse_bool("leave_apps_in_memory", leave_apps_in_memory)) {
            mask.leave_apps_in_memory = true;
            continue;
        }
        if (xp.parse_bool("confirm_before_connecting", confirm_before_connecting)) {
            mask.confirm_before_connecting = true;
            continue;
        }
        if (xp.parse_bool("hangup_if_dialed", hangup_if_dialed)) {
            mask.hangup_if_dialed = true;
            continue;
        }
        if (xp.parse_bool("dont_verify_images", dont_verify_images)) {
            mask.dont_verify_images = true;
            continue;
        }
        if (xp.parse_double("work_buf_min_days", work_buf_min_days)) {
            if (work_buf_min_days < 0) work_buf_min_days = 0;
            mask.work_buf_min_days = true;
            continue;
        }
        if (xp.parse_double("work_buf_additional_days", work_buf_additional_days)) {
            if (work_buf_additional_days < 0) work_buf_additional_days = 0;
            mask.work_buf_additional_days = true;
            continue;
        }
        if (xp.parse_double("max_ncpus_pct", max_ncpus_pct)) {
            if (max_ncpus_pct < 0) max_ncpus_pct = 0;
            if (max_ncpus_pct > 100) max_ncpus_pct = 100;
            mask.max_ncpus_pct = true;
            continue;
        }
        if (xp.parse_double("niu_max_ncpus_pct", niu_max_ncpus_pct)) {
            if (niu_max_ncpus_pct <= 0) niu_max_ncpus_pct = 100;
            if (niu_max_ncpus_pct > 100) niu_max_ncpus_pct = 100;
            mask.niu_max_ncpus_pct = true;
            continue;
        }
        if (xp.parse_int("max_cpus", max_ncpus)) {
            if (max_ncpus < 0) max_ncpus = 0;
            mask.max_ncpus = true;
            continue;
        }
        if (xp.parse_double("disk_interval", disk_interval)) {
            if (disk_interval<0) disk_interval = 0;
            mask.disk_interval = true;
            continue;
        }
        if (xp.parse_double("cpu_scheduling_period_minutes", cpu_scheduling_period_minutes)) {
            if (cpu_scheduling_period_minutes < 0.0001) cpu_scheduling_period_minutes = 60;
            mask.cpu_scheduling_period_minutes = true;
            continue;
        }
        if (xp.parse_double("disk_max_used_gb", disk_max_used_gb)) {
            mask.disk_max_used_gb = true;
            continue;
        }
        if (xp.parse_double("disk_max_used_pct", disk_max_used_pct)) {
            mask.disk_max_used_pct = true;
            continue;
        }
        if (xp.parse_double("disk_min_free_gb", disk_min_free_gb)) {
            mask.disk_min_free_gb = true;
            continue;
        }
        if (xp.parse_double("vm_max_used_pct", dtemp)) {
            vm_max_used_frac = dtemp/100;
            mask.vm_max_used_frac = true;
            continue;
        }
        if (xp.parse_double("ram_max_used_busy_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
            ram_max_used_busy_frac = dtemp/100;
            mask.ram_max_used_busy_frac = true;
            continue;
        }
        if (xp.parse_double("ram_max_used_idle_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
            ram_max_used_idle_frac = dtemp/100;
            mask.ram_max_used_idle_frac = true;
            continue;
        }
        if (xp.parse_double("max_bytes_sec_up", max_bytes_sec_up)) {
            if (max_bytes_sec_up < 0) max_bytes_sec_up = 0;
            mask.max_bytes_sec_up = true;
            continue;
        }
        if (xp.parse_double("max_bytes_sec_down", max_bytes_sec_down)) {
            if (max_bytes_sec_down < 0) max_bytes_sec_down = 0;
            mask.max_bytes_sec_down = true;
            continue;
        }
        if (xp.parse_double("cpu_usage_limit", dtemp)) {
            if (dtemp > 0 && dtemp <= 100) {
                cpu_usage_limit = dtemp;
                mask.cpu_usage_limit = true;
            }
            continue;
        }
        if (xp.parse_double("niu_cpu_usage_limit", dtemp)) {
            if (dtemp <= 0) dtemp = 100;
            if (dtemp > 100) dtemp = 100;
            niu_cpu_usage_limit = dtemp;
            mask.niu_cpu_usage_limit = true;
            continue;
        }
        if (xp.parse_double("daily_xfer_limit_mb", dtemp)) {
            if (dtemp >= 0) {
                daily_xfer_limit_mb = dtemp;
                mask.daily_xfer_limit_mb = true;
            }
            continue;
        }
        if (xp.parse_int("daily_xfer_period_days", itemp)) {
            if (itemp >= 0) {
                daily_xfer_period_days = itemp;
                mask.daily_xfer_period_days = true;
            }
            continue;
        }
        if (xp.parse_bool("network_wifi_only", network_wifi_only)) {
            continue;
        }
        if (xp.parse_bool("host_specific", host_specific)) {
            continue;
        }
        // false means don't print anything
        xp.skip_unexpected(false, "GLOBAL_PREFS::parse_override");
    }
    return ERR_XML_PARSE;
}

// Parse global prefs file
//
int GLOBAL_PREFS::parse_file(
    const char* filename, const char* host_venue, bool& found_venue
) {
    FILE* f;
    GLOBAL_PREFS_MASK mask;
    int retval;

#ifndef _USING_FCGI_
    f = fopen(filename, "r");
#else
    f = FCGI::fopen(filename, "r");
#endif
    if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    retval = parse(xp, host_venue, found_venue, mask);
    fclose(f);
    return retval;
}

// Write the global prefs that are actually in force
// (our particular venue, modified by overwrite file).
// This is used to write
// 1) the app init data file
// 2) GUI RPC get_state reply
// 3) scheduler request (<working_global_preferences> element)
//
int GLOBAL_PREFS::write(MIOFILE& f) {
    f.printf(
        "<global_preferences>\n"
        "   <source_project>%s</source_project>\n"
        "   <mod_time>%f</mod_time>\n"
        "   <battery_charge_min_pct>%f</battery_charge_min_pct>\n"
        "   <battery_max_temperature>%f</battery_max_temperature>\n"
        "   <run_on_batteries>%d</run_on_batteries>\n"
        "   <run_if_user_active>%d</run_if_user_active>\n"
        "   <run_gpu_if_user_active>%d</run_gpu_if_user_active>\n"
        "   <suspend_if_no_recent_input>%f</suspend_if_no_recent_input>\n"
        "   <suspend_cpu_usage>%f</suspend_cpu_usage>\n"
        "   <start_hour>%f</start_hour>\n"
        "   <end_hour>%f</end_hour>\n"
        "   <net_start_hour>%f</net_start_hour>\n"
        "   <net_end_hour>%f</net_end_hour>\n"
        "   <leave_apps_in_memory>%d</leave_apps_in_memory>\n"
        "   <confirm_before_connecting>%d</confirm_before_connecting>\n"
        "   <hangup_if_dialed>%d</hangup_if_dialed>\n"
        "   <dont_verify_images>%d</dont_verify_images>\n"
        "   <work_buf_min_days>%f</work_buf_min_days>\n"
        "   <work_buf_additional_days>%f</work_buf_additional_days>\n"
        "   <max_ncpus_pct>%f</max_ncpus_pct>\n"
        "   <niu_max_ncpus_pct>%f</niu_max_ncpus_pct>\n"
        "   <niu_cpu_usage_limit>%f</niu_cpu_usage_limit>\n"
        "   <niu_suspend_cpu_usage>%f</niu_suspend_cpu_usage>\n"
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
        "   <daily_xfer_limit_mb>%f</daily_xfer_limit_mb>\n"
        "   <daily_xfer_period_days>%d</daily_xfer_period_days>\n"
        "   <override_file_present>%d</override_file_present>\n"
        "   <network_wifi_only>%d</network_wifi_only>\n",
        source_project,
        mod_time,
        battery_charge_min_pct,
        battery_max_temperature,
        run_on_batteries?1:0,
        run_if_user_active?1:0,
        run_gpu_if_user_active?1:0,
        suspend_if_no_recent_input,
        suspend_cpu_usage,
        cpu_times.start_hour,
        cpu_times.end_hour,
        net_times.start_hour,
        net_times.end_hour,
        leave_apps_in_memory?1:0,
        confirm_before_connecting?1:0,
        hangup_if_dialed?1:0,
        dont_verify_images?1:0,
        work_buf_min_days,
        work_buf_additional_days,
        max_ncpus_pct,
        niu_max_ncpus_pct,
        niu_cpu_usage_limit,
        niu_suspend_cpu_usage,
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
        cpu_usage_limit,
        daily_xfer_limit_mb,
        daily_xfer_period_days,
        override_file_present?1:0,
        network_wifi_only?1:0
    );
    if (max_ncpus) {
        f.printf("   <max_cpus>%d</max_cpus>\n", max_ncpus);
    }

    write_day_prefs(f);

    f.printf("</global_preferences>\n");

    return 0;
}

void GLOBAL_PREFS::write_day_prefs(MIOFILE& f) {
    for (int i=0; i<7; i++) {
        bool cpu_present = cpu_times.week.days[i].present;
        bool net_present = net_times.week.days[i].present;
        //write only when needed
        if (net_present || cpu_present) {

            f.printf("   <day_prefs>\n");
            f.printf("      <day_of_week>%d</day_of_week>\n", i);
            if (cpu_present) {
                f.printf(
                    "      <start_hour>%.02f</start_hour>\n"
                    "      <end_hour>%.02f</end_hour>\n",
                    cpu_times.week.days[i].start_hour,
                    cpu_times.week.days[i].end_hour
                );
            }
            if (net_present) {
                f.printf(
                    "      <net_start_hour>%.02f</net_start_hour>\n"
                    "      <net_end_hour>%.02f</net_end_hour>\n",
                    net_times.week.days[i].start_hour,
                    net_times.week.days[i].end_hour
                );
            }
            f.printf("   </day_prefs>\n");
        }
    }
}

// write a subset of the global preferences,
// as selected by the mask of bools
//
int GLOBAL_PREFS::write_subset(MIOFILE& f, GLOBAL_PREFS_MASK& mask) {
    if (!mask.are_prefs_set()) return 0;

    f.printf("<global_preferences>\n");
    if (mask.run_on_batteries) {
        f.printf("   <run_on_batteries>%d</run_on_batteries>\n",
            run_on_batteries?1:0
        );
    }
    if (mask.run_if_user_active) {
        f.printf("   <run_if_user_active>%d</run_if_user_active>\n",
            run_if_user_active?1:0
        );
    }
    if (mask.run_gpu_if_user_active) {
        f.printf("   <run_gpu_if_user_active>%d</run_gpu_if_user_active>\n",
            run_gpu_if_user_active?1:0
        );
    }
    if (mask.idle_time_to_run) {
        f.printf("   <idle_time_to_run>%f</idle_time_to_run>\n", idle_time_to_run);
    }
    if (mask.suspend_if_no_recent_input) {

        f.printf("   <suspend_if_no_recent_input>%f</suspend_if_no_recent_input>\n",
            suspend_if_no_recent_input
        );
    }
    if (mask.suspend_cpu_usage) {
        f.printf("   <suspend_cpu_usage>%f</suspend_cpu_usage>\n",
            suspend_cpu_usage
        );
    }
    if (mask.start_hour) {
        f.printf("   <start_hour>%f</start_hour>\n", cpu_times.start_hour);
    }
    if (mask.end_hour) {
        f.printf("   <end_hour>%f</end_hour>\n", cpu_times.end_hour);
    }
    if (mask.net_start_hour) {
        f.printf("   <net_start_hour>%f</net_start_hour>\n", net_times.start_hour);
    }
    if (mask.net_end_hour) {
        f.printf("   <net_end_hour>%f</net_end_hour>\n", net_times.end_hour);
    }
    if (mask.leave_apps_in_memory) {
        f.printf("   <leave_apps_in_memory>%d</leave_apps_in_memory>\n",
            leave_apps_in_memory?1:0
        );
    }
    if (mask.battery_charge_min_pct) {
        f.printf("   <battery_charge_min_pct>%f</battery_charge_min_pct>\n",
            battery_charge_min_pct
        );
    }
    if (mask.battery_max_temperature) {
        f.printf("   <battery_max_temperature>%f</battery_max_temperature>\n",
            battery_max_temperature
        );
    }
    if (mask.confirm_before_connecting) {
        f.printf("   <confirm_before_connecting>%d</confirm_before_connecting>\n",
            confirm_before_connecting?1:0
        );
    }
    if (mask.hangup_if_dialed) {
        f.printf("   <hangup_if_dialed>%d</hangup_if_dialed>\n",
            hangup_if_dialed?1:0
        );
    }
    if (mask.dont_verify_images) {
        f.printf("   <dont_verify_images>%d</dont_verify_images>\n",
            dont_verify_images?1:0
        );
    }
    if (mask.work_buf_min_days) {
        f.printf("   <work_buf_min_days>%f</work_buf_min_days>\n", work_buf_min_days);
    }
    if (mask.work_buf_additional_days) {
        f.printf("   <work_buf_additional_days>%f</work_buf_additional_days>\n", work_buf_additional_days);
    }
    if (mask.max_ncpus_pct) {
        f.printf("   <max_ncpus_pct>%f</max_ncpus_pct>\n", max_ncpus_pct);
    }
    if (mask.niu_max_ncpus_pct) {
        f.printf("   <niu_max_ncpus_pct>%f</niu_max_ncpus_pct>\n", niu_max_ncpus_pct);
    }
    if (mask.niu_cpu_usage_limit) {
        f.printf("   <niu_cpu_usage_limit>%f</niu_cpu_usage_limit>\n", niu_cpu_usage_limit);
    }
    if (mask.niu_suspend_cpu_usage) {
        f.printf("   <niu_suspend_cpu_usage>%f</niu_suspend_cpu_usage>\n",
            niu_suspend_cpu_usage
        );
    }
    if (mask.max_ncpus) {
        f.printf("   <max_cpus>%d</max_cpus>\n", max_ncpus);
    }
    if (mask.cpu_scheduling_period_minutes) {
        f.printf("   <cpu_scheduling_period_minutes>%f</cpu_scheduling_period_minutes>\n", cpu_scheduling_period_minutes);
    }
    if (mask.disk_interval) {
        f.printf("   <disk_interval>%f</disk_interval>\n", disk_interval);
    }
    if (mask.disk_max_used_gb) {
        f.printf("   <disk_max_used_gb>%f</disk_max_used_gb>\n", disk_max_used_gb);
    }
    if (mask.disk_max_used_pct) {
        f.printf("   <disk_max_used_pct>%f</disk_max_used_pct>\n", disk_max_used_pct);
    }
    if (mask.disk_min_free_gb) {
        f.printf("   <disk_min_free_gb>%f</disk_min_free_gb>\n", disk_min_free_gb);
    }
    if (mask.vm_max_used_frac) {
        f.printf("   <vm_max_used_pct>%f</vm_max_used_pct>\n", vm_max_used_frac*100);
    }
    if (mask.ram_max_used_busy_frac) {
        f.printf("   <ram_max_used_busy_pct>%f</ram_max_used_busy_pct>\n", ram_max_used_busy_frac*100);
    }
    if (mask.ram_max_used_idle_frac) {
        f.printf("   <ram_max_used_idle_pct>%f</ram_max_used_idle_pct>\n", ram_max_used_idle_frac*100);
    }
    if (mask.max_bytes_sec_up) {
        f.printf("   <max_bytes_sec_up>%f</max_bytes_sec_up>\n", max_bytes_sec_up);
    }
    if (mask.max_bytes_sec_down) {
        f.printf("   <max_bytes_sec_down>%f</max_bytes_sec_down>\n", max_bytes_sec_down);
    }
    if (mask.cpu_usage_limit) {
        f.printf("   <cpu_usage_limit>%f</cpu_usage_limit>\n", cpu_usage_limit);
    }
    if (mask.daily_xfer_limit_mb) {
        f.printf("   <daily_xfer_limit_mb>%f</daily_xfer_limit_mb>\n", daily_xfer_limit_mb);
    }
    if (mask.daily_xfer_period_days) {
        f.printf("   <daily_xfer_period_days>%d</daily_xfer_period_days>\n", daily_xfer_period_days);
    }
    if (mask.network_wifi_only) {
        f.printf("   <network_wifi_only>%d</network_wifi_only>\n", network_wifi_only?1:0 );
    }

    write_day_prefs(f);
    f.printf("</global_preferences>\n");
    return 0;
}

// parse the <mod_time> element from a prefs XML string
//
double GLOBAL_PREFS::parse_mod_time(const char* p) {
    const char *q = strstr(p, "<mod_time>");
    if (!q) return 0;
    return atof(q+strlen("<mod_time>"));
}
