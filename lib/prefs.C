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
#include "util.h"

#include "error_numbers.h"
#include "prefs.h"

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
    if (end_hour == start_hour || (start_hour == 0.0 && end_hour == 24.0)) {
        return Always;
    } else if (start_hour == 24.0 && end_hour == 0.0) {
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


bool TIME_PREFS::suspended() const {
    time_t now = time(0);
    struct tm* tmp = localtime(&now);
    double hour = (tmp->tm_hour * 3600 + tmp->tm_min * 60 + tmp->tm_sec) / 3600.;
    int day = tmp->tm_wday;

    // Use day-specific settings, if they exist:
    const TIME_SPAN* span = week.get(day) ? week.get(day) : this;

    return span->suspended(hour);
}


// WEEK_PREFS implementation

WEEK_PREFS::WEEK_PREFS() {
    for (int i=0; i<7; i++) {
        days[i] = 0;
    }
}


WEEK_PREFS::WEEK_PREFS(const WEEK_PREFS& original) {
    for (int i=0; i<7; i++) {
        TIME_SPAN* time = original.days[i];
        if (time) {
            days[i] = new TIME_SPAN(time->start_hour, time->end_hour);
        } else {
            days[i] = 0;
        }
    }
}


WEEK_PREFS& WEEK_PREFS::operator=(const WEEK_PREFS& rhs) {
    if (this != &rhs) {
        for (int i=0; i<7; i++) {
            TIME_SPAN* time = rhs.days[i];
            if (time) {
                if (days[i]) {
                    *days[i] = *time;
                } else {
                    days[i] = new TIME_SPAN(*time);
                }
            } else {
                unset(i);
            }
        }
    }
    return *this;
}


// Create a deep copy.
void WEEK_PREFS::copy(const WEEK_PREFS& original) {
    for (int i=0; i<7; i++) {
        TIME_SPAN* time = original.days[i];
        if (time) {
            days[i] = new TIME_SPAN(time->start_hour, time->end_hour);
        } else {
            days[i] = 0;
        }
    }
}


WEEK_PREFS::~WEEK_PREFS() {
    clear();
}


void WEEK_PREFS::clear() {
    for (int i=0; i<7; i++) {
        if (days[i]) {
            delete days[i];
            days[i] = 0;
        }
    }
}


TIME_SPAN* WEEK_PREFS::get(int day) const {

    if (day < 0 || day > 6) return 0;
    return days[day];
}


void WEEK_PREFS::set(int day, double start, double end) {
    if (day < 0 || day > 6) return;
    if (days[day]) delete days[day];
    days[day] = new TIME_SPAN(start, end);
}


void WEEK_PREFS::set(int day, TIME_SPAN* time) {
    if (day < 0 || day > 6) return;
    if (days[day] == time) return;
    if (days[day]) delete days[day];
    days[day] = time;
}

void WEEK_PREFS::unset(int day) {
    if (day < 0 || day > 6) return;
    if (days[day]) {
        delete days[day];
        days[day] = 0;
    }
}

int GLOBAL_PREFS::parse_file(const char* filename, std::vector<GLOBAL_PREFS*>& venues) {
    FILE* f;
    int retval;

    f = fopen(filename, "r");
    if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    retval = parse_venues(xp, venues);
    fclose(f);
    return retval;
}

// Parses all venues (including the default nameless venue) into the supplied vector.
// Also returns the requested venue, or the default venue if it isn't found.
int GLOBAL_PREFS::parse_venues(XML_PARSER& xp, std::vector<GLOBAL_PREFS*>& venues) {
    char tag[256], venue_name[32];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "global_preferences")) {
            // parse default venue
            GLOBAL_PREFS* default_venue = new GLOBAL_PREFS();
            default_venue->parse_preference_tags(xp);
            venues.push_back(default_venue);
            continue;
        }
        if (!strcmp(tag, "/global_preferences")) {
            return 0;
        }
        if (strstr(tag, "venue")) {
            GLOBAL_PREFS* venue = new GLOBAL_PREFS();
            parse_attr(tag, "name", venue_name, sizeof(venue_name));
            strncpy(venue->venue_name, venue_name, sizeof(venue->venue_name));
            venue->parse_preference_tags(xp);
            venues.push_back(venue);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

// The following values determine how the client behaves
// if there are no global prefs (e.g. on our very first RPC).
// These should impose minimal restrictions,
// so that the client can do the RPC and get the global prefs from the server
//
void GLOBAL_PREFS::defaults() {
    run_on_batteries = true;
    run_if_user_active = true;
    idle_time_to_run = 3;
    suspend_if_no_recent_input = 0;
    cpu_times.clear();
    net_times.clear();
    leave_apps_in_memory = false;
    confirm_before_connecting = true;
    hangup_if_dialed = false;
    dont_verify_images = false;
    work_buf_min_days = 0.1;
    work_buf_additional_days = 0.25;
    max_ncpus_pct = 100;
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
    strcpy(venue_name, "");
    strcpy(venue_description, "");
    defaults();
}

// Parse XML global prefs, setting defaults first.
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

int GLOBAL_PREFS::parse_day(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;

    int day_of_week = -1;
    bool has_cpu = false;
    bool has_net = false;
    double start_hour = 0;
    double end_hour = 0;
    double net_start_hour = 0;
    double net_end_hour = 0;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/day_prefs")) {
            if (day_of_week < 0 || day_of_week > 6) return ERR_XML_PARSE;
            if (has_cpu) {
                cpu_times.week.set(day_of_week, start_hour, end_hour);
            }
            if (has_net) {
                net_times.week.set(day_of_week, net_start_hour, net_end_hour);
            }
            return 0;
        }
        if (xp.parse_int(tag, "day_of_week", day_of_week)) continue;
        if (xp.parse_double(tag, "start_hour", start_hour)) {
            has_cpu = true;
            continue;
        }
        if (xp.parse_double(tag, "end_hour", end_hour)) {
            has_cpu = true;
            continue;
        }
        if (xp.parse_double(tag, "net_start_hour", net_start_hour)) {
            has_net = true;
            continue;
        }
        if (xp.parse_double(tag, "net_end_hour", net_end_hour)) {
            has_net = true;
            continue;
        }
        xp.skip_unexpected(tag, true, "GLOBAL_PREFS::parse_day");
    }
    return ERR_XML_PARSE;
}


// DEPRECATED
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
    XML_PARSER& xp, const char* host_venue, bool& found_venue
) {
    char tag[256], buf2[256];
    bool in_venue = false, in_correct_venue=false, is_tag;

    found_venue = false;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "global_preferences")) continue;
        if (!strcmp(tag, "/global_preferences")) {
            return 0;
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
        parse_preference_tags(xp);
    }
    return ERR_XML_PARSE;
}

// Parse global prefs, overriding whatever is currently in the structure.
// xp must be positioned at the start of the structure to parse.
// The opening tag is already consumed.
//
int GLOBAL_PREFS::parse_preference_tags(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    double dtemp;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/global_preferences")) {
            return 0;
        }
        if (!strcmp(tag, "/venue")) {
            return 0;
        }
        if (xp.parse_str(tag, "venue_description", venue_description, sizeof(venue_description))) continue;
        if (xp.parse_str(tag, "source_project", source_project, sizeof(source_project))) continue;
        if (xp.parse_str(tag, "source_scheduler", source_scheduler, sizeof(source_scheduler))) {
            continue;
        }
        if (xp.parse_double(tag, "mod_time", mod_time)) {
            double now = dtime();
            if (mod_time > now) {
                mod_time = now;
            }
            continue;
        }
        if (xp.parse_bool(tag, "run_on_batteries", run_on_batteries)) continue;
        if (xp.parse_bool(tag, "run_if_user_active", run_if_user_active)) continue;
        if (xp.parse_double(tag, "idle_time_to_run", idle_time_to_run)) continue;
        if (xp.parse_double(tag, "suspend_if_no_recent_input", suspend_if_no_recent_input)) continue;
        if (xp.parse_double(tag, "start_hour", cpu_times.start_hour)) continue;
        if (xp.parse_double(tag, "end_hour", cpu_times.end_hour)) continue;
        if (xp.parse_double(tag, "net_start_hour", net_times.start_hour)) continue;
        if (xp.parse_double(tag, "net_end_hour", net_times.end_hour)) continue;

        if (!strcmp(tag, "day_prefs")) {
            parse_day(xp);
            continue;
        }
        if (xp.parse_bool(tag, "leave_apps_in_memory", leave_apps_in_memory)) continue;
        if (xp.parse_bool(tag, "confirm_before_connecting", confirm_before_connecting)) continue;
        if (xp.parse_bool(tag, "hangup_if_dialed", hangup_if_dialed)) continue;
        if (xp.parse_bool(tag, "dont_verify_images", dont_verify_images)) continue;

        if (xp.parse_double(tag, "work_buf_min_days", work_buf_min_days)) {
            if (work_buf_min_days < 0.00001) work_buf_min_days = 0.00001;
            continue;
        }
        if (xp.parse_double(tag, "work_buf_additional_days", work_buf_additional_days)) {
            if (work_buf_additional_days < 0) work_buf_additional_days = 0;
            continue;
        }
        if (xp.parse_double(tag, "max_ncpus_pct", max_ncpus_pct)) {
            if (max_ncpus_pct <= 0) max_ncpus_pct = 100;
            if (max_ncpus_pct > 100) max_ncpus_pct = 100;
            continue;
        }
        if (xp.parse_double(tag, "disk_interval", disk_interval)) {
            if (disk_interval<0) disk_interval = 0;
            continue;
        }
        if (xp.parse_double(tag, "cpu_scheduling_period_minutes", cpu_scheduling_period_minutes)) {
            if (cpu_scheduling_period_minutes < 0.0001) cpu_scheduling_period_minutes = 60;
            continue;
        }
        if (xp.parse_double(tag, "disk_max_used_gb", disk_max_used_gb)) continue;
        if (xp.parse_double(tag, "disk_max_used_pct", disk_max_used_pct)) continue;
        if (xp.parse_double(tag, "disk_min_free_gb", disk_min_free_gb)) continue;

        if (xp.parse_double(tag, "vm_max_used_pct", dtemp)) {
            vm_max_used_frac = dtemp/100;
            continue;
        }
        if (xp.parse_double(tag, "ram_max_used_busy_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
            ram_max_used_busy_frac = dtemp/100;
            continue;
        }
        if (xp.parse_double(tag, "ram_max_used_idle_pct", dtemp)) {
            if (!dtemp) dtemp = 100;
            ram_max_used_idle_frac = dtemp/100;
            continue;
        }
        if (xp.parse_double(tag, "max_bytes_sec_up", max_bytes_sec_up)) {
            if (max_bytes_sec_up < 0) max_bytes_sec_up = 0;
            continue;
        }
        if (xp.parse_double(tag, "max_bytes_sec_down", max_bytes_sec_down)) {
            if (max_bytes_sec_down < 0) max_bytes_sec_down = 0;
            continue;
        }
        if (xp.parse_double(tag, "cpu_usage_limit", dtemp)) {
            if (dtemp > 0 && dtemp <= 100) {
                cpu_usage_limit = dtemp;
            }
            continue;
        }
        if (xp.parse_bool(tag, "host_specific", host_specific)) {
            continue;
        }
        // false means don't print anything
        xp.skip_unexpected(tag, false, "GLOBAL_PREFS::parse_override");
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
        "   <mod_time>%f</mod_time>\n"
        "   <venue_description>%s</venue_description>\n"
        "%s%s"
        "   <suspend_if_no_recent_input>%f</suspend_if_no_recent_input>\n"
        "   <start_hour>%f</start_hour>\n"
        "   <end_hour>%f</end_hour>\n"
        "   <net_start_hour>%f</net_start_hour>\n"
        "   <net_end_hour>%f</net_end_hour>\n"
        "%s%s%s%s"
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
        "   <cpu_usage_limit>%f</cpu_usage_limit>\n",
        mod_time,
        venue_description,
        run_on_batteries?"   <run_on_batteries/>\n":"",
        run_if_user_active?"   <run_if_user_active/>\n":"",
        suspend_if_no_recent_input,
        cpu_times.start_hour,
        cpu_times.end_hour,
        net_times.start_hour,
        net_times.end_hour,
        leave_apps_in_memory?"   <leave_apps_in_memory/>\n":"",
        confirm_before_connecting?"   <confirm_before_connecting/>\n":"",
        hangup_if_dialed?"   <hangup_if_dialed/>\n":"",
        dont_verify_images?"   <dont_verify_images/>\n":"",
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
        cpu_usage_limit
    );

    for (int i=0; i<7; i++) {
        TIME_SPAN* cpu = cpu_times.week.get(i);
        TIME_SPAN* net = net_times.week.get(i);
        //write only when needed
        if (net || cpu) {
            
            f.printf("   <day_prefs>\n");                
            f.printf("      <day_of_week>%d</day_of_week>\n", i);
            if (cpu) {
                f.printf("      <start_hour>%.02f</start_hour>\n", cpu->start_hour);
                f.printf("      <end_hour>%.02f</end_hour>\n", cpu->end_hour);
            }
            if (net) {
                f.printf("      <net_start_hour>%.02f</net_start_hour>\n", net->start_hour);
                f.printf("      <net_end_hour>%.02f</net_end_hour>\n", net->end_hour);
            }
            f.printf("   </day_prefs>\n");
        }
    }
    f.printf("</global_preferences>\n");

    return 0;
}

const char *BOINC_RCSID_3fb442bb02 = "$Id$";



