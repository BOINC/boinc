static volatile const char *BOINCrcsid="$Id$";
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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cassert>
#include <cstring>
#include <string>
#endif

#include "util.h"
#include "msg_log.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

using std::string;

// MSG_LOG is a base class for writing messages not intended for the end user.
// This includes all server messages and client debugging messages.
// SCHED_MSG_LOG (in sched/sched_msg_log.C) decides which scheduler messages
// to print and formats the "kind" keyword;
// CLIENT_MSG_LOG does the same thing for client debugging output.
//
// MSG_LOG has an "indent_level" state for how many spaces to indent output.
// This corresponds in general to the function-call recursion level.
// Call MSG_LOG::enter_level() to increase by 1 level
// and leave_level() to decrease by 1 level.
// The SCOPE_MSG_LOG class takes care of calling leave_level() for you.
// Create a SCOPE_MSG_LOG object on the stack at the beginning of a function;
// it will increment the level by 1 on construction,
// and decrement the level by 1 on destruction at end of scope.
// This way you don't have to worry about decrementing
// before mid-function returns, exceptions, etc.

// Each [v]printf* function prints the timestamp, the formatted KIND string,
// indentation level, then the specified string.
// The string to print can be a one-line string (including the trailing \n),
// a multi-line string (it's broken up into lines
// to get the prefix on each line), or a file (also broken up into lines).

// Scheduler functions should use "sched_messages" which is an instance of
// SCHED_MSG_LOG.  Client functions should use "client_messages",
// which is an instance of CLIENT_MSG_LOG.

// See sched/sched_msg_log.C and client/client_msg_log.C for those classes.

MSG_LOG::MSG_LOG(FILE* output_) {
    output = output_;
    indent_level = 0;
    spaces[0] = 0;
    strcpy(spaces+1, "                                                                              ");
}

void MSG_LOG::enter_level(int diff) {
    assert (indent_level >= 0);
    spaces[indent_level] = ' ';
    indent_level += diff*2;
    spaces[indent_level] = 0;
    assert (indent_level >= 0);
}

void MSG_LOG::vprintf(int kind, const char* format, va_list va) {
    const char* now_timestamp = time_to_string(time(0));
    if (!v_message_wanted(kind)) return;
    fprintf(output, "%s [%s]%s ", now_timestamp, v_format_kind(kind), spaces);
    vfprintf(output, format, va);
}

// break a multi-line string into lines (so that we show prefix on each line)
void MSG_LOG::vprintf_multiline(
    int kind, const char* str, const char* prefix_format, va_list va
) {
    if (!v_message_wanted(kind)) return;
    if (str == NULL) return;

    char sprefix[256] = "";
    if (prefix_format) {
        vsprintf(sprefix, prefix_format, va);
    }
    const char* now_timestamp = time_to_string(time(0));
    const char* skind = v_format_kind(kind);

    string line;
    while (*str) {
        if (*str == '\n') {
            fprintf(output, "%s [%s]%s %s%s\n", now_timestamp, skind, spaces, sprefix, line.c_str());
            line.erase();
        } else {
            line += *str;
        }
        ++str;
    }
    if (!line.empty()) {
        fprintf(output, "%s %s[%s] %s%s\n", now_timestamp, spaces, skind, sprefix, line.c_str());
    }
}

void MSG_LOG::vprintf_file(
    int kind, const char* filename, const char* prefix_format, va_list va
) {
    if (!v_message_wanted(kind)) return;

    char sprefix[256] = "";
    if (prefix_format) {
        vsprintf(sprefix, prefix_format, va);
    }
    const char* now_timestamp = time_to_string(time(0));
    const char* skind = v_format_kind(kind);

    FILE* f = fopen(filename, "r");
    if (!f) return;
    char buf[256];

    while (fgets(buf, 256, f)) {
        fprintf(output, "%s [%s]%s %s%s\n", now_timestamp, skind, spaces, sprefix, buf);
    }
    fclose(f);
}

void MSG_LOG::printf(int kind, const char* format, ...) {
    va_list va;
    va_start(va, format);
    vprintf(kind, format, va);
    va_end(va);
}

void MSG_LOG::printf_multiline(
    int kind, const char* str, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    vprintf_multiline(kind, str, prefix_format, va);
    va_end(va);
}

void MSG_LOG::printf_file(
    int kind, const char* filename, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    vprintf_file(kind, filename, prefix_format, va);
    va_end(va);
}

// These SCOPE_MSG_LOG functions are utility functions that call their
// corresponding MSG_LOG functions with the same name, passing the KIND that
// was specified on creation of the SCOPE_MSG_LOG object.

void SCOPE_MSG_LOG::printf(const char* format, ...) {
    va_list va;
    va_start(va, format);
    messages.vprintf(kind, format, va);
    va_end(va);
}

void SCOPE_MSG_LOG::printf_multiline(
    const char* str, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    messages.vprintf_multiline(kind, str, prefix_format, va);
    va_end(va);
}

void SCOPE_MSG_LOG::printf_file(
    const char* filename, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    messages.vprintf_file(kind, filename, prefix_format, va);
    va_end(va);
}
