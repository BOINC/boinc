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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#endif

#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "filesys.h"

#include "file_names.h"
#include "hostinfo.h"
#include "hostinfo_network.h"
#include "network.h"
#include "http.h"
#include "log_flags.h"
#include "client_msgs.h"
#include "client_state.h"

using std::max;
using std::string;
using std::vector;

CLIENT_STATE gstate;

CLIENT_STATE::CLIENT_STATE() {
    net_xfers = new NET_XFER_SET;
    http_ops = new HTTP_OP_SET(net_xfers);
    file_xfers = new FILE_XFER_SET(http_ops);
    pers_file_xfers = new PERS_FILE_XFER_SET(file_xfers);
    scheduler_op = new SCHEDULER_OP(http_ops);
    client_state_dirty = false;
    exit_when_idle = false;
    check_all_logins = false;
    return_results_immediately = false;
    allow_remote_gui_rpc = false;
    run_cpu_benchmarks = false;
    skip_cpu_benchmarks = false;
    file_xfer_giveup_period = PERS_GIVEUP;
    contacted_sched_server = false;
    activities_suspended = false;
    network_suspended = false;
    core_client_major_version = BOINC_MAJOR_VERSION;
    core_client_minor_version = BOINC_MINOR_VERSION;
    platform_name = HOSTTYPE;
    exit_after_app_start_secs = 0;
    app_started = 0;
    exit_before_upload = false;
    proxy_info.clear();
    show_projects = false;
    strcpy(detach_project_url, "");
    strcpy(main_host_venue, "");
    strcpy(attach_project_url, "");
    strcpy(attach_project_auth, "");
    user_run_request = USER_RUN_REQUEST_AUTO;
    user_network_request = USER_RUN_REQUEST_AUTO;
    started_by_screensaver = false;
    requested_exit = false;
    master_fetch_period = MASTER_FETCH_PERIOD;
    retry_base_period = RETRY_BASE_PERIOD;
    retry_cap = RETRY_CAP;
    master_fetch_retry_cap = MASTER_FETCH_RETRY_CAP;
    master_fetch_interval = MASTER_FETCH_INTERVAL;
    sched_retry_delay_min = SCHED_RETRY_DELAY_MIN;
    sched_retry_delay_max = SCHED_RETRY_DELAY_MAX;
    pers_retry_delay_min = PERS_RETRY_DELAY_MIN;
    pers_retry_delay_max = PERS_RETRY_DELAY_MAX;
    pers_giveup = PERS_GIVEUP;
    executing_as_daemon = false;
    redirect_io = false;
    cpu_sched_last_time = 0;
    cpu_sched_work_done_this_period = 0;
    must_schedule_cpus = true;
}

#if 0
// Deallocate memory.  Can be used to check for memory leaks.
// Turned off for now.
//
void CLIENT_STATE::free_mem() {
    vector<PROJECT*>::iterator proj_iter;
    vector<APP*>::iterator app_iter;
    vector<FILE_INFO*>::iterator fi_iter;
    vector<APP_VERSION*>::iterator av_iter;
    vector<WORKUNIT*>::iterator wu_iter;
    vector<RESULT*>::iterator res_iter;
    PROJECT *proj;
    APP *app;
    FILE_INFO *fi;
    APP_VERSION *av;
    WORKUNIT *wu;
    RESULT *res;

    proj_iter = projects.begin();
    while (proj_iter != projects.end()) {
        proj = projects[0];
        proj_iter = projects.erase(proj_iter);
        delete proj;
    }

    app_iter = apps.begin();
    while (app_iter != apps.end()) {
        app = apps[0];
        app_iter = apps.erase(app_iter);
        delete app;
    }

    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fi = file_infos[0];
        fi_iter = file_infos.erase(fi_iter);
        delete fi;
    }

    av_iter = app_versions.begin();
    while (av_iter != app_versions.end()) {
        av = app_versions[0];
        av_iter = app_versions.erase(av_iter);
        delete av;
    }

    wu_iter = workunits.begin();
    while (wu_iter != workunits.end()) {
        wu = workunits[0];
        wu_iter = workunits.erase(wu_iter);
        delete wu;
    }

    res_iter = results.begin();
    while (res_iter != results.end()) {
        res = results[0];
        res_iter = results.erase(res_iter);
        delete res;
    }

    active_tasks.free_mem();
}
#endif

int CLIENT_STATE::init() {
    int retval;
    unsigned int i;
    char buf[256];

    srand(time(NULL));

    language.read_language_file(LANGUAGE_FILE_NAME);

#ifdef _DEBUG
    msg_printf(
        NULL, MSG_INFO, "Starting BOINC client version %d.%02d for %s (DEBUG)",
        core_client_major_version, core_client_minor_version, platform_name
    );
#else
    msg_printf(
        NULL, MSG_INFO, "Starting BOINC client version %d.%02d for %s",
        core_client_major_version, core_client_minor_version, platform_name
    );
#endif


    if (executing_as_daemon) {
        msg_printf(NULL, MSG_INFO, "Option: Executing as a daemon");
    }

    relative_to_absolute("", buf);
    msg_printf(NULL, MSG_INFO, "Data directory: %s", buf);

    // if we are running as anybody other than localsystem
    // and executing as a daemon then app graphics won't work.
    // display a note at startup reminding user  of that.
    //
#ifdef _WIN32
    DWORD  buf_size = sizeof(buf);
    LPTSTR pbuf = buf;
    
    GetUserName(pbuf, &buf_size);
    if (executing_as_daemon && (0 != strcmp("SYSTEM", pbuf))) {
        msg_printf(NULL, MSG_INFO,
            "BOINC is running as a service and as a non-system user."
        );
        msg_printf(NULL, MSG_INFO,
            "No application graphics will be available."
        );
    }
#endif

    parse_account_files();

    // check for app_info.xml file in project dirs.
    // If find, read app info from there, set project.anonymous_platform
    //
    check_anonymous();

    // Parse the client state file,
    // ignoring any <project> tags (and associated stuff)
    // for projects with no account file
    //
    host_info.clear_host_info();
    parse_state_file();

    // Check to see if we can write the state file.
    //
    retval = write_state_file();
    if (retval) {
        msg_printf(NULL, MSG_INFO,
            "Couldn't modify the state file.  make sure you have permissions configured for "
            "BOINC to run on this system under this user account\n"
        );
        return retval;
    }

    // scan user prefs; create file records
    //
    parse_preferences_for_user_files();

    print_summary();
    do_cmdline_actions();

    if (core_client_major_version != old_major_version) {
        msg_printf(NULL, MSG_INFO,
            "State file has different major version (%d.%02d); resetting projects\n",
            old_major_version, old_minor_version
        );
        for (i=0; i<projects.size(); i++) {
            reset_project(projects[i]);
        }
    }

    if ((core_client_major_version == old_major_version) && (core_client_minor_version != old_minor_version)) {
        msg_printf(NULL, MSG_INFO,
            "Version Change Detected (%d.%02d -> %d.%02d); running CPU benchmarks\n",
            old_major_version, old_minor_version, core_client_major_version, core_client_minor_version
        );
        run_cpu_benchmarks = true;
    }

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->hostid) {
            msg_printf(p, MSG_INFO, "Host ID is %d", p->hostid);
        } else {
            msg_printf(p, MSG_INFO, "Host ID not assigned yet");
        }
    }

    // Read the global preferences file.
    // 1) read the prefs file to get the source project
    // 2) get the main host venue (venue of source project)
    // 3) read the prefs file again, using that venue
    //
    bool found_venue;
    retval = global_prefs.parse_file(GLOBAL_PREFS_FILE_NAME, "", found_venue);
    if (retval) {
        msg_printf(NULL, MSG_INFO,
            "No general preferences found - using BOINC defaults"
        );
    } else {
        PROJECT* p = global_prefs_source_project();
        if (p) {
            strcpy(main_host_venue, p->host_venue);
            retval = global_prefs.parse_file(
                GLOBAL_PREFS_FILE_NAME, p->host_venue, found_venue
            );
        }
        show_global_prefs_source(found_venue);
    }
    install_global_prefs();

    // Getting host info is very fast, so we can do it anytime
    //
    host_info.get_host_info();

    set_ncpus();

    // set period start time and reschedule
    //
    must_schedule_cpus = true;
    cpu_sched_last_time = dtime();

    // set up the project and slot directories
    //
    retval = make_project_dirs();
    if (retval) return retval;

    // Just to be on the safe side; something may have been modified
    //
    set_client_state_dirty("init");

    retval = gui_rpcs.init();
    if (retval) return retval;

    return 0;
}

// sleep up to x seconds,
// but if network I/O becomes possible,
// wake up and do as much as limits allow.
// If suspended, just sleep x seconds.
// This gets called from the Win GUI to allow high network throughput.
// NOTE: when a socket is connecting, this gets called repeatedly,
// because the NetActivity messages seems to get sent repeatedly.
// This is inefficient but not a problem (I guess)
//
int CLIENT_STATE::net_sleep(double x) {
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);
    scope_messages.printf("CLIENT_STATE::net_sleep(%f)\n", x);
    if (activities_suspended || network_suspended) {
        boinc_sleep(x);
        return 0;
    } else {
        return net_xfers->net_sleep(x);
    }
}

#define POLL_ACTION(name, func) \
    do { if (func(now)) { \
            ++actions; \
            scope_messages.printf("CLIENT_STATE::do_something(): active task: " #name "\n"); \
        } } while(0)

// do_something polls each of the client's finite-state machine layers,
// possibly triggering state transitions.
// Returns true if something happened
// (in which case should call this again immediately)
//
bool CLIENT_STATE::do_something(double now) {
    int actions = 0, suspend_reason, retval;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_POLL);
    static bool tasks_restarted = false;

    if (should_run_cpu_benchmarks() && !are_cpu_benchmarks_running()) {
        run_cpu_benchmarks = false;
        start_cpu_benchmarks();
    }

    check_suspend_activities(now, suspend_reason);

    // Restart tasks on startup.
    // Do this here (rather than CLIENT_STATE::init())
    // so that if we do benchmark on startup,
    // we don't immediately suspend apps
    // (this fixes a CPDN problem where quitting the app
    // right after start kills it)
    //
    if (!suspend_reason && !tasks_restarted) {
        restart_tasks();
        tasks_restarted = true;
    }

    // suspend or resume activities (but only if already did startup)
    //
    if (tasks_restarted) {
        if (suspend_reason) {
            if (!activities_suspended) {
                suspend_activities(suspend_reason);
            }
        } else {
            if (activities_suspended) {
                resume_activities();
            }
        }
    }
    activities_suspended = (suspend_reason != 0);

    // if we're doing CPU benchmarks, don't do much else
    //
    if (suspend_reason & SUSPEND_REASON_BENCHMARKS) {
        // wait for applications to become suspended
        //
        if (active_tasks.is_task_executing()) {
            POLL_ACTION(active_tasks, active_tasks.poll);
        } else {
            cpu_benchmarks_poll();
        }
        return gui_rpcs.poll(dtime());
    }

    check_suspend_network(now, suspend_reason);
    if (suspend_reason) {
        if (!network_suspended) {
            suspend_network(suspend_reason);
        }
    } else {
        if (network_suspended) {
            resume_network();
        }
    }
    network_suspended = (suspend_reason != 0);

    scope_messages.printf("CLIENT_STATE::do_something(): Begin poll:\n");
    ++scope_messages;

    // NOTE:
    // The order of calls in the following lists generally doesn't matter,
    // except for the following:
    // must have:
    //  active_tasks_poll
    //  handle_finished_apps
    //  schedule_cpus
    // in that order (active_tasks_poll() sets must_schedule_cpus,
    // and handle_finished_apps() must be done before schedule_cpus()

    ss_logic.poll();
    if (activities_suspended) {
        scope_messages.printf("CLIENT_STATE::do_something(): activities suspended\n");
        //POLL_ACTION(data_manager           , data_manager_poll      );
        POLL_ACTION(net_xfers              , net_xfers->poll        );
        POLL_ACTION(http_ops               , http_ops->poll         );
        POLL_ACTION(active_tasks           , active_tasks.poll      );
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
        POLL_ACTION(garbage_collect        , garbage_collect        );
        POLL_ACTION(update_results         , update_results         );
        POLL_ACTION(gui_rpc                , gui_rpcs.poll          );
        POLL_ACTION(acct_mgr                , acct_mgr.poll          );
    } else if (network_suspended) {
        scope_messages.printf("CLIENT_STATE::do_something(): network suspended\n");
        //POLL_ACTION(data_manager           , data_manager_poll      );
        POLL_ACTION(net_xfers              , net_xfers->poll        );
        POLL_ACTION(http_ops               , http_ops->poll         );
        POLL_ACTION(active_tasks           , active_tasks.poll      );
        POLL_ACTION(handle_finished_apps   , handle_finished_apps   );
        POLL_ACTION(schedule_cpus          , schedule_cpus          );
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
        POLL_ACTION(garbage_collect        , garbage_collect        );
        POLL_ACTION(update_results         , update_results         );
        POLL_ACTION(gui_rpc                , gui_rpcs.poll          );        
        POLL_ACTION(acct_mgr                , acct_mgr.poll          );
    } else {
        net_stats.poll(*net_xfers);
        //
        //POLL_ACTION(data_manager           , data_manager_poll      );
        POLL_ACTION(net_xfers              , net_xfers->poll        );
        POLL_ACTION(http_ops               , http_ops->poll         );
        POLL_ACTION(file_xfers             , file_xfers->poll       );
        POLL_ACTION(active_tasks           , active_tasks.poll      );
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
        POLL_ACTION(pers_file_xfers        , pers_file_xfers->poll  );
        POLL_ACTION(handle_finished_apps   , handle_finished_apps   );
        POLL_ACTION(schedule_cpus          , schedule_cpus          );
        POLL_ACTION(handle_pers_file_xfers , handle_pers_file_xfers );
        POLL_ACTION(garbage_collect        , garbage_collect        );
        POLL_ACTION(update_results         , update_results         );
        POLL_ACTION(gui_rpc                , gui_rpcs.poll          );
        POLL_ACTION(acct_mgr                , acct_mgr.poll          );
    }

    retval = write_state_file_if_needed();
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "Couldn't write state file: %d", retval);
    }
    --log_messages;
    scope_messages.printf(
        "CLIENT_STATE::do_something(): End poll: %d tasks active\n", actions
    );
    if (actions > 0) {
        return true;
    } else {
        int connected_state = get_connected_state();
        time_stats.update(now, connected_state, !activities_suspended);
        return false;
    }
}

// See if the project specified by master_url already exists
// in the client state record.  Ignore any trailing "/" characters
//
PROJECT* CLIENT_STATE::lookup_project(const char* master_url) {
    int len1, len2;
    char *mu;

    len1 = strlen(master_url);
    if (master_url[strlen(master_url)-1] == '/') len1--;

    for (unsigned int i=0; i<projects.size(); i++) {
        mu = projects[i]->master_url;
        len2 = strlen(mu);
        if (mu[strlen(mu)-1] == '/') len2--;
        if (!strncmp(master_url, projects[i]->master_url, max(len1,len2))) {
            return projects[i];
        }
    }
    return 0;
}

APP* CLIENT_STATE::lookup_app(PROJECT* p, const char* name) {
    for (unsigned int i=0; i<apps.size(); i++) {
        APP* app = apps[i];
        if (app->project == p && !strcmp(name, app->name)) return app;
    }
    return 0;
}

RESULT* CLIENT_STATE::lookup_result(PROJECT* p, const char* name) {
    for (unsigned int i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->project == p && !strcmp(name, rp->name)) return rp;
    }
    return 0;
}

WORKUNIT* CLIENT_STATE::lookup_workunit(PROJECT* p, const char* name) {
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

FILE_INFO* CLIENT_STATE::lookup_file_info(PROJECT* p, const char* name) {
    for (unsigned int i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->project == p && !strcmp(fip->name, name)) {
            return fip;
        }
    }
    return 0;
}

// Find the active task for a given result
//
ACTIVE_TASK* CLIENT_STATE::lookup_active_task_by_result(RESULT* rep) {
    for (unsigned int i = 0; i < active_tasks.active_tasks.size(); i ++) {
        if (active_tasks.active_tasks[i]->result == rep) {
            return active_tasks.active_tasks[i];
        }
    }
    return NULL;
}

// functions to create links between state objects
// (which, in their XML form, reference one another by name)
// Return nonzero if already in client state.
//
int CLIENT_STATE::link_app(PROJECT* p, APP* app) {
    if (lookup_app(p, app->name)) return ERR_NOT_UNIQUE;
    app->project = p;
    return 0;
}

int CLIENT_STATE::link_file_info(PROJECT* p, FILE_INFO* fip) {
    if (lookup_file_info(p, fip->name)) return ERR_NOT_UNIQUE;
    fip->project = p;
#if 0
    // I got rid of the from_server arg
    if (from_server) {
        if (p->associate_file(fip)) {
            return 0;
        } else {
            return 1;
        }
    }
#endif

    return 0;
}

int CLIENT_STATE::link_app_version(PROJECT* p, APP_VERSION* avp) {
    APP* app;
    FILE_INFO* fip;
    unsigned int i;

    avp->project = p;
    app = lookup_app(p, avp->app_name);
    if (!app) {
        msg_printf(p, MSG_ERROR, "app_version refers to nonexistent app: %s\n", avp->app_name);
        return ERR_NOT_FOUND;
    }
    avp->app = app;

    if (lookup_app_version(app, avp->version_num)) return ERR_NOT_UNIQUE;

    for (i=0; i<avp->app_files.size(); i++) {
        FILE_REF& file_ref = avp->app_files[i];
        fip = lookup_file_info(p, file_ref.file_name);
        if (!fip) {
            msg_printf(p, MSG_ERROR,
                "app_version refers to nonexistent file: %s\n",
                file_ref.file_name
            );
            return ERR_NOT_FOUND;
        }

        // any file associated with an app version must be signed
        //
        fip->signature_required = true;
        file_ref.file_info = fip;
    }
    return 0;
}

int CLIENT_STATE::link_file_ref(PROJECT* p, FILE_REF* file_refp) {
    FILE_INFO* fip;

    fip = lookup_file_info(p, file_refp->file_name);
    if (!fip) {
        msg_printf(p, MSG_ERROR, "File ref refers to nonexistent file: %s\n", file_refp->file_name);
        return ERR_NOT_FOUND;
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
        msg_printf(p, MSG_ERROR, "WU refers to nonexistent app: %s\n", wup->app_name);
        return ERR_NOT_FOUND;
    }
    avp = lookup_app_version(app, wup->version_num);
    if (!avp) {
        msg_printf(p, MSG_ERROR,
            "WU refers to nonexistent app_version: %s %d\n",
            wup->app_name, wup->version_num
        );
        return ERR_NOT_FOUND;
    }
    wup->project = p;
    wup->app = app;
    wup->avp = avp;
    for (i=0; i<wup->input_files.size(); i++) {
        retval = link_file_ref(p, &wup->input_files[i]);
        if (retval) {
            msg_printf(p, MSG_ERROR,
                "WU refers to nonexistent file: %s\n",
                wup->input_files[i].file_name
            );
            return retval;
        }
    }
    return 0;
}

int CLIENT_STATE::link_result(PROJECT* p, RESULT* rp) {
    WORKUNIT* wup;
    unsigned int i;
    int retval;

    wup = lookup_workunit(p, rp->wu_name);
    if (!wup) {
        msg_printf(p, MSG_ERROR, "link_result: nonexistent WU %s\n", rp->wu_name);
        return ERR_NOT_FOUND;
    }
    rp->project = p;
    rp->wup = wup;
    rp->app = wup->app;
    for (i=0; i<rp->output_files.size(); i++) {
        retval = link_file_ref(p, &rp->output_files[i]);
        if (retval) {
            msg_printf(p, MSG_ERROR, "link_result: link_file_ref failed\n");
            return retval;
        }
    }
    return 0;
}

// Print debugging information about how many projects/files/etc
// are currently in the client state record
//
void CLIENT_STATE::print_summary() {
    unsigned int i;
    double t, now=dtime();
    if (!log_flags.state_debug) return;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);
    scope_messages.printf("CLIENT_STATE::print_summary(): Client state summary:\n");
    ++log_messages;
    scope_messages.printf("%d projects:\n", (int)projects.size());
    for (i=0; i<projects.size(); i++) {
        t = projects[i]->min_rpc_time;
        if (t) {
            scope_messages.printf("    %s min RPC %f.0 seconds from now\n", projects[i]->master_url, t-now);
        } else {
            scope_messages.printf("    %s\n", projects[i]->master_url);
        }
    }
    scope_messages.printf("%d file_infos:\n", (int)file_infos.size());
    for (i=0; i<file_infos.size(); i++) {
        scope_messages.printf("    %s status:%d %s\n", file_infos[i]->name, file_infos[i]->status, file_infos[i]->pers_file_xfer?"active":"inactive");
    }
    scope_messages.printf("%d app_versions\n", (int)app_versions.size());
    for (i=0; i<app_versions.size(); i++) {
        scope_messages.printf("    %s %d\n", app_versions[i]->app_name, app_versions[i]->version_num);
    }
    scope_messages.printf("%d workunits\n", (int)workunits.size());
    for (i=0; i<workunits.size(); i++) {
        scope_messages.printf("    %s\n", workunits[i]->name);
    }
    scope_messages.printf("%d results\n", (int)results.size());
    for (i=0; i<results.size(); i++) {
        scope_messages.printf("    %s state:%d\n", results[i]->name, results[i]->state);
    }
    scope_messages.printf("%d persistent file xfers\n", (int)pers_file_xfers->pers_file_xfers.size());
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        scope_messages.printf("    %s http op state: %d\n", pers_file_xfers->pers_file_xfers[i]->fip->name, (pers_file_xfers->pers_file_xfers[i]->fxp?pers_file_xfers->pers_file_xfers[i]->fxp->http_op_state:-1));
    }
    scope_messages.printf("%d active tasks\n", (int)active_tasks.active_tasks.size());
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        scope_messages.printf("    %s\n", active_tasks.active_tasks[i]->result->name);
    }
    --log_messages;
}

// delete unneeded records and files
//
bool CLIENT_STATE::garbage_collect(double now) {
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
    string error_msgs;
    PROJECT* project;
    char buf[1024];

    static double last_time=0;
    if (now>0) {
        if (now - last_time < 1.0) return false;
        last_time = now;
    }

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

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

    // reference-count project files
    //
    for (i=0; i<projects.size(); i++) {
        project = projects[i];
        for (j=0; j<project->user_files.size(); j++) {
            project->user_files[j].file_info->ref_cnt++;
        }
    }

    // Scan through RESULTs.
    // delete RESULTs that have been reported and acked.
    // Check for results whose WUs had download failures
    // Check for results that had upload failures
    // Reference-count output files
    // Reference-count WUs
    //
    result_iter = results.begin();
    while (result_iter != results.end()) {
        rp = *result_iter;
        if (rp->got_server_ack) {
            scope_messages.printf("CLIENT_STATE::garbage_collect(): deleting result %s\n", rp->name);
            delete rp;
            result_iter = results.erase(result_iter);
            action = true;
            continue;
        }
        // See if the files for this result's workunit had
        // any errors (download failure, MD5, RSA, etc)
        // and we don't already have an error for this result
        //
        if (!rp->ready_to_report) {
            wup = rp->wup;
            if (wup->had_download_failure(failnum)) {
                wup->get_file_errors(error_msgs);
                report_result_error(
                    *rp, "WU download error: %s", error_msgs.c_str()
                );
            } else if (wup->avp && wup->avp->had_download_failure(failnum)) {
                wup->avp->get_file_errors(error_msgs);
                report_result_error(
                    *rp, "app_version download error: %s", error_msgs.c_str()
                );
            }
        }
        bool found_error = false;
        std::string error_str;
        for (i=0; i<rp->output_files.size(); i++) {
            // If one of the output files had an upload failure,
            // mark the result as done and report the error.
            // The result, workunits, and file infos
            // will be cleaned up after the server is notified
            //
            if (!rp->ready_to_report) {
                fip = rp->output_files[i].file_info;
                if (fip->had_failure(failnum, buf)) {
                    found_error = true;
                    error_str += buf;
                }
            }
            rp->output_files[i].file_info->ref_cnt++;
        }
        if (found_error) {
            report_result_error(*rp, error_str.c_str());
        }
        rp->wup->ref_cnt++;
        result_iter++;
    }

    // delete WORKUNITs not referenced by any in-progress result;
    // reference-count files and APP_VERSIONs referred to by other WUs
    //
    wu_iter = workunits.begin();
    while (wu_iter != workunits.end()) {
        wup = *wu_iter;
        if (wup->ref_cnt == 0) {
            scope_messages.printf("CLIENT_STATE::garbage_collect(): deleting workunit %s\n", wup->name);
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
    // and superceded by a more recent version.
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
            avp->app_files[j].file_info->ref_cnt++;
        }
    }

    // reference count files involved in PERS_FILE_XFER or FILE_XFER
    // (this seems redundant, but apparently not)
    //
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        file_xfers->file_xfers[i]->fip->ref_cnt++;
    }
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        pers_file_xfers->pers_file_xfers[i]->fip->ref_cnt++;
    }

    // delete FILE_INFOs (and corresponding files) that are not referenced
    // Don't do this if sticky and not marked for delete
    //
    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fip = *fi_iter;
        bool exempt = fip->sticky;
        if (fip->status < 0) exempt = false;
        if (fip->marked_for_delete) exempt = false;
        if (fip->ref_cnt==0 && !exempt) {
            if (fip->pers_file_xfer) {
                pers_file_xfers->remove(fip->pers_file_xfer);
                delete fip->pers_file_xfer;
                fip->pers_file_xfer = 0;
            }
#if 0
            fip->project->size -= fip->nbytes;
#endif
            fip->delete_file();
            scope_messages.printf(
                "CLIENT_STATE::garbage_collect(): deleting file %s\n",
                fip->name
            );
            delete fip;
            fi_iter = file_infos.erase(fi_iter);
            action = true;
        } else {
            fi_iter++;
        }
    }

    if (action) {
        print_summary();
    }

    return action;
}

// update the state of results
//
bool CLIENT_STATE::update_results(double now) {
    RESULT* rp;
    vector<RESULT*>::iterator result_iter;
    bool action = false;
    static double last_time=0;

    if (now - last_time < 1.0) return false;
    last_time = 0;

    result_iter = results.begin();
    while (result_iter != results.end()) {
        rp = *result_iter;
        // The result has been acked by the scheduling server.
        // It will be deleted on the next garbage collection,
        if (rp->got_server_ack) {
            action = true;
        }

        switch (rp->state) {
        case RESULT_NEW:
            rp->state = RESULT_FILES_DOWNLOADING;
            action = true;
            break;
        case RESULT_FILES_DOWNLOADING:
            if (input_files_available(rp)) {
                rp->state = RESULT_FILES_DOWNLOADED;
                must_schedule_cpus = true;
                action = true;
            }
            break;
        case RESULT_FILES_UPLOADING:
            if (rp->is_upload_done()) {
                rp->ready_to_report = true;
                rp->state = RESULT_FILES_UPLOADED;
                action = true;
            }
            break;
        case RESULT_FILES_UPLOADED:
            break;
        }
        result_iter++;
    }
    return action;
}

// Returns true if client should exit because of debugging criteria
// (timeout or idle)
//
bool CLIENT_STATE::time_to_exit() {
    if (!exit_when_idle && !exit_after_app_start_secs) return false;
    if (exit_after_app_start_secs
        && (app_started>0)
        && ((dtime() - app_started) >= exit_after_app_start_secs)
    ) {
        msg_printf(NULL, MSG_INFO, "exiting because time is up: %d\n", exit_after_app_start_secs);
        return true;
    }
    if (exit_when_idle && (results.size() == 0) && contacted_sched_server) {
        msg_printf(NULL, MSG_INFO, "exiting because no more results\n");
        return true;
    }
    return false;
}

// Call this when a result has a nonrecoverable error.
// - back off on contacting the project's scheduler
//   (so don't crash over and over)
// - Append a description of the error to result.stderr_out
//
int CLIENT_STATE::report_result_error(
    RESULT& res, const char* format, ...
) {
    char buf[MAX_BLOB_LEN],  err_msg[MAX_BLOB_LEN];
    unsigned int i;
    int failnum;

    // only do this once per result
    //
    if (res.ready_to_report) {
        return 0;
    }

    res.ready_to_report = true;

    va_list va;
    va_start(va, format);
    vsnprintf(err_msg, sizeof(err_msg), format, va);
    va_end(va);

    sprintf(buf, "Unrecoverable error for result %s (%s)", res.name, err_msg);
    scheduler_op->backoff(res.project, buf);

    sprintf( buf, "<message>%s\n</message>\n", err_msg);
    res.stderr_out.append(buf);

    switch(res.state) {
    case RESULT_NEW:
    case RESULT_FILES_DOWNLOADING:
        // called from:
        // CLIENT_STATE::garbage_collect()
        //   if WU or app_version had a download failure
        //
        if (!res.exit_status) {
            res.exit_status = ERR_RESULT_DOWNLOAD;
        }
        break;

    case RESULT_FILES_DOWNLOADED:
        // called from:
        // ACTIVE_TASK::start (if couldn't start app)
        // ACTIVE_TASK::restart (if files missing)
        // ACITVE_TASK_SET::restart_tasks (catch other error returns)
        // ACTIVE_TASK::handle_exited_app (on nonzero exit or signal)
        // ACTIVE_TASK::abort_task (if exceeded resource limit)
        // CLIENT_STATE::schedule_cpus (catch-all for resume/start errors)
        //
        if (!res.exit_status) {
            res.exit_status = ERR_RESULT_START;
        }
        break;

    case RESULT_FILES_UPLOADING:
        // called from
        // CLIENT_STATE::garbage_collect() if result had an upload error
        //
        for (i=0; i<res.output_files.size(); i++) {
            if (res.output_files[i].file_info->had_failure(failnum)) {
                sprintf(buf,
                    "<upload_error>\n"
                    "    <file_name>%s</file_name>\n"
                    "    <error_code>%d</error_code>\n"
                    "</upload_error>\n",
                    res.output_files[i].file_info->name, failnum
                );
                res.stderr_out.append(buf);
            }
        }
        if (!res.exit_status) {
            res.exit_status = ERR_RESULT_UPLOAD;
        }
        break;
    case RESULT_COMPUTE_ERROR:
        break;
    case RESULT_FILES_UPLOADED:
        msg_printf(res.project, MSG_ERROR, "report_result_error() called unexpectedly");
        break;
    }

    res.stderr_out = res.stderr_out.substr(0,MAX_BLOB_LEN-1);
    return 0;
}

// "Reset" a project: (clear error conditions)
// - stop all active tasks
// - stop all file transfers
// - stop scheduler RPC if any
// - delete all workunits and results
// - delete all apps and app_versions
// - garbage collect to delete unneeded files
//
// Note: does NOT delete persistent files or user-supplied files;
// does not delete project dir
//
int CLIENT_STATE::reset_project(PROJECT* project) {
    unsigned int i;
    APP_VERSION* avp;
    APP* app;
    vector<APP*>::iterator app_iter;
    vector<APP_VERSION*>::iterator avp_iter;
    RESULT* rp;
    PERS_FILE_XFER* pxp;

    msg_printf(project, MSG_INFO, "Resetting project");
    active_tasks.abort_project(project);

    // TODO: close sockets and open FILEs; delete the various objects
    //
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        pxp = pers_file_xfers->pers_file_xfers[i];
        if (pxp->fip->project == project) {
            if (pxp->fxp) {
                file_xfers->remove(pxp->fxp);
            }
            pers_file_xfers->remove(pxp);
            i--;
        }
    }

    // if we're in the middle of a scheduler op to the project, abort it
    //
    if (scheduler_op->state != SCHEDULER_OP_STATE_IDLE
        && scheduler_op->project == project
    ) {
        http_ops->remove(&scheduler_op->http_op);
        scheduler_op->state = SCHEDULER_OP_STATE_IDLE;
    }

    // mark results as server-acked.
    // This will cause garbage_collect to delete them,
    // and in turn their WUs will be deleted
    //
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == project) {
            rp->got_server_ack = true;
        }
    }

    garbage_collect(0);

    // forcibly remove apps and app_versions
    // (but not if anonymous platform)
    //
    if (!project->anonymous_platform) {
        avp_iter = app_versions.begin();
        while (avp_iter != app_versions.end()) {
            avp = *avp_iter;
            if (avp->project == project) {
                avp_iter = app_versions.erase(avp_iter);
                delete avp;
            } else {
                avp_iter++;
            }
        }

        app_iter = apps.begin();
        while (app_iter != apps.end()) {
            app = *app_iter;
            if (app->project == project) {
                app_iter = apps.erase(app_iter);
                delete app;
            } else {
                app_iter++;
            }
        }
        garbage_collect(0);
    }

    write_state_file();
    return 0;
}

// "Detach" a project:
// - Reset (see above)
// - delete all file infos
// - delete account file
// - delete account directory
//
int CLIENT_STATE::detach_project(PROJECT* project) {
    vector<PROJECT*>::iterator project_iter;
    vector<FILE_INFO*>::iterator fi_iter;
    FILE_INFO* fip;
    PROJECT* p;
    char path[256];
    int retval;

    reset_project(project);

    msg_printf(project, MSG_INFO, "Detaching from project");

    // delete all FILE_INFOs associated with this project
    //
    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fip = *fi_iter;
        if (fip->project == project) {
            file_infos.erase(fi_iter);
            delete fip;
        } else {
            fi_iter++;
        }
    }

    // if global prefs came from this project, delete file and reinit
    //
    p = lookup_project(global_prefs.source_project);
    if (p == project) {
        boinc_delete_file(GLOBAL_PREFS_FILE_NAME);
        global_prefs.defaults();
    }

    // find project and remove it from the vector
    //
    for (project_iter = projects.begin(); project_iter != projects.end(); project_iter++) {
        p = *project_iter;
        if (p == project) {
            projects.erase(project_iter);
            break;
        }
    }

    // delete account file
    //
    get_account_filename(project->master_url, path);
    retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_ERROR, "Can't delete account file: %d\n", retval);
    }

    // remove project directory and its contents
    //
    retval = remove_project_dir(*project);
    if (retval) {
        msg_printf(project, MSG_ERROR, "Can't delete project directory: %d\n", retval);
    }
    delete project;
    write_state_file();

    return 0;
}

double CLIENT_STATE::total_resource_share() {
    unsigned int i;

    double x = 0;
    for (i=0; i<projects.size(); i++) {
        x += projects[i]->resource_share;
    }
    return x;
}

int CLIENT_STATE::version() {
    return core_client_major_version*100 + core_client_minor_version;
}

const char *BOINC_RCSID_e836980ee1 = "$Id$";
