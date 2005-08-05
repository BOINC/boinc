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

#ifndef _ACCT_SETUP_H_
#define _ACCT_SETUP_H_

using std::string;

#include "gui_http.h"

struct GET_PROJECT_CONFIG_OP: public GUI_HTTP_OP {
    int do_rpc(string url);
    virtual void handle_reply(int http_op_retval);
    string reply;
    bool in_progress;

    GET_PROJECT_CONFIG_OP(){in_progress = false;}
    ~GET_PROJECT_CONFIG_OP(){}
};

struct ACCOUNT_IN {
    string url;
    string email_addr;
    string user_name;
    string passwd_hash;

    void parse(char*);
};

struct LOOKUP_ACCOUNT_OP: public GUI_HTTP_OP {
    string reply;
    bool in_progress;

    int do_rpc(ACCOUNT_IN&);
    virtual void handle_reply(int http_op_retval);
    LOOKUP_ACCOUNT_OP(){in_progress = false;}
};

struct CREATE_ACCOUNT_OP: public GUI_HTTP_OP {
    string reply;
    bool in_progress;

    int do_rpc(ACCOUNT_IN&);
    virtual void handle_reply(int http_op_retval);
    CREATE_ACCOUNT_OP(){in_progress = false;}
};

#endif