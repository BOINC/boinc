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
#pragma implementation "ViewMessages.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewMessages.h"
#include "Events.h"


#include "res/mess.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2


IMPLEMENT_DYNAMIC_CLASS(CViewMessages, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewMessages, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYALL, CViewMessages::OnMessagesCopyAll)
    EVT_BUTTON(ID_TASK_MESSAGES_COPYSELECTED, CViewMessages::OnMessagesCopySelected)
    EVT_LIST_ITEM_SELECTED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_MESSAGESVIEW, CViewMessages::OnListDeselected)
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
    m_iPreviousDocCount = 0;


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
        _("Copy the selected messages to the clipboard. "
          "You can select multiple messages by holding down the shift "
          "or control key while clicking on messages."),
        ID_TASK_MESSAGES_COPYSELECTED 
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
    EmptyTasks();
}


wxString& CViewMessages::GetViewName() {
    static wxString strViewName(_("Messages"));
    return strViewName;
}


const char** CViewMessages::GetViewIcon() {
    return mess_xpm;
}


void CViewMessages::OnMessagesCopyAll( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopyAll - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

#ifdef wxUSE_CLIPBOARD

    wxInt32 iIndex          = -1;
    wxInt32 iRowCount       = 0;
    pFrame->UpdateStatusText(_("Copying all messages to the clipboard..."));
    OpenClipboard();

    iRowCount = m_pListPane->GetItemCount();
    for (iIndex = 0; iIndex < iRowCount; iIndex++) {
        CopyToClipboard(iIndex);            
    }

    CloseClipboard();
    pFrame->UpdateStatusText(wxT(""));

#endif

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopyAll - Function End"));
}


void CViewMessages::OnMessagesCopySelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopySelected - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

#ifdef wxUSE_CLIPBOARD

    wxInt32 iIndex = -1;

    pFrame->UpdateStatusText(_("Aborting transfer..."));
    OpenClipboard();

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
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewMessages::OnMessagesCopySelected - Function End"));
}


wxInt32 CViewMessages::GetDocCount() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetMessageCount();
}


void CViewMessages::OnListRender (wxTimerEvent& event) {
    if (!m_bProcessingListRenderEvent) {
        m_bProcessingListRenderEvent = true;

        wxASSERT(m_pListPane);

        wxInt32 iDocCount = GetDocCount();
        if (0 >= iDocCount) {
            m_pListPane->DeleteAllItems();
        } else {
            if (m_iPreviousDocCount != iDocCount)
                m_pListPane->SetItemCount(iDocCount);
        }

        if ((iDocCount) && (_EnsureLastItemVisible()) && (m_iPreviousDocCount != iDocCount)) {
            m_pListPane->EnsureVisible(iDocCount - 1);
        }

        if (m_iPreviousDocCount != iDocCount) {
            m_iPreviousDocCount = iDocCount;
        }

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}


wxString CViewMessages::OnListGetItemText(long item, long column) const {
    wxString        strBuffer   = wxEmptyString;

    switch(column) {
    case COLUMN_PROJECT:
        FormatProjectName(item, strBuffer);
        break;
    case COLUMN_TIME:
        FormatTime(item, strBuffer);
        break;
    case COLUMN_MESSAGE:
        FormatMessage(item, strBuffer);
        break;
    }

    return strBuffer;
}


wxListItemAttr* CViewMessages::OnListGetItemAttr(long item) const {
    wxListItemAttr* pAttribute  = NULL;
    wxString        strBuffer   = wxEmptyString;

    FormatPriority(item, strBuffer);

    if (wxT("E") == strBuffer) {
        pAttribute = m_pMessageErrorAttr;
    } else {
        pAttribute = m_pMessageInfoAttr;
    }

    return pAttribute;
}


bool CViewMessages::EnsureLastItemVisible() {
    return true;
}


void CViewMessages::UpdateSelection() {
    CBOINCBaseView::PreUpdateSelection();
    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewMessages::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->project.c_str());
    }

    return 0;
}


wxInt32 CViewMessages::FormatPriority(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        switch(message->priority) {
        case MSG_PRIORITY_INFO:
            strBuffer = wxT("I");
            break;
        default:
            strBuffer = wxT("E");
            break;
        }
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
        strBuffer = wxString(message->body.c_str());
    }

    strBuffer.Replace(wxT("\n"), wxT(""), true);

    return 0;
}


#ifdef wxUSE_CLIPBOARD
bool CViewMessages::OpenClipboard() {
    bool bRetVal = false;

    bRetVal = wxTheClipboard->Open();
    if (bRetVal) {
        m_bClipboardOpen = true;
        m_strClipboardData = wxEmptyString;
        wxTheClipboard->Clear();
    }

    return bRetVal;
}


wxInt32 CViewMessages::CopyToClipboard(wxInt32 item) {
    wxInt32        iRetVal = -1;

    if (m_bClipboardOpen) {
        wxString       strBuffer = wxEmptyString;
        wxString       strTimeStamp = wxEmptyString;
        wxString       strProject = wxEmptyString;
        wxString       strMessage = wxEmptyString;

        FormatTime(item, strTimeStamp);
        FormatProjectName(item, strProject);
        FormatMessage(item, strMessage);

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


const char *BOINC_RCSID_0be7149475 = "$Id$";
