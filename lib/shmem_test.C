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

// test program for shmem functions

// -a       attach and sleep
// -d       destroy
// -c       create

#define KEY 0xbeefcafe

#include "config.h"
#include <cstring>
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

#include "shmem.h"

int main(int argc, char** argv) {
    void* p;
    int retval;

    if (!strcmp(argv[1], "-a")) {
        retval = attach_shmem(KEY, &p);
        sleep(60);
    } else if (!strcmp(argv[1], "-d")) {
        destroy_shmem(KEY);
    } else if (!strcmp(argv[1], "-c")) {
        create_shmem(KEY, 100, &p);
    }

    return 0;
}

const char *BOINC_RCSID_6911713ff8 = "$Id$";
