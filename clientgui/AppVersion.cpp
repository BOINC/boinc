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
// Revision 1.1  2004/06/25 22:50:56  rwalton
// Client spamming server hotfix
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AppVersion.h"
#endif

#include "stdwx.h"
#include <wx/arrimpl.cpp>
#include "AppVersion.h"
#include "error_numbers.h"


IMPLEMENT_DYNAMIC_CLASS(CAppVersion, CXMLParser)
WX_DEFINE_OBJARRAY(CArrayAppVersion);


CAppVersion::CAppVersion(void)
{
    name = _T("");
    version_num = 0;
    app = NULL;
    project = NULL;
}


CAppVersion::~CAppVersion(void)
{
}


wxInt32 CAppVersion::Parse(wxTextInputStream* input)
{
    wxString buf;
    while (buf = input->ReadLine()) {
        if (match_tag(buf, "</app_version>")) return 0;
        else if (parse_str(buf, "<app_name>", name)) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
    }
    return ERR_XML_PARSE;
}

