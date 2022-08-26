// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// update_stats:
// Update average credit for idle users, hosts and teams.
// These fields are updated as new credit is granted;
// the purpose of this program is to decay credit of entities
// that are inactive for long periods.
// Run it about once a day.
//
// Also updates the nusers field of teams
//
// usage: update_stats args
//  [--update_teams]
//  [--update_users]
//  [--update_hosts]
//  [--min_age nsec] don't update items updated more recently than this


#include "config.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"
#include "error_numbers.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

// If the item's average credit has been updated more recently than this,
// don't update it (optimizes performance).

#define MIN_AGE 86400

double max_update_time;

int update_users() {
    DB_USER user;
    int retval;
    char buf[256];
    double now = dtime();

    while (1) {
        sprintf(buf, "where expavg_credit>0.1 and expavg_time < %f", max_update_time);
        retval = user.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_CRITICAL, "lost DB conn\n");
                exit(1);
            }
            break;
        }
        update_average(
            now, 0, 0, CREDIT_HALF_LIFE, user.expavg_credit, user.expavg_time
        );
        sprintf( buf, "expavg_credit=%f, expavg_time=%f",
            user.expavg_credit, user.expavg_time
        );
        retval = user.update_field(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "Can't update user %lu\n", user.id);
            return retval;
        }
    }

    return 0;
}

int update_hosts() {
    DB_HOST host;
    int retval;
    char buf[256];
    double now = dtime();

    while (1) {
        sprintf(buf, "where expavg_credit>0.1 and expavg_time < %f", max_update_time);
        retval = host.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_CRITICAL, "lost DB conn\n");
                exit(1);
            }
            break;
        }
        update_average(
            now, 0, 0, CREDIT_HALF_LIFE, host.expavg_credit, host.expavg_time
        );
        sprintf(
            buf,"expavg_credit=%f, expavg_time=%f",
            host.expavg_credit, host.expavg_time
        );
        retval = host.update_field(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "Can't update host %lu\n", host.id);
            return retval;
        }
    }

    return 0;
}

int get_team_totals(TEAM& team) {
    long nusers;
    int retval;
    DB_USER user;
    char buf[256];

    // count the number of users on the team
    //
    sprintf(buf, "where teamid=%lu", team.id);
    retval = user.count(nusers, buf);
    if (retval) return retval;

    if (team.nusers != nusers) {
        log_messages.printf(MSG_CRITICAL,
            "updating member count for [TEAM#%lu]: database has %d users, count shows %ld\n",
            team.id, team.nusers, nusers
        );
    }
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
    double now = dtime();

    while (1) {
        retval = team.enumerate("where expavg_credit>0.1");
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_CRITICAL, "lost DB conn\n");
                exit(1);
            }
            break;
        }

        retval = get_team_totals(team);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "update_teams: get_team_credit([TEAM#%lu]) failed: %d\n",
                team.id,
                retval
            );
            continue;
        }
        if (team.expavg_time < max_update_time) {
            update_average(
                now, 0, 0, CREDIT_HALF_LIFE, team.expavg_credit,
                team.expavg_time
            );
        }
        sprintf(
            buf, "expavg_credit=%f, expavg_time=%f, nusers=%d",
            team.expavg_credit, team.expavg_time, team.nusers
        );
        retval = team.update_field(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "Can't update team %lu\n", team.id);
            return retval;
        }
    }
    return 0;
}

void usage(char *name) {
    fprintf(stderr,
        "Update average credit for idle users, hosts and teams.\n"
        "These fields are updates as new credit is granted;\n"
        "the purpose of this program is to decay credit of entities\n"
        "that are inactive for long periods.\n"
        "Hence it should be run about once a day at most.\n\n"
        "Also updates the nusers field of teams\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ -d X ]                        Set debug level to X\n"
        "  [ --update_teams ]              Updates teams.\n"
        "  [ --update_users ]              Updates users.\n"
        "  [ --update_hosts ]              Updates hosts.\n"
        "  [ -h | --help ]                 Shows this help text\n"
        "  [ -v | --version ]              Shows version information\n",
        name
    );
}

int main(int argc, char** argv) {
    int retval, i;
    bool do_update_teams = false;
    bool do_update_users = false;
    bool do_update_hosts = false;

    max_update_time = time(0) - MIN_AGE;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "update_teams")) {
            do_update_teams = true;
        } else if (is_arg(argv[i], "update_users")) {
            do_update_users = true;
        } else if (is_arg(argv[i], "update_hosts")) {
            do_update_hosts = true;
        } else if (is_arg(argv[i], "min_age")) {
            double x = atof(argv[++i]);
            max_update_time = time(0) - x;
        } else if (!strcmp(argv[i], "-d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
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
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    // if no do_update flags set, set them all
    //
    if (!do_update_teams && !do_update_users && !do_update_hosts) {
        do_update_teams = true;
        do_update_users = true;
        do_update_hosts = true;
    }

    log_messages.printf(MSG_NORMAL, "Starting\n");

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }
    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't open DB: %s\n",
            boinc_db.error_string()
        );
        exit(1);
    }
    retval = boinc_db.set_isolation_level(READ_UNCOMMITTED);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %s; %s\n",
            boincerror(retval), boinc_db.error_string()
        );
    }

    if (do_update_users) {
        retval = update_users();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "update_users failed: %s\n", boincerror(retval)
            );
            exit(1);
        }
    }

    if (do_update_hosts) {
        retval = update_hosts();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "update_hosts failed: %s\n", boincerror(retval)
            );
            exit(1);
        }
    }

    if (do_update_teams) {
        retval = update_teams();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "update_teams failed: %s\n", boincerror(retval)
            );
            exit(1);
        }
    }

    log_messages.printf(MSG_NORMAL, "Finished\n");
    return 0;
}
