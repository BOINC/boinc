// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// An assimilator that runs a script to handle completed jobs,
// so that you can do assimilation in Python, PHP, Perl, bash, etc.
//
// cmdline args to this program:
// --script "scriptname arg1 ... argn"
//
// The script assimilates a completed job.
//
// arg1 ... argn represent cmdline args to be passed to the script.
// the options are:
//
// files        list of output files of the job's canonical result
// wu_id        workunit ID
// result_id    ID of the canonical result
// runtime      runtime of the canonical result
// batch_id     the job's batch ID
//
// if no args are specified, the script is invoked as
// scriptname wu_id files
//
// If the workunit has no canonical result (i.e. it failed)
// the script is invoked as
// scriptname --error N wu_id
// where N is an integer encoding the reasons for the job's failure
// (see WU_ERROR_* in html/inc/common_defs.inc)

#include <vector>
#include <string>
#include <sys/param.h>

#include "boinc_db.h"
#include "error_numbers.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "validate_util.h"
#include "validator.h"
#include "sched_config.h"

#include "assimilate_handler.h"

using std::vector;
using std::string;

vector<string> script;

int assimilate_handler_init(int argc, char** argv) {
    // handle project specific arguments here
    for (int i=1; i<argc; i++) {
        if (is_arg(argv[i], "script")) {
            script = split(argv[++i], ' ');
            if (script.size() == 1) {
                script.push_back("wu_id");
                script.push_back("files");
            }
        }
    }
    if (!script.size()) {
        log_messages.printf(MSG_CRITICAL,
            "script name missing from command line\n"
        );
        return 1;
    }
    return 0;
}

void assimilate_handler_usage() {
    // describe the project specific arguments here
    fprintf(stderr,
        "    Custom options:\n"
        "    --script \"X\"  call script to assimilate job\n"
        "                    see comment in script_assimilator.cpp for details\n"
    );
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;
    char cmd[4096], buf[256];
    unsigned int i, j;

    if (wu.canonical_resultid) {
        sprintf(cmd, "../bin/%s", script[0].c_str());
        vector<string> paths;
        retval = get_output_file_paths(canonical_result, paths);
        if (retval) return retval;
        for (i=1; i<script.size(); i++) {
            string& s = script[i];
            if (s == "files") {
                for (j=0; j<paths.size(); j++) {
                    strcat(cmd, " ");
                    strcat(cmd, paths[j].c_str());
                }
            } else if (s == "wu_id") {
                sprintf(buf, " %lu", wu.id);
                strcat(cmd, buf);
            } else if (s == "runtime") {
                sprintf(buf, " %f", canonical_result.elapsed_time);
                strcat(cmd, buf);
            } else if (s == "batch_id") {
                sprintf(buf, " %d", wu.batch);
                strcat(cmd, buf);
            }
        }
    } else {
        sprintf(cmd, "../bin/%s --error %d %s %lu %d",
            script[0].c_str(), wu.error_mask, wu.name, wu.id, wu.batch
        );
    }
    log_messages.printf(MSG_DEBUG, "invoking script: %s\n", cmd);
    retval = system(cmd);
    if (retval) return retval;
    return 0;
}
