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

// dir_hier_path filename
//
// Run this in a project's root directory.
// Prints the absolute path of the file in the download hierarchy

#include <stdio.h>

#include "util.h"
#include "sched_config.h"
#include "sched_util.h"

int main(int /*argc*/, char** argv) {
    SCHED_CONFIG config;
    char path[256];
    int retval;

    retval = config.parse_file(".");
    if (retval) {
        fprintf(stderr, "Can't find config.xml; run this in project root dir\n");
        exit(1);
    }

    dir_hier_path(argv[1], "", config.uldl_dir_fanout, true, path);
    printf("%s%s\n", config.download_dir, path);
}

const char *BOINC_RCSID_c683969ea8 = "$Id$";
