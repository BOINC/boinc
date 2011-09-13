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

// Simple validator framework:
// Lets you create a custom validator by supplying three functions.
// See http://boinc.berkeley.edu/trac/wiki/ValidationSimple
//

#include "config.h"
#include <vector>
#include <cstdlib>
#include <string>

#include "boinc_db.h"
#include "error_numbers.h"

#include "sched_config.h"
#include "sched_msgs.h"

#include "validator.h"
#include "validate_util.h"
#include "validate_util2.h"

using std::vector;

// Given a set of results, check for a canonical result,
// i.e. a set of at least min_quorum/2+1 results for which
// that are equivalent according to check_pair().
//
// invariants:
// results.size() >= wu.min_quorum
// for each result:
//   result.outcome == SUCCESS
//   result.validate_state == INIT
//
int check_set(
    vector<RESULT>& results, WORKUNIT& wu,
    int& canonicalid, double&, bool& retry
) {
    vector<void*> data;
    vector<bool> had_error;
    int i, j, neq = 0, n, retval;
    int min_valid = wu.min_quorum/2+1;

    retry = false;
    n = results.size();
    data.resize(n);
    had_error.resize(n);

    // Initialize results

    for (i=0; i<n; i++) {
        data[i] = NULL;
        had_error[i] = false;
    }
    int good_results = 0;
    for (i=0; i<n; i++) {
        retval = init_result(results[i], data[i]);
        if (retval == ERR_OPENDIR) {
            log_messages.printf(MSG_CRITICAL,
                "check_set: init_result([RESULT#%d %s]) transient failure\n",
                results[i].id, results[i].name
            );
            had_error[i] = true;
        } else if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "check_set: init_result([RESULT#%d %s]) failed: %s\n",
                results[i].id, results[i].name, boincerror(retval)
            );
            results[i].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            results[i].validate_state = VALIDATE_STATE_INVALID;
            had_error[i] = true;
        } else  {
            good_results++;
        }
    }
    if (good_results < wu.min_quorum) goto cleanup;

    // Compare results

    for (i=0; i<n; i++) {
        if (had_error[i]) continue;
        vector<bool> matches;
        matches.resize(n);
        neq = 0;
        for (j=0; j!=n; j++) {
            if (had_error[j]) continue;
            bool match = false;
            if (i == j) {
                ++neq;
                matches[j] = true;
            } else if (compare_results(results[i], data[i], results[j], data[j], match)) {
                log_messages.printf(MSG_CRITICAL,
                    "generic_check_set: check_pair_with_data([RESULT#%d %s], [RESULT#%d %s]) failed\n",
                    results[i].id, results[i].name, results[j].id, results[j].name
                );
            } else if (match) {
                ++neq;
                matches[j] = true;
            }
        }
        if (neq >= min_valid) {

            // set validate state for each result
            //
            for (j=0; j!=n; j++) {
                if (had_error[j]) continue;
                results[j].validate_state = matches[j] ? VALIDATE_STATE_VALID : VALIDATE_STATE_INVALID;
            }
            canonicalid = results[i].id;
            break;
        }
    }

cleanup:

    for (i=0; i<n; i++) {
        cleanup_result(results[i], data[i]);
    }
    return 0;
}

// r1 is the new result; r2 is canonical result
//
void check_pair(RESULT& r1, RESULT& r2, bool& retry) {
    void* data1;
    void* data2;
    int retval;
    bool match;

    retry = false;
    retval = init_result(r1, data1);
    if (retval == ERR_OPENDIR) {
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%d %s]) transient failure 1\n",
            r1.id, r1.name
        );
        retry = true;
        return;
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%d %s]) perm failure 1\n",
            r1.id, r1.name
        );
        r1.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        r1.validate_state = VALIDATE_STATE_INVALID;
        return;
    }

    retval = init_result(r2, data2);
    if (retval == ERR_OPENDIR) {
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%d %s]) transient failure 2\n",
            r2.id, r2.name
        );
        cleanup_result(r1, data1);
        retry = true;
        return;
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%d %s]) perm failure2\n",
            r2.id, r2.name
        );
        cleanup_result(r1, data1);
        r1.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        r1.validate_state = VALIDATE_STATE_INVALID;
        return;
    }

    retval = compare_results(r1, data1, r2, data2, match);
    r1.validate_state = match?VALIDATE_STATE_VALID:VALIDATE_STATE_INVALID;
    cleanup_result(r1, data1);
    cleanup_result(r2, data2);
}
