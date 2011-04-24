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

// put_file [options]
// --host_id N           ID of host to send to
// --file_name name      name of file

#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string>
#include <time.h>

#include "backend_lib.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"

void usage() {
    fprintf(stderr,
        "Usage: put_file [options]\n\n"
        "Arrange to send a file to a host.\n"
        "Options:\n"
        "  --host_id id                    ID of host\n"
        "  --file_name name                name of file to send\n"
        "  [ -h | --help ]                 Show this help text.\n"
        "  [ -v | --version ]              Show version information.\n"
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char file_name[256];
    int host_id;

    strcpy(file_name, "");
    host_id = 0;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "host_id")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            host_id = atoi(argv[i]);
        } else if (is_arg(argv[i], "file_name")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            strcpy(file_name, argv[i]);
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage();
            exit(0);
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            usage();
            exit(1);
        }
    }

    if (!strlen(file_name)) {
        usage();
        exit(1);
    }
    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %s\n", boincerror(retval));
        exit(1);
    }

    retval = put_file(host_id, file_name);

    boinc_db.close();
    return retval;
}

const char *BOINC_RCSID_f3c3c4b892 = "$Id$";
