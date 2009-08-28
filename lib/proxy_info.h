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

#ifndef _PROXY_INFO_
#define _PROXY_INFO_

#include "miofile.h"

struct PROXY_INFO {
    bool use_http_proxy;
    bool use_http_auth;
    char http_server_name[256];
    int http_server_port;
    char http_user_name[256];
    char http_user_passwd[256];

    bool use_socks_proxy;
    int socks_version;
    char socks_server_name[256];
    int socks_server_port;
    char socks5_user_name[256];
    char socks5_user_passwd[256];	

    char noproxy_hosts[256];

    int autodetect_server_protocol;
    char autodetect_server_name[256];
    int autodetect_server_port;
    bool autodetect_proxy_settings;

    int parse(MIOFILE&);
    int write(MIOFILE&);
    void clear();
};

#endif

