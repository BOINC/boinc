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

#include "error_numbers.h"
#include "file_names.h"
#include "hostinfo.h"
#include "log_flags.h"
#include "parse.h"

#include "client_state.h"

CLIENT_STATE gstate;

CLIENT_STATE::CLIENT_STATE() {
    net_xfers = new NET_XFER_SET;
    http_ops = new HTTP_OP_SET(net_xfers);
    file_xfers = new FILE_XFER_SET(http_ops);
    client_state_dirty = false;
    exit_when_idle = false;
    contacted_sched_server = false;
    activities_suspended = false;
}

int CLIENT_STATE::init(ACCOUNTS& accounts) {
    nslots = 1;
    PROJECT* p1, *p2;
    unsigned int i;

    parse_state_file();

    // copy projects from the login file to the state.
    //

    for (i=0; i<accounts.projects.size(); i++) {
        p1 = accounts.projects[i];
        p2 = lookup_project(p1->domain);
        if (p2) {
            if (strcmp(p1->email_addr, p2->email_addr)
                || strcmp(p1->authenticator, p2->authenticator)
            ) {
                fprintf(stderr,
                    "Your account file doesn't match your client state file.\n"
                    "You should probably delete the client state file.\n"
                );
            }
        } else {
            p2 = new PROJECT;
            *p2 = *p1;
            projects.push_back(p2);
        }
    }

    // Error out if there are any projects in the state file
    // not in the project file.
    //
    // TODO

    print_counts();
    make_project_dirs();
    make_slot_dirs();

    // TODO: only get host info every so often
    //
    get_host_info(host_info);
    return 0;
}

// See if (on the basis of user prefs) we should suspend activities.
// If so, suspend tasks
//
int CLIENT_STATE::check_suspend_activities() {
    bool should_suspend = false;
    if (prefs.stop_on_batteries && host_is_running_on_batteries()) {
        should_suspend = true;
    }

    if (should_suspend) {
        if (!activities_suspended) {
            if (log_flags.task_debug) printf("SUSPENDING ACTIVITIES\n");
            active_tasks.suspend_all();
        }
    } else {
        if (activities_suspended) {
            if (log_flags.task_debug) printf("UNSUSPENDING ACTIVITIES\n");
            active_tasks.unsuspend_all();
        }
    }
    activities_suspended = should_suspend;
    return 0;
}

// return true if something happened
//
bool CLIENT_STATE::do_something() {
    int nbytes;
    bool action = false;

    check_suspend_activities();
    if (!activities_suspended) {
        net_xfers->poll(999999, nbytes);
        if (nbytes) action = true;
        action |= http_ops->poll();
        action |= file_xfers->poll();
        action |= active_tasks.poll();
        action |= get_work();
        action |= garbage_collect();
        action |= start_apps();
        action |= handle_running_apps();
        action |= start_file_xfers();
        write_state_file_if_needed();
    }
    if (!action) time_stats.update(true, !activities_suspended);
    return action;
}

int CLIENT_STATE::parse_state_file() {
    char buf[256];
    FILE* f = fopen(STATE_FILE_NAME, "r");
    PROJECT* project;
    int retval;

    if (!f) {
        if (log_flags.state_debug) {
            printf("can't open state file: %s\n", STATE_FILE_NAME);
        }
        return ERR_FOPEN;
    }
    fgets(buf, 256, f);
    if (!match_tag(buf, "<client_state>")) return -1;
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            return 0;
        } else if (match_tag(buf, "<project>")) {
            project = new PROJECT;
            project->parse(f);
            projects.push_back(project);
        } else if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            app->parse(f);
            retval = link_app(project, app);
            if (!retval) apps.push_back(app);
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO* fip = new FILE_INFO;
            fip->parse(f);
            retval = link_file_info(project, fip);
            if (!retval) file_infos.push_back(fip);
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION* avp = new APP_VERSION;
            avp->parse(f);
            retval = link_app_version(project, avp);
            if (!retval) app_versions.push_back(avp);
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wup = new WORKUNIT;
            wup->parse(f);
            retval = link_workunit(project, wup);
            if (!retval) workunits.push_back(wup);
        } else if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT;
            rp->parse(f, "</result>");
            retval = link_result(project, rp);
            if (!retval) results.push_back(rp);
        } else if (match_tag(buf, "<host_info>")) {
            host_info.parse(f);
        } else if (match_tag(buf, "<prefs>")) {
            prefs.parse(f);
        } else if (match_tag(buf, "<time_stats>")) {
            time_stats.parse(f);
        } else if (match_tag(buf, "<net_stats>")) {
            net_stats.parse(f);
        } else if (match_tag(buf, "<active_task_set>")) {
            active_tasks.parse(f, this);
        } else {
            fprintf(stderr, "CLIENT_STATE::parse_state_file: unrecognized: %s\n", buf);
        }
    }
    return ERR_XML_PARSE;
}

int CLIENT_STATE::make_project_dirs() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        make_project_dir(*projects[i]);
    }
    return 0;
}

int CLIENT_STATE::make_slot_dirs() {
    unsigned int i;
    for (i=0; i<nslots; i++) {
        make_slot_dir(i);
    }
    return 0;
}

int CLIENT_STATE::write_state_file() {
    unsigned int i, j;
    FILE* f = fopen(STATE_FILE_TEMP, "w");
    int retval;

    if (!f) {
        fprintf(stderr, "can't open temp state file: %s\n", STATE_FILE_TEMP);
        return ERR_FOPEN;
    }
    fprintf(f, "<client_state>\n");
    host_info.write(f);
    prefs.write(f);
    time_stats.write(f, false);
    net_stats.write(f, false);
    for (j=0; j<projects.size(); j++) {
        PROJECT* p = projects[j];
        p->write(f);
        for (i=0; i<apps.size(); i++) {
            if (apps[i]->project == p) apps[i]->write(f);
        }
        for (i=0; i<file_infos.size(); i++) {
            if (file_infos[i]->project == p) {
                file_infos[i]->write(f, false);
            }
        }
        for (i=0; i<app_versions.size(); i++) {
            if (app_versions[i]->project == p) app_versions[i]->write(f);
        }
        for (i=0; i<workunits.size(); i++) {
            if (workunits[i]->project == p) workunits[i]->write(f);
        }
        for (i=0; i<results.size(); i++) {
            if (results[i]->project == p) results[i]->write(f, false);
        }
    }
    active_tasks.write(f);
    fprintf(f, "</client_state>\n");
    fclose(f);
    retval = rename(STATE_FILE_TEMP, STATE_FILE_NAME);
    if (retval) return ERR_RENAME;
    return 0;
}

PROJECT* CLIENT_STATE::lookup_project(char* domain) {
    for (unsigned int i=0; i<projects.size(); i++) {
        if (!strcmp(domain, projects[i]->domain)) {
            return projects[i];
        }
    }
    return 0;
}

APP* CLIENT_STATE::lookup_app(PROJECT* p, char* name) {
    for (unsigned int i=0; i<apps.size(); i++) {
        APP* app = apps[i];
        if (app->project == p && !strcmp(name, app->name)) return app;
    }
    return 0;
}

RESULT* CLIENT_STATE::lookup_result(PROJECT* p, char* name) {
    for (unsigned int i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->project == p && !strcmp(name, rp->name)) return rp;
    }
    return 0;
}

WORKUNIT* CLIENT_STATE::lookup_workunit(PROJECT* p, char* name) {
    for (unsigned int i=0; i<workunits.size(); i++) {
        WORKUNIT* wup = workunits[i];
        if (wup->project == p && !strcmp(name, wup->name)) return wup;
    }
    return 0;
}

APP_VERSION* CLIENT_STATE::lookup_app_version(APP* app, int version_num) {
    for (unsigned int i=0; i<app_versions.size(); i++) {
        APP_VERSION* avp = app_versions[i];
        if (avp->app == app && version_num==avp->version_num) {
            return avp;
        }
    }
    return 0;
}

FILE_INFO* CLIENT_STATE::lookup_file_info(PROJECT* p, char* name) {
    for (unsigned int i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->project == p && !strcmp(fip->name, name)) {
            return fip;
        }
    }
    return 0;
}

// functions to create links between state objects
// (which, in their XML form, refer to one another by name)
//
int CLIENT_STATE::link_app(PROJECT* p, APP* app) {
    app->project = p;
    return 0;
}

int CLIENT_STATE::link_file_info(PROJECT* p, FILE_INFO* fip) {
    fip->project = p;
    return 0;
}

int CLIENT_STATE::link_app_version(PROJECT* p, APP_VERSION* avp) {
    APP* app;
    FILE_INFO* fip;

    app = lookup_app(p, avp->app_name);
    if (!app) {
        fprintf(stderr,
            "app_version refers to nonexistent app: %s\n", avp->app_name
        );
        return 1;
    }
    fip = lookup_file_info(p, avp->file_name);
    if (!fip) {
        fprintf(stderr,
            "app_version refers to nonexistent file: %s\n", avp->file_name
        );
        return 1;
    }
    avp->project = p;
    avp->app = app;
    avp->file_info = fip;
    return 0;
}

int CLIENT_STATE::link_io_file_desc(PROJECT* p, IO_FILE_DESC* ifd) {
    FILE_INFO* fip;

    fip = lookup_file_info(p, ifd->file_name);
    if (!fip) {
        fprintf(stderr,
            "I/O desc links to nonexistent file: %s\n", ifd->file_name
        );
        return 1;
    }
    ifd->file_info = fip;
    return 0;
}

int CLIENT_STATE::link_workunit(PROJECT* p, WORKUNIT* wup) {
    APP* app;
    APP_VERSION* avp;
    unsigned int i;
    int retval;

    app = lookup_app(p, wup->app_name);
    if (!app) {
        fprintf(stderr,
            "WU refers to nonexistent app: %s\n", wup->app_name
        );
        return 1;
    }
    avp = lookup_app_version(app, app->version_num);
    if (!avp) {
        fprintf(stderr,
            "WU refers to nonexistent app_version: %s %d\n",
            wup->app_name, wup->version_num
        );
        return 1;
    }
    wup->project = p;
    wup->app = app;
    wup->avp = avp;
    for (i=0; i<wup->input_files.size(); i++) {
        retval = link_io_file_desc(p, &wup->input_files[i]);
        if (retval) return retval;
    }
    return 0;
}
int CLIENT_STATE::link_result(PROJECT* p, RESULT* rp) {
    WORKUNIT* wup;
    unsigned int i;
    int retval;

    wup = lookup_workunit(p, rp->wu_name);
    if (!wup) {
        fprintf(stderr, "result refers to nonexistent WU: %s\n", rp->wu_name);
        return 1;
    }
    rp->project = p;
    rp->wup = wup;
    rp->app = wup->app;
    for (i=0; i<rp->output_files.size(); i++) {
        retval = link_io_file_desc(p, &rp->output_files[i]);
        if (retval) return retval;
    }
    return 0;
}

void CLIENT_STATE::print_counts() {
    if (log_flags.state_debug) {
        printf(
            "Client state file:\n"
            "%d projects\n"
            "%d file_infos\n"
            "%d app_versions\n"
            "%d workunits\n"
            "%d results\n",
            projects.size(),
            file_infos.size(),
            app_versions.size(),
            workunits.size(),
            results.size()
        );
    }
}

// delete unneeded records and files
//
bool CLIENT_STATE::garbage_collect() {
    unsigned int i;
    FILE_INFO* fip;
    RESULT* rp;
    WORKUNIT* wup;
    vector<RESULT*>::iterator result_iter;
    vector<WORKUNIT*>::iterator wu_iter;
    vector<FILE_INFO*>::iterator fi_iter;
    bool action = false;

    // zero references counts on WUs and FILE_INFOs
    for (i=0; i<workunits.size(); i++) {
        wup = workunits[i];
        wup->ref_cnt = 0;
    }
    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        fip->ref_cnt = 0;
    }

    // delete RESULTs that have been finished and reported;
    // reference-count files referred to by other results
    //
    result_iter = results.begin();
    while (result_iter != results.end()) {
        rp = *result_iter;
        if (rp->is_server_ack) {
            if (log_flags.state_debug) printf("deleting result %s\n", rp->name);
            delete rp;
            result_iter = results.erase(result_iter);
            action = true;
        } else {
            rp->wup->ref_cnt++;
            for (i=0; i<rp->output_files.size(); i++) {
                rp->output_files[i].file_info->ref_cnt++;
            }
            result_iter++;
        }
    }

    // delete WORKUNITs not referenced by any result;
    // reference-count files referred to by other WUs
    //
    wu_iter = workunits.begin();
    while (wu_iter != workunits.end()) {
        wup = *wu_iter;
        if (wup->ref_cnt == 0) {
            if (log_flags.state_debug) printf("deleting workunit %s\n", wup->name);
            delete wup;
            wu_iter = workunits.erase(wu_iter);
            action = true;
        } else {
            for (i=0; i<wup->input_files.size(); i++) {
                wup->input_files[i].file_info->ref_cnt++;
            }
            wu_iter++;
        }
    }

    // delete FILE_INFOs (and corresponding files)
    // that are not referenced by any WORKUNIT or RESULT,
    // and are not sticky.
    //
    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fip = *fi_iter;
        if (fip->ref_cnt==0 && !fip->sticky && !fip->executable) {
            fip->delete_file();
            if (log_flags.state_debug) printf("deleting file %s\n", fip->name);
            delete fip;
            fi_iter = file_infos.erase(fi_iter);
            action = true;
        } else {
            fi_iter++;
        }
    }

    // TODO: delete obsolete APP_VERSIONs

    return action;
}

// TODO: write no more often than X seconds
//
int CLIENT_STATE::write_state_file_if_needed() {
    int retval;
    if (client_state_dirty) {
        retval = write_state_file();
        if (retval) return retval;
        client_state_dirty = false;
    }
    return 0;
}

void CLIENT_STATE::parse_cmdline(int argc, char** argv) {
    int i;
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-exit_when_idle")) {
            exit_when_idle = true;
        }
    }
}

bool CLIENT_STATE::time_to_exit() {
    if (!exit_when_idle) return false;
    if (results.size() == 0 && contacted_sched_server) return true;
    return false;
}
