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

class PROXY_INFO {
public:
    PROXY_INFO();
    ~PROXY_INFO();

    bool use_http_proxy;
    bool use_http_auth;
    std::string http_server_name;
    int  http_server_port;
    std::string http_user_name;
    std::string http_user_passwd;

    bool use_socks_proxy;
    int  socks_version;
    std::string socks_server_name;
    int  socks_server_port;
    std::string socks5_user_name;
    std::string socks5_user_passwd;

    std::string noproxy_hosts;

    int  autodetect_server_protocol;
    std::string autodetect_server_name;
    int  autodetect_server_port;
    bool autodetect_proxy_settings;
#ifdef _WIN32
    HANDLE autodetect_mutex;
#endif
    void detect_autoproxy_settings() { autodetect_proxy_settings = true; }
    bool should_detect_autoproxy_settings() { return autodetect_proxy_settings; }
    bool get_autoproxy_settings(int& server_protocol, std::string& server_name, int& server_port);
    bool set_autoproxy_settings(int server_protocol, std::string server_name, int server_port);

    int parse(MIOFILE&);
    int write(MIOFILE&);
    void clear();

    PROXY_INFO& operator=(const PROXY_INFO&);
};

#endif

