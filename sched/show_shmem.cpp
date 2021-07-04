// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// show_shmem: display work_item part of shared-memory structure

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "shmem.h"
#include "sched_config.h"
#include "sched_shmem.h"
#include "str_util.h"
#include "svn_version.h"

void usage(char *name) {
    fprintf(stderr,
        "Displays the work_item part of shared-memory structure.\n\n"
        "Usage: %s [OPTION]\n\n"
        "Options:\n"
        "  [ -h | --help ]        Show this help text.\n"
        "  [ -v | --version ]     Shows version information.\n",
        name
    );
}

int main(int argc, char *argv[]) {
    SCHED_SHMEM* ssp;
    int retval;
    void* p;

    for (int c = 1; c < argc; c++) {
        std::string option(argv[c]);
        if(option == "-h" || option == "--help") {
            usage(argv[0]);
            exit(0);
        } else if(option == "-v" || option == "--version") {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            fprintf(stderr, "unknown command line argument: %s\n\n", argv[c]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        printf("Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }
    retval = attach_shmem(config.shmem_key, &p);
    if (retval) {
        printf("can't attach shmem: key %x\n", config.shmem_key);
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    ssp->show(stdout);
}
