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

#include "error_numbers.h"
#include "parse.h"
#include "str_replace.h"

#include "proxy_info.h"

int PROXY_INFO::parse(XML_PARSER& xp) {
    static const PROXY_INFO x;
    *this = x;
    while (!xp.get_tag()) {
        if (xp.match_tag("/proxy_info")) {
            present = false;
            if (strlen(http_server_name)) present = true;
            if (strlen(socks_server_name)) present = true;
            return 0;
        }
        if (xp.parse_bool("use_http_proxy", use_http_proxy)) continue;
        if (xp.parse_bool("use_socks_proxy", use_socks_proxy)) continue;
        if (xp.parse_bool("use_http_auth", use_http_auth)) continue;
        if (xp.parse_str("socks_server_name", socks_server_name, sizeof(socks_server_name))) continue;
        if (xp.parse_int("socks_server_port", socks_server_port)) continue;
        if (xp.parse_str("http_server_name", http_server_name, sizeof(http_server_name))) continue;
        if (xp.parse_int("http_server_port", http_server_port)) continue;
        if (xp.parse_str("socks5_user_name", socks5_user_name,sizeof(socks5_user_name))) continue;
        if (xp.parse_str("socks5_user_passwd", socks5_user_passwd,sizeof(socks5_user_passwd))) continue;
        if (xp.parse_bool("socks5_remote_dns", socks5_remote_dns)) continue;
        if (xp.parse_str("http_user_name", http_user_name,sizeof(http_user_name))) continue;
        if (xp.parse_str("http_user_passwd", http_user_passwd,sizeof(http_user_passwd))) continue;
        if (xp.parse_str("no_proxy", noproxy_hosts, sizeof(noproxy_hosts))) continue;
        if (xp.parse_bool("no_autodetect", no_autodetect)) continue;
    }
    return ERR_XML_PARSE;
}

int PROXY_INFO::parse_config(XML_PARSER& xp) {
    int retval = parse(xp);
    if (retval) return retval;
    if (strlen(http_server_name)) use_http_proxy = true;
    if (strlen(socks_server_name)) use_socks_proxy = true;
    if (strlen(http_user_name)) use_http_auth = true;
    return 0;
}

int PROXY_INFO::write(MIOFILE& out) {
    char s5un[2048], s5up[2048], hun[2048], hup[2048];
    xml_escape(socks5_user_name, s5un, sizeof(s5un));
    xml_escape(socks5_user_passwd, s5up, sizeof(s5up));
    xml_escape(http_user_name, hun, sizeof(hun));
    xml_escape(http_user_passwd, hup, sizeof(hup));
    out.printf(
        "<proxy_info>\n"
        "%s"
        "%s"
        "%s"
        "    <socks_server_name>%s</socks_server_name>\n"
        "    <socks_server_port>%d</socks_server_port>\n"
        "    <http_server_name>%s</http_server_name>\n"
        "    <http_server_port>%d</http_server_port>\n"
        "    <socks5_user_name>%s</socks5_user_name>\n"
        "    <socks5_user_passwd>%s</socks5_user_passwd>\n"
        "    <socks5_remote_dns>%d</socks5_remote_dns>\n"
        "    <http_user_name>%s</http_user_name>\n"
        "    <http_user_passwd>%s</http_user_passwd>\n"
        "    <no_proxy>%s</no_proxy>\n"
        "    <no_autodetect>%d</no_autodetect>\n",
        use_http_proxy?"    <use_http_proxy/>\n":"",
        use_socks_proxy?"    <use_socks_proxy/>\n":"",
        use_http_auth?"    <use_http_auth/>\n":"",
        socks_server_name,
        socks_server_port,
        http_server_name,
        http_server_port,
        s5un,
        s5up,
        socks5_remote_dns?1:0,
        hun,
        hup,
        noproxy_hosts,
        no_autodetect?1:0
    );
    if (strlen(autodetect_server_name)) {
        out.printf(
            "    <autodetect_protocol>%d</autodetect_protocol>\n"
            "    <autodetect_server_name>%d</autodetect_server_name>\n"
            "    <autodetect_port>%d</autodetect_port>\n",
            autodetect_protocol,
            autodetect_server_name,
            autodetect_port
        );
    }
    out.printf(
        "</proxy_info>\n"
    );
    return 0;
}

void PROXY_INFO::clear() {
    present = false;
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_auth = false;
    autodetect_proxy_supported = false;
    safe_strcpy(socks_server_name, "");
    safe_strcpy(http_server_name, "");
    socks_server_port = 80;
    http_server_port = 80;
    safe_strcpy(socks5_user_name, "");
    safe_strcpy(socks5_user_passwd, "");
    socks5_remote_dns = false;
    safe_strcpy(http_user_name, "");
    safe_strcpy(http_user_passwd, "");
    safe_strcpy(noproxy_hosts, "");
    no_autodetect = false;
    safe_strcpy(autodetect_server_name, "");
    autodetect_proxy_supported = false;
    need_autodetect_proxy_settings = false;
    have_autodetect_proxy_settings = false;
    autodetect_port = 80;
    autodetect_protocol = 0;
}
