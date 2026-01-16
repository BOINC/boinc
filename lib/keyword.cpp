// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#include "parse.h"
#include "keyword.h"

int KEYWORD::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/keyword")) {
            return 0;
        }
        if (xp.parse_int("id", id)) continue;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("description", description)) continue;
        if (xp.parse_int("parent", parent)) continue;
        if (xp.parse_int("level", level)) continue;
        if (xp.parse_int("category", category)) continue;
    }
    return ERR_XML_PARSE;
}

#ifndef _USING_FCGI_
void KEYWORD::write_xml(MIOFILE& mf) {
    mf.printf(
        "<keyword>\n"
        "   <name>%s</name>\n"
        "   <description>%s</description>\n"
        "   <parent>%d</parent>\n"
        "   <level>%d</level>\n"
        "   <category>%d</category>\n"
        "</keyword>\n",
        name.c_str(), description.c_str(), parent, level, category
    );
}
#endif

int KEYWORDS::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/keywords")) {
            return 0;
        }
        if (xp.match_tag("keyword")) {
            KEYWORD kw;
            int retval = kw.parse(xp);
            if (retval) {
                printf("KEYWORD parse fail: %d\n", retval);
                return retval;
            }
            keywords[kw.id] = kw;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

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
    boinc::fprintf(f, "<user_keywords>\n");
    for (i=0; i<yes.size(); i++) {
        boinc::fprintf(f, "   <yes>%d</yes>\n", yes[i]);
    }
    for (i=0; i<no.size(); i++) {
        boinc::fprintf(f, "   <no>%d</no>\n", no[i]);
    }
    boinc::fprintf(f, "</user_keywords>\n");
}

// parse space-separated string into vector
//
void JOB_KEYWORD_IDS::parse_str(char* p) {
    stringstream ss(p);
    string token;
    while (getline(ss, token, ' ')) {
        ids.push_back(atoi(token.c_str()));
    }
}

#ifndef _USING_FCGI_

// write list of full keywords
//
void JOB_KEYWORD_IDS::write_xml_text(MIOFILE& mf, KEYWORDS& k) {
    mf.printf("<job_keywords>\n");
    for (unsigned int i=0; i<ids.size(); i++) {
        int id = ids[i];
        k.get(id).write_xml(mf);
    }
    mf.printf("</job_keywords>\n");
}

// write 1-line list of keyword IDs
//
void JOB_KEYWORD_IDS::write_xml_num(MIOFILE& out) {
    bool first = true;
    out.printf("    <job_keyword_ids>");
    for (unsigned int i=0; i<ids.size(); i++) {
        if (first) {
            out.printf("%d", ids[i]);
            first = false;
        } else {
            out.printf(", %d", ids[i]);
        }
    }
    out.printf("</job_keyword_ids>\n");
}
#endif

int JOB_KEYWORDS::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/job_keywords")) {
            return 0;
        }
        if (xp.match_tag("keyword")) {
            KEYWORD kw;
            int retval = kw.parse(xp);
            if (retval) return retval;
            keywords.push_back(kw);
        }
    }
    return ERR_XML_PARSE;
}
