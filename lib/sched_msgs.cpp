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

#include <string.h>
#include "config.h"
#include "sched_msgs.h"

SCHED_MSG_LOG log_messages;

const char* SCHED_MSG_LOG::v_format_kind(int kind) const {
    switch(kind) {
    case MSG_CRITICAL: return "[CRITICAL]";
    case MSG_WARNING:  return "[warning]";
    case MSG_NORMAL:   return "";
    case MSG_DEBUG:    return "[debug]";
    default:       return "*** internal error: invalid MessageKind ***";
    }
}

bool SCHED_MSG_LOG::v_message_wanted(int kind) const {
    return ( kind <= debug_level );
}

#ifdef _USING_FCGI_

SCHED_MSG_LOG::~SCHED_MSG_LOG() {
   close();
}

void SCHED_MSG_LOG::close() {
    if (output) {
        flush();
        boinc::fclose(output);
        output = NULL;
    }
}

void SCHED_MSG_LOG::redirect(FILE* f) {
    close();
    output = f;
}

void SCHED_MSG_LOG::flush() {
    if (output) {
        fflush(output->stdio_stream);
    }
}
#endif
