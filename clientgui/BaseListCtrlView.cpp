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
// Revision 1.7  2004/05/21 06:27:14  rwalton
// *** empty log message ***
//
// Revision 1.6  2004/05/17 22:15:08  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BaseListCtrlView.h"
#endif

#include "stdwx.h"
#include "BaseListCtrlView.h"
#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CBaseListCtrlView, wxListView)


CBaseListCtrlView::CBaseListCtrlView(void)
{
    wxLogTrace("CBaseListCtrlView::CBaseListCtrlView - Function Begining");
    wxLogTrace("CBaseListCtrlView::CBaseListCtrlView - Function Ending");
}


CBaseListCtrlView::CBaseListCtrlView(wxNotebook* pNotebook) :
    wxListView(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("wxListView"))
{
    wxLogTrace("CBaseListCtrlView::CBaseListCtrlView - Function Begining");
    wxLogTrace("CBaseListCtrlView::CBaseListCtrlView - Function Ending");
}


CBaseListCtrlView::~CBaseListCtrlView(void)
{
    wxLogTrace("CBaseListCtrlView::~CBaseListCtrlView - Function Begining");
    wxLogTrace("CBaseListCtrlView::~CBaseListCtrlView - Function Ending");
}


// The user friendly name of the view.
//   If it has not been defined by the view "Undefined" is returned.
//
wxString CBaseListCtrlView::GetViewName(void)
{
    wxLogTrace("CBaseListCtrlView::GetViewName - Function Begining");
    wxLogTrace("CBaseListCtrlView::GetViewName - Function Ending");
    return wxString(_T("Undefined"));
}


// The user friendly icon of the view.
//   If it has not been defined by the view the BOINC icon is returned.
//
char** CBaseListCtrlView::GetViewIcon(void)
{
    wxLogTrace("CBaseListCtrlView::GetViewIcon - Function Begining");

    wxASSERT(NULL != boinc_xpm);

    wxLogTrace("CBaseListCtrlView::GetViewIcon - Function Ending");
    return boinc_xpm;
}


void CBaseListCtrlView::OnRender (wxTimerEvent &event) {
    wxLogTrace("CBaseListCtrlView::OnRender - Function Begining");

    wxLogTrace("CBaseListCtrlView::OnRender - ***** Warning ***** Each page is supposed to have it's own OnRender");
    wxLogTrace("CBaseListCtrlView::OnRender -                     event handler");

    wxLogTrace("CBaseListCtrlView::OnRender - Function Ending");
}

