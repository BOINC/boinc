// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

// Stuff related to the mechanism where the client fetches
// http://boinc.berkeley.edu/download.php?xml=1
// every so often to see if there's a newer client version

#include "filesys.h"

#include "client_msgs.h"
#include "client_state.h"
#include "log_flags.h"

#include "current_version.h"

int GET_CURRENT_VERSION_OP::do_rpc() {
    int retval;

    retval = gui_http->do_rpc(
        this, config.client_version_check_url.c_str(),
        GET_CURRENT_VERSION_FILENAME,
        true
    );
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

static bool is_version_newer(const char* p) {
    int maj=0, min=0, rel=0;

    sscanf(p, "%d.%d.%d", &maj, &min, &rel);
    if (maj > gstate.core_client_version.major) return true;
    if (maj < gstate.core_client_version.major) return false;
    if (min > gstate.core_client_version.minor) return true;
    if (min < gstate.core_client_version.minor) return false;
    if (rel > gstate.core_client_version.release) return true;
    return false;
}

// Parse the output of download.php?xml=1.
// If there is a newer version for our primary platform,
// copy it to new_version and return true.
//
static bool parse_version(FILE* f, char* new_version) {
    char buf2[256];
    bool same_platform = false, newer_version_exists = false;

    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);

    while (!xp.get_tag()) {
        if (xp.match_tag("/version")) {
            return (same_platform && newer_version_exists);
        }
        if (xp.parse_str("dbplatform", buf2, sizeof(buf2))) {
            same_platform = (strcmp(buf2, gstate.get_primary_platform())==0);
        }
        if (xp.parse_str("version_num", buf2, sizeof(buf2))) {
            newer_version_exists = is_version_newer(buf2);
            strcpy(new_version, buf2);
        }
    }
    return false;
}

static void show_newer_version_msg(const char* new_vers) {
    msg_printf_notice(0, true,
        "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=download",
        "%s (%s) <a href=%s>%s</a>",
        _("A new version of BOINC is available."),
        new_vers,
        config.client_download_url.c_str(),
        _("Download")
    );
}

void GET_CURRENT_VERSION_OP::handle_reply(int http_op_retval) {
    char buf[256], new_version[256];
    if (http_op_retval) {
        error_num = http_op_retval;
        return;
    }
    gstate.new_version_check_time = gstate.now;
    FILE* f = boinc_fopen(GET_CURRENT_VERSION_FILENAME, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<version>")) {
            if (parse_version(f, new_version)) {
                show_newer_version_msg(new_version);
                gstate.newer_version = string(new_version);
                break;
            }
        }
    }
    fclose(f);
}

// called at startup to see if the client state file
// says there's a new version
//
void newer_version_startup_check() {
    if (!gstate.newer_version.empty()) {
        if (is_version_newer(gstate.newer_version.c_str())) {
            show_newer_version_msg(gstate.newer_version.c_str());
        } else {
            gstate.newer_version = "";
        }
    }
}

#define NEW_VERSION_CHECK_PERIOD (14*86400)

void CLIENT_STATE::new_version_check() {
    if ((new_version_check_time == 0) ||
        (now - new_version_check_time > NEW_VERSION_CHECK_PERIOD)) {
            // get_current_version_op.handle_reply()
            // updates new_version_check_time
            //
            get_current_version_op.do_rpc();
        }
}

