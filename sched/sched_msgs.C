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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "sched_msgs.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

SCHED_MSG_LOG log_messages;

const char* SCHED_MSG_LOG::v_format_kind(int kind) const {
    switch(kind) {
    case CRITICAL: return "CRITICAL";
    case NORMAL:   return "normal  ";
    case DEBUG:    return "debug   ";
    default:       return "*** internal error: invalid MessageKind ***";
    }
}

bool SCHED_MSG_LOG::v_message_wanted(int kind) const {
    return ( kind <= debug_level );
}

const char *BOINC_RCSID_b40ff9bb53 = "$Id$";
