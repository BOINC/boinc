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

#ifndef _ACCT_SETUP_H_
#define _ACCT_SETUP_H_

#include "gui_http.h"
#include "error_numbers.h"

// represents the contents of project_info.xml
//
struct PROJECT_INIT {
    char url[256];
    char name[256];
    char account_key[256];

    PROJECT_INIT();
    int init();
    int remove();
    void clear();
};

struct ACCOUNT_IN {
    std::string url;
    std::string email_addr;
        // this is the account identifier.  on systems that use
        // usernames it is the username, on systems that use
        // email addresses it is an email address for the user.
    std::string user_name;
        // this is the suggested friendly name for the user
        // during account creation.
    std::string passwd_hash;

    void parse(char*);
};

struct GET_PROJECT_CONFIG_OP: public GUI_HTTP_OP {
    std::string reply;
    int error_num;

    GET_PROJECT_CONFIG_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
    virtual ~GET_PROJECT_CONFIG_OP(){}
    int do_rpc(string url);
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
    int do_rpc(ACCOUNT_IN&);
    virtual void handle_reply(int http_op_retval);
};

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

struct PROJECT_ATTACH {
    int error_num;
    std::vector<std::string> messages;
};

#endif
