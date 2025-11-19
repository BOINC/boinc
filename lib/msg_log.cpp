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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <cstring>
#include <string>
#endif

#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "msg_log.h"

using std::string;

// MSG_LOG is a base class for writing messages not intended for the end user.
// This includes all server messages and client debugging messages.
// SCHED_MSG_LOG (in sched/sched_msg_log.C) decides which scheduler messages
// to print and formats the "kind" keyword;
// CLIENT_MSG_LOG does the same thing for client debugging output.
//
// MSG_LOG has an "indent_level" state for how many spaces to indent output.
// This corresponds in general to the function-call recursion level.
// Call MSG_LOG::enter_level() to increase or decrease by 1 level.

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
    debug_level = 0;
    output = output_;
    indent_level = 0;
    pid = 0;
    spaces[0] = 0;
    for (int i=1; i<80; i++) {
        spaces[i] = 0;
    }
}

void MSG_LOG::set_indent_level(const int new_indent_level) {
    if (new_indent_level < 0) indent_level = 0;
    else if (new_indent_level > 39) indent_level = 39;
    else indent_level = new_indent_level;

    memset(spaces, ' ', sizeof(spaces));
    spaces[indent_level] = 0;
}

void MSG_LOG::vprintf(int kind, const char* format, va_list va) {
    char buf[256];
    const char* now_timestamp = precision_time_to_string(dtime());
    if (!v_message_wanted(kind)) return;
    if (pid) {
        snprintf(buf, sizeof(buf), " [PID=%-5d]", pid);
    } else {
        buf[0] = 0;
    }
    boinc::fprintf(output, "%s%s %s%s ", now_timestamp, buf, v_format_kind(kind), spaces);
    boinc::vfprintf(output, format, va);
}

// break a multi-line string into lines (so that we show prefix on each line)
//
void MSG_LOG::vprintf_multiline(
    int kind, const char* str, const char* prefix_format, va_list va
) {
    if (!v_message_wanted(kind)) return;
    if (str == NULL) return;

    char sprefix[256] = "";
    if (prefix_format) {
        vsnprintf(sprefix, sizeof(sprefix),prefix_format, va);
    }
    const char* now_timestamp = precision_time_to_string(dtime());
    const char* skind = v_format_kind(kind);

    string line;
    while (*str) {
        if (*str == '\n') {
            boinc::fprintf(output, "%s %s%s %s%s\n", now_timestamp, skind, spaces, sprefix, line.c_str());
            line.erase();
        } else {
            line += *str;
        }
        ++str;
    }
    if (!line.empty()) {
        boinc::fprintf(output, "%s %s[%s] %s%s\n", now_timestamp, spaces, skind, sprefix, line.c_str());
    }
}

void MSG_LOG::vprintf_file(
    int kind, const char* filename, const char* prefix_format, va_list va
) {
    if (!v_message_wanted(kind)) return;

    char sprefix[256] = "";
    if (prefix_format) {
        vsnprintf(sprefix, sizeof(sprefix), prefix_format, va);
    }
    const char* now_timestamp = precision_time_to_string(dtime());
    const char* skind = v_format_kind(kind);

    FILE* f = boinc::fopen(filename, "r");
    if (!f) return;
    char buf[256];

    while (boinc::fgets(buf, 256, f)) {
        boinc::fprintf(output, "%s %s%s %s%s\n", now_timestamp, skind, spaces, sprefix, buf);
    }
    boinc::fclose(f);
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
