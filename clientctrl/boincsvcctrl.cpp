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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#include "version.h"
#include "daemonmgt.h"

void version(){
    printf("boincsvcctrl, built from %s \n", PACKAGE_STRING );
    exit(0);
}

void usage() {
    fprintf(stderr, "\n\
usage: boincsvcctrl command\n\n\
Commands:\n\
 --start                            start the BOINC service\n\
 --stop                             stop the BOINC service\n\
 --version, -V                      show core client version\n\
"
);
    exit(1);
}

int main(int argc, char** argv) {
    int i = 1;
    int retval = 0;

    if (argc < 1) {
        usage();
    }

    if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
        usage();
    }
    if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-V")) {
        version();
    }

    if (!strcmp(argv[i], "--start")) {
        retval = !start_daemon();
    }

    if (!strcmp(argv[i], "--stop")) {
        retval = !stop_daemon();
    }

    exit(retval);
}

