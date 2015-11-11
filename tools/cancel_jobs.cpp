// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// cancel_jobs min-ID max-ID
//    cancel jobs from min-ID to max-ID inclusive
// cancel_jobs --name wuname
//    cancel the job with the given name

#include <stdio.h>

#include "backend_lib.h"
#include "sched_config.h"

void usage() {
    fprintf(stderr, "Usage: cancel_jobs min-ID max-ID\n");
    fprintf(stderr, "or cancel_jobs --name wuname\n");
    exit(1);
}

int main(int argc, char** argv) {
    if (argc != 3) usage();

    int retval = config.parse_file();
    if (retval) exit(1);

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        printf("boinc_db.open: %s\n", boincerror(retval));
        exit(1);
    }
    if (!strcmp(argv[1], "--name")) {
        DB_WORKUNIT wu;
        char buf[256];
        snprintf(buf, sizeof(buf), "where name='%s'", argv[2]);
        retval = wu.lookup(buf);
        if (retval) {
            fprintf(stderr, "No workunit named '%s'\n", argv[2]);
            exit(1);
        }
        retval = cancel_job(wu);
    } else {
        int min_id = atoi(argv[1]);
        int max_id = atoi(argv[2]);
        if (!min_id || !max_id) usage();
        retval = cancel_jobs(min_id, max_id);
    }
    if (retval) {
        fprintf(stderr, "cancel job failed: %s\n", boincerror(retval));
        exit(retval);
    }
}
