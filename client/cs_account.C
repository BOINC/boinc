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
#include "parse.h"
#include "util.h"
#include "client_state.h"
#include "client_msgs.h"
#include "error_numbers.h"
#include "file_names.h"

using std::string;

// write account_*.xml file.
// NOTE: this is called only when
// 1) attach to a project, and
// 2) after a scheduler RPC
// So in either case PROJECT.project_prefs
// (which normally is undefined) is valid
//
int PROJECT::write_account_file() {
    char path[256];
    FILE* f;
    int retval;

    get_account_filename(master_url, path);
    f = boinc_fopen(TEMP_FILE_NAME, "w");
    if (!f) return ERR_FOPEN;
#ifndef _WIN32
    chmod(TEMP_FILE_NAME, 0600);
#endif

    fprintf(f,
        "<account>\n"
        "    <master_url>%s</master_url>\n"
        "    <authenticator>%s</authenticator>\n",
        master_url,
        authenticator
    );
    // put project name in account file for informational purposes only
    // (client state file is authoritative)
    //
    if (strlen(project_name)) {
        fprintf(f, "    <project_name>%s</project_name>\n", project_name);
    }
    if (tentative) {
        fprintf(f, "    <tentative/>\n");
    }
    if (strlen(host_venue)) {
        fprintf(f, "    <host_venue>%s</host_venue>\n", host_venue);
    }
    fprintf(f, "<project_preferences>\n%s</project_preferences>\n",
        project_prefs.c_str()
    );
    fprintf(f, gui_urls.c_str());
    fprintf(f, "</account>\n");
    fclose(f);
    retval = boinc_rename(TEMP_FILE_NAME, path);
    if (retval) return ERR_RENAME;
    return 0;
}

// parse an account_*.xml file
//
int PROJECT::parse_account(FILE* in) {
    char buf[256], venue[256];
    char temp[MAX_BLOB_LEN];
    int retval;
    bool got_venue_prefs = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    // Assume master_url_fetch_pending, sched_rpc_pending are
    // true until we read client_state.xml
    //
    master_url_fetch_pending = true;
    sched_rpc_pending = true;
    strcpy(master_url, "");
    strcpy(host_venue, "");
    strcpy(authenticator, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<account>")) continue;
        if (match_tag(buf, "<project_preferences>")) continue;
        if (match_tag(buf, "</project_preferences>")) continue;
        if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) continue;
        if (match_tag(buf, "</account>")) {
            if (strlen(host_venue)) {
                msg_printf(this, MSG_INFO, "Host location: %s", host_venue);
                if (got_venue_prefs) {
                    msg_printf(this, MSG_INFO, "Using separate project prefs for %s", host_venue);
                } else {
                    msg_printf(this, MSG_INFO, "Using your default project prefs");
                }
            } else {
                msg_printf(this, MSG_INFO, "Using your default project prefs");
            }
            return 0;
        }

        else if (match_tag(buf, "<venue")) {
            parse_attr(buf, "name", venue, sizeof(venue));
            if (!strcmp(venue, host_venue)) {
                got_venue_prefs = true;
            } else {
                retval = copy_element_contents(
                    in, "</venue>", temp, sizeof(temp)
                );
                if (retval) return retval;
            }
            continue;
        }

        else if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) {
            canonicalize_master_url(master_url);
            continue;
        }
        else if (parse_str(buf, "<authenticator>", authenticator, sizeof(authenticator))) continue;
        else if (parse_double(buf, "<resource_share>", resource_share)) continue;
        else if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) continue;
        else if (match_tag(buf, "<tentative/>")) {
            tentative = true;
            continue;
        }
#if 0
        else if (match_tag(buf, "<deletion_policy_priority/>")) {
            deletion_policy_priority = true;
            continue;
        }
        else if (match_tag(buf, "<deletion_policy_expire>")) {
            deletion_policy_expire = true;
            continue;
        }
#endif
        else if (match_tag(buf, "<gui_urls>")) {
            string foo;
            retval = copy_element_contents(in, "</gui_urls>", foo);
            if (retval) return retval;
            gui_urls = "<gui_urls>\n"+foo+"</gui_urls>\n";
            continue;
        }
        else if (match_tag(buf, "<project_specific>")) {
            retval = copy_element_contents(
                in,
                "</project_specific>",
                project_specific_prefs
            );
            if (retval) return retval;
            continue;
        }
        else scope_messages.printf("PROJECT::parse_account(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int PROJECT::parse_account_file() {
    char path[256];
    int retval;
    FILE* f;

    get_account_filename(master_url, path);
    f = fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = parse_account(f);
    fclose(f);
    return retval;
}

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
