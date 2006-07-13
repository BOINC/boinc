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

// setprojectgrp.C
//
// When run as
// setprojectgrp Path
// sets group of file at Path to boinc_project
//
// setprojectgrp runs setuid boinc_master and setgid boinc_project

#include <unistd.h>
#include <grp.h>
#include <stdio.h>
#include <cerrno>

int main(int argc, char** argv) {
    gid_t       project_gid;
    int         retval;
    
    project_gid = getegid();

#if 0           // For debugging
    fprintf(stderr, "setprojectgrp argc=%d, arg[1]= %s, boinc_project gid = %d\n", argc, argv[1], project_gid);
    fflush(stderr);
#endif

    retval = chown(argv[1], (uid_t)-1, project_gid);
    if (retval)
        fprintf(stderr, "chown(%s, -1, %d) failed: errno=%d\n", argv[1], project_gid, errno);

    return retval;
}
