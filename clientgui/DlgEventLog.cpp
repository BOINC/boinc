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
#pragma implementation "DlgEventLog.h"
#endif

#include "stdwx.h"
#include "common_defs.h"
#include "diagnostics.h"
#include "str_util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "version.h"
#include "DlgEventLogListCtrl.h"
#include "DlgEventLog.h"
#include "AdvancedFrame.h"



////@begin includes
////@end includes

////@begin XPM images
////@end XPM images


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2


static bool s_bIsFiltered = false;
static std::string s_strFilteredProjectName;

/*!
 * CDlgEventLog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgEventLog, wxDialog )

/*!
 * CDlgEventLog event table definition
 */

BEGIN_EVENT_TABLE( CDlgEventLog, wxDialog )
////@begin CDlgEventLog event table entries
    EVT_HELP(wxID_ANY, CDlgEventLog::OnHelp)
    EVT_BUTTON(wxID_OK, CDlgEventLog::OnOK)
    EVT_BUTTON(ID_COPYAll, CDlgEventLog::OnMessagesCopyAll)
    EVT_BUTTON(ID_COPYSELECTED, CDlgEventLog::OnMessagesCopySelected)
    EVT_BUTTON(ID_TASK_MESSAGES_FILTERBYPROJECT, CDlgEventLog::OnMessagesFilter)
    EVT_BUTTON(ID_SIMPLE_HELP, CDlgEventLog::OnButtonHelp)
	EVT_SIZE(CDlgEventLog::OnSize)
    EVT_MOVE(CDlgEventLog::OnMove)
    EVT_CLOSE(CDlgEventLog::OnClose)
////@end CDlgEventLog event table entries
END_EVENT_TABLE()

/*!
 * CDlgEventLog constructors
 */

CDlgEventLog::CDlgEventLog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::CDlgEventLog - Constructor Function Begin"));

    Create(parent, id, caption, pos, size, style);

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::CDlgEventLog - Constructor Function End"));
}


CDlgEventLog::~CDlgEventLog() {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::CDlgEventLog - Destructor Function Begin"));
    
    SaveState();
    SetWindowDimensions();

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

    m_iFilteredIndexes.Clear();

    wxGetApp().OnEventLogClose();

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::CDlgEventLog - Destructor Function End"));
}


/*!
 * CDlgEventLog creator
 */

bool CDlgEventLog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDlgEventLog member initialisation
    m_iPreviousRowCount = 0;
    m_iTotalDocCount = 0;
    m_iPreviousTotalDocCount = 0;
    if (!s_bIsFiltered) {
        s_strFilteredProjectName.clear();
    }
    m_iFilteredIndexes.Clear();
	m_bProcessingRefreshEvent = false;
    m_bEventLogIsOpen = true;
////@end CDlgEventLog member initialisation

    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxPoint oTempPoint;
    wxSize  oTempSize;

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    if ((pos == wxDefaultPosition) && (size == wxDefaultSize)) {
        // Get size and position from the previous configuration
        GetWindowDimensions( oTempPoint, oTempSize );

#ifdef __WXMSW__
        // Windows does some crazy things if the initial position is a negative
        // value.
        oTempPoint.x = wxDefaultCoord;
        oTempPoint.y = wxDefaultCoord;
#endif

#ifdef __WXMAC__
        // If the user has changed the arrangement of multiple 
        // displays, make sure the window title bar is still on-screen.
    if (!IsWindowOnScreen(oTempPoint.x, oTempPoint.y, oTempSize.GetWidth(), oTempSize.GetHeight())) {
        oTempPoint.y = oTempPoint.x = 30;
    }
#endif  // ! __WXMAC__
    } else {
        oTempPoint = pos;
        oTempSize = size;
    }

    wxDialog::Create( parent, id, caption, oTempPoint, oTempSize, style );

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    // Initialize Application Title
    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        strCaption.Printf(_("%s - Event Log"), pSkinAdvanced->GetApplicationName().c_str());
    }
    SetTitle(strCaption);

    // Initialize Application Icon
    wxIconBundle icons;
    icons.AddIcon(*pSkinAdvanced->GetApplicationIcon());
    icons.AddIcon(*pSkinAdvanced->GetApplicationIcon32());
    SetIcons(icons);

    CreateControls();

	// Create List Pane Items
    m_pList->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 109);
    m_pList->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 130);
    m_pList->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, 378);

    m_pMessageInfoAttr = new wxListItemAttr(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
        wxNullFont
    );
    m_pMessageErrorAttr = new wxListItemAttr(
        *wxRED,
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
        wxNullFont
    );
#if EVENT_LOG_STRIPES
    m_pMessageInfoGrayAttr = new wxListItemAttr(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),
        wxColour(240, 240, 240),
        wxNullFont
    );
    m_pMessageErrorGrayAttr = new wxListItemAttr(*wxRED, wxColour(240, 240, 240), wxNullFont);
#else
    m_pMessageInfoGrayAttr = new wxListItemAttr(*m_pMessageInfoAttr);
    m_pMessageErrorGrayAttr = new wxListItemAttr(*m_pMessageErrorAttr);
#endif

    SetTextColor();
    RestoreState();

    return true;
}


/*!
 * Control creation for CDlgEventLog
 */

void CDlgEventLog::CreateControls()
{
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer2->AddGrowableRow(0);
    itemFlexGridSizer2->AddGrowableCol(0);
    SetSizer(itemFlexGridSizer2);

    m_pList = new CDlgEventLogListCtrl(this, ID_SIMPLE_MESSAGESVIEW, EVENT_LOG_DEFAULT_LIST_MULTI_SEL_FLAGS);
    itemFlexGridSizer2->Add(m_pList, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    
    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 12);

    m_pFilterButton = new wxButton(this, ID_TASK_MESSAGES_FILTERBYPROJECT, _("&Show only this project"),  wxDefaultPosition, wxDefaultSize);
    itemBoxSizer4->Add(m_pFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifdef wxUSE_CLIPBOARD
    wxButton* itemButton1 = new wxButton(this, ID_COPYAll, _("Copy &All"), wxDefaultPosition, wxDefaultSize );
    itemButton1->SetHelpText(
        _("Copy all the messages to the clipboard.")
    );
#if wxUSE_TOOLTIPS
    itemButton1->SetToolTip(
        _("Copy all the messages to the clipboard.")
    );
#endif
    itemBoxSizer4->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pCopySelectedButton = new wxButton(this, ID_COPYSELECTED, _("Copy &Selected"),  wxDefaultPosition, wxDefaultSize );
    m_pCopySelectedButton->SetHelpText(
#ifdef __WXMAC__
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or command key while clicking on messages.")
#else
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or control key while clicking on messages.")
#endif
    );
#if wxUSE_TOOLTIPS
    m_pCopySelectedButton->SetToolTip(
#ifdef __WXMAC__
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or command key while clicking on messages.")
#else
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or control key while clicking on messages.")
#endif
    );
#endif
    itemBoxSizer4->Add(m_pCopySelectedButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    wxButton* itemButton44 = new wxButton(this, wxID_OK, _("&Close"),  wxDefaultPosition, wxDefaultSize);
    itemBoxSizer4->Add(itemButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifndef __WXMAC__
    wxContextHelpButton* itemButton45 = new wxContextHelpButton(this);
    itemBoxSizer4->Add(itemButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#else
	wxButton* itemButton45 = new wxButton(this, ID_SIMPLE_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize);
    wxString helpTip;
    helpTip.Printf(_("Get help with %s"), pSkinAdvanced->GetApplicationShortName().c_str());
    itemButton45->SetHelpText(helpTip);
#ifdef wxUSE_TOOLTIPS
	itemButton45->SetToolTip(helpTip);
#endif
    itemBoxSizer4->Add(itemButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    SetFilterButtonText(); 
}


void CDlgEventLog::SetFilterButtonText() {
    if (s_bIsFiltered) {
        m_pFilterButton->SetLabel( _("Show all &messages") );
        m_pFilterButton->SetHelpText( _("Show messages for all projects") );
#ifdef wxUSE_TOOLTIPS
        m_pFilterButton->SetToolTip(_("Show messages for all projects"));
#endif
    } else {
        m_pFilterButton->SetLabel( _("&Show only this project") );
        m_pFilterButton->SetHelpText( _("Show only the messages for the selected project") );
#ifdef wxUSE_TOOLTIPS
        m_pFilterButton->SetToolTip(_("Show only the messages for the selected project"));
#endif
    }
    // Adjust button size for new text
    Layout();
}


/*!
 * Text color selection for CDlgEventLog
 */

void CDlgEventLog::SetTextColor() {
    bool isConnected = wxGetApp().GetDocument()->IsConnected();

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
}


/*!
 * wxEVT_HELP event handler for ID_DLGMESSAGES
 */

void CDlgEventLog::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnHelp - Function Begin"));

    if (IsShown()) {
        wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

        wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple_messages&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            event.GetId()
        );
        wxLaunchDefaultBrowser(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnHelp - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgEventLog::OnOK( wxCommandEvent& WXUNUSED(event) ) {
    Close();
}


/*!
 * wxEVT_CLOSE event handler for CDlgEventLog (window close control clicked)
 */

void CDlgEventLog::OnClose(wxCloseEvent& WXUNUSED(event)) {
    m_bEventLogIsOpen = false;  // User specifically closed window
    Destroy();
}


void CDlgEventLog::OnMessagesFilter( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesFilter - Function Begin"));

    wxInt32 iIndex = -1;
    MESSAGE* message;
    
    wxASSERT(m_pList);

    m_iFilteredIndexes.Clear();
    s_strFilteredProjectName.clear();

    if (s_bIsFiltered) {
        s_bIsFiltered = false;
        m_iFilteredDocCount = m_iTotalDocCount;
    } else {
        iIndex = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (iIndex >= 0) {
             message = wxGetApp().GetDocument()->message(iIndex);
             if ((message->project).size() > 0) {
                s_strFilteredProjectName = message->project;
                s_bIsFiltered = true;
                for (iIndex = 0; iIndex < m_iTotalDocCount; iIndex++) {
                    message = wxGetApp().GetDocument()->message(iIndex);
                    if (message->project.empty() || (message->project == s_strFilteredProjectName)) {
                        m_iFilteredIndexes.Add(iIndex);
                    }

                }
                m_iFilteredDocCount = (int)(m_iFilteredIndexes.GetCount());
           }
        }
    }
    
    SetFilterButtonText();
    
    // Force a complete update
    m_iPreviousRowCount = 0;
    m_pList->DeleteAllItems();
    m_pList->SetItemCount(m_iFilteredDocCount);
    OnRefresh();

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesFilter - Function End"));
}


wxInt32 CDlgEventLog::GetFilteredMessageIndex( wxInt32 iRow) const {
    if (s_bIsFiltered) return m_iFilteredIndexes[iRow];
    return iRow;
}


// Get the (possibly filtered) item count (i.e., the Row count)
wxInt32 CDlgEventLog::GetDocCount() {
    int i;
    
    m_iTotalDocCount = wxGetApp().GetDocument()->GetMessageCount();
    if (m_iTotalDocCount < m_iPreviousTotalDocCount) {
        // Usually due to a disconnect from client
        ResetMessageFiltering();
    }
    
    if (s_bIsFiltered) {
        for (i = m_iPreviousTotalDocCount; i < m_iTotalDocCount; i++) {
            MESSAGE*   message = wxGetApp().GetDocument()->message(i);
            if (message->project.empty() || (message->project == s_strFilteredProjectName)) {
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


/*!
 * called from CMainDocument::HandleCompletedRPC() after wxEVT_RPC_FINISHED event
 */
void CDlgEventLog::OnRefresh() {
    bool isConnected;
    static bool was_connected = false;
    static wxString strLastMachineName = wxEmptyString;
    wxString strNewMachineName = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    
    if (!IsShown()) return;

    if (!m_bProcessingRefreshEvent) {
        m_bProcessingRefreshEvent = true;

        wxASSERT(m_pList);

        wxInt32 iRowCount = GetDocCount();
        if (0 >= iRowCount) {
            m_pList->DeleteAllItems();
            ResetMessageFiltering();
        } else {
            // If connected computer changed, reset message filtering
            isConnected = wxGetApp().GetDocument()->IsConnected();
            if (isConnected) {
                pDoc->GetConnectedComputerName(strNewMachineName);
                if (strLastMachineName != strNewMachineName) {
                    strLastMachineName = strNewMachineName;
                    was_connected = false;
                    ResetMessageFiltering();
                }
            }

            // If connection status changed, adjust color of messages display
            if (was_connected != isConnected) {
                was_connected = isConnected;
                SetTextColor();

                // Force a complete update
                m_pList->DeleteAllItems();
                m_pList->SetItemCount(iRowCount);
                m_iPreviousRowCount = 0;    // Force scrolling to bottom

            } else {
                // Connection status didn't change
                if (m_iPreviousRowCount != iRowCount) {
                    m_pList->SetItemCount(iRowCount);
                }
            }
        }

        if ((iRowCount > 1) && (EnsureLastItemVisible()) && (m_iPreviousRowCount != iRowCount)) {
            m_pList->EnsureVisible(iRowCount - 1);
        }

        if (m_iPreviousRowCount != iRowCount) {
            m_iPreviousRowCount = iRowCount;
        }

        UpdateButtons();

        m_bProcessingRefreshEvent = false;
    }
}


bool CDlgEventLog::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::SaveState - Function Begin"));

    wxString    strBaseConfigLocation = wxEmptyString;
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxListItem      liColumnInfo;
    wxInt32         iIndex = 0;
    wxInt32         iColumnCount = 0;

    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    strBaseConfigLocation = wxString(wxT("/"));
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("EventLogOpen"), m_bEventLogIsOpen);

    //
    // Save Frame State
    //
    strBaseConfigLocation = wxString(wxT("/EventLog/"));
    pConfig->SetPath(strBaseConfigLocation);

    // Convert to a zero based index
    iColumnCount = m_pList->GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT |
        wxLIST_MASK_WIDTH |
        wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        m_pList->GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());
        pConfig->Write(wxT("Width"), m_pList->GetColumnWidth(iIndex)); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::SaveState - Function End"));
    return true;
}


bool CDlgEventLog::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/EventLog/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxListItem      liColumnInfo;
    wxInt32         iIndex = 0;
    wxInt32         iColumnCount = 0;
    wxInt32         iTempValue = 0;

    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    // Convert to a zero based index
    iColumnCount = m_pList->GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT |
        wxLIST_MASK_WIDTH |
        wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        m_pList->GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());
        pConfig->Read(wxT("Width"), &iTempValue, -1);
        if (-1 != iTempValue) {
            liColumnInfo.SetWidth(iTempValue);
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
            m_pList->SetColumnWidth(iIndex,iTempValue); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
        }

        m_pList->SetColumn(iIndex, liColumnInfo);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::RestoreState - Function End"));
    return true;
}


void CDlgEventLog::GetWindowDimensions( wxPoint& position, wxSize& size ) {
    wxString        strBaseConfigLocation = wxString(wxT("/EventLog/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    int             iHeight = 0, iWidth = 0, iTop = 0, iLeft = 0;

    wxASSERT(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("YPos"), &iTop, 30);
    pConfig->Read(wxT("XPos"), &iLeft, 30);
    pConfig->Read(wxT("Width"), &iWidth, 640);
    pConfig->Read(wxT("Height"), &iHeight, 480);

    position.y = iTop;
    position.x = iLeft;
    size.x = iWidth;
    size.y = iHeight;
}


void CDlgEventLog::SetWindowDimensions() {
    wxString        strBaseConfigLocation = wxString(wxT("/EventLog/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);

    if (!IsIconized()) {
        pConfig->SetPath(strBaseConfigLocation);
        pConfig->Write(wxT("XPos"), GetPosition().x);
        pConfig->Write(wxT("YPos"), GetPosition().y);
        pConfig->Write(wxT("Width"), GetSize().x);
        pConfig->Write(wxT("Height"), GetSize().y);
    }
}


void CDlgEventLog::OnSize(wxSizeEvent& event) {
    SetWindowDimensions();
    event.Skip();
}


void CDlgEventLog::OnMove(wxMoveEvent& event) {
    SetWindowDimensions();
    event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYAll
 */

void CDlgEventLog::OnMessagesCopyAll( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesCopyAll - Function Begin"));

#ifdef wxUSE_CLIPBOARD
    wxInt32 iIndex          = -1;
    wxInt32 iRowCount       = 0;

    iRowCount = m_pList->GetItemCount();

    OpenClipboard( iRowCount * 1024 );

    for (iIndex = 0; iIndex < iRowCount; iIndex++) {
        CopyToClipboard(iIndex);            
    }

    CloseClipboard();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesCopyAll - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYSELECTED
 */

void CDlgEventLog::OnMessagesCopySelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesCopySelected - Function Begin"));

#ifdef wxUSE_CLIPBOARD
    wxInt32 iIndex = -1;
    wxInt32 iRowCount = 0;

    // Count the number of items selected
    for (;;) {
        iIndex = m_pList->GetNextItem(
            iIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED
        );
        if (iIndex == -1) break;

        iRowCount++;            
    }

    OpenClipboard( iRowCount * 1024 );

    // Reset the position indicator
    iIndex = -1;

    for (;;) {
        iIndex = m_pList->GetNextItem(
            iIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED
        );
        if (iIndex == -1) break;

        CopyToClipboard(iIndex);            
    }

    CloseClipboard();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesCopySelected - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
 */

void CDlgEventLog::OnButtonHelp( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnHelp - Function Begin"));

    if (IsShown()) {
    	wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

		wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple_messages&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            event.GetId()
        );
        wxLaunchDefaultBrowser(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnHelp - Function End"));
}


void CDlgEventLog::ResetMessageFiltering() {
    s_bIsFiltered = false;
    s_strFilteredProjectName.clear();
    m_iFilteredIndexes.Clear();
    SetFilterButtonText();
}


void CDlgEventLog::UpdateButtons() {
    bool enableFilterButton = s_bIsFiltered; 
    bool enableCopySelectedButon = false; 
    if (m_iTotalDocCount > 0) {
        int n = m_pList->GetSelectedItemCount();
        if (n > 0) {
            enableCopySelectedButon = true;
        }
        
        if ((n == 1) && (! s_bIsFiltered)) {
            n = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            MESSAGE* message = wxGetApp().GetDocument()->message(n);
            if ((message->project).size() > 0) {
                enableFilterButton = true;
            }
        }
    }
    m_pFilterButton->Enable(enableFilterButton);
    m_pCopySelectedButton->Enable(enableCopySelectedButon);
}


wxString CDlgEventLog::OnListGetItemText(long item, long column) const {
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


wxListItemAttr* CDlgEventLog::OnListGetItemAttr(long item) const {
    wxListItemAttr* pAttribute  = NULL;
    wxInt32         index       = GetFilteredMessageIndex(item);
    MESSAGE*        message     = wxGetApp().GetDocument()->message(index);

    // If we are using some theme where the default background color isn't
    //   white, then our whole system is boned. Use defaults instead.
    if (wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW) != wxColor(wxT("WHITE"))) return NULL;

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


bool CDlgEventLog::EnsureLastItemVisible() {
    int numVisible = m_pList->GetCountPerPage();

    // Auto-scroll only if already at bottom of list
    if ((m_iPreviousRowCount > numVisible)
         && ((m_pList->GetTopItem() + numVisible) < (m_iPreviousRowCount-1)) 
    ) {
        return false;
    }
    
    return true;
}


wxInt32 CDlgEventLog::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->project.c_str(), wxConvUTF8);
    }

    return 0;
}


wxInt32 CDlgEventLog::FormatTime(wxInt32 item, wxString& strBuffer) const {
    wxDateTime dtBuffer;
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        dtBuffer.Set((time_t)message->timestamp);
        strBuffer = dtBuffer.Format();
    }

    return 0;
}


wxInt32 CDlgEventLog::FormatMessage(wxInt32 item, wxString& strBuffer) const {
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);
    
    if (message) {
        strBuffer = wxString(message->body.c_str(), wxConvUTF8);
        wxGetApp().GetDocument()->LocalizeNoticeText(strBuffer, false, true);
    }

    return 0;
}


#ifdef wxUSE_CLIPBOARD
bool CDlgEventLog::OpenClipboard( wxInt32 size ) {
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


wxInt32 CDlgEventLog::CopyToClipboard(wxInt32 item) {
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
        strBuffer.Printf(wxT("%s | %s | %s\r\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#else
        strBuffer.Printf(wxT("%s | %s | %s\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str());
#endif

        m_strClipboardData += strBuffer;

        iRetVal = 0;
    }

    return iRetVal;
}


bool CDlgEventLog::CloseClipboard() {
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

