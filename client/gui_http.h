// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _GUI_HTTP_
#define _GUI_HTTP_

// Management of HTTP operations done in response to a GUI RPC,
// i.e. triggered by the user.
//

using std::string;

#include "http_curl.h"

#define GUI_HTTP_STATE_IDLE     0
#define GUI_HTTP_STATE_BUSY     1

struct GUI_HTTP_OP;

// A "channel" for doing HTTP ops.
// There's one of these for each GUI RPC connection,
// and one for the client itself.
//
struct GUI_HTTP {
    int state;
    GUI_HTTP_OP* gui_http_op;
    HTTP_OP http_op;

    GUI_HTTP(): state(GUI_HTTP_STATE_IDLE) {}
    int do_rpc(GUI_HTTP_OP*, string url, string output_file);
    int do_rpc_post(GUI_HTTP_OP*, string url, string input_file, string output_file);
    bool poll();
};

// base class for various types of ops
//
struct GUI_HTTP_OP {
    GUI_HTTP* gui_http;
    virtual void handle_reply(int) {}
    GUI_HTTP_OP(){}
    virtual ~GUI_HTTP_OP(){}
};

#endif
