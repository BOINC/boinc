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

// switcher.C
//
// When run as
// switcher Path X1 ... Xn
// runs program at Path with args X1. ... Xn

#include <unistd.h>
#include <stdio.h>
#include <cerrno>
#include <sys/param.h>  // for MAXPATHLEN

int main(int argc, char** argv) {

#if 0           // For debugging only
    char	current_dir[MAXPATHLEN];

    getcwd( current_dir, sizeof(current_dir));
    fprintf(stderr, "current directory = %s\n", current_dir);
    
    for (int i=0; i<argc; i++) {
        fprintf(stderr, "switcher arg %d: %s\n", i, argv[i]);
    }
    fflush(stderr);
#endif

    execv(argv[1], argv+2);
    
    // If we got here execv failed
    fprintf(stderr, "Process creation (%s) failed: errno=%d\n", argv[1], errno);

}
