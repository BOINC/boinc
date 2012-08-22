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

// command-line parsing, and handling of 1-time actions

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#ifdef _MSC_VER
#define chdir _chdir
#endif
#else
#include "config.h"
#include <cstdio>
#include <unistd.h>
#endif

#include "str_util.h"
#include "url.h"
#include "str_replace.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "cs_proxy.h"
#include "main.h"
#include "project.h"
#include "sandbox.h"

static void print_options(char* prog) {
    printf(
        "The command-line options for %s are intended for debugging.\n"
        "The recommended command-line interface is a separate program,'boinccmd'.\n"
        "Run boinccmd in the same directory as %s.\n"
        "\n"
        "Usage: %s [options]\n"
        "    --abort_jobs_on_exit           when client exits, abort and report jobs\n"
        "    --allow_remote_gui_rpc         allow remote GUI RPC connections\n"
        "    --allow_multiple_clients       allow >1 instances per host\n"
        "    --attach_project <URL> <key>   attach to a project\n"
        "    --check_all_logins             for idle detection, check remote logins too\n"
        "    --daemon                       run as daemon (Unix)\n"
        "    --detach_console               detach from console (Windows)\n"
        "    --detach_project <URL>         detach from a project\n"
        "    --dir <path>                   use given dir as BOINC home\n"
        "    --exit_after_app_start N       exit N seconds after an app starts\n"
        "    --exit_after_finish            exit right after finishing a job\n"
        "    --exit_before_start            exit right before starting a job\n"
        "    --exit_before_upload           exit right before starting an upload \n"
        "    --exit_when_idle               exit when there are no results\n"
        "    --fetch_minimal_work           fetch only 1 job per device\n"
        "    --file_xfer_giveup_period N    give up on file xfers after N sec\n"
        "    --gui_rpc_port <port>          port for GUI RPCs\n"
        "    --help                         show options\n"
#ifdef SANDBOX
        "    --insecure                     disable app sandboxing (Unix)\n"
#endif
        "    --launched_by_manager          client was launched by Manager\n"
        "    --master_fetch_interval N      limiting period of master retry\n"
        "    --master_fetch_period N        reload master URL after N RPC failures\n"
        "    --master_fetch_retry_cap N     exponential backoff limit\n"
        "    --no_gpus                      don't check for GPUs\n"
        "    --no_gui_rpc                   don't allow GUI RPC, don't make socket\n"
        "    --no_info_fetch                don't fetch project list or client version info\n"
        "    --no_priority_change           run apps at same priority as client\n"
        "    --pers_giveup N                giveup time for persistent file xfer\n"
        "    --pers_retry_delay_max N       max for file xfer exponential backoff\n"
        "    --pers_retry_delay_min N       min for file xfer exponential backoff\n"
        "    --redirectio                   redirect stdout and stderr to log files\n"
        "    --reset_project <URL>          reset (clear) a project\n"
        "    --retry_cap N                  exponential backoff limit\n"
        "    --run_cpu_benchmarks           run the CPU benchmarks\n"
        "    --run_by_updater               set by updater\n"
        "    --saver                        client was launched by screensaver\n"
        "    --sched_retry_delay_max N      max for RPC exponential backoff\n"
        "    --sched_retry_delay_min N      min for RPC exponential backoff\n"
        "    --show_projects                show attached projects\n"
        "    --skip_cpu_benchmarks          don't run CPU benchmarks\n"
        "    --start_delay X                delay starting apps for X secs\n"
        "    --unsigned_apps_ok             allow unsigned apps (for testing)\n"
        "    --update_prefs <URL>           contact a project to update preferences\n"
        "    --version                      show version info\n"
        ,
        prog, prog, prog
    );
}

// Parse the command line arguments passed to the client
// NOTE: init() has not been called at this point
// (i.e. client_state.xml has not been parsed)
// So just record the args here.
// The actions get done in do_cmdline_actions()
//
// Check for both -X (deprecated) and --X
//
#define ARGX2(s1,s2) (!strcmp(argv[i], s1)||!strcmp(argv[i], s2))
#define ARG(S) ARGX2("-"#S, "--"#S)

void CLIENT_STATE::parse_cmdline(int argc, char** argv) {
    int i;
    bool show_options = false;

    // NOTE: if you change or add anything, make the same chane
    // in show_options() (above) and in doc/client.php

    for (i=1; i<argc; i++) {
        if (0) {
        } else if (ARG(abort_jobs_on_exit)) {
            config.abort_jobs_on_exit = true;
        } else if (ARG(allow_multiple_clients)) {
            config.allow_multiple_clients = true;
        } else if (ARG(allow_remote_gui_rpc)) {
            config.allow_remote_gui_rpc = true;
        } else if (ARG(attach_project)) {
            if (i >= argc-2) {
                show_options = true;
            } else {
                safe_strcpy(attach_project_url, argv[++i]);
                safe_strcpy(attach_project_auth, argv[++i]);
            }
        } else if (ARG(check_all_logins)) {
            check_all_logins = true;
        } else if (ARG(daemon)) {
            executing_as_daemon = true;
        } else if (ARG(detach)) {
#ifdef _WIN32
            FreeConsole();
#endif
            detach_console = true;
        } else if (ARG(detach_project)) {
            if (i == argc-1) show_options = true;
            else safe_strcpy(detach_project_url, argv[++i]);
        } else if (ARG(dir)) {
            if (i == argc-1) {
                show_options = true;
            } else {
                if (chdir(argv[++i])) {
                    perror("chdir");
                    exit(1);
                }
            }
        } else if (ARG(exit_after_app_start)) {
            if (i == argc-1) show_options = true;
            else exit_after_app_start_secs = atoi(argv[++i]);
        } else if (ARG(exit_after_finish)) {
            config.exit_after_finish = true;
        } else if (ARG(exit_before_start)) {
            config.exit_before_start = true;
        } else if (ARG(exit_before_upload)) {
            exit_before_upload = true;
        } else if (ARG(exit_when_idle)) {
            config.exit_when_idle = true;
            config.report_results_immediately = true;
        } else if (ARG(fetch_minimal_work)) {
            config.fetch_minimal_work = true;
        } else if (ARG(file_xfer_giveup_period)) {
            if (i == argc-1) show_options = true;
            else file_xfer_giveup_period = atoi(argv[++i]);
        } else if (ARG(gui_rpc_port)) {
            if (i == argc-1) show_options = true;
            else cmdline_gui_rpc_port = atoi(argv[++i]);
        } else if (ARG(help)) {
            print_options(argv[0]);
            exit(0);
        } else if (ARG(insecure)) {
#ifdef SANDBOX
            g_use_sandbox = false;
#endif
        } else if (ARG(launched_by_manager)) {
            launched_by_manager = true;
        } else if (ARG(master_fetch_interval)) {
            if (i == argc-1) show_options = true;
            else master_fetch_interval = atoi(argv[++i]);
        } else if (ARG(master_fetch_period)) {
            if (i == argc-1) show_options = true;
            else master_fetch_period = atoi(argv[++i]);
        } else if (ARG(master_fetch_retry_cap)) {
            if (i == argc-1) show_options = true;
            else master_fetch_retry_cap = atoi(argv[++i]);
        } else if (ARG(no_gpus)) {
            config.no_gpus = true;
        } else if (ARG(no_gui_rpc)) {
            no_gui_rpc = true;
        } else if (ARG(no_info_fetch)) {
            config.no_info_fetch = true;
        } else if (ARG(no_priority_change)) {
            config.no_priority_change = true;
        } else if (ARG(pers_giveup)) {
            if (i == argc-1) show_options = true;
            else pers_giveup = atoi(argv[++i]);
        } else if (ARG(pers_retry_delay_max)) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_max = atoi(argv[++i]);
        } else if (ARG(pers_retry_delay_min)) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_min = atoi(argv[++i]);
        } else if (!strncmp(argv[i], "-psn_", strlen("-psn_"))) {
            // ignore -psn argument on Mac OS X
        } else if (ARG(redirectio)) {
            redirect_io = true;
        } else if (ARG(reset_project)) {
            if (i == argc-1) show_options = true;
            else safe_strcpy(reset_project_url, argv[++i]);
        } else if (ARG(retry_cap)) {
            if (i == argc-1) show_options = true;
            else retry_cap = atoi(argv[++i]);
        } else if (ARG(run_by_updater)) {
            run_by_updater = true;
        } else if (ARG(run_cpu_benchmarks)) {
            run_cpu_benchmarks = true;
        } else if (ARG(saver)) {
            started_by_screensaver = true;
        } else if (ARG(sched_retry_delay_max)) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_max = atoi(argv[++i]);
        } else if (ARG(sched_retry_delay_min)) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_min = atoi(argv[++i]);
        } else if (ARG(show_projects)) {
            show_projects = true;
        } else if (ARG(skip_cpu_benchmarks)) {
            config.skip_cpu_benchmarks = true;
        } else if (ARG(start_delay)) {
            if (i == argc-1) show_options = true;
            else config.start_delay = atof(argv[++i]);
        } else if (ARG(unsigned_apps_ok)) {
            config.unsigned_apps_ok = true;
        } else if (ARG(update_prefs)) {
            if (i == argc-1) show_options = true;
            else safe_strcpy(update_prefs_url, argv[++i]);
        } else if (ARG(version)) {
#ifdef __APPLE__
            CLIENT_STATE cs;
            cs.detect_platforms();
            printf(BOINC_VERSION_STRING " %s\n", cs.get_primary_platform());
#else
            printf(BOINC_VERSION_STRING " " HOSTTYPE "\n");
#endif
            exit(0);
#ifdef __APPLE__
        // workaround for bug in XCode 4.2: accept but ignore 
        // argument -NSDocumentRevisionsDebugMode=YES 
        } else if (ARG(NSDocumentRevisionsDebugMode)) {
            ++i; 
#endif
        } else {
            printf("Unknown option: %s\n", argv[i]);
            show_options = true;
        }
    }
    if (show_options) {
        print_options(argv[0]);
        exit(1);
    }
}

#undef ARG
#undef ARGX2

void CLIENT_STATE::parse_env_vars() {
    char *p;
    PARSED_URL purl;

    p = getenv("HTTP_PROXY");
    if (p && strlen(p) > 0) {
        parse_url(p, purl);
        switch (purl.protocol) {
        case URL_PROTOCOL_HTTP:
        case URL_PROTOCOL_HTTPS:
            env_var_proxy_info.present = true;
            env_var_proxy_info.use_http_proxy = true;
            strcpy(env_var_proxy_info.http_user_name, purl.user);
            strcpy(env_var_proxy_info.http_user_passwd, purl.passwd);
            strcpy(env_var_proxy_info.http_server_name, purl.host);
            env_var_proxy_info.http_server_port = purl.port;
            break;
        default:
            msg_printf_notice(0, false,
                "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=proxy_env",
                _("The HTTP_PROXY environment variable must specify an HTTP proxy")
            );
        }
    }
    p = getenv("HTTP_USER_NAME");
    if (p) {
        env_var_proxy_info.use_http_auth = true;
        strcpy(env_var_proxy_info.http_user_name, p);
        p = getenv("HTTP_USER_PASSWD");
        if (p) {
            strcpy(env_var_proxy_info.http_user_passwd, p);
        }
    }

    p = getenv("SOCKS_SERVER");
    if (!p) p = getenv("SOCKS5_SERVER");
    if (p && strlen(p)) {
        parse_url(p, purl);
        env_var_proxy_info.present = true;
        env_var_proxy_info.use_socks_proxy = true;
        strcpy(env_var_proxy_info.socks5_user_name, purl.user);
        strcpy(env_var_proxy_info.socks5_user_passwd, purl.passwd);
        strcpy(env_var_proxy_info.socks_server_name, purl.host);
        env_var_proxy_info.socks_server_port = purl.port;
    }

    p = getenv("SOCKS5_USER");
    if (!p) p = getenv("SOCKS_USER");
    if (p) {
        strcpy(env_var_proxy_info.socks5_user_name, p);
    }

    p = getenv("SOCKS5_PASSWD");
    if (p) {
        strcpy(env_var_proxy_info.socks5_user_passwd, p);
    }
}

void CLIENT_STATE::do_cmdline_actions() {
    unsigned int i;

    if (show_projects) {
        printf("projects:\n");
        for (i=0; i<projects.size(); i++) {
            msg_printf(NULL, MSG_INFO, "URL: %s name: %s\n",
                projects[i]->master_url, projects[i]->project_name
            );
        }
        exit(0);
    }

    if (strlen(detach_project_url)) {
        canonicalize_master_url(detach_project_url);
        PROJECT* project = lookup_project(detach_project_url);
        if (project) {
            // do this before detaching - it frees the project
            //
            msg_printf(project, MSG_INFO, "detaching from %s\n", detach_project_url);
            detach_project(project);
        } else {
            msg_printf(NULL, MSG_INFO, "project %s not found\n", detach_project_url);
        }
        exit(0);
    }

    if (strlen(reset_project_url)) {
        canonicalize_master_url(reset_project_url);
        PROJECT* project = lookup_project(reset_project_url);
        if (project) {
            reset_project(project, false);
            msg_printf(project, MSG_INFO, "Project %s has been reset", reset_project_url);
        } else {
            msg_printf(NULL, MSG_INFO, "project %s not found\n", reset_project_url);
        }
        exit(0);
    }

    if (strlen(update_prefs_url)) {
        canonicalize_master_url(update_prefs_url);
        PROJECT* project = lookup_project(update_prefs_url);
        if (project) {
            project->sched_rpc_pending = RPC_REASON_USER_REQ;
        } else {
            msg_printf(NULL, MSG_INFO, "project %s not found\n", update_prefs_url);
        }
    }

    if (strlen(attach_project_url)) {
        canonicalize_master_url(attach_project_url);
        add_project(attach_project_url, attach_project_auth, "", false);
    }
}

