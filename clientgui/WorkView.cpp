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
// Revision 1.12  2004/08/11 23:52:13  rwalton
// *** empty log message ***
//
// Revision 1.11  2004/07/13 05:56:02  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
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
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "WorkView.h"
#include "Events.h"

#include "res/result.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETETION       5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7


IMPLEMENT_DYNAMIC_CLASS(CWorkView, CBaseListCtrlView)

BEGIN_EVENT_TABLE(CWorkView, CBaseListCtrlView)
    EVT_LIST_CACHE_HINT(ID_LIST_WORKVIEW, CWorkView::OnCacheHint)
END_EVENT_TABLE()


CWorkView::CWorkView()
{
}


CWorkView::CWorkView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook, ID_LIST_WORKVIEW)
{
    m_bProcessingRenderEvent = false;

    InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_APPLICATION, _("Application"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_NAME, _("Name"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_CPUTIME, _("CPU time"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_TOCOMPLETETION, _("To Completetion"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_REPORTDEADLINE, _("Report Deadline"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, -1);
}


CWorkView::~CWorkView()
{
}


wxString CWorkView::GetViewName()
{
    return wxString(_("Work"));
}


char** CWorkView::GetViewIcon()
{
    return result_xpm;
}


void CWorkView::OnCacheHint ( wxListEvent& event ) {
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


void CWorkView::OnRender(wxTimerEvent &event) {
    if (!m_bProcessingRenderEvent)
    {
        wxLogTrace("CWorkView::OnRender - Processing Render Event...");
        m_bProcessingRenderEvent = true;

        wxInt32 iWorkCount = wxGetApp().GetDocument()->GetWorkCount();
        SetItemCount(iWorkCount);

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


wxString CWorkView::OnGetItemText(long item, long column) const {
    wxString strBuffer;
    switch(column) {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetWorkProjectName(item);
            break;
        case COLUMN_APPLICATION:
            strBuffer = wxGetApp().GetDocument()->GetWorkApplicationName(item);
            break;
        case COLUMN_NAME:
            strBuffer = wxGetApp().GetDocument()->GetWorkName(item);
            break;
        case COLUMN_CPUTIME:
            strBuffer = wxGetApp().GetDocument()->GetWorkCPUTime(item);
            break;
        case COLUMN_PROGRESS:
            strBuffer = wxGetApp().GetDocument()->GetWorkProgress(item);
            break;
        case COLUMN_TOCOMPLETETION:
            strBuffer = wxGetApp().GetDocument()->GetWorkTimeToCompletion(item);
            break;
        case COLUMN_REPORTDEADLINE:
            strBuffer = wxGetApp().GetDocument()->GetWorkReportDeadline(item);
            break;
        case COLUMN_STATUS:
            strBuffer = wxGetApp().GetDocument()->GetWorkStatus(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


int CWorkView::OnGetItemImage(long item) const {
    return -1;
}


wxListItemAttr* CWorkView::OnGetItemAttr(long item) const {
    return NULL;
}

