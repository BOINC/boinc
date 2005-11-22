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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#endif

#include "client_state.h"
#include "filesys.h"
#include "error_numbers.h"

#include "gui_http.h"

int GUI_HTTP::do_rpc(GUI_HTTP_OP* op, string url, string output_file) {
    int retval;

    if (state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }

    http_op.set_proxy(&gstate.proxy_info);
    boinc_delete_file(output_file.c_str());
    retval = http_op.init_get(url.c_str(), output_file.c_str(), true);
    if (!retval) retval = gstate.http_ops->insert(&http_op);
    if (!retval) {
        gui_http_op = op;
        state = GUI_HTTP_STATE_BUSY;
    }
    return retval;
}

bool GUI_HTTP::poll() {
    if (state == GUI_HTTP_STATE_IDLE) return false;
    static double last_time=0;
    if (gstate.now-last_time < 1) return false;
    last_time = gstate.now;

    if (http_op.http_op_state == HTTP_STATE_DONE) {
        gstate.http_ops->remove(&http_op);
        gui_http_op->handle_reply(http_op.http_op_retval);
        gui_http_op = NULL;
        state = GUI_HTTP_STATE_IDLE;
    }
    return true;
}
const char *BOINC_RCSID_7c374a67d3="$Id$";
