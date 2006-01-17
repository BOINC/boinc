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
#include "config.h"
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
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
#include "http_curl.h"
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
    allow_remote_gui_rpc = false;
    cmdline_gui_rpc_port = 0;
    run_cpu_benchmarks = false;
    skip_cpu_benchmarks = false;
    file_xfer_giveup_period = PERS_GIVEUP;
    contacted_sched_server = false;
    tasks_suspended = false;
    network_suspended = false;
    core_client_major_version = BOINC_MAJOR_VERSION;
    core_client_minor_version = BOINC_MINOR_VERSION;
    core_client_release = BOINC_RELEASE;
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
    work_fetch_no_new_work = false;
    cpu_earliest_deadline_first = false;

    cpu_sched_last_time = 0;
    total_wall_cpu_time_this_period = 0;
    must_schedule_cpus = true;
    want_network_flag = false;
    have_sporadic_connection = false;
    no_gui_rpc = false;
    have_tentative_project = false;
    new_version_check_time = 0;
    detach_console = false;
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

void CLIENT_STATE::show_host_info() {
    char buf[256], buf2[256];
    msg_printf(NULL, MSG_INFO,
        "Processor: %d %s %s",
        host_info.p_ncpus, host_info.p_vendor, host_info.p_model
    );

    nbytes_to_string(host_info.m_nbytes, 0, buf, sizeof(buf));
    nbytes_to_string(host_info.m_swap, 0, buf2, sizeof(buf2));
    msg_printf(NULL, MSG_INFO,
        "Memory: %s physical, %s virtual",
        buf, buf2
    );

    nbytes_to_string(host_info.d_total, 0, buf, sizeof(buf));
    nbytes_to_string(host_info.d_free, 0, buf2, sizeof(buf2));
    msg_printf(NULL, MSG_INFO,
        "Disk: %s total, %s free",
        buf, buf2
    );
}

int CLIENT_STATE::init() {
    int retval;
    unsigned int i;
    char buf[256];
    PROJECT* p;

    srand((unsigned int)time(0));
    now = dtime();
    scheduler_op->url_random = drand();

    language.read_language_file(LANGUAGE_FILE_NAME);

    const char* debug_str="";
#ifdef _DEBUG
    debug_str = " (DEBUG)";
#endif
    msg_printf(
        NULL, MSG_INFO, "Starting BOINC client version %d.%d.%d for %s%s",
        core_client_major_version, core_client_minor_version,
        core_client_release, platform_name, debug_str
    );

    msg_printf(NULL, MSG_INFO, curl_version());

    if (executing_as_daemon) {
        msg_printf(NULL, MSG_INFO, "Executing as a daemon");
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

    // Parse various files
    parse_account_files();
    parse_statistics_files();

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

    host_info.get_host_info();
    set_ncpus();
    show_host_info();

    // Check to see if we can write the state file.
    //
    retval = write_state_file();
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "Couldn't write state file");
        msg_printf(NULL, MSG_ERROR,
            "Make sure you have permissions set correctly"
        );
        return retval;
    }

    // scan user prefs; create file records
    //
    parse_preferences_for_user_files();

    print_summary();
    do_cmdline_actions();

    if ((core_client_major_version != old_major_version)
        || (core_client_minor_version != old_minor_version)
        || (core_client_release != old_release)
    ) {
        msg_printf(NULL, MSG_INFO,
            "Version change (%d.%d.%d -> %d.%d.%d); running CPU benchmarks\n",
            old_major_version, old_minor_version, old_release,
            core_client_major_version, core_client_minor_version,
            core_client_release
        );
        run_cpu_benchmarks = true;
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->hostid) {
            sprintf(buf, "%d", p->hostid);
        } else {
            strcpy(buf, "not assigned yet");
        }
        msg_printf(p, MSG_INFO,
            "Computer ID: %s; location: %s; project prefs: %s",
            buf, p->host_venue,
            p->using_venue_specific_prefs?p->host_venue:"default"
        );
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
        p = global_prefs_source_project();
        if (p) {
            strcpy(main_host_venue, p->host_venue);
            retval = global_prefs.parse_file(
                GLOBAL_PREFS_FILE_NAME, p->host_venue, found_venue
            );
        }
        show_global_prefs_source(found_venue);
    }
    install_global_prefs();

    // set period start time and reschedule
    //
    must_schedule_cpus = true;
    cpu_sched_last_time = now;

    // set up the project and slot directories
    //
    delete_old_slot_dirs();
    retval = make_project_dirs();
    if (retval) return retval;

    active_tasks.report_overdue();
    active_tasks.handle_upload_files();

    // Just to be on the safe side; something may have been modified
    //
    set_client_state_dirty("init");

    // initialize GUI RPC data structures before we start accepting
    // GUI RPC's.
    //
    acct_mgr_info.init();
    project_init.init();

    if (!no_gui_rpc) {
        retval = gui_rpcs.init();
        if (retval) return retval;
    }

    return 0;
}

static void double_to_timeval(double x, timeval& t) {
    t.tv_sec = (int)x;
    t.tv_usec = (int)(1000000*(x - (int)x));
}

FDSET_GROUP curl_fds;
FDSET_GROUP gui_rpc_fds;
FDSET_GROUP all_fds;

// Spend x seconds either doing I/O (if possible) or sleeping.
//
void CLIENT_STATE::do_io_or_sleep(double x) {
    int n;
    struct timeval tv;
    now = dtime();
    double end_time = now + x;

    while (1) {
        curl_fds.zero();
        gui_rpc_fds.zero();
        net_xfers->get_fdset(curl_fds);
        all_fds = curl_fds;
        gui_rpcs.get_fdset(gui_rpc_fds, all_fds);
        double_to_timeval(x, tv);
        n = select(
            all_fds.max_fd+1,
            &all_fds.read_fds, &all_fds.write_fds, &all_fds.exc_fds,
            &tv
        );
        //printf("select in %d out %d\n", all_fds.max_fd, n);

        // Note: curl apparently likes to have curl_multi_perform()
        // (called from net_xfers->got_select())
        // called pretty often, even if no descriptors are enabled.
        // So do the "if (n==0) break" AFTER the got_selects().

        net_xfers->got_select(all_fds, x);
        gui_rpcs.got_select(all_fds);

        if (n==0) break;

        now = dtime();
        if (now > end_time) break;
        x = end_time - now;
    }
}

#define POLL_ACTION(name, func) \
    do { if (func()) { \
            ++actions; \
            scope_messages.printf("CLIENT_STATE::poll_slow_events(): " #name "\n"); \
        } } while(0)

// Poll the client's finite-state machines
// possibly triggering state transitions.
// Returns true if something happened
// (in which case should call this again immediately)
//
bool CLIENT_STATE::poll_slow_events() {
    int actions = 0, suspend_reason, network_suspend_reason, retval;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_POLL);
    static bool tasks_restarted = false;

    now = dtime();

    if (should_run_cpu_benchmarks() && !are_cpu_benchmarks_running()) {
        run_cpu_benchmarks = false;
        start_cpu_benchmarks();
    }

    check_suspend_activities(suspend_reason);

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
            if (!tasks_suspended) {
                suspend_tasks(suspend_reason);
            }
        } else {
            if (tasks_suspended) {
                resume_tasks();
            }
        }
    }
    tasks_suspended = (suspend_reason != 0);

    // if we're doing CPU benchmarks, don't do much else
    //
    if (suspend_reason & SUSPEND_REASON_BENCHMARKS) {
        cpu_benchmarks_poll();

        // besides waiting for applications to become suspended
        //
        if (active_tasks.is_task_executing()) {
            POLL_ACTION(active_tasks, active_tasks.poll);
        }
    }

    check_suspend_network(network_suspend_reason);
    suspend_reason |= network_suspend_reason;

    // if we've had a GUI RPC in last few minutes, relax the normal rules
    //
    if (gui_rpcs.got_recent_rpc(300)) {
        suspend_reason &= !SUSPEND_REASON_USER_ACTIVE;
        suspend_reason &= !SUSPEND_REASON_BATTERIES;
    }
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

    scope_messages.printf("CLIENT_STATE::poll_slow_events(): Begin poll:\n");
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
    POLL_ACTION(active_tasks           , active_tasks.poll      );
    POLL_ACTION(garbage_collect        , garbage_collect        );
    POLL_ACTION(update_results         , update_results         );
    POLL_ACTION(gui_http               , gui_http.poll          );
    POLL_ACTION(acct_mgr               , acct_mgr_info.poll          );
    if (!network_suspended) {
        net_stats.poll(*file_xfers, *net_xfers);
        POLL_ACTION(file_xfers             , file_xfers->poll       );
        POLL_ACTION(pers_file_xfers        , pers_file_xfers->poll  );
        POLL_ACTION(handle_pers_file_xfers , handle_pers_file_xfers );
    }
    POLL_ACTION(handle_finished_apps   , handle_finished_apps   );
    if (!tasks_suspended) {
        POLL_ACTION(schedule_cpus          , schedule_cpus          );
    }
    if (!network_suspended) {
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
    }
    retval = write_state_file_if_needed();
    if (retval) {
        msg_printf(NULL, MSG_ERROR,
            "Couldn't write state file: %s", boincerror(retval)
        );
        boinc_sleep(60.0);

        // if we can't write the state file twice in a row, something's hosed;
        // better to not keep trying
        //
        retval = write_state_file_if_needed();
        if (retval) {
            msg_printf(NULL, MSG_ERROR,
                "Couldn't write state file: %s; giving up", boincerror(retval)
            );
            exit(retval);
        }
    }
    --log_messages;
    scope_messages.printf(
        "CLIENT_STATE::do_something(): End poll: %d tasks active\n", actions
    );
    if (actions > 0) {
        return true;
    } else {
        time_stats.update(!tasks_suspended);
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
        msg_printf(p, MSG_ERROR,
            "State file error: bad application name %s",
            avp->app_name
        );
        return ERR_NOT_FOUND;
    }
    avp->app = app;

    if (lookup_app_version(app, avp->version_num)) return ERR_NOT_UNIQUE;

    for (i=0; i<avp->app_files.size(); i++) {
        FILE_REF& file_ref = avp->app_files[i];
        fip = lookup_file_info(p, file_ref.file_name);
        if (!fip) {
            msg_printf(p, MSG_ERROR,
                "State file error: missing application file %s",
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
        msg_printf(p, MSG_ERROR,
            "State file error: missing file %s",
            file_refp->file_name
        );
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
        msg_printf(p, MSG_ERROR,
            "State file error: missing application %s",
            wup->app_name
        );
        return ERR_NOT_FOUND;
    }
    avp = lookup_app_version(app, wup->version_num);
    if (!avp) {
        msg_printf(p, MSG_ERROR,
            "State file error: no application version %s %d\n",
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
                "State file error: missing input file %s\n",
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
        msg_printf(p, MSG_ERROR,
            "State file error: missing task %s\n", rp->wu_name
        );
        return ERR_NOT_FOUND;
    }
    rp->project = p;
    rp->wup = wup;
    rp->app = wup->app;
    for (i=0; i<rp->output_files.size(); i++) {
        retval = link_file_ref(p, &rp->output_files[i]);
        if (retval) return retval;
    }
    return 0;
}

// Print debugging information about how many projects/files/etc
// are currently in the client state record
//
void CLIENT_STATE::print_summary() {
    unsigned int i;
    double t;
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

bool CLIENT_STATE::garbage_collect() {
    static double last_time=0;
    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    return garbage_collect_always();
}

// delete unneeded records and files
//
bool CLIENT_STATE::garbage_collect_always() {
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
bool CLIENT_STATE::update_results() {
    RESULT* rp;
    vector<RESULT*>::iterator result_iter;
    bool action = false;
    static double last_time=0;

    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

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
                request_schedule_cpus("files downloaded");
                action = true;
            }
            break;
        case RESULT_FILES_UPLOADING:
            if (rp->is_upload_done()) {
                rp->ready_to_report = true;
                rp->completed_time = gstate.now;
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
        && ((now - app_started) >= exit_after_app_start_secs)
    ) {
        msg_printf(NULL, MSG_INFO,
            "Exiting because time is up: %d", exit_after_app_start_secs
        );
        return true;
    }
    if (exit_when_idle && (results.size() == 0) && contacted_sched_server) {
        msg_printf(NULL, MSG_INFO, "exiting because no more results");
        return true;
    }
    return false;
}

// Call this when a result has a nonrecoverable error.
// - back off on contacting the project's scheduler
//   (so don't crash over and over)
// - Append a description of the error to result.stderr_out
//
int CLIENT_STATE::report_result_error(RESULT& res, const char* format, ...) {
    char buf[4096],  err_msg[4096];
        // The above store 1-line messages and short XML snippets.
        // Shouldn't exceed a few hundred bytes.
    unsigned int i;
    int failnum;

    // only do this once per result
    //
    if (res.ready_to_report) {
        return 0;
    }

    res.ready_to_report = true;
    res.completed_time = now;

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
        msg_printf(res.project, MSG_ERROR,
            "Error reported for completed task %s", res.name
        );
        break;
    }

    res.stderr_out = res.stderr_out.substr(0, MAX_STDERR_LEN);
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
    scheduler_op->abort(project);

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

    garbage_collect_always();

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
        garbage_collect_always();
    }

    project->duration_correction_factor = 1;
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

    // delete statistics file
    //
    get_statistics_filename(project->master_url, path);
    retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_ERROR,
            "Can't delete statistics file: %s", boincerror(retval)
        );
    }

    // delete account file
    //
    get_account_filename(project->master_url, path);
    retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_ERROR,
            "Can't delete account file: %s", boincerror(retval)
        );
    }

    // remove project directory and its contents
    //
    retval = remove_project_dir(*project);
    if (retval) {
        msg_printf(project, MSG_ERROR,
            "Can't delete project directory: %s", boincerror(retval)
        );
    }

    delete project;
    write_state_file();

    return 0;
}

// Return true if the core client wants a network connection.
// Don't return false if we've actually been using the network
// in the last 10 seconds (so that polling mechanisms
// have a change to trigger)
//
bool CLIENT_STATE::want_network() {
    static double last_true_return=0;

    if (http_ops->nops()) goto return_true;
    if (network_suspended) goto return_false;
    if (want_network_flag) goto return_true;
    if (active_tasks.want_network()) goto return_true;
return_false:
    if ((now - last_true_return) > 10) {
        have_sporadic_connection = false;
        return false;
    }
    return true;
return_true:
    last_true_return = now;
    return true;
}

// There's now a network connection, after some period of disconnection.
// Do all communication that we can.
//
void CLIENT_STATE::network_available() {
    unsigned int i;

    have_sporadic_connection = true;
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        PERS_FILE_XFER* pfx = pers_file_xfers->pers_file_xfers[i];
        pfx->next_request_time = 0;
    }
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->min_rpc_time = 0;
    }

    // tell active tasks that network is available (for Folding@home)
    //
    active_tasks.network_available();
}

// return a random double in the range [rmin,rmax)
static inline double rand_range(double rmin, double rmax) {
    if (rmin < rmax) {
        return drand() * (rmax-rmin) + rmin;
    } else {
        return rmin;
    }
}

// return a random double in the range [MIN,min(e^n,MAX))
//
double calculate_exponential_backoff( int n, double MIN, double MAX) {
    double rmax = std::min(MAX, exp((double)n));
    return rand_range(MIN, rmax);
}


const char *BOINC_RCSID_e836980ee1 = "$Id$";
