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
int LOG_FLAGS::parse(FILE* f) {
    char tag[256], contents[1024];

    while (get_tag(f, tag, contents)) {
        if (!strcmp(tag, "/log_flags")) return 0;
        else if (!strcmp(tag, "task")) task = get_bool(contents);
        else if (!strcmp(tag, "file_xfer")) file_xfer = get_bool(contents);
        else if (!strcmp(tag, "sched_ops")) sched_ops = get_bool(contents);

        else if (!strcmp(tag, "cpu_sched")) cpu_sched = get_bool(contents);
        else if (!strcmp(tag, "cpu_sched_debug")) cpu_sched_debug = get_bool(contents);
        else if (!strcmp(tag, "rr_simulation")) rr_simulation = get_bool(contents);
        else if (!strcmp(tag, "debt_debug")) debt_debug = get_bool(contents);
        else if (!strcmp(tag, "task_debug")) task_debug = get_bool(contents);
        else if (!strcmp(tag, "work_fetch_debug")) work_fetch_debug = get_bool(contents);
        else if (!strcmp(tag, "unparsed_xml")) unparsed_xml = get_bool(contents);
        else if (!strcmp(tag, "state_debug")) state_debug = get_bool(contents);
        else if (!strcmp(tag, "file_xfer_debug")) file_xfer_debug = get_bool(contents);
        else if (!strcmp(tag, "sched_op_debug")) sched_op_debug = get_bool(contents);
        else if (!strcmp(tag, "http_debug")) http_debug = get_bool(contents);
        else if (!strcmp(tag, "proxy_debug")) proxy_debug = get_bool(contents);
        else if (!strcmp(tag, "time_debug")) time_debug = get_bool(contents);
        else if (!strcmp(tag, "net_xfer_debug")) net_xfer_debug = get_bool(contents);
        else if (!strcmp(tag, "measurement_debug")) measurement_debug = get_bool(contents);
        else if (!strcmp(tag, "poll_debug")) poll_debug = get_bool(contents);
        else if (!strcmp(tag, "guirpc_debug")) guirpc_debug = get_bool(contents);
        else if (!strcmp(tag, "scrsave_debug")) scrsave_debug = get_bool(contents);
        else {
            msg_printf(NULL, MSG_ERROR, "Unrecognized tag in %s: %s\n",
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

int CONFIG::parse_options(FILE* f) {
    char tag[256], contents[1024];
    while (get_tag(f, tag, contents)) {
        if (!strcmp(tag, "/options")) {
            return 0;
        } else if (!strcmp(tag, "save_stats_days")) {
            save_stats_days = get_int(contents);
        } else if (!strcmp(tag, "dont_check_file_sizes")) {
            dont_check_file_sizes = get_bool(contents);
        } else if (!strcmp(tag, "ncpus")) {
            ncpus = get_int(contents);
        } else {
            msg_printf(NULL, MSG_ERROR, "Unparsed tag in %s: %s\n",
                CONFIG_FILE, tag
            );
        }
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse(FILE* f) {
    char tag[256];

    get_tag(f, tag);
    if (strstr(tag, "?xml")) get_tag(f, tag);
    if (strcmp(tag, "cc_config")) return ERR_XML_PARSE;

    while (get_tag(f, tag)) {
        if (!strcmp(tag, "/cc_config")) return 0;
        if (!strcmp(tag, "log_flags")) {
            log_flags.parse(f);
            continue;
        } else if (!strcmp(tag, "options")) {
            parse_options(f);
        } else {
            msg_printf(NULL, MSG_ERROR, "Unparsed tag in %s: %s\n",
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
