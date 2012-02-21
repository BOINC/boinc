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

#ifndef _CURRENT_VERSION_
#define _CURRENT_VERSION_

#include "gui_http.h"

struct GET_CURRENT_VERSION_OP: public GUI_HTTP_OP {
    int error_num;

    GET_CURRENT_VERSION_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~GET_CURRENT_VERSION_OP(){}
    int do_rpc();
    virtual void handle_reply(int http_op_retval);
};

extern void newer_version_startup_check();

#endif
