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

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "filesys.h"

#include "account.h"
#include "file_names.h"
#include "hostinfo.h"
#include "http.h"
#include "log_flags.h"
#include "speed_stats.h"
#include "client_state.h"

#define BENCHMARK_PERIOD        (SECONDS_PER_DAY*30)
    // rerun CPU benchmarks this often (hardware may have been upgraded)

CLIENT_STATE gstate;

CLIENT_STATE::CLIENT_STATE() {
    net_xfers = new NET_XFER_SET;
    http_ops = new HTTP_OP_SET(net_xfers);
    file_xfers = new FILE_XFER_SET(http_ops);
    pers_xfers = new PERS_FILE_XFER_SET(file_xfers);
    scheduler_op = new SCHEDULER_OP(http_ops);
    client_state_dirty = false;
    exit_when_idle = false;
    update_prefs = false;
    run_time_test = true;
    giveup_after = PERS_GIVEUP;
    contacted_sched_server = false;
    activities_suspended = false;
    core_client_major_version = MAJOR_VERSION;
    core_client_minor_version = MINOR_VERSION;
    platform_name = HOST;
    exit_after_app_start_secs = 0;
    app_started = 0;
    user_idle = true;
    use_http_proxy = false;
    use_socks_proxy = false;
    safe_strncpy(proxy_server_name, "", sizeof(proxy_server_name));
    proxy_server_port = 80;
    safe_strncpy(socks_user_name, "", sizeof(socks_user_name));
    safe_strncpy(socks_user_passwd, "", sizeof(socks_user_passwd));
    safe_strncpy(host_venue,"", sizeof(host_venue));
    suspend_requested = false;
    start_saver = false;
#ifdef _WIN32
    time_tests_handle = NULL;
#endif
    time_tests_id = 0;
}

void CLIENT_STATE::install_global_prefs() {
    net_xfers->max_bytes_sec_up = global_prefs.max_bytes_sec_up;
    net_xfers->max_bytes_sec_down = global_prefs.max_bytes_sec_down;
    net_xfers->bytes_left_up = global_prefs.max_bytes_sec_up;
    net_xfers->bytes_left_down = global_prefs.max_bytes_sec_down;
}

int CLIENT_STATE::init() {
    int retval;
    unsigned int i;

    srand(time(NULL));

    // parse account files.
    // If there are none, prompt user for project URL and create file
    //
    retval = parse_account_files();
    if (projects.size() == 0) {
        retval = add_new_project();
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
    clear_host_info(host_info);
    parse_state_file();

    if (log_flags.state_debug) {
        print_summary();
    }

    // Read the global preferences file, if it exists.
    // Do this after reading the state file so we know our venue
    //
    retval = global_prefs.parse_file(host_venue);
    if (retval) {
        printf("Using default preferences.\n");
    }
    install_global_prefs();

    // Getting host info is very fast, so we can do it anytime
    //
    get_host_info(host_info);

    // running CPU benchmarks is slow, so do it infrequently
    //
    if (gstate.should_run_time_tests()) {
        time_tests_start = time(0);
        show_message(NULL, "Running CPU benchmarks", MSG_INFO);
#ifdef _WIN32
        time_tests_handle = CreateThread(
            NULL, 0, win_time_tests, NULL, 0, &time_tests_id
        );
#else
        time_tests_id = fork();
        if (time_tests_id == 0) {
            _exit(time_tests());
        }
#endif
    }

    // Set nslots to actual # of CPUs (or less, depending on prefs?)
    //
    if (gstate.host_info.p_ncpus > 0) {
        nslots = gstate.host_info.p_ncpus;
    } else {
        nslots = 1;
    }

    // set up the project and slot directories
    //
    retval = make_project_dirs();
    if (retval) return retval;
    retval = make_slot_dirs();
    if (retval) return retval;

    // Restart any tasks that were running when we last quit the client
    //
    gstate.restart_tasks();

    // If we're supposed to update prefs, arrange to contact all projects
    //
    if (update_prefs) {
        for (i=0; i<projects.size(); i++) {
            projects[i]->sched_rpc_pending = true;
        }
    }

    return 0;
}

// Returns true if time tests should be run
// flag or if it's been a month since we last checked time stats
//
bool CLIENT_STATE::should_run_time_tests() {
    return (
        difftime(time(0), (time_t)host_info.p_calculated) > BENCHMARK_PERIOD
    );
}

#ifdef _WIN32
DWORD WINAPI CLIENT_STATE::win_time_tests(LPVOID) {
    return gstate.time_tests();
}
#endif

// gets info about the host
// NOTE: this locks up the process for 10-20 seconds,
// so it should be called very seldom
//
int CLIENT_STATE::time_tests() {
    HOST_INFO host_info;
    FILE* finfo;
    double fpop_test_secs = 2.0;
    double iop_test_secs = 2.0;
    double mem_test_secs = 2.0;

    clear_host_info(host_info);
    if (log_flags.measurement_debug) {
        printf("Running time tests.\n");
    }
    if (run_time_test) {
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

            // need to check check cache!!
        host_info.m_cache = 1e6;
    } else {
        if (log_flags.measurement_debug) {
            printf("Using fake performance numbers\n");
        }
        host_info.p_fpops = 1e9;
        host_info.p_iops = 1e9;
        host_info.p_membw = 4e9;
        host_info.m_cache = 1e6;
    }

    host_info.p_calculated = (double)time(0);
    finfo = fopen(TIME_TESTS_FILE_NAME, "w");
    if(!finfo) return ERR_FOPEN;
    host_info.write_time_tests(finfo);
    fclose(finfo);

    return 0;
}

// checks if the time tests are running
//
int CLIENT_STATE::check_time_tests() {
    FILE* finfo;
    int retval;
    
    if (time_tests_id) {
#ifdef _WIN32
        DWORD exit_code = 0;
        GetExitCodeThread(time_tests_handle, &exit_code);
        if(exit_code == STILL_ACTIVE) {
            if(time(NULL) > time_tests_start + MAX_TIME_TESTS_SECONDS) {
                show_message(NULL, "CPU benchmarks timed out, using default values", MSG_ERROR);
                TerminateThread(time_tests_handle, 0);
                CloseHandle(time_tests_handle);
                host_info.p_fpops = 1e9;
                host_info.p_iops = 1e9;
                host_info.p_membw = 4e9;
                host_info.m_cache = 1e6;
                time_tests_id = 0;
                return TIME_TESTS_ERROR;
            }
            return TIME_TESTS_RUNNING;
        }
        CloseHandle(time_tests_handle);
#else
        int exit_code = 0;
        retval = waitpid(time_tests_id, &exit_code, WNOHANG);
        if(retval == 0) {
            if((unsigned int)time(NULL) > time_tests_start + MAX_TIME_TESTS_SECONDS) {
                show_message(NULL, "CPU benchmarks timed out, using default values", MSG_ERROR);
                kill(time_tests_id, SIGKILL);
                host_info.p_fpops = 1e9;
                host_info.p_iops = 1e9;
                host_info.p_membw = 4e9;
                host_info.m_cache = 1e6;
                time_tests_id = 0;
                return TIME_TESTS_ERROR;
            }
            return TIME_TESTS_RUNNING;
        }
#endif
        time_tests_id = 0;
        show_message(NULL, "CPU benchmarks complete", MSG_INFO);
        finfo = fopen(TIME_TESTS_FILE_NAME, "r");
        if (!finfo) {
            show_message(NULL, "Can't open CPU benchmark file, using default values", MSG_ERROR);
            host_info.p_fpops = 1e9;
            host_info.p_iops = 1e9;
            host_info.p_membw = 4e9;
            host_info.m_cache = 1e6;
            return TIME_TESTS_ERROR;
        }
        retval = host_info.parse_time_tests(finfo);
        fclose(finfo);
        if (retval) return TIME_TESTS_ERROR;
        file_delete(TIME_TESTS_FILE_NAME);
        return TIME_TESTS_COMPLETE;
    }
    return TIME_TESTS_NOT_RUNNING;
}

// Return the maximum allowed disk usage as determined by user preferences.
// Since there are three different settings in the prefs, it returns the least
// of the three.
//
int CLIENT_STATE::allowed_disk_usage(double& size) {
    double percent_space, min_val;

    // Calculate allowed disk usage based on % pref
    //
    percent_space = host_info.d_total*global_prefs.disk_max_used_pct;

    min_val = host_info.d_free - global_prefs.disk_min_free_gb*1e9;

    // Return the minimum of the three
    //
    size = min(min(global_prefs.disk_max_used_gb*1e9, percent_space), min_val);
    if(size < 0) size = 0;
    return 0;
}

int CLIENT_STATE::project_disk_usage(PROJECT* p, double& size) {
    char buf[256],buf2[256];

    escape_project_url(p->master_url, buf);
    sprintf(buf2, "%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf);

    return dir_size(buf2, size);
}

int CLIENT_STATE::current_disk_usage(double& size) {
    return dir_size(".", size);
}

// See if (on the basis of user prefs) we should suspend activities.
// If so, suspend tasks
//
int CLIENT_STATE::check_suspend_activities() {
    bool should_suspend = false;
    char susp_msg[256];
    if (!global_prefs.run_on_batteries && host_is_running_on_batteries()) {
        sprintf(susp_msg, "Suspending activity - on batteries");
        should_suspend = true;
    }

    // user_idle and suspend_requested are set in the Mac/Win GUI code
    if (!user_idle) {
        should_suspend = true;
        sprintf(susp_msg, "Suspending activity - user is active");
    }
    if (suspend_requested) {
        should_suspend = true;
        sprintf(susp_msg, "Suspending activity - user request");
    }

    if (should_suspend) {
        if (!activities_suspended) {
            if (log_flags.task_debug) printf("SUSPENDING ACTIVITIES\n");
            active_tasks.suspend_all();
            show_message(NULL, susp_msg, MSG_INFO);
        }
    } else {
        if (activities_suspended) {
            if (log_flags.task_debug) printf("UNSUSPENDING ACTIVITIES\n");
            active_tasks.unsuspend_all();
            show_message(NULL, "Resuming activity", MSG_INFO);
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

int CLIENT_STATE::net_sleep(double x) {
    if (activities_suspended) {
        boinc_sleep(x);
        return 0;
    } else {
        return net_xfers->net_sleep(x);
    }
}

// do_something polls each of the client's finite-state machine layers,
// possibly triggering state transitions.
// Returns true if something happened
// (in which case should call this again immediately)
//
bool CLIENT_STATE::do_something() {
    bool action = false, x;

    if (check_time_tests() == TIME_TESTS_RUNNING) return false;

    check_suspend_activities();

    print_log("Polling; active layers:\n");
    net_stats.poll(*net_xfers);
    if (activities_suspended) {
        print_log("None (suspended)\n");
    } else {
        // Call these functions in bottom to top order with
        // respect to the FSM hierarchy

        x = net_xfers->poll();
        if (x) { action=true; print_log("net_xfers\n"); }

        x = http_ops->poll();
        if (x) {action=true; print_log("http_ops\n"); }

        x = file_xfers->poll();
        if (x) {action=true; print_log("file_xfers\n"); }

        x = active_tasks.poll();
        if (x) {action=true; print_log("active_tasks::poll\n"); }

        x = active_tasks.poll_time();
        if (x) {action=true; print_log("active_tasks::poll_time\n"); }

        x = scheduler_rpc_poll();
        if (x) {action=true; print_log("scheduler_rpc\n"); }

        x = start_apps();
        if (x) {action=true; print_log("start_apps\n"); }

        x = pers_xfers->poll();
        if (x) {action=true; print_log("pers_xfers\n"); }

        x = handle_running_apps();
        if (x) {action=true; print_log("handle_running_apps\n"); }

        x = handle_pers_file_xfers();
        if (x) {action=true; print_log("handle_pers_file_xfers\n"); }

        x = garbage_collect();
        if (x) {action=true; print_log("garbage_collect\n"); }

        x = update_results();
        if (x) {action=true; print_log("update_results\n"); }

        if (write_state_file_if_needed()) {
            fprintf(stderr, "Couldn't write state file");
        }
    }
    print_log("End poll\n");
    if (!action) {
        time_stats.update(true, !activities_suspended);
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
    int failnum;

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
                if (fip->had_failure(failnum)) {
                    if (fip->pers_file_xfer) delete fip->pers_file_xfer;
                    fip->pers_file_xfer = NULL;
                }
                // Init PERS_FILE_XFER and push it onto pers_file_xfer stack
                if (fip->pers_file_xfer) {
                    fip->pers_file_xfer->init(fip, fip->upload_when_present);
                    retval = pers_xfers->insert( fip->pers_file_xfer );
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
            retval = host_info.parse(f);
            if (retval) goto done;
        } else if (match_tag(buf, "<time_stats>")) {
            retval = time_stats.parse(f);
            if (retval) goto done;
        } else if (match_tag(buf, "<net_stats>")) {
            retval = net_stats.parse(f);
            if (retval) goto done;
        } else if (match_tag(buf, "<active_task_set>")) {
            retval = active_tasks.parse(f, this);
            if (retval) goto done;
        } else if (match_tag(buf, "<platform_name>")) {
            // should match our current platform name
        } else if (match_tag(buf, "<version>")) {
            // could put logic here to detect incompatible state files
            // after core client update
        } else if (match_tag(buf, "<core_client_major_version>")) {
        } else if (match_tag(buf, "<core_client_minor_version>")) {
        } else if (match_tag(buf, "<use_http_proxy/>")) {
            use_http_proxy = true;
        } else if (parse_str(buf, "<proxy_server_name>", proxy_server_name, sizeof(proxy_server_name))) {
        } else if (parse_int(buf, "<proxy_server_port>", proxy_server_port)) {
        } else if (parse_str(buf, "<socks_user_name>", socks_user_name, sizeof(socks_user_name))) {
        } else if (parse_str(buf, "<socks_user_passwd>", socks_user_passwd, sizeof(socks_user_passwd))) {
        } else if (match_tag(buf, "<suspend_requested/>")) {
            suspend_requested = true;
        } else if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) {
        } else {
            fprintf(stderr, "CLIENT_STATE::parse_state_file: unrecognized: %s\n", buf);
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
    retval = host_info.write(f);
    if (retval) return retval;
    retval = time_stats.write(f, false);
    if (retval) return retval;
    retval = net_stats.write(f, false);
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
    fprintf(f,
        "<platform_name>%s</platform_name>\n"
        "<core_client_major_version>%d</core_client_major_version>\n"
        "<core_client_minor_version>%d</core_client_minor_version>\n",
        platform_name,
        core_client_major_version,
        core_client_minor_version
    );

    // save proxy info
    //
    fprintf(f,
        "%s"
        "%s"
        "<proxy_server_name>%s</proxy_server_name>\n"
        "<proxy_server_port>%d</proxy_server_port>\n"
        "<socks_user_name>%s</socks_user_name>\n"
        "<socks_user_passwd>%s</socks_user_passwd>\n",
        use_http_proxy?"<use_http_proxy/>\n":"",
        use_socks_proxy?"<use_socks_proxy/>\n":"",
        proxy_server_name,
        proxy_server_port,
        socks_user_name,
        socks_user_passwd
    );
    // Save user suspend requests
    if (suspend_requested) {
        fprintf(f, "<suspend_requested/>\n");
    }
    if (strlen(host_venue)) {
        fprintf(f, "<host_venue>%s</host_venue>\n", host_venue);
    }
    fprintf(f, "</client_state>\n");
    fclose(f);
    retval = boinc_rename(STATE_FILE_TEMP, STATE_FILE_NAME);
    if (log_flags.state_debug) {
        printf("Done writing state file\n");
    }
    if (retval) return ERR_RENAME;
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
    if (best < 0) {
        fprintf(stderr, "CLIENT_STATE::latest_version_num: no version\n");
    }
    return best;
}

// Print debugging information about how many projects/files/etc
// are currently in the client state record
//
void CLIENT_STATE::print_summary() {
    unsigned int i;
    int t;
    if (!log_flags.state_debug) return;

    printf("Client state summary:\n");
    printf("  %d projects\n", (int)projects.size());
    for (i=0; i<projects.size(); i++) {
        printf("    %s", projects[i]->master_url);
        t = projects[i]->min_rpc_time;
        if (t) {
            printf(" min RPC %d seconds from now", (int)(t-time(0)));
        }
        printf("\n");
    }
    printf("  %d file_infos\n", (int)file_infos.size());
    for (i=0; i<file_infos.size(); i++) {
        printf("    %s status:%d %s\n", file_infos[i]->name, file_infos[i]->status, file_infos[i]->pers_file_xfer?"active":"inactive");
    }
    printf("  %d app_versions\n", (int)app_versions.size());
    for (i=0; i<app_versions.size(); i++) {
        printf("    %s %d\n", app_versions[i]->app_name, app_versions[i]->version_num);
    }
    printf("  %d workunits\n", (int)workunits.size());
    for (i=0; i<workunits.size(); i++) {
        printf("    %s\n", workunits[i]->name);
    }
    printf("  %d results\n", (int)results.size());
    for (i=0; i<results.size(); i++) {
        printf("    %s state:%d\n", results[i]->name, results[i]->state);
    }
    printf("  %d persistent file xfers\n", (int)pers_xfers->pers_file_xfers.size());
    for (i=0; i<pers_xfers->pers_file_xfers.size(); i++) {
        printf("    %s http op state: %d\n", pers_xfers->pers_file_xfers[i]->fip->name, (pers_xfers->pers_file_xfers[i]->fxp?pers_xfers->pers_file_xfers[i]->fxp->http_op_state:-1));
    }
    printf("  %d active tasks\n", (int)active_tasks.active_tasks.size());
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        printf("    %s\n", active_tasks.active_tasks[i]->result->name);
    }
}

// delete unneeded records and files
//
bool CLIENT_STATE::garbage_collect() {
    unsigned int i, j;
    int failnum;
    FILE_INFO* fip;
    RESULT* rp;
    WORKUNIT* wup;
    APP_VERSION* avp, *avp2;
    vector<RESULT*>::iterator result_iter;
    vector<WORKUNIT*>::iterator wu_iter;
    vector<FILE_INFO*>::iterator fi_iter;
    vector<APP_VERSION*>::iterator avp_iter;
    bool action = false, found;
    
    // zero references counts on WUs, FILE_INFOs and APP_VERSIONs
    for (i=0; i<workunits.size(); i++) {
        wup = workunits[i];
        wup->ref_cnt = 0;
    }
    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        fip->ref_cnt = 0;
    }
    for (i=0; i<app_versions.size(); i++) {
        avp = app_versions[i];
        avp->ref_cnt = 0;
    }
    
    // delete RESULTs that have been finished and reported;
    // reference-count files referred to by other results
    //
    result_iter = results.begin();
    while (result_iter != results.end()) {
        rp = *result_iter;
        if (rp->server_ack) {
            if (log_flags.state_debug) printf("deleting result %s\n", rp->name);
            delete rp;
            result_iter = results.erase(result_iter);
            action = true;
        } else {
            // See if the files for this result's workunit had
            // any errors (MD5, RSA, etc)
            //
            if (rp->wup->had_failure(failnum)) {
                // If we don't already have an error for this file
                if (!rp->ready_to_ack) {
                    // the wu corresponding to this result
                    // had an error downloading some input file(s).
                    //
                    report_result_error(*rp, 0, "Couldn't get input files");
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
                if(rp->output_files[i].file_info->had_failure(failnum)) {
                    if (!rp->ready_to_ack) {
                        // had an error uploading a file for this result
                        //
                        report_result_error(*rp, 0, "Couldn't upload files");
                    }
                } else {
                    rp->output_files[i].file_info->ref_cnt++;
                }
            }
            result_iter++;
        }
    }

    // delete WORKUNITs not referenced by any result;
    // reference-count files and APP_VERSIONs referred to by other WUs
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
            wup->avp->ref_cnt++;
            wu_iter++;
        }
    }

    // go through APP_VERSIONs;
    // delete any not referenced by any WORKUNIT
    // and having a more recent version.
    //
    avp_iter = app_versions.begin();
    while (avp_iter != app_versions.end()) {
        avp = *avp_iter;
        if (avp->ref_cnt == 0) {
            found = false;
            for (j=0; j<app_versions.size(); j++) {
                avp2 = app_versions[j];
                if (avp2->app==avp->app && avp2->version_num>avp->version_num) {
                    found = true;
                    break;
                }
            }
            if (found) {
                delete avp;
                avp_iter = app_versions.erase(avp_iter);
                action = true;
            } else {
                avp_iter++;
            }
        } else {
            avp_iter++;
        }
    }

    // Then go through remaining APP_VERSIONs,
    // bumping refcnt of associated files.
    //
    for (i=0; i<app_versions.size(); i++) {
        avp = app_versions[i];
        for (j=0; j<avp->app_files.size(); j++) {
            avp->app_files[i].file_info->ref_cnt++;
        }
    }

    // delete FILE_INFOs (and corresponding files) that are not sticky
    // and are not referenced by any WORKUNIT, RESULT or APP_VERSION
    //
    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fip = *fi_iter;
        if (fip->ref_cnt==0 && !fip->sticky) {
            fip->delete_file();
            if (log_flags.state_debug) printf("deleting file %s\n", fip->name);
            delete fip;
            fi_iter = file_infos.erase(fi_iter);
            action = true;
        } else {
            fi_iter++;
        }
    }

    if (action && log_flags.state_debug) {
        print_summary();
    }

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
        // The result has been received by the scheduling
        // server.  It will be deleted on the next
        // garbage collection, which we trigger by
        // setting action to true
        if(rp->server_ack)
            action = true;
        
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
                rp->ready_to_ack = true;
                rp->state = RESULT_FILES_UPLOADED;
                action = true;
            }
            break;
            
        case RESULT_FILES_UPLOADED:
            // The transition to SERVER_ACK is performed in
            // handle_scheduler_reply()
            break;
        }
        result_iter++;
    }
    return action;
}


// Parse the command line arguments passed to the client
// NOTE: init() has not been called at this point
// (i.e. client_state.xml has not been parsed)
//
void CLIENT_STATE::parse_cmdline(int argc, char** argv) {
    int i;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-exit_when_idle")) {
            exit_when_idle = true;
        } else if (!strcmp(argv[i], "-no_time_test")) {
            run_time_test = false;
        } else if (!strcmp(argv[i], "-exit_after_app_start")) {
            exit_after_app_start_secs = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-giveup_after")) {
            giveup_after = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-min")) {
            global_prefs.run_minimized = true;
        } else if (!strcmp(argv[i], "-saver")) {
            start_saver = true;

        // the above options are private (i.e. not shown by -help)

        } else if (!strcmp(argv[i], "-update_prefs")) {
            update_prefs = true;
        } else if (!strcmp(argv[i], "-add_new_project")) {
            add_new_project();
        } else if (!strcmp(argv[i], "-version")) {
            printf( "%.2f %s\n", MAJOR_VERSION+(MINOR_VERSION/100.0), HOST );
            exit(0);
        } else if (!strcmp(argv[i], "-help")) {
            printf(
                "Usage: %s [options]\n"
                "    -version                show version info\n"
                "    -add_new_project        add project (will prompt for URL, account key)\n",
                argv[0]
            );
            exit(0);
        } else {
            printf("Unknown option: %s\n", argv[i]);
            exit(1);
        }
    }
}

void CLIENT_STATE::parse_env_vars() {
    char *p, temp[256];

    if ((p = getenv("HTTP_PROXY"))) {
        if (strlen(p) > 0) {
            use_http_proxy = true;
            parse_url(p, proxy_server_name, proxy_server_port, temp);
        }
    }

    if ((p = getenv("SOCKS_SERVER"))) {
        if (strlen(p) > 0) {
            use_socks_proxy = true;
            parse_url(p, proxy_server_name, proxy_server_port, temp);
        }
    }

    if ((p = getenv("SOCKS_USER"))) {
        safe_strncpy(socks_user_name, p, sizeof(socks_user_name));
    }

    if ((p = getenv("SOCKS_PASSWD"))) {
        safe_strncpy(socks_user_passwd, p, sizeof(socks_user_passwd));
    }
}

// Returns true if client should exit because of debugging criteria
// (timeout or idle)
//
bool CLIENT_STATE::time_to_exit() {
    if (!exit_when_idle && !exit_after_app_start_secs) return false;
    if (exit_after_app_start_secs
        && app_started
        && (difftime(time(0), app_started) >= exit_after_app_start_secs)
    ) {
        printf("exiting because time is up: %d\n", exit_after_app_start_secs);
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

// Call this when a result has a nonrecoverable error.
// Append a description of the error to the stderr_out field of the result.
//
// Go through the input and output files for this result
// and generates error messages for upload/download failures.
//
// This function is called in the following situations:
// 1. When the active_task could not start or restart,
//    in which case err_num is set to an OS-specific error_code.
//    and err_msg has an OS-supplied string.
// 2. when we fail in downloading an input file or uploading an output file,
//    in which case err_num and err_msg are zero.
// 3. When the active_task exits with a non_zero error code
//    or it gets signaled.
//
int CLIENT_STATE::report_result_error(
    RESULT& res, int err_num, char *err_msg
) {
    char buf[MAX_BLOB_LEN];
    unsigned int i;
    int failnum;
    
    // only do this once per result
    //
    if (res.ready_to_ack) {
        return 0;
    }
    
    res.ready_to_ack = true;

    scheduler_op->backoff(res.project, "Backing off because a result failed");

    sprintf(
        buf, 
        "<message>%s</message>\n"
        "<active_task_state>%d</active_task_state>\n"
        "<exit_status>%d</exit_status>\n"
        "<signal>%d</signal>\n",
        err_msg,
        res.active_task_state,
        res.exit_status,
        res.signal
    );
    
    if (strlen(res.stderr_out) + strlen(buf) < MAX_BLOB_LEN) {
        strcat(res.stderr_out, buf );
    }
    
    if ((res.state == RESULT_FILES_DOWNLOADED) && err_num) {            
        sprintf(buf,"<couldnt_start>%d</couldnt_start>\n", err_num);
        if (strlen(res.stderr_out) + strlen(buf) < MAX_BLOB_LEN) {
            strcat(res.stderr_out, buf );
        }
    }

    if (res.state == RESULT_NEW) {
        for (i=0;i<res.wup->input_files.size();i++) {
            if (res.wup->input_files[i].file_info->had_failure(failnum)) {
                sprintf(buf,
                    "<download_error>\n"
                    "    <file_name>%s</file_name>\n"
                    "    <error_code>%d</error_code>\n"
                    "</download_error>\n",
                    res.wup->input_files[i].file_info->name, failnum
                );
                if (strlen(res.stderr_out) + strlen(buf) < MAX_BLOB_LEN ) {
                    strcat( res.stderr_out, buf );
                }
            }
        }
    }

    if (res.state == RESULT_COMPUTE_DONE) {
        for (i=0; i<res.output_files.size(); i++) {
            if (res.output_files[i].file_info->had_failure(failnum)) {
                sprintf(buf,
                    "<upload_error>\n"
                    "    <file_name>%s</file_name>\n"
                    "    <error_code>%d</error_code>\n"
                    "</upload_error>\n",
                    res.output_files[i].file_info->name, failnum
                );
                if (strlen(res.stderr_out) + strlen(buf) < MAX_BLOB_LEN ) {
                    strcat( res.stderr_out, buf );
                }
            }
        }
    }
    return 0;
}
