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
#pragma implementation "ViewMessagesGrid.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCGridCtrl.h"
#include "ViewMessagesGrid.h"
#include "Events.h"


#include "res/mess.xpm"

// column indexes
#define COLUMN_PROJECT              0
#define COLUMN_SEQNO				1
#define COLUMN_PRIO					2
#define COLUMN_TIME                 3
#define COLUMN_MESSAGE              4

// buttons in the "tasks" area
#define BTN_COPYALL      0
#define BTN_COPYSELECTED 1


IMPLEMENT_DYNAMIC_CLASS(CViewMessagesGrid, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewMessagesGrid, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYALL, CViewMessagesGrid::OnMessagesCopyAll)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYSELECTED, CViewMessagesGrid::OnMessagesCopySelected)
	EVT_GRID_SELECT_CELL(CViewMessagesGrid::OnGridSelectCell)
	EVT_GRID_RANGE_SELECT(CViewMessagesGrid::OnGridSelectRange)
END_EVENT_TABLE ()


CViewMessagesGrid::CViewMessagesGrid()
{}


CViewMessagesGrid::CViewMessagesGrid(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook)
{
    //
    // Setup View
    //
    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);

	m_pTaskPane = new CBOINCTaskCtrl(this, ID_TASK_MESSAGESGRIDVIEW, DEFAULT_TASK_FLAGS);
    wxASSERT(m_pTaskPane);

	m_pGridPane = new CBOINCGridCtrl(this, ID_LIST_MESSAGESGRIDVIEW);
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
        _("Copy all messages"),
        _("Copy all the messages to the clipboard."),
        ID_TASK_MESSAGES_COPYALL
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Copy selected messages"),
#ifdef __WXMAC__
        _("Copy the selected messages to the clipboard. "
          "You can select multiple messages by holding down the shift "
          "or command key while clicking on messages."),
#else
        _("Copy the selected messages to the clipboard. "
          "You can select multiple messages by holding down the shift "
          "or control key while clicking on messages."),
#endif
        ID_TASK_MESSAGES_COPYSELECTED
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

	// Create Grid
	m_pGridPane->Setup();
	m_pGridPane->SetTable(new CBOINCGridTable(1,5));
	m_pGridPane->SetSelectionMode(wxGrid::wxGridSelectRows);
	// init grid columns
	wxInt32 colSizes[] = {115,40,40,145,550};
	wxString colTitles[] = {_("Project"),_("ID"),_("Priority"),_("Time"),_("Message")};
	for(int i=0; i<= COLUMN_MESSAGE;i++){
		m_pGridPane->SetColLabelValue(i,colTitles[i]);
		m_pGridPane->SetColSize(i,colSizes[i]);
	}	
	//change the default cell renderer
	m_pGridPane->SetDefaultRenderer(new CBOINCGridCellMessageRenderer(COLUMN_PRIO));
	//set column sort types
	m_pGridPane->SetColumnSortType(COLUMN_TIME,CST_TIME);
	m_pGridPane->SetColumnSortType(COLUMN_SEQNO,CST_LONG);
	//set primary key column index
	m_pGridPane->SetPrimaryKeyColumns(COLUMN_SEQNO,-1);

    UpdateSelection();
}


CViewMessagesGrid::~CViewMessagesGrid() {
    EmptyTasks();
}


wxString& CViewMessagesGrid::GetViewName() {
    static wxString strViewName(_("MessagesGrid"));
    return strViewName;
}


wxString& CViewMessagesGrid::GetViewDisplayName() {
    static wxString strViewName(_("Messages"));
    return strViewName;
}


const char** CViewMessagesGrid::GetViewIcon() {
    return mess_xpm;
}


void CViewMessagesGrid::OnMessagesCopyAll( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessagesGrid::OnMessagesCopyAll - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

#ifdef wxUSE_CLIPBOARD

    wxInt32 iIndex          = -1;
    wxInt32 iRowCount       = 0;
    pFrame->UpdateStatusText(_("Copying all messages to the clipboard..."));
    OpenClipboard();

	iRowCount = m_pGridPane->GetRows();
    for (iIndex = 0; iIndex < iRowCount; iIndex++) {
        CopyToClipboard(iIndex);
    }

    CloseClipboard();
    pFrame->UpdateStatusText(wxT(""));

#endif

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessagesGrid::OnMessagesCopyAll - Function End"));
}


void CViewMessagesGrid::OnMessagesCopySelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessagesGrid::OnMessagesCopySelected - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

#ifdef wxUSE_CLIPBOARD

    pFrame->UpdateStatusText(_("Copying selected messages to Clipboard..."));
    OpenClipboard();

	wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();
	for (unsigned int i=0; i< arrSelRows.GetCount();i++) {
        CopyToClipboard(arrSelRows[i]);
    }

    CloseClipboard();
    pFrame->UpdateStatusText(wxT(""));

#endif

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessagesGrid::OnMessagesCopySelected - Function End"));
}


wxInt32 CViewMessagesGrid::GetDocCount() {
    return wxGetApp().GetDocument()->GetMessageCount();
}

bool CViewMessagesGrid::OnSaveState(wxConfigBase* pConfig) {
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


bool CViewMessagesGrid::OnRestoreState(wxConfigBase* pConfig) {
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

void CViewMessagesGrid::OnListRender (wxTimerEvent& WXUNUSED(event)) {
    wxInt32 docCount = GetDocCount();
    wxASSERT(m_pGridPane);

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
        }
        wxASSERT(docCount == m_pGridPane->GetNumberRows());
    }

    m_bIgnoreUIEvents = true;
    m_pGridPane->SaveSelection();
    m_bIgnoreUIEvents = false;

	//update cell values (unsorted, like delivered from core client)
	wxString strBuffer;
    int iMax = m_pGridPane->GetNumberRows();
	for(int iRow = 0; iRow < iMax; iRow++) {

        FormatProjectName(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_PROJECT) != strBuffer) {
		    m_pGridPane->SetCellValue(iRow, COLUMN_PROJECT, strBuffer);
        }

		FormatSeqNo(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_SEQNO) != strBuffer) {
    		m_pGridPane->SetCellValue(iRow, COLUMN_SEQNO, strBuffer);
        }

		FormatPriority(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_PRIO) != strBuffer) {
    		m_pGridPane->SetCellValue(iRow, COLUMN_PRIO, strBuffer);
        }

		FormatTime(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_TIME) != strBuffer) {
    		m_pGridPane->SetCellValue(iRow, COLUMN_TIME, strBuffer);
        }

		FormatMessage(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_MESSAGE) != strBuffer) {
    		m_pGridPane->SetCellValue(iRow, COLUMN_MESSAGE, strBuffer);
        }
    }

    //sorting
	m_pGridPane->SortData();

    m_bIgnoreUIEvents = true;
    m_pGridPane->RestoreSelection();
    m_bIgnoreUIEvents = false;

    UpdateSelection();
}

void CViewMessagesGrid::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;

    CBOINCBaseView::PreUpdateSelection();

    pGroup = m_TaskGroups[0];
	if (m_pGridPane->GetFirstSelectedRow()>=0) {
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_COPYSELECTED]);
    } else {
        m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_COPYSELECTED]);
    }

    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewMessagesGrid::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
		wxString szProjectName = HtmlEntityDecode(wxString(message->project.c_str(), wxConvUTF8));
		if(szProjectName == wxEmptyString) {
			strBuffer = wxT("BOINC core client");
		} else {
			strBuffer = szProjectName;
		}
    }

    return 0;
}

wxInt32 CViewMessagesGrid::FormatTime(wxInt32 item, wxString& strBuffer) const {
    wxDateTime dtBuffer;
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        dtBuffer.Set((time_t)message->timestamp);
        strBuffer = dtBuffer.Format(wxT(" %x %X"));
    }

    return 0;
}

wxInt32 CViewMessagesGrid::FormatSeqNo(wxInt32 item, wxString& strBuffer) const {

    MESSAGE*   message = wxGetApp().GetDocument()->message(item);
    if (message) {
		strBuffer = strBuffer.Format(wxT(" %d"),message->seqno);
    }

    return 0;
}

wxInt32 CViewMessagesGrid::FormatPriority(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        switch(message->priority) {
        case MSG_INFO:
			strBuffer = _("Info");
            break;
        case MSG_USER_ERROR:
			strBuffer = _("Warning");
            break;
        case MSG_INTERNAL_ERROR:
        default:
            strBuffer = _("Error");
            break;
        }
    }
    return 0;
}

wxInt32 CViewMessagesGrid::FormatMessage(wxInt32 item, wxString& strBuffer) const {
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->body.c_str(), wxConvUTF8);
    }

    strBuffer.Replace(wxT("\n"), wxT(""), true);

    return 0;
}


#ifdef wxUSE_CLIPBOARD
bool CViewMessagesGrid::OpenClipboard() {
    bool bRetVal = false;

    bRetVal = wxTheClipboard->Open();
    if (bRetVal) {
        m_bClipboardOpen = true;
        m_strClipboardData = wxEmptyString;
        wxTheClipboard->Clear();
    }

    return bRetVal;
}


wxInt32 CViewMessagesGrid::CopyToClipboard(wxInt32 item) {
    wxInt32        iRetVal = -1;

    if (m_bClipboardOpen) {
        wxString       strBuffer = wxEmptyString;
        wxString       strTimeStamp = wxEmptyString;
        wxString       strProject = wxEmptyString;
        wxString       strMessage = wxEmptyString;

		strTimeStamp = m_pGridPane->GetCellValue(item,COLUMN_TIME).Trim(false);
		strProject = m_pGridPane->GetCellValue(item,COLUMN_PROJECT).Trim(false);
		strMessage = m_pGridPane->GetCellValue(item,COLUMN_MESSAGE).Trim(false);

#ifdef __WXMSW__
        strBuffer.Printf(wxT("%s|%s|%s\r\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#else
        strBuffer.Printf(wxT("%s|%s|%s\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#endif

        m_strClipboardData += strBuffer;

        iRetVal = 0;
    }

    return iRetVal;
}


bool CViewMessagesGrid::CloseClipboard() {
    bool bRetVal = false;

    if (m_bClipboardOpen) {
        wxTheClipboard->SetData(new wxTextDataObject(m_strClipboardData));
        wxTheClipboard->Close();

        m_bClipboardOpen = false;
        m_strClipboardData = wxEmptyString;
    }

    return bRetVal;
}

#endif

