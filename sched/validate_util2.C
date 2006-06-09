// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// Simple validator framework:
// Lets you create a custom validator by supplying three simple functions.
// See http://boinc.berkeley.edu/validate_simple.php
//

#include "config.h"
#include <vector>

#include "boinc_db.h"

#include "sched_msgs.h"
#include "validator.h"

#include "validate_util.h"
#include "validate_util2.h"

using std::vector;

int check_set(
    vector<RESULT>& results, WORKUNIT& wu, int& canonicalid, double& credit,
    bool& retry
) {
    vector<void*> data;
    int i, j, neq = 0, n;
    int min_valid = wu.min_quorum/2+1;

    retry = false;
    n = results.size();
    data.resize(n);

    // 1. INITIALIZE DATA

    for (i=0; i!=n; i++) {
        if (init_result(results[i], data[i])) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "generic_check_set: init_result([RESULT#%d %s]) failed\n",
                results[i].id, results[i].name
            );
            goto cleanup;
        }
    }

    // 2. COMPARE

    for (i=0; i!=n; i++) {
        vector<bool> matches;
        matches.resize(n);
        neq = 0;
        for (j=0; j!=n; j++) {
            bool match = false;
            if (i == j) {
                ++neq;
                matches[j] = true;
            } else if (compare_results(results[i], data[i], results[j], data[j], match)) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
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
                if (config.max_claimed_credit && results[j].claimed_credit > config.max_claimed_credit) {
                    results[j].validate_state = VALIDATE_STATE_INVALID;
                } else {
                    results[j].validate_state = matches[j] ? VALIDATE_STATE_VALID : VALIDATE_STATE_INVALID;
                }
            }
            canonicalid = results[i].id;
            credit = median_mean_credit(results);
            break;
        }
    }

cleanup:

    // 3. CLEANUP

    for (i=0; i!=n; i++) {
        cleanup_result(results[i], data[i]);
    }
    return 0;
}

int check_pair(RESULT & r1, RESULT const& r2, bool& retry) {
    void* data1;
    void* data2;
    int retval;
    bool match;

    retry = false;
    retval = init_result(r1, data1);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d %s] [RESULT#%d %s] Couldn't initialize result 1\n",
            r1.id, r1.name, r2.id, r2.name
        );
        return retval;
    }

    retval = init_result(r2, data2);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d %s] [RESULT#%d %s] Couldn't initialize result 2\n",
            r1.id, r1.name, r2.id, r2.name
        );
        cleanup_result(r1, data1);
        return retval;
    }

    retval = compare_results(r1, data1, r2, data2, match);
    if (config.max_claimed_credit && r1.claimed_credit > config.max_claimed_credit) {
        r1.validate_state = VALIDATE_STATE_INVALID;
    } else {
        r1.validate_state = match?VALIDATE_STATE_VALID:VALIDATE_STATE_INVALID;
    }
    cleanup_result(r1, data1);
    cleanup_result(r2, data2);

    return retval;
}
