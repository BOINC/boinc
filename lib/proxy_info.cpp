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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstring>
#include <string>
#endif

#include "parse.h"
#include "error_numbers.h"
#include "proxy_info.h"


PROXY_INFO::PROXY_INFO() {
    clear();
#ifdef _WIN32
    autodetect_mutex = CreateMutex(NULL, FALSE, NULL);
#endif
}

PROXY_INFO::~PROXY_INFO() {
#ifdef _WIN32
    if (autodetect_mutex) CloseHandle(autodetect_mutex);
#endif
}

bool PROXY_INFO::get_autoproxy_settings(
    int& server_protocol, std::string& server_name, int& server_port
) {
    bool  retval = false;
#ifdef _WIN32
    // Request ownership of mutex.
    DWORD result = WaitForSingleObject(autodetect_mutex, 1000);
    if (result == WAIT_OBJECT_0) {
#endif

        server_protocol = autodetect_server_protocol;
        server_name = autodetect_server_name;
        server_port = autodetect_server_port;
        retval = true;

#ifdef _WIN32
    }
    ReleaseMutex(autodetect_mutex);
#endif
    return retval; 
}

bool PROXY_INFO::set_autoproxy_settings(
    int server_protocol, std::string server_name, int server_port
) {
    bool  retval = false;
#ifdef _WIN32
    // Request ownership of mutex.
    DWORD result = WaitForSingleObject(autodetect_mutex, 1000);
    if (result == WAIT_OBJECT_0) {
#endif

        autodetect_server_protocol = server_protocol;
        autodetect_server_name = server_name;
        autodetect_server_port = server_port;
        autodetect_proxy_settings = false;
        retval = true;

#ifdef _WIN32
    }
    ReleaseMutex(autodetect_mutex);
#endif
    return retval; 
}

int PROXY_INFO::parse(MIOFILE& in) {
    char buf[2048];
    clear();
    while (in.fgets(buf, 2048)) {
        if (match_tag(buf, "</proxy_info>")) return 0;
        else if (parse_bool(buf, "use_http_proxy", use_http_proxy)) continue;
        else if (parse_bool(buf, "use_socks_proxy", use_socks_proxy)) continue;
        else if (parse_bool(buf, "use_http_auth", use_http_auth)) continue;
        else if (parse_int(buf, "<socks_version>", socks_version)) continue;
        else if (parse_str(buf, "<socks_server_name>", socks_server_name)) continue;
        else if (parse_int(buf, "<socks_server_port>", socks_server_port)) continue;
        else if (parse_str(buf, "<http_server_name>", http_server_name)) continue;
        else if (parse_int(buf, "<http_server_port>", http_server_port)) continue;
        else if (parse_str(buf, "<socks5_user_name>", socks5_user_name)) {
            xml_unescape(socks5_user_name);
            continue;
        }
        else if (parse_str(buf, "<socks5_user_passwd>", socks5_user_passwd)) {
            xml_unescape(socks5_user_passwd);
            continue;
        }
        else if (parse_str(buf, "<http_user_name>", http_user_name)) {
            xml_unescape(http_user_name);
            continue;
        }
        else if (parse_str(buf, "<http_user_passwd>", http_user_passwd)) {
            xml_unescape(http_user_passwd);
            continue;
        }
        else if (parse_str(buf, "<no_proxy>", noproxy_hosts)) continue;
    }
    return ERR_XML_PARSE;
}

int PROXY_INFO::write(MIOFILE& out) {
    char s5un[2048], s5up[2048], hun[2048], hup[2048];
    xml_escape(socks5_user_name.c_str(), s5un, sizeof(s5un));
    xml_escape(socks5_user_passwd.c_str(), s5up, sizeof(s5up));
    xml_escape(http_user_name.c_str(), hun, sizeof(hun));
    xml_escape(http_user_passwd.c_str(), hup, sizeof(hup));
    out.printf(
        "<proxy_info>\n"
        "%s"
        "%s"
        "%s"
        "    <socks_version>%d</socks_version>\n"
        "    <socks_server_name>%s</socks_server_name>\n"
        "    <socks_server_port>%d</socks_server_port>\n"
        "    <http_server_name>%s</http_server_name>\n"
        "    <http_server_port>%d</http_server_port>\n"
        "    <socks5_user_name>%s</socks5_user_name>\n"
        "    <socks5_user_passwd>%s</socks5_user_passwd>\n"
        "    <http_user_name>%s</http_user_name>\n"
        "    <http_user_passwd>%s</http_user_passwd>\n"
        "    <no_proxy>%s</no_proxy>\n"
        "</proxy_info>\n",        
        use_http_proxy?"    <use_http_proxy/>\n":"",
        use_socks_proxy?"    <use_socks_proxy/>\n":"",
        use_http_auth?"    <use_http_auth/>\n":"",
        socks_version,
        socks_server_name.c_str(),
        socks_server_port,
        http_server_name.c_str(),
        http_server_port,
        s5un,
        s5up,
        hun,
        hup,
        noproxy_hosts.c_str()
    );
    return 0;
}

void PROXY_INFO::clear() {
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_auth = false;
    socks_server_name.clear();
    http_server_name.clear();
    socks_server_port = 80;
    http_server_port = 80;
    socks5_user_name.clear();
    socks5_user_passwd.clear();
    http_user_name.clear();
    http_user_passwd.clear();
    socks_version = 0;
    autodetect_server_protocol = 0;
    autodetect_server_name.clear();
    autodetect_server_port = 80;
    noproxy_hosts.clear();
}

PROXY_INFO& PROXY_INFO::operator=(const PROXY_INFO& new_pi) {
#ifdef _WIN32
    // Request ownership of mutex.
    DWORD result = WaitForSingleObject(autodetect_mutex, 1000);
    if (result == WAIT_OBJECT_0) {
#endif
        use_http_proxy = new_pi.use_http_proxy;
        use_socks_proxy = new_pi.use_socks_proxy;
        use_http_auth = new_pi.use_http_auth;
        socks_server_name = new_pi.socks_server_name;
        http_server_name = new_pi.http_server_name;
        socks_server_port = new_pi.socks_server_port;
        http_server_port = new_pi.http_server_port;
        socks5_user_name = new_pi.socks5_user_name;
        socks5_user_passwd = new_pi.socks5_user_passwd;
        http_user_name = new_pi.http_user_name;
        http_user_passwd = new_pi.http_user_passwd;
        socks_version = new_pi.socks_version;
        autodetect_server_protocol = new_pi.autodetect_server_protocol;
        autodetect_server_name = new_pi.autodetect_server_name;
        autodetect_server_port = new_pi.autodetect_server_port;
        noproxy_hosts = new_pi.noproxy_hosts;
#ifdef _WIN32
    }
    ReleaseMutex(autodetect_mutex);
#endif
    return *this;
}

const char *BOINC_RCSID_af13db88e5 = "$Id$";
