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
#pragma implementation "sg_DlgMessages.h"
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

#include "sg_DlgMessages.h"
#include "sg_SGUIListControl.h"



////@begin includes
////@end includes

////@begin XPM images
////@end XPM images


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

/*!
 * CPanelPreferences type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPanelMessages, wxPanel )

/*!
 * CPanelPreferences event table definition
 */

BEGIN_EVENT_TABLE( CPanelMessages, wxPanel )
////@begin CPanelPreferences event table entries
    EVT_ERASE_BACKGROUND( CPanelMessages::OnEraseBackground )
    EVT_BUTTON( wxID_OK, CPanelMessages::OnOK )
    EVT_BUTTON(ID_COPYAll, CPanelMessages::OnMessagesCopyAll)
    EVT_BUTTON(ID_COPYSELECTED, CPanelMessages::OnMessagesCopySelected)
    EVT_BUTTON(ID_SIMPLE_HELP, CPanelMessages::OnButtonHelp)
////@end CPanelPreferences event table entries
END_EVENT_TABLE()

/*!
 * CPanelMessages constructors
 */

CPanelMessages::CPanelMessages( )
{
}


CPanelMessages::CPanelMessages( wxWindow* parent ) :  
    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
    Create();
}


/*!
 * CPanelMessages creator
 */

bool CPanelMessages::Create()
{
////@begin CPanelMessages member initialisation
    m_iPreviousDocCount = 0;
	m_bProcessingRefreshEvent = false;
////@end CPanelMessages member initialisation

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

	// Create List Pane Items
    m_pList->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 109);
    m_pList->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 130);
    m_pList->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, 378);

    m_pMessageInfoAttr = new wxListItemAttr(*wxBLACK, *wxWHITE, wxNullFont);
    m_pMessageErrorAttr = new wxListItemAttr(*wxRED, *wxWHITE, wxNullFont);

    return true;
}


CPanelMessages::~CPanelMessages()
{
	if (m_pMessageInfoAttr) {
        delete m_pMessageInfoAttr;
        m_pMessageInfoAttr = NULL;
    }

    if (m_pMessageErrorAttr) {
        delete m_pMessageErrorAttr;
        m_pMessageErrorAttr = NULL;
    }

}


/*!
 * Control creation for CPanelPreferences
 */

void CPanelMessages::CreateControls()
{
    CPanelMessages* itemDialog1 = this;
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer2->AddGrowableRow(0);
    itemFlexGridSizer2->AddGrowableCol(0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    m_pList = new CSGUIListCtrl(this, ID_SIMPLE_MESSAGESVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS);
    itemFlexGridSizer2->Add(m_pList, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    
#ifdef __WXMAC__            // Don't let Close button overlap window's grow icon
    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 12);
#else
    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

#ifdef wxUSE_CLIPBOARD
    wxBitmapButton* itemButton1 = new wxBitmapButton(
        this,
        ID_COPYAll,
        *pSkinSimple->GetCopyAllButton()->GetBitmap(),
        wxDefaultPosition,
        wxSize(
            (*pSkinSimple->GetCopyAllButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetCopyAllButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetCopyAllButton()->GetBitmapClicked() != NULL ) {
		itemButton1->SetBitmapSelected(*pSkinSimple->GetCopyAllButton()->GetBitmapClicked());
	}
    itemButton1->SetHelpText(
        _("Copy all the messages to the clipboard.")
    );
#if wxUSE_TOOLTIPS
    itemButton1->SetToolTip(
        _("Copy all the messages to the clipboard.")
    );
#endif
    itemBoxSizer4->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemButton2 = new wxBitmapButton(
        this,
        ID_COPYSELECTED,
        *pSkinSimple->GetCopyButton()->GetBitmap(),
        wxDefaultPosition,
        wxSize(
            (*pSkinSimple->GetCopyButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetCopyButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetCopyButton()->GetBitmapClicked() != NULL ) {
		itemButton2->SetBitmapSelected(*pSkinSimple->GetCopyButton()->GetBitmapClicked());
	}
    itemButton2->SetHelpText(
#ifdef __WXMAC__
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or command key while clicking on messages.")
#else
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or control key while clicking on messages.")
#endif
    );
#if wxUSE_TOOLTIPS
    itemButton2->SetToolTip(
#ifdef __WXMAC__
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or command key while clicking on messages.")
#else
        _("Copy the selected messages to the clipboard. You can select multiple messages by holding down the shift or control key while clicking on messages.")
#endif
    );
#endif
    itemBoxSizer4->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    wxBitmapButton* itemBitmapButton44 = new wxBitmapButton(
        this,
        wxID_OK,
        *pSkinSimple->GetCloseButton()->GetBitmap(),
        wxDefaultPosition,
        wxSize(
            (*pSkinSimple->GetCloseButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetCloseButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetCloseButton()->GetBitmapClicked() != NULL ) {
		itemBitmapButton44->SetBitmapSelected(*pSkinSimple->GetCloseButton()->GetBitmapClicked());
	}
    itemBoxSizer4->Add(itemBitmapButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifndef __WXMSW__
#ifdef __WXMAC__
	wxBitmapButton* itemButton45 = new wxBitmapButton(
        this,
        ID_SIMPLE_HELP,
        *pSkinSimple->GetHelpButton()->GetBitmap(),
        wxDefaultPosition,
        wxSize(
            (*pSkinSimple->GetHelpButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetHelpButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetHelpButton()->GetBitmapClicked() != NULL ) {
		itemButton45->SetBitmapSelected(*pSkinSimple->GetHelpButton()->GetBitmapClicked());
	}
#ifdef wxUSE_TOOLTIPS
	itemButton45->SetToolTip(new wxToolTip(_("Get help with BOINC")));
#endif
    itemBoxSizer4->Add(itemButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#else
    wxContextHelpButton* itemButton45 = new wxContextHelpButton(this);
    itemBoxSizer4->Add(itemButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif
#endif

}


/*!
 * wxEVT_ERASE_BACKGROUND event handler for ID_DLGMESSAGES
 */

void CPanelMessages::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
    
    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxMemoryDC memDC;
    wxCoord w, h, x, y;

    // Get the desired background bitmap
    wxBitmap bmp(*pSkinSimple->GetDialogBackgroundImage()->GetBitmap());

    // Dialog dimensions
    wxSize sz = GetClientSize();

    // Create a buffered device context to reduce flicker
    wxBufferedDC dc(event.GetDC(), sz, wxBUFFER_CLIENT_AREA);

    // bitmap dimensions
    w = bmp.GetWidth();
    h = bmp.GetHeight();

    // Fill the dialog with a magenta color so people can detect when something
    //   is wrong
    dc.SetBrush(wxBrush(wxColour(255,0,255)));
    dc.SetPen(wxPen(wxColour(255,0,255)));
    dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());

    // Is the bitmap smaller than the window?
    if ( (w < sz.x) || (h < sz.y) ) {
        // Check to see if they need to be rescaled to fit in the window
        wxImage img = bmp.ConvertToImage();
        img.Rescale((int) sz.x, (int) sz.y);

        // Draw our cool background (centered)
        dc.DrawBitmap(wxBitmap(img), 0, 0);
    } else {
        // Snag the center of the bitmap and use it
        //   for the background image
        x = wxMax(0, (w - sz.x)/2);
        y = wxMax(0, (h - sz.y)/2);

        // Select the desired bitmap into the memory DC so we can take
        //   the center chunk of it.
        memDC.SelectObject(bmp);

        // Draw the center chunk on the window
        dc.Blit(0, 0, w, h, &memDC, x, y, wxCOPY);

        // Drop the bitmap
        memDC.SelectObject(wxNullBitmap);
    }
}


/*!
 * called from CSimpleFrame::OnRefreshView()
 */

void CPanelMessages::OnRefresh() {
    bool isConnected;
    static bool was_connected = false;
    
    if (!m_bProcessingRefreshEvent) {
        m_bProcessingRefreshEvent = true;

        wxASSERT(m_pList);

        wxInt32 iDocCount = wxGetApp().GetDocument()->GetMessageCount();
        if (0 >= iDocCount) {
            m_pList->DeleteAllItems();
        } else {
            // If connection status changed, adjust color of messages display
            isConnected = wxGetApp().GetDocument()->IsConnected();
            if (was_connected != isConnected) {
                was_connected = isConnected;
                if (isConnected) {
                    m_pMessageInfoAttr->SetTextColour(*wxBLACK);
                    m_pMessageErrorAttr->SetTextColour(*wxRED);
                } else {
                    wxColourDatabase colorBase;
                    m_pMessageInfoAttr->SetTextColour(wxColour(128, 128, 128));
                    m_pMessageErrorAttr->SetTextColour(wxColour(255, 128, 128));
                }
                // Force a complete update
                m_pList->DeleteAllItems();
                m_pList->SetItemCount(iDocCount);
                 m_iPreviousDocCount = 0;    // Force scrolling to bottom
            } else {
                // Connection status didn't change
                if (m_iPreviousDocCount != iDocCount) {
                    m_pList->SetItemCount(iDocCount);
                }
            }
        }

        if ((iDocCount > 1) && (EnsureLastItemVisible()) && (m_iPreviousDocCount != iDocCount)) {
            m_pList->EnsureVisible(iDocCount - 1);
        }

        if (m_iPreviousDocCount != iDocCount) {
            m_iPreviousDocCount = iDocCount;
        }

        m_bProcessingRefreshEvent = false;
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CPanelMessages::OnOK( wxCommandEvent& event ) {
    event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYAll
 */

void CPanelMessages::OnMessagesCopyAll( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnMessagesCopyAll - Function Begin"));

#ifdef wxUSE_CLIPBOARD
    wxInt32 iIndex          = -1;
    wxInt32 iRowCount       = 0;
    OpenClipboard();

    iRowCount = m_pList->GetItemCount();
    for (iIndex = 0; iIndex < iRowCount; iIndex++) {
        CopyToClipboard(iIndex);            
    }

    CloseClipboard();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnMessagesCopyAll - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYSELECTED
 */

void CPanelMessages::OnMessagesCopySelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnMessagesCopySelected - Function Begin"));

#ifdef wxUSE_CLIPBOARD
    wxInt32 iIndex = -1;

    OpenClipboard();

    for (;;) {
        iIndex = m_pList->GetNextItem(
            iIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED
        );
        if (iIndex == -1) break;

        CopyToClipboard(iIndex);            
    }

    CloseClipboard();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnMessagesCopySelected - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
 */

void CPanelMessages::OnButtonHelp( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnHelp - Function Begin"));

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

    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnHelp - Function End"));
}


bool CPanelMessages::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

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

        pConfig->Write(wxT("Width"), liColumnInfo.GetWidth());
        
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
        pConfig->Write(wxT("Width"), m_pList->GetColumnWidth(iIndex)); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
    }


    return true;
}


bool CPanelMessages::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;
    wxInt32     iTempValue = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

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

    return true;
}


wxString CPanelMessages::OnListGetItemText(long item, long column) const {
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


wxListItemAttr* CPanelMessages::OnListGetItemAttr(long item) const {
    wxListItemAttr* pAttribute  = NULL;
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        switch(message->priority) {
        case MSG_USER_ALERT:
            pAttribute = m_pMessageErrorAttr;
            break;
        default:
            pAttribute = m_pMessageInfoAttr;
            break;
        }
    }

    return pAttribute;
	
}


bool CPanelMessages::EnsureLastItemVisible() {
    int numVisible = m_pList->GetCountPerPage();

    // Auto-scroll only if already at bottom of list
    if ((m_iPreviousDocCount > numVisible)
         && ((m_pList->GetTopItem() + numVisible) < (m_iPreviousDocCount-1)) 
    ) {
        return false;
    }
    
    return true;
}


wxInt32 CPanelMessages::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->project.c_str(), wxConvUTF8);
    }

    return 0;
}


wxInt32 CPanelMessages::FormatTime(wxInt32 item, wxString& strBuffer) const {
    wxDateTime dtBuffer;
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        dtBuffer.Set((time_t)message->timestamp);
        strBuffer = dtBuffer.Format();
    }

    return 0;
}


wxInt32 CPanelMessages::FormatMessage(wxInt32 item, wxString& strBuffer) const {
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->body.c_str(), wxConvUTF8);
    }

    strBuffer.Replace(wxT("\n"), wxT(""), true);

    return 0;
}


#ifdef wxUSE_CLIPBOARD
bool CPanelMessages::OpenClipboard() {
    bool bRetVal = false;

    bRetVal = wxTheClipboard->Open();
    if (bRetVal) {
        m_bClipboardOpen = true;
        m_strClipboardData = wxEmptyString;
        wxTheClipboard->Clear();
    }

    return bRetVal;
}


wxInt32 CPanelMessages::CopyToClipboard(wxInt32 item) {
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


bool CPanelMessages::CloseClipboard() {
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


/*!
 * CDlgMessages type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgMessages, wxDialog )

/*!
 * CDlgMessages event table definition
 */

BEGIN_EVENT_TABLE( CDlgMessages, wxDialog )
////@begin CDlgMessages event table entries
    EVT_HELP(wxID_ANY, CDlgMessages::OnHelp)
    EVT_SHOW( CDlgMessages::OnShow )
    EVT_BUTTON( wxID_OK, CDlgMessages::OnOK )
////@end CDlgMessages event table entries
END_EVENT_TABLE()

/*!
 * CDlgMessages constructors
 */

CDlgMessages::CDlgMessages( )
{
}


CDlgMessages::CDlgMessages( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}


CDlgMessages::~CDlgMessages() {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::CDlgMessages - Destructor Function Begin"));

	SaveState();    // Save state if close box on window frame clicked

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::CDlgMessages - Destructor Function End"));
}


/*!
 * CDlgMessages creator
 */

bool CDlgMessages::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    CSkinAdvanced*         pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        
    SetExtraStyle(GetExtraStyle()|wxDIALOG_EX_CONTEXTHELP|wxWS_EX_BLOCK_EVENTS);

    wxDialog::Create( parent, id, caption, pos, size, style );

    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        strCaption.Printf(_("%s - Messages"), pSkinAdvanced->GetApplicationName().c_str());
    }
    SetTitle(strCaption);

    // Initialize Application Icon
    wxIconBundle icons;
    icons.AddIcon(*pSkinAdvanced->GetApplicationIcon());
    icons.AddIcon(*pSkinAdvanced->GetApplicationIcon32());
    SetIcons(icons);

    Freeze();

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

#ifdef __WXDEBUG__
    SetBackgroundColour(wxColour(255, 0, 255));
#endif
    SetForegroundColour(*wxBLACK);

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Center();

    // To work properly on Mac, RestoreState() must be called _after_ 
    //  calling GetSizer()->Fit(), GetSizer()->SetSizeHints() and Center()
    RestoreState();   

    Thaw();

    return true;
}


/*!
 * Control creation for CDlgMessages
 */

void CDlgMessages::CreateControls(){
    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(1, 1, 0, 0);
    itemFlexGridSizer2->AddGrowableRow(0);
    itemFlexGridSizer2->AddGrowableCol(0);
    SetSizer(itemFlexGridSizer2);

    m_pBackgroundPanel = new CPanelMessages(this);
    itemFlexGridSizer2->Add(m_pBackgroundPanel, 0, wxGROW, 0);

    SetSizer(itemFlexGridSizer2);
}


/*!
 * wxEVT_SHOW event handler for ID_DLGMESSAGES
 */

void CDlgMessages::OnShow(wxShowEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::OnShow - Function Begin"));
    static bool bAlreadyRunning = false;

    if ((event.GetEventObject() == this) && !bAlreadyRunning) {
        bAlreadyRunning = true;

        wxLogTrace(wxT("Function Status"), wxT("CDlgMessages::OnShow - Show/Hide Event for CAdvancedFrame detected"));
        if (event.GetShow()) {
            RestoreWindowDimensions();
        } else {
            SaveWindowDimensions();
        }

        bAlreadyRunning = false;
    } else {
        event.Skip();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::OnShow - Function End"));
}


/*!
 * wxEVT_HELP event handler for ID_DLGMESSAGES
 */

void CDlgMessages::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::OnHelp - Function Begin"));

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

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::OnHelp - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgMessages::OnOK( wxCommandEvent& /*event*/ ) {
    SaveState();
    EndModal(wxID_OK);
}


bool CDlgMessages::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::SaveState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/Simple/Messages"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    // Reterieve and store the latest window dimensions.
    SaveWindowDimensions();

    // Save the list ctrl state
    m_pBackgroundPanel->OnSaveState(pConfig);

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::SaveState - Function End"));
    return true;
}


void CDlgMessages::SaveWindowDimensions() {
    wxString        strBaseConfigLocation = wxString(wxT("/Simple/Messages"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("WindowIconized"), IsIconized());
    pConfig->Write(wxT("WindowMaximized"), IsMaximized());
    pConfig->Write(wxT("Width"), GetSize().GetWidth());
    pConfig->Write(wxT("Height"), GetSize().GetHeight());

#ifdef __WXMAC__
    pConfig->Write(wxT("XPos"), GetPosition().x);
    pConfig->Write(wxT("YPos"), GetPosition().y);
#endif  // ! __WXMAC__
}
    

bool CDlgMessages::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/Simple/Messages"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

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

    // Restore the windows properties
    RestoreWindowDimensions();

    // Restore the list ctrl state
    m_pBackgroundPanel->OnRestoreState(pConfig);

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::RestoreState - Function End"));
    return true;
}


void CDlgMessages::RestoreWindowDimensions() {
    wxString        strBaseConfigLocation = wxString(wxT("/Simple/Messages"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    bool            bWindowIconized = false;
    bool            bWindowMaximized = false;
    int             iHeight = 0;
    int             iWidth = 0;
    int             iTop = 0;
    int             iLeft = 0;

    wxASSERT(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("YPos"), &iTop, 30);
    pConfig->Read(wxT("XPos"), &iLeft, 30);
    pConfig->Read(wxT("Width"), &iWidth, 640);
    pConfig->Read(wxT("Height"), &iHeight, 480);
    pConfig->Read(wxT("WindowIconized"), &bWindowIconized, false);
    pConfig->Read(wxT("WindowMaximized"), &bWindowMaximized, false);

#ifndef __WXMAC__

    Iconize(bWindowIconized);
    Maximize(bWindowMaximized);
    if (!IsIconized() && !IsMaximized()) {
        SetSize(-1, -1, iWidth, iHeight);
    }

#else   // ! __WXMAC__

    // If the user has changed the arrangement of multiple 
    // displays, make sure the window title bar is still on-screen.
    Rect titleRect = {iTop, iLeft, iTop+22, iLeft+iWidth };
    InsetRect(&titleRect, 5, 5);    // Make sure at least a 5X5 piece visible
    RgnHandle displayRgn = NewRgn();
    CopyRgn(GetGrayRgn(), displayRgn);  // Region encompassing all displays
    Rect menuRect = ((**GetMainDevice())).gdRect;
    menuRect.bottom = GetMBarHeight() + menuRect.top;
    RgnHandle menuRgn = NewRgn();
    RectRgn(menuRgn, &menuRect);                // Region hidden by menu bar
    DiffRgn(displayRgn, menuRgn, displayRgn);   // Subtract menu bar retion
    if (!RectInRgn(&titleRect, displayRgn))
        iTop = iLeft = 30;
    DisposeRgn(menuRgn);
    DisposeRgn(displayRgn);

    SetSize(iLeft, iTop, iWidth, iHeight);

#endif  // ! __WXMAC__
}

