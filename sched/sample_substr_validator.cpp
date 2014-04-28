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

// A sample validator that accepts results whose stderr output
// contains a given string.
// Usage:
// sample_substr_validator --stderr_string xxx [other options]

#include "sched_msgs.h"
#include "validate_util2.h"
#include "validator.h"

bool first = true;
char* stderr_string;

void parse_cmdline() {
    for (int i=1; i<g_argc; i++) {
        if (!strcmp(g_argv[i], "--stderr_string")) {
            stderr_string = g_argv[++i];
            return;
        }
    }
    log_messages.printf(MSG_CRITICAL,
        "--stderr_string missing from command line\n"
    );
    exit(1);
}

int init_result(RESULT& r, void*&) {
    if (first) {
        parse_cmdline();
        first = false;
    }
    if (strstr(r.stderr_out, stderr_string)) {
        return 0;
    } else {
        return -1;
    }
}

int compare_results(RESULT&, void*, RESULT const&, void*, bool& match) {
    match = true;
    return 0;
}

int cleanup_result(RESULT const&, void*) {
    return 0;
}

const char *BOINC_RCSID_f3a7a34795 = "$Id$";
