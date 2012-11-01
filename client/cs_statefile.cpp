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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstring>
#include <errno.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "cs_proxy.h"
#include "file_names.h"
#include "project.h"
#include "result.h"

#define MAX_STATE_FILE_WRITE_ATTEMPTS 2

void CLIENT_STATE::set_client_state_dirty(const char* source) {
    if (log_flags.statefile_debug) {
        msg_printf(0, MSG_INFO, "[statefile] set dirty: %s\n", source);
    }
    client_state_dirty = true;
}

static bool valid_state_file(const char* fname) {
    char buf[256];
    FILE* f = boinc_fopen(fname, "r");
    if (!f) return false;
    if (!fgets(buf, 256, f)) {
        fclose(f);
        return false;
    }
    if (!match_tag(buf, "<client_state>")) {
        fclose(f);
        return false;
    }
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}

// return true if r0 arrived before r1
// used to sort result list
//
static inline bool arrived_first(RESULT* r0, RESULT* r1) {
    if (r0->received_time < r1->received_time) {
        return true;
    }
    if (r0->received_time > r1->received_time) {
        return false;
    }
    return (r0->name_md5 < r1->name_md5);
}

// Parse the client_state.xml file
//
int CLIENT_STATE::parse_state_file() {
    const char *fname;

    // Look for a valid state file:
    // First the regular one, then the "next" one.
    //
    if (valid_state_file(STATE_FILE_NEXT)) {
        fname = STATE_FILE_NEXT;
    } else if (valid_state_file(STATE_FILE_NAME)) {
        fname = STATE_FILE_NAME;
    } else if (valid_state_file(STATE_FILE_PREV)) {
        fname = STATE_FILE_PREV;
    } else {
        if (log_flags.statefile_debug) {
            msg_printf(0, MSG_INFO,
                "[statefile] CLIENT_STATE::parse_state_file(): No state file; will create one"
            );
        }

        // avoid warning messages about version
        //
        old_major_version = BOINC_MAJOR_VERSION;
        old_minor_version = BOINC_MINOR_VERSION;
        old_release = BOINC_RELEASE;
        return ERR_FOPEN;
    }
    return parse_state_file_aux(fname);
}

int CLIENT_STATE::parse_state_file_aux(const char* fname) {
    PROJECT *project=NULL;
    int retval=0;
    int failnum;
    bool btemp;
    string stemp;

    FILE* f = fopen(fname, "r");
    if (!f) return ERR_FOPEN;
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    while (!xp.get_tag()) {
        if (xp.match_tag("/client_state")) {
            break;
        }
        if (xp.match_tag("client_state")) {
            continue;
        }
        if (xp.match_tag("project")) {
            PROJECT temp_project;
            retval = temp_project.parse_state(xp);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "Can't parse project in state file");
            } else {
#ifdef SIM
                project = new PROJECT;
                *project = temp_project;
                projects.push_back(project);
#else
                project = lookup_project(temp_project.master_url);
                if (project) {
                    project->copy_state_fields(temp_project);
                } else {
                    msg_printf(&temp_project, MSG_INTERNAL_ERROR,
                        "Project %s is in state file but no account file found",
                        temp_project.get_project_name()
                    );
                }
#endif
            }
            continue;
        }
        if (xp.match_tag("app")) {
            APP* app = new APP;
            retval = app->parse(xp);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Application %s outside project in state file",
                    app->name
                );
                delete app;
                continue;
            }
            if (project->anonymous_platform) {
                delete app;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse application in state file"
                );
                delete app;
                continue;
            }
            retval = link_app(project, app);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't handle application %s in state file",
                    app->name
                );
                delete app;
                continue;
            }
            apps.push_back(app);
            continue;
        }
        if (xp.match_tag("file_info") || xp.match_tag("file")) {
            FILE_INFO* fip = new FILE_INFO;
            retval = fip->parse(xp);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "File info outside project in state file"
                );
                delete fip;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't handle file info in state file"
                );
                delete fip;
                continue;
            }
            retval = link_file_info(project, fip);
            if (project->anonymous_platform && retval == ERR_NOT_UNIQUE) {
                delete fip;
                continue;
            }
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't handle file info %s in state file",
                    fip->name
                );
                delete fip;
                continue;
            }
            file_infos.push_back(fip);
#ifndef SIM
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
                retval = fip->pers_file_xfer->init(fip, fip->pers_file_xfer->is_upload);
                if (retval) {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "Can't initialize file transfer for %s",
                        fip->name
                    );
                }
                retval = pers_file_xfers->insert(fip->pers_file_xfer);
                if (retval) {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "Can't start persistent file transfer for %s",
                        fip->name
                    );
                }
            }
#endif
            continue;
        }
        if (xp.match_tag("app_version")) {
            APP_VERSION* avp = new APP_VERSION;
            retval = avp->parse(xp);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Application version outside project in state file"
                );
                delete avp;
                continue;
            }
            if (project->anonymous_platform) {
                delete avp;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse application version in state file"
                );
                delete avp;
                continue;
            } 
            if (strlen(avp->platform) == 0) {
                strcpy(avp->platform, get_primary_platform());
            } else {
                if (!is_supported_platform(avp->platform)) {
                    // if it's a platform we haven't heard of,
                    // must be that the user tried out a 64 bit client
                    // and then reverted to a 32-bit client.
                    // Let's not throw away the app version and its WUs
                    //
#ifndef SIM
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "App version has unsupported platform %s; changing to %s",
                        avp->platform, get_primary_platform()
                    );
#endif
                    strcpy(avp->platform, get_primary_platform());
                }
            }
            if (avp->missing_coproc) {
                msg_printf(project, MSG_INFO,
                    "Application uses missing %s GPU",
                    avp->missing_coproc_name
                );
            }
            retval = link_app_version(project, avp);
            if (retval) {
                delete avp;
                continue;
            }
            app_versions.push_back(avp);
            continue;
        }
        if (xp.match_tag("workunit")) {
            WORKUNIT* wup = new WORKUNIT;
            retval = wup->parse(xp);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Workunit outside project in state file"
                );
                delete wup;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse workunit in state file"
                );
                delete wup;
                continue;
            }
            retval = link_workunit(project, wup);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't handle workunit in state file"
                );
                delete wup;
                continue;
            }
            workunits.push_back(wup);
            continue;
        }
        if (xp.match_tag("result")) {
            RESULT* rp = new RESULT;
            retval = rp->parse_state(xp);
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Task %s outside project in state file",
                    rp->name
                );
                delete rp;
                continue;
            }
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse task in state file"
                );
                delete rp;
                continue;
            }
            retval = link_result(project, rp);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't link task %s in state file",
                    rp->name
                );
                delete rp;
                continue;
            }
            // handle transition from old clients which didn't store result.platform;
            // skip for anon platform
            if (!project->anonymous_platform) {
                if (!strlen(rp->platform) || !is_supported_platform(rp->platform)) {
                    strcpy(rp->platform, get_primary_platform());
                    rp->version_num = latest_version(rp->wup->app, rp->platform);
                }
            }
            rp->avp = lookup_app_version(
                rp->wup->app, rp->platform, rp->version_num, rp->plan_class
            );
            if (!rp->avp) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "No application found for task: %s %d %s; discarding",
                    rp->platform, rp->version_num, rp->plan_class
                );
                delete rp;
                continue;
            }
            if (rp->avp->missing_coproc) {
                msg_printf(project, MSG_INFO,
                    "Missing coprocessor for task %s", rp->name
                );
                rp->coproc_missing = true;
            }
            rp->wup->version_num = rp->version_num;
            results.push_back(rp);
            continue;
        }
        if (xp.match_tag("project_files")) {
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Project files outside project in state file"
                );
                xp.skip_unexpected();
                continue;
            }
            parse_project_files(xp, project->project_files);
            project->link_project_files();
            continue;
        }
        if (xp.match_tag("host_info")) {
#ifdef SIM
            retval = host_info.parse(xp, false);
            coprocs = host_info._coprocs;
#else
            retval = host_info.parse(xp, true);
#endif
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse host info in state file"
                );
            }
            continue;
        }
        if (xp.match_tag("time_stats")) {
            retval = time_stats.parse(xp);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse time stats in state file"
                );
            }
            continue;
        }
        if (xp.match_tag("net_stats")) {
            retval = net_stats.parse(xp);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse network stats in state file"
                );
            }
            continue;
        }
        if (xp.match_tag("active_task_set")) {
            retval = active_tasks.parse(xp);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse active tasks in state file"
                );
            }
            continue;
        }
        if (xp.parse_string("platform_name", statefile_platform_name)) {
            continue;
        }
        if (xp.parse_string("alt_platform", stemp)) {
            continue;
        }
        if (xp.parse_int("user_run_request", retval)) {
            cpu_run_mode.set(retval, 0);
            continue;
        }
        if (xp.parse_int("user_run_prev_request", retval)) {
            cpu_run_mode.set_prev(retval);
            continue;
        }
        if (xp.parse_int("user_gpu_request", retval)) {
            gpu_run_mode.set(retval, 0);
            continue;
        }
        if (xp.parse_int("user_gpu_prev_request", retval)) {
            gpu_run_mode.set_prev(retval);
            continue;
        }
        if (xp.parse_int("user_network_request", retval)) {
            network_run_mode.set(retval, 0);
            continue;
        }
        if (xp.parse_int("core_client_major_version", old_major_version)) {
            continue;
        }
        if (xp.parse_int("core_client_minor_version", old_minor_version)) {
            continue;
        }
        if (xp.parse_int("core_client_release", old_release)) {
            continue;
        }
        if (xp.parse_bool("cpu_benchmarks_pending", btemp)) {
            if (btemp) run_cpu_benchmarks = true;
            continue;
        }
        if (xp.match_tag("proxy_info")) {
            retval = gui_proxy_info.parse(xp);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse proxy info in state file"
                );
            }
            continue;
        }
        if (xp.parse_str("host_venue", main_host_venue, sizeof(main_host_venue))) {
            continue;
        }
        if (xp.parse_double("new_version_check_time", new_version_check_time)) {
            continue;
        }
        if (xp.parse_double("all_projects_list_check_time", all_projects_list_check_time)) {
            continue;
        }
        if (xp.parse_string("newer_version", newer_version)) {
            continue;
        }
#ifdef ENABLE_AUTO_UPDATE
        if (xp.match_tag("auto_update")) {
            if (!project) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "auto update outside project in state file"
                );
                xp.skip_unexpected();
                continue;
            }
            if (!auto_update.parse(xp) && !auto_update.validate_and_link(project)) {
                auto_update.present = true;
            }
            continue;
        }
#endif
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] state_file: unrecognized: %s",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    sort_results();
    fclose(f);
    
    // if total resource share is zero, set all shares to 1
    //
    if (projects.size()) {
        unsigned int i;
        double x=0;
        for (i=0; i<projects.size(); i++) {
            x += projects[i]->resource_share;
        }
        if (!x) {
            msg_printf(NULL, MSG_INFO,
                "All projects have zero resource share; setting to 100"
            );
            for (i=0; i<projects.size(); i++) {
                projects[i]->resource_share = 100;
            }
        }
    }
    return 0;
}

// this is called whenever new results are added, namely at startup
// and after a scheduler RPC
//
void CLIENT_STATE::sort_results() {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        rp->name_md5 = md5_string(string(rp->name));
    }
    std::sort(
        results.begin(),
        results.end(),
        arrived_first
    );
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        rp->index = i;
    }
}

#ifndef SIM

// Write the client_state.xml file
//
int CLIENT_STATE::write_state_file() {
    MFILE mf;
    int retval, ret1, ret2, attempt;
#ifdef _WIN32
    char win_error_msg[4096];
#endif

    for (attempt=1; attempt<=MAX_STATE_FILE_WRITE_ATTEMPTS; attempt++) {
        if (attempt > 1) boinc_sleep(1.0);
            
        if (log_flags.statefile_debug) {
            msg_printf(0, MSG_INFO,
                "[statefile] Writing state file"
            );
        }
#ifdef _WIN32
        retval = mf.open(STATE_FILE_NEXT, "wc");
#else
        retval = mf.open(STATE_FILE_NEXT, "w");
#endif
        if (retval) {
            if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "Can't open %s: %s",
                    STATE_FILE_NEXT, boincerror(retval)
                );
            }
            if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
            return ERR_FOPEN;
        }
        MIOFILE miof;
        miof.init_mfile(&mf);
        ret1 = write_state(miof);
        ret2 = mf.close();
        if (ret1) {
            if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Couldn't write state file: %s", boincerror(retval)
                );
            }
            if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
            return ret1;
        }
        if (ret2) {
            if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
            return ret2;
        }

        // only attempt to rename the current state file if it exists.
        //
        if (boinc_file_exists(STATE_FILE_NAME)) {
            if (boinc_file_exists(STATE_FILE_PREV)) {
                retval = boinc_delete_file(STATE_FILE_PREV);
                if (retval) {
                    if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
#ifdef _WIN32
                        msg_printf(0, MSG_INFO,
                            "Can't delete previous state file; %s",
                            windows_error_string(win_error_msg, sizeof(win_error_msg))
                        );
#else
                        msg_printf(0, MSG_INFO,
                            "Can't delete previous state file: %s",
                            strerror(errno)
                        );
#endif
                    }
                    if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
                }
            }
            
            retval = boinc_rename(STATE_FILE_NAME, STATE_FILE_PREV);
            if (retval) {
                if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
#ifdef _WIN32
                    msg_printf(0, MSG_INFO,
                        "Can't rename current state file to previous state file; %s",
                        windows_error_string(win_error_msg, sizeof(win_error_msg))
                    );
#else
                    msg_printf(0, MSG_INFO, 
                        "Can't rename current state file to previous state file: %s", 
                        strerror(errno)
                    );
#endif
                }
                if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
            }
        }

        retval = boinc_rename(STATE_FILE_NEXT, STATE_FILE_NAME);
        if (log_flags.statefile_debug) {
            msg_printf(0, MSG_INFO,
                "[statefile] Done writing state file"
            );
        }
        if (!retval) break;     // Success!
        
        if ((attempt == MAX_STATE_FILE_WRITE_ATTEMPTS) || log_flags.statefile_debug) {
#ifdef _WIN32
            msg_printf(0, MSG_INFO,
                "rename error: %s",
                windows_error_string(win_error_msg, sizeof(win_error_msg))
            );
#elif defined (__APPLE__)
            if (log_flags.statefile_debug) {
                system("ls -al /Library/Application\\ Support/BOINC\\ Data/client*.*");
            }
#endif
        }
        if (attempt < MAX_STATE_FILE_WRITE_ATTEMPTS) continue;
        return ERR_RENAME;
    }
    return 0;
}

int CLIENT_STATE::write_state(MIOFILE& f) {
    unsigned int i, j;
    int retval;

#ifdef SIM
    fprintf(stderr, "simulator shouldn't write state file\n");
    exit(1);
#endif
    f.printf("<client_state>\n");
    retval = host_info.write(f, true, true);
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
            if (file_infos[i]->project != p) continue;
            FILE_INFO* fip = file_infos[i];
            // don't write file infos for anonymous platform app files
            //
            if (fip->anonymous_platform_file) continue;
            retval = fip->write(f, false);
            if (retval) return retval;
        }
        for (i=0; i<app_versions.size(); i++) {
            if (app_versions[i]->project == p) {
                app_versions[i]->write(f);
            }
        }
        for (i=0; i<workunits.size(); i++) {
            if (workunits[i]->project == p) workunits[i]->write(f);
        }
        for (i=0; i<results.size(); i++) {
            if (results[i]->project == p) results[i]->write(f, false);
        }
        p->write_project_files(f);
#ifdef ENABLE_AUTO_UPDATE
        if (auto_update.present && auto_update.project==p) {
            auto_update.write(f);
        }
#endif
    }
    active_tasks.write(f);
    f.printf(
        "<platform_name>%s</platform_name>\n"
        "<core_client_major_version>%d</core_client_major_version>\n"
        "<core_client_minor_version>%d</core_client_minor_version>\n"
        "<core_client_release>%d</core_client_release>\n"
        "<user_run_request>%d</user_run_request>\n"
        "<user_run_prev_request>%d</user_run_prev_request>\n"
        "<user_gpu_request>%d</user_gpu_request>\n"
        "<user_gpu_prev_request>%d</user_gpu_prev_request>\n"
        "<user_network_request>%d</user_network_request>\n"
        "%s"
        "<new_version_check_time>%f</new_version_check_time>\n"
        "<all_projects_list_check_time>%f</all_projects_list_check_time>\n",
        get_primary_platform(),
        core_client_version.major,
        core_client_version.minor,
        core_client_version.release,
        cpu_run_mode.get_perm(),
        cpu_run_mode.get_prev(),
        gpu_run_mode.get_perm(),
        gpu_run_mode.get_prev(),
        network_run_mode.get_perm(),
        cpu_benchmarks_pending?"<cpu_benchmarks_pending/>\n":"",
        new_version_check_time,
        all_projects_list_check_time
    );
    if (newer_version.size()) {
        f.printf("<newer_version>%s</newer_version>\n", newer_version.c_str());
    }
    for (i=1; i<platforms.size(); i++) {
        f.printf("<alt_platform>%s</alt_platform>\n", platforms[i].name.c_str());
    }
    if (gui_proxy_info.present) {
        gui_proxy_info.write(f);
    }
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

#endif // ifndef SIM

// look for app_versions.xml file in project dir.
// If find, get app versions from there,
// and use "anonymous platform" mechanism for this project
//
void CLIENT_STATE::check_anonymous() {
    unsigned int i;
    char dir[256], path[MAXPATHLEN];
    FILE* f;
    int retval;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        get_project_dir(p, dir, sizeof(dir));
        sprintf(path, "%s/%s", dir, APP_INFO_FILE_NAME);
        f = fopen(path, "r");
        if (!f) continue;
        msg_printf(p, MSG_INFO,
            "Found %s; using anonymous platform", APP_INFO_FILE_NAME
        );

        p->anonymous_platform = true;
            // flag as anonymous even if can't parse file
        retval = parse_app_info(p, f);
        if (retval) {
            msg_printf_notice(p, false,
                "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=app_info",
                "%s",
                _("Syntax error in app_info.xml")
            );
        }
        fclose(f);
    }
}

// parse a project's app_info.xml (anonymous platform) file
//
int CLIENT_STATE::parse_app_info(PROJECT* p, FILE* in) {
    char buf[256], path[MAXPATHLEN];
    MIOFILE mf;
    mf.init_file(in);
    XML_PARSER xp(&mf);

    while (!xp.get_tag()) {
        if (xp.match_tag("app_info")) continue;
        if (xp.match_tag("/app_info")) return 0;
        if (xp.match_tag("file_info")) {
            FILE_INFO* fip = new FILE_INFO;
            if (fip->parse(xp)) {
                delete fip;
                continue;
            }
            if (!fip->download_urls.empty() || !fip->upload_urls.empty()) {
                msg_printf(p, MSG_INFO,
                    "Can't specify URLs in app_info.xml"
                );
                delete fip;
                continue;
            }
            if (link_file_info(p, fip)) {
                delete fip;
                continue;
            }
            // check that the file is actually there
            //
            get_pathname(fip, path, sizeof(path));
            if (!boinc_file_exists(path)) {
                strcpy(buf,
                    _("File referenced in app_info.xml does not exist: ")
                );
                strcat(buf, fip->name);
                msg_printf(p, MSG_USER_ALERT, "%s", buf);
                delete fip;
                continue;
            }
            fip->status = FILE_PRESENT;
            fip->anonymous_platform_file = true;
            file_infos.push_back(fip);
            continue;
        }
        if (xp.match_tag("app")) {
            APP* app = new APP;
            if (app->parse(xp)) {
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
        if (xp.match_tag("app_version")) {
            APP_VERSION* avp = new APP_VERSION;
            if (avp->parse(xp)) {
                delete avp;
                continue;
            }
            if (strlen(avp->platform) == 0) {
                strcpy(avp->platform, get_primary_platform());
            }
            if (link_app_version(p, avp)) {
                delete avp;
                continue;
            }
            app_versions.push_back(avp);
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(p, MSG_INFO,
                "Unparsed line in app_info.xml: %s",
                xp.parsed_tag
            );
        }
    }
    return ERR_XML_PARSE;
}

#ifndef SIM

int CLIENT_STATE::write_state_gui(MIOFILE& f) {
    unsigned int i, j;
    int retval;

    f.printf("<client_state>\n");

#if 1
    // NOTE: the following stuff is not in CC_STATE.
    // However, BoincView (which does its own parsing) expects it
    // to be in the get_state() reply, so leave it in for now
    //
    retval = host_info.write(f, true, false);
    if (retval) return retval;

    // the following are for compatibility with old managers
    //
    if (coprocs.have_nvidia()) {
        f.printf("<have_cuda/>\n");
    }
    if (coprocs.have_ati()) {
        f.printf("<have_ati/>\n");
    }

    retval = time_stats.write(f, false);
    if (retval) return retval;
    retval = net_stats.write(f);
    if (retval) return retval;
#endif

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
        "<executing_as_daemon>%d</executing_as_daemon>\n",
        get_primary_platform(),
        core_client_version.major,
        core_client_version.minor,
        core_client_version.release,
        executing_as_daemon?1:0
    );
    for (i=0; i<platforms.size(); i++) {
        f.printf(
            "<platform>%s</platform>\n", platforms[i].name.c_str()
        );
    }

    global_prefs.write(f);

    // the following used by BoincView - don't remove
    //
    if (strlen(main_host_venue)) {
        f.printf("<host_venue>%s</host_venue>\n", main_host_venue);
    }

    f.printf("</client_state>\n");
    return 0;
}

int CLIENT_STATE::write_tasks_gui(MIOFILE& f, bool active_only) {
    unsigned int i;

    if (active_only) {
        for (i=0; i<active_tasks.active_tasks.size(); i++) {
            RESULT* rp = active_tasks.active_tasks[i]->result;
            rp->write_gui(f);
        }
    } else {
        for (i=0; i<results.size(); i++) {
            RESULT* rp = results[i];
            rp->write_gui(f);
        }
    }
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

#endif
