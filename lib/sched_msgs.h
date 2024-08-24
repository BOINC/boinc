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

#ifndef BOINC_SCHED_MSGS_H
#define BOINC_SCHED_MSGS_H

#include "msg_log.h"
#include "boinc_stdio.h"

enum { MSG_CRITICAL=1, MSG_WARNING, MSG_NORMAL, MSG_DEBUG, MSG_DETAIL };

class SCHED_MSG_LOG : public MSG_LOG {
    const char* v_format_kind(int kind) const;
    bool v_message_wanted(int kind) const;
public:
    SCHED_MSG_LOG(): MSG_LOG(stderr) { debug_level = MSG_NORMAL; }
    void set_file(FILE* f) {output=f;}
    void set_debug_level(int new_level) { debug_level = new_level; }
    bool debug() {return debug_level >= MSG_DEBUG;}
    bool detail() {return debug_level >= MSG_DETAIL;}

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
