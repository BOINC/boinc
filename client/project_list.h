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

// classes for reading and storing the all-projects list.
// Used for auto-login feature.

#ifndef BOINC_PROJECT_LIST_H
#define BOINC_PROJECT_LIST_H

#include <vector>
#include <string>

#include "parse.h"

// represents an entry in the list
// (either a project or an account manager)

struct PROJECT_LIST_ITEM {
    int id;
    bool is_account_manager;
    std::string master_url;
    std::string name;
    int parse(XML_PARSER&, bool is_am);
    void clear() {
        id = 0;
        master_url.clear();
        name.clear();
    }
};

struct PROJECT_LIST {
    std::vector<PROJECT_LIST_ITEM> items;
    int read_file();
    PROJECT_LIST_ITEM* lookup(int id);
};

#endif
