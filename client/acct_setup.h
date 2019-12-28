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

#ifndef BOINC_ACCT_SETUP_H
#define BOINC_ACCT_SETUP_H

#include "error_numbers.h"
#include "project_list.h"
#include "gui_http.h"

struct ACCOUNT_IN {
    std::string url;
    std::string email_addr;
        // the account identifier (user name or email addr)
    std::string user_name;
        // the suggested friendly name for the user during account creation.
    std::string team_name;
    std::string passwd_hash;
    std::string server_cookie;
    bool ldap_auth;
    bool server_assigned_cookie;
    bool consented_to_terms;

    void parse(XML_PARSER&);
};

struct GET_PROJECT_CONFIG_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    GET_PROJECT_CONFIG_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~GET_PROJECT_CONFIG_OP(){}
    int do_rpc(std::string url);
    virtual void handle_reply(int http_op_retval);
};

struct LOOKUP_ACCOUNT_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    LOOKUP_ACCOUNT_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~LOOKUP_ACCOUNT_OP(){}
    int do_rpc(ACCOUNT_IN&);
    virtual void handle_reply(int http_op_retval);
};

struct CREATE_ACCOUNT_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    CREATE_ACCOUNT_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~CREATE_ACCOUNT_OP(){}
    int do_rpc(ACCOUNT_IN&, std::string rpc_client_name);
    virtual void handle_reply(int http_op_retval);
};

struct GET_PROJECT_LIST_OP: public GUI_HTTP_OP {
    int error_num;

    GET_PROJECT_LIST_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~GET_PROJECT_LIST_OP(){}
    int do_rpc();
    virtual void handle_reply(int http_op_retval);
};

struct LOOKUP_LOGIN_TOKEN_OP: public GUI_HTTP_OP {
    int error_num;
    PROJECT_LIST_ITEM* pli;

    LOOKUP_LOGIN_TOKEN_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~LOOKUP_LOGIN_TOKEN_OP(){}
    int do_rpc(PROJECT_LIST_ITEM*, int user_id, const char* login_token);
    virtual void handle_reply(int http_op_retval);
};

struct PROJECT_ATTACH {
    int error_num;
    std::vector<std::string> messages;
    PROJECT_ATTACH() {
        error_num = 0;
        messages.clear();
    }
};

#endif
