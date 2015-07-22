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

#include "parse.h"
#include "filesys.h"
#include "boinc_api.h"
#include "vboxlogging.h"
#include "vboxcheckpoint.h"


VBOX_CHECKPOINT::VBOX_CHECKPOINT() {
    clear();
}

VBOX_CHECKPOINT::~VBOX_CHECKPOINT() {
    clear();
}

void VBOX_CHECKPOINT::clear() {
    elapsed_time = 0.0;
    cpu_time = 0.0;
    webapi_port = 0;
    remote_desktop_port = 0;
}

int VBOX_CHECKPOINT::parse() {
    MIOFILE mf;

    FILE* f = boinc_fopen(CHECKPOINT_FILENAME, "r");
    if (!f) {
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("vbox_checkpoint")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            vboxlog_msg("VBOX_CHECKPOINT::parse(): unexpected text %s", xp.parsed_tag);
            continue;
        }
        if (xp.match_tag("/vbox_checkpoint")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_double("elapsed_time", elapsed_time)) continue;
        else if (xp.parse_double("cpu_time", cpu_time)) continue;
        else if (xp.parse_int("webapi_port", webapi_port)) continue;
        else if (xp.parse_int("remote_desktop_port", remote_desktop_port)) continue;
        else {
            vboxlog_msg("VBOX_CHECKPOINT::parse(): unexpected text %s", xp.parsed_tag);
        }
    }
    fclose(f);
    return ERR_XML_PARSE;
}

int VBOX_CHECKPOINT::write() {
    MIOFILE mf;
    FILE* f;

    // Write checkpoint info to disk
    //
    f = boinc_fopen(CHECKPOINT_FILENAME, "w");
    mf.init_file(f);

    mf.printf(
        "<vbox_checkpoint>\n"
        "    <elapsed_time>%f</elapsed_time>\n"
        "    <cpu_time>%f</cpu_time>\n"
        "    <webapi_port>%d</webapi_port>\n"
        "    <remote_desktop_port>%d</remote_desktop_port>\n"
        "</vbox_checkpoint>\n",
        elapsed_time,
        cpu_time,
        webapi_port,
        remote_desktop_port
    );

    fclose(f);

    // Write webapi info to disk
    //
    if (!boinc_file_exists(WEBAPI_FILENAME)) {
        f = boinc_fopen(WEBAPI_FILENAME, "w");
        mf.init_file(f);
        mf.printf(
            "<webapi>\n"
            "  <host_port>%d</host_port>\n"
            "</webapi>\n",
            webapi_port
        );
        fclose(f);
    }

    // Write remote desktop info to disk
    //
    if (!boinc_file_exists(REMOTEDESKTOP_FILENAME)) {
        f = boinc_fopen(REMOTEDESKTOP_FILENAME, "w");
        mf.init_file(f);
        mf.printf(
            "<remote_desktop>\n"
            "  <host_port>%d</host_port>\n"
            "</remote_desktop>\n",
            remote_desktop_port
        );
        fclose(f);
    }

    return 0;
}

int VBOX_CHECKPOINT::update(double _elapsed_time, double _cpu_time) {
    elapsed_time = _elapsed_time;
    cpu_time = _cpu_time;
    return write();
}
