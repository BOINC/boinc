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

#include "windows_cpp.h"
#include "error_numbers.h"

#include <stdio.h>
#include <time.h>

#include "account.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "hostinfo.h"
#include "log_flags.h"
#include "parse.h"
#include "speed_stats.h"
#include "client_state.h"

#define SECONDS_IN_MONTH 2592000

CLIENT_STATE gstate;

CLIENT_STATE::CLIENT_STATE() {
    net_xfers = new NET_XFER_SET;
    http_ops = new HTTP_OP_SET(net_xfers);
    file_xfers = new FILE_XFER_SET(http_ops);
    pers_xfers = new PERS_FILE_XFER_SET(file_xfers);
    scheduler_op = new SCHEDULER_OP(http_ops);
    client_state_dirty = false;
    exit_when_idle = false;
    run_time_test = true;
    giveup_after = PERS_GIVEUP;
    contacted_sched_server = false;
    activities_suspended = false;
    core_client_major_version = MAJOR_VERSION;
    core_client_minor_version = MINOR_VERSION;
    platform_name = HOST;
    exit_after = -1;
    app_started = 0;
    max_transfer_rate = 9999999;
    max_bytes = 0;
    user_idle = true;
    suspend_requested = false;
}

int CLIENT_STATE::init() {
    int retval;

    srand(time(NULL));

    // TODO: set this to actual # of CPUs (or less, depending on prefs?)
    //
    nslots = 1;

    // Read the global preferences file, if it exists.
    //
    retval = global_prefs.parse_file();
    if (retval) {
        printf("No global preferences file; will use defaults.\n");
    }

    // parse account files.
    // If there are none, prompt user for project URL and create file
    // 
    retval = parse_account_files();
    if (projects.size() == 0) {
        retval = get_initial_project();
        if (retval) {
            printf("can't get initial project\n");
            return retval;
        }
        retval = parse_account_files();
        if (projects.size() == 0) {
            if (retval) {
                printf("can't get initial project\n");
                return retval;
            }
        }
    }

    // Parse the client state file,
    // ignoring any <project> tags (and associated stuff)
    // for projects with no account file
    //
    parse_state_file();

    if (log_flags.state_debug) {
        print_counts();
    }
    
    // set up the project and slot directories
    //
    make_project_dirs();
    make_slot_dirs();

    // Run the time tests and host information check if needed
    // TODO: break time tests and host information check into two
    //       separate functions? 
    if (gstate.run_time_tests()) {
        gstate.time_tests();
    }
    
    // Restart any tasks that were running when we last quit the client
    gstate.restart_tasks();

    return 0;
}

// Returns true if time tests should be run
// This is determined by seeing if the user passed the "-no_time_test"
// flag or if it's been a month since we last checked time stats
//
bool CLIENT_STATE::run_time_tests() {
    return (run_time_test && (
        difftime(time(0), (time_t)host_info.p_calculated) > SECONDS_IN_MONTH
    ));
}

// gets info about the host
// NOTE: this locks up the process for 10-20 seconds,
// so it should be called very seldom
//
int CLIENT_STATE::time_tests() {
    if (log_flags.measurement_debug) {
        printf("Getting general host information.\n");
    }
    clear_host_info(host_info);
    get_host_info(host_info);       // this is platform dependent
#if 0
    double fpop_test_secs = 2.0;
    double iop_test_secs = 2.0;
    double mem_test_secs = 2.0;

    if (log_flags.measurement_debug) {
        printf(
            "Running floating point test for about %.1f seconds.\n",
            fpop_test_secs
        );
    }
    host_info.p_fpops = run_double_prec_test(fpop_test_secs); //these are not

    if (log_flags.measurement_debug) {
        printf(
            "Running integer test for about %.1f seconds.\n",
            iop_test_secs
        );
    }
    host_info.p_iops = run_int_test(iop_test_secs);

    if (log_flags.measurement_debug) {
        printf(
            "Running memory bandwidth test for about %.1f seconds.\n",
            mem_test_secs
        );
    }
    host_info.p_membw = run_mem_bandwidth_test(mem_test_secs);
#else
    host_info.p_fpops = 1e9;
    host_info.p_iops = 1e9;
    host_info.p_membw = 4e9;
    host_info.m_cache = 1e6;
#endif

    host_info.p_calculated = (double)time(0);
    return 0;
}

// Return the maximum allowed disk usage as determined by user preferences.
// Since there are three different settings in the prefs, it returns the least
// of the three.
double CLIENT_STATE::allowed_disk_usage() {
    double percent_space, min_val;

    // Calculate allowed disk usage based on % pref
    //
    percent_space = host_info.d_total*global_prefs.disk_max_used_pct/100.0;

    min_val = host_info.d_free - global_prefs.disk_min_free_gb*1e9;

    // Return the minimum of the three
    //
    return min(min(global_prefs.disk_max_used_gb*1e9, percent_space), min_val);
}

double CLIENT_STATE::current_disk_usage() {
    double sz = 0;
    sz = dir_size(".");
    return sz;
}

// See if (on the basis of user prefs) we should suspend activities.
// If so, suspend tasks
//
int CLIENT_STATE::check_suspend_activities() {
    bool should_suspend = false;
    if (global_prefs.dont_run_on_batteries && host_is_running_on_batteries()) {
        should_suspend = true;
    }
    if (!user_idle) {
        should_suspend = true;
    }
	if(suspend_requested) {
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

static void print_log(char* p) {
    if (log_flags.poll_debug) {
        printf(p);
    }
}

// do_something is where all the action happens.  This is part of the
// finite state machine abstraction of the client.  Each of the key
// elements of the client is given a chance to perform work here.
// return true if something happened
// TODO: handle errors passed back up to here?
//
bool CLIENT_STATE::do_something() {
    int nbytes=0;
    bool action = false, x;

    check_suspend_activities();
    if (!activities_suspended) {
        // Call these functions in bottom to top order with
        // respect to the FSM hierarchy

        if (max_bytes > 0)
            net_xfers->poll(max_bytes, nbytes);
        if (nbytes) { max_bytes -= nbytes; action=true; print_log("net_xfers\n"); }

        x = http_ops->poll();
        if (x) {action=true; print_log("http_ops::poll\n"); }

        x = file_xfers->poll();
        if (x) {action=true; print_log("file_xfers::poll\n"); }

        x = active_tasks.poll();
        if (x) {action=true; print_log("active_tasks::poll\n"); }

        x = active_tasks.poll_time();
        if (x) {action=true; print_log("active_tasks::poll_time\n"); }

        x = scheduler_rpc_poll();
        if (x) {action=true; print_log("scheduler_rpc_poll\n"); }

        x = start_apps();
        if (x) {action=true; print_log("start_apps\n"); }

        x = pers_xfers->poll();
        if (x) {action=true; print_log("pers_xfers->poll\n"); }

        x = handle_running_apps();
        if (x) {action=true; print_log("handle_running_apps\n"); }

        x = handle_pers_file_xfers();
        if (x) {action=true; print_log("handle_pers_file_xfers\n"); }

        x = garbage_collect();
        if (x) {action=true; print_log("garbage_collect\n"); }

        x = update_results();
        if (x) {action=true; print_log("update_results\n"); }

        write_state_file_if_needed();
    }
    if (!action) {
        time_stats.update(true, !activities_suspended);
        max_bytes = max_transfer_rate;
    }
    return action;
}

// Parse the client_state.xml file
//
int CLIENT_STATE::parse_state_file() {
    char buf[256];
    FILE* f = fopen(STATE_FILE_NAME, "r");
    PROJECT temp_project, *project;
    int retval=0;

    if (!f) {
        if (log_flags.state_debug) {
            printf("No state file; will create one\n");
        }
        return ERR_FOPEN;
    }
    fgets(buf, 256, f);
    if (!match_tag(buf, "<client_state>")) {
        retval = ERR_XML_PARSE;
        goto done;
    }
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            retval = 0;
            break;
        } else if (match_tag(buf, "<project>")) {
            temp_project.parse_state(f);
            project = lookup_project(temp_project.master_url);
            if (project) {
                project->copy_state_fields(temp_project);
            } else {
                fprintf(stderr,
                    "Project %s found in state file but not prefs.\n",
                    temp_project.master_url
                );
            }
        } else if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            app->parse(f);
            if (project) {
                retval = link_app(project, app);
                if (!retval) apps.push_back(app);
            } else {
                delete app;
            }
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO* fip = new FILE_INFO;
            fip->parse(f, false);
            if (project) {
                retval = link_file_info(project, fip);
                if (!retval) file_infos.push_back(fip);
                // If the file had a failure before, there's no reason
                // to start another file transfer
                if (fip->had_failure()) {
                    if (fip->pers_file_xfer) delete fip->pers_file_xfer;
                    fip->pers_file_xfer = NULL;
                }
                // Init PERS_FILE_XFER and push it onto pers_file_xfer stack
                if (fip->pers_file_xfer) {
                    fip->pers_file_xfer->init(fip, fip->upload_when_present);
                    retval = pers_xfers->insert( fip->pers_file_xfer );
                    if (retval) {
                        // TODO: What should we do here?
                    }
                }
            } else {
                delete fip;
            }
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION* avp = new APP_VERSION;
            avp->parse(f);
            if (project) {
                retval = link_app_version(project, avp);
                if (!retval) app_versions.push_back(avp);
            } else {
                delete avp;
            }
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wup = new WORKUNIT;
            wup->parse(f);
            if (project) {
                retval = link_workunit(project, wup);
                if (!retval) workunits.push_back(wup);
            } else {
                delete wup;
            }
        } else if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT;
            rp->parse_state(f);
            if (project) {
                retval = link_result(project, rp);
                if (!retval) results.push_back(rp);
                rp->state = RESULT_NEW;
            } else {
                fprintf(stderr, "error: link_result failed\n");
                delete rp;
            }
        } else if (match_tag(buf, "<host_info>")) {
            host_info.parse(f);
        } else if (match_tag(buf, "<time_stats>")) {
            time_stats.parse(f);
        } else if (match_tag(buf, "<net_stats>")) {
            net_stats.parse(f);
        } else if (match_tag(buf, "<active_task_set>")) {
            active_tasks.parse(f, this);
        } else if (match_tag(buf, "<platform_name>")) {
            // should match out current platform name
        } else if (match_tag(buf, "<version>")) {
            // could put logic here to detect incompatible state files
            // after core client update
        } else if (match_tag(buf, "<core_client_major_version>")) {
            // TODO: handle old client state file if different version
        } else if (match_tag(buf, "<core_client_minor_version>")) {
            // TODO: handle old client state file if different version
        } else {
            fprintf(stderr, "CLIENT_STATE::parse_state_file: unrecognized: %s\n", buf);
            retval = ERR_XML_PARSE;
            goto done;
        }
    }
done:
    fclose(f);
    return retval;
}

// Write the client_state.xml file
//
int CLIENT_STATE::write_state_file() {
    unsigned int i, j;
    FILE* f = fopen(STATE_FILE_TEMP, "w");
    int retval;

    if (log_flags.state_debug) {
        printf("Writing state file\n");
    }
    if (!f) {
        fprintf(stderr, "can't open temp state file: %s\n", STATE_FILE_TEMP);
        return ERR_FOPEN;
    }
    fprintf(f, "<client_state>\n");
    host_info.write(f);
    time_stats.write(f, false);
    net_stats.write(f, false);
    for (j=0; j<projects.size(); j++) {
        PROJECT* p = projects[j];
        p->write_state(f);
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
    fprintf(f,
        "<platform_name>%s</platform_name>\n"
        "<core_client_major_version>%d</core_client_major_version>\n"
        "<core_client_minor_version>%d</core_client_minor_version>\n",
        platform_name,
        core_client_major_version,
        core_client_minor_version
    );
    fprintf(f, "</client_state>\n");
    fclose(f);
    retval = boinc_rename(STATE_FILE_TEMP, STATE_FILE_NAME);
    if (log_flags.state_debug) {
        printf("Done writing state file\n");
    }
    if (retval) return ERR_RENAME;
    return 0;
}

// TODO: write no more often than X seconds
// Write the client_state.xml file if necessary
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

// See if the project specified by master_url already exists
// in the client state record.
//
PROJECT* CLIENT_STATE::lookup_project(char* master_url) {
    for (unsigned int i=0; i<projects.size(); i++) {
        if (!strcmp(master_url, projects[i]->master_url)) {
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

// Find the active task for a given workunit
//
ACTIVE_TASK* CLIENT_STATE::lookup_active_task_by_result(RESULT* rep)
{
    for(unsigned int i = 0; i < active_tasks.active_tasks.size(); i ++) {
        if(active_tasks.active_tasks[i]->result == rep) return active_tasks.active_tasks[i];
    }
    return NULL;
}

// functions to create links between state objects
// (which, in their XML form, reference one another by name)
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
    FILE_REF file_ref;
    unsigned int i;

    avp->project = p;
    app = lookup_app(p, avp->app_name);
    if (!app) {
        fprintf(stderr,
            "app_version refers to nonexistent app: %s\n", avp->app_name
        );
        return 1;
    }
    avp->app = app;

    for (i=0; i<avp->app_files.size(); i++) {
        file_ref = avp->app_files[i];
        fip = lookup_file_info(p, file_ref.file_name);
        if (!fip) {
            fprintf(stderr,
                "app_version refers to nonexistent file: %s\n",
                file_ref.file_name
            );
            return 1;
        }

        // any file associated with an app version must be signed
        //
        fip->signature_required = true;
        avp->app_files[i].file_info = fip;
    }
    return 0;
}

int CLIENT_STATE::link_file_ref(PROJECT* p, FILE_REF* file_refp) {
    FILE_INFO* fip;

    fip = lookup_file_info(p, file_refp->file_name);
    if (!fip) {
        fprintf(stderr,
            "I/O desc links to nonexistent file: %s\n", file_refp->file_name
        );
        return 1;
    }
    file_refp->file_info = fip;
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
    avp = lookup_app_version(app, wup->version_num);
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
        retval = link_file_ref(p, &wup->input_files[i]);
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
        retval = link_file_ref(p, &rp->output_files[i]);
        if (retval) {
            fprintf(stderr, "error: link_result: link_file_ref failed\n");
            return retval;
        }
    }
    return 0;
}

int CLIENT_STATE::latest_version_num(char* app_name) {
    unsigned int i;
    int best = -1;
    APP_VERSION* avp;

    for (i=0; i<app_versions.size(); i++) {
        avp = app_versions[i];
        if (strcmp(avp->app_name, app_name)) continue;
        if (avp->version_num < best) continue;
        best = avp->version_num;
    }
    if (best < 0) fprintf(stderr, "CLIENT_STATE::latest_version_num: no version\n");
    return best;
}

// Print debugging information about how many projects/files/etc
// are currently in the client state record
//
void CLIENT_STATE::print_counts() {
    if (log_flags.state_debug) {
        printf(
            "Client state file:\n"
            "%d projects\n"
            "%d file_infos\n"
            "%d app_versions\n"
            "%d workunits\n"
            "%d results\n",
            (int)projects.size(),
            (int)file_infos.size(),
            (int)app_versions.size(),
            (int)workunits.size(),
            (int)results.size()
        );
    }
}

// delete unneeded records and files
//
bool CLIENT_STATE::garbage_collect() {
    unsigned int i;
    int fail_num;
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
        if (rp->state == RESULT_SERVER_ACK) {
            if (log_flags.state_debug) printf("deleting result %s\n", rp->name);
            delete rp;
            result_iter = results.erase(result_iter);
            action = true;
        } else {
            // See if the files for this result's workunit had
            // any errors (MD5, RSA, etc)
            fail_num = rp->wup->had_failure();
            if (fail_num) {
                if (!rp->exit_status)       // If we don't already have an error for this file
                    rp->exit_status = fail_num;
                if (rp->state < RESULT_READY_TO_ACK) {
                    rp->state = RESULT_READY_TO_ACK;
                }
            } else {
                rp->wup->ref_cnt++;
            }
            for (i=0; i<rp->output_files.size(); i++) {
                // If one of the file infos had a failure,
                // mark the result as done and report the error.
                // The result, workunits, and file infos
                // will be cleaned up after the server is notified
                //
                fail_num = rp->output_files[i].file_info->had_failure();
                if (fail_num) {
                    if (!rp->exit_status)       // If we don't already have an error for this file
                        rp->exit_status = fail_num;
                    if (rp->state < RESULT_READY_TO_ACK) {
                        rp->state = RESULT_READY_TO_ACK;
                    }
                } else {
                    rp->output_files[i].file_info->ref_cnt++;
                }
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
            if (log_flags.state_debug) {
                printf("deleting workunit %s\n", wup->name);
            }
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

    if (log_flags.state_debug && action) printf("garbage_collect\n");
    return action;
}

// update the state of results
//
bool CLIENT_STATE::update_results() {
    RESULT* rp;
    vector<RESULT*>::iterator result_iter;
    bool action = false;
    
    // delete RESULTs that have been finished and reported;
    // reference-count files referred to by other results
    //
    result_iter = results.begin();
    while (result_iter != results.end()) {
        rp = *result_iter;
        switch (rp->state) {
            case RESULT_NEW:
                if (input_files_available(rp)) {
                    rp->state = RESULT_FILES_DOWNLOADED;
                    action = true;
                }
                break;
            case RESULT_FILES_DOWNLOADED:
                // The transition to COMPUTE_DONE is performed
                // in app_finished()
                break;
            case RESULT_COMPUTE_DONE:
                // Once the computation has been done, check
                // that the necessary files have been uploaded
                // before moving on
                if (rp->is_upload_done()) {
                    rp->state = RESULT_READY_TO_ACK;
                    action = true;
                }
                break;
            case RESULT_READY_TO_ACK:
                // The transition to SERVER_ACK is performed in
                // handle_scheduler_reply()
                break;
            case RESULT_SERVER_ACK:
                // The result has been received by the scheduling
                // server.  It will be deleted on the next
                // garbage collection, which we trigger by
                // setting action to true
                action = true;
                break;
        }
        result_iter++;
    }
    return action;
}


// Parse the command line arguments passed to the client
//
void CLIENT_STATE::parse_cmdline(int argc, char** argv) {
    int i;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-exit_when_idle")) {
            exit_when_idle = true;
            continue;
        }        
        if (!strcmp(argv[i], "-no_time_test")) {
            run_time_test = false;
            continue;
        };
        if (!strcmp(argv[i], "-exit_after")) {
            exit_after = atoi(argv[++i]);
            continue;
        };
        // Give up on file transfers after x seconds.  Default value is 1209600 (2 weeks)
        if (!strcmp(argv[i], "-giveup_after")) {
            giveup_after = atoi(argv[++i]);
            continue;
        };

        if (!strcmp(argv[i], "-limit_transfer_rate")) {
            max_transfer_rate = atoi(argv[++i]);
            continue;
        };
    }
}

// Returns true if the core client should exit
//
bool CLIENT_STATE::time_to_exit() {
    if (!exit_when_idle && (exit_after == -1)) return false;
    if ((exit_after != -1) && app_started &&
        (difftime(time(0), app_started) >= exit_after)) {
        printf("exiting because time is up: %d\n", exit_after);
        return true;
    }
    if (exit_when_idle && (results.size() == 0) && contacted_sched_server) {
        printf("exiting because no more results\n");
        return true;
    }
    return false;
}

void CLIENT_STATE::set_client_state_dirty(char* source) {
    if (log_flags.state_debug) {
        printf("set dirty: %s\n", source);
    }
    client_state_dirty = true;
}

// Report error back to project, setting result state to finished and backing
// off on the project.  The error will appear in the stderr_out field of
// the result
// 
int CLIENT_STATE::report_project_error( RESULT &res, int err_num, char *err_msg ) {
    char total_err[256];
    
    res.state = RESULT_READY_TO_ACK;
    scheduler_op->backoff(res.project,"");
    
    sprintf( total_err, "BOINC Core Client: Err %d: %s\n", err_num, err_msg );
    if( strlen(res.stderr_out)+strlen(total_err) < STDERR_MAX_LEN ) {
        strcat( res.stderr_out, total_err );
    }
    
    return 0;
}

