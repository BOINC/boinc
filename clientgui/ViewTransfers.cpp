// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

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
    m_fProgress = -1.0;
    m_fBytesXferred = -1.0;
    m_fTotalBytes = -1.0;
    m_dTime = -1.0;
    m_dSpeed = -1.0;
}


CTransfer::~CTransfer() {
    m_strProjectName.Clear();
    m_strFileName.Clear();
    m_strStatus.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewTransfers, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_TRANSFERS_RETRYNOW, CViewTransfers::OnTransfersRetryNow)
    EVT_BUTTON(ID_TASK_TRANSFERS_ABORT, CViewTransfers::OnTransfersAbort)
    EVT_LIST_ITEM_SELECTED(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnListDeselected)
    EVT_LIST_COL_CLICK(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnColClick)
    EVT_LIST_CACHE_HINT(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnCacheHint)
END_EVENT_TABLE ()


static CViewTransfers* MyCViewTransfers;

static bool CompareViewTransferItems(int iRowIndex1, int iRowIndex2) {
    CTransfer*      transfer1;
    CTransfer*      transfer2;
    int             result = 0;

    try {
        transfer1 = MyCViewTransfers->m_TransferCache.at(iRowIndex1);
    } catch ( std::out_of_range ) {
        return 0;
    }

    try {
        transfer2 = MyCViewTransfers->m_TransferCache.at(iRowIndex2);
    } catch ( std::out_of_range ) {
        return 0;
    }

    switch (MyCViewTransfers->m_iSortColumn) {
    case COLUMN_PROJECT:
        result = transfer1->m_strProjectName.CmpNoCase(transfer2->m_strProjectName);
        break;
    case COLUMN_FILE:
        result = transfer1->m_strFileName.CmpNoCase(transfer2->m_strFileName);
        break;
    case COLUMN_PROGRESS:
        if (transfer1->m_fProgress < transfer2->m_fProgress) {
            result = -1;
        } else if (transfer1->m_fProgress > transfer2->m_fProgress) {
            result = 1;
        }
        break;
    case COLUMN_SIZE:
        if (transfer1->m_fBytesXferred < transfer2->m_fBytesXferred) {
            result = -1;
        } else if (transfer1->m_fBytesXferred > transfer2->m_fBytesXferred) {
            result = 1;
        }
        break;
    case COLUMN_TIME:
        if (transfer1->m_dTime < transfer2->m_dTime) {
            result = -1;
        } else if (transfer1->m_dTime > transfer2->m_dTime) {
            result = 1;
        }
        break;
    case COLUMN_SPEED:
        if (transfer1->m_dSpeed < transfer2->m_dSpeed) {
            result = -1;
        } else if (transfer1->m_dSpeed > transfer2->m_dSpeed) {
            result = 1;
        }
        break;
    case COLUMN_STATUS:
	result = transfer1->m_strStatus.CmpNoCase(transfer2->m_strStatus);
        break;
    }

    // Always return FALSE for equality (result == 0)
    return (MyCViewTransfers->m_bReverseSort ? (result > 0) : (result < 0));
}


CViewTransfers::CViewTransfers()
{}


CViewTransfers::CViewTransfers(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_TRANSFERSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_TRANSFERSVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS)
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
        _("Click 'Abort transfer' to delete the file from the transfer queue. This will prevent you from being granted credit for this result."),
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

    m_iProgressColumn = COLUMN_PROGRESS;

    // Needed by static sort routine;
    MyCViewTransfers = this;
    m_funcSortCompare = CompareViewTransferItems;

    UpdateSelection();
}


CViewTransfers::~CViewTransfers() {
    EmptyCache();
    EmptyTasks();
}


wxString& CViewTransfers::GetViewName() {
    static wxString strViewName(wxT("Transfers"));
    return strViewName;
}


wxString& CViewTransfers::GetViewDisplayName() {
    static wxString strViewName(_("Transfers"));
    return strViewName;
}


const char** CViewTransfers::GetViewIcon() {
    return xfer_xpm;
}


const int CViewTransfers::GetViewCurrentViewPage() {
    return VW_XFER;
}


wxString CViewTransfers::GetKeyValue1(int iRowIndex) {
    CTransfer*  transfer;
    
    if (GetTransferCacheAtIndex(transfer, m_iSortedIndexes[iRowIndex])) {
        return wxEmptyString;
    }

    return transfer->m_strFileName;
}


wxString CViewTransfers::GetKeyValue2(int iRowIndex) {
    CTransfer*  transfer;
    
    if (GetTransferCacheAtIndex(transfer, m_iSortedIndexes[iRowIndex])) {
        return wxEmptyString;
    }
    
    return transfer->m_strProjectURL;
}


int CViewTransfers::FindRowIndexByKeyValues(wxString& key1, wxString& key2) {
    CTransfer*  transfer;
    unsigned int iRowIndex, n = GetCacheCount();
	for(iRowIndex=0; iRowIndex < n; iRowIndex++) {
        if (GetTransferCacheAtIndex(transfer, m_iSortedIndexes[iRowIndex])) {
            continue;
        }
        if(! (transfer->m_strFileName).IsSameAs(key1)) continue;
        if((transfer->m_strProjectURL).IsSameAs(key2)) return iRowIndex;
	}
	return -1;
}


void CViewTransfers::OnTransfersRetryNow( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersRetryNow - Function Begin"));

    CMainDocument*  pDoc    = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    CC_STATUS status;
    pDoc->GetCoreClientStatus(status);
    if (status.network_suspend_reason) {
        wxString msg = _("Network activity is suspended - ");
        msg += suspend_reason_wxstring(status.network_suspend_reason);
        msg += _(".\nYou can enable it using the Activity menu.");
        wxGetApp().SafeMessageBox(
            msg,
            _("BOINC"),
            wxOK | wxICON_INFORMATION,
            this
        );
        return;
    }

    pFrame->UpdateStatusText(_("Retrying transfer now..."));
    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pDoc->TransferRetryNow(m_iSortedIndexes[row]);
    }
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
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting transfer..."));

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        if (GetTransferCacheAtIndex(pTransfer, m_iSortedIndexes[row])) {
            return;
        }

        strMessage.Printf(
            _("Are you sure you want to abort this file transfer '%s'?\nNOTE: Aborting a transfer will invalidate a task and you\nwill not receive credit for it."), 
            pTransfer->m_strFileName.c_str()
        );

        iAnswer = wxGetApp().SafeMessageBox(
            strMessage,
            _("Abort File Transfer"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->TransferAbort(m_iSortedIndexes[row]);
        }
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
    CTransfer* transfer;
    wxString   strBuffer  = wxEmptyString;

    m_pListPane->AddPendingProgressBar(item);

    try {
        transfer = m_TransferCache.at(m_iSortedIndexes[item]);
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
            // CBOINCListCtrl::DrawProgressBars() will draw this using 
            // data provided by GetProgressText() and GetProgressValue(), 
            // but we need it here for accessibility programs.
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


wxInt32 CViewTransfers::AddCacheElement() {
    CTransfer* pItem = new CTransfer();
    wxASSERT(pItem);
    if (pItem) {
        m_TransferCache.push_back(pItem);
        m_iSortedIndexes.Add((int)m_TransferCache.size()-1);
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
    m_iSortedIndexes.Clear();
    return 0;
}


wxInt32 CViewTransfers::GetCacheCount() {
    return (wxInt32)m_TransferCache.size();
}


wxInt32 CViewTransfers::RemoveCacheElement() {
    unsigned int i;
    delete m_TransferCache.back();
    m_TransferCache.erase(m_TransferCache.end() - 1);
    m_iSortedIndexes.Clear();
    for (i=0; i<m_TransferCache.size(); i++) {
        m_iSortedIndexes.Add(i);
    }
    return 0;
}


bool CViewTransfers::IsSelectionManagementNeeded() {
    return true;
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


bool CViewTransfers::SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex) {
    wxString    strDocumentText  = wxEmptyString;
    wxString    strDocumentText2 = wxEmptyString;
    float       fDocumentFloat = 0.0;
    double      fDocumentDouble = 0.0, fDocumentDouble2 = 0.0;
    CTransfer*  transfer;
    bool        bNeedRefresh = false;

    strDocumentText.Empty();

    if (GetTransferCacheAtIndex(transfer, m_iSortedIndexes[iRowIndex])) {
        return false;
    }

    switch(iColumnIndex) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes[iRowIndex], strDocumentText);
            GetDocProjectURL(m_iSortedIndexes[iRowIndex], strDocumentText2);
            if (!strDocumentText.IsSameAs(transfer->m_strProjectName) || !strDocumentText2.IsSameAs(transfer->m_strProjectURL)) {
                transfer->m_strProjectName = strDocumentText;
                transfer->m_strProjectURL = strDocumentText2;
                bNeedRefresh =  true;
            }
            break;
        case COLUMN_FILE:
            GetDocFileName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(transfer->m_strFileName)) {
                transfer->m_strFileName = strDocumentText;
                bNeedRefresh =  true;
            }
            break;
        case COLUMN_PROGRESS:
            GetDocProgress(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != transfer->m_fProgress) {
                transfer->m_fProgress = fDocumentFloat;
                FormatProgress(fDocumentFloat, transfer->m_strProgress);
                bNeedRefresh =  true;
            }
            break;
        case COLUMN_SIZE:
            GetDocBytesXferred(m_iSortedIndexes[iRowIndex], fDocumentDouble);
            GetDocTotalBytes(m_iSortedIndexes[iRowIndex], fDocumentDouble2);
            if (( fDocumentDouble != transfer->m_fBytesXferred) || 
                (fDocumentDouble2 != transfer->m_fTotalBytes)
                ) {
                transfer->m_fBytesXferred = fDocumentDouble;
                transfer->m_fTotalBytes = fDocumentDouble2;
                FormatSize(fDocumentDouble, fDocumentDouble2, transfer->m_strSize);
                bNeedRefresh =  true;
            }
            break;
        case COLUMN_TIME:
            GetDocTime(m_iSortedIndexes[iRowIndex], fDocumentDouble);
            if (fDocumentDouble != transfer->m_dTime) {
                transfer->m_dTime = fDocumentDouble;
                FormatTime(fDocumentDouble, transfer->m_strTime);
                bNeedRefresh =  true;
            }
            break;
        case COLUMN_SPEED:
            GetDocSpeed(m_iSortedIndexes[iRowIndex], fDocumentDouble);
            if (fDocumentDouble != transfer->m_dSpeed) {
                transfer->m_dSpeed = fDocumentDouble;
                FormatSpeed(fDocumentDouble, transfer->m_strSpeed);
                bNeedRefresh =  true;
            }
            break;
        case COLUMN_STATUS:
            GetDocStatus(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(transfer->m_strStatus)) {
                transfer->m_strStatus = strDocumentText;
                return true;
            }
            break;
    }

    return bNeedRefresh;
}


void CViewTransfers::GetDocProjectName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        strBuffer = HtmlEntityDecode(wxString(transfer->project_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}


void CViewTransfers::GetDocFileName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        strBuffer = wxString(transfer->name.c_str(), wxConvUTF8);
    } else {
        strBuffer = wxEmptyString;
    }
}


void CViewTransfers::GetDocProgress(wxInt32 item, float& fBuffer) const {
    float          fBytesSent = 0;
    float          fFileSize = 0;
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    fBuffer = 0;
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

    if (fFileSize) {
        fBuffer = floor((fBytesSent / fFileSize) * 10000)/100;
    }
}


wxInt32 CViewTransfers::FormatProgress(float fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%.2f%%"), fBuffer);
    return 0;
}


void CViewTransfers::GetDocBytesXferred(wxInt32 item, double& fBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        fBuffer = transfer->bytes_xferred;
    } else {
        fBuffer = 0.0;
    }
}


void CViewTransfers::GetDocTotalBytes(wxInt32 item, double& fBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        fBuffer = transfer->nbytes;
    } else {
        fBuffer = 0.0;
    }
}


wxInt32 CViewTransfers::FormatSize(double fBytesSent, double fFileSize, wxString& strBuffer) const {
    double          xTera = 1099511627776.0;
    double          xGiga = 1073741824.0;
    double          xMega = 1048576.0;
    double          xKilo = 1024.0;

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


void CViewTransfers::GetDocTime(wxInt32 item, double& fBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        fBuffer = transfer->time_so_far;
    } else {
        fBuffer = 0.0;
    }
}


wxInt32 CViewTransfers::FormatTime(double fBuffer, wxString& strBuffer) const {
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;

    iHour = (wxInt32)(fBuffer / (60 * 60));
    iMin  = (wxInt32)(fBuffer / 60) % 60;
    iSec  = (wxInt32)(fBuffer) % 60;

    ts = wxTimeSpan(iHour, iMin, iSec);

    strBuffer = ts.Format();

    return 0;
}


void CViewTransfers::GetDocSpeed(wxInt32 item, double& fBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        if (transfer->xfer_active)
            fBuffer = transfer->xfer_speed / 1024;
        else
            fBuffer = 0.0;
    }
}


wxInt32 CViewTransfers::FormatSpeed(double fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%.2f KBps"), fBuffer);

    return 0;
}


void CViewTransfers::GetDocStatus(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    int            retval;
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }
    CC_STATUS      status;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    retval = pDoc->GetCoreClientStatus(status);

    wxDateTime dtNextRequest((time_t)transfer->next_request_time);
    wxDateTime dtNow(wxDateTime::Now());

    if (transfer && !retval) {
        if      (dtNextRequest > dtNow) {
            wxTimeSpan tsNextRequest(dtNextRequest - dtNow);
            strBuffer = _("Retry in ") + tsNextRequest.Format();
        } else if (ERR_GIVEUP_DOWNLOAD == transfer->status) {
            strBuffer = _("Download failed");
        } else if (ERR_GIVEUP_UPLOAD == transfer->status) {
            strBuffer = _("Upload failed");
        } else {
            if (status.network_suspend_reason) {
                strBuffer = transfer->generated_locally
                    ?_("Upload suspended - ")
                    :_("Download suspended - ")
                ;
                strBuffer += suspend_reason_wxstring(status.network_suspend_reason);
            } else {
                if (transfer->xfer_active) {
                    strBuffer = transfer->generated_locally? _("Uploading") : _("Downloading");
                } else {
                    strBuffer = transfer->generated_locally? _("Upload pending") : _("Download pending");
                }
            }
        }
        if (transfer->project_backoff) {
            wxString x;
            FormatTime(transfer->project_backoff, x);
            strBuffer += _(" (project backoff: ") + x + _(")");
        }
    }
}


void CViewTransfers::GetDocProjectURL(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    wxASSERT(transfer);

    if (transfer) {
        strBuffer = wxString(transfer->project_url.c_str(), wxConvUTF8);
    }
}


double CViewTransfers::GetProgressValue(long item) {
    double          fBytesSent = 0;
    double          fFileSize = 0;
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    
    if (pDoc) {
        transfer = pDoc->file_transfer(m_iSortedIndexes[item]);
    }

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

    if ( 0.0 == fFileSize ) return 0.0;
    
    return (fBytesSent / fFileSize);
}


wxString CViewTransfers::GetProgressText( long item) {
    CTransfer* transfer;
    wxString   strBuffer  = wxEmptyString;

    GetTransferCacheAtIndex(transfer, m_iSortedIndexes[item]);
    if (transfer) {
        strBuffer = transfer->m_strProgress;
    }
    return strBuffer;
}


int CViewTransfers::GetTransferCacheAtIndex(CTransfer*& transferPtr, int index) {
    try {
        transferPtr = m_TransferCache.at(index);
    } catch ( std::out_of_range ) {
        transferPtr = NULL;
        return -1;
    }
    
    return 0;
}

