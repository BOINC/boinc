// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014-2015 University of California
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
#include "network.h"
#include "browserlog.h"
#include "webserver.h"
#include "browser.h"


static bool s_bFullscreen;
static bool s_bDebugging;
static int  s_iWebServerPort;


bool is_htmlgfx_in_debug_mode() {
    return s_bDebugging;
}

bool is_htmlgfx_in_fullscreen_mode() {
    return s_bFullscreen;
}

int get_htmlgfx_webserver_port() {
    return s_iWebServerPort;
}


int main(int argc, char** argv) {
    int retval = 0;

    // Initialize diagnostic system
    //
    boinc_init_graphics_diagnostics(BOINC_DIAG_DEFAULTS);

    // Parse Command Line
    //
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--fullscreen")) {
            browserlog_msg("Fullscreen mode requested.");
            s_bFullscreen = true;
        }
        if (!strcmp(argv[i], "--debug")) {
            browserlog_msg("Debug mode requested.");
            s_bDebugging = true;
        }
    }
#ifdef _DEBUG
    // Debug Mode is implied
    browserlog_msg("Debug mode requested.");
    s_bDebugging = true;
#endif

    // Log banner
    //
    browserlog_msg("htmlgfx (%d.%d.%d): starting", BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, VBOXWRAPPER_RELEASE);

    // Launch HTTP Server
    //
    boinc_get_port(false, s_iWebServerPort);
    webserver_initialize();
  
    // Start UI
    //
    retval = run(argc, argv);

    // Shutdown HTTP Server
    //
    webserver_destroy();

    return retval;
}

