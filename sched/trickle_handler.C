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
char variety[256];

extern int handle_trickle(MSG_FROM_HOST&);

int handle_trickle(MSG_FROM_HOST& mfh) {
    int retval;

    printf(
        "got trickle-up \n%s\n\n",
        mfh.xml
    );
    DB_MSG_TO_HOST mth;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = mfh.hostid;
    strcpy(mth.variety, mfh.variety);
    mth.handled = false;
    sprintf(mth.xml,
        "<trickle_down>\n"
        "%s"
        "</trickle_down>\n",
        mfh.xml
    );
    retval = mth.insert();
    if (retval) {
        printf("insert failed %d\n", retval);
    }
    return 0;
}

// make one pass through trickle_ups with handled == 0
// return true if there were any
//
bool do_trickle_scan(APP& app) {
    DB_MSG_FROM_HOST mfh;
    char buf[256];
    bool found=false;
    int retval;

    sprintf(buf, "where variety='%s' and handled=0", variety);
    while (!mfh.enumerate(buf)) {
        retval = handle_trickle(mfh);
        if (!retval) {
            mfh.handled = true;
            mfh.update();
        }
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
        } else if (!strcmp(argv[i], "-variety")) {
            strcpy(variety, argv[++i]);
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

const char *BOINC_RCSID_560388f67e = "$Id$";
