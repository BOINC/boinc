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

#ifndef CLIENT_MSG_LOG_H
#define CLIENT_MSG_LOG_H

// Two types of messages are used in the BOINC client:
//
// - Log messages
//   Debugging messages, not intended to be seen by users
//   Write these using the log_messages object.
//
// - User messages
//   Message intended for users, displayed in the Messages tab of the GUI
//   Write these using the msg_printf() function

#include <vector>

#include "msg_log.h"
#include "client_types.h"

// Show a message, preceded by timestamp and project name
// priorities:

#define MSG_INFO    1
    // write to stdout
    // GUI: write to msg window
#define MSG_ERROR   2
    // write to stderr
    // GUI: write to msg window in bold or red
#define MSG_WARNING 3
    // deprecated - do not use

// the following stores a message in memory, where it can be retrieved via RPC
//
struct MESSAGE_DESC {
    PROJECT* project;
    int priority;
    int timestamp;
    string message;
};

extern vector<MESSAGE_DESC> message_descs;

extern void show_message(class PROJECT *p, char* message, int priority);

class CLIENT_MSG_LOG : public MSG_LOG {
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
        DEBUG_PROXY,
        DEBUG_TIME,        // print message on sleep
        DEBUG_NET_XFER,
        DEBUG_MEASUREMENT, // host measurement notices
        DEBUG_POLL,        // show what polls are responding
        DEBUG_GUIRPC
    };
    CLIENT_MSG_LOG(): MSG_LOG(stdout) {}
};

extern CLIENT_MSG_LOG log_messages;

extern void msg_printf(PROJECT *p, int priority, char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

#endif
