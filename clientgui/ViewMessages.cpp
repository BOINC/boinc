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
#pragma implementation "ViewMessages.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewMessages.h"
#include "Events.h"


#include "res/mess.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

// buttons in the "tasks" area
#define BTN_COPYALL      0
#define BTN_COPYSELECTED 1
#define BTN_FILTERMSGS   2


IMPLEMENT_DYNAMIC_CLASS(CViewMessages, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewMessages, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYALL, CViewMessages::OnMessagesCopyAll)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYSELECTED, CViewMessages::OnMessagesCopySelected)
    EVT_BUTTON(ID_TASK_MESSAGES_FILTERBYPROJECT, CViewMessages::OnMessagesFilter)
    EVT_LIST_ITEM_SELECTED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListDeselected)
    EVT_LIST_CACHE_HINT(ID_LIST_MESSAGESVIEW, CViewMessages::OnCacheHint)
END_EVENT_TABLE ()


CViewMessages::CViewMessages()
{}


CViewMessages::CViewMessages(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_MESSAGESVIEW, DEFAULT_TASK_FLAGS, ID_LIST_MESSAGESVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS)
{
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);


    //
    // Initialize variables used in later parts of the class
    //
    m_iPreviousRowCount = 0;
    m_iTotalDocCount = 0;
    m_iPreviousTotalDocCount = 0;
    m_bIsFiltered = false;
    m_strFilteredProjectName.clear();
    m_iFilteredIndexes.Clear();
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
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or command key while clicking on messages."),
#else
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or control key while clicking on messages."),
#endif
        ID_TASK_MESSAGES_COPYSELECTED 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show only this project"),
        _("Show only the messages for the selected project."),
        ID_TASK_MESSAGES_FILTERBYPROJECT 
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 115);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 145);
    m_pListPane->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, 550);

    m_pMessageInfoAttr = new wxListItemAttr(*wxBLACK, *wxWHITE, wxNullFont);
    m_pMessageErrorAttr = new wxListItemAttr(*wxRED, *wxWHITE, wxNullFont);
    m_pMessageInfoGrayAttr = new wxListItemAttr(*wxBLACK, wxColour(240, 240, 240), wxNullFont);
    m_pMessageErrorGrayAttr = new wxListItemAttr(*wxRED, wxColour(240, 240, 240), wxNullFont);

    UpdateSelection();
}


CViewMessages::~CViewMessages() {
    if (m_pMessageInfoAttr) {
        delete m_pMessageInfoAttr;
        m_pMessageInfoAttr = NULL;
    }

    if (m_pMessageErrorAttr) {
        delete m_pMessageErrorAttr;
        m_pMessageErrorAttr = NULL;
    }

    if (m_pMessageInfoGrayAttr) {
        delete m_pMessageInfoGrayAttr;
        m_pMessageInfoGrayAttr = NULL;
    }

    if (m_pMessageErrorGrayAttr) {
        delete m_pMessageErrorGrayAttr;
        m_pMessageErrorGrayAttr = NULL;
    }

    EmptyTasks();
    m_strFilteredProjectName.clear();
    m_iFilteredIndexes.Clear();
}


wxString& CViewMessages::GetViewName() {
    static wxString strViewName(wxT("Messages"));
    return strViewName;
}


wxString& CViewMessages::GetViewDisplayName() {
    static wxString strViewName(_("Messages"));
    return strViewName;
}


const char** CViewMessages::GetViewIcon() {
    return mess_xpm;
}


void CViewMessages::OnMessagesCopyAll( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopyAll - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

#ifdef wxUSE_CLIPBOARD

    wxInt32 iIndex          = -1;
    wxInt32 iRowCount       = 0;
    pFrame->UpdateStatusText(_("Copying all messages to the clipboard..."));

    iRowCount = m_pListPane->GetItemCount();

    OpenClipboard( iRowCount * 1024 );

    for (iIndex = 0; iIndex < iRowCount; iIndex++) {
        CopyToClipboard(iIndex);            
    }

    CloseClipboard();

    pFrame->UpdateStatusText(wxT(""));

#endif

    UpdateSelection();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopyAll - Function End"));
}


void CViewMessages::OnMessagesCopySelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopySelected - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

#ifdef wxUSE_CLIPBOARD

    wxInt32 iIndex          = -1;
    wxInt32 iRowCount       = 0;

    pFrame->UpdateStatusText(_("Copying selected messages to the clipboard..."));


    // Count the number of items selected
    for (;;) {
        iIndex = m_pListPane->GetNextItem(
            iIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED
        );
        if (iIndex == -1) break;

        iRowCount++;            
    }

    OpenClipboard( iRowCount * 1024 );

    // Reset the position indicator
    iIndex = -1;


    // Copy selected items to clipboard
    for (;;) {
        iIndex = m_pListPane->GetNextItem(
            iIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED
        );
        if (iIndex == -1) break;

        CopyToClipboard(iIndex);            
    }

    CloseClipboard();

    pFrame->UpdateStatusText(wxT(""));

#endif

    UpdateSelection();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopySelected - Function End"));
}


void CViewMessages::OnMessagesFilter( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesFilter - Function Begin"));

    wxInt32 iIndex = -1;
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    MESSAGE* message;
    
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    m_iFilteredIndexes.Clear();
    m_strFilteredProjectName.clear();

    if (m_bIsFiltered) {
        m_bIsFiltered = false;
        m_iFilteredDocCount = m_iTotalDocCount;
    } else {
        iIndex = m_pListPane->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (iIndex >= 0) {
             message = wxGetApp().GetDocument()->message(iIndex);
             if ((message->project).size() > 0) {
                pFrame->UpdateStatusText(_("Filtering messages..."));
                m_strFilteredProjectName = message->project;
                m_bIsFiltered = true;
                for (iIndex = 0; iIndex < m_iTotalDocCount; iIndex++) {
                    message = wxGetApp().GetDocument()->message(iIndex);
                    if (message->project.empty() || (message->project == m_strFilteredProjectName)) {
                        m_iFilteredIndexes.Add(iIndex);
                    }

                }
                m_iFilteredDocCount = (int)(m_iFilteredIndexes.GetCount());
           }
        }
    }
    
    // Force a complete update
    m_iPreviousRowCount = 0;
    m_pListPane->DeleteAllItems();
    m_pListPane->SetItemCount(m_iFilteredDocCount);
    UpdateSelection();
    pFrame->FireRefreshView();
    pFrame->UpdateStatusText(wxT(""));

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesFilter - Function End"));
}


wxInt32 CViewMessages::GetFilteredMessageIndex( wxInt32 iRow) const {
    if (m_bIsFiltered) return m_iFilteredIndexes[iRow];
    return iRow;
}


// Get the (possibly filtered) item count (i.e., the Row count)
wxInt32 CViewMessages::GetDocCount() {
    int i;
    
    m_iTotalDocCount = wxGetApp().GetDocument()->GetMessageCount();
    if (m_iTotalDocCount < m_iPreviousTotalDocCount) {
        // Usually due to a disconnect from client
        m_bIsFiltered = false;
        m_strFilteredProjectName.clear();
        m_iFilteredIndexes.Clear();
        UpdateSelection();
    }
    
    if (m_bIsFiltered) {
        for (i = m_iPreviousTotalDocCount; i < m_iTotalDocCount; i++) {
            MESSAGE*   message = wxGetApp().GetDocument()->message(i);
            if (message->project.empty() || (message->project == m_strFilteredProjectName)) {
                m_iFilteredIndexes.Add(i);
            }
        }
        m_iPreviousTotalDocCount = m_iTotalDocCount;
        m_iFilteredDocCount = (int)(m_iFilteredIndexes.GetCount());
        return m_iFilteredDocCount;
    }

    m_iPreviousTotalDocCount = m_iTotalDocCount;
    m_iFilteredDocCount = m_iTotalDocCount;
    return m_iTotalDocCount;
}


void CViewMessages::OnListRender (wxTimerEvent& event) {
    bool isConnected;
    static bool was_connected = false;
    static wxString strLastMachineName = wxEmptyString;
    wxString strNewMachineName = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    
    if (!m_bProcessingListRenderEvent) {
        m_bProcessingListRenderEvent = true;

        wxASSERT(m_pListPane);

        isConnected = pDoc->IsConnected();
        wxInt32 iRowCount = GetDocCount();
        if (0 >= iRowCount) {
            m_pListPane->DeleteAllItems();
        } else {
            // If connection status changed, adjust color of messages display
            if (was_connected != isConnected) {
                was_connected = isConnected;
                if (isConnected) {
                    m_pMessageInfoAttr->SetTextColour(*wxBLACK);
                    m_pMessageErrorAttr->SetTextColour(*wxRED);
                    m_pMessageInfoGrayAttr->SetTextColour(*wxBLACK);
                    m_pMessageErrorGrayAttr->SetTextColour(*wxRED);
                } else {
                    wxColourDatabase colorBase;
                    m_pMessageInfoAttr->SetTextColour(wxColour(128, 128, 128));
                    m_pMessageErrorAttr->SetTextColour(wxColour(255, 128, 128));
                    m_pMessageInfoGrayAttr->SetTextColour(wxColour(128, 128, 128));
                    m_pMessageErrorGrayAttr->SetTextColour(wxColour(255, 128, 128));
                }
                // Force a complete update
                m_pListPane->DeleteAllItems();
                m_pListPane->SetItemCount(iRowCount);
                m_iPreviousRowCount = 0;    // Force scrolling to bottom
            } else {
                // Connection status didn't change
                if (m_iPreviousRowCount != iRowCount) {
                    m_pListPane->SetItemCount(iRowCount);
                }
            }
        }

        if ((iRowCount>1) && (_EnsureLastItemVisible()) && (m_iPreviousRowCount != iRowCount)) {
            m_pListPane->EnsureVisible(iRowCount - 1);
        }

        if (isConnected) {
            pDoc->GetConnectedComputerName(strNewMachineName);
            if (strLastMachineName != strNewMachineName) {
                strLastMachineName = strNewMachineName;
                     if (iRowCount) {
                        m_pListPane->EnsureVisible(iRowCount - 1);
                    }
            }
        }

        if (m_iPreviousRowCount != iRowCount) {
            m_iPreviousRowCount = iRowCount;
        }

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}


wxString CViewMessages::OnListGetItemText(long item, long column) const {
    wxString        strBuffer   = wxEmptyString;
    wxInt32         index       = GetFilteredMessageIndex(item);
    
    switch(column) {
    case COLUMN_PROJECT:
        FormatProjectName(index, strBuffer);
        break;
    case COLUMN_TIME:
        FormatTime(index, strBuffer);
        break;
    case COLUMN_MESSAGE:
        FormatMessage(index, strBuffer);
        break;
    }

    return strBuffer;
}


wxListItemAttr* CViewMessages::OnListGetItemAttr(long item) const {
    wxListItemAttr* pAttribute  = NULL;
    wxInt32         index       = GetFilteredMessageIndex(item);
    MESSAGE*        message     = wxGetApp().GetDocument()->message(index);

    if (message) {
        switch(message->priority) {
        case MSG_USER_ALERT:
            pAttribute = item % 2 ? m_pMessageErrorGrayAttr : m_pMessageErrorAttr;
            break;
        default:
            pAttribute = item % 2 ? m_pMessageInfoGrayAttr : m_pMessageInfoAttr;
            break;
        }
    }

    return pAttribute;
}


bool CViewMessages::EnsureLastItemVisible() {
    int numVisible = m_pListPane->GetCountPerPage();

    // Auto-scroll only if already at bottom of list
    if ((m_iPreviousRowCount > numVisible)
         && ((m_pListPane->GetTopItem() + numVisible) < (m_iPreviousRowCount-1)) 
    ) {
        return false;
    }
    
    return true;
}


void CViewMessages::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    MESSAGE*            message;

    CBOINCBaseView::PreUpdateSelection();

    pGroup = m_TaskGroups[0];
    int n = m_pListPane->GetSelectedItemCount();
    if (m_iTotalDocCount <= 0) n = 0;

    if (n > 0) {
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_COPYSELECTED]);
    } else {
        m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_COPYSELECTED]);
    }
    
    if (m_bIsFiltered) {
        m_pTaskPane->UpdateTask(
            pGroup->m_Tasks[BTN_FILTERMSGS], 
            _("Show all messages"), 
            _("Show messages for all projects.")
        );
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_FILTERMSGS]);
          
    } else {
        m_pTaskPane->UpdateTask(
            pGroup->m_Tasks[BTN_FILTERMSGS], 
            _("Show only this project"),
            _("Show only the messages for the selected project.")
        );
        m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_FILTERMSGS]);
        if (n == 1) {
            n = m_pListPane->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            message = wxGetApp().GetDocument()->message(n);
            if ((message->project).size() > 0) {
                m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_FILTERMSGS]);
            }
        }
    }
    
    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewMessages::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = HtmlEntityDecode(wxString(message->project.c_str(), wxConvUTF8));
    }

    return 0;
}


wxInt32 CViewMessages::FormatTime(wxInt32 item, wxString& strBuffer) const {
    wxDateTime dtBuffer;
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        dtBuffer.Set((time_t)message->timestamp);
        strBuffer = dtBuffer.Format();
    }

    return 0;
}


wxInt32 CViewMessages::FormatMessage(wxInt32 item, wxString& strBuffer) const {
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = process_client_message(message->body.c_str());
    }
    return 0;
}


#ifdef wxUSE_CLIPBOARD
bool CViewMessages::OpenClipboard( wxInt32 size ) {
    bool bRetVal = false;

    bRetVal = wxTheClipboard->Open();
    if (bRetVal) {
        m_bClipboardOpen = true;

        m_strClipboardData = wxEmptyString;
        m_strClipboardData.Alloc( size );

        wxTheClipboard->Clear();
    }

    return bRetVal;
}


wxInt32 CViewMessages::CopyToClipboard(wxInt32 item) {
    wxInt32        iRetVal = -1;
    wxInt32        index   = GetFilteredMessageIndex(item);

    if (m_bClipboardOpen) {
        wxString       strBuffer = wxEmptyString;
        wxString       strTimeStamp = wxEmptyString;
        wxString       strProject = wxEmptyString;
        wxString       strMessage = wxEmptyString;

        FormatTime(index, strTimeStamp);
        FormatProjectName(index, strProject);
        FormatMessage(index, strMessage);

#ifdef __WXMSW__
        strBuffer.Printf(wxT("%s\t%s\t%s\r\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#else
        strBuffer.Printf(wxT("%s\t%s\t%s\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#endif

        m_strClipboardData += strBuffer;

        iRetVal = 0;
    }

    return iRetVal;
}


bool CViewMessages::CloseClipboard() {
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


const char *BOINC_RCSID_0be7149475 = "$Id: ViewMessages.cpp 21706 2010-06-08 18:56:53Z davea $";
