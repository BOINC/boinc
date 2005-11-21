// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cassert>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
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
    int retval;
    bool got_venue_prefs = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(master_url, "");
    strcpy(host_venue, "");
    strcpy(authenticator, "");
    using_venue_specific_prefs = false;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<account>")) continue;
        if (match_tag(buf, "<project_preferences>")) continue;
        if (match_tag(buf, "</project_preferences>")) continue;
        if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) continue;
        if (match_tag(buf, "</account>")) {
            if (strlen(host_venue)) {
                if (got_venue_prefs) {
                    using_venue_specific_prefs = true;
                }
            }
            return 0;
        }
        else if (match_tag(buf, "<venue")) {
            parse_attr(buf, "name", venue, sizeof(venue));
            if (!strcmp(venue, host_venue)) {
                got_venue_prefs = true;
            } else {
                std::string devnull;
                retval = copy_element_contents(in, "</venue>", devnull);
                if (retval) return retval;
            }
            continue;
        }
        else if (match_tag(buf, "</venue>")) continue;

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
    f = boinc_fopen(path, "r");
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
        if (!is_file(name.c_str())) continue;
        if (!is_account_file(name.c_str())) continue;

        f = boinc_fopen(name.c_str(), "r");
        if (!f) continue;
        project = new PROJECT;

        // Assume master_url_fetch_pending, sched_rpc_pending are
        // true until we read client_state.xml
        //
        project->master_url_fetch_pending = true;
        project->sched_rpc_pending = true;
        retval = project->parse_account(f);
        fclose(f);
        if (retval) {
            msg_printf(NULL, MSG_ERROR,
                "Couldn't parse account file %s", name.c_str()
            );
            delete project;
        } else {
            if (lookup_project(project->master_url)) {
                msg_printf(NULL, MSG_ERROR,
                    "Duplicate account file %s - ignoring", name.c_str()
                );
                delete project;
            } else {
                projects.push_back(project);
            }
        }
    }
    return 0;
}

void DAILY_STATS::clear() {
    memset(this, 0, sizeof(DAILY_STATS));
}

int DAILY_STATS::parse(FILE* in) {
    char buf[256];
    clear();
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</daily_statistics>")) return 0;
        else if (parse_double(buf, "<day>", day)) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
    }
    return ERR_XML_PARSE;
}

// parse an statistics_*.xml file
//
int PROJECT::parse_statistics(FILE* in) {
    int retval;
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</project_statistics>")) return 0;
        else if (match_tag(buf, "<project_statistics>")) continue;
        else if (match_tag(buf, "<daily_statistics>")) {
            DAILY_STATS daily_stats;
            retval = daily_stats.parse(in);
            if (retval) return retval;
            statistics.push_back(daily_stats);
            continue;
        }
        else if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) {
            canonicalize_master_url(master_url);
            continue;
        }
        else scope_messages.printf("PROJECT::parse_statistics(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int CLIENT_STATE::parse_statistics_files() {
    string name;
    PROJECT* project;
    FILE* f;
    int retval;

    DirScanner dir(".");
    while (dir.scan(name)) {
        if (is_statistics_file(name.c_str())) {
            f = boinc_fopen(name.c_str(), "r");
            if (!f) continue;
            PROJECT* temp = new PROJECT;
            retval = temp->parse_statistics(f);
            fclose(f);
            if (retval) {
                msg_printf(NULL, MSG_ERROR,
                    "Couldn't parse statistics file %s", name.c_str()
                );
            } else {
                project=lookup_project(temp->master_url);
                if (project==NULL) {
                    msg_printf(NULL, MSG_ERROR,
                        "Project for statistic file %s not found - ignoring", name.c_str()
                    );
                } else {
                    for (std::vector<DAILY_STATS>::const_iterator i=temp->statistics.begin();
                        i!=temp->statistics.end(); ++i
                    ) {
                        project->statistics.push_back(*i);
                    }
                }
            }
        }
    }
    return 0;
}

int PROJECT::write_statistics_file() {
    char path[256];
    FILE* f;
    int retval;

    get_statistics_filename(master_url, path);
    f = boinc_fopen(TEMP_FILE_NAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f, 
        "<project_statistics>\n"
        "    <master_url>%s</master_url>\n",
        master_url
    );

    for (std::vector<DAILY_STATS>::iterator i=statistics.begin();
        i!=statistics.end(); ++i
    ) {
        fprintf(f, 
            "    <daily_statistics>\n"
            "        <day>%f</day>\n"
            "        <user_total_credit>%f</user_total_credit>\n"
            "        <user_expavg_credit>%f</user_expavg_credit>\n"
            "        <host_total_credit>%f</host_total_credit>\n"
            "        <host_expavg_credit>%f</host_expavg_credit>\n"
            "    </daily_statistics>\n",
            i->day,
            i->user_total_credit,
            i->user_expavg_credit,
            i->host_total_credit,
            i->host_expavg_credit
        );
    }

    fprintf(f, 
        "</project_statistics>\n"
    );

    fclose(f);
    retval = boinc_rename(TEMP_FILE_NAME, path);
    if (retval) return ERR_RENAME;
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
    if (!valid_master_url(canonical_master_url)) {
        msg_printf(0, MSG_ERROR, "Invalid URL: %s", canonical_master_url);
        return ERR_INVALID_URL;
    }

    safe_strcpy(auth, _auth);
    strip_whitespace(auth);
    if (!strlen(auth)) {
        msg_printf(0, MSG_ERROR, "Missing account key");
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
    gstate.have_tentative_project = true;
    retval = project->write_account_file();
    if (retval) return retval;

    get_account_filename(canonical_master_url, path);
    f = boinc_fopen(path, "r");
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
    project->sched_rpc_pending = true;
    set_client_state_dirty("Add project");
    return 0;
}

// called when the client fails to attach to a project
//
void PROJECT::attach_failed(int error_num) {
    gstate.project_attach.error_num = error_num;
    gstate.have_tentative_project = false;
    switch(error_num){
    case ERR_ATTACH_FAIL_INIT:
        msg_printf(this, MSG_ERROR,
            "Couldn't connect to URL %s.\n"
            "Please check URL.",
            master_url
        );
        break;
    case ERR_ATTACH_FAIL_DOWNLOAD:
        msg_printf(this, MSG_ERROR,
            "Couldn't access URL %s.\n"
            "The project's servers may be down,\n"
            "in which case please try again later.",
            master_url
        );
        break;
    case ERR_ATTACH_FAIL_PARSE:
        msg_printf(this, MSG_ERROR,
            "The page at %s contains no BOINC information.\n"
            "It may not be the URL of a BOINC project.\n"
            "Please check the URL and try again.",
            master_url
        );
        break;
    case ERR_ATTACH_FAIL_BAD_KEY:
        msg_printf(this, MSG_ERROR,
            "The account key you provided for %s\n"
            "was not recognized as a valid account key.\n"
            "Please check the account key and try again.",
            master_url
        );
        break;
    case ERR_ATTACH_FAIL_FILE_WRITE:
        msg_printf(this, MSG_ERROR,
            "BOINC was unable to create an account file for %s on your disk.\n"
            "Please check file system permissions and try again.\n",
            master_url
        );
        break;
    }
    gstate.detach_project(this);
}

int CLIENT_STATE::parse_preferences_for_user_files() {
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        projects[i]->parse_preferences_for_user_files();
    }
    return 0;
}

const char *BOINC_RCSID_497223a3f8 = "$Id$";
