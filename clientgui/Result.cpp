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
#pragma implementation "Result.h"
#endif

#include "stdwx.h"
#include <wx/arrimpl.cpp>
#include "Result.h"
#include "error_numbers.h"


IMPLEMENT_DYNAMIC_CLASS(CResult, CXMLParser)
WX_DEFINE_OBJARRAY(CArrayResult);


CResult::CResult(void)
{
    name = _T("");
    wu_name = _T("");
    report_deadline = 0;
    ready_to_report = false;
    got_server_ack = false;
    final_cpu_time = 0.0;
    state = 0;
    exit_status = 0;
    signal = 0;
    active_task_state = 0;
    stderr_out = _T("");
    app = NULL;
    wup = NULL;
    project = NULL;
}


CResult::~CResult(void)
{
}


wxInt32 CResult::Parse(wxTextInputStream* input)
{
    wxString buf;
    while (buf = input->ReadLine()) {
        if (match_tag(buf, "</result>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<wu_name>", wu_name)) continue;
        else if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        else if (match_tag(buf, "<ready_to_report>")) {
            ready_to_report = true;
            continue;
        }
        else if (match_tag(buf, "<got_server_ack>")) {
            got_server_ack = true;
            continue;
        }
        else if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        else if (parse_int(buf, "<state>", state)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (parse_int(buf, "<signal>", signal)) continue;
        else if (parse_int(buf, "<active_task_state>", active_task_state)) continue;
        else if (match_tag(buf, "<stderr_out>")) {
            if (ERR_XML_PARSE == copy_element_contents(input, "</stderr_out>", stderr_out)) {
                return ERR_XML_PARSE;
            }
        }
    }
    return ERR_XML_PARSE;
}

