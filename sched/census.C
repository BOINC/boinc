// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// Census - create a file saying (for each HR type)
// how much RAC each HR class is getting.
// This info is used the feeder to decide how many shared-memory slots
// to devote to each HR class.

#include <stdio.h>

#include "boinc_db.h"
#include "str_util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "hr_info.h"

void show_help() {
    fprintf(stderr,
        "This program writes a file '%s' containing the distribution of\n"
        "host platforms (needed if you use homogeneous redundancy).\n"
        "This should be run ~once/day as a periodic task from config.xml.\n"
        "For more info, see http://boinc.berkeley.edu/trac/wiki/HomogeneousRedundancy",
        HR_INFO_FILENAME
    );
}

int main(int argc, char** argv) {
    HR_INFO hri;
    int retval;
    SCHED_CONFIG config;
    
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            show_help();
            exit(0);
        }
    }
    check_stop_daemons();
    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "Can't parse ../config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Can't open DB\n");
        exit(1);
    }
    boinc_db.set_isolation_level(READ_UNCOMMITTED);
    hri.init();
    hri.scan_db();
    hri.write_file();
}
