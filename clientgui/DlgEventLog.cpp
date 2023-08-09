// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
#include "DlgDiagnosticLogFlags.h"

#ifdef __WXMAC__
#include <time.h>
#endif

////@begin includes
////@end includes

////@begin XPM images
////@end XPM images

const int dlgEventlogInitialWidth = 640;
const int dlgEventLogInitialHeight = 480;
const int dlgEventlogMinWidth = 600;
const int dlgEventlogMinHeight = 250;

#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2


static bool s_bIsFiltered = false;
static bool s_bFilteringChanged = false;
static bool s_bErrorIsFiltered = false;
static bool s_bErrorFilteringChanged = false;
static std::string s_strFilteredProjectName;

/*!
 * CDlgEventLog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgEventLog, DlgEventLogBase )

/*!
 * CDlgEventLog event table definition
 */

BEGIN_EVENT_TABLE( CDlgEventLog, DlgEventLogBase )
////@begin CDlgEventLog event table entries
    EVT_ACTIVATE(CDlgEventLog::OnActivate)
    EVT_HELP(wxID_ANY, CDlgEventLog::OnHelp)
    EVT_BUTTON(wxID_OK, CDlgEventLog::OnOK)
    EVT_BUTTON(ID_COPYAll, CDlgEventLog::OnMessagesCopyAll)
    EVT_BUTTON(ID_COPYSELECTED, CDlgEventLog::OnMessagesCopySelected)
    EVT_BUTTON(ID_TASK_MESSAGES_FILTERBYERROR, CDlgEventLog::OnErrorFilter)
    EVT_BUTTON(ID_TASK_MESSAGES_FILTERBYPROJECT, CDlgEventLog::OnMessagesFilter)
    EVT_BUTTON(ID_SIMPLE_HELP, CDlgEventLog::OnButtonHelp)
	EVT_MENU(ID_SGDIAGNOSTICLOGFLAGS, CDlgEventLog::OnDiagnosticLogFlags)
	EVT_SIZE(CDlgEventLog::OnSize)
    EVT_MOVE(CDlgEventLog::OnMove)
    EVT_CLOSE(CDlgEventLog::OnClose)
    EVT_LIST_COL_END_DRAG(ID_SIMPLE_MESSAGESVIEW, CDlgEventLog::OnColResize)
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
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    m_iPreviousRowCount = 0;
    m_iTotalDocCount = 0;
    m_iPreviousFirstMsgSeqNum = pDoc->GetFirstMsgSeqNum();
    m_iPreviousLastMsgSeqNum = m_iPreviousFirstMsgSeqNum - 1;

    m_iNumDeletedFilteredRows = 0;
    m_iTotalDeletedFilterRows = 0;

    if (!s_bIsFiltered) {
        s_strFilteredProjectName.clear();
    }
    m_iFilteredIndexes.Clear();
	m_bProcessingRefreshEvent = false;
    m_bWasConnected = false;
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
        // Get the current display space for the current window
		int iDisplay = wxNOT_FOUND;
		if ( wxGetApp().GetFrame() != NULL )
            iDisplay = wxDisplay::GetFromWindow(wxGetApp().GetFrame());
		if ( iDisplay == wxNOT_FOUND )
            iDisplay = 0;
        wxDisplay *display = new wxDisplay(iDisplay);
        wxRect rDisplay = display->GetClientArea();

		// Check that the saved height and width is not larger than the displayable space.
		// If it is, then reduce the size.
        if ( oTempSize.GetWidth() > rDisplay.width ) oTempSize.SetWidth(rDisplay.width);
        if ( oTempSize.GetHeight() > rDisplay.height ) oTempSize.SetHeight(rDisplay.height);

        // Check if part of the display was going to be off the screen, if so, center the
        // display on that axis
		if ( oTempPoint.x < rDisplay.x ) {
			oTempPoint.x = rDisplay.x;
		} else if ( oTempPoint.x + oTempSize.GetWidth() > rDisplay.x + rDisplay.width ) {
			oTempPoint.x = rDisplay.x + rDisplay.width - oTempSize.GetWidth();
		}

		if ( oTempPoint.y < rDisplay.y ) {
			oTempPoint.y = rDisplay.y;
		} else if ( oTempPoint.y + oTempSize.GetHeight() > rDisplay.y + rDisplay.height ) {
			oTempPoint.y = rDisplay.y + rDisplay.height - oTempSize.GetHeight();
		}

        delete display;
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

    DlgEventLogBase::Create( parent, id, caption, oTempPoint, oTempSize, style );

    SetSizeHints(dlgEventlogMinWidth, dlgEventlogMinHeight);
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    // Initialize Application Title
    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        strCaption.Printf(_("%s - Event Log"), pSkinAdvanced->GetApplicationName().c_str());
    }
    SetTitle(strCaption);

    // Initialize Application Icon
    SetIcons(*pSkinAdvanced->GetApplicationIcon());

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
    m_pList->EnableAlternateRowColours();
    wxColour stripe_color;

#if wxCHECK_VERSION(3, 1, 0)
    stripe_color = m_pList->GetAlternateRowColour();
    if (!stripe_color.IsOk())
#endif
    {
        // copied from wxListCtrlBase::EnableAlternateRowColours(bool)

        // Determine the alternate rows colour automatically from the
        // background colour.
        const wxColour bgColour = m_pList->GetBackgroundColour();

        // Depending on the background, alternate row color
        // will be 3% more dark or 50% brighter.
        int alpha = bgColour.GetRGB() > 0x808080 ? 97 : 150;
        stripe_color = bgColour.ChangeLightness(alpha);
    }

#ifdef __WXMSW__
    // work around a bug in wxWidgets 3.1 and earlier
    // if row background color is wxSYS_COLOR_BTNFACE selected unfocused row is drawn with wrong colors
    if (stripe_color == wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)) {
        // adjust the color just enough to make it different
        stripe_color.SetRGB(stripe_color.GetRGB() + 1);
    }
#endif

    m_pMessageInfoGrayAttr = new wxListItemAttr(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),
        stripe_color,
        wxNullFont
    );
    m_pMessageErrorGrayAttr = new wxListItemAttr(
        *wxRED,
        stripe_color,
        wxNullFont
    );
#else
    m_pMessageInfoGrayAttr = new wxListItemAttr(*m_pMessageInfoAttr);
    m_pMessageErrorGrayAttr = new wxListItemAttr(*m_pMessageErrorAttr);
#endif

    SetTextColor();
    RestoreState();
    OnRefresh();
    // Register that we had the Event Log open immediately
    SaveState();

    m_Shortcuts[0].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (int)'F', ID_SGDIAGNOSTICLOGFLAGS);
    m_pAccelTable = new wxAcceleratorTable(1, m_Shortcuts);

    SetAcceleratorTable(*m_pAccelTable);

    return true;
}


/*!
 * Control creation for CDlgEventLog
 */

void CDlgEventLog::CreateControls()
{
    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer2->AddGrowableRow(0);
    itemFlexGridSizer2->AddGrowableCol(0);
    SetSizer(itemFlexGridSizer2);

    m_pList = new CDlgEventLogListCtrl(this, ID_SIMPLE_MESSAGESVIEW, EVENT_LOG_DEFAULT_LIST_MULTI_SEL_FLAGS);
    itemFlexGridSizer2->Add(m_pList, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);

    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 12);

    m_pErrorFilterButton = new wxButton(this, ID_TASK_MESSAGES_FILTERBYERROR, _("Show only aler&ts"), wxDefaultPosition, wxDefaultSize);
    itemBoxSizer4->Add(m_pErrorFilterButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

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
    if (s_bErrorIsFiltered) {
        m_pErrorFilterButton->SetLabel(_("Show al&l"));
        m_pErrorFilterButton->SetHelpText(_("Shows messages of all types (information, alerts, etc.)"));
#ifdef wxUSE_TOOLTIPS
        m_pErrorFilterButton->SetToolTip(_("Shows messages of all types (information, alerts, etc.)"));
#endif
    }
    else {
        m_pErrorFilterButton->SetLabel(_("Show only aler&ts"));
        m_pErrorFilterButton->SetHelpText(_("Shows only the messages that are alerts"));
#ifdef wxUSE_TOOLTIPS
        m_pErrorFilterButton->SetToolTip(_("Show only the messages that are alerts"));
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
 * wxEVT_ACTIVATE event handler for ID_DLGMESSAGES
 */

void CDlgEventLog::OnActivate(wxActivateEvent& event) {
    bool isActive = event.GetActive();
    if (isActive) wxGetApp().SetEventLogWasActive(true);
    event.Skip();
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
    SaveState();
    SetWindowDimensions();

    Close();
}


/*!
 * wxEVT_CLOSE event handler for CDlgEventLog (window close control clicked)
 */

void CDlgEventLog::OnClose(wxCloseEvent& WXUNUSED(event)) {
    m_bEventLogIsOpen = false;  // User specifically closed window

    SaveState();
    SetWindowDimensions();

    Destroy();
}


// Function used to filter errors displayed in the Event Log.
// This Function will first check if errors are currently filtered.  Then, regardless if true or false,
// will evaluate if a project filter is active or not.  Depending on the combination of error and
// project filters, the filtered list will be updated.  One of three possibilities could happen:
// 1.  Restarting from the original event log list (if error filter was active and it is being deactivated)
// 2.  Restarting the list and re-filtering (if both filters were active, but then one is being deactivated)
// 3.  Filtering the currently filtered list (if one filter was active and the other filter is being activated).
// After filtering is completed, then the buttons in the event log window are updated and the event log list
// is updated to reflect the changes made to the list.
//
void CDlgEventLog::OnErrorFilter(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnErrorFilter - Function Begin"));
    wxASSERT(m_pList);
    if (s_bErrorIsFiltered) {
        // Errors are currently filtered.  Whether a project filter is enabled or not,
        // the list will need to be reset to the original list first.
        //
        s_bErrorIsFiltered = false;
        m_iFilteredDocCount = m_iTotalDocCount;
        m_iFilteredIndexes.Clear();
        m_iTotalDeletedFilterRows = 0;
        // Now that the settings are changed, need to determine if project filtering is currently
        // enabled or not.  If it is not enabled, do nothing.  If it is enabled, need to re-filter
        // by the project.
        //
        if (s_bIsFiltered) {  // Errors are currently filtered and a project is filtered
            FindProjectMessages(false);
        }
    }
    else {
        // Errors are not currently filtered.  Take the list as it is and filter out errors.
        s_bErrorIsFiltered = true;
        if (!s_bIsFiltered) {  // Errors are not currently filtered but are filtered by a project.  Filter from the current list.
            m_iFilteredDocCount = m_iTotalDocCount;
            m_iFilteredIndexes.Clear();
            m_iTotalDeletedFilterRows = 0;
        }
        FindErrorMessages(s_bIsFiltered);
    }

    s_bErrorFilteringChanged = true;
    SetFilterButtonText();
    // Force a complete update
    m_iPreviousRowCount = 0;
    m_pList->DeleteAllItems();
    m_pList->SetItemCount(m_iFilteredDocCount);
    OnRefresh();
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnErrorFilter - Function End"));
}


void CDlgEventLog::OnMessagesFilter( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesFilter - Function Begin"));

    wxInt32 iIndex = -1;
    wxASSERT(m_pList);

    if (s_bIsFiltered) {
        // Event log is filtering by a project.  Whether the error filter is enabled or not,
        // the list will need to be reset to the original list first.
        //
        s_bIsFiltered = false;
        m_iFilteredDocCount = m_iTotalDocCount;
        m_iFilteredIndexes.Clear();
        s_strFilteredProjectName.clear();
        m_iTotalDeletedFilterRows = 0;
        // Now that the settings are changed, need to determine if error filtering is currently
        // enabled or not.  If it is not enabled, do nothing.  If it is enabled, need to re-filter
        // by errors.
        //
        if (s_bErrorIsFiltered) {  // List is currently filtered by project and by error.
            FindErrorMessages(false);
        }
    } else {  // List will now be filtered by a project.
        // Get project name to be filtered.
        s_strFilteredProjectName.clear();
        MESSAGE* message;
        iIndex = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (iIndex >= 0) {
            message = wxGetApp().GetDocument()->message(GetFilteredMessageIndex(iIndex));
            if (message) {
             if ((message->project).size() > 0) {
                s_strFilteredProjectName = message->project;
                    }
                }
           }
        if (!s_bErrorIsFiltered) {
            // List is not filtered by errors nor by a project, so clear filtered indexes, count, etc.
            m_iFilteredIndexes.Clear();
            m_iFilteredDocCount = m_iTotalDocCount;
            m_iTotalDeletedFilterRows = 0;
        }
        FindProjectMessages(s_bErrorIsFiltered);
        s_bIsFiltered = true;
    }

    s_bFilteringChanged = true;
    SetFilterButtonText();

    // Force a complete update
    m_iPreviousRowCount = 0;
    m_pList->DeleteAllItems();
    m_pList->SetItemCount(m_iFilteredDocCount);
    OnRefresh();

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnMessagesFilter - Function End"));
}


wxInt32 CDlgEventLog::GetFilteredMessageIndex( wxInt32 iRow) const {
    if (s_bIsFiltered || s_bErrorIsFiltered) return m_iFilteredIndexes[iRow];
    return iRow;
}


// NOTE: this function is designed to be called only
// from CDlgEventLog::OnRefresh().  If you need to call it
// from other routines, it will need modification.
//
// Get the (possibly filtered) item count (i.e., the Row count) and
// make any needed adjustments if oldest items have been deleted.
//
wxInt32 CDlgEventLog::GetDocCount() {
    int i, j, numDeletedRows;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    m_iTotalDocCount = pDoc->GetMessageCount();

    numDeletedRows = pDoc->GetFirstMsgSeqNum() - m_iPreviousFirstMsgSeqNum;
    if ((numDeletedRows < 0) || (m_iPreviousFirstMsgSeqNum < 0)) {
        numDeletedRows = 0;
    }
    m_iNumDeletedFilteredRows = 0;

    if (s_bIsFiltered || s_bErrorIsFiltered) {
        if (numDeletedRows > 0) {
            // Remove any deleted messages from our filtered list
            while (m_iFilteredIndexes.GetCount() > 0) {
                if (m_iFilteredIndexes[0] >= numDeletedRows) break;
                m_iFilteredIndexes.RemoveAt(0);
                m_iNumDeletedFilteredRows++;
                m_iTotalDeletedFilterRows++;
            }

            // Adjust the remaining indexes
            for (i = m_iFilteredIndexes.GetCount()-1; i >= 0; i--) {
                m_iFilteredIndexes[i] -= numDeletedRows;
            }
        }

        // Add indexes of new messages to filtered list as appropriate
        i = m_iTotalDocCount - (pDoc->GetLastMsgSeqNum() - m_iPreviousLastMsgSeqNum);
        if (i < 0) i = 0;
        for (; i < m_iTotalDocCount; i++) {
            MESSAGE* message = pDoc->message(i);
                if (message) {
                    if (s_bIsFiltered) {
                        if (s_bErrorIsFiltered) {
                            if (message->priority == MSG_USER_ALERT && (message->project.empty() || message->project == s_strFilteredProjectName)) {
                                m_iFilteredIndexes.Add(i);
                            }
                        } else if (message->project.empty() || message->project == s_strFilteredProjectName) {
                            m_iFilteredIndexes.Add(i);
                        }
                    } else if (s_bErrorIsFiltered && message->priority == MSG_USER_ALERT) {
                m_iFilteredIndexes.Add(i);
            }
        }
            }
        m_iFilteredDocCount = (int)(m_iFilteredIndexes.GetCount());
    } else {
        m_iFilteredDocCount = m_iTotalDocCount;
        m_iNumDeletedFilteredRows = numDeletedRows;
    }

    if (numDeletedRows > 0) {
        // Adjust the selected row numbers
        wxArrayInt arrSelRows;

        i = -1;
        for (;;) {
            i = m_pList->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (i < 0) break;
            arrSelRows.Add(i);
        }
        int count = arrSelRows.Count();
        for (i=0; i<count; i++) {
            m_pList->SetItemState(arrSelRows[i], 0, wxLIST_STATE_SELECTED);
        }

        for (i=0; i<count; i++) {
            if ((j = arrSelRows[i] - m_iNumDeletedFilteredRows) >= 0) {
                m_pList->SetItemState(j, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
    }

    return (s_bIsFiltered || s_bErrorIsFiltered) ? m_iFilteredDocCount : m_iTotalDocCount;
}


/*!
 * called from CMainDocument::HandleCompletedRPC() after wxEVT_RPC_FINISHED event
 */
void CDlgEventLog::OnRefresh() {
    bool isConnected;
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
        long topItem = m_pList->GetTopItem();

        // If the total rows is negative then it is presumed that something went wrong.
        // This conditional resets message filtering, clears the list of event logs that were to
        // be displayed.  This also happens if the row count is zero, since there should always be
        // a message.  One exception to this is when error filtering is enabled since it is possible
        // for no errors to occur.
        //
        if ((0 >= iRowCount) && !s_bErrorIsFiltered) {
            m_pList->DeleteAllItems();
            ResetMessageFiltering();
            m_iPreviousFirstMsgSeqNum = 0;
            m_iPreviousLastMsgSeqNum = 0;
        } else {
            // If connected computer changed, reset message filtering
            isConnected = wxGetApp().GetDocument()->IsConnected();
            if (isConnected) {
                pDoc->GetConnectedComputerName(strNewMachineName);
                if (strLastMachineName != strNewMachineName) {
                    strLastMachineName = strNewMachineName;
                    m_bWasConnected = false;
                    ResetMessageFiltering();
                    m_iPreviousFirstMsgSeqNum = pDoc->GetFirstMsgSeqNum();
                    m_iPreviousLastMsgSeqNum = m_iPreviousFirstMsgSeqNum - 1;
                    iRowCount = m_iTotalDocCount;   // In case we had filtering set
                }
            }

            // If connection status changed, adjust color of messages display
            if (m_bWasConnected != isConnected) {
                m_bWasConnected = isConnected;
                SetTextColor();

                // Force a complete update
                m_pList->DeleteAllItems();
                m_pList->SetItemCount(iRowCount);
                m_iPreviousRowCount = 0;    // Force scrolling to bottom

            } else {
                // Connection status didn't change
                if (m_iPreviousLastMsgSeqNum != pDoc->GetLastMsgSeqNum()) {
                    if (m_iPreviousRowCount == iRowCount) {
                        m_pList->Refresh();
                    } else {
                        m_pList->SetItemCount(iRowCount);
                    }
                }
            }
        }

        if (iRowCount > 1) {
            if (s_bFilteringChanged) {
                m_pList->EnsureVisible(iRowCount - 1);
                s_bFilteringChanged = false;
            } else if (s_bErrorFilteringChanged) {
                m_pList->EnsureVisible(iRowCount - 1);
                s_bErrorFilteringChanged = false;
            } else {
                if (m_iPreviousLastMsgSeqNum != pDoc->GetLastMsgSeqNum()) {
                    if (EnsureLastItemVisible()) {
                        m_pList->EnsureVisible(iRowCount - 1);
                    } else if (topItem > 0) {
                        int n = topItem - m_iNumDeletedFilteredRows;
                        if (n < 0) n = 0;
                        Freeze();   // Avoid flicker if selected rows are visible
                        m_pList->EnsureVisible(n);
                        Thaw();
                    }
                }
            }
        }

        m_iPreviousRowCount = iRowCount;
        if (m_iTotalDocCount > 0) {
            m_iPreviousFirstMsgSeqNum = pDoc->GetFirstMsgSeqNum();
            m_iPreviousLastMsgSeqNum = pDoc->GetLastMsgSeqNum();
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
    pConfig->Read(wxT("Width"), &iWidth, dlgEventlogInitialWidth);
    pConfig->Read(wxT("Height"), &iHeight, dlgEventLogInitialHeight);

    // Guard against a rare situation where registry values are zero
    if (iWidth < 50) iWidth = dlgEventlogInitialWidth;
    if (iHeight < 50) iHeight = dlgEventLogInitialHeight;
    position.y = iTop;
    position.x = iLeft;
    size.x = std::max(iWidth, dlgEventlogMinWidth);
    size.y = std::max(iHeight, dlgEventlogMinHeight);
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


void CDlgEventLog::OnDiagnosticLogFlags(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnDiagnosticLogFlags - Function Begin"));

    CDlgDiagnosticLogFlags dlg(this);
	dlg.ShowModal();

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgEventLog::OnDiagnosticLogFlags - Function End"));
}


void CDlgEventLog::OnColResize( wxListEvent& ) {
    // Register the new column widths immediately
    SaveState();
}

void CDlgEventLog::ResetMessageFiltering() {
    s_bFilteringChanged = false;
    s_bErrorFilteringChanged = false;
    s_bIsFiltered = false;
    s_bErrorIsFiltered = false;
    s_strFilteredProjectName.clear();
    m_iFilteredIndexes.Clear();
    SetFilterButtonText();
    m_iTotalDeletedFilterRows = 0;
}


// Function to search through messages and find messages that contain an error.  This function first reads the input
// to see if it should search all indexes or a pre-filtered set of indexes.  Then, it will search through the indexes.
// For each index that is an error (priority == MSG_USER_ALERT), it will add that message's index to a temporary
// array of indexes.  After the for loop is complete, the recently filtered indexes will be written to m_iFilteredIndexes
// and the recently filtered indexes will be stored in m_iFilteredDocCount.
//
// The following input variable is required:
//  isfiltered:  If the wxArrayInt that we want to search is already filtered (m_iFilteredIndexes), this will be true.
//                 If we want to search all indexes (m_iTotalIndexes), this will be false).
//
void CDlgEventLog::FindErrorMessages(bool isFiltered) {
    wxArrayInt filteredindexes;
    MESSAGE* message;
    wxInt32 i = 0;
    if (isFiltered) {
        for (i=0; i < m_iFilteredDocCount; i++) {
            message = wxGetApp().GetDocument()->message(GetFilteredMessageIndex(i));
            if (message) {
                if (message->priority == MSG_USER_ALERT) {
                    filteredindexes.Add(GetFilteredMessageIndex(i));
                }
            }
        }
    }
    else {
        for (i=0; i < m_iTotalDocCount; i++) {
            message = wxGetApp().GetDocument()->message(i);
            if (message) {
                if (message->priority == MSG_USER_ALERT) {
                    filteredindexes.Add(i);
                }
            }
        }
    }
    m_iFilteredIndexes = filteredindexes;
    m_iFilteredDocCount = static_cast<wxInt32>(filteredindexes.GetCount());
}


// Function to search through messages and find all messages that are associated with a specific project.  Messages that do
// not have a project are included in the filtered indexes.
//
// This function first reads the input to see if it should search all indexes or a pre-filtered set of indexes.
// Then, it will search through those indexes.  For each message that matches the associated
// project (s_strFilteredProjectName) or has no associated project, it will add that message's index to a temporary
// array of indexes.  After the for loop is complete, the recently filtered indexes will be written to
// m_iFilteredIndexes and the recently filtered indexes will be stored in m_iFilteredDocCount.
//
// It is assumed s_strFilteredProjectName is correct prior to this function being called.
//
// The following input variable is required:
//  isfiltered:  If the wxArrayInt that we want to search is already filtered (m_iFilteredIndexes), this will be true.
//                 If we want to search all indexes (m_iTotalIndexes), this will be false).
//
void CDlgEventLog::FindProjectMessages(bool isFiltered) {
    wxArrayInt filteredindexes;
    MESSAGE* message;
    wxInt32 i = 0;
    if (isFiltered) {
        for (i=0; i < m_iFilteredDocCount; i++) {
            message = wxGetApp().GetDocument()->message(GetFilteredMessageIndex(i));
            if (message) {
                if (message->project.empty() || message->project == s_strFilteredProjectName) {
                    filteredindexes.Add(GetFilteredMessageIndex(i));
                }
            }
        }
    }
    else {
        for (i=0; i < m_iTotalDocCount; i++) {
            message = wxGetApp().GetDocument()->message(i);
            if (message) {
                if (message->project.empty() || message->project == s_strFilteredProjectName) {
                    filteredindexes.Add(i);
                }
            }
        }
    }
    m_iFilteredIndexes = filteredindexes;
    m_iFilteredDocCount = static_cast<wxInt32>(m_iFilteredIndexes.GetCount());
}


void CDlgEventLog::UpdateButtons() {
    bool enableFilterButton = s_bIsFiltered;
    bool enableCopySelectedButton = false;
    if (m_iTotalDocCount > 0) {
        int n = m_pList->GetSelectedItemCount();
        if (n > 0) {
            enableCopySelectedButton = true;
        }

        if ((n == 1) && (! s_bIsFiltered)) {
            n = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            MESSAGE* message = wxGetApp().GetDocument()->message(GetFilteredMessageIndex(n));
            if ((message->project).size() > 0) {
                enableFilterButton = true;
            }
        }
    }
    m_pFilterButton->Enable(enableFilterButton);
    m_pCopySelectedButton->Enable(enableCopySelectedButton);
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
        item += s_bIsFiltered ? m_iTotalDeletedFilterRows : m_iPreviousFirstMsgSeqNum;
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
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
#ifdef __WXMAC__
        // Work around a wxCocoa bug(?) in wxDateTime::Format()
        char buf[80];
        // When building as a 64-bit app, we must convert
        // 4-byte int message->timestamp to 8-byte time_t
        time_t timeStamp = message->timestamp;
        struct tm * timeinfo = localtime(&timeStamp);
        strftime(buf, sizeof(buf), "%c", timeinfo);
        strBuffer = buf;
#else
        wxDateTime dtBuffer;
        dtBuffer.Set((time_t)message->timestamp);
        strBuffer = dtBuffer.Format();
#endif
    }

    return 0;
}


wxInt32 CDlgEventLog::FormatMessage(wxInt32 item, wxString& strBuffer) const {
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->body.c_str(), wxConvUTF8);
        remove_eols(strBuffer);
        localize(strBuffer);
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

#if defined(__WXGTK__) || defined(__WXQT__)
        wxTheClipboard->UsePrimarySelection(false);
#endif
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

