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
// Revision 1.5  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#ifndef _BASEWINDOWVIEW_H_
#define _BASEWINDOWVIEW_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BaseWindowView.cpp"
#endif


class CBaseWindowView : public wxPanel
{
    DECLARE_DYNAMIC_CLASS(CBaseWindowView)

public:
    CBaseWindowView(void);
    CBaseWindowView(wxNotebook* pNotebook);

    ~CBaseWindowView(void);

    virtual wxString GetViewName(void);
    virtual char**   GetViewIcon(void);
};


#endif

