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

#include "boinc_api.h"

#ifndef _WIN32
#include <unistd.h>
#endif


#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "prefs.h"
#include "util.h"

// Display a message to the user.  Depending on the priority, the
// message may be more or less obtrusive
//
void show_message(char* message, char* priority) {
    if (!strcmp(priority, "high")) {
        fprintf(stderr, "BOINC core client: %s (priority: %s)\n", message, priority);
    } else {
        printf("BOINC core client: %s (priority: %s)\n", message, priority);
    }
}

// Prompt user for project URL and authenticator,
// and create a prototype prefs file
// TODO: use better input method here, backspace doesn't always seem to work
//
int initialize_prefs() {
    char master_url[256];
    char authenticator[256];

    printf("Enter the URL of the project: ");
    scanf("%s", master_url);
    printf(
        "You should have already registered with the project\n"
        "and received an account ID by email.\n"
        "Paste this ID here: "
    );
    scanf("%s", authenticator);

    // TODO: might be a good idea to verify the ID here
    // by doing an RPC to a scheduling server.
    // But this would require fetching and parsing the master file

    write_initial_prefs(master_url, authenticator);
    return 0;
}

#ifdef __APPLE_CC__
#include "mac_main.h"
int main() {
    signal(SIGPIPE, SIG_IGN);
    read_log_flags();

    if (gstate.init()) return -1;
    
    // mac_setup won't return until the main application loop has quit
    if (!mac_setup ()) return -1;
    // Afterwards, we clean up and exit
    mac_cleanup ();
}

#else

int main(int argc, char** argv) {
    int retval;

    setbuf(stdout, 0);
    read_log_flags();
    gstate.parse_cmdline(argc, argv);
    retval = gstate.init();
    if (retval) exit(retval);
    while (1) {
        fflush(stdout);
        if (!gstate.do_something()) {
            if (log_flags.time_debug) printf("SLEEP 1 SECOND\n");
            fflush(stdout);
            boinc_sleep(1);
        }

        if (gstate.time_to_exit()) {
            printf("time to exit\n");
            break;
        }
    }
    gstate.exit();
    return 0;
}

#endif
