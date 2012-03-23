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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <time.h>
#endif

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "parse.h"
#include "util.h"

#include "error_numbers.h"
#include "prefs.h"


GLOBAL_PREFS_MASK::GLOBAL_PREFS_MASK() {
    clear();
}

void GLOBAL_PREFS_MASK::clear() {
    memset(this, 0, sizeof(GLOBAL_PREFS_MASK));
}

void GLOBAL_PREFS_MASK::set_all() {
    run_on_batteries = true;
    run_if_user_active = true;
    run_gpu_if_user_active = true;
    idle_time_to_run = true;
    suspend_if_no_recent_input = true;
    suspend_cpu_usage = 0;
    start_hour = true;
    end_hour = true;
    net_start_hour = true;
    net_end_hour = true;
    leave_apps_in_memory = true;
    confirm_before_connecting = true;
    hangup_if_dialed = true;
    dont_verify_images = true;
    work_buf_min_days = true;
    work_buf_additional_days = true;
    max_ncpus_pct = true;
    max_ncpus= true;
    cpu_scheduling_period_minutes = true;
    disk_interval = true;
    disk_max_used_gb = true;
    disk_max_used_pct = true;
    disk_min_free_gb = true;
    vm_max_used_frac = true;
    ram_max_used_busy_frac = true;
    ram_max_used_idle_frac = true;
    idle_time_to_run = true;
    max_bytes_sec_up = true;
    max_bytes_sec_down = true;
    cpu_usage_limit = true;
    daily_xfer_limit_mb = true;
    daily_xfer_period_days = true;
}

bool GLOBAL_PREFS_MASK::are_prefs_set() {
    if (run_on_batteries) return true;
    if (run_if_user_active) return true;
    if (run_gpu_if_user_active) return true;
    if (idle_time_to_run) return true;
    if (suspend_if_no_recent_input) return true;
    if (suspend_cpu_usage) return true;
    if (start_hour) return true;
    if (end_hour) return true;
    if (net_start_hour) return true;
    if (net_end_hour) return true;
    if (leave_apps_in_memory) return true;
    if (confirm_before_connecting) return true;
    if (hangup_if_dialed) return true;
    if (dont_verify_images) return true;
    if (work_buf_min_days) return true;
    if (work_buf_additional_days) return true;
    if (max_ncpus_pct) return true;
    if (max_ncpus) return true;
    if (cpu_scheduling_period_minutes) return true;
    if (disk_interval) return true;
    if (disk_max_used_gb) return true;
    if (disk_max_used_pct) return true;
    if (disk_min_free_gb) return true;
    if (vm_max_used_frac) return true;
    if (ram_max_used_busy_frac) return true;
    if (ram_max_used_idle_frac) return true;
    if (idle_time_to_run) return true;
    if (max_bytes_sec_up) return true;
    if (max_bytes_sec_down) return true;
    if (cpu_usage_limit) return true;
    if (daily_xfer_limit_mb) return true;
    if (daily_xfer_period_days) return true;
    return false;
}

bool GLOBAL_PREFS_MASK::are_simple_prefs_set() {
    if (start_hour) return true;
    if (end_hour) return true;
    if (net_start_hour) return true;
    if (net_end_hour) return true;
    if (disk_max_used_gb) return true;
    if (cpu_usage_limit) return true;
    if (run_on_batteries) return true;
    if (run_if_user_active) return true;
    if (idle_time_to_run) return true;
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
    days[day].present = true;
    days[day].start_hour = start;
    days[day].end_hour = end;
}


void WEEK_PREFS::set(int day, TIME_SPAN* time) {
    if (day < 0 || day > 6) return;
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
    run_on_batteries = true;
    run_if_user_active = true;
    run_gpu_if_user_active = false;
    idle_time_to_run = 3;
    suspend_if_no_recent_input = 0;
    suspend_cpu_usage = 25;
    cpu_times.clear();
    net_times.clear();
    leave_apps_in_memory = false;
    confirm_before_connecting = true;
    hangup_if_dialed = false;
    dont_verify_images = false;
    work_buf_min_days = 0.1;
    work_buf_additional_days = 0.5;
    max_ncpus_pct = 0;
    max_ncpus = 0;
    cpu_scheduling_period_minutes = 60;
    disk_interval = 60;
    disk_max_used_gb = 10;
    disk_max_used_pct = 50;
    disk_min_free_gb = 0.1;
    vm_max_used_frac = 0.75;
    ram_max_used_busy_frac = 0.5;
    ram_max_used_idle_frac = 0.9;
    max_bytes_sec_up = 0;
    max_bytes_sec_down = 0;
    cpu_usage_limit = 100;
    daily_xfer_limit_mb = 0;
    daily_xfer_period_days = 0;

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

void GLOBAL_PREFS::init() {
    defaults();
    strcpy(source_project, "");
    strcpy(source_scheduler, "");
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
    clear_bools();
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
            return 0;
        }
        if (in_venue) {
            if (xp.match_tag("/venue")) {
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
            if (strstr(xp.parsed_tag, "venue")) {
                in_venue = true;
                parse_attr(attrs, "name", buf2, sizeof(buf2));
                if (!strcmp(buf2, host_venue)) {
                    defaults();
                    clear_bools();
                    mask.clear();
                    in_correct_venue = true;
                    found_venue = true;
                } else {
                    in_correct_venue = false;
                }
                continue;
            }
        }
        if (xp.parse_str("source_project", source_project, sizeof(source_project))) continue;
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
        "   <override_file_present>%d</override_file_present>\n",
        source_project,
        mod_time,
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
        override_file_present?1:0
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

    write_day_prefs(f);
    f.printf("</global_preferences>\n");
    return 0;
}

