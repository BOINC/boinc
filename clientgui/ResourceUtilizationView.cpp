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
// Revision 1.12  2004/08/11 23:52:12  rwalton
// *** empty log message ***
//
// Revision 1.11  2004/07/13 05:56:02  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
// Revision 1.10  2004/05/29 00:09:40  rwalton
// *** empty log message ***
//
// Revision 1.9  2004/05/27 06:17:57  rwalton
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
#pragma implementation "ResourceUtilizationView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ResourceUtilizationView.h"
#include "Events.h"

#include "res/usage.xpm"


IMPLEMENT_DYNAMIC_CLASS(CResourceUtilizationView, CBaseListCtrlView)

BEGIN_EVENT_TABLE(CResourceUtilizationView, CBaseListCtrlView)
    EVT_LIST_CACHE_HINT(ID_LIST_RESOURCEUTILIZATIONVIEW, CResourceUtilizationView::OnCacheHint)
END_EVENT_TABLE()


CResourceUtilizationView::CResourceUtilizationView()
{
}


CResourceUtilizationView::CResourceUtilizationView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook, ID_LIST_RESOURCEUTILIZATIONVIEW)
{
}


CResourceUtilizationView::~CResourceUtilizationView()
{
}


wxString CResourceUtilizationView::GetViewName()
{
    return wxString(_("Disk"));
}


char** CResourceUtilizationView::GetViewIcon()
{
    return usage_xpm;
}


void CResourceUtilizationView::OnCacheHint ( wxListEvent& event ) {
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


void CResourceUtilizationView::OnRender(wxTimerEvent &event) {
    wxLogTrace("CResourceUtilizationView::OnRender - Function Begining");
    wxLogTrace("CResourceUtilizationView::OnRender - Function Ending");
}


wxString CResourceUtilizationView::OnGetItemText(long item, long column) const {
    wxString strBuffer;
    switch(column) {
        case 0:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetProjectProjectName(item);
            break;
        case 1:
            strBuffer = wxGetApp().GetDocument()->GetProjectAccountName(item);
            break;
        case 2:
            strBuffer = wxGetApp().GetDocument()->GetProjectTeamName(item);
            break;
        case 3:
            strBuffer = wxGetApp().GetDocument()->GetProjectTotalCredit(item);
            break;
        case 4:
            strBuffer = wxGetApp().GetDocument()->GetProjectAvgCredit(item);
            break;
        case 5:
            strBuffer = wxGetApp().GetDocument()->GetProjectResourceShare(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


int CResourceUtilizationView::OnGetItemImage(long item) const {
    return -1;
}


wxListItemAttr* CResourceUtilizationView::OnGetItemAttr(long item) const {
    return NULL;
}

