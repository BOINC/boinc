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
// Revision 1.10  2004/05/29 00:09:41  rwalton
// *** empty log message ***
//
// Revision 1.9  2004/05/27 06:17:58  rwalton
// *** empty log message ***
//
// Revision 1.8  2004/05/24 23:50:14  rwalton
// *** empty log message ***
//
// Revision 1.7  2004/05/21 06:27:15  rwalton
// *** empty log message ***
//
// Revision 1.6  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "WorkView.h"
#endif

#include "stdwx.h"
#include "WorkView.h"

#include "res/result.xpm"


IMPLEMENT_DYNAMIC_CLASS(CWorkView, CBaseListCtrlView)


CWorkView::CWorkView()
{
    wxLogTrace("CWorkView::CWorkView - Function Begining");

    wxLogTrace("CWorkView::CWorkView - Function Ending");
}


CWorkView::CWorkView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook)
{
    wxLogTrace("CWorkView::CWorkView - Function Begining");

    InsertColumn(0, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(1, _("Application"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(2, _("Name"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(3, _("CPU time"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("To Completetion"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Report Deadline"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Status"), wxLIST_FORMAT_LEFT, -1);

    wxLogTrace("CWorkView::CWorkView - Function Ending");
}


CWorkView::~CWorkView()
{
    wxLogTrace("CWorkView::~CWorkView - Function Begining");

    wxLogTrace("CWorkView::~CWorkView - Function Ending");
}


wxString CWorkView::GetViewName()
{
    wxLogTrace("CWorkView::GetViewName - Function Begining");

    wxLogTrace("CWorkView::GetViewName - Function Ending");
    return wxString(_("Work"));
}


char** CWorkView::GetViewIcon()
{
    wxLogTrace("CWorkView::GetViewIcon - Function Begining");

    wxLogTrace("CWorkView::GetViewIcon - Function Ending");
    return result_xpm;
}


void CWorkView::OnRender(wxTimerEvent &event) {
    wxLogTrace("CWorkView::OnRender - Function Begining");
    wxLogTrace("CWorkView::OnRender - Function Ending");
}

