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

//
// trickle_handler - check and validate new trickle messages
//  -app appname
//  [-d debug_level]
//  [-one_pass]     // make one pass through table, then exit
//  [-asynch]       // fork, run in separate process
//
// This program must be linked with an app-specific function:
//
// int handle_trickle(TRICKLE_UP&)
//    handle a trickle message
//
// return nonzero on error

using namespace std;

#include <unistd.h>

#include "boinc_db.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

SCHED_CONFIG config;
char app_name[256];

extern int handle_trickle(TRICKLE_UP&);

int handle_trickle(TRICKLE_UP&) {
    return 0;
}

// make one pass through trickle_ups with handled == 0
// return true if there were any
//
bool do_trickle_scan(APP& app) {
    DB_TRICKLE_UP tup;
    char buf[256];
    bool found=false;

    sprintf(buf, "where appid=%d and handled == 0", app.id);
    while (!tup.enumerate(buf)) {
        handle_trickle(tup);
        found = true;
    }
    return found;
}

int main_loop(bool one_pass) {
    int retval;
    DB_APP app;
    bool did_something;
    char buf[256];

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't find app %s\n", app.name);
        exit(1);
    }

    while (1) {
        check_stop_daemons();
        did_something = do_trickle_scan(app);
        if (one_pass) break;
        if (!did_something) {
            sleep(5);
        }
    }
    return 0;
}


int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false, one_pass = false;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-app")) {
            strcpy(app_name, argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "unrecognized arg: %s\n", argv[i]);
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "Can't parse config file: %d\n", retval
        );
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    log_messages.printf(SCHED_MSG_LOG::NORMAL, "Starting trickle handler\n");

    install_stop_signal_handler();

    main_loop(one_pass);
}
