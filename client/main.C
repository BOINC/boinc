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

#include "boinc_api.h"
#include "account.h"
#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "prefs.h"
#include "util.h"

// Display a message to the user.
// Depending on the priority, the message may be more or less obtrusive
//
void show_message(char* message, char* priority) {
    if (!strcmp(priority, "high")) {
        fprintf(stderr, "BOINC core client: %s (priority: %s)\n", message, priority);
    } else {
        printf("BOINC core client: %s (priority: %s)\n", message, priority);
    }
}

// Prompt user for project URL and authenticator,
// and create an account file
// TODO: use better input method here, backspace doesn't always seem to work
//
int add_new_project() {
    char master_url[256];
    char authenticator[256];

    printf("Enter the URL of the project: ");
    scanf("%s", master_url);
    printf(
        "You should have already registered with the project\n"
        "and received an account key by email.\n"
        "Paste this ID here: "
    );
    scanf("%s", authenticator);

    // TODO: might be a good idea to verify the ID here
    // by doing an RPC to a scheduling server.
    // But this would require fetching and parsing the master file

    write_account_file(master_url, authenticator);
    return 0;
}

int main(int argc, char** argv) {
    int retval;

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
    while (1) {
        if (!gstate.do_something()) {
            double x;
            gstate.net_sleep(1.);
            x = 1;
            //boinc_sleep(1);
            if (log_flags.time_debug) printf("SLEPT %f SECONDS\n", x);
            fflush(stdout);
        }

        if (gstate.time_to_exit()) {
            printf("time to exit\n");
            break;
        }
    }
    gstate.cleanup_and_exit();
    return 0;
}
