// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// An assimilator that runs a script of your choosing to handle completed jobs.
// This script is invoked as
//
// scriptname --wu_name X f1 ... fn
//   where X is the workunit name
//   and f1 ... fn are the output files of the canonical result
// or
// scriptname --wu_name X --error N
// where N is an integer encoding the reasons for the job's failure
// (see WU_ERROR_* in html/inc/common_defs.inc)

#include <vector>
#include <string>
#include <sys/param.h>

#include "boinc_db.h"
#include "error_numbers.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "validator.h"
#include "sched_config.h"

using std::vector;
using std::string;

bool first = true;
char script[MAXPATHLEN];

void parse_cmdline() {
    strcpy(script, "");
    for (int i=1; i<g_argc; i++) {
        if (!strcmp(g_argv[i], "--script")) {
            sprintf(script, "../bin/%s", g_argv[++i]);
        }
    }
    if (!strlen(script)) {
        log_messages.printf(MSG_CRITICAL,
            "script name missing from command line\n"
        );
        exit(1);
    }
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;
    char cmd[4096];

    if (first) {
        parse_cmdline();
        first = false;
    }

    if (wu.canonical_resultid) {
        sprintf(cmd, "%s --wu_name %s", script, wu.name);
        vector<string> paths;
        retval = get_output_file_paths(canonical_result, paths);
        if (retval) return retval;
        for (unsigned int i=0; i<paths.size(); i++) {
            strcat(cmd, " ");
            strcat(cmd, paths[i].c_str());
        }
    } else {
        sprintf(cmd, "%s --wu_name %s --error %d",
            script, wu.name, wu.error_mask
        );
    }
    retval = system(cmd);
    if (retval) return retval;
    return 0;
}
