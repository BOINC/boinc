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
#pragma implementation "ProjectsView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ProjectsView.h"
#include "Events.h"

#include "res/proj.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5


IMPLEMENT_DYNAMIC_CLASS(CProjectsView, CBaseListCtrlView)

BEGIN_EVENT_TABLE(CProjectsView, CBaseListCtrlView)
    EVT_LIST_CACHE_HINT(ID_LIST_PROJECTSVIEW, CProjectsView::OnCacheHint)
END_EVENT_TABLE()


CProjectsView::CProjectsView()
{
}


CProjectsView::CProjectsView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook, ID_LIST_PROJECTSVIEW)
{
    m_bProcessingRenderEvent = false;

    InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_TOTALCREDIT, _("Total Credit"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_AVGCREDIT, _("Avg. Credit"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_RESOURCESHARE, _("Resource Share"), wxLIST_FORMAT_LEFT, -1);
}


CProjectsView::~CProjectsView()
{
}


wxString CProjectsView::GetViewName()
{
    return wxString(_("Projects"));
}


char** CProjectsView::GetViewIcon()
{
    return proj_xpm;
}


void CProjectsView::OnCacheHint ( wxListEvent& event ) {
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


void CProjectsView::OnRender(wxTimerEvent &event) {
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
}


wxString CProjectsView::OnGetItemText(long item, long column) const {
    wxString strBuffer;
    switch(column) {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetProjectProjectName(item);
            break;
        case COLUMN_ACCOUNTNAME:
            strBuffer = wxGetApp().GetDocument()->GetProjectAccountName(item);
            break;
        case COLUMN_TEAMNAME:
            strBuffer = wxGetApp().GetDocument()->GetProjectTeamName(item);
            break;
        case COLUMN_TOTALCREDIT:
            strBuffer = wxGetApp().GetDocument()->GetProjectTotalCredit(item);
            break;
        case COLUMN_AVGCREDIT:
            strBuffer = wxGetApp().GetDocument()->GetProjectAvgCredit(item);
            break;
        case COLUMN_RESOURCESHARE:
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

