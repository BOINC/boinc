// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// support for replicated trickles

struct TRICKLE_UP_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    TRICKLE_UP_OP(GUI_HTTP* p) {
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~TRICKLE_UP_OP(){}
    int do_rpc(std::string url);
    virtual void handle_reply(int http_op_retval);
};
