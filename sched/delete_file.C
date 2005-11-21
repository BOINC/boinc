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

//------------------------------------
//
// delete_file         [-host_id host_id -file_name file_name]
// -host_id            number of host to upload from
//                     or 'all' if for all active hosts
// -file_name          name of the file to delete
//
// Create a msg_to_host_that requests that the host delete the
// given file from the client

#include "config.h"
#include <ctime>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "boinc_db.h"
#include "sched_config.h"
#include "sched_util.h"

SCHED_CONFIG config;

int delete_host_file(int host_id, const char* file_name) {
    DB_MSG_TO_HOST mth;
    sprintf(mth.xml, "<delete_file_info>%s</delete_file_info>\n", file_name);
    sprintf(mth.variety, "delete_file");
    int retval;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    mth.handled = false;
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

    // get arguments
    for(i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-host_id")) {
            if(!strcmp(argv[++i], "all")) {
                host_id = 0;
            } else {
                host_id = atoi(argv[i]);
            }
        } else if(!strcmp(argv[i], "-file_name")) {
            strcpy(file_name, argv[i++]);
        } else if (!strcmp(argv[i], "-help")) {
            fprintf(stdout,
                    "delete_file: deletes a file on a specific host\n\n"
                    "It takes the following arguments and types:\n"
                    "-hostid (int); the number of the host\n"
                    "-file_name (string); the name of the file to get\n");
            exit(0);
        } else {
            if (!strncmp("-",argv[i],1)) {
                fprintf(stderr, "request_file_list: bad argument '%s'\n", argv[i]);
                fprintf(stderr, "type get_file -help for more information\n");
                exit(1);
            }
        }
    }

    // if no arguments are given, error and exit
    if (!strlen(file_name) || host_id == 0) {
        fprintf(stderr, "request_file_list: bad command line, requires a valid host_id and file_name\n");
        exit(1);
    }

    // parse the configuration file to get database information
    retval = config.parse_file("..");
    if (retval) {
        fprintf(stderr, "Can't parse config file: %d\n", retval);
        exit(1);
    }

    // open the database
    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    // run the get file routine
    retval = delete_host_file(host_id, file_name);
    // close the database
    boinc_db.close();
    // return with error code if any
    return retval;
}

const char *BOINC_RCSID_f6337b04b0 = "$Id$";
