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
#include <time.h>
#endif

#include "parse.h"

#include "error_numbers.h"
#include "prefs.h"


GLOBAL_PREFS_MASK::GLOBAL_PREFS_MASK() {
    clear();
}

void GLOBAL_PREFS_MASK::clear() {
    memset(this, 0, sizeof(GLOBAL_PREFS_MASK));
}

bool GLOBAL_PREFS_MASK::are_prefs_set() {
    if (run_on_batteries) return true;
    if (run_if_user_active) return true;
    if (start_hour) return true;         // 0..23; no restriction if start==end
    if (end_hour) return true;
    if (net_start_hour) return true;     // 0..23; no restriction if start==end
    if (net_end_hour) return true;
    if (leave_apps_in_memory) return true;
    if (confirm_before_connecting) return true;
    if (hangup_if_dialed) return true;
    if (dont_verify_images) return true;
    if (work_buf_min_days) return true;
    if (work_buf_additional_days) return true;
    if (max_cpus) return true;
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
    return false;
}

bool GLOBAL_PREFS_MASK::are_simple_prefs_set() {
    if (start_hour) return true;
    if (end_hour) return true;
    if (net_start_hour) return true;
    if (net_end_hour) return true;
    if (disk_max_used_gb) return true;
    if (cpu_usage_limit) return true;
    if (run_if_user_active) return true;
    if (run_on_batteries) return true;
    if (idle_time_to_run) return true;
    return false;
}

void TIME_PREFS::clear() {
    start_hour = 0;
    end_hour = 0;
    net_start_hour = 0;
    net_end_hour = 0;
}

// The following values determine how the client behaves
// if there are no global prefs (e.g. on our very first RPC).
// These should impose minimal restrictions,
// so that the client can do the RPC and get the global prefs from the server
//
void GLOBAL_PREFS::defaults() {
    run_on_batteries = true;
    run_if_user_active = true;
    time_prefs.clear();
    leave_apps_in_memory = false;
    confirm_before_connecting = true;
    hangup_if_dialed = false;
    dont_verify_images = false;
    work_buf_min_days = 0.1;
    work_buf_additional_days = 0.25;
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
    week_prefs.present = false;
	for(int i=0;i<7;i++) {		
		week_prefs.days[i].time_prefs.clear();
	}
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
    for (int i=0; i<7; i++) {
        week_prefs.days[i].present = false;
    }
}

bool TIME_PREFS::suspended(double hour, int which) {
    double start, end;
    switch (which) {
    case PREFS_CPU:
        start = start_hour;
        end = end_hour;
        break;
    case PREFS_NETWORK:
        start = net_start_hour;
        end = net_end_hour;
        break;
    default:
        return false;
    }
    if (start==end) return false;
    if (start==0 && end==24) return false;  // redundant?
    if (start==24 && end==0) return true;
    if (start < end) {
        return (hour < start || hour > end);
    } else {
        return (hour >= end && hour < start);
    }
}

bool GLOBAL_PREFS::suspended_time_of_day(int which) {
    time_t now = time(0);
    struct tm *tmp = localtime(&now);
    double hour = (tmp->tm_hour*3600 + tmp->tm_min*60 + tmp->tm_sec)/3600.;
    int day = tmp->tm_wday;

    if (week_prefs.present && week_prefs.days[day].present) {
        return week_prefs.days[day].time_prefs.suspended(hour, which);
    } else {
        return time_prefs.suspended(hour, which);
    }
}

GLOBAL_PREFS::GLOBAL_PREFS() {
    defaults();
}

// Parse XML global prefs, setting defaults first.
//
int GLOBAL_PREFS::parse(
    XML_PARSER& xp, const char* host_venue, bool& found_venue, GLOBAL_PREFS_MASK& mask
) {
    defaults();
    clear_bools();

    strcpy(source_project, "");
    strcpy(source_scheduler, "");
    mod_time = 0;
    host_specific = false;

    return parse_override(xp, host_venue, found_venue, mask);
}

int DAY_PREFS::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;

    day_of_week = -1;
    time_prefs.clear();
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/day_prefs")) {
            if (day_of_week < 0 || day_of_week > 6) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_int(tag, "day_of_week", day_of_week)) continue;
        if (xp.parse_double(tag, "start_hour", time_prefs.start_hour)) continue;
        if (xp.parse_double(tag, "end_hour", time_prefs.end_hour)) continue;
        if (xp.parse_double(tag, "net_start_hour", time_prefs.net_start_hour)) continue;
        if (xp.parse_double(tag, "net_end_hour", time_prefs.net_end_hour)) continue;
        xp.skip_unexpected(tag);
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
    char tag[256], buf2[256];
    bool in_venue = false, in_correct_venue=false, is_tag;
    double dtemp;
    int retval;

    found_venue = false;
    mask.clear();

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "global_preferences")) continue;
        if (!strcmp(tag, "/global_preferences"))  return 0;
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
                    mask.clear();
                    in_correct_venue = true;
                    found_venue = true;
                } else {
                    in_correct_venue = false;
                }
                continue;
            }
        }
        if (xp.parse_str(tag, "source_project", source_project, sizeof(source_project))) continue;
        if (xp.parse_str(tag, "source_scheduler", source_scheduler, sizeof(source_scheduler))) {
            continue;
        }
        if (xp.parse_int(tag, "mod_time", mod_time)) continue;
        if (xp.parse_bool(tag, "run_on_batteries", run_on_batteries)) {
            mask.run_on_batteries = true;
            continue;
        }
        if (xp.parse_bool(tag, "run_if_user_active", run_if_user_active)) {
            mask.run_if_user_active = true;
            continue;
        }
        if (xp.parse_double(tag, "start_hour", time_prefs.start_hour)) {
            mask.start_hour = true;
            continue;
        }
        if (xp.parse_double(tag, "end_hour", time_prefs.end_hour)) {
            mask.end_hour = true;
            continue;
        }
        if (xp.parse_double(tag, "net_start_hour", time_prefs.net_start_hour)) {
            mask.net_start_hour = true;
            continue;
        }
        if (xp.parse_double(tag, "net_end_hour", time_prefs.net_end_hour)) {
            mask.net_end_hour = true;
            continue;
        }
        if (!strcmp(tag, "day_prefs")) {
            DAY_PREFS dp;
            retval = dp.parse(xp);
            if (!retval) {
                dp.present = true;
                week_prefs.present = true;
                week_prefs.days[dp.day_of_week] = dp;
            }
        }
        if (xp.parse_bool(tag, "leave_apps_in_memory", leave_apps_in_memory)) {
            mask.leave_apps_in_memory = true;
            continue;
        }
        if (xp.parse_bool(tag, "confirm_before_connecting", confirm_before_connecting)) {
            mask.confirm_before_connecting = true;
            continue;
        }
        if (xp.parse_bool(tag, "hangup_if_dialed", hangup_if_dialed)) {
            mask.hangup_if_dialed = true;
            continue;
        }
        if (xp.parse_bool(tag, "dont_verify_images", dont_verify_images)) {
            mask.dont_verify_images = true;
            continue;
        }
        if (xp.parse_double(tag, "work_buf_min_days", work_buf_min_days)) {
            if (work_buf_min_days < 0.00001) work_buf_min_days = 0.00001;
            mask.work_buf_min_days = true;
            continue;
        }
        if (xp.parse_double(tag, "work_buf_additional_days", work_buf_additional_days)) {
            if (work_buf_additional_days < 0) work_buf_additional_days = 0;
            mask.work_buf_additional_days = true;
            continue;
        }
        if (xp.parse_int(tag, "max_cpus", max_cpus)) {
            if (max_cpus < 1) max_cpus = 1;
            mask.max_cpus = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_interval", disk_interval)) {
            if (disk_interval<0) disk_interval = 0;
            mask.disk_interval = true;
            continue;
        }
        if (xp.parse_double(tag, "cpu_scheduling_period_minutes", cpu_scheduling_period_minutes)) {
            if (cpu_scheduling_period_minutes < 0.0001) cpu_scheduling_period_minutes = 60;
            mask.cpu_scheduling_period_minutes = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_max_used_gb", disk_max_used_gb)) {
            mask.disk_max_used_gb = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_max_used_pct", disk_max_used_pct)) {
            mask.disk_max_used_pct = true;
            continue;
        }
        if (xp.parse_double(tag, "disk_min_free_gb", disk_min_free_gb)) {
            mask.disk_min_free_gb = true;
            continue;
        }
        if (xp.parse_double(tag, "vm_max_used_pct", dtemp)) {
			vm_max_used_frac = dtemp/100;
            mask.vm_max_used_frac = true;
            continue;
        }
        if (xp.parse_double(tag, "ram_max_used_busy_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
			ram_max_used_busy_frac = dtemp/100;
            mask.ram_max_used_busy_frac = true;
            continue;
        }
        if (xp.parse_double(tag, "ram_max_used_idle_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
			ram_max_used_idle_frac = dtemp/100;
            mask.ram_max_used_idle_frac = true;
            continue;
        }
        if (xp.parse_double(tag, "idle_time_to_run", idle_time_to_run)) {
            mask.idle_time_to_run = true;
            continue;
        }
        if (xp.parse_double(tag, "max_bytes_sec_up", max_bytes_sec_up)) {
            if (max_bytes_sec_up < 0) max_bytes_sec_up = 0;
            mask.max_bytes_sec_up = true;
            continue;
        }
        if (xp.parse_double(tag, "max_bytes_sec_down", max_bytes_sec_down)) {
            if (max_bytes_sec_down < 0) max_bytes_sec_down = 0;
            mask.max_bytes_sec_down = true;
            continue;
        }
        if (xp.parse_double(tag, "cpu_usage_limit", dtemp)) {
            if (dtemp > 0 && dtemp <= 100) {
                cpu_usage_limit = dtemp;
                mask.cpu_usage_limit = true;
            }
            continue;
        }
        if (xp.parse_bool(tag, "host_specific", host_specific)) {
            continue;
        }
        xp.skip_unexpected(tag);
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

    f = fopen(filename, "r");
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
// Not used for scheduler request; there, we just copy the
// global_prefs.xml file (which includes all venues).
//
int GLOBAL_PREFS::write(MIOFILE& f) {
    f.printf(
        "<global_preferences>\n"
        "   <mod_time>%d</mod_time>\n"
        "%s%s"
        "   <start_hour>%f</start_hour>\n"
        "   <end_hour>%f</end_hour>\n"
        "   <net_start_hour>%f</net_start_hour>\n"
        "   <net_end_hour>%f</net_end_hour>\n"
        "%s%s%s%s"
        "   <work_buf_min_days>%f</work_buf_min_days>\n"
        "   <work_buf_additional_days>%f</work_buf_additional_days>\n"
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
        time_prefs.start_hour,
        time_prefs.end_hour,
        time_prefs.net_start_hour,
        time_prefs.net_end_hour,
        leave_apps_in_memory?"   <leave_apps_in_memory/>\n":"",
        confirm_before_connecting?"   <confirm_before_connecting/>\n":"",
        hangup_if_dialed?"   <hangup_if_dialed/>\n":"",
        dont_verify_images?"   <dont_verify_images/>\n":"",
        work_buf_min_days,
        work_buf_additional_days,
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
    if (mask.start_hour) {
        f.printf("   <start_hour>%f</start_hour>\n", time_prefs.start_hour);
    }
    if (mask.end_hour) {
        f.printf("   <end_hour>%f</end_hour>\n", time_prefs.end_hour);
    }
    if (mask.net_start_hour) {
        f.printf("   <net_start_hour>%f</net_start_hour>\n", time_prefs.net_start_hour);
    }
    if (mask.net_end_hour) {
        f.printf("   <net_end_hour>%f</net_end_hour>\n", time_prefs.net_end_hour);
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
    if (mask.max_cpus) {
        f.printf("   <max_cpus>%d</max_cpus>\n", max_cpus);
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
    if (mask.idle_time_to_run) {
        f.printf("   <idle_time_to_run>%f</idle_time_to_run>\n", idle_time_to_run);
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
	if (week_prefs.present) {
		for(int i=0; i< 7;i++) {
			DAY_PREFS dp = week_prefs.days[i];
			//write only when needed
			if(dp.present && 
				(dp.time_prefs.start_hour != dp.time_prefs.end_hour || 
				dp.time_prefs.net_start_hour != dp.time_prefs.net_end_hour)) {
				
				f.printf("   <day_prefs>\n");				
				f.printf("      <day_of_week>%d</day_of_week>\n",dp.day_of_week);
				if(dp.time_prefs.start_hour != dp.time_prefs.end_hour) {
					f.printf("      <start_hour>%.02f</start_hour>\n",dp.time_prefs.start_hour);
					f.printf("      <end_hour>%.02f</end_hour>\n",dp.time_prefs.end_hour);
				}
				if(dp.time_prefs.net_start_hour != dp.time_prefs.net_end_hour) {
					f.printf("      <net_start_hour>%.02f</net_start_hour>\n",dp.time_prefs.net_start_hour);
					f.printf("      <net_end_hour>%.02f</net_end_hour>\n",dp.time_prefs.net_end_hour);
				}
				f.printf("   </day_prefs>\n");
			}
		}
	}
    f.printf("</global_preferences>\n");
    
    return 0;
}

const char *BOINC_RCSID_3fb442bb02 = "$Id$";

