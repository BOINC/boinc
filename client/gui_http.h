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

#ifndef BOINC_GUI_HTTP_H
#define BOINC_GUI_HTTP_H

// A high-level interface for client-initiated HTTP requests.

// GUI_HTTP represents a "channel" for doing a sequence of HTTP ops,
// possibly to different servers.
// There's no queuing:
// if you call do_rpc() while an op is in progress, you get an error.
// You must call poll() periodically to make things work.
//
// GUI_HTTP_OP is base class for various types of ops.
// Each instance has a pointer to a particular GUI_HTTP.
// When the op is completed or failed, its handle_reply() is called
//
// The set of GUI_HTTPs:
// - one for each GUI RPC connection
//   GUI_HTTP_OPs that use this channel:
//      GUI_RPC_CONN::get_project_config_op
//      GUI_RPC_CONN::lookup_account_op
//      GUI_RPC_CONN::create_account_op
//
// - one for the client itself
//   GUI_HTTP_OPs that use this channel:
//      CLIENT_STATE::lookup_website_op
//      CLIENT_STATE::get_current_version
//      CLIENT_STATE::get_project_list
//      CLIENT_STATE::acct_mgr_op
//   These are all "best effort": if an op is requested while
//   another is in progress, it's OK; it will be retried later.
//
// - for each project:
//   a vector of TRICKLE_UP_OPs for that project's alternative trickle URLs.
//   Each one has a pointer to a dynamically allocated GUI_HTTP.

#include "http_curl.h"

#define GUI_HTTP_STATE_IDLE     0
#define GUI_HTTP_STATE_BUSY     1

struct GUI_HTTP {
    int gui_http_state;
    struct GUI_HTTP_OP* gui_http_op;
    HTTP_OP http_op;

    GUI_HTTP(): gui_http_state(GUI_HTTP_STATE_IDLE), gui_http_op(NULL) {}
    int do_rpc(
        GUI_HTTP_OP*, const char* url, const char* output_file,
        bool is_background
    );
    int do_rpc_post(
        GUI_HTTP_OP*, char* url,
        const char* input_file, const char* output_file,
        bool is_background
    );
    int do_rpc_post_str(GUI_HTTP_OP*, char* url, char* req, int len);
    bool poll();
    inline bool is_busy() {
        return (gui_http_state == GUI_HTTP_STATE_BUSY);
    }
};

// GUI_HTTP_OP is base class for various types of ops.
// its gui_http field says what channel to use
// Derived classes override handle_reply() to handle completion,
// and provide a do_rpc() function to initiate requests.

struct GUI_HTTP_OP {
    GUI_HTTP* gui_http;
    virtual void handle_reply(int) {}
    GUI_HTTP_OP(): gui_http(NULL) {}
    virtual ~GUI_HTTP_OP(){}
};

#endif
