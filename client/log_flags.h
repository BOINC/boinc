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

struct LOG_FLAGS {
    // on by default, user-readable
    //
    bool task;              // task start and finish
    bool file_xfer;         // file transfer start and finish
    bool sched_ops;         // interactions with schedulers

    // off by default; intended for developers and testers
    //
    bool cpu_sched;         // preemption and resumption
    bool cpu_sched_debug;   // explain scheduler decisions
    bool rr_simulation;     // results of rr simulator
    bool debt_debug;        // changes to debt
    bool task_debug;        // task start and control details
                            // also prints when apps checkpoint
    bool work_fetch_debug;  // work fetch policy 

    bool unparsed_xml;      // show unparsed XML lines
    bool state_debug;       // print textual summary of CLIENT_STATE initially
                            // and after each scheduler RPC and garbage collect
                            // also show actions of garbage collector
                            // also show when state file is read or written
    bool file_xfer_debug;   // show completion of FILE_XFER
    bool sched_op_debug;
    bool http_debug;
    bool proxy_debug;
    bool time_debug;        // print message on sleep
    bool net_xfer_debug;
    bool measurement_debug; // host measurement notices
    bool poll_debug;        // show what polls are responding
    bool guirpc_debug;
    bool scrsave_debug;

    LOG_FLAGS();
    int parse(FILE*);
};

struct CONFIG {
    bool dont_check_file_sizes;
    int save_stats_days;
    int ncpus;
    CONFIG();
    int parse(FILE*);
    int parse_options(FILE*);
};

extern LOG_FLAGS log_flags;
extern CONFIG config;
extern void read_config_file();

#endif

