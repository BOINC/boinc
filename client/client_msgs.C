// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
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

#ifdef _WIN32
#include "boinc_win.h"
#endif
#ifndef _WIN32
#include <vector>
#include <string>
#include <stdarg.h>
#endif

#include "log_flags.h"
#include "client_msgs.h"

#define MAX_SAVED_MESSAGES 1000

CLIENT_MSG_LOG log_messages;

const char* CLIENT_MSG_LOG::v_format_kind(int kind) const {
    switch(kind) {
    case DEBUG_STATE:       return "DEBUG_STATE      ";
    case DEBUG_TASK:        return "DEBUG_TASK       ";
    case DEBUG_FILE_XFER:   return "DEBUG_FILE_XFER  ";
    case DEBUG_SCHED_OP:    return "DEBUG_SCHED_OP   ";
    case DEBUG_HTTP:        return "DEBUG_HTTP       ";
    case DEBUG_PROXY:       return "DEBUG_PROXY      ";
    case DEBUG_TIME:        return "DEBUG_TIME       ";
    case DEBUG_NET_XFER:    return "DEBUG_NET_XFER   ";
    case DEBUG_MEASUREMENT: return "DEBUG_MEASUREMENT";
    case DEBUG_POLL:        return "DEBUG_POLL       ";
    case DEBUG_GUIRPC:      return "DEBUG_GUIRPC     ";
    default:                return "*** internal error: invalid MessageKind ***";
    }
}

bool CLIENT_MSG_LOG::v_message_wanted(int kind) const {
    switch (kind) {
    case DEBUG_STATE:       return log_flags.state_debug;
    case DEBUG_TASK:        return log_flags.task_debug;
    case DEBUG_FILE_XFER:   return log_flags.file_xfer_debug;
    case DEBUG_SCHED_OP:    return log_flags.sched_op_debug;
    case DEBUG_HTTP:        return log_flags.http_debug;
    case DEBUG_PROXY:       return log_flags.proxy_debug;
    case DEBUG_TIME:        return log_flags.time_debug;
    case DEBUG_NET_XFER:    return log_flags.net_xfer_debug;
    case DEBUG_MEASUREMENT: return log_flags.measurement_debug;
    case DEBUG_POLL:        return log_flags.poll_debug;
    case DEBUG_GUIRPC:      return log_flags.guirpc_debug;
    default: return false;
    }
}

list<MESSAGE_DESC*> message_descs;

// Takes a printf style formatted string, inserts the proper values,
// and passes it to show_message
// TODO: add translation functionality
//
void msg_printf(PROJECT *p, int priority, char *fmt, ...) {
    char        buf[512];
    va_list     ap;

    if (fmt == NULL) return;

    // Since Windows doesn't support vsnprintf, we have to do a
    // workaround to prevent buffer overruns
    //
    if (strlen(fmt) > 512) fmt[511] = '\0';
    va_start(ap, fmt); // Parses string for variables
    vsprintf(buf, fmt, ap); // And convert symbols To actual numbers
    va_end(ap); // Results are stored in text

    show_message(p, buf, priority);
}

// stash message in memory
//
void record_message(PROJECT* p, int priority, int now, char* message) {
    MESSAGE_DESC* mdp = new MESSAGE_DESC;
    static int seqno = 1;
    mdp->project = p;
    mdp->priority = priority;
    mdp->timestamp = now;
    mdp->seqno = seqno++;
    mdp->message = message;
    while (message_descs.size() > MAX_SAVED_MESSAGES) {
        delete message_descs.back();
        message_descs.pop_back();
    }
    message_descs.push_front(mdp);
}