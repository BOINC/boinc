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

#include "parse.h"
#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"

void CLIENT_STATE::set_client_state_dirty(const char* source) {
    log_messages.printf(CLIENT_MSG_LOG::DEBUG_STATE, "set dirty: %s\n", source);
    client_state_dirty = true;
}

// Parse the client_state.xml file
//
int CLIENT_STATE::parse_state_file() {
    char buf[256];
    PROJECT *project=NULL;
    int retval=0;
    int failnum;
    const char *fname;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    // Look for a valid state file:
    // First the regular one, then the "next" one.
    //
    if (boinc_file_exists(STATE_FILE_NAME)) {
        fname = STATE_FILE_NAME;
    } else if (boinc_file_exists(STATE_FILE_NEXT)) {
        fname = STATE_FILE_NEXT;
    } else {
        scope_messages.printf("CLIENT_STATE::parse_state_file(): No state file; will create one\n");

        // avoid warning messages about version
        //
        old_major_version = BOINC_MAJOR_VERSION;
        old_minor_version = BOINC_MINOR_VERSION;
        old_release = BOINC_RELEASE;
        return ERR_FOPEN;
    }

    FILE* f = fopen(fname, "r");
    MIOFILE mf;
    mf.init_file(f);
    fgets(buf, 256, f);
    if (!match_tag(buf, "<client_state>")) {

        // if file is invalid (probably empty), try the previous statefile
        //
        fclose(f);
        f = fopen(STATE_FILE_PREV, "r");
        mf.init_file(f);
        fgets(buf, 256, f);
        if (!match_tag(buf, "<client_state>")) {
            msg_printf(NULL, MSG_ERROR, "Missing open tag in state file.\n");
            retval = ERR_XML_PARSE;
            goto done;
        }
    }
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            retval = 0;
            break;
        } else if (match_tag(buf, "<project>")) {
            PROJECT temp_project;
            retval = temp_project.parse_state(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse project in state file");
            } else {
                project = lookup_project(temp_project.master_url);
                if (project) {
                    project->copy_state_fields(temp_project);
                } else {
                    msg_printf(&temp_project, MSG_ERROR,
                        "Project %s is in state file but no account file found",
                        temp_project.get_project_name()
                    );
                }
            }
        } else if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            retval = app->parse(mf);
            if (project && project->anonymous_platform) {
                delete app;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse app in state file");
                delete app;
            } else {
                if (project) {
                    retval = link_app(project, app);
                    if (retval) {
                        msg_printf(project, MSG_ERROR,
                            "Can't link app %s in state file", app->name
                        );
                        delete app;
                    } else {
                        apps.push_back(app);
                    }
                } else {
                    msg_printf(NULL, MSG_ERROR,
                        "App %s outside project in state file", app->name
                    );
                    delete app;
                }
            }
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO* fip = new FILE_INFO;
            retval = fip->parse(mf, false);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse file info in state file");
                delete fip;
            } else {
                if (project) {
                    retval = link_file_info(project, fip);
                    if (project->anonymous_platform && retval == ERR_NOT_UNIQUE) {
                        continue;
                    }
                    if (retval) {
                        msg_printf(project, MSG_ERROR,
                            "Can't link file info %s in state file", fip->name
                        );
                        delete fip;
                    } else {
                        file_infos.push_back(fip);
                        // If the file had a failure before,
                        // don't start another file transfer
                        //
                        if (fip->had_failure(failnum)) {
                            if (fip->pers_file_xfer) {
                                delete fip->pers_file_xfer;
                                fip->pers_file_xfer = NULL;
                            }
                        }

                        if (fip->pers_file_xfer) {
                            retval = fip->pers_file_xfer->init(fip, fip->upload_when_present);
                            if (retval) {
                                msg_printf(project, MSG_ERROR,
                                    "Can't initialize pers file xfer for %s", fip->name
                                );
                            }
                            retval = pers_file_xfers->insert(fip->pers_file_xfer);
                            if (retval) {
                                msg_printf(project, MSG_ERROR,
                                    "Can't insert pers file xfer for %s", fip->name
                                );
                            }
                        }
                    }
                } else {
                    msg_printf(NULL, MSG_ERROR, "File info outside project in state file");
                    delete fip;
                }
            }
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION* avp = new APP_VERSION;
            retval = avp->parse(mf);
            if (project && project->anonymous_platform) {
                delete avp;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse app version in state file");
                delete avp;
            } else {
                if (project) {
                    retval = link_app_version(project, avp);
                    if (retval) {
                        msg_printf(project, MSG_ERROR, "Can't link app version in state file");
                        delete avp;
                    } else {
                        app_versions.push_back(avp);
                    }
                } else {
                    msg_printf(NULL, MSG_ERROR, "App version outside project in state file");
                    delete avp;
                }
            }
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wup = new WORKUNIT;
            retval = wup->parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse workunit in state file");
                delete wup;
            } else {
                if (project) {
                    retval = link_workunit(project, wup);
                    if (retval) {
                        msg_printf(project, MSG_ERROR, "Can't link workunit in state file");
                        delete wup;
                    } else {
                        workunits.push_back(wup);
                    }
                } else {
                    msg_printf(NULL, MSG_ERROR, "Workunit outside project in state file");
                    delete wup;
                }
            }
        } else if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT;
            retval = rp->parse_state(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse result in state file");
                delete rp;
            } else {
                if (project) {
                    retval = link_result(project, rp);
                    if (retval) {
                        msg_printf(project, MSG_ERROR,
                            "Can't link result %s in state file",
                            rp->name
                        );
                        delete rp;
                    } else {
                        results.push_back(rp);
                    }
                } else {
                    msg_printf(NULL, MSG_ERROR,
                        "Result %s outside project in state file",
                        rp->name
                    );
                    delete rp;
                }
            }
        } else if (match_tag(buf, "<host_info>")) {
            retval = host_info.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse host info in state file\n");
            }
        } else if (match_tag(buf, "<time_stats>")) {
            retval = time_stats.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse time stats in state file\n");
            }
        } else if (match_tag(buf, "<net_stats>")) {
            retval = net_stats.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse net stats in state file\n");
            }
        } else if (match_tag(buf, "<active_task_set>")) {
            retval = active_tasks.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse active tasks in state file\n");
            }
        } else if (match_tag(buf, "<platform_name>")) {
            // should match our current platform name
        } else if (parse_int(buf, "<user_run_request>", user_run_request)) {
            continue;
        } else if (parse_int(buf, "<user_network_request>", user_network_request)) {
            continue;
        } else if (parse_int(buf, "<core_client_major_version>", old_major_version)) {
        } else if (parse_int(buf, "<core_client_minor_version>", old_minor_version)) {
        } else if (parse_int(buf, "<core_client_release>", old_release)) {
        } else if (match_tag(buf, "<cpu_benchmarks_pending/>")) {
            run_cpu_benchmarks = true;
        } else if (match_tag(buf, "<work_fetch_no_new_work/>")) {
            work_fetch_no_new_work = true;
        } else if (match_tag(buf, "cpu_earliest_deadline_first/>")) {
            cpu_earliest_deadline_first = true;
        } else if (match_tag(buf, "<proxy_info>")) {
            retval = proxy_info.parse(mf);
            if (retval) {
                msg_printf(NULL, MSG_ERROR, "Can't parse proxy info in state file\n");
            }
        // } else if (parse_int(buf, "<user_run_request/>")) {
        } else if (parse_str(buf, "<host_venue>", main_host_venue, sizeof(main_host_venue))) {
        } else if (parse_double(buf, "<new_version_check_time>", new_version_check_time)) {
        } else if (parse_str(buf, "<newer_version>", newer_version)) {
        } else scope_messages.printf("CLIENT_STATE::parse_state_file: unrecognized: %s\n", buf);
    }
done:
    fclose(f);

    return retval;
}


// Write the client_state.xml file
//
int CLIENT_STATE::write_state_file() {
    FILE* f = boinc_fopen(STATE_FILE_NEXT, "w");

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);
    scope_messages.printf("CLIENT_STATE::write_state_file(): Writing state file\n");
    if (!f) {
        msg_printf(0, MSG_ERROR, "Can't open temp state file: %s\n", STATE_FILE_NEXT);
        return ERR_FOPEN;
    }
    MIOFILE mf;
    mf.init_file(f);
    int retval = write_state(mf);
    fclose(f);
    if (retval) return retval;

    // the following fails if no current file, so don't check
    //
    retval = boinc_rename(STATE_FILE_NAME, STATE_FILE_PREV);

    retval = boinc_rename(STATE_FILE_NEXT, STATE_FILE_NAME);
    scope_messages.printf("CLIENT_STATE::write_state_file(): Done writing state file\n");
    if (retval) return ERR_RENAME;
    return 0;
}

int CLIENT_STATE::write_state(MIOFILE& f) {
    unsigned int i, j;
    int retval;

    f.printf("<client_state>\n");
    retval = host_info.write(f);
    if (retval) return retval;
    retval = time_stats.write(f, false);
    if (retval) return retval;
    retval = net_stats.write(f);
    if (retval) return retval;
    for (j=0; j<projects.size(); j++) {
        PROJECT* p = projects[j];
        retval = p->write_state(f);
        if (retval) return retval;
        for (i=0; i<apps.size(); i++) {
            if (apps[i]->project == p) {
                retval = apps[i]->write(f);
                if (retval) return retval;
            }
        }
        for (i=0; i<file_infos.size(); i++) {
            if (file_infos[i]->project == p) {
                retval = file_infos[i]->write(f, false);
                if (retval) return retval;
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
    f.printf(
        "<platform_name>%s</platform_name>\n"
        "<core_client_major_version>%d</core_client_major_version>\n"
        "<core_client_minor_version>%d</core_client_minor_version>\n"
        "<core_client_release>%d</core_client_release>\n"
        "<user_run_request>%d</user_run_request>\n"
        "<user_network_request>%d</user_network_request>\n"
        "%s"
        "<new_version_check_time>%f</new_version_check_time>\n",
        platform_name,
        core_client_major_version,
        core_client_minor_version,
        core_client_release,
        user_run_request,
        user_network_request,
        cpu_benchmarks_pending?"<cpu_benchmarks_pending/>\n":"",
        new_version_check_time
    );
    if (newer_version.size()) {
        f.printf("<newer_version>%s</newer_version>\n", newer_version.c_str());
    }

    proxy_info.write(f);
    if (strlen(main_host_venue)) {
        f.printf("<host_venue>%s</host_venue>\n", main_host_venue);
    }
    f.printf("</client_state>\n");
    return 0;
}

// Write the client_state.xml file if necessary
// TODO: write no more often than X seconds
//
int CLIENT_STATE::write_state_file_if_needed() {
    int retval;
    if (client_state_dirty) {
        client_state_dirty = false;
        retval = write_state_file();
        if (retval) return retval;
    }
    return 0;
}

// look for app_versions.xml file in project dir.
// If find, get app versions from there,
// and use "anonymous platform" mechanism for this project
//
void CLIENT_STATE::check_anonymous() {
    unsigned int i;
    char dir[256], path[256];
    FILE* f;
    int retval;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        get_project_dir(p, dir);
        sprintf(path, "%s%s%s", dir, PATH_SEPARATOR, APP_INFO_FILE_NAME);
        f = fopen(path, "r");
        if (!f) continue;
        msg_printf(p, MSG_INFO,
            "Found %s; using anonymous platform\n", APP_INFO_FILE_NAME
        );

        p->anonymous_platform = true;
            // flag as anonymous even if can't parse file
        retval = parse_app_info(p, f);
        fclose(f);
    }
}

int CLIENT_STATE::parse_app_info(PROJECT* p, FILE* in) {
    char buf[256];
    MIOFILE mf;
    mf.init_file(in);

    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<app_info>")) continue;
        if (match_tag(buf, "</app_info>")) return 0;
        if (match_tag(buf, "<file_info>")) {
            FILE_INFO* fip = new FILE_INFO;
            if (fip->parse(mf, false)) {
                delete fip;
                continue;
            }
            if (link_file_info(p, fip)) {
                delete fip;
                continue;
            }
            fip->status = FILE_PRESENT;
            file_infos.push_back(fip);
            continue;
        }
        if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            if (app->parse(mf)) {
                delete app;
                continue;
            }
            if (lookup_app(p, app->name)) {
                delete app;
                continue;
            }
            link_app(p, app);
            apps.push_back(app);
            continue;
        }
        if (match_tag(buf, "<app_version>")) {
            APP_VERSION* avp = new APP_VERSION;
            if (avp->parse(mf)) {
                delete avp;
                continue;
            }
            if (gstate.link_app_version(p, avp)) {
                delete avp;
                continue;
            }
            link_app_version(p, avp);
            app_versions.push_back(avp);
            continue;
        }
        msg_printf(p, MSG_ERROR, "Unparsed line in app_info.xml: %s", buf);
    }
    return ERR_XML_PARSE;
}

int CLIENT_STATE::write_state_gui(MIOFILE& f) {
    unsigned int i, j;
    int retval;

    f.printf("<client_state>\n");

    retval = host_info.write(f);
    if (retval) return retval;
    retval = time_stats.write(f, false);
    if (retval) return retval;
    retval = net_stats.write(f);
    if (retval) return retval;

    for (j=0; j<projects.size(); j++) {
        PROJECT* p = projects[j];
        retval = p->write_state(f, true);
        if (retval) return retval;
        for (i=0; i<apps.size(); i++) {
            if (apps[i]->project == p) {
                retval = apps[i]->write(f);
                if (retval) return retval;
            }
        }
        for (i=0; i<app_versions.size(); i++) {
            if (app_versions[i]->project == p) app_versions[i]->write(f);
        }
        for (i=0; i<workunits.size(); i++) {
            if (workunits[i]->project == p) workunits[i]->write(f);
        }
        for (i=0; i<results.size(); i++) {
            if (results[i]->project == p) results[i]->write_gui(f);
        }
    }
    f.printf(
        "<platform_name>%s</platform_name>\n"
        "<core_client_major_version>%d</core_client_major_version>\n"
        "<core_client_minor_version>%d</core_client_minor_version>\n"
        "<core_client_release>%d</core_client_release>\n"
        "%s%s",
        platform_name,
        core_client_major_version,
        core_client_minor_version,
        core_client_release,
        work_fetch_no_new_work?"<work_fetch_no_new_work/>\n":"",
        cpu_earliest_deadline_first?"<cpu_earliest_deadline_first/>\n":""
    );

    global_prefs.write(f);

    if (strlen(main_host_venue)) {
        f.printf("<host_venue>%s</host_venue>\n", main_host_venue);
    }

    f.printf("</client_state>\n");
    return 0;
}

int CLIENT_STATE::write_tasks_gui(MIOFILE& f) {
    unsigned int i;
    
    f.printf("<results>\n");
    for(i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        rp->write_gui(f);
    }
    f.printf("</results>\n");

    return 0;
}

int CLIENT_STATE::write_file_transfers_gui(MIOFILE& f) {
    unsigned int i;

    f.printf("<file_transfers>\n");
    for (i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->pers_file_xfer) {
            fip->write_gui(f);
        }
    }
    f.printf("</file_transfers>\n");

    return 0;
}

const char *BOINC_RCSID_375ec798cc = "$Id$";
