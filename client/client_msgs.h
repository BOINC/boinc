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
//
// Both types of messages are controlled by the flags in log_flags.xml

#include <deque>
#include <string>

#include "msg_log.h"

class PROJECT;

// Show a message, preceded by timestamp and project name
// priorities (this MUST match those in lib/gui_rpc_client.h)

#define MSG_INFO    1
    // write to stdout
    // GUI: write to msg window
#define MSG_ERROR   2
    // write to stderr
    // GUI: write to msg window in bold or red
//#define MSG_WARNING 3
    // deprecated - do not use
#define MSG_ALERT_INFO   4
    // write to stdout
    // GUI: put in a modal dialog
#define MSG_ALERT_ERROR     5
    // write to stderr
    // GUI: put in a modal error dialog

// the following stores a message in memory, where it can be retrieved via RPC
//
struct MESSAGE_DESC {
    char project_name[256];
    int priority;
    int timestamp;
    int seqno;
    std::string message;
};

extern std::deque<MESSAGE_DESC*> message_descs;
extern void record_message(class PROJECT *p, int priority, int now, char* msg);

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
        DEBUG_GUIRPC,
        DEBUG_CPU_SCHED,
        DEBUG_WORK_FETCH,
        DEBUG_SCRSAVE
    };
    CLIENT_MSG_LOG(): MSG_LOG(stdout) {}
    ~CLIENT_MSG_LOG(){}
};

extern CLIENT_MSG_LOG log_messages;

extern void msg_printf(PROJECT *p, int priority, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

#endif
