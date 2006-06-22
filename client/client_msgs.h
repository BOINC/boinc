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

// write messages ONLY as follows:
// if (log_flags.X) {
//     msg_printf();
// }

#include <deque>
#include <string>

#include "log_flags.h"

class PROJECT;

// Show a message, preceded by timestamp and project name
// priorities (this MUST match those in lib/gui_rpc_client.h)

#define MSG_INFO    1
    // write to stdout
    // GUI: write to msg window
#define MSG_ERROR   2
    // write to stdout
    // GUI: write to msg window in bold or red

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
extern void msg_printf(PROJECT *p, int priority, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

#endif
