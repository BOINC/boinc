// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// client initialization and main loop

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
#include <cmath>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif

#ifdef __EMX__
#define INCL_DOS
#include <os2.h>
#endif

#include "cpp.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "url.h"
#include "util.h"
#ifdef _WIN32
#include "run_app_windows.h"
#endif

#include "app_config.h"
#include "async_file.h"
#include "client_msgs.h"
#include "cs_notice.h"
#include "cs_proxy.h"
#include "cs_trickle.h"
#include "file_names.h"
#include "hostinfo.h"
#include "http_curl.h"
#include "network.h"
#include "project.h"
#include "result.h"
#include "sandbox.h"
#include "shmem.h"

#include "client_state.h"

using std::max;

CLIENT_STATE gstate;
COPROCS coprocs;

#ifndef SIM
THREAD_LOCK client_thread_mutex;
THREAD throttle_thread;
#endif

CLIENT_STATE::CLIENT_STATE()
    : lookup_website_op(&gui_http),
    get_current_version_op(&gui_http),
    get_project_list_op(&gui_http),
    acct_mgr_op(&gui_http),
    lookup_login_token_op(&gui_http)
{
    http_ops = new HTTP_OP_SET();
    file_xfers = new FILE_XFER_SET(http_ops);
    pers_file_xfers = new PERS_FILE_XFER_SET(file_xfers);
#ifndef SIM
    scheduler_op = new SCHEDULER_OP(http_ops);
#endif
    time_stats.init();
    client_state_dirty = false;
    old_major_version = 0;
    old_minor_version = 0;
    old_release = 0;
    clock_change = false;
    check_all_logins = false;
    user_active = false;
    cmdline_gui_rpc_port = 0;
    run_cpu_benchmarks = false;
    file_xfer_giveup_period = PERS_GIVEUP;
    had_or_requested_work = false;
    tasks_suspended = false;
    tasks_throttled = false;
    network_suspended = false;
    file_xfers_suspended = false;
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
    safe_strcpy(language, "");
    safe_strcpy(client_brand, "");
    exit_after_app_start_secs = 0;
    app_started = 0;
    cmdline_dir = false;
    exit_before_upload = false;
#ifndef _WIN32
    boinc_project_gid = 0;
#endif
    show_projects = false;
    safe_strcpy(detach_project_url, "");
    safe_strcpy(reset_project_url, "");
    safe_strcpy(update_prefs_url, "");
    safe_strcpy(main_host_venue, "");
    safe_strcpy(attach_project_url, "");
    safe_strcpy(attach_project_auth, "");
    cpu_run_mode.set(RUN_MODE_AUTO, 0);
    gpu_run_mode.set(RUN_MODE_AUTO, 0);
    network_run_mode.set(RUN_MODE_AUTO, 0);
    started_by_screensaver = false;
    requested_exit = false;
    os_requested_suspend = false;
    os_requested_suspend_time = 0;
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
    cant_write_state_file = false;
    n_usable_cpus = 1;
    benchmarks_running = false;
    client_disk_usage = 0.0;
    total_disk_usage = 0.0;
    device_status_time = 0;

    rec_interval_start = 0;
    total_cpu_time_this_rec_interval = 0.0;
    must_enforce_cpu_schedule = false;
    must_schedule_cpus = true;
    must_check_work_fetch = true;
    retry_shmem_time = 0;
    no_gui_rpc = false;
    autologin_in_progress = false;
    autologin_fetching_project_list = false;
    gui_rpc_unix_domain = false;
    gui_rpc_websocket = false;
    new_version_check_time = 0;
    all_projects_list_check_time = 0;
    client_version_check_url = DEFAULT_VERSION_CHECK_URL;
    detach_console = false;
#ifdef SANDBOX
    g_use_sandbox = true; // User can override with -insecure command-line arg
#endif
    launched_by_manager = false;
    run_by_updater = false;
    now = 0.0;
    initialized = false;
    last_wakeup_time = dtime();
    device_status_time = 0;
#ifdef _WIN32
    have_sysmon_msg = false;
#endif
    have_sporadic_app = false;
}

void CLIENT_STATE::show_host_info() {
    char buf[256], buf2[256];

    msg_printf(NULL, MSG_INFO,
        "Computer name: %s",
        host_info.domain_name
    );
    nbytes_to_string(host_info.m_cache, 0, buf, sizeof(buf));
    msg_printf(NULL, MSG_INFO,
        "Processor: %d %s %s",
        host_info.p_ncpus, host_info.p_vendor, host_info.p_model
    );
    if (n_usable_cpus != host_info.p_ncpus) {
        msg_printf(NULL, MSG_INFO, "Using %d CPUs", n_usable_cpus);
    }
#if 0
    if (host_info.m_cache > 0) {
        msg_printf(NULL, MSG_INFO,
            "Processor: %s cache",
            buf
        );
    }
#endif
    msg_printf(NULL, MSG_INFO,
        "Processor features: %s", host_info.p_features
    );
#ifdef __APPLE__
    buf[0] = '\0';
    FILE *f = popen("sw_vers -productVersion", "r");
    fgets(buf, sizeof(buf), f);
    strip_whitespace(buf);
    pclose(f);
    msg_printf(NULL, MSG_INFO,
        "OS: Mac OS X %s (%s %s)", buf,
        host_info.os_name, host_info.os_version
    );
#else
    msg_printf(NULL, MSG_INFO,
        "OS: %s: %s", host_info.os_name, host_info.os_version
    );
#endif

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

#ifdef _WIN64
    if (host_info.wsl_distros.distros.empty()) {
        msg_printf(NULL, MSG_INFO, "WSL: no usable distros found");
    } else {
        msg_printf(NULL, MSG_INFO, "Usable WSL distros:");
        for (auto& wsl : host_info.wsl_distros.distros) {
            msg_printf(NULL, MSG_INFO,
                "-   %s (WSL %d)%s",
                wsl.distro_name.c_str(),
                wsl.wsl_version,
                wsl.is_default ? " (default)" : ""
            );
            msg_printf(NULL, MSG_INFO,
                "-      OS: %s (%s)",
                wsl.os_name.c_str(), wsl.os_version.c_str()
            );
            if (!wsl.libc_version.empty()) {
                msg_printf(NULL, MSG_INFO,
                    "-      libc version: %s", wsl.libc_version.c_str()
                );
            }
            if (!wsl.docker_version.empty()) {
                msg_printf(NULL, MSG_INFO, "-      Docker version %s (%s)",
                    wsl.docker_version.c_str(),
                    docker_type_str(wsl.docker_type)
                );
            }
            if (!wsl.docker_compose_version.empty()) {
                msg_printf(NULL, MSG_INFO, "-      Docker compose version %s (%s)",
                    wsl.docker_compose_version.c_str(),
                    docker_type_str(wsl.docker_compose_type)
                );
            }
            if (wsl.boinc_buda_runner_version) {
                msg_printf(NULL, MSG_INFO, "-      BOINC WSL distro version %d",
                    wsl.boinc_buda_runner_version
                );
            }
        }
    }
#endif

    if (strlen(host_info.virtualbox_version)) {
        msg_printf(NULL, MSG_INFO,
            "VirtualBox version: %s",
            host_info.virtualbox_version
        );
    } else {
#if defined (_WIN32) && !defined(_WIN64)
        if (!strcmp(get_primary_platform(), "windows_x86_64")) {
            msg_printf(NULL, MSG_USER_ALERT,
                "Can't detect VirtualBox because this is a 32-bit version of BOINC; to fix, please install a 64-bit version."
            );
        }
#endif
    }

#ifndef _WIN64
    if (strlen(host_info.docker_version)) {
        msg_printf(NULL, MSG_INFO, "Docker: version %s (%s)",
            host_info.docker_version,
            docker_type_str(host_info.docker_type)
        );
    }
    if (strlen(host_info.docker_compose_version)) {
        msg_printf(NULL, MSG_INFO, "Docker compose: version %s (%s)",
            host_info.docker_compose_version,
            docker_type_str(host_info.docker_compose_type)
        );
    }
#endif
}

// TODO: the following 3 should be members of COPROCS

int rsc_index(const char* name) {
    const char* nm = strcmp(name, "CUDA")?name:GPU_TYPE_NVIDIA;
        // handle old state files
    for (int i=0; i<coprocs.n_rsc; i++) {
        if (!strcmp(nm, coprocs.coprocs[i].type)) {
            return i;
        }
    }
    return -1;
}

// used in XML and COPROC::type
//
const char* rsc_name(int i) {
    return coprocs.coprocs[i].type;
}

// user-friendly version
//
const char* rsc_name_long(int i) {
    int num = coproc_type_name_to_num(coprocs.coprocs[i].type);
    if (num >= 0) return proc_type_name(num);   // CPU, NVIDIA GPU, AMD GPU or Intel GPU
    return coprocs.coprocs[i].type;             // Some other type
}

#ifndef SIM
// alert user if any jobs need more RAM than available
// (based on RAM estimate, not measured size)
//
static void check_too_large_jobs() {
    unsigned int i, j;
    double m = gstate.max_available_ram();
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        bool found = false;
        for (j=0; j<gstate.results.size(); j++) {
            RESULT* rp = gstate.results[j];
            if (rp->project == p && rp->wup->rsc_memory_bound > m) {
                found = true;
                break;
            }
        }
        if (found) {
            msg_printf(p, MSG_USER_ALERT,
                _("Some tasks need more memory than allowed by your preferences.  Please check the preferences.")
            );
        }
    }
}
#endif

// Something has failed N times.
// Calculate an exponential backoff between MIN and MAX
//
double calculate_exponential_backoff(int n, double MIN, double MAX) {
    double x = pow(2, (double)n);
    x *= MIN;
    if (x > MAX) x = MAX;
    x *= (.5 + .5*drand());
    return x;
}

#ifndef SIM

void CLIENT_STATE::set_now() {
    double x = dtime();

    // if time went backward significantly, clear delays
    //
    clock_change = false;
    if (x < (now-60)) {
        clock_change = true;
        msg_printf(NULL, MSG_INFO,
            "New system time (%.0f) < old system time (%.0f); clearing timeouts",
            x, now
        );
        clear_absolute_times();
    }

#ifdef _WIN32
    // On Win, check for evidence that we're awake after a suspension
    // (in case we missed the event announcing this)
    //
    if (os_requested_suspend) {
        if (x > now+10) {
            msg_printf(0, MSG_INFO, "Resuming after OS suspension");
            os_requested_suspend = false;
        } else if (x > os_requested_suspend_time + 300) {
            msg_printf(0, MSG_INFO, "Resuming after OS suspension");
            os_requested_suspend = false;
        }
    }
#endif
    now = x;
}

// Check if version or platform has changed;
// if so we're running a different client than before.
//
bool CLIENT_STATE::is_new_client() {
    bool new_client = false;
    if ((core_client_version.major != old_major_version)
        || (core_client_version.minor != old_minor_version)
        || (core_client_version.release != old_release)
    ) {
        if (old_major_version) {
            msg_printf_notice(0, true, 0,
                "The BOINC client version has changed from %d.%d.%d to %d.%d.%d.<br>To see what's new, view the <a href=%s>Client release notes</a>.",
                old_major_version, old_minor_version, old_release,
                core_client_version.major,
                core_client_version.minor,
                core_client_version.release,
                "https://github.com/BOINC/boinc/wiki/Client-release-notes"
            );
        }
        new_client = true;
    }
    if (statefile_platform_name.size() && strcmp(get_primary_platform(), statefile_platform_name.c_str())) {
        msg_printf(NULL, MSG_INFO,
            "Platform changed from %s to %s",
            statefile_platform_name.c_str(), get_primary_platform()
        );
        new_client = true;
    }
    return new_client;
}

#ifdef _WIN32
typedef DWORD (WINAPI *STP)(HANDLE, DWORD);
#endif

static void set_client_priority() {
#ifdef _WIN32
    STP stp = (STP) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "SetThreadPriority");
    if (!stp) return;
    if (stp(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN)) {
        msg_printf(NULL, MSG_INFO, "Running at background priority");
    } else {
        msg_printf(NULL, MSG_INFO, "Failed to set background priority");
    }
#endif
#ifdef __linux__
    char buf[1024];
    snprintf(buf, sizeof(buf), "ionice -c 3 -p %d", getpid());
    if (!system(buf)) {}
#endif
}

// initialize the client, and print messages about
// the host HW/SW and the configuration.
//
int CLIENT_STATE::init() {
    int retval;
    unsigned int i;
    char buf[256];
    PROJECT* p;

    srand((unsigned int)time(0));
    now = dtime();
#ifdef ANDROID
    device_status_time = dtime();
#endif
    scheduler_op->url_random = drand();

    notices.init();
    daily_xfer_history.init();
    time_stats.init();

    detect_platforms();
    time_stats.start();

    msg_printf(
        NULL, MSG_INFO, "Starting BOINC client version %d.%d.%d for %s%s",
        core_client_version.major,
        core_client_version.minor,
        core_client_version.release,
        HOSTTYPE,
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

    log_flags.show();

    msg_printf(NULL, MSG_INFO, "Libraries: %s", curl_version());

    if (cc_config.lower_client_priority) {
        set_client_priority();
    }

    if (executing_as_daemon) {
#ifdef _WIN32
        msg_printf(NULL, MSG_INFO, "Running as a daemon (GPU computing disabled)");
#else
        msg_printf(NULL, MSG_INFO, "Running as a daemon");
#endif
    }

    relative_to_absolute("", buf);
    msg_printf(NULL, MSG_INFO, "Data directory: %s", buf);

#ifdef _WIN32
    DWORD  buf_size = sizeof(buf);
    LPTSTR pbuf = buf;

    GetUserName(pbuf, &buf_size);
    msg_printf(NULL, MSG_INFO, "Running under account %s", pbuf);
#endif

    FILE* f = fopen(CLIENT_BRAND_FILENAME, "r");
    if (f) {
        if (fgets(client_brand, sizeof(client_brand), f)) {
            strip_whitespace(client_brand);
            msg_printf(NULL, MSG_INFO, "Client brand: %s", client_brand);
        }
        fclose(f);
    }

    // parse keyword file if present
    //
    f = fopen(KEYWORD_FILENAME, "r");
    if (f) {
        MIOFILE mf;
        mf.init_file(f);
        XML_PARSER xp(&mf);
        retval = keywords.parse(xp);
        if (!retval) keywords.present = true;
        fclose(f);
#if 0
        std::map<int, KEYWORD>::iterator it;
        for (it = keywords.keywords.begin(); it != keywords.keywords.end(); it++) {
            int id = it->first;
            KEYWORD& kw = it->second;
            printf("keyword %d: %s\n", id, kw.name.c_str());
        }
#endif
    }

    parse_account_files();
    parse_statistics_files();

    // check for GPUs.
    //
    coprocs.bound_counts();     // show GPUs described in cc_config.xml
    if (!cc_config.no_gpus
#ifdef _WIN32
        && !executing_as_daemon
#endif
        ) {
        vector<string> descs;
        vector<string> warnings;
        coprocs.get(
            cc_config.use_all_gpus, descs, warnings, cc_config.ignore_gpu_instance
        );
        for (i=0; i<descs.size(); i++) {
            msg_printf(NULL, MSG_INFO, "%s", descs[i].c_str());
        }
        if (log_flags.coproc_debug) {
            for (i=0; i<warnings.size(); i++) {
                msg_printf(NULL, MSG_INFO, "[coproc] %s", warnings[i].c_str());
            }
        }
#if 0
        msg_printf(NULL, MSG_INFO, "Faking an NVIDIA GPU");
        coprocs.nvidia.fake(18000, 512*MEGA, 490*MEGA, 2);
#endif
#if 0
        msg_printf(NULL, MSG_INFO, "Faking an ATI GPU");
        coprocs.ati.fake(512*MEGA, 256*MEGA, 2);
#endif
#if 0
        msg_printf(NULL, MSG_INFO, "Faking an Intel GPU");
        coprocs.intel_gpu.fake(512*MEGA, 256*MEGA, 2);
#endif
#if 0
        fake_opencl_gpu("Mali-T628");
#endif
    }

    if (coprocs.have_nvidia()) {
        if (rsc_index(GPU_TYPE_NVIDIA)>0) {
            msg_printf(NULL, MSG_INFO, "NVIDIA GPU info taken from cc_config.xml");
        } else {
            coprocs.add(coprocs.nvidia);
        }
    }
    if (coprocs.have_ati()) {
        if (rsc_index(GPU_TYPE_ATI)>0) {
            msg_printf(NULL, MSG_INFO, "ATI GPU info taken from cc_config.xml");
        } else {
            coprocs.add(coprocs.ati);
        }
    }
    if (coprocs.have_intel_gpu()) {
        if (rsc_index(GPU_TYPE_INTEL)>0) {
            msg_printf(NULL, MSG_INFO, "INTEL GPU info taken from cc_config.xml");
        } else {
            coprocs.add(coprocs.intel_gpu);
        }
    }
    if (coprocs.have_apple_gpu()) {
        if (rsc_index(GPU_TYPE_APPLE)>0) {
            msg_printf(NULL, MSG_INFO, "APPLE GPU info taken from cc_config.xml");
        } else {
            coprocs.add(coprocs.apple_gpu);
        }
    }
    coprocs.add_other_coproc_types();

    host_info.coprocs = coprocs;

    if (coprocs.none() ) {
        msg_printf(NULL, MSG_INFO, "No usable GPUs found");
    }

    set_no_rsc_config();

    // check for app_info.xml file in project dirs.
    // If find, read app info from there, set project.anonymous_platform
    // - this must follow coproc.get() (need to know if GPUs are present)
    // - this is being done before CPU speed has been read from state file,
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

    app_test_init();

    bool new_client = is_new_client();

    // this follows parse_state_file() since we need to have read
    // domain_name for Android
    //
    host_info.get_host_info(true);

    // clear the VM extensions disabled flag.
    // It's possible that the user enabled them since the last VM failure,
    // or that the last failure was specious.
    //
    host_info.p_vm_extensions_disabled = false;

    set_n_usable_cpus();
    show_host_info();

    // this follows parse_state_file() because that's where we read project names
    //
    sort_projects_by_name();

    // check for app_config.xml files in project dirs
    //
    check_app_config();
    show_app_config();

    // fill in resource usage for app versions that are missing it
    // (typically anonymous platform)
    //
    for (APP_VERSION* avp: app_versions) {
        if (!avp->resource_usage.avg_ncpus) {
            avp->resource_usage.avg_ncpus = 1;
        }
        if (!avp->resource_usage.flops) {
            avp->resource_usage.flops = avp->resource_usage.avg_ncpus * host_info.p_fpops;

            // for GPU apps, use conservative estimate:
            // assume GPU runs at 10X peak CPU speed
            //
            if (avp->resource_usage.rsc_type) {
                avp->resource_usage.flops += avp->resource_usage.coproc_usage * 10 * host_info.p_fpops;
            }
        }
    }

    // must go after check_app_config() and parse_state_file()
    // and after the above app version stuff
    //
    init_result_resource_usage();

    // this needs to go after parse_state_file() because
    // GPU exclusions refer to projects
    //
    cc_config.show();

    // inform the user if there's a newer version of client
    // NOTE: this must be called AFTER
    // read_nvc_config_file()
    //
    newer_version_startup_check();

#if !defined(SIM) && defined(_WIN32)
    show_wsl_messages();
#endif

    // parse account files again,
    // now that we know the host's venue on each project
    //
    parse_account_files_venue();

    // fill in p->no_X_apps for anon platform projects,
    // and check no_rsc_apps for others
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->anonymous_platform) {
            p->check_no_apps();
        } else {
            p->check_no_rsc_apps();
        }
    }

    process_gpu_exclusions();

    docker_cleanup();

    check_clock_reset();

    // Check to see if we can write the state file.
    //
    retval = write_state_file();
    if (retval) {
        msg_printf_notice(NULL, false,
            "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=statefile",
            _("Couldn't write state file; check directory permissions")
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

    // if new version of client,
    // - run CPU benchmarks
    // - get new project list
    // - contact reference site (or some project) to trigger firewall alert
    //
    if (new_client) {
        run_cpu_benchmarks = true;
        all_projects_list_check_time = 0;
        if (cc_config.dont_contact_ref_site) {
            if (projects.size() > 0) {
                projects[0]->master_url_fetch_pending = true;
            }
        } else {
            net_status.need_to_contact_reference_site = true;
        }
    }
    if (host_info.p_fpops == 0) {
        run_cpu_benchmarks = true;
    }

    check_if_need_benchmarks();

    read_global_prefs();

    // do CPU scheduler and work fetch
    //
    request_schedule_cpus("Startup");
    request_work_fetch("Startup");
    work_fetch.init();
    rec_interval_start = now;

    // set up the project and slot directories
    //
    msg_printf(NULL, MSG_INFO, "Setting up project and slot directories");
    delete_old_slot_dirs();
    retval = make_project_dirs();
    if (retval) return retval;

    msg_printf(NULL, MSG_INFO, "Checking active tasks");
    active_tasks.init();
    check_overdue();
    active_tasks.handle_upload_files();
    had_or_requested_work = (active_tasks.active_tasks.size() > 0);

    // Just to be on the safe side; something may have been modified
    //
    set_client_state_dirty("init");

    // check for initialization files
    //
    process_autologin(true);
    acct_mgr_info.init();
    project_init.init();

    // if project_init.xml specifies an account, attach
    //
    if (strlen(project_init.url) && strlen(project_init.account_key)) {
        add_project(
            project_init.url, project_init.account_key, project_init.name, "",
            false
        );
        project_init.remove();
    }

    log_show_projects();    // this must follow acct_mgr_info.init()

    // set up for handling GUI RPCs
    //
    if (!no_gui_rpc) {
        msg_printf(NULL, MSG_INFO, "Setting up GUI RPC socket");
        if (gui_rpc_unix_domain) {
            retval = gui_rpcs.init_unix_domain();
        } else {
            if(gui_rpc_websocket) {
                retval = gui_rpcs.init_websocket();
            } else {
                // When we're running at boot time,
                // it may be a few seconds before we can socket/bind/listen.
                // So retry a few times.
                //
                for (i=0; i<30; i++) {
                    bool last_time = (i==29);
                    retval = gui_rpcs.init_tcp(last_time);
                    if (!retval) break;
                    boinc_sleep(1.0);
                }
            }
        }
        if (retval) return retval;
    }

    if (g_use_sandbox) get_project_gid();
#ifdef _WIN32
    get_sandbox_account_service_token();
    if (sandbox_account_service_token != NULL) {
        g_use_sandbox = true;
    }
#endif

    msg_printf(NULL, MSG_INFO,
        "Checking presence of %d project files", (int)file_infos.size()
    );
    check_file_existence();
    if (!boinc_file_exists(ALL_PROJECTS_LIST_FILENAME)) {
        all_projects_list_check_time = 0;
    }

#ifdef ENABLE_AUTO_UPDATE
    auto_update.init();
#endif

    http_ops->cleanup_temp_files();

    // must parse env vars after parsing state file
    // otherwise items will get overwritten with state file info
    //
    parse_env_vars();

    // do this after parsing env vars
    //
    proxy_info_startup();

    if (!autologin_in_progress) {
        if (gstate.projects.size() == 0) {
            msg_printf(NULL, MSG_INFO,
                "This computer is not attached to any projects"
            );
        }
    }

    if(gui_rpc_unix_domain){
            msg_printf(NULL, MSG_INFO,
                "Client run in unix domain mode"
            );
    } else if(gui_rpc_websocket) {
            msg_printf(NULL, MSG_INFO,
                "Client run in websocket mode"
            );
    } else {
                    msg_printf(NULL, MSG_INFO,
                "Client run in tcp mode"
            );
    }

    // get list of BOINC projects occasionally,
    // and initialize notice RSS feeds
    //
    if (!cc_config.no_info_fetch) {
        all_projects_list_check();
        notices.init_rss();
    }

    // check for jobs with finish files
    // (i.e. they finished just as client was exiting)
    //
    active_tasks.check_for_finished_jobs();

    // warn user if some jobs need more memory than available
    //
    check_too_large_jobs();

    // initialize project priorities (for the GUI, in case we're suspended)
    //
    project_priority_init(false);

    client_thread_mutex.lock();
    throttle_thread.run(throttler, NULL);

    sporadic_init();

    // if Docker not present, notify user
    //
#ifndef ANDROID
#ifdef _WIN32
    const char* url = "https://github.com/BOINC/boinc/wiki/Installing-Docker-on-Windows";
#elif defined(__APPLE__)
    const char* url = "https://github.com/BOINC/boinc/wiki/Installing-Docker-on-Mac";
#else
    const char* url = "https://github.com/BOINC/boinc/wiki/Installing-Docker-on-Linux";
#endif
    if (!host_info.have_docker()) {
        msg_printf(NULL, MSG_INFO,
            "Some projects require Docker."
        );
        msg_printf(NULL, MSG_INFO,
            "To install Docker, visit %s", url
        );
        NOTICE n;
        n.description = "Some projects require Docker.  We recommend that you install it.  Instructions are <a href=" + (string)url + (string)">here</a>.";
        strcpy(n.link, url);
        n.create_time = now;
        n.arrival_time = now;
        strcpy(n.category, "client");
        notices.append(n);
    }
#endif

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
void CLIENT_STATE::do_io_or_sleep(double max_time) {
    int n;
    struct timeval tv;
    set_now();
    double end_time = now + max_time;
    double time_remaining = max_time;

    while (1) {
        curl_fds.zero();
        gui_rpc_fds.zero();
        http_ops->get_fdset(curl_fds);
        all_fds = curl_fds;
        if (!autologin_in_progress) {
            gui_rpcs.get_fdset(gui_rpc_fds, all_fds);
        }

        bool have_async = have_async_file_op();

        // prioritize network (including GUI RPC) over async file ops.
        // if there's a pending asynch file op, do the select with zero timeout;
        // otherwise do it for the remaining amount of time.

        double_to_timeval(have_async?0:time_remaining, tv);
        client_thread_mutex.unlock();

        if (all_fds.max_fd == -1) {
            boinc_sleep(time_remaining);
            n = 0;
        } else {
            n = select(
                all_fds.max_fd+1,
                &all_fds.read_fds, &all_fds.write_fds, &all_fds.exc_fds,
                &tv
            );
        }
        //printf("select in %d out %d\n", all_fds.max_fd, n);
        client_thread_mutex.lock();

        // Note: curl apparently likes to have curl_multi_perform()
        // (called from net_xfers->got_select())
        // called pretty often, even if no descriptors are enabled.
        // So do the "if (n==0) break" AFTER the got_selects().

        http_ops->got_select(all_fds, time_remaining);
        gui_rpcs.got_select(all_fds);

        if (have_async) {
            // do the async file op only if no network activity
            //
            if (n == 0) {
                do_async_file_op();
            }
        } else {
            if (n == 0) {
                break;
            }
        }

        set_now();
        if (now > end_time) break;
        time_remaining = end_time - now;
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
// Called every POLL_INTERVAL (1 sec)
//
bool CLIENT_STATE::poll_slow_events() {
    int actions = 0, retval;
    static int last_suspend_reason=0;
    static bool tasks_restarted = false;
    static bool first=true;
    double old_now = now;

    set_now();

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

    if (run_cpu_benchmarks && can_run_cpu_benchmarks()) {
        run_cpu_benchmarks = false;
        start_cpu_benchmarks();
    }

#ifdef _WIN32
    if (have_sysmon_msg) {
        msg_printf(NULL, MSG_INFO, sysmon_msg);
        have_sysmon_msg = false;
    }
#endif

    // there are 2 reasons to get idle state:
    // if needed for computing prefs,
    // or (on Mac) we were started by screensaver
    //
    bool get_idle_state = global_prefs.need_idle_state;
#ifdef __APPLE__
    if (started_by_screensaver) get_idle_state = true;
#endif
    long idle_time;
    if (get_idle_state) {
        bool old_user_active = user_active;
#ifdef ANDROID
        user_active = device_status.user_active;
#else
        idle_time = host_info.user_idle_time(check_all_logins);
        user_active = idle_time < global_prefs.idle_time_to_run * 60;
#endif
        if (user_active != old_user_active) {
            set_n_usable_cpus();
                // if niu_max_ncpus_pct pref is set, # usable CPUs may change
            request_schedule_cpus(user_active?"Not idle":"Idle");
        }
    } else {
        user_active = false;    // shouldn't matter what it is
    }

#if 0
    // NVIDIA provides an interface for finding if a GPU is
    // running a graphics app.  ATI doesn't as far as I know
    //
    if (host_info.have_nvidia() && user_active && !global_prefs.run_gpu_if_user_active) {
        if (host_info.coprocs.nvidia.check_running_graphics_app()) {
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
    if (started_by_screensaver && (idle_time < 30) && (getppid() == 1)) {
        // pid is 1 if parent has exited
        requested_exit = true;
    }

    // Exit if we were launched by Manager and it crashed.
    //
    if (launched_by_manager && (getppid() == 1)) {
        gstate.requested_exit = true;
    }
#endif

    // active_tasks.get_memory_usage() sets variables needed by
    // check_suspend_processing(), so it must be called first.
    //
    active_tasks.get_memory_usage();
    suspend_reason = check_suspend_processing();

    // suspend or resume activities (but only if already did startup)
    //
    if (tasks_restarted) {
        if (suspend_reason) {
            if (!tasks_suspended) {
                show_suspend_tasks_message(suspend_reason);
                active_tasks.suspend_all(suspend_reason);
            }
            last_suspend_reason = suspend_reason;
        } else {
            if (tasks_suspended && !tasks_throttled) {
                resume_tasks(last_suspend_reason);
            }
        }
    } else if (first) {
        // if suspended, show message the first time around
        //
        first = false;
        if (suspend_reason) {
            show_suspend_tasks_message(suspend_reason);
        }
    }
    tasks_suspended = (suspend_reason != 0);

    if (benchmarks_running) {
        cpu_benchmarks_poll();
    }

    int old_network_suspend_reason = network_suspend_reason;
    bool old_network_suspended = network_suspended;
    check_suspend_network();
    if (network_suspend_reason) {
        if (!old_network_suspend_reason) {
            char buf[256];
            if (network_suspended) {
                snprintf(buf, sizeof(buf),
                    "Suspending network activity - %s",
                    suspend_reason_string(network_suspend_reason)
                );
                request_schedule_cpus("network suspended");
                    // in case any "needs_network" jobs are running
            } else {
                snprintf(buf, sizeof(buf),
                    "Suspending file transfers - %s",
                    suspend_reason_string(network_suspend_reason)
                );
            }
            msg_printf(NULL, MSG_INFO, "%s", buf);
            pers_file_xfers->suspend();
        }
    } else {
        if (old_network_suspend_reason) {
            if (old_network_suspended) {
                msg_printf(NULL, MSG_INFO, "Resuming network activity");
            } else {
                msg_printf(NULL, MSG_INFO, "Resuming file transfers");
            }
            request_schedule_cpus("network resumed");
        }

        // if we're emerging from a bandwidth quota suspension,
        // add a random delay to avoid DDOS effect
        //
        if (
            old_network_suspend_reason == SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED
            && network_run_mode.get_current() == RUN_MODE_AUTO
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
    //  schedule_cpus
    // in that order (active_tasks_poll() sets must_schedule_cpus,
    // and handle_finished_apps() must be done before schedule_cpus()

    check_project_timeout();
#ifdef ENABLE_AUTO_UPDATE
    auto_update.poll();
#endif
    POLL_ACTION(active_tasks           , active_tasks.poll      );
    POLL_ACTION(garbage_collect        , garbage_collect        );
        // remove PERS_FILE_XFERs (and associated FILE_XFERs and HTTP_OPs)
        // for unreferenced files
    POLL_ACTION(gui_http               , gui_http.poll          );
    POLL_ACTION(gui_rpc_http           , gui_rpcs.poll          );
    POLL_ACTION(trickle_up_ops,        trickle_up_poll);
        // scan FILE_INFOS and create PERS_FILE_XFERs
        // for PERS_FILE_XFERS that are done, delete them

    if (!network_suspended && suspend_reason != SUSPEND_REASON_BENCHMARKS) {
        // don't initiate network activity if we're doing CPU benchmarks
        net_status.poll();
        daily_xfer_history.poll();
        POLL_ACTION(acct_mgr               , acct_mgr_info.poll     );
        POLL_ACTION(file_xfers             , file_xfers->poll       );
            // check for file xfer completion; don't delete anything
        POLL_ACTION(pers_file_xfers        , pers_file_xfers->poll  );
            // poll PERS_FILE_XFERS
            // if we need to start xfer, creat FILE_XFER and init
            // if FILE_XFER is complete
            //      handle transient and permanent failures
            //      delete the FILE_XFER

        if (!cc_config.no_info_fetch) {
            POLL_ACTION(rss_feed_op            , rss_feed_op.poll );
        }
    }
    POLL_ACTION(create_and_delete_pers_file_xfers ,
        create_and_delete_pers_file_xfers
    );
    POLL_ACTION(handle_finished_apps   , handle_finished_apps   );
    POLL_ACTION(update_results         , update_results         );
    if (!tasks_suspended) {
        POLL_ACTION(schedule_cpus, schedule_cpus          );
        tasks_restarted = true;
    }
    if (!network_suspended) {
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
    }
    if (have_sporadic_app) {
        sporadic_poll();
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
        time_stats.update(suspend_reason, gpu_suspend_reason);

        // on some systems, DNS resolution only starts working
        // a few minutes after system boot.
        // If it didn't work before, try it again.
        //
        if (!strlen(host_info.domain_name)) {
            host_info.get_local_network_info();
        }
        return false;
    }
}

#endif // ifndef SIM

// Find the project with the given master_url.
// Ignore differences in protocol, case, leading 'www.', and trailing /
// (the URL could come from an account manager,
// with differences from the real URL)
//
PROJECT* CLIENT_STATE::lookup_project(const char* master_url) {
    char buf[256];

    safe_strcpy(buf, master_url);
    canonicalize_master_url(buf, sizeof(buf));
    const char* p = strstr(buf, "//");
    if (!p) return NULL;
    p += 2;
    if (strcasestr(p, "www.") == p) p += 4;

    for (PROJECT *project: projects) {
        const char* q = strstr(project->master_url, "//");
        if (!q) continue;
        q += 2;
        if (strcasestr(q, "www.") == q) q += 4;
        if (!strcasecmp(p, q)) {
            // note: canonicalize_master_url() doesn't lower-case
            return project;
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
#ifndef SIM
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: duplicate app version: %s %s %d %s",
            avp->app_name, avp->platform, avp->version_num, avp->plan_class
        );
#endif
        return ERR_NOT_UNIQUE;
    }

#ifndef SIM

    safe_strcpy(avp->graphics_exec_path, "");
    safe_strcpy(avp->graphics_exec_file, "");

    for (unsigned int i=0; i<avp->app_files.size(); i++) {
        FILE_REF& file_ref = avp->app_files[i];
        FILE_INFO* fip = lookup_file_info(p, file_ref.file_name);
        if (!fip) {
            msg_printf(p, MSG_INTERNAL_ERROR,
                "State file error: missing application file %s",
                file_ref.file_name
            );
            return ERR_NOT_FOUND;
        }

        if (!strcmp(file_ref.open_name, GRAPHICS_APP_FILENAME)) {
            avp->graphics_exec_fip = fip;
        }

        // any file associated with an app version must be signed
        //
        if (!cc_config.unsigned_apps_ok) {
            fip->signature_required = true;
        }

        file_ref.file_info = fip;
    }
#endif
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
        const PERS_FILE_XFER* pers_file_xfer = pers_file_xfers->pers_file_xfers[i];
        msg_printf(0, MSG_INFO, "    %s http op state: %d", pers_file_xfer->fip->name, pers_file_xfer->fxp?pers_file_xfer->fxp->http_op_state:-1);
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
        if (!rp->is_not_started()) continue;
        if (rp->report_deadline > now) continue;
        msg_printf(rp->project, MSG_INFO,
            "Aborting task %s; not started and deadline has passed",
            rp->name
        );
        rp->abort_inactive(EXIT_UNSTARTED_LATE);
        action = true;
    }
    return action;
}

bool CLIENT_STATE::garbage_collect() {
    bool action;
    static double last_time=0;
    if (!clock_change && now - last_time < GARBAGE_COLLECT_PERIOD) return false;
    last_time = gstate.now;

    action = abort_unstarted_late_jobs();
    if (action) return true;
    action = garbage_collect_always();
    if (action) return true;

#ifndef SIM
    // Detach projects that are marked for detach when done
    // and are in fact done (have no results).
    // This is done here (not in garbage_collect_always())
    // because detach_project() calls garbage_collect_always(),
    // and we need to avoid infinite recursion
    //
    while (1) {
        bool found = false;
        for (unsigned i=0; i<projects.size(); i++) {
            PROJECT* p = projects[i];
            if (p->detach_when_done && !nresults_for_project(p)) {
                // If we're using an AM,
                // wait until the next successful RPC to detach project,
                // so the AM will be informed of its work done.
                //
                if (!p->attached_via_acct_mgr) {
                    msg_printf(p, MSG_INFO, "Detaching - no more tasks");
                    detach_project(p);
                    action = true;
                    found = true;
                }
            }
        }
        if (!found) break;
    }
#endif
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

#ifdef ENABLE_AUTO_UPDATE
    // reference-count auto update files
    //
    if (auto_update.present) {
        for (i=0; i<auto_update.file_refs.size(); i++) {
            auto_update.file_refs[i].file_info->ref_cnt++;
        }
    }
#endif

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
#ifndef SIM
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
                    "Got ack for job that's still active"
                );
            } else {
                if (log_flags.state_debug) {
                    msg_printf(0, MSG_INFO,
                        "[state] garbage_collect: deleting result %s\n",
                        rp->name
                    );
                }
                add_old_result(*rp);
                delete rp;
                result_iter = results.erase(result_iter);
                action = true;
                continue;
            }
        }
#endif
        // See if the files for this result's workunit had
        // any errors (download failure, MD5, RSA, etc)
        // and we don't already have an error for this result
        //
        if (!rp->ready_to_report) {
            wup = rp->wup;
            if (wup->had_download_failure(failnum)) {
                wup->get_file_errors(error_msgs);
                string err_msg = "WU download error: " + error_msgs;
                report_result_error(*rp, err_msg.c_str());
            } else if (rp->avp && rp->avp->had_download_failure(failnum)) {
                rp->avp->get_file_errors(error_msgs);
                string err_msg = "app_version download error: " + error_msgs;
                report_result_error(*rp, err_msg.c_str());
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
#ifdef SIM
        (void)found_error;
#else
        if (found_error) {
            // check for process still running; this can happen
            // e.g. if an intermediate upload fails
            //
            ACTIVE_TASK* atp = active_tasks.lookup_result(rp);
            if (atp) {
                switch (atp->task_state()) {
                case PROCESS_EXECUTING:
                case PROCESS_SUSPENDED:
                    atp->abort_task(ERR_RESULT_UPLOAD, "upload failure");
                }
            }
            string err_msg = "upload failure: " + error_str;
            report_result_error(*rp, err_msg.c_str());
        }
#endif
        rp->avp->ref_cnt++;
        rp->wup->ref_cnt++;
        ++result_iter;
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
            ++wu_iter;
        }
    }

    // go through APP_VERSIONs;
    // delete any not referenced by any WORKUNIT
    // and superceded by a more recent version
    // for the same platform and plan class
    //
    avp_iter = app_versions.begin();
    while (avp_iter != app_versions.end()) {
        avp = *avp_iter;
        if (avp->ref_cnt == 0) {
            found = false;
            for (j=0; j<app_versions.size(); j++) {
                avp2 = app_versions[j];
                if (avp2->app == avp->app
                    && avp2->version_num > avp->version_num
                    && (!strcmp(avp2->plan_class, avp->plan_class))
                    && (!strcmp(avp2->platform, avp->platform))
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
                ++avp_iter;
            }
        } else {
            ++avp_iter;
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

    for (fi_iter = file_infos.begin(); fi_iter!=file_infos.end(); ++fi_iter) {
        fip = *fi_iter;
        if (fip->sticky_expire_time && now > fip->sticky_expire_time) {
            fip->sticky = false;
            fip->sticky_expire_time = 0;
        }
        if (!fip->sticky) continue;
        if (fip->status < 0) continue;
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
            ++pfx_iter;
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
            ++fi_iter;
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

    if (!clock_change && now - last_time < UPDATE_RESULTS_PERIOD) return false;
    last_time = now;

    result_iter = results.begin();
    while (result_iter != results.end()) {
        rp = *result_iter;

        switch (rp->state()) {
        case RESULT_NEW:
            rp->set_state(RESULT_FILES_DOWNLOADING, "CS::update_results");
            action = true;
            break;
#ifndef SIM
        case RESULT_FILES_DOWNLOADING:
            if (task_files_present(rp, false) == 0) {
                if (rp->avp->app_files.size()==0) {
                    // if this is a file-transfer app, start the upload phase
                    //
                    rp->set_state(RESULT_FILES_UPLOADING, "CS::update_results");
                    rp->clear_uploaded_flags();
                } else {
                    // else try to start the computation
                    //
                    rp->set_state(RESULT_FILES_DOWNLOADED, "CS::update_results");
                    request_schedule_cpus("files downloaded");
                }
                action = true;
            }
            break;
#endif
        case RESULT_FILES_UPLOADING:
            if (rp->is_upload_done()) {
                rp->set_ready_to_report();
                rp->completed_time = gstate.now;
                rp->set_state(RESULT_FILES_UPLOADED, "CS::update_results");

                // clear backoffs for app's resources;
                // this addresses the situation where the project has a
                // "max # jobs in progress" limit,
                // and we're backed off because of that
                //
                work_fetch.clear_backoffs(*rp->avp);
                action = true;
            }
            break;
        case RESULT_FILES_UPLOADED:
            break;
        case RESULT_ABORTED:
            if (!rp->ready_to_report) {
                rp->set_ready_to_report();
                rp->completed_time = now;
                action = true;
            }
            break;
        }
        ++result_iter;
    }
    return action;
}

// Returns true if client should exit for various reasons
//
bool CLIENT_STATE::time_to_exit() {
    if (exit_after_app_start_secs
        && (app_started>0)
        && ((now - app_started) >= exit_after_app_start_secs)
    ) {
        msg_printf(NULL, MSG_INFO,
            "Exiting because %d elapsed since started task",
            exit_after_app_start_secs
        );
        return true;
    }
    if (cc_config.exit_when_idle
        && (results.size() == 0)
        && had_or_requested_work
    ) {
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
int CLIENT_STATE::report_result_error(RESULT& res, const char* err_msg) {
    char buf[1024];
    unsigned int i;
    int failnum;

    // only do this once per result
    //
    if (res.ready_to_report) {
        return 0;
    }

    res.set_ready_to_report();
    res.completed_time = now;

    snprintf(buf, sizeof(buf), "Unrecoverable error for task %s", res.name);
#ifndef SIM
    scheduler_op->project_rpc_backoff(res.project, buf);
#endif

    res.stderr_out.append("<message>\n");
    res.stderr_out.append(err_msg);
    res.stderr_out.append("</message>\n");

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
        // ACTIVE_TASK_SET::restart_tasks (catch other error returns)
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
                snprintf(buf, sizeof(buf),
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
        res.set_state(RESULT_UPLOAD_FAILED, "CS::report_result_error");
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

#ifndef SIM

// "Reset" a project: (clear error conditions)
// - stop all active tasks
// - stop all file transfers
// - stop scheduler RPC if any
// - delete workunits and results
// - delete apps and app_versions
// - garbage collect to delete unneeded files
// - clear backoffs
//
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

    // stop and remove file transfers
    //
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

    // abort other HTTP operations
    //
    //http_ops.abort_project_ops(project);

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

    // clear flags so that sticky files get deleted
    //
    for (i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->project == project) {
            fip->sticky = false;
        }
    }

    garbage_collect_always();

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
                ++avp_iter;
            }
        }

        app_iter = apps.begin();
        while (app_iter != apps.end()) {
            app = *app_iter;
            if (app->project == project) {
                app_iter = apps.erase(app_iter);
                delete app;
            } else {
                ++app_iter;
            }
        }
        garbage_collect_always();
    }

    // if not anonymous platform, clean out the project dir
    // except for app_config.xml
    //
    if (!project->anonymous_platform) {
        client_clean_out_dir(
            project->project_dir(),
            "reset project",
            "app_config.xml"
        );
    }

    // force refresh of scheduler URLs
    //
    project->scheduler_urls.clear();

    project->duration_correction_factor = 1;
    project->ams_resource_share = -1;
    project->min_rpc_time = 0;
    project->pwf.reset(project);
    for (int j=0; j<coprocs.n_rsc; j++) {
        project->rsc_pwf[j].reset(j);
    }
    write_state_file();
    return 0;
}

// "Detach" a project:
// - Reset (see above)
// - delete all file infos
// - delete account file
// - delete project directory
// - delete various per-project files
//
int CLIENT_STATE::detach_project(PROJECT* project) {
    vector<PROJECT*>::iterator project_iter;
    vector<FILE_INFO*>::iterator fi_iter;
    FILE_INFO* fip;
    PROJECT* p;
    char path[MAXPATHLEN];
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
            ++fi_iter;
        }
    }

    // find project and remove it from the vector
    //
    for (project_iter = projects.begin(); project_iter != projects.end(); ++project_iter) {
        p = *project_iter;
        if (p == project) {
            project_iter = projects.erase(project_iter);
            break;
        }
    }

    // delete statistics file
    //
    get_statistics_filename(project->master_url, path, sizeof(path));
    retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "Can't delete statistics file: %s", boincerror(retval)
        );
    }

    // delete account file
    //
    get_account_filename(project->master_url, path, sizeof(path));
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

    // remove miscellaneous per-project files
    //
    //job_log_filename(*project, path, sizeof(path));
    //boinc_delete_file(path);
    delete_project_notice_files(project);

    rss_feeds.update_feed_list();

    delete project;
    write_state_file();

    adjust_rec();
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
    // calculate REC (for state file)
    //
    adjust_rec();

    daily_xfer_history.write_file();
    write_state_file();
    gui_rpcs.close();
    abort_cpu_benchmarks();
    time_stats.quit();

    // stop jobs.
    // Do this last because it could take a long time,
    // and the OS might kill us in the middle
    //
    int retval = active_tasks.exit_tasks();
    if (retval) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Couldn't exit tasks: %s", boincerror(retval)
        );
    }

    return 0;
}

#endif

// Called at startup to see if a timestamp in the client state file
// is later than the current time.
// If so, the user must have decremented the system clock.
//
void CLIENT_STATE::check_clock_reset() {
    if (!time_stats.last_update) return;
    if (time_stats.last_update <= now) return;
    msg_printf(NULL, MSG_INFO,
        "System clock (%.0f) < state file timestamp (%.0f); clearing timeouts",
        now, time_stats.last_update
    );
    clear_absolute_times();
}

// The system clock seems to have been set back,
// possibly by a large amount (years).
// Clear various "wait until X" absolute times.
//
// Note: there are other absolute times (like job deadlines)
// that we could try to patch up, but it's not clear how.
//
void CLIENT_STATE::clear_absolute_times() {
    exclusive_app_running = 0;
    exclusive_gpu_app_running = 0;
    new_version_check_time = now;
    all_projects_list_check_time = now;
    retry_shmem_time = 0;
    cpu_run_mode.temp_timeout = 0;
    gpu_run_mode.temp_timeout = 0;
    network_run_mode.temp_timeout = 0;
    time_stats.last_update = now;

    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->min_rpc_time = 0;
        if (p->next_rpc_time) {
            p->next_rpc_time = now;
        }
        p->download_backoff.next_xfer_time = 0;
        p->upload_backoff.next_xfer_time = 0;
        for (int j=0; j<coprocs.n_rsc; j++) {
            p->rsc_pwf[j].clear_backoff();
        }
//#ifdef USE_REC
        p->pwf.rec_time = now;
//#endif
    }
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        PERS_FILE_XFER* pfx = pers_file_xfers->pers_file_xfers[i];
        pfx->next_request_time = 0;
    }

    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        rp->schedule_backoff = 0;
    }
}

void CLIENT_STATE::log_show_projects() {
    char buf[256];
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->hostid) {
            snprintf(buf, sizeof(buf), "%d", p->hostid);
        } else {
            safe_strcpy(buf, "not assigned yet");
        }
        msg_printf(p, MSG_INFO,
            "URL %s; Computer ID %s; resource share %.0f",
            p->master_url, buf, p->resource_share
        );
        if (p->ended) {
            msg_printf(p, MSG_INFO, "Project has ended - OK to detach");
        }
        p->show_no_work_notice();
    }
}

#ifndef SIM

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
            atp->abort_task(EXIT_CLIENT_EXITING, "aborting on client exit");
        } else {
            rp->abort_inactive(EXIT_CLIENT_EXITING);
        }
    }
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->min_rpc_time = 0;
        p->dont_request_more_work = true;
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

#endif  // !SIM

// for each result, copy resource usage either from
// - workunit if present there (e.g. BUDA jobs)
// - app version otherwise
//
// call this on startup and after reread app_config.xml
// (which can change app version resource usage)
//
void CLIENT_STATE::init_result_resource_usage() {
    for (RESULT* rp: results) {
        rp->init_resource_usage();
        if (rp->resource_usage.missing_coproc) {
            msg_printf(rp->project, MSG_INFO,
                "Missing coprocessor for task %s", rp->name
            );
        }
    }
}
