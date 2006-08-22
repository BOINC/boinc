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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstring>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "client_msgs.h"
#include "parse.h"

#include "filesys.h"

LOG_FLAGS log_flags;
CONFIG config;

LOG_FLAGS::LOG_FLAGS() {

    memset(this, 0, sizeof(LOG_FLAGS));

    // on by default
    // (others are off by default)
    //
    task = true;
    file_xfer = true;
    sched_ops = true;
}

// Parse log flag preferences
//
int LOG_FLAGS::parse(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_ERROR,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/log_flags")) return 0;
        else if (xp.parse_bool(tag, "task", task)) continue;
        else if (xp.parse_bool(tag, "file_xfer", file_xfer)) continue;
        else if (xp.parse_bool(tag, "sched_ops", sched_ops)) continue;
        else if (xp.parse_bool(tag, "cpu_sched", cpu_sched)) continue;
        else if (xp.parse_bool(tag, "cpu_sched_debug", cpu_sched_debug)) continue;
        else if (xp.parse_bool(tag, "rr_simulation", rr_simulation)) continue;
        else if (xp.parse_bool(tag, "debt_debug", debt_debug)) continue;
        else if (xp.parse_bool(tag, "task_debug", task_debug)) continue;
        else if (xp.parse_bool(tag, "work_fetch_debug", work_fetch_debug)) continue;
        else if (xp.parse_bool(tag, "unparsed_xml", unparsed_xml)) continue;
        else if (xp.parse_bool(tag, "state_debug", state_debug)) continue;
        else if (xp.parse_bool(tag, "file_xfer_debug", file_xfer_debug)) continue;
        else if (xp.parse_bool(tag, "sched_op_debug", sched_op_debug)) continue;
        else if (xp.parse_bool(tag, "http_debug", http_debug)) continue;
        else if (xp.parse_bool(tag, "proxy_debug", proxy_debug)) continue;
        else if (xp.parse_bool(tag, "time_debug", time_debug)) continue;
        else if (xp.parse_bool(tag, "net_xfer_debug", net_xfer_debug)) continue;
        else if (xp.parse_bool(tag, "measurement_debug", measurement_debug)) continue;
        else if (xp.parse_bool(tag, "poll_debug", poll_debug)) continue;
        else if (xp.parse_bool(tag, "guirpc_debug", guirpc_debug)) continue;
        else if (xp.parse_bool(tag, "scrsave_debug", scrsave_debug)) continue;
        else if (xp.parse_bool(tag, "app_msg_debug", app_msg_debug)) continue;


        else {
            msg_printf(NULL, MSG_ERROR, "Unrecognized tag in %s: <%s>\n",
                CONFIG_FILE, tag
            );
        }
    }
    return ERR_XML_PARSE;
}

CONFIG::CONFIG() {
    memset(this, 0, sizeof(CONFIG));
    dont_check_file_sizes = false;
    save_stats_days = 30;
}

int CONFIG::parse_options(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_ERROR,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/options")) {
            return 0;
        }
        else if (xp.parse_int(tag, "save_stats_days", save_stats_days)) continue;
        else if (xp.parse_bool(tag, "dont_check_file_sizes", dont_check_file_sizes)) continue;
        else if (xp.parse_int(tag, "ncpus", ncpus)) continue;
        else {
            msg_printf(NULL, MSG_ERROR, "Unparsed tag in %s: <%s>\n",
                CONFIG_FILE, tag
            );
        }
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse(FILE* f) {
    char tag[256];
    MIOFILE mf;
    XML_PARSER xp(&mf);
    bool is_tag;

    mf.init_file(f);
    if (!xp.parse_start("cc_config")) return ERR_XML_PARSE;
    while (!xp.get(tag, is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_ERROR,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/cc_config")) return 0;
        if (!strcmp(tag, "log_flags")) {
            log_flags.parse(xp);
            continue;
        } else if (!strcmp(tag, "options")) {
            parse_options(xp);
        } else {
            msg_printf(NULL, MSG_ERROR, "Unparsed tag in %s: <%s>\n",
                CONFIG_FILE, tag
            );
        }
    }
    return ERR_XML_PARSE;
}

void read_config_file() {
    FILE* f;

    f = boinc_fopen(CONFIG_FILE, "r");
    if (!f) return;
    config.parse(f);
    fclose(f);
}

const char *BOINC_RCSID_5f23de6652 = "$Id$";
