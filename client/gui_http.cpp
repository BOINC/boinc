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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstring>
#endif

#include "client_state.h"
#include "filesys.h"
#include "error_numbers.h"

#include "gui_http.h"

int GUI_HTTP::do_rpc(
    GUI_HTTP_OP* op, const char* url, const char* output_file, bool is_bkgd
) {
    int retval;

    // this check should be done at a higher level.
    // Do it here too just in case
    //
    if (gui_http_state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }

    boinc_delete_file(output_file);
    retval = http_op.init_get(0, url, output_file, true, 0, 0);
    if (retval) return retval;
    gstate.http_ops->insert(&http_op);
    gui_http_op = op;
    gui_http_state = GUI_HTTP_STATE_BUSY;
    http_op.is_background = is_bkgd;
    return 0;
}

int GUI_HTTP::do_rpc_post(
    GUI_HTTP_OP* op, char* url,
    const char* input_file, const char* output_file,
    bool is_bkgd
) {
    int retval;

    if (gui_http_state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }

    boinc_delete_file(output_file);
    retval = http_op.init_post(0, url, input_file, output_file);
    if (retval) return retval;
    gstate.http_ops->insert(&http_op);
    gui_http_op = op;
    gui_http_state = GUI_HTTP_STATE_BUSY;
    http_op.is_background = is_bkgd;
    return 0;
}

int GUI_HTTP::do_rpc_post_str(GUI_HTTP_OP* op, char* url, char* req_buf, int len) {
    if (gui_http_state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }
    int retval = http_op.init_post2(0, url, req_buf, len, NULL, 0);
    if (retval) return retval;
    gstate.http_ops->insert(&http_op);
    gui_http_op = op;
    gui_http_state = GUI_HTTP_STATE_BUSY;
    return 0;
}

bool GUI_HTTP::poll() {
    if (gui_http_state == GUI_HTTP_STATE_IDLE) return false;
    static double last_time=0;
    if (!gstate.clock_change && gstate.now-last_time < GUI_HTTP_POLL_PERIOD) return false;
    last_time = gstate.now;

    if (http_op.http_op_state == HTTP_STATE_DONE) {
        gstate.http_ops->remove(&http_op);
        gui_http_op->handle_reply(http_op.http_op_retval);
        gui_http_op = NULL;
        gui_http_state = GUI_HTTP_STATE_IDLE;
        return true;
    }
    return false;
}
