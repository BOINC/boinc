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

// dir_hier_move src_dir dst_dir fanout
//
// move files from src_dir (flat) into dst_dir (hierarchical)
// with the given fanout

#include "config.h"
#include <cstdio>
#include <string>
#include <cstdlib>
#include <cerrno>

#include "error_numbers.h"
#include "filesys.h"
#include "util.h"
#include "sched_util.h"

const char *usage = 
"\nUsage: dir_hier_move <src_dir> <dst_dir> <fanout>\n"	
"Moves files from <src_dir> (flat) into <dst_dir> (hierarchical) with the given <fanout>\n\n";

int main(int argc, char** argv) {
    char* src_dir, *dst_dir;
    int fanout=0;
    std::string filename;
    char dst_path[MAXPATHLEN], src_path[MAXPATHLEN];
    int retval;
    
    if ( (argc == 1) || !strcmp(argv[1], "-h")  || !strcmp(argv[1],"--help") || (argc != 4) ) {
        fprintf(stderr, usage);
        exit(1);
    }
    src_dir = argv[1];
    dst_dir = argv[2];
    fanout = atoi(argv[3]);
    if (!fanout) {
        fprintf(stderr, usage);
        exit(1);
    }

    DirScanner scanner(src_dir);
    while (scanner.scan(filename)) {
        retval = dir_hier_path(filename.c_str(), dst_dir, fanout, dst_path, true);
        if (retval) {
            fprintf(stderr, "dir_hier_path: %s\n", boincerror(retval));
            exit(1);
        }
        sprintf(src_path, "%s/%s", src_dir, filename.c_str());
        retval = rename(src_path, dst_path);
        if (retval) {
            perror("rename");
            exit(1);
        }
    }
}

const char *BOINC_RCSID_d6492ba662 = "$Id$";
