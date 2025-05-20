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

#ifndef BOINC_CLIENT_MSGS_H
#define BOINC_CLIENT_MSGS_H

#include <algorithm>
#include <deque>
#include <string>
#include <string.h>

#include "client_types.h"
#include "common_defs.h"
#include "log_flags.h"

// stores a message in memory, where it can be retrieved via RPC

struct MESSAGE_DESC {
    char project_name[256];
    int priority;
    int timestamp;
    int seqno;
    std::string message;
};

#define MAX_SAVED_MESSAGES 2000

// a cache of MAX_SAVED_MESSAGES most recent messages,
// stored in newest-first order
//
struct MESSAGE_DESCS {
    std::deque<MESSAGE_DESC*> msgs;
    void insert(PROJ_AM *p, int priority, int now, char* msg);
    void write(int seqno, MIOFILE&, bool translatable);
    int highest_seqno();
    void cleanup();
};

extern MESSAGE_DESCS message_descs;

// the __attribute((format...)) tags are GCC extensions that let the compiler
// do like-checking on printf-like arguments
//
#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x) /*nothing*/
#endif

// Show a message, preceded by timestamp and project name

extern void msg_printf(PROJ_AM *p, int priority, const char *fmt, ...)
    __attribute__ ((format (printf, 3, 4)))
;

// Show a MSG_USER_ALERT message (i.e. will be shown as a notice)
// Additional info:
// is_html: true if message body contains HTML tags
// link: URL for "more..." link
//
extern void msg_printf_notice(PROJ_AM *p, bool is_html, const char* link, const char *fmt, ...)
    __attribute__ ((format (printf, 4, 5)))
;

#define _(x) "_(\"" x "\")"

extern std::string app_list_string(PROJECT*);

#endif
