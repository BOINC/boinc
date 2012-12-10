// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "url.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "project.h"

using std::string;
using std::sort;

// write account_*.xml file.
// NOTE: this is called only when
// 1) attach to a project, and
// 2) after a scheduler RPC
// So in either case PROJECT.project_prefs
// (which normally is undefined) is valid
//
int PROJECT::write_account_file() {
    char path[MAXPATHLEN];
    FILE* f;
    int retval;

    get_account_filename(master_url, path);
    f = boinc_fopen(TEMP_ACCT_FILE_NAME, "w");
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
    fprintf(f, "<project_preferences>\n%s</project_preferences>\n",
        project_prefs.c_str()
    );
    fprintf(f, gui_urls.c_str());
    fprintf(f, "</account>\n");
    fclose(f);
    retval = boinc_rename(TEMP_ACCT_FILE_NAME, path);
    if (retval) return ERR_RENAME;
    return 0;
}

void handle_no_rsc_pref(PROJECT* p, const char* name) {
    int i = rsc_index(name);
    if (i < 0) return;
    p->no_rsc_pref[i] = true;
}

// parse an account_*.xml file, ignoring <venue> elements
// (since we don't know the host venue yet)
//
int PROJECT::parse_account(FILE* in) {
    char buf2[256];
    int retval;
    bool in_project_prefs = false, btemp;
    for (int i=0; i<coprocs.n_rsc; i++) {
        no_rsc_pref[i] = false;
    }
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(in);

    strcpy(master_url, "");
    strcpy(authenticator, "");
    while (!xp.get_tag()) {
        if (xp.match_tag("account")) continue;
        if (xp.match_tag("project_preferences")) {
            in_project_prefs = true;
            continue;
        }
        if (xp.match_tag("/project_preferences")) {
            in_project_prefs = false;
            continue;
        }
        if (xp.match_tag("/account")) {
            return 0;
        } else if (xp.match_tag("venue")) {
            std::string devnull;
            retval = copy_element_contents(xp.f->f, "</venue>", devnull);
            if (retval) return retval;
            continue;
        } else if (xp.parse_str("master_url", master_url, sizeof(master_url))) {
            canonicalize_master_url(master_url);
            continue;
        } else if (xp.parse_str("authenticator", authenticator, sizeof(authenticator))) continue;
        else if (xp.parse_double("resource_share", resource_share)) continue;
        else if (xp.parse_bool("no_cpu", btemp)) {
            if (btemp) handle_no_rsc_pref(this, "CPU");
            continue;
        }

        // deprecated
        else if (xp.parse_bool("no_cuda", btemp)) {
            if (btemp) handle_no_rsc_pref(this, GPU_TYPE_NVIDIA);
            continue;
        }
        else if (xp.parse_bool("no_ati", btemp)) {
            if (btemp) handle_no_rsc_pref(this, GPU_TYPE_ATI);
            continue;
        }

        else if (xp.parse_str("no_rsc", buf2, sizeof(buf2))) {
            handle_no_rsc_pref(this, buf2);
            continue;
        }
        else if (xp.parse_str("project_name", project_name, sizeof(project_name))) continue;
        else if (xp.match_tag("gui_urls")) {
            string foo;
            retval = copy_element_contents(xp.f->f, "</gui_urls>", foo);
            if (retval) return retval;
            gui_urls = "<gui_urls>\n"+foo+"</gui_urls>\n";
            continue;
        } else if (xp.match_tag("project_specific")) {
            retval = copy_element_contents(
                xp.f->f,
                "</project_specific>",
                project_specific_prefs
            );
            if (retval) return retval;
            continue;
        } else {
            // don't show unparsed XML errors if we're in project prefs
            //
            if (!in_project_prefs && log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] PROJECT::parse_account(): unrecognized: %s\n",
                    xp.parsed_tag
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

// scan an account_*.xml file, looking for a <venue> element
// that matches this host's venue,
// and parsing that for resource share and prefs.
// Call this only after client_state.xml has been read
// (so that we know the host venue)
//
int PROJECT::parse_account_file_venue() {
    char attr_buf[256], venue[256], path[MAXPATHLEN], buf2[256];
    int retval;
    bool in_right_venue = false, btemp;

    get_account_filename(master_url, path);
    FILE* in = boinc_fopen(path, "r");
    if (!in) return ERR_FOPEN;

    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(in);
    while (!xp.get_tag(attr_buf, sizeof(attr_buf))) {
        if (xp.match_tag("/account")) {
            fclose(in);
            return 0;
        } else if (xp.match_tag("venue")) {
            parse_attr(attr_buf, "name", venue, sizeof(venue));
            if (!strcmp(venue, host_venue)) {
                using_venue_specific_prefs = true;
                in_right_venue = true;

                // reset these
                //
                for (int i=0; i<coprocs.n_rsc; i++) {
                    no_rsc_pref[i] = false;
                }
            } else {
                std::string devnull;
                retval = copy_element_contents(in, "</venue>", devnull);
                if (retval) return retval;
            }
            continue;
        }
        if (!in_right_venue) continue;
        if (xp.match_tag("/venue")) {
            in_right_venue = false;
            continue;
        } else if (xp.match_tag("project_specific")) {
            retval = copy_element_contents(
                xp.f->f,
                "</project_specific>",
                project_specific_prefs
            );
            if (retval) return retval;
            continue;
        } else if (xp.parse_double("resource_share", resource_share)) {
            continue;
        }
        else if (xp.parse_bool("no_cpu", btemp)) {
            if (btemp) handle_no_rsc_pref(this, "CPU");
            continue;
        }

        // deprecated syntax
        else if (xp.parse_bool("no_cuda", btemp)) {
            if (btemp) handle_no_rsc_pref(this, GPU_TYPE_NVIDIA);
            continue;
        }
        else if (xp.parse_bool("no_ati", btemp)) {
            if (btemp) handle_no_rsc_pref(this, GPU_TYPE_ATI);
            continue;
        }

        else if (xp.parse_str("no_rsc", buf2, sizeof(buf2))) {
            handle_no_rsc_pref(this, buf2);
            continue;
        }
        else {
            // skip project preferences the client doesn't know about
            //
            xp.skip_unexpected();
        }
    }
    fclose(in);
    return ERR_XML_PARSE;
}

int PROJECT::parse_account_file() {
    char path[MAXPATHLEN];
    int retval;
    FILE* f;

    get_account_filename(master_url, path);
    f = boinc_fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = parse_account(f);
    fclose(f);
    return retval;
}

int CLIENT_STATE::parse_account_files_venue() {
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (strlen(p->host_venue)) {
            p->parse_account_file_venue();
        }
    }
    return 0;
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
        project->sched_rpc_pending = RPC_REASON_INIT;
        retval = project->parse_account(f);
        fclose(f);
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Couldn't parse account file %s", name.c_str()
            );
            delete project;
        } else {
            if (lookup_project(project->master_url)) {
                msg_printf(project, MSG_INFO,
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
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(in);

    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/daily_statistics")) {
            if (day == 0) return ERR_XML_PARSE;
            return 0;
        }
        else if (xp.parse_double("day", day)) continue;
        else if (xp.parse_double("user_total_credit", user_total_credit)) continue;
        else if (xp.parse_double("user_expavg_credit", user_expavg_credit)) continue;
        else if (xp.parse_double("host_total_credit", host_total_credit)) continue;
        else if (xp.parse_double("host_expavg_credit", host_expavg_credit)) continue;
    }
    return ERR_XML_PARSE;
}

bool operator <  (const DAILY_STATS& x1, const DAILY_STATS& x2) {
    return (x1.day < x2.day);
}

// parse an statistics_*.xml file
//
int PROJECT::parse_statistics(FILE* in) {
    int retval;

    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(in);

    while (!xp.get_tag()) {
        if (xp.match_tag("/project_statistics")) {
            sort(statistics.begin(), statistics.end());
            return 0;
        }
        if (xp.match_tag("project_statistics")) continue;
        if (xp.match_tag("daily_statistics")) {
            DAILY_STATS daily_stats;
            retval = daily_stats.parse(in);
            if (!retval) {
                statistics.push_back(daily_stats);
            }
            continue;
        }
        if (xp.parse_str("master_url", master_url, sizeof(master_url))) {
            canonicalize_master_url(master_url);
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] PROJECT::parse_statistics(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
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
        PROJECT temp;
        if (is_statistics_file(name.c_str())) {
            f = boinc_fopen(name.c_str(), "r");
            if (!f) continue;
            retval = temp.parse_statistics(f);
            fclose(f);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Couldn't parse %s", name.c_str()
                );
            } else {
                project = lookup_project(temp.master_url);
                if (project == NULL) {
                    msg_printf(NULL, MSG_INFO,
                        "Project for %s not found - ignoring",
                        name.c_str()
                    );
                } else {
                    for (std::vector<DAILY_STATS>::const_iterator i=temp.statistics.begin();
                        i!=temp.statistics.end(); i++
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
    char path[MAXPATHLEN];
    FILE* f;
    int retval;

    get_statistics_filename(master_url, path);
    f = boinc_fopen(TEMP_STATS_FILE_NAME, "w");
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
    retval = boinc_rename(TEMP_STATS_FILE_NAME, path);
    if (retval) return ERR_RENAME;
    return 0;
}

int CLIENT_STATE::add_project(
    const char* master_url, const char* _auth, const char* project_name,
    bool attached_via_acct_mgr
) {
    char path[MAXPATHLEN], canonical_master_url[256], auth[256], dir[256];
    PROJECT* project;
    FILE* f;
    int retval;

    if (config.disallow_attach) {
        return ERR_USER_PERMISSION;
    }

    safe_strcpy(canonical_master_url, master_url);
    strip_whitespace(canonical_master_url);
    canonicalize_master_url(canonical_master_url);
    if (!valid_master_url(canonical_master_url)) {
        msg_printf(0, MSG_INFO, "Invalid URL: %s", canonical_master_url);
        return ERR_INVALID_URL;
    }

    safe_strcpy(auth, _auth);
    strip_whitespace(auth);
    if (!strlen(auth)) {
        msg_printf(0, MSG_INFO, "Missing account key");
        return ERR_AUTHENTICATOR;
    }

    // check if we're already attached to this project
    //
    if (lookup_project(canonical_master_url)) {
        msg_printf(0, MSG_INFO, "Already attached to %s", canonical_master_url);
        return ERR_ALREADY_ATTACHED;
    }

    // create project state
    //
    project = new PROJECT;
    strcpy(project->master_url, canonical_master_url);
    strcpy(project->authenticator, auth);
    strcpy(project->project_name, project_name);
    project->attached_via_acct_mgr = attached_via_acct_mgr;

    retval = project->write_account_file();
    if (retval) return retval;

    get_account_filename(canonical_master_url, path);
    f = boinc_fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = project->parse_account(f);
    fclose(f);
    if (retval) return retval;

    // remove any old files
    // (unless PROJECT/app_info.xml is found, so that
    // people using anonymous platform don't have to get apps again)
    //
    get_project_dir(project, dir, sizeof(dir));
    sprintf(path, "%s/%s", dir, APP_INFO_FILE_NAME);
    if (boinc_file_exists(path)) {
        project->anonymous_platform = true;
        f = fopen(path, "r");
        if (f) {
            parse_app_info(project, f);
            fclose(f);
        }
    } else {
        retval = remove_project_dir(*project);
    }

    retval = make_project_dir(*project);
    if (retval) return retval;
    projects.push_back(project);
    project->sched_rpc_pending = RPC_REASON_INIT;
    set_client_state_dirty("Add project");
    return 0;
}

int CLIENT_STATE::parse_preferences_for_user_files() {
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        projects[i]->parse_preferences_for_user_files();
    }
    return 0;
}

