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

// utility classes for keywords

#include <vector>
#include "parse.h"

struct USER_KEYWORDS {
    std::vector<int> yes;
    std::vector<int> no;
    int parse(XML_PARSER&);
    void clear() {
        yes.clear();
        no.clear();
    }
    void write(FILE*);
    bool empty() {
        return yes.empty() && no.empty();
    }
};

struct JOB_KEYWORDS {
    std::vector<int> ids;
    void parse_str(char*);
        // parse space-separated list
};

extern double keyword_score(USER_KEYWORDS&, JOB_KEYWORDS&);
