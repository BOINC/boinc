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
    bool time_debug;        // print message on sleep
    bool net_xfer_debug;
    bool measurement_debug; // host measurement notices
    bool poll_debug;        // show what polls are responding
    bool dont_check_file_sizes;

    LOG_FLAGS();
    int parse(FILE*);
};

extern LOG_FLAGS log_flags;
extern void read_log_flags();

#endif

