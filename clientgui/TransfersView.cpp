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
#pragma implementation "TransfersView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "TransfersView.h"
#include "Events.h"

#include "res/xfer.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_SPEED                5
#define COLUMN_STATUS               6


IMPLEMENT_DYNAMIC_CLASS(CTransfersView, CBaseListCtrlView)

BEGIN_EVENT_TABLE(CTransfersView, CBaseListCtrlView)
    EVT_LIST_CACHE_HINT(ID_LIST_TRANSFERSVIEW, CTransfersView::OnCacheHint)
END_EVENT_TABLE()


CTransfersView::CTransfersView()
{
}


CTransfersView::CTransfersView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook, ID_LIST_TRANSFERSVIEW)
{
    InsertColumn(0, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(1, _("File"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(2, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(3, _("Size"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Time"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(5, _("Speed"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(6, _("Status"), wxLIST_FORMAT_LEFT, -1);
}


CTransfersView::~CTransfersView()
{
}


wxString CTransfersView::GetViewName()
{
    return wxString(_("Transfers"));
}


char** CTransfersView::GetViewIcon()
{
    return xfer_xpm;
}


void CTransfersView::OnCacheHint ( wxListEvent& event ) {
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


void CTransfersView::OnRender(wxTimerEvent &event) {
    if (!m_bProcessingRenderEvent)
    {
        wxLogTrace("CTransfersView::OnRender - Processing Render Event...");
        m_bProcessingRenderEvent = true;

        wxInt32 iTransferCount = wxGetApp().GetDocument()->GetTransferCount();
        SetItemCount(iTransferCount);

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


wxString CTransfersView::OnGetItemText(long item, long column) const {
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


int CTransfersView::OnGetItemImage(long item) const {
    return -1;
}


wxListItemAttr* CTransfersView::OnGetItemAttr(long item) const {
    return NULL;
}

