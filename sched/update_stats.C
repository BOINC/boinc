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
// usage: update_stats [-update_teams] [-update_users] [-update_hosts] [-asynch]

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

    while (!boinc_db_user_enum_id(user)) {
        update_average(0, 0, user.expavg_credit, user.expavg_time);
        retval = boinc_db_user_update(user);
        if (retval) return retval;
    }

    return 0;
}

int update_hosts() {
    HOST host;
    int retval;

    while (!boinc_db_host_enum_id(host)) {
        update_average(0, 0, host.expavg_credit, host.expavg_time);
        retval = boinc_db_host_update(host);
        if (retval) return retval;
    }

    return 0;
}

int get_team_credit(TEAM &t) {
    int nusers;
    double expavg_credit, total_credit;
    int retval;

    // count the number of users on a team
    retval = boinc_db_user_count_team(t, nusers);
    if (retval) return retval;

    // get the summed credit values for a team
    retval = boinc_db_user_sum_team_expavg_credit(t, expavg_credit);
    if (retval) return retval;
    retval = boinc_db_user_sum_team_total_credit(t, total_credit);
    if (retval) return retval;

    t.nusers = nusers;
    t.total_credit = total_credit;
    t.expavg_credit = expavg_credit;

    return 0;
}

// fill in the nusers, total_credit and expavg_credit fields
// of the team table.
// This may take a while; don't do it often
//
int update_teams() {
    TEAM team;
    int retval;

    while (!boinc_db_team_enum(team)) {
        retval = get_team_credit(team);
        if (retval) return retval;

        // update the team record
        retval = boinc_db_team_update(team);
        if (retval) return retval;
    }
    return 0;
}

int main(int argc, char** argv) {
    CONFIG config;
    int retval, i;
    bool do_update_teams = false, do_update_users = false;
    bool do_update_hosts = false, asynch = false;
    char buf[256];

    check_stop_trigger();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-update_teams")) {
            do_update_teams = true;
        } else if (!strcmp(argv[i], "-update_users")) {
            do_update_users = true;
        } else if (!strcmp(argv[i], "-update_hosts")) {
            do_update_hosts = true;
        } else if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else {
            sprintf(buf, "Unrecognized arg: %s\n", argv[i]);
            write_log(buf);
        }
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // Call lock_file after fork(), because file locks are not always inherited
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

    if (do_update_teams) {
        retval = update_teams();
        if (retval) {
            fprintf(stderr, "update_teams failed: %d\n", retval);
            exit(1);
        }
    }

    return 0;
}
