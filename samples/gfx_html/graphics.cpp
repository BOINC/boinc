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
#include "app_ipc.h"
#include "browserlog.h"
#include "graphics.h"


#define GRAPHICS_FILENAME "boinc_graphics.xml"


int parse_graphics(
    std::string& default_url, std::string& running_url, std::string& suspended_url, std::string& network_suspended_url, std::string& exiting_url
){
    std::string path;
    MIOFILE mf;

    boinc_resolve_filename_s(GRAPHICS_FILENAME, path);
    FILE* f = boinc_fopen(path.c_str(), "r");
    if (!f) {
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("boinc_graphics")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/boinc_graphics")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_string("default_url", default_url)) continue;
        else if (xp.parse_string("running_url", running_url)) continue;
        else if (xp.parse_string("suspended_url", suspended_url)) continue;
        else if (xp.parse_string("network_suspended_url", network_suspended_url)) continue;
        else if (xp.parse_string("exiting_url", exiting_url)) continue;
    }
    fclose(f);
    return ERR_XML_PARSE;
}
