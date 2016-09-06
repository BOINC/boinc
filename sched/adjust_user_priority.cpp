// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// adjust_user_priority [--no_update] --user userid --flops N --app app_name
//
// adjust user priority (i.e. logical start time)
// to reflect a certain amount of computing
// and write the new value to stdout.
// For use by multi-user projects quotas.
//
// --no_update: don't update DB

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "backend_lib.h"

void usage(const char* msg="") {
    fprintf(stderr,
        "%susage: adjust_user_priority [--no_update] --user userid --flops flop_count --app app_name\n",
        msg
    );
    exit(1);
}

int main(int argc, char** argv) {
    char buf[256];
    bool no_update = false;
    int userid=0;
    char* app_name = NULL;
    double flop_count = 0;

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--no_update")) {
            no_update = true;
        } else if (!strcmp(argv[i], "--user")) {
            userid = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--flops")) {
            flop_count = atof(argv[++i]);
        } else {
            fprintf(stderr, "bad arg: %s\n", argv[i]);
            usage();
        }
    }
    if (!app_name) usage("missing --app\n");
    if (!userid) usage("missing --user\n");
    if (flop_count <= 0) usage("missing --flops\n");

    int retval = config.parse_file();
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

    DB_APP app;
    snprintf(buf, sizeof(buf), "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        fprintf(stderr, "no such app %s\n", argv[3]);
        exit(1);
    }

    // normalize by the app's min avg PFC
    //
    if (app.min_avg_pfc) {
        flop_count *= app.min_avg_pfc;
    }

    DB_USER user;
    retval = user.lookup_id(userid);
    if (retval) {
        fprintf(stderr, "no such user %d\n", userid);
        exit(1);
    }
    DB_USER_SUBMIT us;
    sprintf(buf, "where user_id=%d", userid);
    retval = us.lookup(buf);
    if (retval) {
        fprintf(stderr, "unauthorized user %d\n", userid);
        exit(1);
    }

    double total_quota, project_flops;
    retval = get_total_quota(total_quota);
    if (retval) {
        fprintf(stderr, "get_total_quota() failed: %d\n", retval);
        exit(1);
    }
    retval = get_project_flops(project_flops);
    if (retval) {
        fprintf(stderr, "get_project_flops() failed: %d\n", retval);
        exit(1);
    }
    double delta = user_priority_delta(
        us, flop_count, total_quota, project_flops
    );

    double x = us.logical_start_time;
    if (x < dtime()) x = dtime();
    x += delta;

    if (!no_update) {
        char set_clause[256], where_clause[256];
        sprintf(set_clause, "logical_start_time=%f", x);
        sprintf(where_clause, "user_id=%lu", us.user_id);
        retval = us.update_fields_noid(set_clause, where_clause);
        if (retval) {
            fprintf(stderr, "update_fields_noid() failed: %d\n", retval);
            exit(1);
        }
    }
    printf("%f\n", x);
}
