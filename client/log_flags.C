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
#include "common_defs.h"
#include "file_names.h"
#include "client_msgs.h"
#include "file_xfer.h"
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

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_USER_ERROR,
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
        else if (xp.parse_bool(tag, "http_xfer_debug", http_xfer_debug)) continue;
        else if (xp.parse_bool(tag, "benchmark_debug", benchmark_debug)) continue;
        else if (xp.parse_bool(tag, "poll_debug", poll_debug)) continue;
        else if (xp.parse_bool(tag, "guirpc_debug", guirpc_debug)) continue;
        else if (xp.parse_bool(tag, "scrsave_debug", scrsave_debug)) continue;
        else if (xp.parse_bool(tag, "app_msg_send", app_msg_send)) continue;
        else if (xp.parse_bool(tag, "app_msg_receive", app_msg_receive)) continue;
        else if (xp.parse_bool(tag, "mem_usage_debug", mem_usage_debug)) continue;
        else if (xp.parse_bool(tag, "network_status_debug", network_status_debug)) continue;
        else {
            msg_printf(NULL, MSG_USER_ERROR, "Unrecognized tag in %s: <%s>\n",
                CONFIG_FILE, tag
            );
        }
    }
    return ERR_XML_PARSE;
}

static void show_flag(char* buf, bool flag, const char* flag_name) {
    if (!flag) return;
    int n = (int)strlen(buf);
    if (!n) {
        strcpy(buf, flag_name);
        return;
    }
    strcat(buf, ", ");
    strcat(buf, flag_name);
    if (strlen(buf) > 60) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf);
        strcpy(buf, "");
    }
}

void LOG_FLAGS::show() {
    char buf[256];
    strcpy(buf, "");
    show_flag(buf, task, "task");
    show_flag(buf, file_xfer, "file_xfer");
    show_flag(buf, sched_ops, "sched_ops");
    show_flag(buf, cpu_sched, "cpu_sched");
    show_flag(buf, cpu_sched_debug, "cpu_sched_debug");
    show_flag(buf, rr_simulation, "rr_simulation");
    show_flag(buf, debt_debug, "debt_debug");
    show_flag(buf, task_debug, "task_debug");
    show_flag(buf, work_fetch_debug, "work_fetch_debug");
    show_flag(buf, unparsed_xml, "unparsed_xml");
    show_flag(buf, state_debug, "state_debug");
    show_flag(buf, file_xfer_debug, "file_xfer_debug");
    show_flag(buf, sched_op_debug, "sched_op_debug");
    show_flag(buf, http_debug, "http_debug");
    show_flag(buf, proxy_debug, "proxy_debug");
    show_flag(buf, time_debug, "time_debug");
    show_flag(buf, http_xfer_debug, "http_xfer_debug");
    show_flag(buf, benchmark_debug, "benchmark_debug");
    show_flag(buf, poll_debug, "poll_debug");
    show_flag(buf, guirpc_debug, "guirpc_debug");
    show_flag(buf, scrsave_debug, "scrsave_debug");
    show_flag(buf, app_msg_send, "app_msg_send");
    show_flag(buf, app_msg_receive, "app_msg_receive");
    show_flag(buf, mem_usage_debug, "mem_usage_debug");
    show_flag(buf, network_status_debug, "network_status_debug");
    if (strlen(buf)) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf);
    }
}

CONFIG::CONFIG() {
    memset(this, 0, sizeof(CONFIG));
    dont_check_file_sizes = false;
	http_1_0 = false;
    save_stats_days = 30;
    max_file_xfers = MAX_FILE_XFERS;
    max_file_xfers_per_project = MAX_FILE_XFERS_PER_PROJECT;
    work_request_factor = 1;
}

int CONFIG::parse_options(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_USER_ERROR,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/options")) {
            return 0;
        }
        else if (xp.parse_int(tag, "save_stats_days", save_stats_days)) continue;
        else if (xp.parse_bool(tag, "dont_check_file_sizes", dont_check_file_sizes)) continue;
        else if (xp.parse_bool(tag, "http_1_0", http_1_0)) continue;
        else if (xp.parse_int(tag, "ncpus", ncpus)) continue;
        else if (xp.parse_int(tag, "max_file_xfers", max_file_xfers)) continue;
        else if (xp.parse_int(tag, "max_file_xfers_per_project", max_file_xfers_per_project)) continue;
        else if (xp.parse_double(tag, "work_request_factor", work_request_factor)) continue;
        else {
            msg_printf(NULL, MSG_USER_ERROR, "Unparsed tag in %s: <%s>\n",
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
    if (!xp.parse_start("cc_config")) {
        msg_printf(NULL, MSG_USER_ERROR, "Missing start tag in %s", CONFIG_FILE);
        return ERR_XML_PARSE;
    }
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_USER_ERROR,
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
            msg_printf(NULL, MSG_USER_ERROR, "Unparsed tag in %s: <%s>\n",
                CONFIG_FILE, tag
            );
        }
    }
    msg_printf(NULL, MSG_USER_ERROR, "Missing end tag in %s", CONFIG_FILE);
    return ERR_XML_PARSE;
}

int read_config_file() {
    FILE* f;

    f = boinc_fopen(CONFIG_FILE, "r");
    if (!f) return ERR_FOPEN;
    config.parse(f);
    fclose(f);
    return 0;
}

const char *BOINC_RCSID_5f23de6652 = "$Id$";
