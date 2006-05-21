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

#include "log_flags.h"
#include "filesys.h"

LOG_FLAGS log_flags;
CONFIG config;

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
    proxy_debug = false;
    time_debug = false;
    net_xfer_debug = false;
    measurement_debug = false;
    guirpc_debug = false;
    sched_cpu_debug = false;
    scrsave_debug = false;
    dont_check_file_sizes = false;
}

// Parse log flag preferences
//
int LOG_FLAGS::parse(FILE* in) {
    char buf[256];

    while (fgets(buf, 256, in)) {
        if (strlen(buf) < 2) continue;
        if (match_tag(buf, "</log_flags>")) return 0;
        else if (parse_bool(buf, "task", task)) continue;
        else if (parse_bool(buf, "file_xfer", file_xfer)) continue;
        else if (parse_bool(buf, "sched_ops", sched_ops)) continue;
        else if (parse_bool(buf, "state_debug", state_debug)) continue;
        else if (parse_bool(buf, "task_debug", task_debug)) continue;
        else if (parse_bool(buf, "file_xfer_debug", file_xfer_debug)) continue;
        else if (parse_bool(buf, "sched_op_debug", sched_op_debug)) continue;
        else if (parse_bool(buf, "http_debug", http_debug)) continue;
        else if (parse_bool(buf, "proxy_debug", proxy_debug)) continue;
        else if (parse_bool(buf, "time_debug", time_debug)) continue;
        else if (parse_bool(buf, "net_xfer_debug", net_xfer_debug)) continue;
        else if (parse_bool(buf, "measurement_debug", measurement_debug)) continue;
        else if (parse_bool(buf, "poll_debug", poll_debug)) continue;
        else if (parse_bool(buf, "guirpc_debug", guirpc_debug)) continue;
        else if (parse_bool(buf, "sched_cpu_debug", sched_cpu_debug)) continue;
        else if (parse_bool(buf, "scrsave_debug", scrsave_debug)) continue;
        else if (parse_bool(buf, "dont_check_file_sizes", dont_check_file_sizes)) continue;
        msg_printf(NULL, MSG_ERROR, "Unparsed line in %s: %s\n",
            CONFIG_FILE, buf
        );
    }
    return ERR_XML_PARSE;
}

CONFIG::CONFIG() {
    save_stats_days = 30;
}

int CONFIG::parse(FILE* in) {
    char buf[256];

    while (fgets(buf, 256, in)) {
        if (strlen(buf) < 2) continue;
        if (match_tag(buf, "</cc_config>")) return 0;
        if (match_tag(buf, "<log_flags>")) {
            log_flags.parse(in);
            continue;
        }
        if (parse_int(buf, "<save_stats_days>", save_stats_days)) {
            continue;
        }
        msg_printf(NULL, MSG_ERROR, "Unparsed line in %s: %s\n",
            CONFIG_FILE, buf
        );
    }
    return ERR_XML_PARSE;
}

void read_config_file() {
    FILE* f;

    if (boinc_file_exists(CONFIG_FILE)) {
        f = boinc_fopen(CONFIG_FILE, "r");
        config.parse(f);
        fclose(f);
    }

}

const char *BOINC_RCSID_5f23de6652 = "$Id$";
