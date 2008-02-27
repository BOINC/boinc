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

#include <string.h>
#include "config.h"
#include "sched_msgs.h"

SCHED_MSG_LOG log_messages;

const char* SCHED_MSG_LOG::v_format_kind(int kind) const {
    switch(kind) {
    case MSG_CRITICAL: return "CRITICAL";
    case MSG_NORMAL:   return "normal  ";
    case MSG_DEBUG:    return "debug   ";
    default:       return "*** internal error: invalid MessageKind ***";
    }
}

bool SCHED_MSG_LOG::v_message_wanted(int kind) const {
    return ( kind <= debug_level );
}

void SCHED_MSG_LOG::set_indent_level(const int new_indent_level) {
    if (new_indent_level < 0) indent_level = 0;
    else if (new_indent_level > 39) indent_level = 39;
    else indent_level = new_indent_level;

    memset(spaces, ' ', sizeof(spaces));
    spaces[indent_level] = 0;
}

#ifdef _USING_FCGI_

SCHED_MSG_LOG::~SCHED_MSG_LOG() {
   close();
}

void SCHED_MSG_LOG::close() {
    if (output) {
	   flush();
	   fclose(output);
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

const char *BOINC_RCSID_b40ff9bb53 = "$Id$";
