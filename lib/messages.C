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
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <cassert>
#include <cstring>
#include <string>
#include <fstream>
using namespace std;
#endif

#include "util.h"

//////////////////////////////////////////////////////////////////////
//
// Messages is a base class for writing messages not intended for the end
// user.  This includes all server messages and client debugging messages.
// SchedMessages (in sched/sched_messages.C) decides which scheduler messages
// to print and formats the "kind" keyword; ClientMessages does the same thing
// for client debugging output.
//
// Messages has an "indent_level" state for how many spaces to indent output.
// This corresponds in general to the function-call recursion level.  Call
// Messages::enter_level() to increase by 1 level and leave_level() to
// decrease by 1 level.  The ScopeMessages class takes care of calling
// leave_level() for you.  Create a ScopeMessages object on the stack at the
// beginning of a function; it will increment the level by 1 on construction,
// and decrement the level by 1 on destruction at end of scope.  This way you
// don't have to worry about decrementing before mid-function returns,
// exceptions, etc.

// Each [v]printf* function prints the timestamp, the formatted KIND string,
// indentation level, then the specified string.  The string to print can be
// a one-line string (including the trailing \n), a multi-line string (it's
// broken up into lines to get the prefix on each line), or a file (also
// broken up into lines).

// Scheduler functions should use "log_messages" which is an instance of
// SchedMessages.  Client functions should use "log_messages" (also) which is
// an instance of ClientMessages.

// See sched/sched_messages.C and client/client_messages.C for those classes.

Messages::Messages(FILE* output_) {
    output = output_;
    indent_level = 0;
    spaces[0] = 0;
    strcpy(spaces+1, "                                                                              ");
}

void Messages::enter_level(int diff) {
    assert (indent_level >= 0);
    spaces[indent_level] = ' ';
    indent_level += diff*2;
    spaces[indent_level] = 0;
    assert (indent_level >= 0);
}

void Messages::vprintf(int kind, const char* format, va_list va) {
    const char* now_timestamp = time_to_string(time(0));
    if (!v_message_wanted(kind)) return;
    fprintf(output, "%s [%s]%s ", now_timestamp, v_format_kind(kind), spaces);
    vfprintf(output, format, va);
}

// break a multi-line string into lines (so that we show prefix on each line)
void Messages::vprintf_multiline(
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

void Messages::vprintf_file(
    int kind, const char* filename, const char* prefix_format, va_list va
) {
    if (!v_message_wanted(kind)) return;

    char sprefix[256] = "";
    if (prefix_format) {
        vsprintf(sprefix, prefix_format, va);
    }
    const char* now_timestamp = time_to_string(time(0));
    const char* skind = v_format_kind(kind);

    ifstream f(filename);
    if (!f) return;

    string line;
    while (getline(f, line)) {
        fprintf(output, "%s [%s]%s %s%s\n", now_timestamp, skind, spaces, sprefix, line.c_str());
    }
}

void Messages::printf(int kind, const char* format, ...) {
    va_list va;
    va_start(va, format);
    vprintf(kind, format, va);
    va_end(va);
}

void Messages::printf_multiline(
    int kind, const char* str, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    vprintf_multiline(kind, str, prefix_format, va);
    va_end(va);
}

void Messages::printf_file(
    int kind, const char* filename, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    vprintf_file(kind, filename, prefix_format, va);
    va_end(va);
}

//////////////////////////////////////////////////////////////////////
// These ScopeMessages functions are utility functions that call their
// corresponding Messages functions with the same name, passing the KIND that
// was specified on creation of the ScopeMessages object.

void ScopeMessages::printf(const char* format, ...) {
    va_list va;
    va_start(va, format);
    messages.vprintf(kind, format, va);
    va_end(va);
}

void ScopeMessages::printf_multiline(
    const char* str, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    messages.vprintf_multiline(kind, str, prefix_format, va);
    va_end(va);
}

void ScopeMessages::printf_file(
    const char* filename, const char* prefix_format, ...
) {
    va_list va;
    va_start(va, prefix_format);
    messages.vprintf_file(kind, filename, prefix_format, va);
    va_end(va);
}
