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
// Revision 1.1  2004/06/25 22:50:57  rwalton
// Client spamming server hotfix
//
//
//

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "Message.cpp"
#endif

#include "XMLParser.h"


class CMessage : public CXMLParser
{
    DECLARE_DYNAMIC_CLASS(CMessage)

private:
    wxString        project;
    wxInt32         priority;
    wxInt32         timestamp;
    wxString        body;

public:
    CMessage();
    ~CMessage();

    wxInt32         Parse(wxTextInputStream* input);

    wxString        GetProject()                    { return project; }
    wxInt32         GetPriority()                   { return priority; }
    wxInt32         GetTimestamp()                  { return timestamp; }
    wxString        GetHostname()                   { return body; }

};


#endif

