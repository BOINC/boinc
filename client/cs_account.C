// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstdio>
#include <cassert>
#endif

#include "filesys.h"
#include "util.h"
#include "client_state.h"
#include "client_msgs.h"
#include "error_numbers.h"
#include "file_names.h"

using std::string;

int CLIENT_STATE::parse_account_files() {
    string name;
    PROJECT* project;
    FILE* f;
    int retval;

    DirScanner dir(".");
    while (dir.scan(name)) {
        if (is_account_file((char*)name.c_str())) {
            f = fopen(name.c_str(), "r");
            if (!f) continue;
            project = new PROJECT;
            retval = project->parse_account(f);
            if (retval) {
                msg_printf(NULL, MSG_ERROR,
                    "Couldn't parse account file %s",
                    name.c_str()
                );
            } else {
                projects.push_back(project);
            }
            fclose(f);
        }
    }
    return 0;
}

int CLIENT_STATE::add_project(const char* master_url, const char* _auth) {
    char path[256], canonical_master_url[256], auth[256];
    PROJECT* project;
    FILE* f;
    int retval;

    safe_strcpy(canonical_master_url, master_url);
    strip_whitespace(canonical_master_url);
    canonicalize_master_url(canonical_master_url);
    if (invalid_url(canonical_master_url)) {
        msg_printf(0, MSG_ERROR, "Invalid project URL: %s", canonical_master_url);
        return ERR_INVALID_URL;
    }

    safe_strcpy(auth, _auth);
    strip_whitespace(auth);
    if (!strlen(auth)) {
        msg_printf(0, MSG_ERROR, "Invalid account ID: %s", auth);
        return ERR_AUTHENTICATOR;
    }

    // check if this project is already running
    //
    if (lookup_project(canonical_master_url)) {
        msg_printf(0, MSG_ERROR, "Already attached to %s", canonical_master_url);
        return ERR_ALREADY_ATTACHED;
    }

    // create project state
    //
    project = new PROJECT;
    strcpy(project->master_url, canonical_master_url);
    strcpy(project->authenticator, auth);

    project->tentative = true;
    retval = project->write_account_file();
    if (retval) return retval;

    get_account_filename(canonical_master_url, path);
    f = fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = project->parse_account(f);
    fclose(f);
    if (retval) return retval;

    // remove any old files
    //
    retval = remove_project_dir(*project);

    retval = make_project_dir(*project);
    if (retval) return retval;
    projects.push_back(project);
    return 0;
}

int CLIENT_STATE::parse_preferences_for_user_files() {
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        projects[i]->parse_preferences_for_user_files();
    }
    return 0;
}
