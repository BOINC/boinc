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

// update_stats
// update average credit for users and hosts,
// and calculate total users/credit for teams
//
// usage: update_stats [-update_teams] [-update_users] [-update_hosts] [-asynch]

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "boinc_db.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"

#define LOCKFILE "update_stats.out"
#define PIDFILE  "update_stats.pid"

int update_users() {
    DB_USER user;
    int retval;

    while (!user.enumerate()) {
        update_average(0, 0, user.expavg_credit, user.expavg_time);
        retval = user.update();
        if (retval) return retval;
    }

    return 0;
}

int update_hosts() {
    DB_HOST host;
    int retval;

    while (!host.enumerate()) {
        update_average(0, 0, host.expavg_credit, host.expavg_time);
        retval = host.update();
        if (retval) return retval;
    }

    return 0;
}

int get_team_credit(TEAM& team) {
    int nusers;
    double expavg_credit, total_credit;
    int retval;
    DB_USER user;
    char buf[256];

    // count the number of users on the team
    //
    sprintf(buf, "where teamid=%d", team.id);
    retval = user.count(nusers, buf);
    if (retval) return retval;

    // get the summed credit values for a team
    //
    sprintf(buf, "where teamid=%d", team.id);
    retval = user.sum(expavg_credit, "expavg_credit", buf);
    if (retval) return retval;
    retval = user.sum(total_credit, "total_credit", buf);
    if (retval) return retval;

    team.nusers = nusers;
    team.total_credit = total_credit;
    team.expavg_credit = expavg_credit;

    return 0;
}

// fill in the nusers, total_credit and expavg_credit fields
// of the team table.
// This may take a while; don't do it often
//
int update_teams() {
    DB_TEAM team;
    int retval;

    while (!team.enumerate()) {
        retval = get_team_credit(team);
        if (retval) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "update_teams: get_team_credit([TEAM#%d]) failed: %d\n",
                team.id,
                retval);
            //return retval;
            continue;
        }

        retval = team.update();
        if (retval) return retval;
    }
    return 0;
}

int main(int argc, char** argv) {
    SCHED_CONFIG config;
    int retval, i;
    bool do_update_teams = false, do_update_users = false;
    bool do_update_hosts = false, asynch = false;

    check_stop_trigger();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-update_teams")) {
            do_update_teams = true;
        } else if (!strcmp(argv[i], "-update_users")) {
            do_update_users = true;
        } else if (!strcmp(argv[i], "-update_hosts")) {
            do_update_hosts = true;
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else {
            log_messages.printf(SchedMessages::CRITICAL, "Unrecognized arg: %s\n", argv[i]);
        }
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SchedMessages::NORMAL, "Another copy of update_stats is already running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SchedMessages::NORMAL, "Starting\n");


    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "Can't parse config file\n");
        exit(1);
    }
    retval = boinc_db.open(config.db_name, config.db_host, config.db_passwd);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "Can't open DB\n");
        exit(1);
    }

    if (do_update_users) {
        retval = update_users();
        if (retval) {
            log_messages.printf(SchedMessages::CRITICAL, "update_users failed: %d\n", retval);
            exit(1);
        }
    }

    if (do_update_hosts) {
        retval = update_hosts();
        if (retval) {
            log_messages.printf(SchedMessages::CRITICAL, "update_hosts failed: %d\n", retval);
            exit(1);
        }
    }

    if (do_update_teams) {
        retval = update_teams();
        if (retval) {
            log_messages.printf(SchedMessages::CRITICAL, "update_teams failed: %d\n", retval);
            exit(1);
        }
    }

    return 0;
}
