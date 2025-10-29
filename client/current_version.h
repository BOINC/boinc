// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

#ifndef BOINC_CURRENT_VERSION_H
#define BOINC_CURRENT_VERSION_H

#include "gui_http.h"

#define DEFAULT_VERSION_CHECK_URL "https://boinc.berkeley.edu/download.php?xml=1"

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

// where to get latest-version list from,
// where to download new version from, etc.
// Normally points to BOINC server but can override
// using a file nvc_config.xml
//
struct NVC_CONFIG {
    std::string client_download_url;
    std::string client_new_version_name;
        // if empty, we're using BOINC server and name is 'BOINC'
    std::string client_version_check_url;
    std::string network_test_url;

    NVC_CONFIG();
    void defaults();
    int parse(FILE*);
};

extern NVC_CONFIG nvc_config;

extern int read_nvc_config_file(void);

#endif
