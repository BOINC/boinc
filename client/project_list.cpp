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

#include "client_msgs.h"
#include "project_list.h"

int PROJECT_LIST_ITEM::parse(XML_PARSER& xp, bool is_am) {
    clear();
    is_account_manager = is_am;
    const char* end_tag = is_am?"/account_manager":"/project";
    while (!xp.get_tag()) {
        if (xp.match_tag(end_tag)) {
            if (!id || !master_url.size() || !name.size()) {
                return ERR_XML_PARSE;
            }
            return 0;
        } else if (xp.parse_int("id", id)) {
            continue;
        } else if (xp.parse_string("url", master_url)) {
            continue;
        } else if (xp.parse_string("name", name)) {
            continue;
        } else if (xp.match_tag("platforms")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/platforms")) {
                    break;
                }
            }
        }
    }
    return ERR_XML_PARSE;
}

int PROJECT_LIST::read_file() {
    items.clear();
    FILE* f = fopen(ALL_PROJECTS_LIST_FILENAME, "r");
    if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    int retval;
    if (!xp.parse_start("projects")) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "missing start tag in project list file"
        );
        fclose(f);
        return ERR_XML_PARSE;
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/projects")) {
            fclose(f);
            return 0;
        } else if (xp.match_tag("project")) {
            PROJECT_LIST_ITEM p;
            retval = p.parse(xp, false);
            if (!retval) {
                items.push_back(p);
            }
            continue;
        } else if (xp.match_tag("account_manager")) {
            PROJECT_LIST_ITEM p;
            retval = p.parse(xp, true);
            if (!retval) {
                items.push_back(p);
            }
        }
    }
    fclose(f);
    return ERR_XML_PARSE;
}

PROJECT_LIST_ITEM* PROJECT_LIST::lookup(int id) {
    for (unsigned int i=0; i<items.size(); i++) {
        PROJECT_LIST_ITEM* p = &items[i];
        if (p->id == id) {
            return p;
        }
    }
    return NULL;
}
