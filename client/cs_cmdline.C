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

#include <stdio.h>

#include "client_state.h"

static void print_options(char* prog) {
    printf(
        "Usage: %s [options]\n"
        "    -version               show version info\n"
        "    -show_projects         show attached projects\n"
        "    -detach_project URL    detach from a project\n"
        "    -reset_project URL     reset (clear) a project\n"
        "    -attach_project        attach to a project (will prompt for URL, account key)\n"
        "    -update_prefs          contact all projects to update preferences\n"
        "    -run_cpu_benchmarks    run the CPU benchmarks\n",
        prog
    );
}

// Parse the command line arguments passed to the client
// NOTE: init() has not been called at this point
// (i.e. client_state.xml has not been parsed)
//
void CLIENT_STATE::parse_cmdline(int argc, char** argv) {
    int i;
    bool show_options = false;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-exit_when_idle")) {
            exit_when_idle = true;
        } else if (!strcmp(argv[i], "-return_results_immediately")) {
            return_results_immediately = true;
        } else if (!strcmp(argv[i], "-skip_cpu_benchmarks")) {
            skip_cpu_benchmarks = true;
        } else if (!strcmp(argv[i], "-exit_after_app_start")) {
            if (i == argc-1) show_options = true;
            else exit_after_app_start_secs = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-file_xfer_giveup_period")) {
            if (i == argc-1) show_options = true;
            else file_xfer_giveup_period = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-min")) {
            global_prefs.run_minimized = true;
        } else if (!strcmp(argv[i], "-suspend")) {
            user_run_request = USER_RUN_REQUEST_NEVER;
        } else if (!strcmp(argv[i], "-saver")) {
            start_saver = true;
        } else if (!strncmp(argv[i], "-psn_", strlen("-psn_"))) {
            // ignore -psn argument on Mac OS X
        } else if (!strcmp(argv[i], "-exit_before_upload")) {
            exit_before_upload = true;
        // The following are only used for testing to alter scheduler/file transfer
        // backoff rates
        } else if (!strcmp(argv[i], "-master_fetch_period")) {
            if (i == argc-1) show_options = true;
            else master_fetch_period = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-retry_base_period")) {
            if (i == argc-1) show_options = true;
            else retry_base_period = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-retry_cap")) {
            if (i == argc-1) show_options = true;
            else retry_cap = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-master_fetch_retry_cap")) {
            if (i == argc-1) show_options = true;
            else master_fetch_retry_cap = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-master_fetch_interval")) {
            if (i == argc-1) show_options = true;
            else master_fetch_interval = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-sched_retry_delay_min")) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_min = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-sched_retry_delay_max")) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_max = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-pers_retry_delay_min")) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_min = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-pers_retry_delay_max")) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_max = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-pers_giveup")) {
            if (i == argc-1) show_options = true;
            else pers_giveup = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-debug_fake_exponential_backoff")) {
            debug_fake_exponential_backoff = true;

        // the above options are private (i.e. not shown by -help)
        // Public options follow.
        // NOTE: if you change or add anything, make the same chane
        // in show_options() (above) and in doc/client.php

        } else if (!strcmp(argv[i], "-show_projects")) {
            show_projects = true;
        } else if (!strcmp(argv[i], "-detach_project")) {
            if (i == argc-1) show_options = true;
            else strcpy(detach_project_url, argv[++i]);
        } else if (!strcmp(argv[i], "-reset_project")) {
            if (i == argc-1) show_options = true;
            else strcpy(reset_project_url, argv[++i]);
        } else if (!strcmp(argv[i], "-update_prefs")) {
            if (i == argc-1) show_options = true;
            else strcpy(update_prefs_url, argv[++i]);
        } else if (!strcmp(argv[i], "-run_cpu_benchmarks")) {
            run_cpu_benchmarks = true;
        } else if (!strcmp(argv[i], "-attach_project")) {
            add_new_project();
        } else if (!strcmp(argv[i], "-version")) {
            printf( "%.2f %s\n", MAJOR_VERSION+(MINOR_VERSION/100.0), HOSTTYPE );
            exit(0);
        } else if (!strcmp(argv[i], "-help")) {
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
