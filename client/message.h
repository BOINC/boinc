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
// Portions created by the SETI@home project are Copyright (C) 2002, 2003
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef MESSAGE_H
#define MESSAGE_H

#include "util.h"

// Show a message, preceded by timestamp and project name
// priorities:

#define MSG_INFO    1
    // cmdline: write to stdout
    // GUI: write to msg window
#define MSG_ERROR   2
    // cmdline: write to stderr
    // GUI: write to msg window in bold or red

extern void show_message(class PROJECT *p, char* message, int priority);


class ClientMessages : public Messages {
    int debug_level;
    const char* v_format_kind(int kind) const;
    bool v_message_wanted(int kind) const;
public:
    enum Kind {
        DEBUG_STATE,       // changes to CLIENT_STATE structure
        DEBUG_TASK,
        DEBUG_FILE_XFER,
        DEBUG_SCHED_OP,
        DEBUG_HTTP,
        DEBUG_TIME,        // print message on sleep
        DEBUG_NET_XFER,
        DEBUG_MEASUREMENT, // host measurement notices
        DEBUG_POLL,        // show what polls are responding
    };
    ClientMessages(): Messages(stdout) {}
};

extern ClientMessages log_messages;

#endif
