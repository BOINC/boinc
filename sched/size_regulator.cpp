// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// daemon to regulate the transition of results from INACTIVE to UNSENT,
// to maintain a buffer of UNSENT results of each size class.

#include <stdio.h>

#include "boinc_db.h"

#include "util.h"

#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

char* app_name = NULL;
int lo = 0;
int hi = 0;
int sleep_time = 0;
DB_APP app;
const char* order_clause = "";

void usage(){
    fprintf(stderr, "usage: size_regulator --app_name x --lo x --hi x --sleep_time x\n");
    exit(1);
}

int do_pass(bool& action) {
    DB_RESULT result;
    int unsent[100];
    int retval = result.get_unsent_counts(app, unsent, hi);
    if (retval) return retval;
    action = false;
    for (int i=0; i<app.n_size_classes; i++) {
        log_messages.printf(MSG_NORMAL, "%d unsent for class %d\n", unsent[i], i);
        if (unsent[i] < lo) {
            int n = hi - unsent[i], nchanged;
            log_messages.printf(MSG_NORMAL,
                "releasing %d jobs of size class %d\n", n, i
            );
            retval = result.make_unsent(app, i, n, order_clause, nchanged);
            if (retval) return retval;
            log_messages.printf(MSG_NORMAL,
                "%d jobs released\n", nchanged
            );
            if (nchanged == n) {
                action = true;
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    int retval;
    char buf[256];

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--app_name")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--lo")) {
            lo = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--hi")) {
            hi = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "--debug_leveld")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "--sleep_time")) {
            sleep_time = atoi(argv[++i]);
            if (sleep_time < 0) sleep_time = 0;
            if (sleep_time > 1000000) sleep_time = 1000000;
        } else if (!strcmp(argv[i], "--random_order")) {
            order_clause = " order by random ";
        } else if (!strcmp(argv[i], "--priority_asc")) {
            order_clause = " order by priority asc ";
        } else if (!strcmp(argv[i], "--priority_order")) {
            order_clause = " order by priority desc ";
        } else if (!strcmp(argv[i], "--priority_order_create_time")) {
            order_clause = " order by priority desc, workunitid ";
        } else {
            usage();
        }
    }
    if (!app_name || !lo || !hi || !sleep_time) {
        usage();
    }

    log_messages.printf(MSG_NORMAL, "Starting\n");

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
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        exit(1);
    }

    snprintf(buf, sizeof(buf), "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "no such app: %s\n", app_name);
        exit(1);
    }
    if (app.n_size_classes < 2) {
        log_messages.printf(MSG_CRITICAL, "app '%s' is not multi-size\n", app_name);
        exit(1);
    }
    while (1) {
        bool action;
        retval = do_pass(action);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "do_pass(): %s", boincerror(retval)
            );
            exit(1);
        }
        if (!action) {
            log_messages.printf(MSG_NORMAL, "sleeping\n");
            daemon_sleep(sleep_time);
        }
    }
}
