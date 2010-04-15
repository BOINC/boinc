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

// Census - create a file saying (for each HR type)
// how much RAC each HR class is getting.
// This info is used the feeder to decide how many shared-memory slots
// to devote to each HR class.

#include <cstdio>

#include "boinc_db.h"
#include "str_util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "hr_info.h"
#include "svn_version.h"

void usage(char *name) {
    fprintf(stderr,
        "This program scans the 'host' DB table and creates two files:\n\n"
        "%s: how much RAC each HR class is getting\n"
        "    (needed if you use homogeneous redundancy).\n"
        "%s: statistics of host performance\n"
        "    (needed if you use the 'job_size_matching' scheduling option).\n\n"
        "This should be run as a periodic task (about once a day) from config.xml.\n"
        "For more info, see http://boinc.berkeley.edu/trac/wiki/HomogeneousRedundancy\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  -h --help     shows this help text.\n"
        "  -v --version  shows version information.\n",
        HR_INFO_FILENAME, PERF_INFO_FILENAME, name
    );
}

int main(int argc, char** argv) {
    HR_INFO hri;
    int retval;
    
    for (int i=1; i<argc; i++) {
        if (is_arg(argv[i], "help") || is_arg(argv[i], "h")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "version") || is_arg(argv[i], "v")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "unknown command line argument: %s\n\n", argv[i]
            );
            usage(argv[0]);
            exit(1);
        }
    }
    check_stop_daemons();
    retval = config.parse_file();
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
        log_messages.printf(MSG_CRITICAL, "Can't open DB\n");
        exit(1);
    }
    log_messages.printf(MSG_NORMAL, "Starting\n");
    boinc_db.set_isolation_level(READ_UNCOMMITTED);
    hri.init();
    hri.scan_db();
    hri.write_file();
    hri.perf_info.write_file();
    log_messages.printf(MSG_NORMAL, "Finished\n");
}
