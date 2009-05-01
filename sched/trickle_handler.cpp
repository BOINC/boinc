// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//
// trickle_handler - framework for trickle-up message handler
//
//  -variety variety
//  [-d debug_level]
//  [-one_pass]     // make one pass through table, then exit
//
// This program must be linked with an app-specific function:
//
// int handle_trickle(MSG_FROM_HOST&)
//    handle a trickle message
//
// return nonzero on error

#include "config.h"
#include <unistd.h>

#include "boinc_db.h"
#include "util.h"
#include "error_numbers.h"
#include "str_util.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

char variety[256];

extern int handle_trickle(MSG_FROM_HOST&);

// The following is an example.
// It echoes whatever the app sent us, as a trickle-down message.
// Replace it with your own function.
//
// Note: you're passed the host ID (in mfh.hostid).
// From this you can get the HOST and USER records with
// DB_HOST host;
// DB_USER user;
// host.lookup_id(mfh.hostid);
// user.lookup_id(host.userid);
//
// Then you can modify and update these as needed, e.g. to grant credit.
// (in that case you may also need update the team).
// See is_valid() in validator.cpp.
//
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
bool do_trickle_scan() {
    DB_MSG_FROM_HOST mfh;
    char buf[256];
    bool found=false;
    int retval;

    sprintf(buf, "where variety='%s' and handled=0", variety);
    while (1) {
        retval = mfh.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                fprintf(stderr, "lost DB conn\n");
                exit(1);
            }
            break;
        }
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
    bool did_something;

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    while (1) {
        check_stop_daemons();
        did_something = do_trickle_scan();
        if (one_pass) break;
        if (!did_something) {
            sleep(5);
        }
    }
    return 0;
}


int main(int argc, char** argv) {
    int i, retval;
    bool one_pass = false;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-variety")) {
            strcpy(variety, argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else {
            log_messages.printf(MSG_CRITICAL,
                "unrecognized arg: %s\n", argv[i]
            );
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse ../config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    log_messages.printf(MSG_NORMAL,
        "Starting trickle handler\n"
    );

    install_stop_signal_handler();

    main_loop(one_pass);
}

const char *BOINC_RCSID_560388f67e = "$Id$";
