// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

#include "parse.h"

#include "error_numbers.h"
#include "client_msgs.h"
#include "file_names.h"

#include "client_state.h"
#include "prefs.h"

// The following values determine how the client behaves
// if there are no global prefs yet (e.g. on our very first RPC).
// - Should impose minimal restrictions, so that the client can do the RPC
// and get the global prefs from the server
//
void GLOBAL_PREFS::init() {
    run_on_batteries = true;
    run_if_user_active = true;
    start_hour = 0;
    end_hour = 0;
    run_minimized = false;
    run_on_startup = false;
    confirm_before_connecting = false;
    hangup_if_dialed = false;
    work_buf_max_days = 0.2;
    work_buf_min_days = 0.1;
    max_cpus = 1;
    disk_interval = 60;
    disk_max_used_gb = 1;
    disk_max_used_pct = 0.5;
    disk_min_free_gb = 0.1;
    idle_time_to_run = 3;
    max_bytes_sec_up = 1e9;
    max_bytes_sec_down = 1e9;
    max_memory_mbytes = 128;
    proc_priority = 1;
    cpu_affinity = -1;
};

GLOBAL_PREFS::GLOBAL_PREFS() {
    init();
}

// Parse XML global prefs.
// If host_venue is nonempty and we find an element of the form
// <venue name="X">
//   ...
// </venue>
// where X==host_venue, then parse that and ignore the rest.
// Otherwise ignore <venue> elements.
//
int GLOBAL_PREFS::parse(FILE* in, char* host_venue, bool& found_venue) {
    char buf[256], buf2[256];
    bool in_venue = false, in_correct_venue=false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

	init();
    source_project = "";
    source_scheduler = "";

    // set all booleans to false here
    run_on_batteries = false;
    run_if_user_active = false;
    confirm_before_connecting = false;
    run_minimized = false;
    run_on_startup = false;
    hangup_if_dialed = false;

    found_venue = false;
    while (fgets(buf, 256, in)) {
        if (in_venue) {
            if (match_tag(buf, "</venue>")) {
                if (in_correct_venue) {
                    break;
                } else {
                    in_venue = false;
                    continue;
                }
            } else {
                if(!in_correct_venue) continue;
            }
        } else {
            if (match_tag(buf, "<venue")) {
                in_venue = true;
                parse_attr(buf, "name", buf2, sizeof(buf2));
                if (!strcmp(buf2, host_venue)) {
                    init();
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
        } else if (parse_str(buf, "<source_project>", source_project)) {
            continue;
        } else if (parse_str(buf, "<source_scheduler>", source_scheduler)) {
            continue;
        } else if (parse_int(buf, "<mod_time>", mod_time)) {
            continue;
        } else if (match_tag(buf, "</global_preferences>")) {
            break;
        } else if (match_tag(buf, "<run_on_batteries/>")) {
            run_on_batteries = true;
            continue;
        } else if (match_tag(buf, "<run_if_user_active/>")) {
            run_if_user_active = true;
            continue;
        } else if (parse_int(buf, "<start_hour>", start_hour)) {
            continue;
        } else if (parse_int(buf, "<end_hour>", end_hour)) {
            continue;
        } else if (match_tag(buf, "<confirm_before_connecting/>")) {
            confirm_before_connecting = true;
            continue;
        } else if (match_tag(buf, "<hangup_if_dialed/>")) {
            hangup_if_dialed = true;
            continue;
        } else if (match_tag(buf, "<run_minimized/>")) {
            run_minimized = true;
            continue;
        } else if (match_tag(buf, "<run_on_startup/>")) {
            run_on_startup = true;
            continue;
        } else if (parse_double(buf, "<work_buf_max_days>", work_buf_max_days)) {
            continue;
        } else if (parse_double(buf, "<work_buf_min_days>", work_buf_min_days)) {
            continue;
        } else if (parse_int(buf, "<max_cpus>", max_cpus)) {
            if (max_cpus < 1) max_cpus = 1;
            continue;
        } else if (parse_double(buf, "<disk_interval>", disk_interval)) {
            if (disk_interval<0) disk_interval = 0;
            continue;
        } else if (parse_double(buf, "<disk_max_used_gb>", disk_max_used_gb)) {
            continue;
        } else if (parse_double(buf, "<disk_max_used_pct>", disk_max_used_pct)) {
            continue;
        } else if (parse_double(buf, "<disk_min_free_gb>", disk_min_free_gb)) {
            continue;
        } else if (parse_double(buf, "<idle_time_to_run>", idle_time_to_run)) {
            continue;
        } else if (parse_double(buf, "<max_bytes_sec_up>", max_bytes_sec_up)) {
            if (max_bytes_sec_up <= 0) max_bytes_sec_up = 1e12;
            continue;
        } else if (parse_double(buf, "<max_bytes_sec_down>", max_bytes_sec_down)) {
            if (max_bytes_sec_down <= 0) max_bytes_sec_down = 1e12;
            continue;
        } else if (parse_int(buf, "<max_memory_mbytes>", max_memory_mbytes)) {
            continue;
        } else if (parse_int(buf, "<cpu_affinity>", cpu_affinity)) {
            continue;
        } else {
            scope_messages.printf("GLOBAL_PREFS::parse: unrecognized: %s\n", buf);
        }
    }
    return 0;
}

// Parse global prefs file
//
int GLOBAL_PREFS::parse_file(
    char* filename, char* host_venue, bool& found_venue
) {
    FILE* f;
    int retval;

    f = fopen(filename, "r");
    if (!f) return ERR_FOPEN;
    retval = parse(f, host_venue, found_venue);
    fclose(f);
    return retval;
}
