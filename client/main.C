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

#include "accounts.h"
#include "file_names.h"
#include "log_flags.h"
#include "client_state.h"
#include "util.h"

void show_message(char* message, char* priority) {
    if (!strcmp(priority, "high")) {
        fprintf(stderr, "BOINC core client: %s\n", message);
    } else {
        printf("BOINC core client: %s\n", message);
    }
}

int main(int argc, char** argv) {
    CLIENT_STATE cs;
    ACCOUNTS accounts;
    FILE* f;
    int retval;

    f = fopen(LOG_FLAGS_FILE, "r");
    if (f) {
        log_flags.parse(f);
    }

    retval = accounts.parse_file();
    if (retval) {
        fprintf(stderr, "Can't read accounts file: %d\n", retval);
        exit(retval);
    }

    cs.init(accounts);
    cs.parse_cmdline(argc, argv);
    cs.restart_tasks();
    while (1) {
        if (!cs.do_something()) {
            if (log_flags.time_debug) printf("SLEEP 1 SECOND\n");
            fflush(stdout);
            boinc_sleep(1);
        }
        if (cs.time_to_exit()) {
            exit(0);
        }
    }

	return 0;
}
