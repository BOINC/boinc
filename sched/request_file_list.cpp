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

// request_file_list [options]
// --host_id            number of host to upload from
//                     or 'all' if for all active hosts
//
// Create a msg_to_host_that requests the list of permanant files
// associated with the project
//
// Run this in the project root dir

#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <time.h>

#include "boinc_db.h"
#include "str_util.h"
#include "error_numbers.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"

int request_file_list(int host_id) {
    DB_MSG_TO_HOST mth;
    int retval;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    strcpy(mth.variety, "request_file_list");
    mth.handled = false;
    strcpy(mth.xml, "<request_file_list/>");
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
        return retval;
    }
    return 0;
}

int request_files_from_all() {
    DB_HOST host;

    while(1) {
        int retval = host.enumerate();
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                fprintf(stderr, "lost DB connection\n");
                exit(1);
            }
            break;
        }
        request_file_list(host.get_id());
    }
    return 0;
}

void usage(char *name) {
    fprintf(stderr,
        "Create a msg_to_host_that requests the list of sticky files\n"
        "associated with the project\n"
        "Run this in the project root dir\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  --host_id                       number of host to upload from\n"
        "                                  or 'all' or '0' if for all active hosts\n"
        "  [ -v | --version ]              show version information\n"
        "  [ -h | --help ]                 show this help text\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    int host_id;

    host_id = 0;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "host_id")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            if (!strcmp(argv[i], "all")) {
                host_id = 0;
            } else {
                host_id = atoi(argv[i]);
            }
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            fprintf(stderr, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    if (host_id == 0) {
        fprintf(stderr,
            "request_file_list: bad command line, requires a valid host_id\n"
        );
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %s\n", boincerror(retval));
        exit(1);
    }

    if (host_id == 0) {
        retval = request_files_from_all();
    } else {
        retval = request_file_list(host_id);
    }

    boinc_db.close();
    return retval;
}

const char *BOINC_RCSID_41d8f8c3fa = "$Id$";
