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
// always return zero
//
int check_set(
    vector<RESULT>& results, WORKUNIT& wu,
    DB_ID_TYPE& canonicalid, double&, bool& retry
) {
    vector<void*> data;
    vector<bool> had_error;
    int i, j, neq = 0, n, retval=0;
    int min_valid = wu.min_quorum/2+1;

    retry = false;
    n = results.size();
    data.resize(n);
    had_error.resize(n);

    // Initialize results
    // For each one we potentially allocate data,
    // so always exit via goto cleanup:
    // to free this mem

    for (i=0; i<n; i++) {
        data[i] = NULL;
        had_error[i] = false;
    }
    int good_results = 0;
    int suspicious_results = 0;
    for (i=0; i<n; i++) {
        retval = init_result(results[i], data[i]);
        switch (retval) {
        case 0:
            good_results++;
            break;
        case ERR_OPENDIR:
        case VAL_RESULT_TRANSIENT_ERROR:
            log_messages.printf(MSG_CRITICAL,
                "check_set: init_result([RESULT#%lu %s]) transient failure\n",
                results[i].id, results[i].name
            );
            retry = true;
            had_error[i] = true;
            break;
        case VAL_RESULT_SUSPICIOUS:
            log_messages.printf(MSG_NORMAL,
                "[RESULT#%lu %s] considered to be suspicious\n",
                results[i].id, results[i].name
            );
            suspicious_results++;
            break;
        default:
            log_messages.printf(MSG_CRITICAL,
                "check_set: init_result([RESULT#%lu %s]) failed: %s\n",
                results[i].id, results[i].name, boincerror(retval)
            );
            results[i].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            results[i].validate_state = VALIDATE_STATE_INVALID;
            had_error[i] = true;
            break;
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

    // for each result, count how many it matches (including itself)
    // If this is at least min_valid, it's the canonical result

    for (i=0; i<n; i++) {
        if (had_error[i]) continue;
        vector<bool> matches;
        matches.resize(n);
        neq = 0;
        for (j=0; j<n; j++) {
            if (had_error[j]) continue;
            if (i == j) {
                ++neq;
                matches[j] = true;
                continue;
            }
            bool match = false;
            retval = compare_results(
                results[i], data[i], results[j], data[j], match
            );
            switch (retval) {
            case ERR_OPENDIR:
            case VAL_RESULT_TRANSIENT_ERROR:
                retry = true;
                goto cleanup;
            case 0:
                if (match) {
                    ++neq;
                    matches[j] = true;
                }
                break;
            default:
                log_messages.printf(MSG_CRITICAL,
                    "check_set(): compare_results([RESULT#%lu %s], [RESULT#%lu %s]) failed\n",
                    results[i].id, results[i].name, results[j].id, results[j].name
                );
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

// a straggler instance has arrived after the WU is already validated.
// r1 is the new result; r2 is canonical result
//
void check_pair(RESULT& r1, RESULT& r2, bool& retry) {
    void* data1;
    void* data2;
    int retval;
    bool match;

    retry = false;
    retval = init_result(r1, data1);
    switch (retval) {
    case ERR_OPENDIR:
    case VAL_RESULT_TRANSIENT_ERROR:
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%lu %s]) transient failure 1\n",
            r1.id, r1.name
        );
        retry = true;
        return;
    case 0:
        break;
    default:
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%lu %s]) perm failure 1\n",
            r1.id, r1.name
        );
        r1.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        r1.validate_state = VALIDATE_STATE_INVALID;
        return;
    }

    retval = init_result(r2, data2);
    switch (retval) {
    case ERR_OPENDIR:
    case VAL_RESULT_TRANSIENT_ERROR:
        log_messages.printf(MSG_CRITICAL,
            "check_pair: init_result([RESULT#%lu %s]) transient failure 2\n",
            r2.id, r2.name
        );
        cleanup_result(r1, data1);
        retry = true;
        return;
    case 0:
        break;
    default:
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
    switch (retval) {
    case ERR_OPENDIR:
    case VAL_RESULT_TRANSIENT_ERROR:
        retry = true;
        break;
    case 0:
        r1.validate_state = match?VALIDATE_STATE_VALID:VALIDATE_STATE_INVALID;
        break;
    default:
        log_messages.printf(MSG_CRITICAL,
            "check_pair: compare_results([RESULT#%lu RESULT#%lu]) perm failure\n",
            r1.id, r2.id
        );
        r1.validate_state = VALIDATE_STATE_INVALID;
        break;
    }
    cleanup_result(r1, data1);
    cleanup_result(r2, data2);
}
