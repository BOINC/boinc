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
//
// A job's keywords are stored in workunit.keywords as a char string.
// We don't want to parse that every time we score the job,
// so we maintain a list of JOB_KEYWORD_IDS paralleling the job array.

#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include "filesys.h"
#include "sched_main.h"

#include "sched_keyword.h"

JOB_KEYWORD_IDS *job_keywords_array;

// read user's KW prefs from file; return -1 if none
//
static void parse_kw_line(const char *buf, vector<int> &kws) {
    stringstream ss(buf);
    string item;
    while (getline(ss, item, ' ')) {
        kws.push_back(atoi(item.c_str()));
    }
}

int read_kw_prefs(int userid, USER_KEYWORDS &ukws) {
    char path[256], buf[512];
    snprintf(path, sizeof(path), "../kw_prefs/%d", userid);
    if (!boinc_file_exists(path)) return -1;
    FILE *f = boinc::fopen(path, "r");
    if (!f) return -1;
    if (config.debug_keyword) {
        log_messages.printf(MSG_NORMAL,
            "[keyword] reading user kw prefs from file\n"
        );
    }
    ukws.clear();
    int ret = 0;
    if (boinc::fgets(buf, sizeof(buf), f)) {
        parse_kw_line(buf, ukws.yes);
    } else {
        ret = 1;
    }
    if (boinc::fgets(buf, sizeof(buf), f)) {
        parse_kw_line(buf, ukws.no);
    } else {
        ret = 1;
    }
    boinc::fclose(f);
    return ret;
}

// compute the score increment for the given job and user keywords,
// or -1 if user has "no" for one of the job keywords
//
double keyword_score_aux(
    USER_KEYWORDS& uks, JOB_KEYWORD_IDS& jks
) {
    double score = 0;

    for (unsigned int i=0; i<jks.ids.size(); i++) {
        int jk = jks.ids[i];
        if (std::find(uks.yes.begin(), uks.yes.end(), jk) != uks.yes.end()) {
            score += 1;
        } else if (std::find(uks.no.begin(), uks.no.end(), jk) != uks.no.end()) {
            add_no_work_message(
                "Some jobs are ruled out by your science/location preferences"
            );
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
            log_messages.printf(MSG_NORMAL, "[keyword] user has no keywords; returning 0\n");
        }
        return 0;
    }

    // parse job keywords if not already done
    //
    JOB_KEYWORD_IDS& jk = job_keywords_array[i];
    if (jk.empty()) {
        WU_RESULT& wr = ssp->wu_results[i];
        if (!strlen(wr.workunit.keywords)) {
            if (config.debug_keyword) {
                log_messages.printf(MSG_NORMAL, "[keyword] job has no keywords; returning 0\n");
            }
            return 0;
        }
        jk.parse_str(wr.workunit.keywords);
    }
    double s = keyword_score_aux(uk, jk);
    if (config.debug_keyword) {
        log_messages.printf(MSG_NORMAL, "[keyword] keyword score: %f\n", s);
    }
    return s;
}

// called when job is removed from array
//
void keyword_sched_remove_job(int i) {
    job_keywords_array[i].clear();
}

// called at CGI start to initialize job keyword array
//
void keyword_sched_init() {
    job_keywords_array = new JOB_KEYWORD_IDS[ssp->max_wu_results];
}
