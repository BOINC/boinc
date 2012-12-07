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

// adjust_user_priority userid flop_count app_name
//
// adjust user priority (i.e. logical start time)
// to reflect a certain amount of computing

#include <stdio.h>
#include <stdlib.h>

#include "backend_lib.h"

void usage() {
    fprintf(stderr, "usage: adjust_user_priority userid flop_count app_name\n");
    exit(1);
}

int main(int argc, char** argv) {
    char buf[256];

    if (argc != 4) usage();
    int userid = atoi(argv[1]);
    double flop_count = atof(argv[2]);

    if (flop_count <= 0) usage();

    DB_APP app;
    sprintf(buf, "name='%s'", argv[3]);
    int retval = app.lookup(buf);
    if (retval) {
        fprintf(stderr, "no such app %s\n", argv[3]);
        exit(1);
    }

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
    sprintf(buf, "user_id=%d", userid);
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
    retval = adjust_user_priority(us, flop_count, total_quota, project_flops);
    if (retval) {
        fprintf(stderr, "adjust_user_priority() failed: %d\n", retval);
        exit(1);
    }
}
