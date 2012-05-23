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
// message_handler - check and validate new messages
//  [--d debug_level]
//  [--one_pass]     // make one pass through table, then exit
//
// int handle_message(MSG_FROM_HOST&)
//    handle a message from the host
//
// return nonzero on error


#include "config.h"
#include <unistd.h>
#include <cstdlib>
#include <string>

#include "boinc_db.h"
#include "util.h"
#include "error_numbers.h"
#include "str_util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

char app_name[256];

extern int handle_message(MSG_FROM_HOST&);

int handle_message(MSG_FROM_HOST& mfh) {
    int retval;

    printf("got message \n%s\n", mfh.xml);
    DB_MSG_TO_HOST mth;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = mfh.hostid;
    mth.handled = false;
    strcpy(mth.xml, mfh.xml);
    retval = mth.insert();
    if (retval) {
        printf("insert failed %s\n", boincerror(retval));
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
    while (1) {
        retval = mfh.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_DEBUG,
                    "DB connection lost, exiting\n"
                );
                exit(0);
            }
            break;
        }
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
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open failed: %s\n", boincerror(retval)
        );
        exit(1);
    }
/*
    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
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
            daemon_sleep(5);
        }
    }
    return 0;
}

void usage(char *name) {
    fprintf(stderr,
        "check and validate new messages\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ -d X ]                        Set debug level to X\n"
        "  [ --one_pass ]                  make one pass through table, then exit\n"
        "  [ -h --help ]                   show this help text.\n"
        "  [ -v | --version ]              show version information\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    bool one_pass = false;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    log_messages.printf(MSG_NORMAL, "Starting message handler\n");

    install_stop_signal_handler();

    main_loop(one_pass);
}

const char *BOINC_RCSID_ff3b9880d4 = "$Id$";
