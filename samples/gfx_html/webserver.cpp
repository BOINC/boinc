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
#include "browser.h"
#include "browserlog.h"
#include "mongoose.h"
#include "webapi.h"
#include "webstatic.h"
#include "webserver.h"

#if defined(_MSC_VER)
#define snprintf    _snprintf
#define getcwd      _getcwd
#endif


#define WEBSERVER_STATE_UNINIT  0
#define WEBSERVER_STATE_INIT    1
#define WEBSERVER_STATE_POLLING 2
#define WEBSERVER_STATE_EXITING 3


struct mg_server* webserver;
int  webserver_state = WEBSERVER_STATE_UNINIT;
char webserver_listening[64];
char webserver_documentroot[64];
char webserver_domain[64];


static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
    switch (ev) {
        case MG_REQUEST:
            if        (!strcmp(conn->uri, "/api/static/index.html")) { 
                return handle_static_index_html(conn); 
            } else if (!strcmp(conn->uri, "/api/static/boinc.js")) { 
                return handle_static_boinc_js(conn); 
            } else if (!strcmp(conn->uri, "/api/static/boinc.png")) { 
                return handle_static_boinc_png(conn); 
            } else if (!strcmp(conn->uri, "/api/getInitData")) { 
                return handle_get_init_data(conn); 
            } else if (!strcmp(conn->uri, "/api/getGraphicsStatus")) {
                return handle_get_graphics_status(conn);
            } else if (!strcmp(conn->uri, "/api/resetReadFlag")) {
                return handle_reset_read_flag(conn);
            } else if (boinc_file_exists(conn->uri+1)) {
                return handle_filesystem_request(conn);
            }
            return MG_FALSE;
        case MG_AUTH:
            return MG_TRUE;
        default:
            return MG_FALSE;
    }
}


static int webserver_handler() {
    int retval = 1;

    switch(webserver_state) {
        case WEBSERVER_STATE_INIT:
            webserver = mg_create_server(NULL, ev_handler);
            mg_set_option(webserver, "listening_port", webserver_listening);
            mg_set_option(webserver, "document_root", webserver_documentroot);
            mg_set_option(webserver, "enable_directory_listing", "no");
            mg_set_option(webserver, "index_files", "");
            mg_set_option(webserver, "auth_domain", webserver_domain);
            webserver_state = WEBSERVER_STATE_POLLING;
            break;
        case WEBSERVER_STATE_POLLING:
            handle_poll_server();
            mg_poll_server(webserver, 1000);
            break;
        case WEBSERVER_STATE_EXITING:
            mg_destroy_server(&webserver);
            retval = 0;
        default:
            assert(false);
    }

    return retval;
}

#ifdef _WIN32
DWORD WINAPI webserver_thread(void*) {
    while (webserver_handler()) {}
    return 0;
}
#else
static void* webserver_thread(void*) {
    while (webserver_handler()) {}
    return 0;
}
#endif

int start_webserver_thread() {
#ifdef _WIN32
    if (!CreateThread(NULL, 0, webserver_thread, 0, 0, NULL)) {
        return GetLastError();
    }
#else
    int retval = pthread_create(NULL, NULL, webserver_thread, NULL);
    if (retval) {
        return retval;
    }
#endif
    return 0;
}


int webserver_initialize() {
    snprintf(
        webserver_listening, sizeof(webserver_listening)-1,
        "127.0.0.1:%d",
        get_htmlgfx_webserver_port()
    );

    getcwd(webserver_documentroot, sizeof(webserver_documentroot)-1);

    snprintf(
        webserver_domain, sizeof(webserver_domain)-1,
        "htmlgfx"
    );

    webserver_state = WEBSERVER_STATE_INIT;
    return start_webserver_thread();
}

int webserver_destroy() {
    webserver_state = WEBSERVER_STATE_EXITING;
    return 0;
}

