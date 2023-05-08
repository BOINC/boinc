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

DEPRECATED - we're not going to do auto-update

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstring>
#endif

#include "parse.h"
#include "error_numbers.h"
#include "filesys.h"
#include "util.h"

#include "client_msgs.h"
#include "file_names.h"
#include "client_state.h"
#include "log_flags.h"

#include "auto_update.h"

// while an update is being downloaded,
// it's treated like other project files,
// except that a version directory is prepended to filenames.
//
// When the download is complete,
// the version directory is moved to the top level dir.

AUTO_UPDATE::AUTO_UPDATE() {
    present = false;
    install_failed = false;
}

void AUTO_UPDATE::init() {
    if (!present) return;

    // if we (the core client) were run by the updater,
    // and we're not the same as the new version,
    // the install must have failed.
    // Mark it as such so we don't try it again.
    //
    if (version.greater_than(gstate.core_client_version)) {
        if (gstate.run_by_updater) {
            install_failed = true;
            msg_printf(0, MSG_INTERNAL_ERROR,
                "Client auto-update failed; keeping existing version"
            );
            gstate.set_client_state_dirty("auto update init");
        }
    }
}

int AUTO_UPDATE::parse(MIOFILE& in) {
    char buf[256];
    int retval;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</auto_update>")) {
            return 0;
        } else if (parse_bool(buf, "install_failed", install_failed)) {
            continue;
        } else if (match_tag(buf, "<version>")) {
            version.parse(in);
        } else if (match_tag(buf, "<file_ref>")) {
            FILE_REF fref;
            retval = fref.parse(in);
            if (retval) return retval;
            file_refs.push_back(fref);
        } else {
            if (log_flags.unparsed_xml) {
                msg_printf(NULL, MSG_INFO, "unparsed in boinc_update: %s", buf);
            }
        }
    }
    return ERR_XML_PARSE;
}

void AUTO_UPDATE::write(MIOFILE& out) {
    out.printf(
        "<auto_update>\n"
    );
    version.write(out);
    if (install_failed) {
        out.printf("<install_failed>1</install_failed>\n");
    }
    for (unsigned int i=0; i<file_refs.size(); i++) {
        file_refs[i].write(out);
    }
    out.printf(
        "</auto_update>\n"
    );
}

// Check whether this <auto_update> is valid,
// and if so link it up.
//
int AUTO_UPDATE::validate_and_link(PROJECT* proj) {
    char dir[256];
    int retval;
    unsigned int i;
    FILE_INFO* fip;

    if (!version.greater_than(gstate.core_client_version)) {
        msg_printf(NULL, MSG_INFO,
            "Got request to update to %d.%d.%d; already running %d.%d.%d",
            version.major, version.minor, version.release,
            gstate.core_client_version.major,
            gstate.core_client_version.minor,
            gstate.core_client_version.release
        );
        return ERR_INVALID_PARAM;
    }
    if (gstate.auto_update.present) {
        if (!version.greater_than(gstate.auto_update.version)) {
            msg_printf(NULL, MSG_INFO,
                "Got request to update to %d.%d.%d; already updating to %d.%d.%d",
                version.major, version.minor, version.release,
                gstate.auto_update.version.major,
                gstate.auto_update.version.minor,
                gstate.auto_update.version.release
            );
            return ERR_INVALID_PARAM;
        }
        // TODO: abort in-progress update (and delete files) here
    }
    project = proj;

    int nmain = 0;
    for (i=0; i<file_refs.size(); i++) {
        FILE_REF& fref = file_refs[i];
        fip = gstate.lookup_file_info(project, fref.file_name);
        if (!fip) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "missing update file %s", fref.file_name
            );
            return ERR_INVALID_PARAM;
        }
        fref.file_info = fip;
        fip->is_auto_update_file = true;
        if (fref.main_program) nmain++;
    }

    if (nmain != 1) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "Auto update has %d main programs", nmain
        );
        return ERR_INVALID_PARAM;
    }
    // create version directory
    //
    boinc_version_dir(*project, version, dir);
    retval = boinc_mkdir(dir);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR, "Couldn't make version dir %s", dir);
        return retval;
    }
    gstate.auto_update = *this;
    return 0;
}

void AUTO_UPDATE::install() {
    unsigned int i;
    FILE_INFO* fip=0;
    char version_dir[1024];
    char cwd[256];
    char *argv[10];
    int retval, argc;
#ifdef _WIN32
    HANDLE pid;
#else
    int pid;
#endif

    msg_printf(NULL, MSG_INFO, "Installing new version of BOINC: %d.%d.%d",
        version.major, version.minor, version.release
    );
    for (i=0; i<file_refs.size(); i++) {
        FILE_REF& fref = file_refs[i];
        if (fref.main_program) {
            fip = fref.file_info;
            break;
        }
    }
    if (!fip) {
        msg_printf(NULL, MSG_INTERNAL_ERROR, "Main program not found");
        return;
    }
    boinc_version_dir(*project, version, version_dir);
    boinc_getcwd(cwd);
    argv[0] = fip->name;
    argv[1] = "--install_dir";
    argv[2] = cwd;
    argv[3] = "--run_core";
    argv[4] = 0;
    argc = 4;
    retval = run_program(version_dir, fip->name, argc, argv, 5, pid);
    if (retval) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Couldn't launch updater; staying with current version"
        );
        install_failed = true;
        gstate.set_client_state_dirty("auto update install");
    } else {
        gstate.requested_exit = true;
    }
}

// When an update is ready to install, we may need to wait a little:
// 1) If there's a GUI RPC connection from a local screensaver
//    (i.e. that has done a get_screensaver_mode())
//    wait for the next get_screensaver_mode() and send it SS_STATUS_QUIT
// 2) If there's a GUI RPC connection from a GUI
//    (i.e. that has done a get_cc_status())
//    wait for the next get_cc_status() and return manager_must_quit = true.
// Wait an additional 10 seconds in any case

void AUTO_UPDATE::poll() {
#ifndef SIM
    if (!present) return;
    static double last_time = 0;
    static bool ready_to_install = false;
    static double quits_sent = 0;

    if (install_failed) return;
    if (gstate.now - last_time < 10) return;
    last_time = gstate.now;

    if (ready_to_install) {
        if (quits_sent) {
            if (gstate.now - quits_sent >= 10) {
                install();
            }
        } else {
            if (gstate.gui_rpcs.quits_sent()) {
                quits_sent = gstate.now;
            }
        }
    } else {
        for (unsigned int i=0; i<file_refs.size(); i++) {
            FILE_REF& fref = file_refs[i];
            FILE_INFO* fip = fref.file_info;
            if (fip->status != FILE_PRESENT) return;
        }
        ready_to_install = true;
        gstate.gui_rpcs.send_quits();
    }
#endif
}

int VERSION_INFO::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</version>")) return 0;
        if (parse_int(buf, "<major>", major)) continue;
        if (parse_int(buf, "<minor>", minor)) continue;
        if (parse_int(buf, "<release>", release)) continue;
        if (parse_bool(buf, "<prerelease>", prerelease)) continue;
    }
    return ERR_XML_PARSE;
}

void VERSION_INFO::write(MIOFILE& out) {
    out.printf(
        "<version>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <release>%d</release>\n"
        "   <prerelease>%d</prerelease>\n"
        "</version>\n",
        major, minor, release, prerelease
    );
}

bool VERSION_INFO::greater_than(VERSION_INFO& vi) {
    if (major > vi.major) return true;
    if (major < vi.major) return false;
    if (minor > vi.minor) return true;
    if (minor < vi.minor) return false;
    if (release > vi.release) return true;
    return false;
}
