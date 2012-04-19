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

// A sample assimilator that only writes a log message.

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <string>

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
    SCOPE_MSG_LOG scope_messages(log_messages, MSG_NORMAL);
    scope_messages.printf("[%s] Assimilating\n", wu.name);
    if (wu.canonical_resultid) {
        OUTPUT_FILE_INFO output_file;

        scope_messages.printf("[%s] Found canonical result\n", wu.name);
        log_messages.printf_multiline(
            MSG_DEBUG, canonical_result.xml_doc_out,
            "[%s] canonical result", wu.name
        );
       if (!(get_output_file_info(canonical_result, output_file))) {
           scope_messages.printf(
                "[%s] Output file path %s\n",
                wu.name, output_file.path.c_str()
            );
       }
    } else {
        scope_messages.printf("[%s] No canonical result\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        log_messages.printf(MSG_CRITICAL,
            "[%s] Error: couldn't send a result\n", wu.name
        );
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        log_messages.printf(MSG_CRITICAL,
            "[%s] Error: too many error results\n", wu.name
        );
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_TOTAL_RESULTS) {
        log_messages.printf(MSG_CRITICAL,
            "[%s] Error: too many total results\n", wu.name
        );
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_SUCCESS_RESULTS) {
        log_messages.printf(MSG_CRITICAL,
            "[%s] Error: too many success results\n", wu.name
        );
    }
    return 0;
}

const char *BOINC_RCSID_8f6a5a2d27 = "$Id$";
