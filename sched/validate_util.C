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

#include "validate_util.h"
#include "sched_util.h"
#include "sched_config.h"
#include "parse.h"
#include <cassert>

extern CONFIG config;

// get the name of a result's (first) output file
//
int get_output_file_path(RESULT const& result, string& path) {
    char buf[256];
    bool flag;

    flag = parse_str(result.xml_doc_in, "<name>", buf, sizeof(buf));
    if (!flag) return -1;
    path = config.upload_dir;
    path += '/';
    path += buf;
    return 0;
}


// If we have a canonical result, and N results (including itself) that match
// it, compute a canonical credit as follows:
// - if N==1, give that credit
// - if N==2, give min credit
// - if N>2, toss out min and max, give average of rest
//
double median_mean_credit(vector<RESULT> const& results) {
    typedef vector<RESULT>::const_iterator it;

    it it_low = results.end(), it_high;
    double credit_low = 0, credit_high = 0;

    size_t n_valid = 0;

    for (it i = results.begin(); i != results.end(); ++i) {
        if (i->validate_state != VALIDATE_STATE_VALID) continue;
        ++n_valid;
        if (it_low == results.end()) {
            it_low = it_high = i;
            credit_low = credit_high = i->claimed_credit;
        } else {
            if (i->claimed_credit < credit_low) {
                it_low = i;
                credit_low = i->claimed_credit;
            }
            if (i->claimed_credit > credit_high) {
                it_high = i;
                credit_high = i->claimed_credit;
            }
        }
    }

    // compute a canonical credit as follows:
    // - if N==1, give that credit
    // - if N==2, give min credit
    // - if N>2, toss out min and max, give average of rest
    //
    if (n_valid == 1) {
        return credit_low;
    } else if (n_valid == 2) {
        return credit_low;
    } else {
        double sum = 0;

        for (it i = results.begin(); i != results.end(); ++i) {
            if (i == it_low) continue;
            if (i == it_high) continue;
            if (i->validate_state != VALIDATE_STATE_VALID) continue;

            sum += i->claimed_credit;
        }
        return sum/(n_valid-2);
    }
}

// Generic validation function that compares each result to each other one
// and sees if there is a strict majority.  The comparison function is
// similar to check_pair but takes an additional initialization parameter.
//
// This function takes 3 call-back functions, each of which accept a void*
// and should return !=0 on error:
//
//    1. init_result - initialize all results - for example, call
//       read_file_string and compute an MD5. Return a void*
//    2. check_pair_with_data - same as check_pair but with extra data from
//       init_result
//    3. cleanup_result - deallocate anything created by init_result.  Should
//       do nothing with NULL data
//
// see validate_test.C example usage.
//
int generic_check_set_majority(
    vector<RESULT>& results, int& canonicalid, double& credit,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f
) {
    assert (!results.empty());

    vector<void*> data;
    vector<RESULT>::size_type i, j, neq = 0, n = results.size();
    data.resize(n);

    // 1. INITIALIZE DATA
    for (i = 0; i != n; ++i) {
        if (init_result_f(results[i], data[i])) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "check_set_majority: init_result([RESULT#%d %s]) failed\n",
                results[i].id, results[i].name);
            goto cleanup;
        }
    }

    // 2. COMPARE
    for (i = 0; i != n; ++i) {
        vector<bool> matches;
        matches.resize(n);
        neq = 0;
        for (j = 0; j != n; ++j) {
            bool match = false;
            if (i == j) {
                ++neq;
                matches[j] = true;
            } else if (check_pair_with_data_f(results[i], data[i], results[j], data[j], match)) {
                log_messages.printf(
                    SchedMessages::CRITICAL,
                    "check_set_majority: check_pair_with_data([RESULT#%d %s], [RESULT#%d %s]) failed\n",
                    results[i].id, results[i].name, results[j].id, results[j].name);
            } else if (match) {
                ++neq;
                matches[j] = true;
            }
        }
        if (neq > n/2) {
            // set validate state for each result
            for (j = 0; j != n; ++j) {
                results[j].validate_state = matches[j] ? VALIDATE_STATE_VALID : VALIDATE_STATE_INVALID;
            }
            canonicalid = results[i].id;
            credit = median_mean_credit(results);
            break;
        }
    }

cleanup:
    // 3. CLEANUP
    for (i = 0; i != n; ++i) {
        cleanup_result_f(results[i], data[i]);
    }
    return 0;
}

int generic_check_pair(
    RESULT const& r1, RESULT const& r2, bool& match,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f
) {
    void* data1;
    void* data2;
    int retval;

    retval = init_result_f(r1, data1);
    if (retval) {
        log_messages.printf(
            SchedMessages::CRITICAL,
            "[RESULT#%d %s] [RESULT#%d %s] Couldn't initialize result 1\n",
            r1.id, r1.name, r2.id, r2.name
        );
        return retval;
    }

    retval = init_result_f(r2, data2);
    if (retval) {
        log_messages.printf(
            SchedMessages::CRITICAL,
            "[RESULT#%d %s] [RESULT#%d %s] Couldn't initialize result 2\n",
            r1.id, r1.name, r2.id, r2.name
            );
        cleanup_result_f(r1, data1);
        return retval;
    }

    retval = check_pair_with_data_f(r1, data1, r2, data2, match);

    cleanup_result_f(r1, data1);
    cleanup_result_f(r2, data2);

    return retval;
}
