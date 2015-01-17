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
#include "app_ipc.h"
#include "util.h"
#include "boinc_api.h"
#include "graphics2.h"
#include "browser.h"
#include "browserlog.h"
#include "mongoose.h"
#include "vboxwrapper.h"
#include "graphics.h"
#include "webserver.h"
#include "webapi.h"


bool          g_bWebServerInitialized = false;
bool          g_bExit;
double        g_dExitTimeout;
bool          g_bVboxJob;
long          g_lRemoteDesktopPort;
long          g_lWebAPIPort;
APP_INIT_DATA g_aid;
BOINC_STATUS  g_status;
double        g_dUpdateTime;
double        g_dCPUTime;
double        g_dElapsedTime;
double        g_dFractionDone;
std::string   g_strDefaultURL;
std::string   g_strRunningURL;
std::string   g_strSuspendedURL;
std::string   g_strNetworkSuspendedURL;
std::string   g_strExitingURL;


std::string normalize_url(std::string& url) {
    std::string normalized;
    char buf[256];

    if (starts_with(url, "http://") || starts_with(url, "https://")) {
        normalized = url;
    } else {
        // Assume it is a local file
        _snprintf(buf, sizeof(buf), "http://localhost:%d/", get_htmlgfx_webserver_port());
        normalized  = buf;
        normalized += url;
    }

    return normalized;
}

int determine_exit_state(double& exit_timeout) {
    exit_timeout = g_dExitTimeout;
    return g_bExit;
}

int determine_state_url(std::string& url) {
    char buf[256];
    
    // Start out with the default URL
    url = g_strDefaultURL;

    // See if we need to override the default
    if ((g_status.abort_request || g_status.quit_request || g_status.no_heartbeat) && !g_strExitingURL.empty()) {
        url = g_strExitingURL;
    } else if (g_status.suspended && !g_strSuspendedURL.empty()) {
        url = g_strSuspendedURL;
    } else if (g_status.network_suspended && !g_strNetworkSuspendedURL.empty()) {
        url = g_strNetworkSuspendedURL;
    } else if (!g_strRunningURL.empty()) {
        url = g_strRunningURL;
    }

    // Are we running a vbox job?  If so, does it expose a webapi port number?
    if ((g_bVboxJob && g_lWebAPIPort) && (url.length() == 0)) {
        _snprintf(buf, sizeof(buf), "http://localhost:%d/", g_lWebAPIPort);
        url = buf;
    }

    // If no other URL is selected, use the default HTML page embedded within the
    // executable
    if (url.size() == 0) {
        _snprintf(buf, sizeof(buf), "http://localhost:%d/api/static/index.html", get_htmlgfx_webserver_port());
        url = buf;
    }

    return 0;
}

void handle_poll_server() {
    int retval = 0;
    BOINC_STATUS status;
    std::string strDefaultURL;
    std::string strRunningURL;
    std::string strSuspendedURL;
    std::string strNetworkSuspendedURL;
    std::string strExitingURL;
    int temp = 0;


    retval = boinc_parse_graphics_status(
        &g_dUpdateTime,
        &g_dCPUTime,
        &g_dElapsedTime,
        &g_dFractionDone,
        &status
    );
    if (!retval) {

        g_bExit = g_status.abort_request || g_status.no_heartbeat || g_status.quit_request;
        if (g_bExit && ((dtime() - g_dUpdateTime) > 5.0)) {
            g_dExitTimeout = dtime() - g_dUpdateTime - 5;
        } else {
            g_dExitTimeout = 0.0;
        }

        // Update status entries sans the reread_init_data_file entry.
        // Give the HTML Web App time to assimulate the new info
        g_status.abort_request = status.abort_request;
        g_status.no_heartbeat = status.no_heartbeat;
        g_status.quit_request = status.quit_request;
        g_status.suspended = status.suspended;
        g_status.network_suspended = status.network_suspended;

        // Check to see if vboxwrapper has logged any Web API port info or 
        // Remote Desktop port info
        //
        if (g_bVboxJob) {
            if (!g_lRemoteDesktopPort) {
                if (!parse_vbox_remote_desktop_port(temp)) {
                    g_lRemoteDesktopPort = temp;
                    browserlog_msg("Vboxwrapper remote desktop port assignment (%d).", g_lRemoteDesktopPort);
                }
            }
            if (!g_lWebAPIPort) {
                if (!parse_vbox_webapi_port(temp)) {
                    g_lWebAPIPort = temp;
                    browserlog_msg("Vboxwrapper web api port assignment (%d).", g_lWebAPIPort);
                }
            }
        }

        if (status.reread_init_data_file || !g_bWebServerInitialized)
        {
            g_bWebServerInitialized = true;

            // Notify the HTML Web application of the possible change in preferences and other
            // init data.
            browserlog_msg("Preference change detected.");
            g_status.reread_init_data_file = 1;
            g_status.max_working_set_size = 0.0;
            g_status.working_set_size = 0.0;

            // Get updated state
            //
            if (g_aid.project_preferences) {
                delete g_aid.project_preferences;
                g_aid.project_preferences = NULL;
            }
            boinc_parse_init_data_file();
            boinc_get_init_data(g_aid);

            // Check for vboxwrapper state
            //
            if (is_vbox_job())
            {
                g_bVboxJob = true;
                browserlog_msg("Vboxwrapper task detected.");
            }

            // Check for project configured state urls
            //
            if (!parse_graphics(strDefaultURL, strRunningURL, strSuspendedURL, strNetworkSuspendedURL, strExitingURL))
            {
                if (strDefaultURL.size())
                {
                    g_strDefaultURL = normalize_url(strDefaultURL);
                    browserlog_msg("Configured default_url: '%s'.", strDefaultURL.c_str());
                }
                if (strRunningURL.size())
                {
                    g_strRunningURL = normalize_url(strRunningURL);
                    browserlog_msg("Configured running_url: '%s'.", strRunningURL.c_str());
                }
                if (strSuspendedURL.size())
                {
                    g_strSuspendedURL = normalize_url(strSuspendedURL);
                    browserlog_msg("Configured suspended_url: '%s'.", strSuspendedURL.c_str());
                }
                if (strNetworkSuspendedURL.size())
                {
                    g_strNetworkSuspendedURL = normalize_url(strNetworkSuspendedURL);
                    browserlog_msg("Configured network_suspended_url: '%s'.", strNetworkSuspendedURL.c_str());
                }
                if (strExitingURL.size())
                {
                    g_strExitingURL = normalize_url(strExitingURL);
                    browserlog_msg("Configured exiting_url: '%s'.", strExitingURL.c_str());
                }
            }
        }
    }
}


//
// Web Server Restful APIs
//

int handle_get_init_data(struct mg_connection *conn) {
    std::string contents;
    read_file_string("init_data.xml", contents);

    mg_send_status(conn, 200);
    mg_send_header(conn, "Content-Type", "text/xml");
    mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate");
    mg_send_header(conn, "Access-Control-Allow-Origin", "*");
    mg_printf_data(conn, "%s", contents.c_str());
    return MG_TRUE;
}

int handle_get_graphics_status(struct mg_connection *conn) {
    mg_send_status(conn, 200);
    mg_send_header(conn, "Content-Type", "text/xml");
    mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate");
    mg_send_header(conn, "Access-Control-Allow-Origin", "*");
    mg_printf_data(
        conn,
        "<graphics_status>\n"
        "    <updated_time>%f</updated_time>\n"
        "    <cpu_time>%f</cpu_time>\n"
        "    <elapsed_time>%f</elapsed_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <no_heartbeat>%d</no_heartbeat>\n"
        "    <suspended>%d</suspended>\n"
        "    <quit_request>%d</quit_request>\n"
        "    <abort_request>%d</abort_request>\n"
        "    <network_suspended>%d</network_suspended>\n"
        "    <reread_init_data_file>%d</reread_init_data_file>\n"
        "    <exit>%d</exit>\n"
        "    <exit_timeout>%f</exit_timeout>\n"
        "    <vbox_job>%d</vbox_job>\n"
        "    <webapi_port>%d</webapi_port>\n"
        "    <remote_desktop_port>%d</remote_desktop_port>\n"
        "</graphics_status>\n",
        g_dUpdateTime,
        g_dCPUTime,
        g_dElapsedTime,
        g_dFractionDone,
        g_status.no_heartbeat,
        g_status.suspended,
        g_status.quit_request,
        g_status.abort_request,
        g_status.network_suspended,
        g_status.reread_init_data_file,
        g_bExit,
        g_dExitTimeout,
        g_bVboxJob ? 1 : 0,
        g_lWebAPIPort,
        g_lRemoteDesktopPort
    );
    return MG_TRUE;
}

int handle_reset_read_flag(struct mg_connection *conn) {
    g_status.reread_init_data_file = 0;

    mg_send_status(conn, 200);
    mg_send_header(conn, "Content-Type", "text/xml");
    mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate");
    mg_send_header(conn, "Access-Control-Allow-Origin", "*");
    mg_printf_data(conn, "<success/>\n");
    return MG_TRUE;
}

int handle_filesystem_request(struct mg_connection *conn) {
    std::string uri;
    boinc_resolve_filename_s(conn->uri+1, uri);
    mg_send_file(
        conn,
        uri.c_str(),
        "Cache-Control: max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate\r\n"
        "Access-Control-Allow-Origin: *\r\n"
    );
    return MG_MORE; 
}
