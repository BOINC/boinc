// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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

#include "parse.h"
#include "error_numbers.h"
#include "filesys.h"

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
}

int AUTO_UPDATE::parse(MIOFILE& in) {
    char buf[256];
    int retval;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</auto_update>")) {
            return 0;
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
        "<boinc_update>\n"
    );
    version.write(out);
    for (unsigned int i=0; i<file_refs.size(); i++) {
        file_refs[i].write(out);
    }
    out.printf(
        "</boinc_update>\n"
    );
}

// a scheduler reply included an <auto_update>.  Deal with it.
//
void AUTO_UPDATE::handle_in_reply(PROJECT* proj) {
    char dir[256], buf[256];
    int retval;
	unsigned int i;
	FILE_INFO* fip;

    if (gstate.auto_update.present) {
        if (!version.greater_than(gstate.auto_update.version)) {
            msg_printf(NULL, MSG_INFO,
                "Got request to updated to %d.%d.%d; already updating to %d.%d.%d",
                version.major, version.minor, version.release,
                gstate.auto_update.version.major,
                gstate.auto_update.version.minor,
                gstate.auto_update.version.release
            );
            return;
        }
        // TODO: abort in-progress update (and delete files) here
    }
    project = proj;

    for (i=0; i<file_refs.size(); i++) {
        FILE_REF& fref = file_refs[i];
        fip = gstate.lookup_file_info(project, fref.file_name);
		if (!fip) {
			msg_printf(project, MSG_ERROR, "missing update file %s", fref.file_name);
			return;
		}
		fref.file_info = fip;
		fip->is_auto_update_file = true;
	}

    // create version directory
    //
    boinc_version_dir(*project, version, dir);
    retval = boinc_mkdir(dir);
	if (retval) {
		msg_printf(project, MSG_ERROR, "Couldn't make version dir %s", dir);
		return;
	}
    gstate.auto_update = *this;
}

void AUTO_UPDATE::install() {
    msg_printf(NULL, MSG_INFO, "Installing new version: %d.%d.%d",
        version.major, version.minor, version.release
    );
}

void AUTO_UPDATE::poll() {
    if (!present) return;
    static double last_time = 0;

    if (gstate.now - last_time < 10) return;
    last_time = gstate.now;

    for (unsigned int i=0; i<file_refs.size(); i++) {
        FILE_REF& fref = file_refs[i];
        FILE_INFO* fip = fref.file_info;
        if (fip->status != FILE_PRESENT) return;
    }
    install();
}

int VERSION_INFO::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</version>")) return 0;
        if (parse_int(buf, "<major>", major)) continue;
        if (parse_int(buf, "<minor>", minor)) continue;
        if (parse_int(buf, "<release>", release)) continue;
    }
    return ERR_XML_PARSE;
}

void VERSION_INFO::write(MIOFILE& out) {
    out.printf(
        "<version>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <release>%d</release>\n"
        "</version>\n",
        major, minor, release
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
