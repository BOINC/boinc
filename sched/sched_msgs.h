// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#ifndef _BOINC_SCHED_MSGS_H_
#define _BOINC_SCHED_MSGS_H_

#include "msg_log.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

enum { MSG_CRITICAL=1, MSG_NORMAL, MSG_DEBUG };

class SCHED_MSG_LOG : public MSG_LOG {
    const char* v_format_kind(int kind) const;
    bool v_message_wanted(int kind) const;
public:
    int debug_level;
    enum { MSG_CRITICAL=1, MSG_NORMAL, MSG_DEBUG };
    SCHED_MSG_LOG(): MSG_LOG(stderr) { debug_level = MSG_NORMAL; }
    void set_debug_level(int new_level) { debug_level = new_level; }
    void set_indent_level(const int new_indent_level);
#ifdef _USING_FCGI_
    ~SCHED_MSG_LOG();
    void redirect(FCGI_FILE* f);
    void close();
    void flush();
#endif
};

#define _(x) "_(\""x"\")"

extern SCHED_MSG_LOG log_messages;
#endif
