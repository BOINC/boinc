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

#include <stdio.h>
#include <cassert>

#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "util.h"

static inline string filename_to_project_dirname(const string& filename) {
    assert(starts_with(filename, "account_"));
    assert(ends_with(filename, ".xml"));
    return string(PROJECTS_DIR) + PATH_SEPARATOR + filename.substr(8,filename.size()-12);
}

// we want to canonicalize filenames to NOT have a trailing _ . However, old
// clients <=1.03 did not ensure this so we have to rename them when someone
// upgrades.
// returns true if an error occurred.
//
static bool maybe_rename_old_filename_format(string& filename) {
    if (ends_with(filename, "_.xml")) {
        string newfilename = filename.substr(0, filename.length()-5) + ".xml";
        msg_printf(NULL, MSG_INFO, "Renaming %s to %s", filename.c_str(), newfilename.c_str());
        if (rename(filename.c_str(), newfilename.c_str())) {
            msg_printf(NULL, MSG_ERROR, "Couldn't rename %s to %s", filename.c_str(), newfilename.c_str());
            return true;
        }

        // also rename the project directory.
        string newproject_dir = filename_to_project_dirname(newfilename);
        string oldproject_dir = newproject_dir + '_';
        msg_printf(NULL, MSG_INFO, "Renaming %s to %s", oldproject_dir.c_str(), newproject_dir.c_str());
        if (rename(oldproject_dir.c_str(), newproject_dir.c_str())) {
            msg_printf(NULL, MSG_ERROR, "Couldn't rename %s to %s", oldproject_dir.c_str(), newproject_dir.c_str());
        }
        filename = newfilename;
    }
    return false;
}

int CLIENT_STATE::parse_account_files() {
    string name;
    PROJECT* project;
    FILE* f;

    DirScanner dir(".");
    while (dir.scan(name)) {
        if (is_account_file((char*)name.c_str())) {
            if (maybe_rename_old_filename_format(name)) {
                msg_printf(NULL, MSG_ERROR, "Warning: not adding project %s", name.c_str());
                continue;       // Error occurred renaming
            }
            f = fopen(name.c_str(), "r");
            if (!f) continue;
            project = new PROJECT;
            project->parse_account(f);
            projects.push_back(project);
            fclose(f);
        }
    }
    return 0;
}

int CLIENT_STATE::add_project(const char* master_url, const char* authenticator) {
    char path[256], canonical_master_url[256];
    PROJECT* project;
    FILE* f;
    int retval;

    safe_strcpy(canonical_master_url, master_url);
    canonicalize_master_url(canonical_master_url);

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
    strcpy(project->authenticator, authenticator);
    strip_whitespace(project->authenticator);

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
