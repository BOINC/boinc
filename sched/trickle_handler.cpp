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


// trickle_handler - framework for trickle-up message handler
//
//  --variety variety
//  [--d debug_level]
//  [--one_pass]     // make one pass through table, then exit
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
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "trickle_handler.h"

char variety[256];

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
    while (1) {
        check_stop_daemons();
        bool did_something = do_trickle_scan();
        if (one_pass) break;
        if (!did_something) {
            daemon_sleep(5);
        }
    }
    return 0;
}

void usage(char *name) {
    fprintf(stderr,
        "Framework for trickle-up message handler\n"
        "This program must be linked with app-specific functions:\n\n"
        "int handle_trickle_init(int argc, char** argv)\n"
        "  - initialize\n\n"
        "int handle_trickle(MSG_FROM_HOST&)\n"
        "  - handle a trickle message\n\n"
        "return nonzero on error\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  --variety X                     Set Variety to X\n"
        "  [ -d X ]                        Set debug level to X\n"
        "  [ --one_pass ]                  Make one pass through table, then exit\n"
        "  [ -h | --help ]                 Show this help text\n"
        "  [ -v | --version ]              Shows version information\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    bool one_pass = false;

    check_stop_daemons();

    int j=1;
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "variety")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL,
                    "%s requires an argument\n\n", argv[--i]
                );
                usage(argv[0]);
                exit(1);
            }
            strcpy(variety, argv[i]);
        } else if (!strcmp(argv[i], "-d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL,
                    "%s requires an argument\n\n", argv[--i]
                    
                );
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(0);
        } else {
            // unknown arg - pass to handler
            argv[j++] = argv[i];
        }
    }
    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open failed: %s\n", boincerror(retval)
        );
        exit(1);
    }

    argv[j] = 0;
    retval = handle_trickle_init(argc, argv);
    if (retval) exit(1);

    log_messages.printf(MSG_NORMAL, "Starting trickle handler\n");

    install_stop_signal_handler();

    main_loop(one_pass);
}

const char *BOINC_RCSID_560388f67e = "$Id$";
