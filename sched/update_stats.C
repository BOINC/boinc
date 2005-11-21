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

// update_stats:
// Update average credit for idle users, hosts and teams.
// These fields are updates as new credit is granted;
// the purpose of this program is to decay credit of entities
// that are inactive for long periods.
// Hence it should be run about once a day at most.
//
// Also updates the nusers field of teams
//
// usage: update_stats [-update_teams] [-update_users] [-update_hosts] [-asynch]
#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "boinc_db.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE "update_stats.out"
#define PIDFILE  "update_stats.pid"

#define UPDATE_INTERVAL 3600*24*4;

double update_time_cutoff;

int update_users() {
    DB_USER user;
    int retval;
    char buf[256];

    while (!user.enumerate()) {
        if (user.expavg_time > update_time_cutoff) continue;
        update_average(0, 0, CREDIT_HALF_LIFE, user.expavg_credit, user.expavg_time);
        sprintf(
            buf,"expavg_credit=%f, expavg_time=%f",
            user.expavg_credit, user.expavg_time
        );
        retval = user.update_field(buf);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Can't update user %d\n", user.id);
            return retval;
        }
    }

    return 0;
}

int update_hosts() {
    DB_HOST host;
    int retval;
    char buf[256];

    while (!host.enumerate()) {
        if (host.expavg_time > update_time_cutoff) continue;
        update_average(0, 0, CREDIT_HALF_LIFE, host.expavg_credit, host.expavg_time);
        sprintf(
            buf,"expavg_credit=%f, expavg_time=%f",
            host.expavg_credit, host.expavg_time
        );
        retval = host.update_field(buf);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Can't update host %d\n", host.id);
            return retval;
        }
    }

    return 0;
}

int get_team_totals(TEAM& team) {
    int nusers;
    int retval;
    DB_USER user;
    char buf[256];

    // count the number of users on the team
    //
    sprintf(buf, "where teamid=%d", team.id);
    retval = user.count(nusers, buf);
    if (retval) return retval;

    team.nusers = nusers;

    return 0;
}

// fill in the nusers, total_credit and expavg_credit fields
// of the team table.
// This may take a while; don't do it often
//
int update_teams() {
    DB_TEAM team;
    int retval;
    char buf[256];

    while (!team.enumerate()) {
        retval = get_team_totals(team);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "update_teams: get_team_credit([TEAM#%d]) failed: %d\n",
                team.id,
                retval
            );
            continue;
        }
        if (team.expavg_time < update_time_cutoff) {
            update_average(0, 0, CREDIT_HALF_LIFE, team.expavg_credit, team.expavg_time);
        }
        sprintf(
            buf, "expavg_credit=%f, expavg_time=%f",
            team.expavg_credit, team.expavg_time
        );
        retval = team.update_field(buf);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Can't update team %d\n", team.id);
            return retval;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    SCHED_CONFIG config;
    int retval, i;
    bool do_update_teams = false, do_update_users = false;
    bool do_update_hosts = false, asynch = false;

    update_time_cutoff = time(0) - UPDATE_INTERVAL;

    check_stop_daemons();

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
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Unrecognized arg: %s\n", argv[i]);
        }
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting\n");


    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Can't parse config file\n");
        exit(1);
    }
    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Can't open DB\n");
        exit(1);
    }

    if (do_update_users) {
        retval = update_users();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "update_users failed: %d\n", retval);
            exit(1);
        }
    }

    if (do_update_hosts) {
        retval = update_hosts();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "update_hosts failed: %d\n", retval);
            exit(1);
        }
    }

    if (do_update_teams) {
        retval = update_teams();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "update_teams failed: %d\n", retval);
            exit(1);
        }
    }

    return 0;
}

const char *BOINC_RCSID_6b05e9ecce = "$Id$";
