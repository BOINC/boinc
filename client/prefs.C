// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "windows_cpp.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parse.h"

#include "error_numbers.h"
#include "file_names.h"

#include "prefs.h"

// the following values determine how the client behaves
// if there are no global prefs yet
//
GLOBAL_PREFS::GLOBAL_PREFS() {
    run_on_batteries = true;
    run_if_user_active = true;
    run_minimized = false;
    run_on_startup = false;
    confirm_before_connecting = false;
    hangup_if_dialed = false;
    work_buf_max_days = 3;
    work_buf_min_days = 1;
    disk_max_used_gb = 1;
    disk_max_used_pct = 0.5;
    disk_min_free_gb = 0.1;
    idle_time_to_run = 0;
    max_bytes_sec_up = 1e9;
    max_bytes_sec_down = 1e9;
};

// Parse XML global prefs
//
int GLOBAL_PREFS::parse(FILE* in) {
    char buf[256];

    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</global_preferences>")) {
            return 0;
        } else if (match_tag(buf, "<run_on_batteries/>")) {
            run_on_batteries = true;
            continue;
        } else if (match_tag(buf, "<run_if_user_active/>")) {
            run_if_user_active = true;
            continue;
        } else if (match_tag(buf, "<confirm_before_connecting/>")) {
            confirm_before_connecting = true;
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
        } else if (parse_double(buf, "<disk_max_used_gb>", disk_max_used_gb)) {
            continue;
        } else if (parse_double(buf, "<disk_max_used_pct>", disk_max_used_pct)) {
            continue;
        } else if (parse_double(buf, "<disk_min_free_gb>", disk_min_free_gb)) {
            continue;
        } else if (parse_double(buf, "<idle_time_to_run>", idle_time_to_run)) {
            continue;
        } else if (parse_double(buf, "<max_bytes_sec_up>", max_bytes_sec_up)) {
			if (max_bytes_sec_up <= 0) max_bytes_sec_up = 1e9;
            continue;
        } else if (parse_double(buf, "<max_bytes_sec_down>", max_bytes_sec_down)) {
			if (max_bytes_sec_down <= 0) max_bytes_sec_down = 1e9;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

// Parse global prefs file
//
int GLOBAL_PREFS::parse_file() {
    FILE* f;
    int retval;

    f = fopen(GLOBAL_PREFS_FILE_NAME, "r");
    if (!f) return ERR_FOPEN;
    retval = parse(f);
    fclose(f);
    return retval;
}
