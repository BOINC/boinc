// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
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

#include "message.h"
#include "log_flags.h"

ClientMessages log_messages;

const char* ClientMessages::v_format_kind(int kind) const
{
    switch(kind) {
    case DEBUG_STATE:       return "DEBUG_STATE      ";
    case DEBUG_TASK:        return "DEBUG_TASK       ";
    case DEBUG_FILE_XFER:   return "DEBUG_FILE_XFER  ";
    case DEBUG_SCHED_OP:    return "DEBUG_SCHED_OP   ";
    case DEBUG_HTTP:        return "DEBUG_HTTP       ";
    case DEBUG_TIME:        return "DEBUG_TIME       ";
    case DEBUG_NET_XFER:    return "DEBUG_NET_XFER   ";
    case DEBUG_MEASUREMENT: return "DEBUG_MEASUREMENT";
    case DEBUG_POLL:        return "DEBUG_POLL       ";
    default:                return "*** internal error: invalid MessageKind ***";
    }
}

bool ClientMessages::v_message_wanted(int kind) const
{
    switch (kind) {
    case DEBUG_STATE:       return log_flags.state_debug;
    case DEBUG_TASK:        return log_flags.task_debug;
    case DEBUG_FILE_XFER:   return log_flags.file_xfer_debug;
    case DEBUG_SCHED_OP:    return log_flags.sched_op_debug;
    case DEBUG_HTTP:        return log_flags.http_debug;
    case DEBUG_TIME:        return log_flags.time_debug;
    case DEBUG_NET_XFER:    return log_flags.net_xfer_debug;
    case DEBUG_MEASUREMENT: return log_flags.measurement_debug;
    case DEBUG_POLL:        return log_flags.poll_debug;
    default: return false;
    }
}
