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
// See https://github.com/BOINC/boinc/wiki/ValidationSimple
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

// Given a set of results:
// 1) call init_result() for each one;
//    this detects results with bad or missing output files
// 2) if # of good results is >= wu.min_quorum,
//    check for a canonical result,
//    i.e. a set of at least min_quorum/2+1 results for which
//    that are equivalent according to check_pair().
//
// input invariants:
// for each result:
//   result.outcome == SUCCESS
//   result.validate_state == INIT
//
// Outputs:
// canonicalid: the ID of canonical result, if any
// result.outcome, result.validate_state
//    modified; caller must update DB
// retry: set to true if some result had a transient failure
//    (i.e. there was a broken NFS mount).
//    Should call this again after a while.
//
int check_set(
    vector<RESULT>& results, WORKUNIT& wu,
    DB_ID_TYPE& canonicalid, double&, bool& retry
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
    int suspicious_results = 0;
    for (i=0; i<n; i++) {
        retval = init_result(results[i], data[i]);
        if (retval == ERR_OPENDIR) {
            log_messages.printf(MSG_CRITICAL,
                "check_set: init_result([RESULT#%lu %s]) transient failure\n",
                results[i].id, results[i].name
            );
            retry = true;
            had_error[i] = true;
        } else if (retval == VAL_RESULT_SUSPICIOUS) {
            log_messages.printf(MSG_NORMAL,
                "[RESULT#%lu %s] considered to be suspicious\n",
                results[i].id, results[i].name
            );
            suspicious_results++;
        } else if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "check_set: init_result([RESULT#%lu %s]) failed: %s\n",
                results[i].id, results[i].name, boincerror(retval)
            );
            results[i].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            results[i].validate_state = VALIDATE_STATE_INVALID;
            had_error[i] = true;
        } else  {
            good_results++;
        }
    }

    // don't count a single "suspicious" result as "good",
    // but do if there are more results to compare it with
    //
    if (suspicious_results > 1 || good_results > 0) {
        good_results += suspicious_results;
    }

    // if there are "suspicious" results and min_quorum < g_app->target_nresults
    // (i.e. adaptive replication), raise min_quorum to g_app->target_nresults
    // the abs() is there for Einstein@Home-specific use of app->target_nresults,
    // please leave it in there.
    //
    if (suspicious_results && wu.min_quorum < abs(g_app->target_nresults)) {
        log_messages.printf(MSG_NORMAL, "suspicious result - raising quorum\n");
        wu.min_quorum = abs(g_app->target_nresults);
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
                    "generic_check_set: check_pair_with_data([RESULT#%lu %s], [RESULT#%lu %s]) failed\n",
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
            "check_pair: init_result([RESULT#%lu %s]) transient failure 1\n",
            r1.id, r1.name
        );
        retry = true;
        return;
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%lu %s]) perm failure 1\n",
            r1.id, r1.name
        );
        r1.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        r1.validate_state = VALIDATE_STATE_INVALID;
        return;
    }

    retval = init_result(r2, data2);
    if (retval == ERR_OPENDIR) {
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%lu %s]) transient failure 2\n",
            r2.id, r2.name
        );
        cleanup_result(r1, data1);
        retry = true;
        return;
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%lu %s]) perm failure2\n",
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
