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

// assimilator [ -noinsert ]
//

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <vector>

#include "boinc_db.h"
#include "parse.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "assimilate_handler.h"

#define LOCKFILE "assimilator.out"
#define PIDFILE  "assimilator.pid"

SCHED_CONFIG config;
bool noinsert = false;

#define SLEEP_INTERVAL 10

// assimilate all WUs that need it
// return nonzero if did anything
//
bool do_pass(APP& app) {
    DB_WORKUNIT wu;
    DB_RESULT canonical_result, result;
    bool did_something = false;
    char buf[256];
    // int retval;


    check_stop_daemons();

    sprintf(buf, "where appid=%d and assimilate_state=%d", app.id, ASSIMILATE_READY);
    while (!wu.enumerate(buf)) {
        vector<RESULT> results;     // must be inside while()!
        did_something = true;

        log_messages.printf(SCHED_MSG_LOG::DEBUG,
            "[%s] assimilating; state=%d\n", wu.name, wu.assimilate_state
        );

        sprintf(buf, "where workunitid=%d", wu.id);
        while (!result.enumerate(buf)) {
            results.push_back(result);
            if (result.id == wu.canonical_resultid) {
                canonical_result = result;
            }
        }

        assimilate_handler(wu, results, canonical_result);

        wu.assimilate_state = ASSIMILATE_DONE;
        wu.transition_time = time(0);
        wu.update();
    }
    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false;
    DB_APP app;
    int i;
    char buf[256];

    check_stop_daemons();
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-app")) {
            strcpy(app.name, argv[++i]);
        } else if (!strcmp(argv[i], "-noinsert")) {
            noinsert = true;
        } else {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Unrecognized arg: %s\n", argv[i]);
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SCHED_MSG_LOG::NORMAL, "Another copy of assimilator is already running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SCHED_MSG_LOG::NORMAL, "Starting\n");

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't open DB\n");
        exit(1);
    }
    sprintf(buf, "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't find app\n");
        exit(1);
    }
    install_stop_signal_handler();
    if (one_pass) {
        do_pass(app);
    } else {
        while (1) {
            if (!do_pass(app)) sleep(SLEEP_INTERVAL);
        }
    }
}
