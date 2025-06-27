// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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
#include "str_replace.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"

#include "current_version.h"

// NVC_CONFIG allows branded installers to have the client check for a new
// branded client on their serve instead of checking the Berkeley server for
// a new standard (unbranded) BOINC client.
//
// If the nvc_config.xml file is absent from the BOINC Data folder, use
// default values (Berkeley server.)
// Unbranded BOINC should not have an nvc_config.xml file.
// Branded installers can create or replace this file to customize these values.
// Standard (unbranded) BOINC installers should either delete the file or
// create or replace it with one containing default values.

NVC_CONFIG nvc_config;

NVC_CONFIG::NVC_CONFIG() {
    defaults();
}

// this is called first thing by client right after CC_CONFIG::defaults()
//
void NVC_CONFIG::defaults() {
    client_download_url = "https://boinc.berkeley.edu/download.php";
    client_new_version_name.clear();
    client_version_check_url = DEFAULT_VERSION_CHECK_URL;
    network_test_url = "https://www.google.com/";
};

int NVC_CONFIG::parse(FILE* f) {
    MIOFILE mf;
    XML_PARSER xp(&mf);

    mf.init_file(f);
    if (!xp.parse_start("nvc_config")) {
        msg_printf_notice(NULL, false,
            "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s",
            _("Missing start tag in nvc_config.xml")
        );
        return ERR_XML_PARSE;
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            msg_printf_notice(NULL, false,
                "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
                "%s: %s",
                _("Unexpected text in nvc_config.xml"),
                xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/nvc_config")) {
            notices.remove_notices(NULL, REMOVE_CONFIG_MSG);
            return 0;
        }
        if (xp.parse_string("client_download_url", client_download_url)) {
            downcase_string(client_download_url);
            continue;
        }
        if (xp.parse_string("client_new_version_name", client_new_version_name)) {
            continue;
        }
        if (xp.parse_string("client_version_check_url", client_version_check_url)) {
            downcase_string(client_version_check_url);
            continue;
        }
        if (xp.parse_string("network_test_url", network_test_url)) {
            downcase_string(network_test_url);
            continue;
        }
        msg_printf_notice(NULL, false,
            "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s: <%s>",
            _("Unrecognized tag in nvc_config.xml"),
            xp.parsed_tag
        );
        xp.skip_unexpected(true, "NVC_CONFIG.parse");
    }
    msg_printf_notice(NULL, false,
        "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
        "%s",
        _("Missing end tag in nvc_config.xml")
    );
    return ERR_XML_PARSE;
}

int read_nvc_config_file() {
    nvc_config.defaults();
    FILE* f = boinc_fopen(NVC_CONFIG_FILE, "r");
    if (!f) {
        return ERR_FOPEN;
    }
    nvc_config.parse(f);
    fclose(f);
    return 0;
}

int GET_CURRENT_VERSION_OP::do_rpc() {
    int retval;

    retval = gui_http->do_rpc(
        this, nvc_config.client_version_check_url.c_str(),
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

static bool is_version_newer(const char* p, int major, int minor, int release) {
    int maj=0, min=0, rel=0;

    sscanf(p, "%d.%d.%d", &maj, &min, &rel);
    if (maj > major) return true;
    if (maj < major) return false;
    if (min > minor) return true;
    if (min < minor) return false;
    if (rel > release) return true;
    return false;
}

#ifdef __APPLE__
// Encode current MacOS version xx.yy.zz as an integer xxyyzz
static int get_current_macos_version() {
    char buf[100];
    char *p1 = NULL, *p2 = NULL;
    int vers = 0;
    FILE *f;
    buf[0] = '\0';
    f = popen("sw_vers -productVersion", "r");
    if (f) {
        fscanf(f, "%s", buf);
        pclose(f);
    }
    if (buf[0] == '\0') {
        return 0;
    }

    // Extract the major system version number
    vers = atoi(buf) * 10000;
    // Extract the minor system version number
    p1 = strchr(buf, '.');
    vers += atoi(p1+1) * 100;
    p2 = strchr(p1+1, '.');
    if (p2) {
        vers += atoi(p2+1);
    }
    return vers;
}
#endif

// Parse the output of download.php?xml=1.
// If there is a version for our primary platform
// that's newer than what we're running,
// copy the version string to new_version and return true.
//
static bool parse_version(XML_PARSER &xp, char* new_version, int len) {
    char buf2[256];
    bool same_platform = false, newer_version_exists = false;
#ifdef __APPLE__
    bool min_macos_OK = false, max_macos_OK = false;
    int val = 0;
    int macOS_version = get_current_macos_version();
#endif

    while (!xp.get_tag()) {
        if (xp.match_tag("/version")) {
#ifdef __APPLE__
            return (same_platform
                && newer_version_exists
                && min_macos_OK
                && max_macos_OK
            );
#else
            return (same_platform && newer_version_exists);
#endif
        }
        if (xp.parse_str("dbplatform", buf2, sizeof(buf2))) {
            same_platform = (strcmp(buf2, gstate.get_primary_platform())==0);
        }
        if (xp.parse_str("version_num", buf2, sizeof(buf2))) {
            newer_version_exists = is_version_newer(
                buf2,
                gstate.core_client_version.major,
                gstate.core_client_version.minor,
                gstate.core_client_version.release
            );
            strlcpy(new_version, buf2, len);
        }
#ifdef __APPLE__
        if (xp.parse_int("min_os_version", val)) {
            min_macos_OK = (val <= macOS_version);
        }
        if (xp.parse_int("max_os_version", val)) {
            max_macos_OK = (val >= macOS_version);
        }
#endif
    }
    return false;
}

static void show_newer_version_msg(const char* new_vers) {
    char buf[1024];

    if (nvc_config.client_new_version_name.empty()) {
        msg_printf_notice(0, true,
            "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=download",
            "%s (%s). <a href=%s>%s</a>",
            _("A new version of BOINC is available"),
            new_vers,
            nvc_config.client_download_url.c_str(),
            _("Download")
        );
    } else {
        snprintf(buf, sizeof(buf), _("A new version of %s is available"),
            nvc_config.client_new_version_name.c_str()
        );
        msg_printf_notice(0, true, NULL,
            "%s (%s). <a href=%s>%s</a>",
            buf,
            new_vers,
            nvc_config.client_download_url.c_str(),
            _("Download")
        );
    }
}

void GET_CURRENT_VERSION_OP::handle_reply(int http_op_retval) {
    char new_version[256], newest_version[256];
    int maj=0, min=0, rel=0;
#ifdef _WIN32
    int bbrv;
#endif

    if (http_op_retval) {
        error_num = http_op_retval;
        return;
    }
    gstate.new_version_check_time = gstate.now;
    newest_version[0] = '\0';
    FILE* f = boinc_fopen(GET_CURRENT_VERSION_FILENAME, "r");
    if (!f) return;
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    while (!xp.get_tag()) {
        if (xp.match_tag("version")) {
            if (parse_version(xp, new_version, sizeof(new_version))) {
                if (is_version_newer(new_version, maj, min, rel)) {
                    strlcpy(newest_version, new_version, sizeof(newest_version));
                    sscanf(newest_version, "%d.%d.%d", &maj, &min, &rel);
                }
            }
#ifdef _WIN32
        } else if (xp.parse_int("boinc_buda_runner_version", bbrv)) {
            gstate.latest_boinc_buda_runner_version = bbrv;
#endif
        }
    }
    fclose(f);

    if (newest_version[0]) {
        show_newer_version_msg(newest_version);
    }

#if !defined(SIM) && defined(_WIN32)
    show_wsl_messages();
#endif

    // Cache newer version number. Empty string if no newer version
    gstate.newer_version = string(newest_version);
}

// called at startup to see if the client state file
// says there's a new version. This must be called after
// read_vc_config_file()
//
void newer_version_startup_check() {
    // If version check URL has changed (perhaps due to installing a build of
    // BOINC with different branding), reset any past new version information
    //
    if (gstate.client_version_check_url != nvc_config.client_version_check_url) {
        gstate.client_version_check_url = nvc_config.client_version_check_url;
        gstate.newer_version = "";
        return;
    }

    if (!gstate.newer_version.empty()) {
        if (is_version_newer(gstate.newer_version.c_str(),
            gstate.core_client_version.major,
            gstate.core_client_version.minor,
            gstate.core_client_version.release)
        ) {
            show_newer_version_msg(gstate.newer_version.c_str());
        } else {
            gstate.newer_version = "";
        }
    }
}

#define NEW_VERSION_CHECK_PERIOD (14*86400)

// get client version info from the BOINC server if we haven't done so recently.
// Called periodically from the main loop.
// Also called with force=true for the get_newer_version() GUI RPC
//
void CLIENT_STATE::new_version_check(bool force) {
    if (force
        || (new_version_check_time == 0)
        || (now - new_version_check_time > NEW_VERSION_CHECK_PERIOD)
    ) {
        // get_current_version_op.handle_reply()
        // updates new_version_check_time
        //
        get_current_version_op.do_rpc();
    }
}

