// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//
// message_handler - check and validate new messages
//  [-d debug_level]
//  [-one_pass]     // make one pass through table, then exit
//  [-asynch]       // fork, run in separate process
//
// int handle_message(MSG_FROM_HOST&)
//    handle a message from the host
//
// return nonzero on error

using namespace std;

#include "config.h"
#include <unistd.h>

#include "boinc_db.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

SCHED_CONFIG config;
char app_name[256];

extern int handle_message(MSG_FROM_HOST&);

int handle_message(MSG_FROM_HOST& mfh) {
    int retval;

    printf(
        "got message \n%s\n",
        mfh.xml
        );
    DB_MSG_TO_HOST mth;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = mfh.hostid;
    mth.handled = false;
    strcpy(mth.xml, mfh.xml);
    retval = mth.insert();
    if (retval) {
        printf("insert failed %d\n", retval);
    }
    return 0;
}

// make one pass through msgs_from_host with handled == 0
// return true if there were any
//
bool do_message_scan() {
    DB_MSG_FROM_HOST mfh;
    char buf[256];
    bool found=false;
    int retval;

    sprintf(buf, "where handled=0");
    while (!mfh.enumerate(buf)) {
        retval = handle_message(mfh);
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
    bool did_something;
    // char buf[256];

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "boinc_db.open failed: %d\n", retval
        );
        exit(1);
    }
/*
    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "can't find app %s\n", app_name
        );
        exit(1);
    }
*/
    while (1) {
        check_stop_daemons();
        did_something = do_message_scan();
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
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "unrecognized arg: %s\n", argv[i]
            );
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "Can't parse config file: %d\n", retval
        );
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting message handler\n");

    install_stop_signal_handler();

    main_loop(one_pass);
}

const char *BOINC_RCSID_ff3b9880d4 = "$Id$";
