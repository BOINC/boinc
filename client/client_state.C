// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cassert>

#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "filesys.h"

#include "account.h"
#include "file_names.h"
#include "hostinfo.h"
#include "http.h"
#include "speed_stats.h"
#include "client_state.h"
#include "log_flags.h"
#include "maybe_gui.h"

#define BENCHMARK_PERIOD        (SECONDS_PER_DAY*30)
    // rerun CPU benchmarks this often (hardware may have been upgraded)

CLIENT_STATE gstate;

CLIENT_STATE::CLIENT_STATE() {
    net_xfers = new NET_XFER_SET;
    http_ops = new HTTP_OP_SET(net_xfers);
    file_xfers = new FILE_XFER_SET(http_ops);
    pers_file_xfers = new PERS_FILE_XFER_SET(file_xfers);
    scheduler_op = new SCHEDULER_OP(http_ops);
    client_state_dirty = false;
    exit_when_idle = false;
    run_cpu_benchmarks = false;
    skip_cpu_benchmarks = false;
    file_xfer_giveup_period = PERS_GIVEUP;
    contacted_sched_server = false;
    activities_suspended = false;
    previous_activities_suspended = false;
    core_client_major_version = MAJOR_VERSION;
    core_client_minor_version = MINOR_VERSION;
    platform_name = HOSTTYPE;
    exit_after_app_start_secs = 0;
    app_started = 0;
    exit_before_upload = false;
    user_idle = true;
    use_http_proxy = false;
    use_socks_proxy = false;
    show_projects = false;
    strcpy(detach_project_url, "");
    strcpy(proxy_server_name, "");
    proxy_server_port = 80;
    strcpy(socks_user_name, "");
    strcpy(socks_user_passwd, "");
    strcpy(host_venue, "");
    user_run_request = USER_RUN_REQUEST_AUTO;
    start_saver = false;
    requested_exit = false;
#ifdef _WIN32
    cpu_benchmarks_handle = NULL;
#endif
    cpu_benchmarks_id = 0;
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
}

#if 0
// Deallocate memory to prevent unneeded reporting of memory leaks
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

    language.read_language_file(LANGUAGE_FILE_NAME);

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

    print_summary();

    if (show_projects) {
        printf("projects:\n");
        for (i=0; i<projects.size(); i++) {
            printf("URL: %s name: %s\n",
                projects[i]->master_url, projects[i]->project_name
            );
        }
        exit(0);
    }

    if (strlen(detach_project_url)) {
        PROJECT* project = lookup_project(detach_project_url);
        if (project) {
            detach_project(project);
        } else {
            printf("project %s not found\n", detach_project_url);
        }
        exit(0);
    }

    if (strlen(reset_project_url)) {
        PROJECT* project = lookup_project(reset_project_url);
        if (project) {
            reset_project(project);
        } else {
            printf("project %s not found\n", reset_project_url);
        }
        exit(0);
    }

    if (strlen(update_prefs_url)) {
        PROJECT* project = lookup_project(update_prefs_url);
        if (project) {
            project->sched_rpc_pending = true;
        } else {
            printf("project %s not found\n", update_prefs_url);
        }
    }

    msg_printf(NULL, MSG_INFO, "Starting BOINC client version %d.%02d",
        core_client_major_version, core_client_minor_version);

    if (core_client_major_version != old_major_version) {
        msg_printf(NULL, MSG_INFO,
            "State file has different major version (%d.%02d); resetting projects\n",
            old_major_version, old_minor_version
        );
        for (i=0; i<projects.size(); i++) {
            reset_project(projects[i]);
        }
    }

    // Read the global preferences file, if it exists.
    // Do this after reading the state file so we know our venue
    //
    retval = global_prefs.parse_file(host_venue);
    if (retval) {
        msg_printf(NULL, MSG_INFO, "Using default preferences");
    }
    install_global_prefs();

    // Getting host info is very fast, so we can do it anytime
    //
    get_host_info(host_info);

    // running CPU benchmarks is slow, so do it infrequently
    //
    if (should_run_cpu_benchmarks()) {
		fork_run_cpu_benchmarks();
    }

    set_nslots();

    // set up the project and slot directories
    //
    retval = make_project_dirs();
    if (retval) return retval;

    // Restart any tasks that were running when we last quit the client
    //
    restart_tasks();

    return 0;
}

void CLIENT_STATE::fork_run_cpu_benchmarks() {
	cpu_benchmarks_start = time(0);
	msg_printf(NULL, MSG_INFO, "Running CPU benchmarks");
#ifdef _WIN32
    cpu_benchmarks_handle = CreateThread(
        NULL, 0, win_cpu_benchmarks, NULL, 0, &cpu_benchmarks_id
    );
#else
    cpu_benchmarks_id = fork();
    if (cpu_benchmarks_id == 0) {
        _exit(cpu_benchmarks());
    }
#endif
}

int CLIENT_STATE::set_nslots() {
    int retval;

    // Set nslots to actual # of CPUs (or less, depending on prefs)
    //
    if (host_info.p_ncpus > 0) {
        nslots = host_info.p_ncpus;
    } else {
        nslots = 1;
    }
    if (nslots > global_prefs.max_cpus) nslots = global_prefs.max_cpus;

    retval = make_slot_dirs();
    if (retval) return retval;

    return 0;
}

// Returns true if CPU benchmarks should be run:
// flag is set or it's been a month since we last ran
//
bool CLIENT_STATE::should_run_cpu_benchmarks() {
    // Note: we if skip_cpu_benchmarks we still should "run" cpu benchmarks
    // (we'll just use default values in cpu_benchmarks())
    return (
        run_cpu_benchmarks ||
        (difftime(time(0), (time_t)host_info.p_calculated) > BENCHMARK_PERIOD)
    );
}

#ifdef _WIN32
DWORD WINAPI CLIENT_STATE::win_cpu_benchmarks(LPVOID) {
    return gstate.cpu_benchmarks();
}
#endif

// gets info about the host
// NOTE: this locks up the process for 10-20 seconds,
// so it should be called very seldom
//
int CLIENT_STATE::cpu_benchmarks() {
    HOST_INFO host_info;
    FILE* finfo;
    double fpop_test_secs = 3.3;
    double iop_test_secs = 3.3;
    double mem_test_secs = 3.3;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_MEASUREMENT);
    scope_messages.printf("CLIENT_STATE::cpu_benchmarks(): Running CPU benchmarks.\n");

#ifdef _WIN32
	guiOnBenchmarksBegin();
#endif

    clear_host_info(host_info);
    ++log_messages;
    if (skip_cpu_benchmarks) {
        scope_messages.printf("CLIENT_STATE::cpu_benchmarks(): Skipping CPU benchmarks.\n");
        host_info.p_fpops = 1e9;
        host_info.p_iops = 1e9;
        host_info.p_membw = 4e9;
        host_info.m_cache = 1e6;
    } else {
        scope_messages.printf(
            "CLIENT_STATE::cpu_benchmarks(): Running floating point test for about %.1f seconds.\n",
            fpop_test_secs
        );
        host_info.p_fpop_err = run_double_prec_test(fpop_test_secs, host_info.p_fpops);

        scope_messages.printf(
            "CLIENT_STATE::cpu_benchmarks(): Running integer test for about %.1f seconds.\n",
            iop_test_secs
        );
        host_info.p_iop_err = run_int_test(iop_test_secs, host_info.p_iops);

        scope_messages.printf(
            "CLIENT_STATE::cpu_benchmarks(): Running memory bandwidth test for about %.1f seconds.\n",
            mem_test_secs
        );
        host_info.p_membw_err = run_mem_bandwidth_test(mem_test_secs, host_info.p_membw);

        // need to check cache!!
        host_info.m_cache = 1e6;

        msg_printf(NULL, MSG_INFO, "Benchmark results: FP: %.0fe6%s;  Int: %.0fe6%s;  Mem BW: %.0fe6%s",
                   host_info.p_fpops/1e6, (host_info.p_fpop_err?" [ERROR]":""),
                   host_info.p_iops/1e6,  (host_info.p_iop_err?" [ERROR]":""),
                   host_info.p_membw/1e6, (host_info.p_membw_err?" [ERROR]":"")
        );
    }

    host_info.p_calculated = (double)time(0);
    finfo = fopen(CPU_BENCHMARKS_FILE_NAME, "w");
    if(!finfo) return ERR_FOPEN;
    host_info.write_cpu_benchmarks(finfo);
    fclose(finfo);
#ifdef _WIN32
	guiOnBenchmarksEnd();
#endif
    --log_messages;
    return 0;
}

// checks if the CPU benchmarks are running
//
int CLIENT_STATE::check_cpu_benchmarks() {
    FILE* finfo;
    int retval;

    if (cpu_benchmarks_id) {
#ifdef _WIN32
        DWORD exit_code = 0;
        GetExitCodeThread(cpu_benchmarks_handle, &exit_code);
        if(exit_code == STILL_ACTIVE) {
            if(time(NULL) > cpu_benchmarks_start + MAX_CPU_BENCHMARKS_SECONDS) {
                msg_printf(NULL, MSG_ERROR, "CPU benchmarks timed out, using default values");
                TerminateThread(cpu_benchmarks_handle, 0);
                CloseHandle(cpu_benchmarks_handle);
                host_info.p_fpops = 1e9;
                host_info.p_iops = 1e9;
                host_info.p_membw = 4e9;
                host_info.m_cache = 1e6;
                cpu_benchmarks_id = 0;
                return CPU_BENCHMARKS_ERROR;
            }
            return CPU_BENCHMARKS_RUNNING;
        }
        CloseHandle(cpu_benchmarks_handle);
		guiOnBenchmarksEnd();
#else
        int exit_code = 0;
        retval = waitpid(cpu_benchmarks_id, &exit_code, WNOHANG);
        if(retval == 0) {
            if((unsigned int)time(NULL) > cpu_benchmarks_start + MAX_CPU_BENCHMARKS_SECONDS) {
                msg_printf(NULL, MSG_ERROR, "CPU benchmarks timed out, using default values");
                kill(cpu_benchmarks_id, SIGKILL);
                host_info.p_fpops = 1e9;
                host_info.p_iops = 1e9;
                host_info.p_membw = 4e9;
                host_info.m_cache = 1e6;
                cpu_benchmarks_id = 0;
                return CPU_BENCHMARKS_ERROR;
            }
            return CPU_BENCHMARKS_RUNNING;
        }
#endif
        cpu_benchmarks_id = 0;
        msg_printf(NULL, MSG_INFO, "CPU benchmarks complete");
        finfo = fopen(CPU_BENCHMARKS_FILE_NAME, "r");
        if (!finfo) {
            msg_printf(NULL, MSG_ERROR, "Can't open CPU benchmark file, using default values");
            host_info.p_fpops = 1e9;
            host_info.p_iops = 1e9;
            host_info.p_membw = 4e9;
            host_info.m_cache = 1e6;
            return CPU_BENCHMARKS_ERROR;
        }
        retval = host_info.parse_cpu_benchmarks(finfo);
        fclose(finfo);
        if (retval) return CPU_BENCHMARKS_ERROR;
        file_delete(CPU_BENCHMARKS_FILE_NAME);
        return CPU_BENCHMARKS_COMPLETE;
    }
    return CPU_BENCHMARKS_NOT_RUNNING;
}

// Return the maximum allowed disk usage as determined by user preferences.
// There are three different settings in the prefs;
// return the least of the three.
//
int CLIENT_STATE::allowed_disk_usage(double& size) {
    double percent_space, min_val;

    percent_space = host_info.d_total*global_prefs.disk_max_used_pct;

    min_val = host_info.d_free - global_prefs.disk_min_free_gb*(1024.*1024.*1024.);

    size = min(min(global_prefs.disk_max_used_gb*(1024.*1024.*1024.), percent_space), min_val);
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

// estimate how long a WU will take on this host
//
double CLIENT_STATE::estimate_cpu_time(WORKUNIT& wu) {
    double x;

    x = wu.rsc_fpops/host_info.p_fpops;
    x += wu.rsc_iops/host_info.p_iops;
    return x;
}

// returns true if start_hour == end_hour or start_hour <= now < end_hour
inline bool now_between_two_hours(int start_hour, int end_hour)
{
    if (start_hour == end_hour) {
        // always work
        return true;
    }

    time_t now = time(0);
    struct tm *tmp = localtime(&now);
    int hour = tmp->tm_hour;
    if (start_hour < end_hour) {
        return (hour >= start_hour && hour < end_hour);
    } else {
        return !(hour >= end_hour && hour < start_hour);
    }
}

enum SUSPEND_REASON_t {
    SUSPEND_REASON_BATTERIES = 1,
    SUSPEND_REASON_USER_ACTIVE = 2,
    SUSPEND_REASON_USER_REQ = 4,
    SUSPEND_REASON_TIME_OF_DAY = 8,
    SUSPEND_REASON_BENCHMARKS = 16
};

// See if (on the basis of user run request and prefs)
// we should suspend activities.
//
void CLIENT_STATE::check_suspend_activities(int& reason) {
    reason = 0;

    // Don't work while we're running CPU benchmarks
    //
    if (check_cpu_benchmarks() == CPU_BENCHMARKS_RUNNING) {
        reason |= SUSPEND_REASON_BENCHMARKS;
    }

    if (user_run_request == USER_RUN_REQUEST_ALWAYS) return;

    if (user_run_request == USER_RUN_REQUEST_NEVER) {
        reason |= SUSPEND_REASON_USER_REQ;
        return;
    }

    if (!global_prefs.run_on_batteries && host_is_running_on_batteries()) {
        reason |= SUSPEND_REASON_BATTERIES;
    }

    // user_idle is set in the Mac/Win GUI code
    //
    if (!global_prefs.run_if_user_active && !user_idle) {
        reason |= SUSPEND_REASON_USER_ACTIVE;
    }

    if (!now_between_two_hours(global_prefs.start_hour, global_prefs.end_hour)) {
        reason |= SUSPEND_REASON_TIME_OF_DAY;
    }

    return;
}

// sleep up to x seconds,
// but if network I/O becomes possible,
// wake up and do as much as limits allow.
// If suspended, just sleep x seconds
//
int CLIENT_STATE::net_sleep(double x) {
    if (activities_suspended) {
        boinc_sleep(x);
        return 0;
    } else {
        return net_xfers->net_sleep(x);
    }
}

int CLIENT_STATE::suspend_activities(int reason) {
    string s_reason;
    s_reason = "Suspending computation and file transfer";
    if (reason & SUSPEND_REASON_BATTERIES) {
        s_reason += " - on batteries";
    }
    if (reason & SUSPEND_REASON_USER_ACTIVE) {
        s_reason += " - user is active";
    }
    if (reason & SUSPEND_REASON_USER_REQ) {
        s_reason += " - user request";
    }
    if (reason & SUSPEND_REASON_TIME_OF_DAY) {
        s_reason += " - time of day";
    }
    if (reason & SUSPEND_REASON_BENCHMARKS) {
        s_reason += " - running CPU benchmarks";
    }
    msg_printf(NULL, MSG_INFO, const_cast<char*>(s_reason.c_str()));
    active_tasks.suspend_all();
    pers_file_xfers->suspend();
    return 0;
}

// persistent file xfers will resume of their own accord
// since activities_suspended is now true
//
int CLIENT_STATE::resume_activities() {
    msg_printf(NULL, MSG_INFO, "Resuming activity");
    active_tasks.unsuspend_all();
    return 0;
}

#define POLL_ACTION(name, func)                                                \
    do { if (func()) {                                                         \
            ++actions;                                                         \
            scope_messages.printf("CLIENT_STATE::do_evil(): active task: " #name "\n"); \
        } } while(0)

// do_something polls each of the client's finite-state machine layers,
// possibly triggering state transitions.
// Returns true if something happened
// (in which case should call this again immediately)
//
bool CLIENT_STATE::do_something() {
    int actions = 0, reason;
    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_POLL);

    check_suspend_activities(reason);
    if (reason) {
        if (!activities_suspended) {
            suspend_activities(reason);
        }
    } else {
        if (activities_suspended) {
            resume_activities();
        }
    }
    previous_activities_suspended = activities_suspended;
    activities_suspended = (reason != 0);

    // if we're doing CPU benchmarks, don't do anything else
    //
    if (reason & SUSPEND_REASON_BENCHMARKS) return false;

    scope_messages.printf("CLIENT_STATE::do_evil(): Begin poll:\n");
    ++scope_messages;

    ss_logic.poll();
    if (activities_suspended) {
        scope_messages.printf("CLIENT_STATE::do_evil(): No active tasks! (suspended)\n");
        POLL_ACTION(net_xfers              , net_xfers->poll        );
        POLL_ACTION(http_ops               , http_ops->poll         );
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
    } else {
        net_stats.poll(*net_xfers);
        // Call these functions in bottom to top order with
        // respect to the FSM hierarchy
        //
        POLL_ACTION(net_xfers              , net_xfers->poll        );
        POLL_ACTION(http_ops               , http_ops->poll         );
        POLL_ACTION(file_xfers             , file_xfers->poll       );
        POLL_ACTION(active_tasks           , active_tasks.poll      );
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
        POLL_ACTION(start_apps             , start_apps             );
        POLL_ACTION(pers_file_xfers        , pers_file_xfers->poll       );
        POLL_ACTION(handle_finished_apps   , handle_finished_apps   );
        POLL_ACTION(handle_pers_file_xfers , handle_pers_file_xfers );
        POLL_ACTION(garbage_collect        , garbage_collect        );
        POLL_ACTION(update_results         , update_results         );
    }

    if (write_state_file_if_needed()) {
        msg_printf(NULL, MSG_ERROR, "Couldn't write state file");
    }
    --log_messages;
    scope_messages.printf(
        "CLIENT_STATE::do_evil(): End poll: %d tasks active\n", actions
    );
    if (actions > 0) {
        return true;
    } else {
        time_stats.update(true, !activities_suspended);
        return false;
    }
}

// Parse the client_state.xml file
//
int CLIENT_STATE::parse_state_file() {
    char buf[256];
    FILE* f = fopen(STATE_FILE_NAME, "r");
    PROJECT temp_project, *project=NULL;
    int retval=0;
    int failnum;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_STATE);
    if (!f) {
        scope_messages.printf("CLIENT_STATE::parse_state_file(): No state file; will create one\n");

        // avoid warning messages about version
        //
        old_major_version = MAJOR_VERSION;
        old_minor_version = MINOR_VERSION;
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
                msg_printf(NULL, MSG_ERROR, "Project %s found in state file but not prefs.\n",
                    temp_project.master_url);
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
                    retval = pers_file_xfers->insert( fip->pers_file_xfer );
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
            } else {
                msg_printf(NULL, MSG_ERROR,
                    "<result> found before any project\n"
                );
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
        } else if (parse_int(buf, "<core_client_major_version>", old_major_version)) {
        } else if (parse_int(buf, "<core_client_minor_version>", old_minor_version)) {
        } else if (match_tag(buf, "<use_http_proxy/>")) {
            use_http_proxy = true;
        } else if (match_tag(buf, "<use_socks_proxy/>")) {
            use_socks_proxy = true;
        } else if (parse_str(buf, "<proxy_server_name>", proxy_server_name, sizeof(proxy_server_name))) {
        } else if (parse_int(buf, "<proxy_server_port>", proxy_server_port)) {
        } else if (parse_str(buf, "<socks_user_name>", socks_user_name, sizeof(socks_user_name))) {
        } else if (parse_str(buf, "<socks_user_passwd>", socks_user_passwd, sizeof(socks_user_passwd))) {
        // } else if (parse_int(buf, "<user_run_request/>")) {
        } else if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) {
        } else {
            msg_printf(NULL, MSG_ERROR, "CLIENT_STATE::parse_state_file: unrecognized: %s\n", buf);
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

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_STATE);
    scope_messages.printf("CLIENT_STATE::write_state_file(): Writing state file\n");
    if (!f) {
        msg_printf(0, MSG_ERROR, "Can't open temp state file: %s\n", STATE_FILE_TEMP);
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
#if 0
    fprintf(f, "<user_run_request>%d</user_run_request>\n", user_run_request);
#endif
    if (strlen(host_venue)) {
        fprintf(f, "<host_venue>%s</host_venue>\n", host_venue);
    }
    fprintf(f, "</client_state>\n");
    fclose(f);
    retval = boinc_rename(STATE_FILE_TEMP, STATE_FILE_NAME);
    scope_messages.printf("CLIENT_STATE::write_state_file(): Done writing state file\n");
    if (retval) return ERR_RENAME;
    return 0;
}

// Write the client_state.xml file if necessary
// TODO: write no more often than X seconds
//
int CLIENT_STATE::write_state_file_if_needed() {
    int retval;
    long idle = time(0) - last_write_state_file;
    if (client_state_dirty && idle > global_prefs.disk_interval) {
        client_state_dirty = false;
        retval = write_state_file();
        if (retval) return retval;
        time(&last_write_state_file);
    }
    return 0;
}

// See if the project specified by master_url already exists
// in the client state record.  Ignore any trailing "/" characters
//
PROJECT* CLIENT_STATE::lookup_project(char* master_url) {
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
        msg_printf(0, MSG_ERROR, "app_version refers to nonexistent app: %s\n", avp->app_name);
        return 1;
    }
    avp->app = app;

    for (i=0; i<avp->app_files.size(); i++) {
        file_ref = avp->app_files[i];
        fip = lookup_file_info(p, file_ref.file_name);
        if (!fip) {
            msg_printf(0, MSG_ERROR, "app_version refers to nonexistent file: %s\n",
                file_ref.file_name);
            return 1;
        }

        // any executable file associated with an app version must be signed
        //
        if (fip->executable) {
            fip->signature_required = true;
        }
        avp->app_files[i].file_info = fip;
    }
    return 0;
}

int CLIENT_STATE::link_file_ref(PROJECT* p, FILE_REF* file_refp) {
    FILE_INFO* fip;

    fip = lookup_file_info(p, file_refp->file_name);
    if (!fip) {
        msg_printf(0, MSG_ERROR, "File ref refers to nonexistent file: %s\n", file_refp->file_name);
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
        msg_printf(0, MSG_ERROR, "WU refers to nonexistent app: %s\n", wup->app_name);
        return 1;
    }
    avp = lookup_app_version(app, wup->version_num);
    if (!avp) {
        msg_printf(0, MSG_ERROR, "WU refers to nonexistent app_version: %s %d\n",
            wup->app_name, wup->version_num);
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
            msg_printf(0, MSG_ERROR, "link_result: link_file_ref failed\n");
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
        msg_printf(0, MSG_ERROR, "CLIENT_STATE::latest_version_num: no version\n");
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

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_STATE);
    scope_messages.printf("CLIENT_STATE::print_summary(): Client state summary:\n");
    ++log_messages;
    scope_messages.printf("%d projects:\n", (int)projects.size());
    for (i=0; i<projects.size(); i++) {
        t = projects[i]->min_rpc_time;
        if (t) {
            scope_messages.printf("    %s min RPC %d seconds from now\n", projects[i]->master_url, (int)(t-time(0)));
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

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_STATE);

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
            scope_messages.printf("CLIENT_STATE::garbage_collect(): deleting result %s\n", rp->name);
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
            }
            rp->wup->ref_cnt++;
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
                        switch(failnum) {
                        case ERR_FILE_TOO_BIG:
                            report_result_error(*rp, 0, "Output file exceeded size limit");
                            break;
                        default:
                            report_result_error(*rp, 0, "Couldn't upload files or other output file error");
                        }
                    }
                }
                rp->output_files[i].file_info->ref_cnt++;
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
            avp->app_files[j].file_info->ref_cnt++;
        }
    }

    // delete FILE_INFOs (and corresponding files) that are not sticky
    // and are not referenced by any WORKUNIT, RESULT or APP_VERSION
    //
    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fip = *fi_iter;
        if (fip->ref_cnt==0 && fip->pers_file_xfer==NULL && !fip->sticky) {
            fip->delete_file();
            scope_messages.printf("CLIENT_STATE::garbage_collect(): deleting file %s\n", fip->name);
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
        if (rp->server_ack)
            action = true;

        switch (rp->state) {
        case RESULT_NEW:
            rp->state = RESULT_FILES_DOWNLOADING;
            action = true;
            break;
        case RESULT_FILES_DOWNLOADING:
            if (input_files_available(rp)) {
                rp->state = RESULT_FILES_DOWNLOADED;
                action = true;
            }
            break;

            // app_finished() transitions to either RESULT_COMPUTE_DONE or
            // RESULT_FILES_UPLOADING. RESULT_COMPUTE_DONE is a dead-end state
            // indicating we had an error at the end of computation.

        // case RESULT_FILES_DOWNLOADED:
        //     break;
        // case RESULT_COMPUTE_DONE:
        //     rp->state = RESULT_FILES_UPLOADING;
        //     action = true;
        //     break;
        case RESULT_FILES_UPLOADING:
            // Once the computation has been done, check that the necessary
            // files have been uploaded before moving on
            if (rp->is_upload_done()) {
                rp->ready_to_ack = true;
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


// Parse the command line arguments passed to the client
// NOTE: init() has not been called at this point
// (i.e. client_state.xml has not been parsed)
//
void CLIENT_STATE::parse_cmdline(int argc, char** argv) {
    int i;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-exit_when_idle")) {
            exit_when_idle = true;
        } else if (!strcmp(argv[i], "-skip_cpu_benchmarks")) {
            skip_cpu_benchmarks = true;
        } else if (!strcmp(argv[i], "-exit_after_app_start")) {
            exit_after_app_start_secs = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-file_xfer_giveup_period")) {
            file_xfer_giveup_period = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-min")) {
            global_prefs.run_minimized = true;
        } else if (!strcmp(argv[i], "-saver")) {
            start_saver = true;
        } else if (!strncmp(argv[i], "-psn_", strlen("-psn_"))) {
            // ignore -psn argument on Mac OS X
        } else if (!strcmp(argv[i], "-exit_before_upload")) {
            exit_before_upload = true;
        // The following are only used for testing to alter scheduler/file transfer
        // backoff rates
        } else if (!strcmp(argv[i], "-master_fetch_period")) {
            master_fetch_period = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-retry_base_period")) {
            retry_base_period = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-retry_cap")) {
            retry_cap = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-master_fetch_retry_cap")) {
            master_fetch_retry_cap = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-master_fetch_interval")) {
            master_fetch_interval = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-sched_retry_delay_min")) {
            sched_retry_delay_min = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-sched_retry_delay_max")) {
            sched_retry_delay_max = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-pers_retry_delay_min")) {
            pers_retry_delay_min = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-pers_retry_delay_max")) {
            pers_retry_delay_max = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-pers_giveup")) {
            pers_giveup = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-debug_fake_exponential_backoff")) {
            debug_fake_exponential_backoff = true;

        // the above options are private (i.e. not shown by -help)

        } else if (!strcmp(argv[i], "-show_projects")) {
            show_projects = true;
        } else if (!strcmp(argv[i], "-detach_project")) {
            strcpy(detach_project_url, argv[++i]);
        } else if (!strcmp(argv[i], "-reset_project")) {
            strcpy(reset_project_url, argv[++i]);
        } else if (!strcmp(argv[i], "-update_prefs")) {
            strcpy(update_prefs_url, argv[++i]);
        } else if (!strcmp(argv[i], "-run_cpu_benchmarks")) {
            run_cpu_benchmarks = true;
        } else if (!strcmp(argv[i], "-attach_project")) {
            add_new_project();
        } else if (!strcmp(argv[i], "-version")) {
            printf( "%.2f %s\n", MAJOR_VERSION+(MINOR_VERSION/100.0), HOSTTYPE );
            exit(0);
        } else if (!strcmp(argv[i], "-help")) {
            printf(
                "Usage: %s [options]\n"
                "    -version                show version info\n"
                "    -attach_project         attach to a project (will prompt for URL, account key)\n"
                "    -update_prefs           contact all projects to update preferences\n"
                "    -run_cpu_benchmarks     run the CPU benchmarks\n",
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
        safe_strcpy(socks_user_name, p);
    }

    if ((p = getenv("SOCKS_PASSWD"))) {
        safe_strcpy(socks_user_passwd, p);
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
    log_messages.printf(ClientMessages::DEBUG_STATE, "set dirty: %s\n", source);
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

    sprintf(buf, "Unrecoverable error for result %s (%s)", res.name, err_msg);
    scheduler_op->backoff(res.project, buf);

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
    res.stderr_out.append(buf);

    if ((res.state == RESULT_FILES_DOWNLOADED) && err_num) {
        sprintf(buf,"<couldnt_start>%d</couldnt_start>\n", err_num);
        res.stderr_out.append(buf);
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
                res.stderr_out.append(buf);
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
                res.stderr_out.append(buf);
            }
        }
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
int CLIENT_STATE::reset_project(PROJECT* project) {
    unsigned int i;
    APP_VERSION* avp;
    APP* app;
    ACTIVE_TASK* atp;
    vector<APP*>::iterator app_iter;
    vector<APP_VERSION*>::iterator avp_iter;
    RESULT* rp;
    PERS_FILE_XFER* pxp;

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->result->project == project) {
            atp->abort();
            active_tasks.remove(atp);
            i--;
        }
    }

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

    if (scheduler_op->state != SCHEDULER_OP_STATE_IDLE
        && scheduler_op->project == project
    ) {
        http_ops->remove(&scheduler_op->http_op);
    }

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == project) {
            rp->server_ack = true;
        }
    }

    avp_iter = app_versions.begin();
    while (avp_iter != app_versions.end()) {
        avp = *avp_iter;
        if (avp->project == project) {
            avp_iter = app_versions.erase(avp_iter);
        } else {
            avp_iter++;
        }
    }

    app_iter = apps.begin();
    while (app_iter != apps.end()) {
        app = *app_iter;
        if (app->project == project) {
            app_iter = apps.erase(app_iter);
        } else {
            app_iter++;
        }
    }

    garbage_collect();
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
    vector<PROJECT*>::iterator iter;
    PROJECT* p;
    char path[256];
    int retval;

    reset_project(project);

    // find project and remove it from the vector
    //
    for (iter = projects.begin(); iter != projects.end(); iter++) {
        p = *iter;
        if (p == project) {
            projects.erase(iter);
            break;
        }
    }

    // delete account file
    //
    get_account_filename(project->master_url, path);
    retval = file_delete(path);

    // remove project directory and its contents
    //
    remove_project_dir(*project);
    delete project;
    write_state_file();

    return 0;
}

void CLIENT_STATE::check_project_pointer(PROJECT* p) {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        if (p == projects[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_app_pointer(APP* p) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (p == apps[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_file_info_pointer(FILE_INFO* p) {
    unsigned int i;
    for (i=0; i<file_infos.size(); i++) {
        if (p == file_infos[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_app_version_pointer(APP_VERSION* p) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (p == app_versions[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_workunit_pointer(WORKUNIT* p) {
    unsigned int i;
    for (i=0; i<workunits.size(); i++) {
        if (p == workunits[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_result_pointer(RESULT* p) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (p == results[i]) return;
    }
    assert(0);
}

void CLIENT_STATE::check_pers_file_xfer_pointer(PERS_FILE_XFER* p) {
    unsigned int i;
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        if (p == pers_file_xfers->pers_file_xfers[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_file_xfer_pointer(FILE_XFER* p) {
    unsigned int i;
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        if (p == file_xfers->file_xfers[i]) return;
    }
    assert(0);
}

void CLIENT_STATE::check_app(APP& p) {
    check_project_pointer(p.project);
}

void CLIENT_STATE::check_file_info(FILE_INFO& p) {
    if (p.pers_file_xfer) check_pers_file_xfer_pointer(p.pers_file_xfer);
    if (p.result) check_result_pointer(p.result);
    check_project_pointer(p.project);
}

void CLIENT_STATE::check_file_ref(FILE_REF& p) {
    check_file_info_pointer(p.file_info);
}

void CLIENT_STATE::check_app_version(APP_VERSION& p) {
    unsigned int i;
    check_app_pointer(p.app);
    check_project_pointer(p.project);
    for (i=0; i<p.app_files.size(); i++) {
        check_file_ref(p.app_files[i]);
    }
}

void CLIENT_STATE::check_workunit(WORKUNIT& p) {
    unsigned int i;
    for (i=0; i<p.input_files.size(); i++) {
        check_file_ref(p.input_files[i]);
    }
    check_project_pointer(p.project);
    check_app_pointer(p.app);
    check_app_version_pointer(p.avp);
}

void CLIENT_STATE::check_result(RESULT& p) {
    unsigned int i;
    for (i=0; i<p.output_files.size(); i++) {
        check_file_ref(p.output_files[i]);
    }
    check_app_pointer(p.app);
    check_workunit_pointer(p.wup);
    check_project_pointer(p.project);
}

void CLIENT_STATE::check_active_task(ACTIVE_TASK& p) {
    check_result_pointer(p.result);
    check_workunit_pointer(p.wup);
    check_app_version_pointer(p.app_version);
}

void CLIENT_STATE::check_pers_file_xfer(PERS_FILE_XFER& p) {
    check_file_xfer_pointer(p.fxp);
    check_file_info_pointer(p.fip);
}

void CLIENT_STATE::check_file_xfer(FILE_XFER& p) {
    check_file_info_pointer(p.fip);
}

void CLIENT_STATE::check_all() {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        check_app(*apps[i]);
    }
    for (i=0; i<file_infos.size(); i++) {
        check_file_info(*file_infos[i]);
    }
    for (i=0; i<app_versions.size(); i++) {
        check_app_version(*app_versions[i]);
    }
    for (i=0; i<workunits.size(); i++) {
        check_workunit(*workunits[i]);
    }
    for (i=0; i<results.size(); i++) {
        check_result(*results[i]);
    }
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        check_active_task(*active_tasks.active_tasks[i]);
    }
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        check_pers_file_xfer(*pers_file_xfers->pers_file_xfers[i]);
    }
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        check_file_xfer(*file_xfers->file_xfers[i]);
    }
}
