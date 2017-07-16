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

// utility functions for keywords

#include <stdio.h>
#include <algorithm>

#include "parse.h"
#include "keyword.h"

int USER_KEYWORDS::parse(XML_PARSER& xp) {
    clear();
    int x;
    while (!xp.get_tag()) {
        if (xp.match_tag("/user_keywords")) {
            return 0;
        }
        if (xp.parse_int("yes", x)) {
            yes.push_back(x);
        } else if (xp.parse_int("no", x)) {
            no.push_back(x);
        }
    }
    return ERR_XML_PARSE;
}

void USER_KEYWORDS::write(FILE* f) {
    if (empty()) {
        return;
    }
    unsigned int i;
    fprintf(f, "<user_keywords>\n");
    for (i=0; i<yes.size(); i++) {
        fprintf(f, "   <yes>%d</yes>\n", yes[i]);
    }
    for (i=0; i<no.size(); i++) {
        fprintf(f, "   <no>%d</no>\n", no[i]);
    }
    fprintf(f, "</user_keywords>\n");
}

double keyword_score(USER_KEYWORDS& user_keywords, JOB_KEYWORDS& job_keywords) {
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
