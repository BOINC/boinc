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
// Revision 1.3  2004/07/13 05:56:02  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
// Revision 1.2  2004/07/12 08:46:25  rwalton
// Document parsing of the <get_state/> message
//
// Revision 1.1  2004/06/25 22:50:57  rwalton
// Client spamming server hotfix
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "Project.h"
#endif

#include "stdwx.h"
#include <wx/arrimpl.cpp>
#include "Project.h"
#include "error_numbers.h"


IMPLEMENT_DYNAMIC_CLASS(CProject, CXMLParser)
WX_DEFINE_OBJARRAY(CArrayProject);


CProject::CProject()
{
    master_url = _T("");
    resource_share = 0.0;
    project_name = _T("");
    user_name = _T("");
    team_name = _T("");
    user_total_credit = 0.0;
    user_expavg_credit = 0.0;
    host_total_credit = 0.0;
    host_expavg_credit = 0.0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
    tentative = false;
}


CProject::~CProject()
{
}


wxInt32 CProject::Parse(wxTextInputStream* input)
{
    wxString buf;
    while (buf = input->ReadLine()) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<master_url>", master_url)) continue;
        else if (parse_double(buf, "<resource_share>", resource_share)) continue;
        else if (parse_str(buf, "<project_name>", project_name)) continue;
        else if (parse_str(buf, "<user_name>", user_name)) continue;
        else if (parse_str(buf, "<team_name>", team_name)) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        else if (parse_int(buf, "<min_rpc_time>", min_rpc_time)) continue;
        else if (match_tag(buf, "<master_url_fetch_pending>")) {
            master_url_fetch_pending = true;
            continue;
        }
        else if (match_tag(buf, "<sched_rpc_pending>")) {
            sched_rpc_pending = true;
            continue;
        }
        else if (match_tag(buf, "<tentative>")) {
            tentative = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

