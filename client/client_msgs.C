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

#ifdef _WIN32
#include "boinc_win.h"
#endif
#ifndef _WIN32
#include <cstdarg>
#include <deque>
#endif

using std::deque;

#include "log_flags.h"
#include "client_types.h"
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
    case DEBUG_SCHED_CPU:   return "DEBUG_SCHED_CPU  ";
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
    case DEBUG_SCHED_CPU:   return log_flags.sched_cpu_debug;
    default: return false;
    }
}

// a dequeue of up to MAX_SAVED_MESSAGES most recent messages,
// stored in newest-first order
//
deque<MESSAGE_DESC*> message_descs;

// Takes a printf style formatted string, inserts the proper values,
// and passes it to show_message
// TODO: add translation functionality
//
void msg_printf(PROJECT *p, int priority, const char *fmt, ...) {
    char        buf[8192];  // output can be much longer than format
    va_list     ap;

    if (fmt == NULL) return;

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
    strcpy(mdp->project_name, "");
    if (p) {
        strcpy(mdp->project_name, p->get_project_name());
    }
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

const char *BOINC_RCSID_9572274f4f = "$Id$";
