// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"

#include "account.h"

int CLIENT_STATE::parse_account_files() {
    DIRREF dir;
    char name[256];
    PROJECT* project;
    FILE* f;

    dir = dir_open(".");
    while (dir_scan(name, dir, sizeof(name)) == 0) {
        if (is_account_file(name)) {
            f = fopen(name, "r");
            if (!f) continue;
            project = new PROJECT;
            project->parse_account(f);
            projects.push_back(project);
            fclose(f);
        }
    }
    dir_close(dir);
    return 0;
}

int CLIENT_STATE::add_project(char* master_url, char* authenticator) {
    char path[256];
    PROJECT* project;
    FILE* f;
    int retval;

    // check if this project is already running
    //
    if (lookup_project(master_url)) return -1;

    // create project state
    //
    project = new PROJECT;
    strcpy(project->master_url, master_url);
    strcpy(project->authenticator, authenticator);
    project->tentative = true;
    retval = project->write_account_file();
    if (retval) return retval;

    get_account_filename(master_url, path);
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

#if 0
// Wrong approach.
// The user must detach and reattach.
// In any case the following has a major bug:
// the call to remove_project_dir() won't work because
// it gets the new, not the old, project URL
// 
int CLIENT_STATE::change_project(
    int index, char* master_url, char* authenticator
) {
    char path[256];
    PROJECT* project;
    FILE* f;
    int retval;

    // check if this project is already running
    //
    if (lookup_project(master_url)) return -1;

    // delete old account file
    //
    project = projects[index];
    get_account_filename(project->master_url, path);
    retval = file_delete(path);

    // create project state
    //
    retval = write_account_file(master_url, authenticator);
    if (retval) return retval;
    get_account_filename(master_url, path);
    f = fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = project->parse_account(f);
    fclose(f);
    if (retval) return retval;

    // remove any old files
    retval = remove_project_dir(*project);

    retval = make_project_dir(*project);
    if(retval) {
        return retval;
    }
    return 0;
}

// TODO: see if any activities are in progress for this project, and stop them
//
int CLIENT_STATE::quit_project(PROJECT* project) {
    PROJECT* p;
    vector <PROJECT*>::iterator iter;
    char path[256];
    int retval;

    // find project and remove it from the vector
    //
    for (iter = projects.begin(); iter != projects.end(); iter++) {
        p = *iter;
        if (p == project) {
            projects.erase(iter);
            break;
        }
    }

    // delete account file
    //
    get_account_filename(project->master_url, path);
    retval = file_delete(path);

    // if tentative, there are no activities so we can safely delete everything
    //
    if (project->tentative) {
        // remove project directory and its contents
        //
        remove_project_dir(*project);
        delete project;
    } else {
#ifdef _WIN32
        AfxMessageBox("Please restart the client to complete the quit.", MB_OK, 0);
#endif
    }
    return 0;
}
#endif
