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

// command-line version of the BOINC core client

// This file contains no GUI-related code,
// and is not included in the source code for Mac or Win GUI clients

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "account.h"
#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "prefs.h"
#include "util.h"

// dummies
void create_curtain(){}
void delete_curtain(){}

// This gets called when the client fails to add a project
//
void project_add_failed(PROJECT* project) {
    if (project->scheduler_urls.size()) {
        printf(
            "BOINC failed to log in to %s.\n "
            "Please check your account ID and try again.\n",
            project->master_url
        );
    } else {
        printf(
            "BOINC couldn't get main page for %s.\n"
            "Please check the URL and try again.\n",
            project->master_url
        );
    }
    gstate.detach_project(project);
    exit(1);
}

// Display a message to the user.
// Depending on the priority, the message may be more or less obtrusive
//
void show_message(PROJECT *p, char* msg, int priority) {
    char* x;
    char message[1024];

    strcpy(message, msg);
    if (message[strlen(message)-1] == '\n') {
        message[strlen(message)-1] = 0;
    }

    if (p) {
        x = p->get_project_name();
    } else {
        x = "BOINC";
    }
    switch (priority) {
    case MSG_ERROR:
        fprintf(stderr, "%s [%s] %s\n", timestamp(), x, message);
    default:
        printf("%s [%s] %s\n", timestamp(), x, message);
    }
}

// Prompt user for project URL and authenticator,
// and create an account file
//
int add_new_project() {
    PROJECT project;

    printf("Enter the URL of the project: ");
    scanf("%s", project.master_url);
    printf(
        "You should have already registered with the project\n"
        "and received an account key by email.\n"
        "Paste the account key here: "
    );
    scanf("%s", project.authenticator);

    project.tentative = true;
    project.write_account_file();
    return 0;
}

void quit_client(int a) {
    gstate.requested_exit = true;
}

#ifndef _WIN32
void susp_client(int a) {
    gstate.active_tasks.suspend_all();
    msg_printf(NULL, MSG_INFO, "Suspending activity - user request");
    signal(SIGTSTP, SIG_DFL);
    raise(SIGTSTP);
}

void resume_client(int a) {
    gstate.active_tasks.unsuspend_all();
    msg_printf(NULL, MSG_INFO, "Resuming activity");
}
#endif

int main(int argc, char** argv) {
    int retval;
    double dt;

    setbuf(stdout, 0);
    if (lock_file(LOCK_FILE_NAME)) {
        fprintf(stderr, "Another copy of BOINC is already running\n");
        exit(1);
    }

    read_log_flags();
    gstate.parse_cmdline(argc, argv);
    gstate.parse_env_vars();
    retval = gstate.init();
    if (retval) exit(retval);

#ifndef _WIN32
    // Handle quit signals gracefully
    signal(SIGHUP, quit_client);
    signal(SIGINT, quit_client);
    signal(SIGQUIT, quit_client);
#ifdef SIGPWR
    signal(SIGPWR, quit_client);
#endif
    signal(SIGTSTP, susp_client);
    signal(SIGCONT, resume_client);
#endif

    while (1) {
        if (!gstate.do_something()) {
            dt = dtime();
            gstate.net_sleep(1.);
            dt = dtime() - dt;
            if (log_flags.time_debug) printf("SLEPT %f SECONDS\n", dt);
            fflush(stdout);
        }

        if (gstate.time_to_exit() || gstate.requested_exit) {
            break;
        }
    }
    gstate.cleanup_and_exit();
    return 0;
}
