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
// Revision 1.8  2004/05/27 06:17:57  rwalton
// *** empty log message ***
//
// Revision 1.7  2004/05/21 06:27:14  rwalton
// *** empty log message ***
//
// Revision 1.6  2004/05/17 22:15:08  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BaseWindowView.h"
#endif

#include "stdwx.h"
#include "BaseWindowView.h"
#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CBaseWindowView, wxPanel)


CBaseWindowView::CBaseWindowView(void)
{
    wxLogTrace("CBaseWindowView::CBaseWindowView - Function Begining");
    wxLogTrace("CBaseWindowView::CBaseWindowView - Function Ending");
}


CBaseWindowView::CBaseWindowView(wxNotebook* pNotebook) :
    wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxTAB_TRAVERSAL, _T("wxBlankPanel"))
{
    wxLogTrace("CBaseWindowView::CBaseWindowView - Function Begining");
    wxLogTrace("CBaseWindowView::CBaseWindowView - Function Ending");
}


CBaseWindowView::~CBaseWindowView(void)
{
    wxLogTrace("CBaseWindowView::~CBaseWindowView - Function Begining");
    wxLogTrace("CBaseWindowView::~CBaseWindowView - Function Ending");
}


// The user friendly name of the view.
//   If it has not been defined by the view "Undefined" is returned.
//
wxString CBaseWindowView::GetViewName(void)
{
    wxLogTrace("CBaseWindowView::GetViewName - Function Begining");
    wxLogTrace("CBaseWindowView::GetViewName - Function Ending");
    return wxString(_T("Undefined"));
}


// The user friendly icon of the view.
//   If it has not been defined by the view the BOINC icon is returned.
//
char** CBaseWindowView::GetViewIcon(void)
{
    wxLogTrace("CBaseWindowView::GetViewIcon - Function Begining");

    wxASSERT(NULL != boinc_xpm);

    wxLogTrace("CBaseWindowView::GetViewIcon - Function Ending");
    return boinc_xpm;
}


void CBaseWindowView::OnRender (wxTimerEvent &event) {
    wxLogTrace("CBaseWindowView::OnRender - Function Begining");

    wxLogTrace("CBaseWindowView::OnRender - ***** Warning ***** Each page is supposed to have it's own OnRender");
    wxLogTrace("CBaseWindowView::OnRender -                     event handler");

    wxLogTrace("CBaseWindowView::OnRender - Function Ending");
}


bool CBaseWindowView::OnSaveState() {
    wxLogTrace("CBaseWindowView::OnSaveState - Function Begining");

    wxLogTrace("CBaseWindowView::OnSaveState - ***** Warning ***** Each page is supposed to have it's own OnSaveState");
    wxLogTrace("CBaseWindowView::OnSaveState -                     event handler");

    wxLogTrace("CBaseWindowView::OnSaveState - Function Ending");
    return true;
}


bool CBaseWindowView::OnRestoreState() {
    wxLogTrace("CBaseWindowView::OnRestoreState - Function Begining");

    wxLogTrace("CBaseWindowView::OnRestoreState - ***** Warning ***** Each page is supposed to have it's own OnRestoreState");
    wxLogTrace("CBaseWindowView::OnRestoreState -                     event handler");

    wxLogTrace("CBaseWindowView::OnRestoreState - Function Ending");
    return true;
}

