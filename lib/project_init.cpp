// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "url.h"
#include "parse.h"
#include "str_replace.h"

#include "project_init.h"

void PROJECT_INIT::clear() {
    safe_strcpy(url, "");
    safe_strcpy(name, "");
    safe_strcpy(account_key, "");
    safe_strcpy(team_name, "");
    safe_strcpy(setup_cookie, "");
    embedded = false;
}

PROJECT_INIT::PROJECT_INIT() {
    clear();
}

int PROJECT_INIT::init() {
    clear();
    FILE* f = fopen(PROJECT_INIT_FILENAME, "r");
    if (!f) return 0;

    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
        if (xp.match_tag("/project_init")) break;
        else if (xp.parse_str("name", name, sizeof(name))) continue;
        else if (xp.parse_str("team_name", team_name, sizeof(team_name))) continue;
        else if (xp.parse_str("account_key", account_key, sizeof(account_key))) continue;
        else if (xp.parse_str("setup_cookie", setup_cookie, sizeof(setup_cookie))) continue;
        else if (xp.parse_str("url", url, sizeof(url))) {
            canonicalize_master_url(url, sizeof(url));
            continue;
        }
        else if (xp.parse_bool("embedded", embedded)) continue;
    }
    fclose(f);

	return 0;
}

int PROJECT_INIT::remove() {
    clear();
    return boinc_delete_file(PROJECT_INIT_FILENAME);
}

int PROJECT_INIT::write() {
    FILE* f = fopen(PROJECT_INIT_FILENAME, "w");
    if (!f) return ERR_FOPEN;

    fprintf(f,
        "<project_init>\n"
        "  <url>%s</url>\n"
        "  <name>%s</name>\n"
        "  <team_name>%s</team_name>\n"
        "  <account_key>%s</account_key>\n"
        "  <setup_cookie>%s</setup_cookie>\n"
        "  <embedded>%d</embedded>\n"
        "</project_init>\n",
        url,
        name,
        team_name,
        account_key,
        setup_cookie,
        embedded
    );
    fclose(f);

    return 0;
}

