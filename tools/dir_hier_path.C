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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// dir_hier_path filename
//
// Run this in a project's root directory.
// Prints the absolute path of the file in the download hierarchy,
// and creates the directory if needed.

#include "config.h"
#include <stdio.h>

#include "util.h"
#include "sched_config.h"
#include "sched_util.h"


const char *usage = 
"\nUsage: dir_hier_path <filename>\n"
"   Run this in a project's root directory.\n"
"   Prints the absolute path of the file in the download hierarchy,\n"
"   and creates the directory if needed.\n\n";

int main(int argc, char** argv) {
    SCHED_CONFIG config;
    char path[256];
    int retval;

    if ( (argc == 1) ||  !strcmp(argv[1], "-h")  || !strcmp(argv[1],"--help") || (argc != 2) ) {
      printf (usage);
      exit(1);
    }

    retval = config.parse_file(".");
    if (retval) {
        fprintf(stderr, "Can't find config.xml; run this in project root dir\n");
        exit(1);
    }

    retval = dir_hier_path(
        argv[1], config.download_dir, config.uldl_dir_fanout, path, true
    );
    if (retval) {
        fprintf(stderr, "dir_hier_path(): %d\n", retval);
        exit(1);
    }
    printf("%s\n", path);
}

const char *BOINC_RCSID_c683969ea8 = "$Id$";
