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
// Revision 1.3  2004/09/24 22:19:01  rwalton
// *** empty log message ***
//
// Revision 1.2  2004/09/24 02:01:53  rwalton
// *** empty log message ***
//
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
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewTransfers.h"
#include "Events.h"

#include "res/xfer.xpm"
#include "res/task.xpm"
#include "res/tips.xpm"

#define VIEW_HEADER                 "xfer"

#define SECTION_TASK                VIEW_HEADER "task"
#define SECTION_TIPS                VIEW_HEADER "tips"

#define BITMAP_TRANSFER             VIEW_HEADER ".xpm"
#define BITMAP_TASKHEADER           SECTION_TASK ".xpm"
#define BITMAP_TIPSHEADER           SECTION_TIPS ".xpm"

#define LINK_TASKRETRY              SECTION_TASK "retry"
#define LINK_TASKABORT              SECTION_TASK "abort"

#define LINK_DEFAULT                "default"

#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_SPEED                5
#define COLUMN_STATUS               6


IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CBOINCBaseView)


CViewTransfers::CViewTransfers()
{
}


CViewTransfers::CViewTransfers(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_TRANSFERSVIEW, ID_LIST_TRANSFERSVIEW)
{
    m_bProcessingRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpTransfer(xfer_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpTransfer.SetMask(new wxMask(bmpTransfer, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(wxT(BITMAP_TRANSFER), bmpTransfer, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TASKHEADER), bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TIPSHEADER), bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_FILE, _("File"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_SIZE, _("Size"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_SPEED, _("Speed"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        wxT(LINK_DEFAULT), 
        _("Please select a transfer item to see additional options.")
    );

    UpdateSelection();
}


CViewTransfers::~CViewTransfers()
{
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

        long lSelected = m_pListPane->GetFirstSelected();
        if ( (-1 == lSelected) && m_bItemSelected )
        {
            UpdateSelection();
        }

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CViewTransfers::OnListSelected ( wxListEvent& event )
{
    wxLogTrace("CViewTransfers::OnListSelected - Processing Event...");
    UpdateSelection();
    event.Skip();
}


void CViewTransfers::OnListDeselected ( wxListEvent& event )
{
    wxLogTrace("CViewTransfers::OnListDeselected - Processing Event...");
    UpdateSelection();
    event.Skip();
}


wxString CViewTransfers::OnListGetItemText(long item, long column) const
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


void CViewTransfers::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxString strMessage;

    if ( link.GetHref() == wxT(SECTION_TASK) )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;

    if ( link.GetHref() == wxT(SECTION_TIPS) )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;


    UpdateSelection();
}


void CViewTransfers::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( wxT(LINK_TASKRETRY) == strLink )
        {
            if  ( wxT(LINK_TASKRETRY) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKRETRY), 
                    _("<b>Retry Now</b><br>"
                      "Selecting retry now will attempt to upload the result data file "
                      "to the project server now.")
                );

                bUpdateSelection = true;
            }
        }
        else if ( wxT(LINK_TASKABORT) == strLink )
        {
            if  ( wxT(LINK_TASKABORT) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKABORT), 
                    _("<b>Abort Upload</b><br>"
                      "Selecting abort upload will delete the result from the upload queue. "
                      "Doing this will keep you from being granted any credit for this result.")
                );

                bUpdateSelection = true;
            }
        }
        else
        {
            long lSelected = m_pListPane->GetFirstSelected();
            if ( -1 == lSelected )
            {
                if  ( wxT(LINK_DEFAULT) != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        wxT(LINK_DEFAULT), 
                        _("Please select an result upload to see additional options.")
                    );

                    bUpdateSelection = true;
                }
            }
        }

        if ( bUpdateSelection )
        {
            UpdateSelection();
        }
    }
}


void CViewTransfers::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    long lSelected = m_pListPane->GetFirstSelected();
    if ( -1 == lSelected )
    {
        m_bTaskHeaderHidden = true;
        m_bTaskRetryHidden = true;
        m_bTaskAbortHidden = true;

        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = false;
        m_bTaskRetryHidden = false;
        m_bTaskAbortHidden = false;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewTransfers::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( wxT(SECTION_TASK), wxT(BITMAP_TASKHEADER), m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( wxT(LINK_TASKRETRY), wxT(BITMAP_TRANSFER), _("Retry Now"), m_bTaskRetryHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKABORT), wxT(BITMAP_TRANSFER), _("Abort Upload"), m_bTaskAbortHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip(wxT(SECTION_TIPS), wxT(BITMAP_TIPSHEADER), GetCurrentQuickTipText(), m_bTipsHeaderHidden);

    m_pTaskPane->EndTaskPage();
}

