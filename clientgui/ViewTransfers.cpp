// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#include "SkinManager.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewTransfers.h"
#include "Events.h"
#include "error_numbers.h"


#include "res/xfer.xpm"


// Column IDs must be equal to the column's default
// position (left to right, zero-based) when all
// columns are shown.  However, any column may be
// hidden, either by default or by the user.
// (On MS Windows only, the user can also rearrange
// the columns from the default order.)
//
// Column IDs
#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_TOCOMPLETION         5
#define COLUMN_SPEED                6
#define COLUMN_STATUS               7

// DefaultShownColumns is an array containing the
// columnIDs of the columns to be shown by default,
// in ascending order.  It may or may not include
// all columns.
//
// For now, show all columns by default
static int DefaultShownColumns[] = { COLUMN_PROJECT, COLUMN_FILE, COLUMN_PROGRESS,
                                COLUMN_SIZE, COLUMN_TIME, COLUMN_TOCOMPLETION, COLUMN_SPEED,
                                COLUMN_STATUS};

// buttons in the "tasks" area
#define BTN_RETRY       0
#define BTN_ABORT       1


CTransfer::CTransfer() {
    m_fProgress = -1.0;
    m_fBytesXferred = -1.0;
    m_fTotalBytes = -1.0;
    m_dTime = -1.0;
    m_fTimeToCompletion = -1.0;
    m_dSpeed = -1.0;
}


CTransfer::~CTransfer() {
    m_strProjectName.Clear();
    m_strFileName.Clear();
    m_strStatus.Clear();
    m_strProjectURL.Clear();
    m_strProgress.Clear();
    m_strSize.Clear();
    m_strTime.Clear();
    m_strTimeToCompletion.Clear();
    m_strSpeed.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewTransfers, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_TRANSFERS_RETRYNOW, CViewTransfers::OnTransfersRetryNow)
    EVT_BUTTON(ID_TASK_TRANSFERS_ABORT, CViewTransfers::OnTransfersAbort)
// We currently handle EVT_LIST_CACHE_HINT on Windows or
// EVT_CHECK_SELECTION_CHANGED on Mac & Linux instead of EVT_LIST_ITEM_SELECTED
// or EVT_LIST_ITEM_DESELECTED.  See CBOINCBaseView::OnCacheHint() for info.
#if USE_LIST_CACHE_HINT
    EVT_LIST_CACHE_HINT(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnCacheHint)
#else
	EVT_CHECK_SELECTION_CHANGED(CViewTransfers::OnCheckSelectionChanged)
#endif
    EVT_LIST_COL_CLICK(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnColClick)
    EVT_LIST_COL_END_DRAG(ID_LIST_TRANSFERSVIEW, CViewTransfers::OnColResize)
END_EVENT_TABLE ()


static CViewTransfers* MyCViewTransfers;

static bool CompareViewTransferItems(int iRowIndex1, int iRowIndex2) {
    CTransfer*      transfer1;
    CTransfer*      transfer2;
    int             result = 0;

    try {
        transfer1 = MyCViewTransfers->m_TransferCache.at(iRowIndex1);
    } catch ( std::out_of_range& ) {
        return 0;
    }

    try {
        transfer2 = MyCViewTransfers->m_TransferCache.at(iRowIndex2);
    } catch ( std::out_of_range& ) {
        return 0;
    }

    switch (MyCViewTransfers->m_iSortColumnID) {
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
        if (transfer1->m_fTotalBytes < transfer2->m_fTotalBytes) {
            result = -1;
        } else if (transfer1->m_fTotalBytes > transfer2->m_fTotalBytes) {
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
    case COLUMN_TOCOMPLETION:
        if (transfer1->m_fTimeToCompletion < transfer2->m_fTimeToCompletion) {
            result = -1;
        }
        else if (transfer1->m_fTimeToCompletion > transfer2->m_fTimeToCompletion) {
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
    CBOINCBaseView(pNotebook, ID_TASK_TRANSFERSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_TRANSFERSVIEW, DEFAULT_LIST_FLAGS)
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
        _("Retry the file transfer now"),
        ID_TASK_TRANSFERS_RETRYNOW
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Abort Transfer"),
        _("Abort this file transfer.  You won't get credit for the task."),
        ID_TASK_TRANSFERS_ABORT
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // m_aStdColNameOrder is an array of all column heading labels
    // (localized) in order of ascending Column ID.
    // Once initialized, it should not be modified.
    //
    m_aStdColNameOrder = new wxArrayString;
    m_aStdColNameOrder->Insert(_("Project"), COLUMN_PROJECT);
    m_aStdColNameOrder->Insert(_("File"), COLUMN_FILE);
    m_aStdColNameOrder->Insert(_("Progress"), COLUMN_PROGRESS);
    m_aStdColNameOrder->Insert(_("Size"), COLUMN_SIZE);
    m_aStdColNameOrder->Insert(_("Elapsed"), COLUMN_TIME);
    m_aStdColNameOrder->Insert(_("Remaining (estimated)"), COLUMN_TOCOMPLETION);
    m_aStdColNameOrder->Insert(_("Speed"), COLUMN_SPEED);
    m_aStdColNameOrder->Insert(_("Status"), COLUMN_STATUS);

    // m_iStdColWidthOrder is an array of the width for each column.
    // Entries must be in order of ascending Column ID.  We initialize
    // it here to the default column widths.  It is updated by
    // CBOINCListCtrl::OnRestoreState() and also when a user resizes
    // a column by dragging the divider between two columns.
    //
    m_iStdColWidthOrder.Clear();
    m_iStdColWidthOrder.Insert(125, COLUMN_PROJECT);
    m_iStdColWidthOrder.Insert(205, COLUMN_FILE);
    m_iStdColWidthOrder.Insert(60, COLUMN_PROGRESS);
    m_iStdColWidthOrder.Insert(80, COLUMN_SIZE);
    m_iStdColWidthOrder.Insert(80, COLUMN_TIME);
    m_iStdColWidthOrder.Insert(100, COLUMN_TOCOMPLETION);
    m_iStdColWidthOrder.Insert(80, COLUMN_SPEED);
    m_iStdColWidthOrder.Insert(150, COLUMN_STATUS);

    m_iDefaultShownColumns = DefaultShownColumns;
    m_iNumDefaultShownColumns = sizeof(DefaultShownColumns) / sizeof(int);

    wxASSERT(m_iStdColWidthOrder.size() == m_aStdColNameOrder->size());

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


// Create List Pane Items
void CViewTransfers::AppendColumn(int columnID){
    switch(columnID) {
        case COLUMN_PROJECT:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_PROJECT],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_PROJECT]);
            break;
        case COLUMN_FILE:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_FILE],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_FILE]);
            break;
        case COLUMN_PROGRESS:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_PROGRESS],
                wxLIST_FORMAT_CENTRE, m_iStdColWidthOrder[COLUMN_PROGRESS]);
            break;
        case COLUMN_SIZE:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_SIZE],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_SIZE]);
            break;
        case COLUMN_TIME:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_TIME],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_TIME]);
            break;
        case COLUMN_TOCOMPLETION:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_TOCOMPLETION],
                wxLIST_FORMAT_RIGHT, m_iStdColWidthOrder[COLUMN_TOCOMPLETION]);
            break;
        case COLUMN_SPEED:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_SPEED],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_SPEED]);
            break;
        case COLUMN_STATUS:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_STATUS],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_STATUS]);
            break;
    }
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


int CViewTransfers::GetViewCurrentViewPage() {
    return VW_XFER;
}


wxString CViewTransfers::GetKeyValue1(int iRowIndex) {
    CTransfer*  transfer;

    if (GetTransferCacheAtIndex(transfer, m_iSortedIndexes[iRowIndex])) {
        return wxEmptyString;
    }

    if (m_iColumnIDToColumnIndex[COLUMN_FILE] < 0) {
        // Column is hidden, so SynchronizeCacheItem() did not set its value
        GetDocFileName(m_iSortedIndexes[iRowIndex], transfer->m_strFileName);
    }

    return transfer->m_strFileName;
}


wxString CViewTransfers::GetKeyValue2(int iRowIndex) {
    CTransfer*  transfer;

    if (GetTransferCacheAtIndex(transfer, m_iSortedIndexes[iRowIndex])) {
        return wxEmptyString;
    }

    if (m_iColumnIDToColumnIndex[COLUMN_PROJECT] < 0) {
        // Column is hidden, so SynchronizeCacheItem() did not set its value
        GetDocProjectURL(m_iSortedIndexes[iRowIndex], transfer->m_strProjectURL);
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
    CSkinAdvanced*  pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
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
            pSkinAdvanced->GetApplicationShortName(),
            wxOK | wxICON_INFORMATION,
            this
        );
        return;
    }

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        pDoc->TransferRetryNow(m_iSortedIndexes[row]);
    }

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

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfers::OnTransfersAbort - Function End"));
}


void CViewTransfers::OnColResize( wxListEvent& ) {
    // Register the new column widths immediately
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    pFrame->SaveState();
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
    } catch ( std::out_of_range& ) {
        transfer = NULL;
    }

    if (transfer && (column >= 0)) {
        switch(m_iColumnIndexToColumnID[column]) {
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
        case COLUMN_TOCOMPLETION:
            strBuffer = transfer->m_strTimeToCompletion;
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
    double       x = 0.0;
    double      fDocumentDouble = 0.0, fDocumentDouble2 = 0.0;
    CTransfer*  transfer;
    bool        bNeedRefresh = false;

    strDocumentText.Empty();

    if (GetTransferCacheAtIndex(transfer, m_iSortedIndexes[iRowIndex])) {
        return false;
    }

    if (iColumnIndex < 0) return false;

    switch (m_iColumnIndexToColumnID[iColumnIndex]) {
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
            GetDocProgress(m_iSortedIndexes[iRowIndex], x);
            if (x != transfer->m_fProgress) {
                transfer->m_fProgress = x;
                FormatProgress(x, transfer->m_strProgress);
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
                transfer->m_strTime = FormatTime(fDocumentDouble);
                bNeedRefresh =  true;
            }
            break;
        case COLUMN_TOCOMPLETION:
            GetDocTimeToCompletion(m_iSortedIndexes[iRowIndex], fDocumentDouble);
            if (fDocumentDouble != transfer->m_fTimeToCompletion) {
                transfer->m_fTimeToCompletion = fDocumentDouble;
                transfer->m_strTimeToCompletion = FormatTime(fDocumentDouble);
                bNeedRefresh = true;
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


void CViewTransfers::GetDocProgress(wxInt32 item, double& fBuffer) const {
    double          fBytesSent = 0;
    double          fFileSize = 0;
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


wxInt32 CViewTransfers::FormatProgress(double fBuffer, wxString& strBuffer) const {
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
        } else if (fBytesSent > 0) {
            strBuffer.Printf(wxT("%0.0f bytes"), fBytesSent);
        } else {
            strBuffer.Printf(wxT("---"));
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

void CViewTransfers::GetDocTimeToCompletion(wxInt32 item, double& fBuffer) const {
    FILE_TRANSFER* transfer = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        transfer = pDoc->file_transfer(item);
    }

    if (transfer) {
        fBuffer = transfer->estimated_xfer_time_remaining;
    }
    else {
        fBuffer = 0.0;
    }
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
    int retval;
    strBuffer = wxString("", wxConvUTF8);

    transfer = pDoc->file_transfer(item);
    if (!transfer) return;
    CC_STATUS      status;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    retval = pDoc->GetCoreClientStatus(status);
    if (retval) return;

    wxDateTime dtNextRequest((time_t)transfer->next_request_time);
    wxDateTime dtNow(wxDateTime::Now());

    strBuffer = transfer->is_upload?_("Upload"):_("Download");
    strBuffer += wxString(": ", wxConvUTF8);
    if (dtNextRequest > dtNow) {
        wxTimeSpan tsNextRequest(dtNextRequest - dtNow);
        strBuffer += _("retry in ") + tsNextRequest.Format();
    } else if (transfer->status == ERR_GIVEUP_DOWNLOAD || transfer->status == ERR_GIVEUP_UPLOAD) {
        strBuffer = _("failed");
    } else {
        if (status.network_suspend_reason) {
            strBuffer += _("suspended");
            strBuffer += wxString(" - ", wxConvUTF8);
            strBuffer += suspend_reason_wxstring(status.network_suspend_reason);
        } else {
            if (transfer->xfer_active) {
                strBuffer += _("active");
            } else {
                strBuffer += _("pending");
            }
        }
    }
    if (transfer->project_backoff) {
        wxString x = FormatTime(transfer->project_backoff);
        strBuffer += _(" (project backoff: ") + x + _(")");
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
    } catch ( std::out_of_range& ) {
        transferPtr = NULL;
        return -1;
    }

    return 0;
}

