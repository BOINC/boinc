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

void show_message(char* message, char* priority) {
    if (!strcmp(priority, "high")) {
        fprintf(stderr, "BOINC core client: %s\n", message);
    } else {
        printf("BOINC core client: %s\n", message);
    }
}

// Prompt user for project URL and authenticator,
// and create a prototype prefs file
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
    CLIENT_STATE cs;
    PREFS* prefs;
    FILE* f;
    int retval;

    setbuf(stdout, 0);

    f = fopen(LOG_FLAGS_FILE, "r");
    if (f) {
        log_flags.parse(f);
        fclose(f);
    }

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

    cs.init(prefs);
    cs.parse_cmdline(argc, argv);
    if(cs.run_time_tests()) {
        cs.time_tests();
    }
    cs.restart_tasks();
    while (1) {
        if (!cs.do_something()) {
            if (log_flags.time_debug) printf("SLEEP 1 SECOND\n");
            fflush(stdout);
            boinc_sleep(1);
        }
        if (cs.time_to_exit()) {
            printf("time to exit\n");
            break;
        }
    }
    cs.exit_tasks();
    return 0;
}
