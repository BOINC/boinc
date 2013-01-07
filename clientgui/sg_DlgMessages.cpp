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
#include "NoticeListCtrl.h"
#include "BOINCInternetFSHandler.h"



////@begin includes
////@end includes

////@begin XPM images
////@end XPM images


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

/*!
 * CPanelMessages type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPanelMessages, wxPanel )

/*!
 * CPanelMessages event table definition
 */

BEGIN_EVENT_TABLE( CPanelMessages, wxPanel )
////@begin CPanelMessages event table entries
    EVT_NOTICELIST_ITEM_DISPLAY( CPanelMessages::OnLinkClicked )
    EVT_ERASE_BACKGROUND( CPanelMessages::OnEraseBackground )
    EVT_BUTTON( wxID_OK, CPanelMessages::OnOK )
    EVT_BUTTON(ID_SIMPLE_HELP, CPanelMessages::OnButtonHelp)
    EVT_BUTTON( ID_LIST_RELOADNOTICES, CPanelMessages::OnRetryButton )
////@end CPanelMessages event table entries
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
    m_bProcessingRefreshEvent = false;
    m_bWaitingToClose = false;
////@end CPanelMessages member initialisation

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    return true;
}


CPanelMessages::~CPanelMessages()
{
}


/*!
 * Control creation for CPanelMessages
 */

void CPanelMessages::CreateControls()
{
    CPanelMessages* itemDialog1 = this;

    m_ReloadNoticesPanel = new wxPanel(this);
    m_ReloadNoticesPanel->SetBackgroundColour(*wxWHITE);

    wxFlexGridSizer* itemReloadButtonSizer = new wxFlexGridSizer(1, 2, 0, 0);
    itemReloadButtonSizer->AddGrowableCol(1);
   
    m_ReloadNoticesText = new wxStaticText(m_ReloadNoticesPanel,
                            wxID_ANY,
                            _("One or more items failed to load from the Internet."),
                            wxDefaultPosition, wxDefaultSize, 0
                            );
    itemReloadButtonSizer->Add(m_ReloadNoticesText, 1, wxALL, 5);
    
    m_ReloadNoticesButton = new wxButton(
                                    m_ReloadNoticesPanel,
                                    ID_LIST_RELOADNOTICES,
                                    _("Retry now"),
                                    wxDefaultPosition, wxDefaultSize, 0
                                    );
    itemReloadButtonSizer->Add(m_ReloadNoticesButton, 1, wxALL, 5);
    
    m_ReloadNoticesPanel->SetSizer(itemReloadButtonSizer);
    m_ReloadNoticesPanel->Layout();

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(3, 1, 1, 0);
    itemFlexGridSizer2->AddGrowableRow(1);
    itemFlexGridSizer2->AddGrowableCol(0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    itemFlexGridSizer2->Add(m_ReloadNoticesPanel, 1, wxGROW|wxALL, 1);

    m_pHtmlListPane = new CNoticeListCtrl(itemDialog1);
    itemFlexGridSizer2->Add(m_pHtmlListPane, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    
#ifdef __WXMAC__            // Don't let Close button overlap window's grow icon
    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 12);
#else
    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    wxButton* itemButton44 = new wxButton(itemDialog1, wxID_OK, _("Close"),  wxDefaultPosition, wxDefaultSize);

    itemBoxSizer4->Add(itemButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    
    m_FetchingNoticesText = new wxStaticText(
                                    this, wxID_ANY, 
                                    _("Fetching notices; please wait..."), 
                                    wxPoint(20, 20), wxDefaultSize, 0
                                    );
    m_FetchingNoticesText->SetBackgroundColour(*wxWHITE);
    
    m_NoNoticesText = new wxStaticText(
                                    this, wxID_ANY, 
                                    _("There are no notices at this time."), 
                                    wxPoint(20, 20), wxDefaultSize, 0
                                    );
    m_NoNoticesText->SetBackgroundColour(*wxWHITE);

    m_NoNoticesText->Hide();
    m_ReloadNoticesPanel->Hide();
    Layout();
    
    m_bMissingItems =  false;
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
    if (m_bWaitingToClose) return;
    
    if (!m_bProcessingRefreshEvent) {
        m_bProcessingRefreshEvent = true;

        static wxString strLastMachineName = wxEmptyString;
        wxString strNewMachineName = wxEmptyString;
        bool bMissingItems;
        CC_STATUS status;
        CMainDocument* pDoc = wxGetApp().GetDocument();
        wxFileSystemHandler *internetFSHandler = wxGetApp().GetInternetFSHandler();
        
        wxASSERT(pDoc);
        wxASSERT(m_pHtmlListPane);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));
        wxASSERT(internetFSHandler);

        if (pDoc->IsConnected()) {
            pDoc->GetConnectedComputerName(strNewMachineName);
            if (strLastMachineName != strNewMachineName) {
                strLastMachineName = strNewMachineName;
                m_FetchingNoticesText->Show();
                m_NoNoticesText->Hide();
                ((CBOINCInternetFSHandler*)internetFSHandler)->ClearCache();
                m_pHtmlListPane->Clear();
                if (m_bMissingItems) {
                    m_ReloadNoticesPanel->Hide();
                    m_bMissingItems = false;
                    Layout();
                }
            }
        }

        // Don't call Freeze() / Thaw() here because it causes an unnecessary redraw
        m_pHtmlListPane->UpdateUI();

        if (m_bWaitingToClose) {
            wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_OK);
            GetEventHandler()->AddPendingEvent(evt);
        } else {
            bMissingItems = ((CBOINCInternetFSHandler*)internetFSHandler)->ItemsFailedToLoad();
            if (bMissingItems != m_bMissingItems) {
                m_ReloadNoticesPanel->Show(bMissingItems);
                Layout();
                m_bMissingItems = bMissingItems;
            }
            
            m_FetchingNoticesText->Show(m_pHtmlListPane->m_bDisplayFetchingNotices);
            m_NoNoticesText->Show(m_pHtmlListPane->m_bDisplayEmptyNotice);
        }
        
        m_bProcessingRefreshEvent = false;
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CPanelMessages::OnOK( wxCommandEvent& event ) {
    // Shut down any asynchronous Internet access in progress
    wxFileSystemHandler *internetFSHandler = wxGetApp().GetInternetFSHandler();
    if (internetFSHandler) {
        ((CBOINCInternetFSHandler*)internetFSHandler)->SetAbortInternetIO();
    }
    
    // If we were called during Yield() in async Internet I/O, 
    // it is not safe to call our destructor until OnRefresh()
    // has returned from m_pHtmlListPane->UpdateUI(), so just
    // set a flag to close this dialog at that time.
    if (m_bProcessingRefreshEvent) {
        m_bWaitingToClose = true;
        return;
    }
    
    event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
 */

void CPanelMessages::OnButtonHelp( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnHelp - Function Begin"));

    if (IsShown()) {
    	wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

		wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple_messages&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            ID_SIMPLE_HELP
        );
        wxLaunchDefaultBrowser(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CPanelMessages::OnHelp - Function End"));
}


/*!
 * wxEVT_NOTICELIST_ITEM_DISPLAY event handler for ID_LIST_NOTIFICATIONSVIEW
 */

void CPanelMessages::OnLinkClicked( NoticeListCtrlEvent& event ) {
    if (event.GetURL().StartsWith(wxT("http://"))) {
		wxLaunchDefaultBrowser(event.GetURL());
    }
}


void CPanelMessages::OnRetryButton( wxCommandEvent& event ) {
    m_ReloadNoticesPanel->Hide();
    m_bMissingItems = false;
    Layout();
    ReloadNotices();
}


void CPanelMessages::ReloadNotices() {
    wxFileSystemHandler *internetFSHandler = wxGetApp().GetInternetFSHandler();
    if (internetFSHandler) {
        ((CBOINCInternetFSHandler*)internetFSHandler)->UnchacheMissingItems();
        m_pHtmlListPane->Clear();
        m_FetchingNoticesText->Show();
        m_NoNoticesText->Hide();
    }
}

bool CPanelMessages::OnSaveState(wxConfigBase* /* pConfig */) {
    return true;
}


bool CPanelMessages::OnRestoreState(wxConfigBase* /* pConfig */) {
    return true;
}


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
	EVT_SIZE(CDlgMessages::OnSize)
    EVT_MOVE(CDlgMessages::OnMove)
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

        
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    wxDialog::Create( parent, id, caption, pos, size, style );

    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        strCaption.Printf(_("%s - Notices"), pSkinAdvanced->GetApplicationName().c_str());
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

void CDlgMessages::OnHelp(wxHelpEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgMessages::OnHelp - Function Begin"));

    if (IsShown()) {
        wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

        wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple_messages&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            ID_SIMPLE_HELP
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
    if (!IsIconized()) {
        pConfig->Write(wxT("Width"), GetSize().GetWidth());
        pConfig->Write(wxT("Height"), GetSize().GetHeight());
        pConfig->Write(wxT("XPos"), GetPosition().x);
        pConfig->Write(wxT("YPos"), GetPosition().y);
    }
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

    // If either co-ordinate is less then 0 then set it equal to 0 to ensure
    // it displays on the screen.
    if ( iLeft < 0 ) iLeft = 30;
    if ( iTop < 0 ) iTop = 30;

    // Read the size of the screen
    wxInt32 iMaxWidth = wxSystemSettings::GetMetric( wxSYS_SCREEN_X );
    wxInt32 iMaxHeight = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );

    // Max sure that it doesn't go off to the right or bottom
    if ( iLeft + iWidth > iMaxWidth ) iLeft = iMaxWidth - iWidth;
    if ( iTop + iHeight > iMaxHeight ) iTop = iMaxHeight - iHeight;

    if (!IsIconized() && !IsMaximized()) {
        SetSize(iLeft, iTop, iWidth, iHeight);
    }
    Iconize(bWindowIconized);
    Maximize(bWindowMaximized);

#else   // ! __WXMAC__

    // If the user has changed the arrangement of multiple 
    // displays, make sure the window title bar is still on-screen.
    if (!IsWindowOnScreen(iLeft, iTop, iWidth, iHeight)) {
        iTop = iLeft = 30;
    }
        SetSize(iLeft, iTop, iWidth, iHeight);
#endif  // ! __WXMAC__
}

void CDlgMessages::OnSize(wxSizeEvent& event) {
    if (IsShown()) {
        SaveWindowDimensions();
    }
    event.Skip();
}


void CDlgMessages::OnMove(wxMoveEvent& event) {
    if (IsShown()) {
        SaveWindowDimensions();
    }
    event.Skip();
}
