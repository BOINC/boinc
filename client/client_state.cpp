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
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <cstring>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif

#ifdef __EMX__
#define INCL_DOS
#include <os2.h>
#endif

#include "cpp.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "filesys.h"
#ifdef _WIN32
#include "proc_control.h"
#endif

#include "file_names.h"
#include "hostinfo.h"
#include "hostinfo_network.h"
#include "network.h"
#include "http_curl.h"
#include "client_msgs.h"
#include "shmem.h"
#include "sandbox.h"
#include "cs_notice.h"

#include "client_state.h"

using std::max;

CLIENT_STATE gstate;
COPROC_CUDA* coproc_cuda;
COPROC_ATI* coproc_ati;

CLIENT_STATE::CLIENT_STATE():
    lookup_website_op(&gui_http),
    get_current_version_op(&gui_http),
    get_project_list_op(&gui_http)
{
    http_ops = new HTTP_OP_SET();
    file_xfers = new FILE_XFER_SET(http_ops);
    pers_file_xfers = new PERS_FILE_XFER_SET(file_xfers);
    scheduler_op = new SCHEDULER_OP(http_ops);
    client_state_dirty = false;
    exit_when_idle = false;
    exit_before_start = false;
    exit_after_finish = false;
    check_all_logins = false;
    cmdline_gui_rpc_port = 0;
    run_cpu_benchmarks = false;
    skip_cpu_benchmarks = false;
    file_xfer_giveup_period = PERS_GIVEUP;
    contacted_sched_server = false;
    tasks_suspended = false;
    network_suspended = false;
    suspend_reason = 0;
    network_suspend_reason = 0;
    core_client_version.major = BOINC_MAJOR_VERSION;
    core_client_version.minor = BOINC_MINOR_VERSION;
    core_client_version.release = BOINC_RELEASE;
#ifdef BOINC_PRERELEASE
    core_client_version.prerelease = true;
#else
    core_client_version.prerelease = false;
#endif
    exit_after_app_start_secs = 0;
    app_started = 0;
    exit_before_upload = false;
    show_projects = false;
    strcpy(detach_project_url, "");
    strcpy(main_host_venue, "");
    strcpy(attach_project_url, "");
    strcpy(attach_project_auth, "");
    run_mode.set(RUN_MODE_AUTO, 0);
    gpu_mode.set(RUN_MODE_AUTO, 0);
    network_mode.set(RUN_MODE_AUTO, 0);
    started_by_screensaver = false;
    requested_exit = false;
    requested_suspend = false;
    requested_resume = false;
    cleanup_completed = false;
    in_abort_sequence = false;
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
    disable_graphics = false;
    work_fetch_no_new_work = false;
    cant_write_state_file = false;
    unsigned_apps_ok = false;

    debt_interval_start = 0;
    retry_shmem_time = 0;
    must_schedule_cpus = true;
    must_enforce_cpu_schedule = true;
    no_gui_rpc = false;
    new_version_check_time = 0;
    all_projects_list_check_time = 0;
    detach_console = false;
#ifdef SANDBOX
    g_use_sandbox = true; // User can override with -insecure command-line arg
#endif
    launched_by_manager = false;
    initialized = false;
    last_wakeup_time = dtime();
}

void CLIENT_STATE::show_host_info() {
    char buf[256], buf2[256];

    nbytes_to_string(host_info.m_cache, 0, buf, sizeof(buf));
    msg_printf(NULL, MSG_INFO,
        "Processor: %d %s %s",
        host_info.p_ncpus, host_info.p_vendor, host_info.p_model
    );
    if (ncpus != host_info.p_ncpus) {
        msg_printf(NULL, MSG_INFO, "Using %d CPUs", ncpus);
    }
    if (host_info.m_cache > 0) {
        msg_printf(NULL, MSG_INFO,
            "Processor: %s cache",
            buf
        );
    }
    msg_printf(NULL, MSG_INFO,
        "Processor features: %s", host_info.p_features
    );
    msg_printf(NULL, MSG_INFO,
        "OS: %s: %s", host_info.os_name, host_info.os_version
    );

    nbytes_to_string(host_info.m_nbytes, 0, buf, sizeof(buf));
    nbytes_to_string(host_info.m_swap, 0, buf2, sizeof(buf2));
    msg_printf(NULL, MSG_INFO,
        "Memory: %s physical, %s virtual",
        buf, buf2
    );

    nbytes_to_string(host_info.d_total, 0, buf, sizeof(buf));
    nbytes_to_string(host_info.d_free, 0, buf2, sizeof(buf2));
    msg_printf(NULL, MSG_INFO, "Disk: %s total, %s free", buf, buf2);
    int tz = host_info.timezone/3600;
    msg_printf(0, MSG_INFO, "Local time is UTC %s%d hours",
        tz<0?"":"+", tz
    );
}

int CLIENT_STATE::init() {
    int retval;
    unsigned int i;
    char buf[256];
    PROJECT* p;

    srand((unsigned int)time(0));
    now = dtime();
    client_start_time = now;
    scheduler_op->url_random = drand();

    notices.init();
    daily_xfer_history.init();
    
    detect_platforms();
    time_stats.start();

    msg_printf(
        NULL, MSG_INFO, "Starting BOINC client version %d.%d.%d for %s%s",
        core_client_version.major,
        core_client_version.minor,
        core_client_version.release,
        get_primary_platform(),
#ifdef _DEBUG
        " (DEBUG)"
#else
        ""
#endif
    );

    if (core_client_version.prerelease) {
        msg_printf(NULL, MSG_INFO,
            "This a development version of BOINC and may not function properly"
        );
    }

    config.show();
    log_flags.show();

    msg_printf(NULL, MSG_INFO, "Libraries: %s", curl_version());

    if (executing_as_daemon) {
        msg_printf(NULL, MSG_INFO, "Running as a daemon");
    }

    relative_to_absolute("", buf);
    msg_printf(NULL, MSG_INFO, "Data directory: %s", buf);

#ifdef _WIN32
    DWORD  buf_size = sizeof(buf);
    LPTSTR pbuf = buf;

    GetUserName(pbuf, &buf_size);
    msg_printf(NULL, MSG_INFO, "Running under account %s", pbuf);
#endif

    parse_account_files();
    parse_statistics_files();

    host_info.clear_host_info();
    host_info.get_host_info();
    set_ncpus();
    show_host_info();

    // check for GPUs.
    //
    if (!config.no_gpus) {
        vector<string> descs;
        vector<string> warnings;
        host_info.coprocs.get(
            config.use_all_gpus, descs, warnings,
            config.ignore_cuda_dev, config.ignore_ati_dev
        );
        for (i=0; i<descs.size(); i++) {
            msg_printf(NULL, MSG_INFO, descs[i].c_str());
        }
        if (log_flags.coproc_debug) {
            for (i=0; i<warnings.size(); i++) {
                msg_printf(NULL, MSG_INFO, warnings[i].c_str());
            }
        }
        if (host_info.coprocs.coprocs.size() == 0) {
            msg_printf(NULL, MSG_INFO, "No usable GPUs found");
        }
#if 0
        msg_printf(NULL, MSG_INFO, "Faking an NVIDIA GPU");
        coproc_cuda = fake_cuda(host_info.coprocs, 256*MEGA, 2);
        coproc_cuda->available_ram_fake[0] = 256*MEGA;
        coproc_cuda->available_ram_fake[1] = 192*MEGA;
#endif
#if 0
        msg_printf(NULL, MSG_INFO, "Faking an ATI GPU");
        coproc_ati = fake_ati(host_info.coprocs, 512*MEGA, 2);
        coproc_ati->available_ram_fake[0] = 256*MEGA;
        coproc_ati->available_ram_fake[1] = 192*MEGA;
#endif
        coproc_cuda = (COPROC_CUDA*)host_info.coprocs.lookup("CUDA");
        coproc_ati = (COPROC_ATI*)host_info.coprocs.lookup("ATI");
    }

    // check for app_info.xml file in project dirs.
    // If find, read app info from there, set project.anonymous_platform
    // - this must follow coproc.get() (need to know if GPUs are present)
    // - this is being done before CPU speed has been read,
    // so we'll need to patch up avp->flops later;
    //
    check_anonymous();

    // first time, set p_fpops nonzero to avoid div by zero
    //
    cpu_benchmarks_set_defaults();

    // Parse the client state file,
    // ignoring any <project> tags (and associated stuff)
    // for projects with no account file
    //
    parse_state_file();

    // parse account files again,
    // now that we know the host's venue on each project
    //
    parse_account_files_venue();


    // fill in avp->flops for anonymous platform projects
    //
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION* avp = app_versions[i];
        if (!avp->flops) {
            if (!avp->avg_ncpus) {
                avp->avg_ncpus = 1;
            }
            avp->flops = avp->avg_ncpus * host_info.p_fpops;

            // for GPU apps, use conservative estimate:
            // assume app will run at peak CPU speed, not peak GPU
            //
            if (avp->ncudas) {
                avp->flops += avp->ncudas * host_info.p_fpops;
            }
            if (avp->natis) {
                avp->flops += avp->natis * host_info.p_fpops;
            }
        }
    }

    check_clock_reset();

    // Check to see if we can write the state file.
    //
    retval = write_state_file();
    if (retval) {
        msg_printf(NULL, MSG_USER_ALERT,
            "Couldn't write state file; make sure directory permissions are set correctly"
        );
        cant_write_state_file = true;
    }

    // scan user prefs; create file records
    //
    parse_preferences_for_user_files();

    if (log_flags.state_debug) {
        print_summary();
    }
    do_cmdline_actions();

    // check if version or platform has changed.
    // Either of these is evidence that we're running a different
    // client than previously.
    //
    bool new_client = false;
    if ((core_client_version.major != old_major_version)
        || (core_client_version.minor != old_minor_version)
        || (core_client_version.release != old_release)
    ) {
        msg_printf(NULL, MSG_INFO,
            "Version change (%d.%d.%d -> %d.%d.%d)",
            old_major_version, old_minor_version, old_release,
            core_client_version.major,
            core_client_version.minor,
            core_client_version.release
        );
        new_client = true;
    }
    if (statefile_platform_name.size() && strcmp(get_primary_platform(), statefile_platform_name.c_str())) {
        msg_printf(NULL, MSG_INFO,
            "Platform changed from %s to %s",
            statefile_platform_name.c_str(), get_primary_platform()
        );
        new_client = true;
    }
    // if new version of client,
    // - run CPU benchmarks
    // - contact reference site (or some project) to trigger firewall alert
    //
    if (new_client) {
        run_cpu_benchmarks = true;
        if (config.dont_contact_ref_site) {
            if (projects.size() > 0) {
                projects[0]->master_url_fetch_pending = true;
            }
        } else {
            net_status.need_to_contact_reference_site = true;
        }
    }


    // show projects
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->hostid) {
            sprintf(buf, "%d", p->hostid);
        } else {
            strcpy(buf, "not assigned yet");
        }
        msg_printf(p, MSG_INFO,
            "URL %s; Computer ID %s; resource share %.0f",
            p->master_url, buf, p->resource_share
        );
        if (p->ended) {
            msg_printf(p, MSG_INFO, "Project has ended - OK to detach");
        }
    }

    read_global_prefs();

    // do CPU scheduler and work fetch
    //
    request_schedule_cpus("Startup");
    request_work_fetch("Startup");
    work_fetch.init();
    debt_interval_start = now;

    // set up the project and slot directories
    //
    delete_old_slot_dirs();
    retval = make_project_dirs();
    if (retval) return retval;

    active_tasks.init();
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
        // When we're running at boot time,
        // it may be a few seconds before we can socket/bind/listen.
        // So retry a few times.
        //
        for (i=0; i<30; i++) {
            bool last_time = (i==29);
            retval = gui_rpcs.init(last_time);
            if (!retval) break;
            boinc_sleep(1.0);
        }
        if (retval) return retval;
    }

#ifdef SANDBOX
    get_project_gid();
#endif
#ifdef _WIN32
    get_sandbox_account_service_token();
    if (sandbox_account_service_token != NULL) g_use_sandbox = true;
#endif

    check_file_existence();
    if (!boinc_file_exists(ALL_PROJECTS_LIST_FILENAME)) {
        all_projects_list_check_time = 0;
    }
    all_projects_list_check();

    auto_update.init();

    http_ops->cleanup_temp_files();
    notices.init_rss();

    initialized = true;
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
    int loops = 0;

    while (1) {
        curl_fds.zero();
        gui_rpc_fds.zero();
		http_ops->get_fdset(curl_fds);
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

        http_ops->got_select(all_fds, x);
        gui_rpcs.got_select(all_fds);

        if (n==0) break;

        // Limit number of times thru this loop.
        // Can get stuck in while loop, if network isn't available,
        // DNS lookups tend to eat CPU cycles.
        //
        if (loops++ > 99) {
            boinc_sleep(.01);
#ifdef __EMX__
            DosSleep(0);
#endif
            break;
        }

        now = dtime();
        if (now > end_time) break;
        x = end_time - now;
    }
}

#define POLL_ACTION(name, func) \
    do { if (func()) { \
            ++actions; \
            if (log_flags.poll_debug) { \
                msg_printf(0, MSG_INFO, "[poll] CLIENT_STATE::poll_slow_events(): " #name "\n"); \
            } \
        } } while(0)

// Poll the client's finite-state machines
// possibly triggering state transitions.
// Returns true if something happened
// (in which case should call this again immediately)
//
bool CLIENT_STATE::poll_slow_events() {
    int actions = 0, retval;
    static int last_suspend_reason=0;
    static bool tasks_restarted = false;
    static bool first=true;
    double old_now = now;
#ifdef __APPLE__
    double idletime;
#endif

    now = dtime();

    if (cant_write_state_file) {
        return false;
    }

    if (now - old_now > POLL_INTERVAL*10) {
        if (log_flags.network_status_debug) {
            msg_printf(0, MSG_INFO,
                "[network_status] woke up after %f seconds",
                now - old_now
            );
        }
        last_wakeup_time = now;
    }

	if (should_run_cpu_benchmarks() && !are_cpu_benchmarks_running()) {
        run_cpu_benchmarks = false;
        start_cpu_benchmarks();
    }

    bool old_user_active = user_active;
    user_active = !host_info.users_idle(
        check_all_logins, global_prefs.idle_time_to_run
#ifdef __APPLE__
         , &idletime
#endif
    );

    if (user_active != old_user_active) {
        request_schedule_cpus("Idle state change");
    }

#if 0
    // NVIDIA provides an interface for finding if a GPU is
    // running a graphics app.  ATI doesn't as far as I know
    //
    if (coproc_cuda && user_active && !global_prefs.run_gpu_if_user_active) {
        if (coproc_cuda->check_running_graphics_app()) {
            request_schedule_cpus("GPU state change");
        }
    }
#endif

#ifdef __APPLE__
    // Mac screensaver launches client if not already running.
    // OS X quits screensaver when energy saver puts display to sleep,
    // but we want to keep crunching.
    // Also, user can start Mac screensaver by putting cursor in "hot corner"
    // so idletime may be very small initially.
    // If screensaver started client, this code tells client
    // to exit when user becomes active, accounting for all these factors.
    //
    if (started_by_screensaver && (idletime < 30) && (getppid() == 1)) {
        // pid is 1 if parent has exited
        requested_exit = true;
    }

    // Exit if we were launched by Manager and it crashed.
    //
    if (launched_by_manager && (getppid() == 1)) {
        gstate.requested_exit = true;
    }
#endif

    suspend_reason = check_suspend_processing();

    // suspend or resume activities (but only if already did startup)
    //
    if (tasks_restarted) {
        if (suspend_reason) {
            if (!tasks_suspended) {
                suspend_tasks(suspend_reason);
            }
            last_suspend_reason = suspend_reason;
        } else {
            if (tasks_suspended) {
                resume_tasks(last_suspend_reason);
            }
        }
    } else if (first) {
        // if suspended, show message the first time around
        //
        first = false;
        if (suspend_reason) {
            print_suspend_tasks_message(suspend_reason);
        }
    }
    tasks_suspended = (suspend_reason != 0);

    if (suspend_reason & SUSPEND_REASON_BENCHMARKS) {
        cpu_benchmarks_poll();
    }

    int old_network_suspend_reason = network_suspend_reason;
    bool old_network_suspended = network_suspended;
    check_suspend_network();
    if (network_suspend_reason) {
        if (!old_network_suspend_reason) {
            char buf[256];
            if (network_suspended) {
                sprintf(buf,
                    "Suspending network activity - %s",
                    suspend_reason_string(network_suspend_reason)
                );
            } else {
                sprintf(buf,
                    "Suspending file transfers - %s",
                    suspend_reason_string(network_suspend_reason)
                );
            }
            msg_printf(NULL, MSG_INFO, buf);
            pers_file_xfers->suspend();
        }
    } else {
        if (old_network_suspend_reason) {
            if (old_network_suspended) {
                msg_printf(NULL, MSG_INFO, "Resuming network activity");
            } else {
                msg_printf(NULL, MSG_INFO, "Resuming file transfers");
            }
        }

        // if we're emerging from a bandwidth quota suspension,
        // add a random delay to avoid DDOS effect
        //
        if (
            old_network_suspend_reason == SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED
            && network_mode.get_current() == RUN_MODE_AUTO
        ) {
            pers_file_xfers->add_random_delay(3600);
        }
    }

    // NOTE:
    // The order of calls in the following lists generally doesn't matter,
    // except for the following:
    // must have:
    //  active_tasks_poll
    //  handle_finished_apps
    //  possibly_schedule_cpus
    //  enforce_schedule
    // in that order (active_tasks_poll() sets must_schedule_cpus,
    // and handle_finished_apps() must be done before possibly_schedule_cpus()

	check_project_timeout();
    //auto_update.poll();
    POLL_ACTION(active_tasks           , active_tasks.poll      );
    POLL_ACTION(garbage_collect        , garbage_collect        );
    POLL_ACTION(gui_http               , gui_http.poll          );
    POLL_ACTION(gui_rpc_http           , gui_rpcs.poll          );
    if (!network_suspended && suspend_reason != SUSPEND_REASON_BENCHMARKS) {
        // don't initiate network activity if we're doing CPU benchmarks
        net_status.poll();
        daily_xfer_history.poll();
        POLL_ACTION(acct_mgr               , acct_mgr_info.poll     );
        POLL_ACTION(file_xfers             , file_xfers->poll       );
        POLL_ACTION(pers_file_xfers        , pers_file_xfers->poll  );
        POLL_ACTION(handle_pers_file_xfers , handle_pers_file_xfers );
        POLL_ACTION(rss_feed_op            , rss_feed_op.poll );
    }
    POLL_ACTION(handle_finished_apps   , handle_finished_apps   );
    POLL_ACTION(update_results         , update_results         );
    if (!tasks_suspended) {
        POLL_ACTION(possibly_schedule_cpus, possibly_schedule_cpus          );
        POLL_ACTION(enforce_schedule    , enforce_schedule  );
        tasks_restarted = true;
    }
    if (!network_suspended) {
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
    }
    retval = write_state_file_if_needed();
    if (retval) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Couldn't write state file: %s; giving up", boincerror(retval)
        );
        exit(EXIT_STATEFILE_WRITE);
    }
    if (log_flags.poll_debug) {
        msg_printf(0, MSG_INFO,
            "[poll] CLIENT_STATE::do_something(): End poll: %d tasks active\n", actions
        );
    }
    if (actions > 0) {
        return true;
    } else {
        time_stats.update(suspend_reason);

        // on some systems, gethostbyname() only starts working
        // a few minutes after system boot.
        // If it didn't work before, try it again.
        //
        if (!strlen(host_info.domain_name)) {
            host_info.get_local_network_info();
        }
        return false;
    }
}

// See if the project specified by master_url already exists
// in the client state record.  Ignore any trailing "/" characters
//
PROJECT* CLIENT_STATE::lookup_project(const char* master_url) {
    int len1, len2;
    char *mu;

    len1 = (int)strlen(master_url);
    if (master_url[strlen(master_url)-1] == '/') len1--;

    for (unsigned int i=0; i<projects.size(); i++) {
        mu = projects[i]->master_url;
        len2 = (int)strlen(mu);
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

APP_VERSION* CLIENT_STATE::lookup_app_version(
    APP* app, char* platform, int version_num, char* plan_class
) {
    for (unsigned int i=0; i<app_versions.size(); i++) {
        APP_VERSION* avp = app_versions[i];
        if (avp->app != app) continue;
        if (version_num != avp->version_num) continue;
        if (strcmp(avp->platform, platform)) continue;
        if (strcmp(avp->plan_class, plan_class)) continue;
        return avp;
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
    return 0;
}

int CLIENT_STATE::link_app_version(PROJECT* p, APP_VERSION* avp) {
    APP* app;
    FILE_INFO* fip;
    unsigned int i;

    avp->project = p;
    app = lookup_app(p, avp->app_name);
    if (!app) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: bad application name %s",
            avp->app_name
        );
        return ERR_NOT_FOUND;
    }
    avp->app = app;

    if (lookup_app_version(app, avp->platform, avp->version_num, avp->plan_class)) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: duplicate app version: %s %s %d %s",
            avp->app_name, avp->platform, avp->version_num, avp->plan_class
        );
        return ERR_NOT_UNIQUE;
    }
    
    strcpy(avp->graphics_exec_path, "");
    strcpy(avp->graphics_exec_file, "");

    for (i=0; i<avp->app_files.size(); i++) {
        FILE_REF& file_ref = avp->app_files[i];
        fip = lookup_file_info(p, file_ref.file_name);
        if (!fip) {
            msg_printf(p, MSG_INTERNAL_ERROR,
                "State file error: missing application file %s",
                file_ref.file_name
            );
            return ERR_NOT_FOUND;
        }

        if (!strcmp(file_ref.open_name, GRAPHICS_APP_FILENAME)) {
            char relpath[512], path[512];
            get_pathname(fip, relpath, sizeof(relpath));
            relative_to_absolute(relpath, path);
            strlcpy(avp->graphics_exec_path, path, sizeof(avp->graphics_exec_path));
            strcpy(avp->graphics_exec_file, fip->name);
        }

        // any file associated with an app version must be signed
        //
        if (!unsigned_apps_ok) {
            fip->signature_required = true;
        }

        file_ref.file_info = fip;
    }
    return 0;
}

int CLIENT_STATE::link_file_ref(PROJECT* p, FILE_REF* file_refp) {
    FILE_INFO* fip;

    fip = lookup_file_info(p, file_refp->file_name);
    if (!fip) {
        msg_printf(p, MSG_INTERNAL_ERROR,
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
    unsigned int i;
    int retval;

    app = lookup_app(p, wup->app_name);
    if (!app) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: missing application %s",
            wup->app_name
        );
        return ERR_NOT_FOUND;
    }
    wup->project = p;
    wup->app = app;
    for (i=0; i<wup->input_files.size(); i++) {
        retval = link_file_ref(p, &wup->input_files[i]);
        if (retval) {
            msg_printf(p, MSG_INTERNAL_ERROR,
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
        msg_printf(p, MSG_INTERNAL_ERROR,
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

    msg_printf(0, MSG_INFO, "[state] Client state summary:");
    msg_printf(0, MSG_INFO, "%d projects:", (int)projects.size());
    for (i=0; i<projects.size(); i++) {
        t = projects[i]->min_rpc_time;
        if (t) {
            msg_printf(0, MSG_INFO, "    %s min RPC %f.0 seconds from now", projects[i]->master_url, t-now);
        } else {
            msg_printf(0, MSG_INFO, "    %s", projects[i]->master_url);
        }
    }
    msg_printf(0, MSG_INFO, "%d file_infos:", (int)file_infos.size());
    for (i=0; i<file_infos.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s status:%d %s", file_infos[i]->name, file_infos[i]->status, file_infos[i]->pers_file_xfer?"active":"inactive");
    }
    msg_printf(0, MSG_INFO, "%d app_versions", (int)app_versions.size());
    for (i=0; i<app_versions.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s %d", app_versions[i]->app_name, app_versions[i]->version_num);
    }
    msg_printf(0, MSG_INFO, "%d workunits", (int)workunits.size());
    for (i=0; i<workunits.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s", workunits[i]->name);
    }
    msg_printf(0, MSG_INFO, "%d results", (int)results.size());
    for (i=0; i<results.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s state:%d", results[i]->name, results[i]->state());
    }
    msg_printf(0, MSG_INFO, "%d persistent file xfers", (int)pers_file_xfers->pers_file_xfers.size());
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s http op state: %d", pers_file_xfers->pers_file_xfers[i]->fip->name, (pers_file_xfers->pers_file_xfers[i]->fxp?pers_file_xfers->pers_file_xfers[i]->fxp->http_op_state:-1));
    }
    msg_printf(0, MSG_INFO, "%d active tasks", (int)active_tasks.active_tasks.size());
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s", active_tasks.active_tasks[i]->result->name);
    }
}

int CLIENT_STATE::nresults_for_project(PROJECT* p) {
    int n=0;
    for (unsigned int i=0; i<results.size(); i++) {
        if (results[i]->project == p) n++;
    }
    return n;
}

bool CLIENT_STATE::abort_unstarted_late_jobs() {
    bool action = false;
    if (now < 1235668593) return false; // skip if user reset system clock
    for (unsigned int i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (!rp->not_started()) continue;
        if (rp->report_deadline > now) continue;
        msg_printf(rp->project, MSG_INFO,
            "Aborting task %s; not started and deadline has passed",
            rp->name
        );
        rp->abort_inactive(ERR_UNSTARTED_LATE);
        action = true;
    }
    return action;
}

bool CLIENT_STATE::garbage_collect() {
    bool action;
    static double last_time=0;
    if (gstate.now - last_time < GARBAGE_COLLECT_PERIOD) return false;
    last_time = gstate.now;

    action = abort_unstarted_late_jobs();
    if (action) return true;
    action = garbage_collect_always();
    if (action) return true;

    // Detach projects that are marked for detach when done
    // and are in fact done (have no results).
    // This is done here (not in garbage_collect_always())
    // because detach_project() calls garbage_collect_always(),
    // and we need to avoid infinite recursion
    //
    for (unsigned i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->detach_when_done && !nresults_for_project(p)) {
            detach_project(p);
            action = true;
        }
    }
    return action;
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

    // reference-count user and project files
    //
    for (i=0; i<projects.size(); i++) {
        project = projects[i];
        for (j=0; j<project->user_files.size(); j++) {
            project->user_files[j].file_info->ref_cnt++;
        }
        for (j=0; j<project->project_files.size(); j++) {
            project->project_files[j].file_info->ref_cnt++;
        }
    }

    // reference-count auto update files
    //
    if (auto_update.present) {
        for (i=0; i<auto_update.file_refs.size(); i++) {
            auto_update.file_refs[i].file_info->ref_cnt++;
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
            // see if - for some reason - there's an active task
            // for this result.  don't want to create dangling ptr.
            //
            ACTIVE_TASK* atp = active_tasks.lookup_result(rp);
            if (atp) {
                msg_printf(rp->project, MSG_INTERNAL_ERROR,
                    "garbage_collect(); still have active task for acked result %s; state %d",
                    rp->name, atp->task_state()
                );
                atp->abort_task(
                    EXIT_ABORTED_BY_CLIENT,
                    "Got ack for job that's till active"
                );
            } else {
                if (log_flags.state_debug) {
                    msg_printf(0, MSG_INFO,
                        "[state] garbage_collect: deleting result %s\n",
                        rp->name
                    );
                }
                delete rp;
                result_iter = results.erase(result_iter);
                action = true;
                continue;
            }
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
            } else if (rp->avp && rp->avp->had_download_failure(failnum)) {
                rp->avp->get_file_errors(error_msgs);
                report_result_error(
                    *rp, "app_version download error: %s", error_msgs.c_str()
                );
            }
        }
        bool found_error = false;
        string error_str;
        for (i=0; i<rp->output_files.size(); i++) {
            // If one of the output files had an upload failure,
            // mark the result as done and report the error.
            //
            if (!rp->ready_to_report) {
                fip = rp->output_files[i].file_info;
                if (fip->had_failure(failnum)) {
                    string msg;
                    fip->failure_message(msg);
                    found_error = true;
                    error_str += msg;
                }
            }
            rp->output_files[i].file_info->ref_cnt++;
        }
        if (found_error) {
            // check for process still running; this can happen
            // e.g. if an intermediate upload fails
            //
            ACTIVE_TASK* atp = active_tasks.lookup_result(rp);
            if (atp) {
                switch (atp->task_state()) {
                case PROCESS_EXECUTING:
                case PROCESS_SUSPENDED:
                    atp->abort_task(
                        EXIT_ABORTED_BY_CLIENT,
                        "Got ack for job that's till active"
                    );
                }
            }
            report_result_error(*rp, "%s", error_str.c_str());
        }
        rp->avp->ref_cnt++;
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
            if (log_flags.state_debug) {
                msg_printf(0, MSG_INFO,
                    "[state] CLIENT_STATE::garbage_collect(): deleting workunit %s\n",
                    wup->name
                );
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
                if (avp2->app==avp->app
                    && (!strcmp(avp2->plan_class, avp->plan_class))
                    && avp2->version_num>avp->version_num
                ) {
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

    // reference-count sticky files not marked for deletion
    //
    
    for (fi_iter = file_infos.begin(); fi_iter!=file_infos.end(); fi_iter++) {
        fip = *fi_iter;
        if (!fip->sticky) continue;
        if (fip->status < 0) continue;
        if (fip->marked_for_delete) continue;
        fip->ref_cnt++;
    }

    // remove PERS_FILE_XFERs (and associated FILE_XFERs and HTTP_OPs)
    // for unreferenced files
    //
    vector<PERS_FILE_XFER*>::iterator pfx_iter;
    pfx_iter = pers_file_xfers->pers_file_xfers.begin();
    while (pfx_iter != pers_file_xfers->pers_file_xfers.end()) {
        PERS_FILE_XFER* pfx = *pfx_iter;
        if (pfx->fip->ref_cnt == 0) {
            pfx->suspend();
            delete pfx;
            pfx_iter = pers_file_xfers->pers_file_xfers.erase(pfx_iter);
        } else {
            pfx_iter++;
        }
    }

    // delete FILE_INFOs (and corresponding files) that are not referenced
    //
    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fip = *fi_iter;
        if (fip->ref_cnt==0) {
            fip->delete_file();
            if (log_flags.state_debug) {
                msg_printf(0, MSG_INFO,
                    "[state] CLIENT_STATE::garbage_collect(): deleting file %s\n",
                    fip->name
                );
            }
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

// For results that are waiting for file transfer,
// check if the transfer is done,
// and if so switch to new state and take other actions.
// Also set some fields for newly-aborted results.
//
bool CLIENT_STATE::update_results() {
    RESULT* rp;
    vector<RESULT*>::iterator result_iter;
    bool action = false;
    static double last_time=0;
    int retval;

    if (gstate.now - last_time < UPDATE_RESULTS_PERIOD) return false;
    last_time = gstate.now;

    result_iter = results.begin();
    while (result_iter != results.end()) {
        rp = *result_iter;

        switch (rp->state()) {
        case RESULT_NEW:
            rp->set_state(RESULT_FILES_DOWNLOADING, "CS::update_results");
            action = true;
            break;
        case RESULT_FILES_DOWNLOADING:
            retval = input_files_available(rp, false);
            if (!retval) {
                rp->set_state(RESULT_FILES_DOWNLOADED, "CS::update_results");
                if (rp->avp->app_files.size()==0) {
                    // if this is a file-transfer app, start the upload phase
                    //
                    rp->set_state(RESULT_FILES_UPLOADING, "CS::update_results");
                    rp->clear_uploaded_flags();
                } else {
                    // else try to start the computation
                    //
                    request_schedule_cpus("files downloaded");
                }
                action = true;
            }
            break;
        case RESULT_FILES_UPLOADING:
            if (rp->is_upload_done()) {
                rp->ready_to_report = true;
                rp->completed_time = gstate.now;
                rp->set_state(RESULT_FILES_UPLOADED, "CS::update_results");
                action = true;
            }
            break;
        case RESULT_FILES_UPLOADED:
            break;
        case RESULT_ABORTED:
            if (!rp->ready_to_report) {
                rp->ready_to_report = true;
                rp->completed_time = now;
                action = true;
            }
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
    if (cant_write_state_file) {
        static bool first = true;
        double t = now - last_wakeup_time;
        if (first && t > 50) {
            first = false;
            msg_printf(NULL, MSG_INFO,
                "Can't write state file, exiting in 10 seconds"
            );
        }
        if (t > 60) {
            msg_printf(NULL, MSG_INFO,
                "Can't write state file, exiting now"
            );
            return true;
        }
    }
    return false;
}

// Call this when a result has a nonrecoverable error.
// - back off on contacting the project's scheduler
//   (so don't crash over and over)
// - Append a description of the error to result.stderr_out
// - If result state is FILES_DOWNLOADED, change it to COMPUTE_ERROR
//   so that we don't try to run it again.
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

    sprintf(buf, "Unrecoverable error for task %s (%s)", res.name, err_msg);
    scheduler_op->backoff(res.project, buf);

    sprintf( buf, "<message>\n%s\n</message>\n", err_msg);
    res.stderr_out.append(buf);

    switch(res.state()) {
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
        res.set_state(RESULT_COMPUTE_ERROR, "CS::report_result_error");
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
    case RESULT_FILES_UPLOADED:
        msg_printf(res.project, MSG_INTERNAL_ERROR,
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
// - clear debts and backoffs
//
// Note: does NOT delete persistent files or user-supplied files;
// does not delete project dir
//
int CLIENT_STATE::reset_project(PROJECT* project, bool detaching) {
    unsigned int i;
    APP_VERSION* avp;
    APP* app;
    vector<APP*>::iterator app_iter;
    vector<APP_VERSION*>::iterator avp_iter;
    RESULT* rp;
    PERS_FILE_XFER* pxp;

    msg_printf(project, MSG_INFO, "Resetting project");
    active_tasks.abort_project(project);

    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        pxp = pers_file_xfers->pers_file_xfers[i];
        if (pxp->fip->project == project) {
            if (pxp->fxp) {
                file_xfers->remove(pxp->fxp);
                delete pxp->fxp;
            }
            pers_file_xfers->remove(pxp);
            delete pxp;
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

    project->user_files.clear();
    project->project_files.clear();

    garbage_collect_always();

    // "ordered_scheduled_results" may contain pointers (now dangling)
    // to tasks of this project.
    //
    ordered_scheduled_results.clear();

    // remove apps and app_versions (but not if anonymous platform)
    //
    if (!project->anonymous_platform || detaching) {
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

        char buf[1024];
        get_project_dir(project, buf, sizeof(buf));
        client_clean_out_dir(buf, "reset or detach project");
    }

    project->duration_correction_factor = 1;
    project->ams_resource_share = -1;
    project->min_rpc_time = 0;
    project->pwf.reset(project);
    project->cpu_pwf.reset();
    project->cuda_pwf.reset();
    project->ati_pwf.reset();
    write_state_file();
    return 0;
}

// "Detach" a project:
// - Reset (see above)
// - delete all file infos
// - delete account file
// - delete project directory
//
int CLIENT_STATE::detach_project(PROJECT* project) {
    vector<PROJECT*>::iterator project_iter;
    vector<FILE_INFO*>::iterator fi_iter;
    FILE_INFO* fip;
    PROJECT* p;
    char path[256];
    int retval;

    reset_project(project, true);

    msg_printf(project, MSG_INFO, "Detaching from project");

    // delete all FILE_INFOs associated with this project
    //
    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fip = *fi_iter;
        if (fip->project == project) {
            fi_iter = file_infos.erase(fi_iter);
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
            project_iter = projects.erase(project_iter);
            break;
        }
    }

    // delete statistics file
    //
    get_statistics_filename(project->master_url, path);
    retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "Can't delete statistics file: %s", boincerror(retval)
        );
    }

    // delete account file
    //
    get_account_filename(project->master_url, path);
    retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "Can't delete account file: %s", boincerror(retval)
        );
    }

    get_sched_request_filename(*project, path, sizeof(path));
    retval = boinc_delete_file(path);

    get_sched_reply_filename(*project, path, sizeof(path));
    retval = boinc_delete_file(path);

    get_master_filename(*project, path, sizeof(path));
    retval = boinc_delete_file(path);

    // remove project directory and its contents
    //
    retval = remove_project_dir(*project);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "Can't delete project directory: %s", boincerror(retval)
        );
    }

    delete project;
    write_state_file();

    adjust_debts();
    request_schedule_cpus("Detach");
    request_work_fetch("Detach");
    return 0;
}

// Quit running applications, quit benchmarks,
// write the client_state.xml file
// (in principle we could also terminate net_xfers here,
// e.g. flush buffers, but why bother)
//
int CLIENT_STATE::quit_activities() {
    // calculate long-term debts (for state file)
    //
    adjust_debts();

    int retval = active_tasks.exit_tasks();
    if (retval) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Couldn't exit tasks: %s", boincerror(retval)
        );
    }
    write_state_file();
    gui_rpcs.close();
    abort_cpu_benchmarks();
    time_stats.quit();
    return 0;
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

// See if a timestamp in the client state file
// is later than the current time.
// If so, the user must have decremented the system clock.
// Clear all timeout variables.
//
void CLIENT_STATE::check_clock_reset() {
    now = time(0);
    if (!time_stats.last_update) return;
    if (time_stats.last_update <= now) return;

    msg_printf(NULL, MSG_INFO,
        "System clock was turned backwards; clearing timeouts"
    );
    new_version_check_time = now;
    all_projects_list_check_time = now;

    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->min_rpc_time = 0;
        if (p->next_rpc_time) {
            p->next_rpc_time = now;
        }
        p->download_backoff.next_xfer_time = 0;
        p->upload_backoff.next_xfer_time = 0;
    }
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        PERS_FILE_XFER* pfx = pers_file_xfers->pers_file_xfers[i];
        pfx->next_request_time = 0;
    }

    // RESULT: could change report_deadline, but not clear how
}

// the following is done on client exit if the
// "abort_jobs_on_exit" flag is present.
// Abort jobs, and arrange to tell projects about it.
//
void CLIENT_STATE::start_abort_sequence() {
    unsigned int i;

    in_abort_sequence = true;

    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        rp->project->sched_rpc_pending = RPC_REASON_USER_REQ;
        if (rp->computing_done()) continue;
        ACTIVE_TASK* atp = lookup_active_task_by_result(rp);
        if (atp) {
            atp->abort_task(ERR_ABORTED_ON_EXIT, "aborting on client exit");
        } else {
            rp->abort_inactive(ERR_ABORTED_ON_EXIT);
        }
    }
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->min_rpc_time = 0;
    }
}

// The second part of the above; check if RPCs are done
//
bool CLIENT_STATE::abort_sequence_done() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->sched_rpc_pending == RPC_REASON_USER_REQ) return false;
    }
    return true;
}

