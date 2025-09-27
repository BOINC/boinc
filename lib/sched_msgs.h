// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// API for logging in server components

#ifndef BOINC_SCHED_MSGS_H
#define BOINC_SCHED_MSGS_H

#include "msg_log.h"
#include "boinc_stdio.h"

// Message priority levels (also called 'kind').
// Each message has a level.
// When you set a logging level (see below),
// all messages of that priority and lower are shown
//
enum {
    MSG_CRITICAL=1,
        // something is broken and needs to be fixed.
        // Typically the program does exit(1) after this
    MSG_WARNING=2,
        // something unusual but not fatal, like unrecognized XML
    MSG_NORMAL=3,
        // default; say tersely what the program did
    MSG_DEBUG=4
        // debugging output for some part of the program.
        // You might use flags to enable specific parts
};

class SCHED_MSG_LOG : public MSG_LOG {
    const char* v_format_kind(int kind) const;
    bool v_message_wanted(int kind) const;
public:
    SCHED_MSG_LOG(): MSG_LOG(stderr) {
        debug_level = MSG_NORMAL;
    }
    void set_file(FILE* f) {
        output=f;
    }
    void set_debug_level(int new_level) {
        debug_level = new_level;
    }

#ifdef _USING_FCGI_
    ~SCHED_MSG_LOG();
    void redirect(FCGI_FILE* f);
    void close();
    void flush();
#endif
};

#define _(x) "_(\"" x "\")"

extern SCHED_MSG_LOG log_messages;
#endif
