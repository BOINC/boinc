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
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <string.h>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "message.h"
#include "parse.h"

#include "log_flags.h"
#include "filesys.h"

LOG_FLAGS log_flags;

LOG_FLAGS::LOG_FLAGS() {

    // informational output is on by default
    //
    task = true;
    file_xfer = true;
    sched_ops = true;

    // debugging output is off by default
    //
    state_debug = false;
    task_debug = false;
    file_xfer_debug = false;
    sched_op_debug = false;
    http_debug = false;
    time_debug = false;
    net_xfer_debug = false;
    measurement_debug = false;
}

// Parse log flag preferences
//
int LOG_FLAGS::parse(FILE* in) {
    char buf[256];

    fgets(buf, 256, in);
    if (!match_tag(buf, "<log_flags>")) return ERR_XML_PARSE;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</log_flags>")) return 0;
        else if (match_tag(buf, "<task/>")) {
            task = true;
            continue;
        } else if (match_tag(buf, "<file_xfer/>")) {
            file_xfer = true;
            continue;
        } else if (match_tag(buf, "<sched_ops/>")) {
            sched_ops = true;
            continue;
        } else if (match_tag(buf, "<state_debug/>")) {
            state_debug = true;
            continue;
        } else if (match_tag(buf, "<task_debug/>")) {
            task_debug = true;
            continue;
        } else if (match_tag(buf, "<file_xfer_debug/>")) {
            file_xfer_debug = true;
            continue;
        } else if (match_tag(buf, "<sched_op_debug/>")) {
            sched_op_debug = true;
            continue;
        } else if (match_tag(buf, "<http_debug/>")) {
            http_debug = true;
            continue;
        } else if (match_tag(buf, "<time_debug/>")) {
            time_debug = true;
            continue;
        } else if (match_tag(buf, "<net_xfer_debug/>")) {
            net_xfer_debug = true;
            continue;
        } else if (match_tag(buf, "<measurement_debug/>")) {
            measurement_debug = true;
            continue;
        } else if (match_tag(buf, "<poll_debug/>")) {
            poll_debug = true;
            continue;
        } else if (match_tag(buf, "<dont_check_file_sizes/>")) {
            dont_check_file_sizes = true;
            continue;
        }
        else msg_printf(NULL, MSG_ERROR, "LOG_FLAGS::parse: unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

void read_log_flags() {
    FILE* f;

    if (boinc_file_exists(LOG_FLAGS_FILE)) {
        f = fopen(LOG_FLAGS_FILE, "r");
        log_flags.parse(f);
        fclose(f);
    }

}
