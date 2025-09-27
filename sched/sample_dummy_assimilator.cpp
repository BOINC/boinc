// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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
// But WUs are marked as assimilated, which means file deleter
// will delete output files unless you mark them as no_delete,
// or include 'no_delete' in the WU name

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

int assimilate_handler_init(int, char**) {
    return 0;
}

void assimilate_handler_usage() {
    // describe the project specific arguments here
    //fprintf(stderr,
    //    "    Custom options:\n"
    //    "    [--project_option X]  a project specific option\n"
    //);
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    log_messages.printf(MSG_NORMAL, "Assimilating %s\n", wu.name);
    if (wu.canonical_resultid) {
        OUTPUT_FILE_INFO output_file;

        log_messages.printf(MSG_NORMAL, "Found canonical result\n");
        log_messages.printf_multiline(
            MSG_DEBUG, canonical_result.xml_doc_out,
            "[%s] canonical result", wu.name
        );
        if (!(get_output_file_info(canonical_result, output_file))) {
            log_messages.printf(MSG_DEBUG,
                "[%s] Output file path %s\n",
                wu.name, output_file.path.c_str()
            );
        }
    } else {
        log_messages.printf(MSG_NORMAL, "[%s] No canonical result\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        log_messages.printf(MSG_WARNING,
            "[%s] Warning: couldn't send a result\n", wu.name
        );
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        log_messages.printf(MSG_WARNING,
            "[%s] Warning: too many error results\n", wu.name
        );
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_TOTAL_RESULTS) {
        log_messages.printf(MSG_WARNING,
            "[%s] Warning: too many total results\n", wu.name
        );
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_SUCCESS_RESULTS) {
        log_messages.printf(MSG_WARNING,
            "[%s] Warning: too many success results\n", wu.name
        );
    }
    return 0;
}
