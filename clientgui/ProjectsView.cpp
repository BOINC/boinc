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
#pragma implementation "ProjectsView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ProjectsView.h"
#include "Events.h"

#include "res/proj.xpm"


IMPLEMENT_DYNAMIC_CLASS(CProjectsView, CBaseListCtrlView)

BEGIN_EVENT_TABLE(CProjectsView, CBaseListCtrlView)
    EVT_LIST_CACHE_HINT(ID_LIST_PROJECTSVIEW, CProjectsView::OnCacheHint)
END_EVENT_TABLE()


CProjectsView::CProjectsView()
{
    wxLogTrace("CProjectsView::CProjectsView - Function Begining");

    wxLogTrace("CProjectsView::CProjectsView - Function Ending");
}


CProjectsView::CProjectsView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook, ID_LIST_PROJECTSVIEW)
{
    wxLogTrace("CProjectsView::CProjectsView - Function Begining");

    m_bProcessingRenderEvent = false;

    InsertColumn(0, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(1, _("Account"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(2, _("Total Credit"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(3, _("Avg. Credit"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Resource Share"), wxLIST_FORMAT_LEFT, -1);

    wxLogTrace("CProjectsView::CProjectsView - Function Ending");
}


CProjectsView::~CProjectsView()
{
    wxLogTrace("CProjectsView::~CProjectsView - Function Begining");

    wxLogTrace("CProjectsView::~CProjectsView - Function Ending");
}


wxString CProjectsView::GetViewName()
{
    wxLogTrace("CProjectsView::GetViewName - Function Begining");

    wxLogTrace("CProjectsView::GetViewName - Function Ending");
    return wxString(_("Projects"));
}


char** CProjectsView::GetViewIcon()
{
    wxLogTrace("CProjectsView::GetViewIcon - Function Begining");

    wxLogTrace("CProjectsView::GetViewIcon - Function Ending");
    return proj_xpm;
}


void CProjectsView::OnCacheHint ( wxListEvent& event ) {
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


void CProjectsView::OnRender(wxTimerEvent &event) {
    wxLogTrace("CProjectsView::OnRender - Function Begining");

    if (!m_bProcessingRenderEvent)
    {
        wxLogTrace("CProjectsView::OnRender - Processing Render Event...");
        m_bProcessingRenderEvent = true;

        wxInt32 iProjectCount = wxGetApp().GetDocument()->GetProjectCount();
        SetItemCount(iProjectCount);

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }

    wxLogTrace("CProjectsView::OnRender - Function Ending");
}


wxString CProjectsView::OnGetItemText(long item, long column) const {
    wxString strBuffer;
    switch(column) {
        case 0:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetProjectName(item);
            break;
        case 1:
            strBuffer = wxGetApp().GetDocument()->GetProjectAccountName(item);
            break;
        case 2:
            strBuffer = wxGetApp().GetDocument()->GetProjectTotalCredit(item);
            break;
        case 3:
            strBuffer = wxGetApp().GetDocument()->GetProjectAvgCredit(item);
            break;
        case 4:
            strBuffer = wxGetApp().GetDocument()->GetProjectResourceShare(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


int CProjectsView::OnGetItemImage(long item) const {
    return -1;
}


wxListItemAttr* CProjectsView::OnGetItemAttr(long item) const {
    return NULL;
}

