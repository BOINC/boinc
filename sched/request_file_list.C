static volatile const char *BOINCrcsid="$Id$";
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
// request_file_list [-host_id host_id]
// -host_id            number of host to upload from
//                     or 'all' if for all active hosts
//
// Create a msg_to_host_that requests the list of permanant files
// associated with the project

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "boinc_db.h"
#include "sched_config.h"
#include "sched_util.h"

SCHED_CONFIG config;

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
        fprintf(stderr, "msg_to_host.insert(): %d\n", retval);
        return retval;
    }
    return 0;
}

int request_files_from_all() {
    DB_HOST host;
    while(!host.enumerate()) {
        request_file_list(host.get_id());
    }
    return 0;
}

int main(int argc, char** argv) {
    int i, retval;
    int host_id;

    host_id = 0;

    check_stop_daemons();

    for(i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-host_id")) {
            if (!strcmp(argv[++i], "all")) {
                host_id = 0;
            } else {
                host_id = atoi(argv[i]);
            }
        } else {
            if (!strncmp("-",argv[i],1)) {
                fprintf(stderr, "request_file_list: bad argument '%s'\n", argv[i]);
                exit(1);
            }
        }
    }

    if (host_id == 0) {
        fprintf(stderr, "request_file_list: bad command line, requires a valid host_id\n");
        exit(1);
    }

    retval = config.parse_file("..");
    if (retval) {
        fprintf(stderr, "Can't parse config file: %d\n", retval);
        exit(1);
    }

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %d\n", retval);
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
