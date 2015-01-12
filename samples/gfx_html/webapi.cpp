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
#include "mongoose.h"
#include "webapi.h"
#include "webserver.h"


static const char *webapi_header = 
  "Cache-Control: max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate\r\n"
  "Access-Control-Allow-Origin: *\r\n"; 


void handle_get_init_data(struct mg_connection *conn) {
    mg_send_file(
        conn,
        "init_data.xml",
        webapi_header
    );
}

void handle_get_graphics_status(struct mg_connection *conn) {
    mg_send_file(
        conn,
        "graphics_status.xml",
        webapi_header
    );
}

void handle_log_message(struct mg_connection *conn) {
    char level[64], message[1024]; 
    mg_get_var(conn, "level", level, sizeof(level)); 
    mg_get_var(conn, "message", message, sizeof(message)); 
    browserlog_msg("Console: (%s) %s\n", level, message);
}

void handle_filesystem_request(struct mg_connection *conn) {
    std::string uri;
    boinc_resolve_filename_s(conn->uri+1, uri);
    mg_send_file(
        conn,
        uri.c_str(),
        webapi_header
    );
}

