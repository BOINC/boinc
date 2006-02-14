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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _GUI_HTTP_
#define _GUI_HTTP_

// Management of HTTP operations done in response to a GUI RPC,
// i.e. triggered by the user.
//

using std::string;

#include "http_curl.h"

// base class for various types of ops
//
struct GUI_HTTP_OP {
    virtual void handle_reply(int) {}
    GUI_HTTP_OP(){}
    virtual ~GUI_HTTP_OP(){}
};

#define GUI_HTTP_STATE_IDLE     0
#define GUI_HTTP_STATE_BUSY     1

// the manager class
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

#endif
