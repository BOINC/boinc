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

// Code to facilitate writing validators.
// Can be used as the basis for a validator that accepts everything
// (see validate_trivial.C),
// or that requires strict or fuzzy equality.

#include <cassert>

#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "filesys.h"

#include "sched_util.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "validate_util.h"

using std::vector;
using std::string;

extern SCHED_CONFIG config;

// get the name of a result's (first) output file
//
int get_output_file_path(RESULT const& result, string& path_str) {
    char buf[256], path[256];
    bool flag;

    flag = parse_str(result.xml_doc_out, "<name>", buf, sizeof(buf));
    if (!flag) return ERR_XML_PARSE;
    dir_hier_path(buf, config.upload_dir, config.uldl_dir_fanout, true, path);
	if (!boinc_file_exists(path)) {
		dir_hier_path(buf, config.upload_dir, config.uldl_dir_fanout, false, path);
	}
    path_str = path;
    return 0;
}


// If we have N correct results, compute a canonical credit as follows:
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

// Generic validation function that compares each result to each other one and
// sees if there MIN_VALID results match.  The comparison function is similar
// to check_pair but takes an additional data parameter.
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
int generic_check_set(
    vector<RESULT>& results, int& canonicalid, double& credit,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f,
    size_t min_valid)
{
    assert (!results.empty());

    vector<void*> data;
    vector<RESULT>::size_type i, j, neq = 0, n = results.size();
    data.resize(n);

    // 1. INITIALIZE DATA
    for (i = 0; i != n; ++i) {
        if (init_result_f(results[i], data[i])) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "generic_check_set: init_result([RESULT#%d %s]) failed\n",
                results[i].id, results[i].name
            );
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
                    SCHED_MSG_LOG::CRITICAL,
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

// similar to generic_check_set, but require a strict majority of results
// (N_results / 2) to be valid
int generic_check_set_majority(
    vector<RESULT>& results, int& canonicalid, double& credit,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f)
{
    return generic_check_set(
        results, canonicalid, credit,
        init_result_f, check_pair_with_data_f, cleanup_result_f,
        results.size() / 2
    );
}

int generic_check_pair(
    RESULT & r1, RESULT const& r2,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f
) {
    void* data1;
    void* data2;
    int retval;
    bool match;

    retval = init_result_f(r1, data1);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[RESULT#%d %s] [RESULT#%d %s] Couldn't initialize result 1\n",
            r1.id, r1.name, r2.id, r2.name
        );
        return retval;
    }

    retval = init_result_f(r2, data2);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[RESULT#%d %s] [RESULT#%d %s] Couldn't initialize result 2\n",
            r1.id, r1.name, r2.id, r2.name
        );
        cleanup_result_f(r1, data1);
        return retval;
    }

    retval = check_pair_with_data_f(r1, data1, r2, data2, match);
    r1.validate_state = match?VALIDATE_STATE_VALID:VALIDATE_STATE_INVALID;

    cleanup_result_f(r1, data1);
    cleanup_result_f(r2, data2);

    return retval;
}

const char *BOINC_RCSID_07049e8a0e = "$Id$";
