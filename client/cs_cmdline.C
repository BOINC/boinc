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

// command-line parsing, and handling of 1-time actions

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#endif

#include "main.h"
#include "util.h"
#include "client_msgs.h"
#include "client_state.h"

static void print_options(char* prog) {
    printf(
        "Usage: %s [options]\n"
        "    -version               show version info\n"
        "    -exit_when_idle        Get/process/report work, then exit\n"
        "    -show_projects         show attached projects\n"
        "    -return_results_immediately   contact server when have results\n"
        "    -detach_project URL    detach from a project\n"
        "    -reset_project URL     reset (clear) a project\n"
        "    -attach_project        attach to a project (will prompt for URL, account key)\n"
        "    -update_prefs URL      contact a project to update preferences\n"
        "    -run_cpu_benchmarks    run the CPU benchmarks\n"
        "    -check_all_logins      check input from remote users\n"
        "    -allow_remote_gui_rpc  allow remote GUI RPC connections\n"
        "    -redirectio            redirect stdout and stderr to log files\n",
        prog
    );
}

// Parse the command line arguments passed to the client
// NOTE: init() has not been called at this point
// (i.e. client_state.xml has not been parsed)
//
#define ARGX2(s1,s2) (!strcmp(argv[i], s1)||!strcmp(argv[i], s2))
#define ARG(S) ARGX2("-"#S, "--"#S)

void CLIENT_STATE::parse_cmdline(int argc, char** argv) {
    int i;
    bool show_options = false;

    for (i=1; i<argc; i++) {
        if (ARG(exit_when_idle)) {
            exit_when_idle = true;
        } else if (ARG(check_all_logins)) {
            check_all_logins = true;
        } else if (ARG(daemon)) {
            executing_as_daemon = true;
        } else if (ARG(return_results_immediately)) {
            return_results_immediately = true;
        } else if (ARG(skip_cpu_benchmarks)) {
            skip_cpu_benchmarks = true;
        } else if (ARG(exit_after_app_start)) {
            if (i == argc-1) show_options = true;
            else exit_after_app_start_secs = atoi(argv[++i]);
        } else if (ARG(file_xfer_giveup_period)) {
            if (i == argc-1) show_options = true;
            else file_xfer_giveup_period = atoi(argv[++i]);
        } else if (ARG(min)) {
            global_prefs.run_minimized = true;
        } else if (ARG(suspend)) {
            user_run_request = USER_RUN_REQUEST_NEVER;
        } else if (ARG(saver)) {
            started_by_screensaver = true;
        } else if (!strncmp(argv[i], "-psn_", strlen("-psn_"))) {
            // ignore -psn argument on Mac OS X
        } else if (ARG(exit_before_upload)) {
            exit_before_upload = true;
        // The following are only used for testing to alter scheduler/file transfer
        // backoff rates
        } else if (ARG(master_fetch_period)) {
            if (i == argc-1) show_options = true;
            else master_fetch_period = atoi(argv[++i]);
        } else if (ARG(retry_base_period)) {
            if (i == argc-1) show_options = true;
            else retry_base_period = atoi(argv[++i]);
        } else if (ARG(retry_cap)) {
            if (i == argc-1) show_options = true;
            else retry_cap = atoi(argv[++i]);
        } else if (ARG(master_fetch_retry_cap)) {
            if (i == argc-1) show_options = true;
            else master_fetch_retry_cap = atoi(argv[++i]);
        } else if (ARG(master_fetch_interval)) {
            if (i == argc-1) show_options = true;
            else master_fetch_interval = atoi(argv[++i]);
        } else if (ARG(sched_retry_delay_min)) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_min = atoi(argv[++i]);
        } else if (ARG(sched_retry_delay_max)) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_max = atoi(argv[++i]);
        } else if (ARG(pers_retry_delay_min)) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_min = atoi(argv[++i]);
        } else if (ARG(pers_retry_delay_max)) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_max = atoi(argv[++i]);
        } else if (ARG(pers_giveup)) {
            if (i == argc-1) show_options = true;
            else pers_giveup = atoi(argv[++i]);
        } else if (ARG(debug_fake_exponential_backoff)) {
            debug_fake_exponential_backoff = true;

        // the above options are private (i.e. not shown by -help)
        // Public options follow.
        // NOTE: if you change or add anything, make the same chane
        // in show_options() (above) and in doc/client.php

        } else if (ARG(show_projects)) {
            show_projects = true;
        } else if (ARG(detach_project)) {
            if (i == argc-1) show_options = true;
            else strcpy(detach_project_url, argv[++i]);
        } else if (ARG(reset_project)) {
            if (i == argc-1) show_options = true;
            else strcpy(reset_project_url, argv[++i]);
        } else if (ARG(update_prefs)) {
            if (i == argc-1) show_options = true;
            else strcpy(update_prefs_url, argv[++i]);
        } else if (ARG(run_cpu_benchmarks)) {
            run_cpu_benchmarks = true;
        } else if (ARG(attach_project)) {
            add_new_project();
        } else if (ARG(version)) {
            printf(BOINC_VERSION_STRING " " HOSTTYPE "\n");
            exit(0);
        } else if (ARG(allow_remote_gui_rpc)) {
            allow_remote_gui_rpc = true;
        } else if (ARG(redirectio)) {
            redirect_io = true;
        } else if (ARG(help)) {
            print_options(argv[0]);
            exit(0);
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
    char *p, temp[256];

    p = getenv("HTTP_PROXY");
    if (p && strlen(p) > 0) {
        proxy_info.use_http_proxy = true;
        parse_url(p, proxy_info.http_server_name, proxy_info.http_server_port, temp);
    }
    p = getenv("HTTP_USER_NAME");
    if (p) {
        proxy_info.use_http_auth = true;
        strcpy(proxy_info.http_user_name, p);
        p = getenv("HTTP_USER_PASSWD");
        if (p) {
            strcpy(proxy_info.http_user_passwd, p);
        }
    }

    proxy_info.socks_version =
        getenv("SOCKS5_SERVER")?SOCKS_VERSION_5:
        getenv("SOCKS4_SERVER")?SOCKS_VERSION_4:
        getenv("SOCKS_SERVER")?SOCKS_VERSION_5:
        SOCKS_VERSION_5;

    p = getenv("SOCKS4_SERVER");
    if (p && strlen(p) > 0) {
        proxy_info.use_socks_proxy = true;
        parse_url(p, proxy_info.socks_server_name, proxy_info.socks_server_port, temp);
    }

    if ((p = getenv("SOCKS_SERVER")) || (p = getenv("SOCKS5_SERVER"))) {
        if (strlen(p) > 0) {
            proxy_info.use_socks_proxy = true;
            parse_url(p, proxy_info.socks_server_name, proxy_info.socks_server_port, temp);
        }
    }

    if ((p = getenv("SOCKS5_USER")) || (p = getenv("SOCKS_USER"))) {
        safe_strcpy(proxy_info.socks5_user_name, p);
    }

    if ((p = getenv("SOCKS5_PASSWD"))) {
        safe_strcpy(proxy_info.socks5_user_passwd, p);
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
        PROJECT* project = lookup_project(detach_project_url);
        if (project) {
            detach_project(project);
            msg_printf(project, MSG_INFO, "detached from %s\n", detach_project_url);
        } else {
            msg_printf(NULL, MSG_ERROR, "project %s not found\n", detach_project_url);
        }
        exit(0);
    }

    if (strlen(reset_project_url)) {
        PROJECT* project = lookup_project(reset_project_url);
        if (project) {
            reset_project(project);
            msg_printf(project, MSG_INFO, "Project %s has been reset", reset_project_url);
        } else {
            msg_printf(NULL, MSG_ERROR, "project %s not found\n", reset_project_url);
        }
        exit(0);
    }

    if (strlen(update_prefs_url)) {
        PROJECT* project = lookup_project(update_prefs_url);
        if (project) {
            project->sched_rpc_pending = true;
        } else {
            msg_printf(NULL, MSG_ERROR, "project %s not found\n", update_prefs_url);
        }
    }
}

const char *BOINC_RCSID_829bd0f60b = "$Id$";
