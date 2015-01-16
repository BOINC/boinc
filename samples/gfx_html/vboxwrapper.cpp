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

#include "parse.h"
#include "filesys.h"
#include "boinc_api.h"
#include "browserlog.h"
#include "vboxwrapper.h"


#define CHECKPOINT_FILENAME "vbox_checkpoint.xml"
#define WEBAPI_FILENAME "vbox_webapi.xml"
#define REMOTEDESKTOP_FILENAME "vbox_remote_desktop.xml"


bool is_vbox_job() {
    return 1 == boinc_file_exists(CHECKPOINT_FILENAME);
}

int parse_vbox_remote_desktop_port(int& remote_desktop_port) {
    MIOFILE mf;
    FILE* f = boinc_fopen(REMOTEDESKTOP_FILENAME, "r");
    if (!f) {
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("remote_desktop")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/remote_desktop")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_int("host_port", remote_desktop_port)) continue;
    }
    fclose(f);
    return ERR_XML_PARSE;
}

int parse_vbox_webapi_port(int& webapi_port) {
    MIOFILE mf;
    FILE* f = boinc_fopen(WEBAPI_FILENAME, "r");
    if (!f) {
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("webapi")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/webapi")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_int("host_port", webapi_port)) continue;
    }
    fclose(f);
    return ERR_XML_PARSE;
}
