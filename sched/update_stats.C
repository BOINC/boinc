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

// update_stats: update exponential average credit for users and hosts,
//               and calculate total users/credit for teams
//
// usage: update_stats [-update_teams] [-update_users] [-update_hosts]

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "db.h"
#include "util.h"
#include "config.h"
#include "sched_util.h"

#define LOCKFILE "update_stats.out"

int update_users() {
    USER user;
    int retval;

    while (!db_user_enum_id(user)) {
        update_average(0, 0, user.expavg_credit, user.expavg_time);
        retval = db_user_update(user);
        if (retval) return retval;
    }

    return 0;
}

int update_hosts() {
    HOST host;
    int retval;

    while (!db_host_enum_id(host)) {
        update_average(0, 0, host.expavg_credit, host.expavg_time);
        retval = db_host_update(host);
        if (retval) return retval;
    }

    return 0;
}

int main(int argc, char** argv) {
    CONFIG config;
    int retval, i;
    bool do_update_teams = false;
    bool do_update_users = false;
    bool do_update_hosts = false;

    check_stop_trigger();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-update_teams")) {
            do_update_teams = true;
        } else if (!strcmp(argv[i], "-update_users")) {
            do_update_users = true;
        } else if (!strcmp(argv[i], "-update_hosts")) {
            do_update_hosts = true;
        }
    }

    if (lock_file(LOCKFILE)) {
        fprintf(stderr, "Another copy of update_stats is already running\n");
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config file\n");
        exit(1);
    }
    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr, "Can't open DB\n");
        exit(1);
    }

    if (do_update_teams) {
        retval = update_teams();
        if (retval) {
            fprintf(stderr, "update_teams failed: %d\n", retval);
            exit(1);
        }
    }

    if (do_update_users) {
        retval = update_users();
        if (retval) {
            fprintf(stderr, "update_users failed: %d\n", retval);
            exit(1);
        }
    }

    if (do_update_hosts) {
        retval = update_hosts();
        if (retval) {
            fprintf(stderr, "update_hosts failed: %d\n", retval);
            exit(1);
        }
    }

    return 0;
}
