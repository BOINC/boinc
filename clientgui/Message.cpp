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
// Revision 1.2  2004/07/12 08:46:25  rwalton
// Document parsing of the <get_state/> message
//
// Revision 1.1  2004/06/25 22:50:57  rwalton
// Client spamming server hotfix
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "Message.h"
#endif

#include "stdwx.h"
#include <wx/arrimpl.cpp>
#include "Message.h"
#include "error_numbers.h"


IMPLEMENT_DYNAMIC_CLASS(CMessage, CXMLParser)
WX_DEFINE_OBJARRAY(CArrayMessage);


CMessage::CMessage(void)
{
    seq_no = 0;
    project = _T("");
    priority = 0;
    timestamp = 0;
    body = _T("");
}


CMessage::~CMessage(void)
{
}


wxInt32 CMessage::Parse(wxTextInputStream* input)
{
    wxString buf;
    while (buf = input->ReadLine()) {
        if (match_tag(buf, "</msg>")) return 0;
        if (parse_str(buf, "<project>", project)) continue;
        if (match_tag(buf, "<body>" )) {
            copy_element_contents(input, "</body>", body);
            continue;
        }
        if (parse_int(buf, "<seq_no>", priority)) continue;
        if (parse_int(buf, "<pri>", priority)) continue;
        if (parse_int(buf, "<time>", timestamp)) continue;
    }
    return ERR_XML_PARSE;
}


