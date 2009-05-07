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

#include "boinc_db.h"
#include "str_util.h"

#include "sched_config.h"
#include "sched_util.h"

void usage(char** argv) {
    fprintf(stderr,
        "Arrange to delete a file from a host.\n\n"
        "Usage: %s -host_id H -file_name F\n",
        argv[0]
    );
    exit(0);
}

int delete_host_file(int host_id, const char* file_name) {
    DB_MSG_TO_HOST mth;
    int retval;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    mth.handled = false;
    sprintf(mth.xml, "<delete_file_info>%s</delete_file_info>\n", file_name);
    sprintf(mth.variety, "delete_file");
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %d\n", retval);
        return retval;
    }
    return 0;
}

int main(int argc, char** argv) {
    int i, retval;
    char file_name[256];
    int host_id;

    host_id = 0;
    strcpy(file_name, "");

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-host_id")) {
            host_id = atoi(argv[++i]);
        } else if(!strcmp(argv[i], "-file_name")) {
            strcpy(file_name, argv[++i]);
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(argv);
            exit(0);
        } else {
            fprintf(stderr, "bad arg %s\n", argv[i]);
            usage(argv);
        }
    }

    if (!strlen(file_name) || host_id == 0) {
        usage(argv);
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
        fprintf(stderr, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    retval = delete_host_file(host_id, file_name);
    boinc_db.close();
    return retval;
}

const char *BOINC_RCSID_f6337b04b0 = "$Id$";
