// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewTransfers.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewTransfers.h"
#include "Events.h"
#include "error_numbers.h"


#include "res/xfer.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_SPEED                5
#define COLUMN_STATUS               6

// buttons in the "tasks" area
#define BTN_RETRY       0
#define BTN_ABORT       1


CTransfer::CTransfer() {
}


CTransfer::~CTransfer() {
    m_strProjectName.Clear();
    m_strFileName.Clear();
    m_strProgress.Clear();
    m_strSize.Clear();
    m_strTime.Clear();
    m_strSpeed.Clear();
    m_strStatus.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewTransfers, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_TRANSFERS_RETRYNOW, CViewTransfers::OnTransfersRetryNow)
    EVT_BUTTON(ID_TASK_TRANSFERS_ABORT, CViewTransfers::OnTransfersAbort)
    EVT_LIST_ITEM_SELECTED(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnListDeselected)
END_EVENT_TABLE ()


CViewTransfers::CViewTransfers()
{}


CViewTransfers::CViewTransfers(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_TRANSFERSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_TRANSFERSVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS)
{
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);


    //
    // Setup View
    //
	pGroup = new CTaskItemGroup( _("Commands") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("Retry Now"),
        _("Click 'Retry now' to transfer the file now"),
        ID_TASK_TRANSFERS_RETRYNOW 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Abort Transfer"),
        _("Click 'Abort transfer' to delete the file from the transfer queue. "
          "This will prevent you from being granted credit for this result."),
        ID_TASK_TRANSFERS_ABORT 
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 125);
    m_pListPane->InsertColumn(COLUMN_FILE, _("File"), wxLIST_FORMAT_LEFT, 205);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_CENTRE, 60);
    m_pListPane->InsertColumn(COLUMN_SIZE, _("Size"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Elapsed Time"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_SPEED, _("Speed"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    UpdateSelection();
}


CViewTransfers::~CViewTransfers() {
    EmptyCache();
    EmptyTasks();
}


wxString& CViewTransfers::GetViewName() {
    static wxString strViewName(_("Transfers"));
    return strViewName;
}


wxString& CViewTransfers::GetViewDisplayName() {
    static wxString strViewName(_("Transfers"));
    return strViewName;
}


const char** CViewTransfers::GetViewIcon() {
    return xfer_xpm;
}


void CViewTransfers::OnTransfersRetryNow( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersRetryNow - Function Begin"));

    CMainDocument*  pDoc    = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Retrying transfer now..."));
    pDoc->TransferRetryNow(m_pListPane->GetFirstSelected());
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ResetReminderTimers();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersRetryNow - Function End"));
}


void CViewTransfers::OnTransfersAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersAbort - Function Begin"));

    wxInt32         iAnswer    = 0; 
    wxString        strMessage = wxEmptyString;
    CMainDocument*  pDoc       = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame     = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    CTransfer*      pTransfer  = NULL;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting transfer..."));

    pTransfer = m_TransferCache.at(m_pListPane->GetFirstSelected());

    strMessage.Printf(
        _("Are you sure you want to abort this file transfer '%s'?\n"
          "NOTE: Aborting a transfer will invalidate a task and you\n"
          "will not receive credit for it."), 
        pTransfer->m_strFileName.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Abort File Transfer"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->TransferAbort(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersAbort - Function End"));
}


wxInt32 CViewTransfers::GetDocCount() {
    return wxGetApp().GetDocument()->GetTransferCount();
}


wxString CViewTransfers::OnListGetItemText(long item, long column) const {
    CTransfer* transfer   = m_TransferCache.at(item);
    wxString   strBuffer  = wxEmptyString;

    try {
        transfer = m_TransferCache.at(item);
    } catch ( std::out_of_range ) {
        transfer = NULL;
    }

    if (transfer) {
        switch(column) {
            case COLUMN_PROJECT:
                strBuffer = transfer->m_strProjectName;
                break;
            case COLUMN_FILE:
                strBuffer = transfer->m_strFileName;
                break;
            case COLUMN_PROGRESS:
                strBuffer = transfer->m_strProgress;
                break;
            case COLUMN_SIZE:
                strBuffer = transfer->m_strSize;
                break;
            case COLUMN_TIME:
                strBuffer = transfer->m_strTime;
                break;
            case COLUMN_SPEED:
                strBuffer = transfer->m_strSpeed;
                break;
            case COLUMN_STATUS:
                strBuffer = transfer->m_strStatus;
                break;
        }
    }

    return strBuffer;
}


wxString CViewTransfers::OnDocGetItemText(long item, long column) const {
    wxString       strBuffer = wxEmptyString;

    switch(column) {
        case COLUMN_PROJECT:
            FormatProjectName(item, strBuffer);
            break;
        case COLUMN_FILE:
            FormatFileName(item, strBuffer);
            break;
        case COLUMN_PROGRESS:
            FormatProgress(item, strBuffer);
            break;
        case COLUMN_SIZE:
            FormatSize(item, strBuffer);
            break;
        case COLUMN_TIME:
            FormatTime(item, strBuffer);
            break;
        case COLUMN_SPEED:
            FormatSpeed(item, strBuffer);
            break;
        case COLUMN_STATUS:
            FormatStatus(item, strBuffer);
            break;
    }

    return strBuffer;
}


wxInt32 CViewTransfers::AddCacheElement() {
    CTransfer* pItem = new CTransfer();
    wxASSERT(pItem);
    if (pItem) {
        m_TransferCache.push_back(pItem);
        return 0;
    }
    return -1;
}


wxInt32 CViewTransfers::EmptyCache() {
    unsigned int i;
    for (i=0; i<m_TransferCache.size(); i++) {
        delete m_TransferCache[i];
    }
    m_TransferCache.clear();
    return 0;
}


wxInt32 CViewTransfers::GetCacheCount() {
    return (wxInt32)m_TransferCache.size();
}


wxInt32 CViewTransfers::RemoveCacheElement() {
    delete m_TransferCache.back();
    m_TransferCache.erase(m_TransferCache.end() - 1);
    return 0;
}


wxInt32 CViewTransfers::UpdateCache(long item, long column, wxString& strNewData) {
    CTransfer* transfer   = m_TransferCache.at(item);

    switch(column) {
        case COLUMN_PROJECT:
            transfer->m_strProjectName = strNewData;
            break;
        case COLUMN_FILE:
            transfer->m_strFileName = strNewData;
            break;
        case COLUMN_PROGRESS:
            transfer->m_strProgress = strNewData;
            break;
        case COLUMN_SIZE:
            transfer->m_strSize = strNewData;
            break;
        case COLUMN_TIME:
            transfer->m_strTime = strNewData;
            break;
        case COLUMN_SPEED:
            transfer->m_strSpeed = strNewData;
            break;
        case COLUMN_STATUS:
            transfer->m_strStatus = strNewData;
            break;
    }

    return 0;
}


void CViewTransfers::UpdateSelection() {
    CTaskItemGroup* pGroup = m_TaskGroups[0];

    CBOINCBaseView::PreUpdateSelection();

    if (m_pListPane->GetSelectedItemCount()) {
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }

    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewTransfers::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        strBuffer = HtmlEntityDecode(wxString(transfer->project_name.c_str(), wxConvUTF8));
    }
    return 0;
}


wxInt32 CViewTransfers::FormatFileName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        strBuffer = wxString(transfer->name.c_str(), wxConvUTF8);
    }
    return 0;
}


wxInt32 CViewTransfers::FormatProgress(wxInt32 item, wxString& strBuffer) const {
    float          fBytesSent = 0;
    float          fFileSize = 0;
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        fBytesSent = transfer->bytes_xferred;
        fFileSize = transfer->nbytes;
    }

    // Curl apparently counts the HTTP header in byte count.
    // Prevent this from causing > 100% display
    //
    if (fBytesSent > fFileSize) {
        fBytesSent = fFileSize;
    }

    if ( 0.0 == fFileSize ) {
        strBuffer = wxT("0%");
    } else {
        strBuffer.Printf(wxT("%.2f%%"), floor((fBytesSent / fFileSize) * 10000)/100);
    }

    return 0;
}


wxInt32 CViewTransfers::FormatSize(wxInt32 item, wxString& strBuffer) const {
    float          fBytesSent = 0;
    float          fFileSize = 0;
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        fBytesSent = transfer->bytes_xferred;
        fFileSize = transfer->nbytes;
    }

    if (fFileSize != 0) {
        if      (fFileSize >= xTera) {
            strBuffer.Printf(wxT("%0.2f/%0.2f TB"), fBytesSent/xTera, fFileSize/xTera);
        } else if (fFileSize >= xGiga) {
            strBuffer.Printf(wxT("%0.2f/%0.2f GB"), fBytesSent/xGiga, fFileSize/xGiga);
        } else if (fFileSize >= xMega) {
            strBuffer.Printf(wxT("%0.2f/%0.2f MB"), fBytesSent/xMega, fFileSize/xMega);
        } else if (fFileSize >= xKilo) {
            strBuffer.Printf(wxT("%0.2f/%0.2f KB"), fBytesSent/xKilo, fFileSize/xKilo);
        } else {
            strBuffer.Printf(wxT("%0.0f/%0.0f bytes"), fBytesSent, fFileSize);
        }
    } else {
        if      (fBytesSent >= xTera) {
            strBuffer.Printf(wxT("%0.2f TB"), fBytesSent/xTera);
        } else if (fBytesSent >= xGiga) {
            strBuffer.Printf(wxT("%0.2f GB"), fBytesSent/xGiga);
        } else if (fBytesSent >= xMega) {
            strBuffer.Printf(wxT("%0.2f MB"), fBytesSent/xMega);
        } else if (fBytesSent >= xKilo) {
            strBuffer.Printf(wxT("%0.2f KB"), fBytesSent/xKilo);
        } else {
            strBuffer.Printf(wxT("%0.0f bytes"), fBytesSent);
        }
    }

    return 0;
}


wxInt32 CViewTransfers::FormatTime(wxInt32 item, wxString& strBuffer) const {
    float          fBuffer = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        fBuffer = transfer->time_so_far;
    }

    iHour = (wxInt32)(fBuffer / (60 * 60));
    iMin  = (wxInt32)(fBuffer / 60) % 60;
    iSec  = (wxInt32)(fBuffer) % 60;

    ts = wxTimeSpan(iHour, iMin, iSec);

    strBuffer = ts.Format();

    return 0;
}


wxInt32 CViewTransfers::FormatSpeed(wxInt32 item, wxString& strBuffer) const {
    float          fTransferSpeed = 0;
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        if (transfer->xfer_active)
            fTransferSpeed = transfer->xfer_speed / 1024;
        else
            fTransferSpeed = 0.0;
    }

    strBuffer.Printf(wxT("%.2f KBps"), fTransferSpeed);

    return 0;
}


wxInt32 CViewTransfers::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);
    CC_STATUS      status;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    doc->GetCoreClientStatus(status);

    wxDateTime dtNextRequest((time_t)transfer->next_request_time);
    wxDateTime dtNow(wxDateTime::Now());

    if (transfer) {
        if      (dtNextRequest > dtNow) {
            wxTimeSpan tsNextRequest(dtNextRequest - dtNow);
            strBuffer = _("Retry in ") + tsNextRequest.Format();
        } else if (ERR_GIVEUP_DOWNLOAD == transfer->status) {
            strBuffer = _("Download failed");
        } else if (ERR_GIVEUP_UPLOAD == transfer->status) {
            strBuffer = _("Upload failed");
        } else {
            if (status.network_suspend_reason) {
                strBuffer = _("Suspended");
            } else {
                if (transfer->xfer_active) {
                    strBuffer = transfer->generated_locally? _("Uploading") : _("Downloading");
                } else {
                    strBuffer = transfer->generated_locally? _("Upload pending") : _("Download pending");
                }
            }
        }
    }

    return 0;
}


const char *BOINC_RCSID_7aadb93332 = "$Id: ViewTransfers.cpp 13804 2007-10-09 11:35:47Z fthomas $";
