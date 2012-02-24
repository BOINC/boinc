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

// delete_file         [-host_id host_id -file_name file_name]
// -host_id            number of host to delete file from
// -file_name          name of the file to delete
//
// Create a msg_to_host_that requests that the host delete the
// given file from the client
//
// Run from the project root dir

#include "config.h"
#include <ctime>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string>
#include <iostream>

#include "backend_lib.h"
#include "boinc_db.h"
#include "str_util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"

void usage(char* name) {
    fprintf(stderr,
        "Usage: delete_file [options] : delete a file from a host \n\n"
        "Options:\n"
        "  --file_name F                 file name\n"
        "  --host_id H                   host DB ID\n"
        "  [-h | --help]                 Show this help text.\n"
        "  [-v | --version]              Show version information.\n"
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char file_name[256];
    int host_id;

    host_id = 0;
    strcpy(file_name, "");

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "host_id")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            host_id = atoi(argv[i]);
        } else if (is_arg(argv[i], "file_name")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            strcpy(file_name, argv[i]);
        } else if (is_arg(argv[i], "help") || is_arg(argv[i], "h")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "version") || is_arg(argv[i], "v")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            fprintf(stderr, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    if (!strlen(file_name) || host_id == 0) {
        usage(argv[0]);
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

    retval = create_delete_file_msg(host_id, file_name);
    boinc_db.close();
    return retval;
}

const char *BOINC_RCSID_f6337b04b0 = "$Id$";
