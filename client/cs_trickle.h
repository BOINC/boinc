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

#ifndef BOINC_CS_TRICKLE_H
#define BOINC_CS_TRICKLE_H

#include "gui_http.h"

struct TRICKLE_UP_OP: public GUI_HTTP_OP {
    std::string reply;
    std::string url;
    int error_num;
    char* req_buf;

    TRICKLE_UP_OP(std::string& u) : url(u) {
        error_num = 0;
        gui_http = new GUI_HTTP;
        req_buf = NULL;
    }
    virtual ~TRICKLE_UP_OP(){}
    int do_rpc(const char*);
    virtual void handle_reply(int);
};

extern bool trickle_up_poll();
extern int parse_trickle_up_urls(XML_PARSER&, std::vector<std::string>&);
extern void update_trickle_up_urls(PROJECT* p, std::vector<std::string> &urls);
extern void send_replicated_trickles(PROJECT* p, const char* msg, char* result_name, int t);

#endif
