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
// Revision 1.1  2004/09/21 01:26:26  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewWork.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ViewWork.h"
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


IMPLEMENT_DYNAMIC_CLASS(CViewWork, CBOINCBaseView)

BEGIN_EVENT_TABLE(CViewWork, CBOINCBaseView)
    EVT_LIST_CACHE_HINT(ID_LIST_TRANSFERSVIEW, CViewWork::OnCacheHint)
END_EVENT_TABLE()


CViewWork::CViewWork()
{
    wxLogTrace("CViewWork::CViewWork - Function Begining");
    wxLogTrace("CViewWork::CViewWork - Function Ending");
}


CViewWork::CViewWork(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_LIST_WORKVIEW, ID_HTML_WORKVIEW)
{
    m_bProcessingRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_APPLICATION, _("Application"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_NAME, _("Name"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_CPUTIME, _("CPU time"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TOCOMPLETETION, _("To Completetion"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_REPORTDEADLINE, _("Report Deadline"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, -1);
}


CViewWork::~CViewWork()
{
    wxLogTrace("CViewWork::~CViewWork - Function Begining");
    wxLogTrace("CViewWork::~CViewWork - Function Ending");
}


wxString CViewWork::GetViewName()
{
    return wxString(_("Work"));
}


char** CViewWork::GetViewIcon()
{
    return result_xpm;
}


void CViewWork::OnRender(wxTimerEvent &event)
{
    wxLogTrace("CViewWork::OnRender - Processing Render Event...");
    if (!m_bProcessingRenderEvent)
    {
        m_bProcessingRenderEvent = true;

        wxInt32 iProjectCount = wxGetApp().GetDocument()->GetWorkCount();
        wxASSERT(NULL != m_pListPane);
        m_pListPane->SetItemCount(iProjectCount);

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


wxString CViewWork::OnGetItemText(long item, long column) const
{
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

