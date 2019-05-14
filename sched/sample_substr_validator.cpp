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

// A sample validator that accepts results whose stderr output
// contains or doesn't contain a given string.
// Usage:
// sample_substr_validator --stderr_string xxx [--reject_if_present]] [other options]
// --reject_if_present: reject (invalidate) the result if the string is present
// (default: accept it if the string is present)

#include <vector>

#include "sched_msgs.h"
#include "sched_util_basic.h"
#include "validate_util2.h"
#include "validator.h"

using std::vector;

vector<char*> stderr_strings;
bool reject_if_present = false;

int validate_handler_init(int argc, char** argv) {
    // handle project specific arguments here
    bool found = false;
    for (int i=1; i<argc; i++) {
        if (is_arg(argv[i], "stderr_string")) {
            stderr_strings.push_back(argv[++i]);
            found = true;
        } else if (is_arg(argv[i], "reject_if_present")) {
            reject_if_present = true;
        }
    }

    if (!found) {
        log_messages.printf(MSG_CRITICAL,
            "--stderr_string missing from command line\n"
        );
        return 1;
    }
    return 0;
}

void validate_handler_usage() {
    // describe the project specific arguments here
    fprintf(stderr,
        "    Custom options:\n"
        "    --stderr_string X     accept task if X is present in stderr_out\n"
        "    [--reject_if_present] reject (invalidate) the task if X is present\n"
    );
}

int init_result(RESULT& r, void*&) {
    for(unsigned int i=0; i<stderr_strings.size(); i++) {
        char* stderr_string = stderr_strings[i];
        if (strstr(r.stderr_out, stderr_string)) {
            if (reject_if_present) return -1;
        } else {
            if (!reject_if_present) return -1;
        }
    }
    return 0;
}

int compare_results(RESULT&, void*, RESULT const&, void*, bool& match) {
    match = true;
    return 0;
}

int cleanup_result(RESULT const&, void*) {
    return 0;
}
