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
// Revision 1.1  2004/09/21 01:26:25  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewTransfers.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ViewTransfers.h"
#include "Events.h"

#include "res/xfer.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_SPEED                5
#define COLUMN_STATUS               6


IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CBOINCBaseView)

BEGIN_EVENT_TABLE(CViewTransfers, CBOINCBaseView)
    EVT_LIST_CACHE_HINT(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnCacheHint)
END_EVENT_TABLE()


CViewTransfers::CViewTransfers()
{
    wxLogTrace("CViewTransfers::CViewTransfers - Function Begining");
    wxLogTrace("CViewTransfers::CViewTransfers - Function Ending");
}


CViewTransfers::CViewTransfers(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_LIST_TRANSFERSVIEW, ID_HTML_TRANSFERSVIEW)
{
    m_bProcessingRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_FILE, _("File"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_SIZE, _("Size"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_SPEED, _("Speed"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, -1);
}


CViewTransfers::~CViewTransfers()
{
    wxLogTrace("CViewTransfers::~CViewTransfers - Function Begining");
    wxLogTrace("CViewTransfers::~CViewTransfers - Function Ending");
}


wxString CViewTransfers::GetViewName()
{
    return wxString(_("Transfers"));
}


char** CViewTransfers::GetViewIcon()
{
    return xfer_xpm;
}


void CViewTransfers::OnRender(wxTimerEvent &event)
{
    wxLogTrace("CViewTransfers::OnRender - Processing Render Event...");
    if (!m_bProcessingRenderEvent)
    {
        m_bProcessingRenderEvent = true;

        wxInt32 iProjectCount = wxGetApp().GetDocument()->GetTransferCount();
        wxASSERT(NULL != m_pListPane);
        m_pListPane->SetItemCount(iProjectCount);

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


wxString CViewTransfers::OnGetItemText(long item, long column) const
{
    wxString strBuffer;
    switch(column) {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetTransferProjectName(item);
            break;
        case COLUMN_FILE:
            strBuffer = wxGetApp().GetDocument()->GetTransferFileName(item);
            break;
        case COLUMN_PROGRESS:
            strBuffer = wxGetApp().GetDocument()->GetTransferProgress(item);
            break;
        case COLUMN_SIZE:
            strBuffer = wxGetApp().GetDocument()->GetTransferSize(item);
            break;
        case COLUMN_TIME:
            strBuffer = wxGetApp().GetDocument()->GetTransferTime(item);
            break;
        case COLUMN_SPEED:
            strBuffer = wxGetApp().GetDocument()->GetTransferSpeed(item);
            break;
        case COLUMN_STATUS:
            strBuffer = wxGetApp().GetDocument()->GetTransferStatus(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}

