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

#include "client_state.h"
#include "file_names.h"
#include "parse.h"
#include "util.h"

#include "acct_setup.h"

void ACCOUNT_IN::parse(char* buf) {
    url = "";
    email_addr = "";
    passwd_hash = "";
    user_name = "";

    parse_str(buf, "<url>", url);
    parse_str(buf, "<email_addr>", email_addr);
    parse_str(buf, "<passwd_hash>", passwd_hash);
    parse_str(buf, "<user_name>", user_name);
    canonicalize_master_url(url);
}

int GET_PROJECT_CONFIG_OP::do_rpc(string master_url) {
    int retval;
    string url = master_url + "get_project_config.php";
    printf("doing RPC to %s, file %s\n", url.c_str(), GET_PROJECT_CONFIG_FILENAME);
    retval = gstate.gui_http.do_rpc(this, url, GET_PROJECT_CONFIG_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void GET_PROJECT_CONFIG_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        FILE* f = fopen(GET_PROJECT_CONFIG_FILENAME, "r");
        if (f) {
            file_to_str(f, reply);
            fclose(f);
            error_num = 0;
        } else {
            error_num = ERR_FOPEN;
        }
    }
}

int LOOKUP_ACCOUNT_OP::do_rpc(ACCOUNT_IN& ai) {
    int retval;
    string url;

    url = ai.url + "/lookup_account.php?email_addr="+ai.email_addr+"&passwd_hash="+ai.passwd_hash;
    retval = gstate.gui_http.do_rpc(this, url, LOOKUP_ACCOUNT_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void LOOKUP_ACCOUNT_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        FILE* f = fopen(LOOKUP_ACCOUNT_FILENAME, "r");
        if (f) {
            file_to_str(f, reply);
            fclose(f);
            error_num = 0;
        } else {
            error_num = ERR_FOPEN;
        }
    }
}

int CREATE_ACCOUNT_OP::do_rpc(ACCOUNT_IN& ai) {
    int retval;
    string url;

    url = ai.url + "/create_account.php?email_addr="+ai.email_addr+"&passwd_hash="+ai.passwd_hash+"&user_name="+ai.user_name;
    retval = gstate.gui_http.do_rpc(this, url, CREATE_ACCOUNT_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void CREATE_ACCOUNT_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        FILE* f = fopen(CREATE_ACCOUNT_FILENAME, "r");
        if (f) {
            file_to_str(f, reply);
            fclose(f);
            error_num = 0;
        } else {
            error_num = ERR_FOPEN;
        }
    }
}

int LOOKUP_WEBSITE_OP::do_rpc(string& url) {
    int retval;

    retval = gstate.gui_http.do_rpc(this, url, LOOKUP_WEBSITE_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void LOOKUP_WEBSITE_OP::handle_reply(int http_op_retval) {
    error_num = http_op_retval;
}
const char *BOINC_RCSID_84df3fc17e="$Id$";
