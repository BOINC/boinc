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

class XML_PARSER;
class MIOFILE;

// info on whether HTTP requests need to go through a proxy
//
struct PROXY_INFO {
    bool present;
        // set if rest of structure is filled in

    // the following is populated if user has specified an HTTP proxy
    //
    bool use_http_proxy;
    bool use_http_auth;
    char http_server_name[256];
    int http_server_port;
    char http_user_name[256];
    char http_user_passwd[256];

    // the following is populated if user has specified a SOCKS proxy.
    // Only SOCKS 5 is supported.
    //
    bool use_socks_proxy;
    char socks_server_name[256];
    int socks_server_port;
    char socks5_user_name[256];
    char socks5_user_passwd[256];	

    // a list of hosts for which we should NOT go through a proxy
    // (e.g. a company PC attached to both local and remote projects)
    //
    char noproxy_hosts[256];

    // On Windows, if neither HTTP nor SOCKS proxy is specified,
    // we try the "autodetect" mechanism.
    // If it gets anything, the info is filled in below
    //
    bool autodetect_proxy_supported;
        // if true, some mechinism for detecting proxy servers is
        // supported by the client.
    int autodetect_protocol;
        // URL_PROTOCOL_SOCKS, URL_PROTOCOL_HTTP, or URL_PROTOCOL_HTTPS
    char autodetect_server_name[256];
    int autodetect_port;
    bool need_autodetect_proxy_settings;
        // if true, we need to detect proxy settings.
        // set to true if ref web site lookup fails
    bool have_autodetect_proxy_settings;
        // whether above fields are defined

    int parse(XML_PARSER&);
    int parse_config(XML_PARSER&);
    int write(MIOFILE&);
    void clear();
};

#endif
