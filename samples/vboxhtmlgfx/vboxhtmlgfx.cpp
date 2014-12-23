// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <unistd.h>
#endif


#include "version.h"
#include "boinc_api.h"
#include "diagnostics.h"
#include "vboxlogging.h"


extern int run(int, char**);

int main(int argc, char** argv) {
    int retval = 0;

    // Initialize diagnostic system
    //
    boinc_init_graphics_diagnostics(BOINC_DIAG_DEFAULTS);

    // Log banner
    //
    vboxlog_msg("vboxhtmlgfx (%d.%d.%d): starting", BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, VBOXWRAPPER_RELEASE);

    retval = run(argc, argv);

    boinc_finish_diag();
    return retval;
}

