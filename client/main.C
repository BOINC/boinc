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
    if(message==NULL) {
        fprintf(stderr, "error: show_message: unexpected NULL pointer message\n");
    }
    if(priority==NULL) {
        fprintf(stderr, "error: show_message: unexpected NULL pointer priority\n");
    }
    if (!strcmp(priority, "high")) {
        fprintf(stderr, "BOINC core client: %s\n", message);
    } else {
        printf("BOINC core client: %s\n", message);
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

int main(int argc, char** argv) {
    PREFS* prefs;
    FILE* f;
    int retval;

    // Set the stdout buffer to 0, such that stdout output
    // immediately gets written out
    setbuf(stdout, 0);

    // Read any log flags preferences, used mainly for debugging
    f = fopen(LOG_FLAGS_FILE, "r");
    if (f) {
        log_flags.parse(f);
        fclose(f);
    }

    // Read the user preferences file, if it exists.  If it doesn't,
    // prompt user for project URL via initialize_prefs()
    prefs = new PREFS;
    retval = prefs->parse_file();
    if (retval) {
        retval = initialize_prefs();
        if (retval) {
            printf("can't initialize prefs.xml\n");
            exit(retval);
        }
        retval = prefs->parse_file();
        if (retval) {
            printf("can't initialize prefs.xml\n");
            exit(retval);
        }
    }

    // Initialize the client state with the preferences
    gstate.init(prefs);
    // Parse command line arguments
    gstate.parse_cmdline(argc, argv);
    // Run the time tests and host information check if needed
    // TODO: break time tests and host information check into two
    //       separate functions?
    if(gstate.run_time_tests()) {
        gstate.time_tests();
    }
    // Restart any tasks that were running when we last quit the client
    gstate.restart_tasks();
    while (1) {
        // do_something is where the meat of the clients work is done
        // it will return false if it had nothing to do, in which case
        // client will go to sleep for a second
        if (!gstate.do_something()) {
            if (log_flags.time_debug) printf("SLEEP 1 SECOND\n");
            fflush(stdout);
            boinc_sleep(1);
        }
        // If it's time to exit, break out of the while loop
        if (gstate.time_to_exit()) {
            printf("time to exit\n");
            break;
        }
    }
    // Clean everything up and gracefully exit
    gstate.exit_tasks();
    return 0;
}
