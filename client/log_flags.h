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

// flags determining what is written to standard out.
// (errors go to stderr)
//
// NOTE: all writes to stdout should have an if (log.*) {} around them.
//

#ifndef _LOGFLAGS_H_
#define _LOGFLAGS_H_

#ifndef _WIN32
#include <stdio.h>
#endif

class LOG_FLAGS {
public:
    // the following write user-readable summaries
    //
    bool task;              // task executions
    bool file_xfer;         // file transfers
    bool sched_ops;         // interactions with schedulers

    // the following generate debugging info
    //
    bool state_debug;       // changes to CLIENT_STATE structure
    bool task_debug;
    bool file_xfer_debug;
    bool sched_op_debug;
    bool http_debug;
    bool proxy_debug;
    bool time_debug;        // print message on sleep
    bool net_xfer_debug;
    bool measurement_debug; // host measurement notices
    bool poll_debug;        // show what polls are responding
    bool guirpc_debug;
    bool dont_check_file_sizes;

    LOG_FLAGS();
    int parse(FILE*);
};

extern LOG_FLAGS log_flags;
extern void read_log_flags();

#endif

