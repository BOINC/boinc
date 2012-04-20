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

// dir_hier_path filename
//
// Run this in a project's root directory.
// Prints the absolute path of the file in the download hierarchy,
// and creates the directory if needed.

#include "config.h"
#include <cstdio>

#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "str_util.h"


const char *usage = 
"\nUsage: dir_hier_path <filename>\n"
"   Run this in a project's root directory.\n"
"   Prints the absolute path of the file in the download hierarchy,\n"
"   and creates the directory if needed.\n\n";

int main(int argc, char** argv) {
    char path[256];
    int retval;

    if ( (argc == 1) ||  !strcmp(argv[1], "-h")  || !strcmp(argv[1],"--help") || (argc != 2) ) {
      printf (usage);
      exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = dir_hier_path(
        argv[1], config.download_dir, config.uldl_dir_fanout, path, true
    );
    if (retval) {
        fprintf(stderr, "dir_hier_path(): %s\n", boincerror(retval));
        exit(1);
    }
    printf("%s\n", path);
}

const char *BOINC_RCSID_c683969ea8 = "$Id$";
