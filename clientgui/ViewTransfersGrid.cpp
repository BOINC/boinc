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
#pragma implementation "ViewTransfersGrid.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "ViewTransfersGrid.h"
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
#define COLUMN_HIDDEN_URL           7
#define NUM_COLUMNS                 (COLUMN_HIDDEN_URL+1)

// buttons in the "tasks" area
#define BTN_RETRY       0
#define BTN_ABORT       1



IMPLEMENT_DYNAMIC_CLASS(CViewTransfersGrid, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewTransfersGrid, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_TRANSFERS_RETRYNOW, CViewTransfersGrid::OnTransfersRetryNow)
    EVT_BUTTON(ID_TASK_TRANSFERS_ABORT, CViewTransfersGrid::OnTransfersAbort)
	EVT_GRID_SELECT_CELL(CViewTransfersGrid::OnGridSelectCell)
	EVT_GRID_RANGE_SELECT(CViewTransfersGrid::OnGridSelectRange)
END_EVENT_TABLE ()


CViewTransfersGrid::CViewTransfersGrid()
{}


CViewTransfersGrid::CViewTransfersGrid(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook)
{
    //
    // Setup View
    //
    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
	m_pTaskPane = new CBOINCTaskCtrl(this, ID_TASK_TRANSFERSGRIDVIEW, DEFAULT_TASK_FLAGS);
    wxASSERT(m_pTaskPane);

	m_pGridPane = new CBOINCGridCtrl(this, ID_LIST_TRANSFERSGRIDVIEW);
    wxASSERT(m_pGridPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pGridPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();
	
	// Setup TaskPane
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);


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

    // Create Grid
    m_pGridPane->Setup();
    m_pGridPane->SetTable(new CBOINCGridTable(1,NUM_COLUMNS));
    m_pGridPane->SetSelectionMode(wxGrid::wxGridSelectRows);
    // init grid columns
    wxInt32 colSizes[] = {125,205,60,80,80,80,150,0};
    wxString colTitles[] = {_("Project"),_("File"),_("Progress"),_("Size"),
                            _("Elapsed Time"),_("Speed"),_("Status"),wxEmptyString
                            };
    for(int i=0; i<NUM_COLUMNS;i++){
        m_pGridPane->SetColLabelValue(i,colTitles[i]);
        m_pGridPane->SetColSize(i,colSizes[i]);
    }
    
    //change the default cell renderer
    m_pGridPane->SetDefaultRenderer(new CBOINCGridCellProgressRenderer(COLUMN_PROGRESS,false));
    //set column sort types
    m_pGridPane->SetColumnSortType(COLUMN_PROGRESS,CST_FLOAT);
    m_pGridPane->SetColumnSortType(COLUMN_TIME,CST_TIME);
    //m_pGridPane->SetColumnSortType(COLUMN_SIZE,CST_FLOAT);
    m_pGridPane->SetColumnSortType(COLUMN_SPEED,CST_FLOAT);
    //set primary key column index
    m_pGridPane->SetPrimaryKeyColumns(COLUMN_FILE,COLUMN_HIDDEN_URL);
    // Hide the URL column
    int min_width = m_pGridPane->GetColMinimalAcceptableWidth();
    m_pGridPane->SetColMinimalAcceptableWidth(0);
    m_pGridPane->SetColSize(COLUMN_HIDDEN_URL,0);
    m_pGridPane->SetColMinimalAcceptableWidth(min_width);
    UpdateSelection();
}


CViewTransfersGrid::~CViewTransfersGrid() {
    EmptyTasks();
}


wxString& CViewTransfersGrid::GetViewName() {
    static wxString strViewName(_("TransfersGrid"));
    return strViewName;
}


wxString& CViewTransfersGrid::GetViewDisplayName() {
    static wxString strViewName(_("Transfers"));
    return strViewName;
}


const char** CViewTransfersGrid::GetViewIcon() {
    return xfer_xpm;
}


void CViewTransfersGrid::OnTransfersRetryNow( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfersGrid::OnTransfersRetryNow - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    pFrame->UpdateStatusText(_("Retrying transfer now..."));
    
	wxArrayInt aRows = m_pGridPane->GetSelectedRows2();
	for(unsigned int i=0; i< aRows.Count();i++) {
            int row = aRows.Item(i);
            wxString fileName = m_pGridPane->GetCellValue(row,COLUMN_FILE).Trim(false);
            wxString projectURL = m_pGridPane->GetCellValue(row,COLUMN_HIDDEN_URL).Trim(false);
            pDoc->TransferRetryNow(fileName, projectURL);		
	}

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ResetReminderTimers();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfersGrid::OnTransfersRetryNow - Function End"));
}


void CViewTransfersGrid::OnTransfersAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfersGrid::OnTransfersAbort - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strName        = wxEmptyString;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pGridPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting transfer(s)..."));
	strMessage.Printf(
        _("Are you sure you want to abort this file(s) transfer ?\n"
          "NOTE: Aborting a transfer will invalidate a task and you\n"
          "will not receive credit for it."));
    iAnswer = ::wxMessageBox(strMessage,_("Abort File Transfer(s)"),wxYES_NO | wxICON_QUESTION,this);
    if (wxYES == iAnswer) {
        wxArrayInt aRows = m_pGridPane->GetSelectedRows2();
        for(unsigned int i=0; i< aRows.Count();i++) {
            int row = aRows.Item(i);
            wxString fileName = m_pGridPane->GetCellValue(row,COLUMN_FILE).Trim(false);
            wxString projectURL = m_pGridPane->GetCellValue(row,COLUMN_HIDDEN_URL).Trim(false);
            pDoc->TransferAbort(fileName, projectURL);		
        }		
    }
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTransfersGrid::OnTransfersAbort - Function End"));
}


wxInt32 CViewTransfersGrid::GetDocCount() {
    return wxGetApp().GetDocument()->GetTransferCount();
}

void CViewTransfersGrid::UpdateSelection() {
    CTaskItemGroup* pGroup = m_TaskGroups[0];

    CBOINCBaseView::PreUpdateSelection();

	if (m_pGridPane->GetSelectedRows2().size() > 0) {
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }

    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewTransfersGrid::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        strBuffer = wxT(" ") + HtmlEntityDecode(wxString(transfer->project_name.c_str(), wxConvUTF8));
    }
    return 0;
}


wxInt32 CViewTransfersGrid::FormatFileName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        strBuffer = wxT(" ") + wxString(transfer->name.c_str(), wxConvUTF8);
    }
    return 0;
}


wxInt32 CViewTransfersGrid::FormatProgress(wxInt32 item, wxString& strBuffer) const {
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


wxInt32 CViewTransfersGrid::FormatSize(wxInt32 item, wxString& strBuffer) const {
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

	strBuffer = wxT(" ") + strBuffer;

    return 0;
}


wxInt32 CViewTransfersGrid::FormatTime(wxInt32 item, wxString& strBuffer) const {
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

    strBuffer = wxT(" ") + ts.Format();

    return 0;
}


wxInt32 CViewTransfersGrid::FormatSpeed(wxInt32 item, wxString& strBuffer) const {
    float          fTransferSpeed = 0;
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        if (transfer->xfer_active)
            fTransferSpeed = transfer->xfer_speed / 1024;
        else
            fTransferSpeed = 0.0;
    }

    strBuffer.Printf(wxT(" %.2f KBps"), fTransferSpeed);

    return 0;
}


wxInt32 CViewTransfersGrid::FormatStatus(wxInt32 item, wxString& strBuffer) const {
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

	strBuffer = wxT(" ") + strBuffer;

    return 0;
}


wxInt32 CViewTransfersGrid::FormatProjectURL(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    wxASSERT(transfer);

    if (transfer) {
        strBuffer = wxString(transfer->project_url.c_str(), wxConvUTF8);
    }

    return 0;
}

bool CViewTransfersGrid::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    if (!m_pGridPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;
}


bool CViewTransfersGrid::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
	wxASSERT(m_pGridPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    if (!m_pGridPane->OnRestoreState(pConfig)) {
        return false;
    }

    return true;
}

void CViewTransfersGrid::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
    wxInt32 docCount = GetDocCount();

    // We haven't connected up to the CC yet, there is nothing to display, make sure
    //   everything is deleted.
    if ( docCount <= 0 ) {
        if ( m_pGridPane->GetNumberRows() ) {
            m_pGridPane->DeleteRows(0, m_pGridPane->GetNumberRows());
        }
        return;
    }
    
    // Right-size the grid so that the number of rows matches
    //   the document state.
    if(docCount != m_pGridPane->GetNumberRows()) {
        if (docCount > m_pGridPane->GetNumberRows()) {
    	    m_pGridPane->AppendRows(docCount - m_pGridPane->GetNumberRows());
        } else {
            m_pGridPane->DeleteRows(0, m_pGridPane->GetNumberRows() - docCount);
    	    m_bForceUpdateSelection = true;
        }
        wxASSERT(docCount == m_pGridPane->GetNumberRows());
    }

    m_bIgnoreUIEvents = true;
    m_pGridPane->SaveSelection();
    m_bIgnoreUIEvents = false;

	//update cell values
	wxString strBuffer;
    int iMax = m_pGridPane->GetNumberRows();
    for(int iRow = 0; iRow < iMax; iRow++) {

        FormatProjectName(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_PROJECT) != strBuffer) {
		    m_pGridPane->SetCellValue(iRow, COLUMN_PROJECT, strBuffer);
        }

		FormatFileName(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_FILE) != strBuffer) {
		    m_pGridPane->SetCellValue(iRow, COLUMN_FILE, strBuffer);
        }

        FormatProgress(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_PROGRESS) != strBuffer) {
		    m_pGridPane->SetCellValue(iRow, COLUMN_PROGRESS, strBuffer);
		    m_pGridPane->SetCellAlignment(iRow, COLUMN_PROGRESS, wxALIGN_CENTRE, wxALIGN_CENTRE);
        }

        FormatSize(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_SIZE) != strBuffer) {
		    m_pGridPane->SetCellValue(iRow, COLUMN_SIZE, strBuffer);
        }

        FormatTime(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_TIME) != strBuffer) {
		    m_pGridPane->SetCellValue(iRow, COLUMN_TIME, strBuffer);
        }
		
        FormatSpeed(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_SPEED) != strBuffer) {
		    m_pGridPane->SetCellValue(iRow, COLUMN_SPEED, strBuffer);
        }

        strBuffer = wxEmptyString;
        FormatStatus(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_STATUS) != strBuffer) {
    		m_pGridPane->SetCellValue(iRow, COLUMN_STATUS, strBuffer);
        }

        FormatProjectURL(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_HIDDEN_URL) != strBuffer) {
    		m_pGridPane->SetCellValue(iRow, COLUMN_HIDDEN_URL, strBuffer);
        }
    }

    m_pGridPane->SortData();

    m_bIgnoreUIEvents = true;
    m_pGridPane->RestoreSelection();
    m_bIgnoreUIEvents = false;

    UpdateSelection();
}

