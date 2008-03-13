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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _BOINC_SCHED_MSGS_H_
#define _BOINC_SCHED_MSGS_H_

#include "msg_log.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
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
    void redirect(FILE* f);
    void close();
    void flush();
#endif
};

extern SCHED_MSG_LOG log_messages;
#endif
