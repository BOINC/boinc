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

// utility classes for keywords

#ifndef BOINC_KEYWORD_H
#define BOINC_KEYWORD_H

#include <vector>
#include <map>
#include "parse.h"

// a keyword
//
struct KEYWORD {
    int id;
    std::string name;
    std::string description;
    int parent;
    int level;
    int category;

    void write_xml(MIOFILE&);
    int parse(XML_PARSER&);
};

// the set of all keywords
//
struct KEYWORDS {
    std::map<int, KEYWORD> keywords;
    bool present;
    KEYWORDS() {
        present = false;
    }
    int parse(XML_PARSER&);
    inline KEYWORD& get(int id) {return keywords[id];}
};

// a user's keyword preferences
//
struct USER_KEYWORDS {
    std::vector<int> yes;
    std::vector<int> no;
    int parse(XML_PARSER&);
    inline void clear() {
        yes.clear();
        no.clear();
    }
    void write(FILE*);
    inline bool empty() {
        return yes.empty() && no.empty();
    }
};

// the keywords IDs associated with a job (workunit)
//
struct JOB_KEYWORD_IDS {
    std::vector<int> ids;
    void parse_str(char*);
        // parse space-separated list
    inline bool empty() {
        return ids.empty();
    }
    inline void clear() {
        ids.clear();
    }
    void write_xml_text(MIOFILE&, KEYWORDS&);
    void write_xml_num(MIOFILE&);
};

// same, but the entire keyword objects (for GUI RPC client)
//
struct JOB_KEYWORDS {
    std::vector<KEYWORD> keywords;
    inline bool empty() {
        return keywords.empty();
    }
    inline void clear() {
        keywords.clear();
    }
    int parse(XML_PARSER&);
};


#endif
