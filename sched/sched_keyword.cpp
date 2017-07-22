// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// code related to keyword-based job scoring

#include <algorithm>

#include "sched_main.h"
#include "keyword.h"

JOB_KEYWORDS *job_keywords_array;

double keyword_score_aux(
    USER_KEYWORDS& user_keywords, JOB_KEYWORDS& job_keywords
) {
    double score = 0;
    for (unsigned int i=0; i<job_keywords.ids.size(); i++) {
        int jk = job_keywords.ids[i];
        if (std::find(user_keywords.yes.begin(), user_keywords.yes.end(), jk) != user_keywords.yes.end()) {
            score += 1;
        } else if (std::find(user_keywords.no.begin(), user_keywords.no.end(), jk) != user_keywords.no.end()) {
            return -1;
        }
    }
    return score;
}

// return keyword score for ith job in array
//
double keyword_score(int i) {
    USER_KEYWORDS& uk = g_request->user_keywords;
    if (uk.empty()) {
        if (config.debug_keyword) {
            log_messages.printf(MSG_NORMAL, "user has no keywords; returning 0\n");
        }
        return 0;
    }

    // parse job keywords if not already done
    //
    JOB_KEYWORDS& jk = job_keywords_array[i];
    if (jk.empty()) {
        WU_RESULT& wr = ssp->wu_results[i];
        if (empty(wr.workunit.keywords)) {
            if (config.debug_keyword) {
                log_messages.printf(MSG_NORMAL, "job has no keywords; returning 0\n");
            }
            return 0;
        }
        jk.parse_str(wr.workunit.keywords);
    }
    double s = keyword_score_aux(uk, jk);
    if (config.debug_keyword) {
        log_messages.printf(MSG_NORMAL, "keyword score: %f\n", s);
    }
    return s;
}

// called when job is removed from array
//
void keyword_sched_remove_job(int i) {
    job_keywords_array[i].clear();
}

void keyword_sched_init() {
    job_keywords_array = new JOB_KEYWORDS[ssp->max_wu_results];
}
