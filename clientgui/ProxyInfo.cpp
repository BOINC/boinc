// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//
// $Log$
// Revision 1.2  2004/07/12 08:46:26  rwalton
// Document parsing of the <get_state/> message
//
// Revision 1.1  2004/06/25 22:50:57  rwalton
// Client spamming server hotfix
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ProxyInfo.h"
#endif

#include "stdwx.h"
#include "ProxyInfo.h"
#include "error_numbers.h"


IMPLEMENT_DYNAMIC_CLASS(CProxyInfo, CXMLParser)


CProxyInfo::CProxyInfo()
{
    use_http_proxy = false;
    use_http_authenticaton = false;
    http_server_name = _T("");
    http_server_port = 0;
    http_server_username = _T("");
    http_server_password = _T("");
    use_socks_proxy = false;
    socks_version = 0;
    socks_server_name = _T("");
    socks_server_port = 0;
    socks_server_username = _T("");
    socks_server_password = _T("");
}


CProxyInfo::~CProxyInfo()
{
}


wxInt32 CProxyInfo::Parse(wxTextInputStream* input)
{
    wxString buf;
    while (buf = input->ReadLine()) {
        if (match_tag(buf, "</proxy_info>")) return 0;
        else if (parse_int(buf, "<socks_version>", socks_version)) continue;
        else if (parse_str(buf, "<socks_server_name>", socks_server_name)) continue;
        else if (parse_int(buf, "<socks_server_port>", socks_server_port)) continue;
        else if (parse_str(buf, "<socks5_user_name>", socks_server_username)) continue;
        else if (parse_str(buf, "<socks5_user_passwd>", socks_server_password)) continue;
        else if (parse_str(buf, "<http_server_name>", http_server_name)) continue;
        else if (parse_int(buf, "<http_server_port>", http_server_port)) continue;
        else if (parse_str(buf, "<http_user_name>", http_server_username)) continue;
        else if (parse_str(buf, "<http_user_passwd>", http_server_password)) continue;
        else if (match_tag(buf, "<use_http_proxy/>")) {
            use_http_proxy = true;
            continue;
        }
        else if (match_tag(buf, "<use_socks_proxy/>")) {
            use_socks_proxy = true;
            continue;
        }
        else if (match_tag(buf, "<use_http_auth/>")) {
            use_http_authenticaton = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

