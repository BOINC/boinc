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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

// buttons in the "tasks" area
#define BTN_RETRY       0
#define BTN_ABORT       1



IMPLEMENT_DYNAMIC_CLASS(CViewTransfersGrid, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewTransfersGrid, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_TRANSFERS_RETRYNOW, CViewTransfersGrid::OnTransfersRetryNow)
    EVT_BUTTON(ID_TASK_TRANSFERS_ABORT, CViewTransfersGrid::OnTransfersAbort)
	EVT_GRID_SELECT_CELL( CViewTransfersGrid::OnSelectCell )
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
	m_pGridPane->SetTable(new CBOINCGridTable(1,7));
	m_pGridPane->SetSelectionMode(wxGrid::wxGridSelectRows);
	// init grid columns
	wxInt32 colSizes[] = {125,205,60,80,80,80,150};
	wxString colTitles[] = {_("Project"),_("File"),_("Progress"),_("Size"),_("Elapsed Time"),_("Speed"),_("Status")};
	for(int i=0; i<= COLUMN_STATUS;i++){
		m_pGridPane->SetColLabelValue(i,colTitles[i]);
		m_pGridPane->SetColSize(i,colSizes[i]);
	}
	//change the default cell renderer
	m_pGridPane->SetDefaultRenderer(new CBOINCGridCellProgressRenderer(COLUMN_PROGRESS));
	//set column sort types
	m_pGridPane->SetColumnSortType(COLUMN_PROGRESS,CST_FLOAT);
	m_pGridPane->SetColumnSortType(COLUMN_TIME,CST_TIME);
	//m_pGridPane->SetColumnSortType(COLUMN_SIZE,CST_FLOAT);
	m_pGridPane->SetColumnSortType(COLUMN_SPEED,CST_FLOAT);
	//
    UpdateSelection();
}


CViewTransfersGrid::~CViewTransfersGrid() {
    EmptyTasks();
}


wxString& CViewTransfersGrid::GetViewName() {
    static wxString strViewName(_("TransfersGrid"));
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
    //pDoc->TransferRetryNow(m_pGridPane->GetFirstSelectedRow());
	wxString searchName = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_FILE).Trim(false);
	pDoc->TransferRetryNow(searchName);
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
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting transfer..."));

	wxString searchName = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_FILE).Trim(false);
    strMessage.Printf(
        _("Are you sure you want to abort this file transfer '%s'?\n"
          "NOTE: Aborting a transfer will invalidate a task and you\n"
          "will not receive credit for it."), 
        wxString(searchName, wxConvUTF8).c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Abort File Transfer"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->TransferAbort(searchName);
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

	if (m_pGridPane->GetFirstSelectedRow() >= 0) {
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }

    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewTransfersGrid::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        strBuffer = wxString(" ", wxConvUTF8) + wxString(transfer->project_name.c_str(), wxConvUTF8);
    }
    return 0;
}


wxInt32 CViewTransfersGrid::FormatFileName(wxInt32 item, wxString& strBuffer) const {
    FILE_TRANSFER* transfer = wxGetApp().GetDocument()->file_transfer(item);

    if (transfer) {
        strBuffer = wxString(" ", wxConvUTF8) + wxString(transfer->name.c_str(), wxConvUTF8);
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

	strBuffer = wxString(" ", wxConvUTF8) + strBuffer;

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

    strBuffer = wxString(" ", wxConvUTF8) + ts.Format();

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

    if (status.task_mode == RUN_MODE_NEVER) {
        strBuffer = wxT(" (") + strBuffer + wxT(") ");
        strBuffer = _("Suspended") + strBuffer;
    }

	strBuffer = wxString(" ", wxConvUTF8) + strBuffer;

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
	//prevent grid from flicker
	m_pGridPane->BeginBatch();
	//remember selected rows 
	wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();
	wxArrayString arrSelNames;
	for(unsigned int i=0; i< arrSelRows.GetCount();i++) {
		arrSelNames.Add(m_pGridPane->GetCellValue(arrSelRows[i],COLUMN_FILE));
	}
	//remember grid cursor position
	int ccol = m_pGridPane->GetGridCursorCol();
	int crow = m_pGridPane->GetGridCursorRow();
	wxString cursorName;
	if(crow>=0 && ccol >=0) {
		cursorName = m_pGridPane->GetCellValue(crow,COLUMN_FILE);
	}
	//(re)create rows, if necessary
	if(this->GetDocCount()!= m_pGridPane->GetRows()) {
		//at first, delet all current rows
		if(m_pGridPane->GetRows()>0) {
			m_pGridPane->DeleteRows(0,m_pGridPane->GetRows());
		}
		//insert new rows
		for(int rownum=0; rownum < this->GetDocCount();rownum++) {		
			m_pGridPane->AppendRows();
		}
	}

	//update cell values
	wxString buffer;
	for(int rownum=0; rownum < this->GetDocCount();rownum++) {				
		this->FormatProjectName(rownum,buffer);
		m_pGridPane->SetCellValue(rownum,COLUMN_PROJECT,buffer);

		this->FormatFileName(rownum,buffer);
		m_pGridPane->SetCellValue(rownum,COLUMN_FILE,buffer);

		this->FormatProgress(rownum,buffer);
		m_pGridPane->SetCellValue(rownum,COLUMN_PROGRESS,buffer);
		m_pGridPane->SetCellAlignment(rownum,COLUMN_PROGRESS,wxALIGN_CENTRE,wxALIGN_CENTRE);

		this->FormatSize(rownum,buffer);
		m_pGridPane->SetCellValue(rownum,COLUMN_SIZE,buffer);

		this->FormatTime(rownum,buffer);
		m_pGridPane->SetCellValue(rownum,COLUMN_TIME,buffer);
		
		this->FormatSpeed(rownum,buffer);
		m_pGridPane->SetCellValue(rownum,COLUMN_SPEED,buffer);

		this->FormatStatus(rownum,buffer);
		m_pGridPane->SetCellValue(rownum,COLUMN_STATUS,buffer);
	}
	m_pGridPane->SortData();
	// restore grid cursor position
	int index = m_pGridPane->GetTable()->FindRowIndexByColValue(COLUMN_FILE,cursorName);
	if(index >=0) {
		m_bIgnoreSelectionEvents =true;
		m_pGridPane->SetGridCursor(index,ccol);		
		m_bIgnoreSelectionEvents =false;
	}
	//restore selection
	for(unsigned int i=0;i < arrSelNames.size();i++) {		
		int index = m_pGridPane->GetTable()->FindRowIndexByColValue(COLUMN_FILE,arrSelNames[i]);
		if(index >=0) {
			m_pGridPane->SelectRow(index);
		}
	}
	m_pGridPane->EndBatch();	
	//
	UpdateSelection();
}

/**
	handle selection events
*/
void CViewTransfersGrid::OnSelectCell( wxGridEvent& ev )
{
    // you must call Skip() if you want the default processing
    // to occur in wxGrid
    ev.Skip();
	if(!m_bIgnoreSelectionEvents) {
		m_bForceUpdateSelection = true;
	}
}
