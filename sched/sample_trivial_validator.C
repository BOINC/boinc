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

// A sample validator that grants credit to any result whose CPU time is above
// a certain minimum

#include "validate_util.h"

using std::vector;

static const double MIN_CPU_TIME = 0;

int init_result_trivial(RESULT const& result, void*& data) {
    return 0;
}

int check_pair_initialized_trivial(
    RESULT & r1, void* /*data1*/,
    RESULT const& r2, void* /*data2*/,
    bool& match
) {
    match = (r1.cpu_time >= MIN_CPU_TIME && r2.cpu_time >= MIN_CPU_TIME);
    return 0;
}

int cleanup_result_trivial(RESULT const&, void*) {
    return 0;
}

int check_set(
    vector<RESULT>& results, WORKUNIT&, int& canonicalid, double& credit,
    bool& retry
) {
    retry = false;
    return generic_check_set(
        results, canonicalid, credit,
        init_result_trivial,
        check_pair_initialized_trivial,
        cleanup_result_trivial,
        1
    );
}

int check_pair(RESULT & r1, RESULT const& r2, bool& retry) {
    bool match;
    retry = false;
    int retval = check_pair_initialized_trivial(
        r1, NULL,
        r2, NULL,
        match
    );
    r1.validate_state = match?VALIDATE_STATE_VALID:VALIDATE_STATE_INVALID;
    return retval;
}

