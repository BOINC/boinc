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

// A sample assimilator that only writes a log message.

#include <cstdio>

#include "boinc_db.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "assimilate_handler.h"
#include "validate_util.h"

using std::vector;
using std::string;

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    SCOPE_MSG_LOG scope_messages(log_messages, SCHED_MSG_LOG::NORMAL);
    scope_messages.printf("[%s] Assimilating\n", wu.name);
    if (wu.canonical_resultid) {
        string output_file_name;

        scope_messages.printf("[%s] Found canonical result\n", wu.name);
        log_messages.printf_multiline(
            SCHED_MSG_LOG::DEBUG, canonical_result.xml_doc_out,
            "[%s] canonical result", wu.name
        );
       if (!(get_output_file_path(canonical_result, output_file_name))) {
           scope_messages.printf("[%s] Output file path %s\n", wu.name, output_file_name.c_str());
       }
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
