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

//------------------------------------
//
// delete_file         [-host_id host_id -file_name file_name]
// -host_id            number of host to upload from
//                     or 'all' if for all active hosts
// -file_name          name of the file to delete
//
// Create a msg_to_host_that requests that the host delete the
// given file from the client

#if HAVE_UNISTD_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#endif

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
