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

// A sample assimilator that only writes a log message.

#include <cstdio>

#include "boinc_db.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "assimilate_handler.h"

using std::vector;

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
) {
    SCOPE_MSG_LOG scope_messages(log_messages, SCHED_MSG_LOG::NORMAL);
    scope_messages.printf("[%s] Assimilating\n", wu.name);
    if (wu.canonical_resultid) {
        scope_messages.printf("[%s] Found canonical result\n", wu.name);
        log_messages.printf_multiline(
            SCHED_MSG_LOG::DEBUG, canonical_result.xml_doc_out,
            "[%s] canonical result", wu.name
        );
    } else {
        scope_messages.printf("[%s] No canonical result\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: couldn't send a result\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: too many error results\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_TOTAL_RESULTS) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: too many total results\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_SUCCESS_RESULTS) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: too many success results\n", wu.name);
    }
    return 0;
}

const char *BOINC_RCSID_8f6a5a2d27 = "$Id$";
