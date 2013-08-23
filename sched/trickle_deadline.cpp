// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// The following is a deadline trickle handler.
// It extends the deadline of the task_id the app sends us.

// A trickle handler that grants credit based on run time
//
// message format:
//
// <cpu_time>x</cpu_time>
// <task_name>x</task_name>
//
// Required cmdline arg:
//
// --extension_period X     add X seconds to the current deadline
// --extension_timeframe X  extend only if deadline is within the next X secs
//

#include "util.h"
#include "sched_msgs.h"
#include "trickle_handler.h"

double extension_period = 0;
double extension_timeframe = 0;

int handle_trickle_init(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--extension_period")) {
            extension_period = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--extension_timeframe")) {
            extension_timeframe = atof(argv[++i]);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown arg %s\n", argv[i]);
            return ERR_XML_PARSE;
        }
    }
    if (!extension_period) {
        log_messages.printf(MSG_CRITICAL, "missing --extension_period arg\n");
        return ERR_NULL;
    }
    if (!extension_timeframe) {
        log_messages.printf(MSG_CRITICAL, "missing --extension_timeframe arg\n");
        return ERR_NULL;
    }

    log_messages.printf(MSG_NORMAL,
        "extension_period: %f sec\n", extension_period
    );
    log_messages.printf(MSG_NORMAL,
        "extension_timeframe: %f sec\n", extension_timeframe
    );

    return 0;
}

int handle_trickle(MSG_FROM_HOST& mfh) {
    char task_name[256];
    int cpu_time = 0; // not needed but parsed to limit unexpected tag warnings

    printf("got trickle-up \n%s\n\n", mfh.xml);

    MIOFILE mf;

    mf.init_buf_read(mfh.xml);
    XML_PARSER xp(&mf);

    while (!xp.get_tag()) {
        if (xp.parse_int("cpu_time", cpu_time)) break;
        if (xp.parse_str("task_name", task_name, 256)) break;
        log_messages.printf(MSG_NORMAL, "unexpected tag: %s\n", xp.parsed_tag);
    }
    if (strlen(task_name) == 0) {
        log_messages.printf(MSG_NORMAL,
            "unexpected empty task_name attribute\n"
        );
        return ERR_XML_PARSE;
    }

    DB_RESULT task;
    int retval = task.lookup(task_name);
    if (retval) return retval;

    // sanity checks - customize as needed
    //
    if (task.report_deadline < dtime()) {
        log_messages.printf(MSG_NORMAL,
            "Report deadline is in the past: %d\n", task.report_deadline
        );
        // don't do anything for now
        return 0;
    }
    if ((task.report_deadline - extension_timeframe) > dtime()) {
        log_messages.printf(MSG_DEBUG,
            "Report deadline is too far in the future: %d\n",
            task.report_deadline
        );
        // don't do anything
        return 0;
    }
    // extend the deadline
    //
    task.report_deadline += extension_period;
    retval = task.update();
    if (retval) return retval;
    log_messages.printf(MSG_DEBUG,
        "Report deadline of result %d extended to %d\n",
        task.id, task.report_deadline
    );
    return 0;
}
