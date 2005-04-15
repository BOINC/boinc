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
#pragma implementation "MainFrame.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainFrame.h"
#include "Events.h"
#include "BOINCBaseView.h"
#include "ViewProjects.h"
#include "ViewWork.h"
#include "ViewTransfers.h"
#include "ViewMessages.h"
#include "ViewResources.h"
#include "ViewStatistics.h"
#include "DlgAbout.h"
#include "DlgOptions.h"
#include "DlgAttachProject.h"
#include "DlgAccountManager.h"
#include "DlgSelectComputer.h"

#include "res/BOINCGUIApp.xpm"
#include "res/connect.xpm"
#include "res/disconnect.xpm"


enum STATUSBARFIELDS {
    STATUS_TEXT,
    STATUS_CONNECTION_STATUS
};


IMPLEMENT_DYNAMIC_CLASS(CStatusBar, wxStatusBar)

BEGIN_EVENT_TABLE(CStatusBar, wxStatusBar)
    EVT_SIZE(CStatusBar::OnSize)
END_EVENT_TABLE()


CStatusBar::CStatusBar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Default Constructor Function End"));
}


CStatusBar::CStatusBar(wxWindow *parent) :
    wxStatusBar(parent, ID_STATUSBAR, wxST_SIZEGRIP, _T("statusBar"))
{
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Function Begin"));

    const int widths[] = {-1, 200, 20};
    SetFieldsCount(WXSIZEOF(widths), widths);

    m_pbmpConnected = new wxStaticBitmap(this, -1, wxIcon(connect_xpm));
    wxASSERT(NULL != m_pbmpConnected);
    m_pbmpConnected->Hide();

    m_ptxtConnected = new wxStaticText(this, -1, _("Connected"), wxPoint(0, 0), wxDefaultSize, wxALIGN_LEFT);
    wxASSERT(NULL != m_ptxtConnected);
    m_ptxtConnected->Hide();

    m_pbmpDisconnect = new wxStaticBitmap(this, -1, wxIcon(disconnect_xpm));
    wxASSERT(NULL != m_pbmpDisconnect);
    m_pbmpDisconnect->Hide();

    m_ptxtDisconnect = new wxStaticText(this, -1, _("Disconnected"), wxPoint(0, 0), wxDefaultSize, wxALIGN_LEFT);
    wxASSERT(NULL != m_ptxtDisconnect);
    m_ptxtDisconnect->Hide();

    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Function End"));
}


CStatusBar::~CStatusBar()
{

}


void CStatusBar::OnSize(wxSizeEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::OnSize - Function Begin"));

    if (IsShown()) {
        wxRect rect;
        wxSize size;

        GetFieldRect(STATUS_CONNECTION_STATUS, rect);

        if (m_pbmpConnected) {
            size = m_pbmpConnected->GetSize();
            m_pbmpConnected->Move(rect.x + 1,
                                  rect.y + (rect.height - size.y) / 2);
        }

        if (m_ptxtConnected) {
            m_ptxtConnected->Move((rect.x + size.x) + 2,
                                  (rect.y + (rect.height - size.y) / 2) + 1);
        }

        if (m_pbmpDisconnect) {
            size = m_pbmpConnected->GetSize();
            m_pbmpDisconnect->Move(rect.x + 1,
                                   rect.y + (rect.height - size.y) / 2);
        }

        if (m_ptxtDisconnect) {
            m_ptxtDisconnect->Move((rect.x + size.x) + 2,
                                   (rect.y + (rect.height - size.y) / 2) + 1);
        }
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::OnSize - Function End"));
}


DEFINE_EVENT_TYPE(wxEVT_MAINFRAME_CONNECT)
DEFINE_EVENT_TYPE(wxEVT_MAINFRAME_CONNECT_ERROR)
DEFINE_EVENT_TYPE(wxEVT_MAINFRAME_CONNECT_ERROR_AUTHENTICATION)
DEFINE_EVENT_TYPE(wxEVT_MAINFRAME_INITIALIZED)
DEFINE_EVENT_TYPE(wxEVT_MAINFRAME_REFRESHVIEW)


IMPLEMENT_DYNAMIC_CLASS(CMainFrame, wxFrame)

BEGIN_EVENT_TABLE (CMainFrame, wxFrame)
    EVT_MENU(ID_HIDE, CMainFrame::OnHide)
    EVT_MENU_RANGE(ID_ACTIVITYRUNALWAYS, ID_ACTIVITYSUSPEND, CMainFrame::OnActivitySelection)
    EVT_MENU_RANGE(ID_NETWORKRUNALWAYS, ID_NETWORKSUSPEND, CMainFrame::OnNetworkSelection)
    EVT_MENU(ID_RUNBENCHMARKS, CMainFrame::OnRunBenchmarks)
    EVT_MENU(ID_SELECTCOMPUTER, CMainFrame::OnSelectComputer)
    EVT_MENU(wxID_EXIT, CMainFrame::OnExit)
    EVT_MENU(ID_TOOLSUPDATEACCOUNTS, CMainFrame::OnToolsUpdateAccounts)
    EVT_MENU(ID_TOOLSOPTIONS, CMainFrame::OnToolsOptions)
    EVT_MENU(wxID_ABOUT, CMainFrame::OnAbout)
    EVT_CLOSE(CMainFrame::OnClose)
    EVT_CHAR(CMainFrame::OnChar)
    EVT_HELP(ID_FRAME, CMainFrame::OnHelp)
    EVT_MAINFRAME_CONNECT(CMainFrame::OnConnect)
    EVT_MAINFRAME_CONNECT_ERROR(CMainFrame::OnConnectError)
    EVT_MAINFRAME_CONNECT_ERROR_AUTHENTICATION(CMainFrame::OnConnectErrorAuthentication)
    EVT_MAINFRAME_INITIALIZED(CMainFrame::OnInitialized)
    EVT_MAINFRAME_REFRESH(CMainFrame::OnRefreshView)
    EVT_TIMER(ID_REFRESHSTATETIMER, CMainFrame::OnRefreshState)
    EVT_TIMER(ID_FRAMERENDERTIMER, CMainFrame::OnFrameRender)
    EVT_TIMER(ID_FRAMELISTRENDERTIMER, CMainFrame::OnListPanelRender)
    EVT_TIMER(ID_DOCUMENTPOLLTIMER, CMainFrame::OnDocumentPoll)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_FRAMENOTEBOOK, CMainFrame::OnNotebookSelectionChanged)
END_EVENT_TABLE ()


CMainFrame::CMainFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CMainFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CMainFrame - Default Constructor Function End"));
}


CMainFrame::CMainFrame(wxString strTitle) : 
    wxFrame ((wxFrame *)NULL, ID_FRAME, strTitle, wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CMainFrame - Function Begin"));
    
    m_pMenubar = NULL;
    m_pNotebook = NULL;
    m_pStatusbar = NULL;
    m_iSelectedLanguage = 0;

    m_strBaseTitle = strTitle;

    m_aSelectedComputerMRU.Clear();


    SetIcon(wxICON(APP_ICON));


    wxCHECK_RET(CreateMenu(), _T("Failed to create menu bar."));
    wxCHECK_RET(CreateNotebook(), _T("Failed to create notebook."));
    wxCHECK_RET(CreateStatusbar(), _T("Failed to create status bar."));


    m_pRefreshStateTimer = new wxTimer(this, ID_REFRESHSTATETIMER);
    wxASSERT(NULL != m_pRefreshStateTimer);

    m_pFrameRenderTimer = new wxTimer(this, ID_FRAMERENDERTIMER);
    wxASSERT(NULL != m_pFrameRenderTimer);

    m_pFrameListPanelRenderTimer = new wxTimer(this, ID_FRAMELISTRENDERTIMER);
    wxASSERT(NULL != m_pFrameListPanelRenderTimer);

    m_pDocumentPollTimer = new wxTimer(this, ID_DOCUMENTPOLLTIMER);
    wxASSERT(NULL != m_pDocumentPollTimer);

    m_pRefreshStateTimer->Start(60000);              // Send event every 60 seconds
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second
    m_pFrameListPanelRenderTimer->Start(5000);       // Send event every 5 seconds
    m_pDocumentPollTimer->Start(250);                // Send event every 250 milliseconds

    RestoreState();

    SetStatusBarPane(0);

    // Complete any remaining initialization that has to happen after we are up
    //   and running
    CMainFrameEvent event(wxEVT_MAINFRAME_INITIALIZED, this);
    AddPendingEvent(event);

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CMainFrame - Function End"));
}


CMainFrame::~CMainFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::~CMainFrame - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pRefreshStateTimer);
    wxASSERT(m_pFrameRenderTimer);
    wxASSERT(m_pFrameListPanelRenderTimer);
    wxASSERT(m_pDocumentPollTimer);
    wxASSERT(m_pMenubar);
    wxASSERT(m_pNotebook);
    wxASSERT(m_pStatusbar);


    SaveState();

    if (m_pRefreshStateTimer) {
        m_pRefreshStateTimer->Stop();
        delete m_pRefreshStateTimer;
    }

    if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
    }

    if (m_pFrameListPanelRenderTimer) {
        m_pFrameListPanelRenderTimer->Stop();
        delete m_pFrameListPanelRenderTimer;
    }

    if (m_pDocumentPollTimer) {
        m_pDocumentPollTimer->Stop();
        delete m_pDocumentPollTimer;
    }

    if (m_pStatusbar)
        wxCHECK_RET(DeleteStatusbar(), _T("Failed to delete status bar."));

    if (m_pNotebook)
        wxCHECK_RET(DeleteNotebook(), _T("Failed to delete notebook."));

    if (m_pMenubar)
        wxCHECK_RET(DeleteMenu(), _T("Failed to delete menu bar."));

    

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::~CMainFrame - Function End"));
}


bool CMainFrame::CreateMenu() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateMenu - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // File menu
    wxMenu *menuFile = new wxMenu;

#ifdef __WXMAC__
    menuFile->Append(
        ID_HIDE, 
        _("Close"),
        _("Closes the main BOINC Manager window")
    );
#else
    menuFile->Append(
        ID_HIDE, 
        _("&Hide"),
        _("Hides the main BOINC Manager window")
    );
#endif
    menuFile->AppendSeparator();

    menuFile->AppendRadioItem(
        ID_ACTIVITYRUNALWAYS,
        _("&Run always"),
        _("Does work regardless of preferences")
    );
    menuFile->AppendRadioItem(
        ID_ACTIVITYRUNBASEDONPREPERENCES,
        _("Run based on &preferences"),
        _("Does work according to your preferences")
    );
    menuFile->AppendRadioItem(
        ID_ACTIVITYSUSPEND,
        _("&Suspend"),
        _("Stops work regardless of preferences")
    );

    menuFile->AppendSeparator();

    menuFile->AppendCheckItem(
        ID_NETWORKSUSPEND,
        _("&Disable BOINC Network Access"),
        _("Stops BOINC network activity")
    );

    menuFile->AppendSeparator();

    menuFile->Append(
        ID_RUNBENCHMARKS, 
        _("Run &Benchmarks"),
        _("Runs BOINC CPU benchmarks")
    );

    menuFile->AppendSeparator();

    menuFile->Append(
        ID_SELECTCOMPUTER, 
        _("Select Computer..."),
        _("Connect to another computer running BOINC")
    );

    menuFile->AppendSeparator();

    menuFile->Append(
        wxID_EXIT,
        _("E&xit"),
        _("Exit the BOINC Manager")
    );

    // Tools menu
    wxMenu *menuTools = new wxMenu;
    if (pDoc->IsAccountManagerFound()) {
        menuTools->Append(
            ID_TOOLSUPDATEACCOUNTS, 
            _("&Update Accounts"),
            _("Connect to your account manager website and update all of your accounts")
        );

        menuTools->AppendSeparator();
    }

    menuTools->Append(
        ID_TOOLSOPTIONS, 
        _("&Options"),
        _("Configure GUI options and proxy settings")
    );

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(
        wxID_ABOUT,
        _("&About BOINC Manager..."), 
        _("Show information about BOINC and BOINC Manager")
    );

    // construct menu
    m_pMenubar = new wxMenuBar;
    m_pMenubar->Append(
        menuFile,
        _("&File")
    );
    m_pMenubar->Append(
        menuTools,
        _("&Tools")
    );
    m_pMenubar->Append(
        menuHelp,
        _("&Help")
    );
    SetMenuBar(m_pMenubar);

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateMenu - Function End"));
    return true;
}


bool CMainFrame::CreateNotebook() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateNotebook - Function Begin"));

    // create frame panel
    wxPanel *pPanel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize,
                                 wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxNO_BORDER);
    pPanel->SetAutoLayout(TRUE);

    // initialize notebook
    m_pNotebook = new wxNotebook(pPanel, ID_FRAMENOTEBOOK, wxDefaultPosition, wxDefaultSize,
                                wxNB_FIXEDWIDTH|wxCLIP_CHILDREN);

    wxNotebookSizer *pNotebookSizer = new wxNotebookSizer(m_pNotebook);

    // layout frame panel
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    pPanelSizer->Add(new wxStaticLine(pPanel, -1), 0, wxEXPAND);
	pPanelSizer->Add(0, 5);
    pPanelSizer->Add(pNotebookSizer, 1, wxEXPAND);


    // create the various notebook pages
    CreateNotebookPage(new CViewProjects(m_pNotebook));
    CreateNotebookPage(new CViewWork(m_pNotebook));
    CreateNotebookPage(new CViewTransfers(m_pNotebook));
    CreateNotebookPage(new CViewMessages(m_pNotebook));
	CreateNotebookPage(new CViewStatistics(m_pNotebook));
    CreateNotebookPage(new CViewResources(m_pNotebook));


    // have the panel calculate everything after the pages are created so
    //   the mac can display the html control width correctly
    pPanel->SetSizerAndFit(pPanelSizer);


    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateNotebook - Function End"));
    return true;
}


template < class T >
bool CMainFrame::CreateNotebookPage(T pwndNewNotebookPage) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateNotebookPage - Function Begin"));

    wxImageList*    pImageList;
    int         iImageIndex = 0;

    wxASSERT(NULL != pwndNewNotebookPage);
    wxASSERT(NULL != m_pNotebook);
    wxASSERT(wxDynamicCast(pwndNewNotebookPage, CBOINCBaseView));


    pImageList = m_pNotebook->GetImageList();
    if (!pImageList) {
        pImageList = new wxImageList(16, 16, true, 0);
        wxASSERT(pImageList != NULL);
        m_pNotebook->SetImageList(pImageList);
    }
    
    iImageIndex = pImageList->Add(wxBitmap(pwndNewNotebookPage->GetViewIcon()), wxColour(255, 0, 255));
    m_pNotebook->AddPage(pwndNewNotebookPage, pwndNewNotebookPage->GetViewName(), TRUE, iImageIndex);

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateNotebookPage - Function End"));
    return true;
}


bool CMainFrame::CreateStatusbar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateStatusbar - Function Begin"));

    if (m_pStatusbar)
        return true;

    m_pStatusbar = new CStatusBar(this);
    wxASSERT(NULL != m_pStatusbar);

    SetStatusBar(m_pStatusbar);

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateStatusbar - Function End"));
    return true;
}


bool CMainFrame::DeleteMenu() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::DeleteMenu - Function Begin"));

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::DeleteMenu - Function End"));
    return true;
}


bool CMainFrame::DeleteNotebook() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::DeleteNotebook - Function Begin"));

    wxImageList*    pImageList;

    wxASSERT(NULL != m_pNotebook);

    pImageList = m_pNotebook->GetImageList();

    wxASSERT(NULL != pImageList);

    if (pImageList)
        delete pImageList;

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::DeleteNotebook - Function End"));
    return true;
}


bool CMainFrame::DeleteStatusbar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::DeleteStatusbar - Function Begin"));

    if (!m_pStatusbar)
        return true;

    SetStatusBar(NULL);

    delete m_pStatusbar;
    m_pStatusbar = NULL;

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::DeleteStatusbar - Function End"));
    return true;
}


bool CMainFrame::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::SaveState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    CBOINCBaseView* pView = NULL;
    wxString        strConfigLocation = wxEmptyString;
    wxString        strPreviousLocation = wxEmptyString;
    wxString        strBuffer = wxEmptyString;
    int            iIndex = 0;
    int            iItemCount = 0;


    wxASSERT(pConfig);
    wxASSERT(m_pNotebook);

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("Language"), m_iSelectedLanguage);

    pConfig->Write(wxT("CurrentPage"), m_pNotebook->GetSelection());

    pConfig->Write(wxT("WindowIconized"), IsIconized());
#if defined(__WXMSW__) || defined(__WXMAC__)
    pConfig->Write(wxT("WindowMaximized"), IsMaximized());
#endif
    if (!IsIconized() && !IsMaximized()) {
        pConfig->Write(wxT("Width"), GetSize().GetWidth());
        pConfig->Write(wxT("Height"), GetSize().GetHeight());
#ifdef __WXMAC__
        pConfig->Write(wxT("XPos"),GetPosition().x);
        pConfig->Write(wxT("YPos"), GetPosition().y);
#endif
    }


    //
    // Save Page(s) State
    //
 
    // Convert to a zero based index
    iItemCount = m_pNotebook->GetPageCount() - 1;

    for (iIndex = 0; iIndex <= iItemCount; iIndex++) {   
        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        strPreviousLocation = pConfig->GetPath();
        strConfigLocation = strPreviousLocation + pView->GetViewName();

        pConfig->SetPath(strConfigLocation);
        pView->FireOnSaveState(pConfig);
        pConfig->SetPath(strPreviousLocation);
    }


    //
    // Save Computer MRU list
    //
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);

    iItemCount = m_aSelectedComputerMRU.GetCount() - 1;
    for (iIndex = 0; iIndex <= iItemCount; iIndex++) {
        strBuffer.Printf(wxT("%d"), iIndex);
        pConfig->Write(
            strBuffer,
            m_aSelectedComputerMRU.Item(iIndex)
        );
    }

    pConfig->SetPath(strPreviousLocation);


    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::SaveState - Function End"));
    return true;
}


bool CMainFrame::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    CBOINCBaseView* pView = NULL;
    wxString        strConfigLocation = wxEmptyString;
    wxString        strPreviousLocation = wxEmptyString;
    wxString        strBuffer = wxEmptyString;
    wxString        strValue = wxEmptyString;
    long            iIndex = 0;
    long            iPageCount = 0;
    bool            bKeepEnumerating = false;
    bool            bWindowIconized = false;
#if defined(__WXMSW__) || defined(__WXMAC__)
    bool            bWindowMaximized = false;
#endif
#ifdef __WXMAC__
    long            iTop = 0;
    long            iLeft = 0;
#endif
    long            iHeight = 0;
    long            iWidth = 0;


    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pNotebook);


    //
    // Restore Frame State
    //
    int         iCurrentPage;


    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("Language"), &m_iSelectedLanguage, 0L);


    pConfig->Read(wxT("CurrentPage"), &iCurrentPage, 1);
    m_pNotebook->SetSelection(iCurrentPage);


    pConfig->Read(wxT("WindowIconized"), &bWindowIconized, false);
#if defined(__WXMSW__) || defined(__WXMAC__)
    pConfig->Read(wxT("WindowMaximized"), &bWindowMaximized, false);
#endif
    pConfig->Read(wxT("Width"), &iWidth, 800);
    pConfig->Read(wxT("Height"), &iHeight, 600);

    SetSize(-1, -1, iWidth, iHeight);
    Iconize(bWindowIconized);
#if defined(__WXMSW__) || defined(__WXMAC__)
    Maximize(bWindowMaximized);
#endif

#ifdef __WXMAC__
    pConfig->Read(wxT("YPos"), &iTop, 30);
    pConfig->Read(wxT("XPos"), &iLeft, 30);

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

    SetPosition(wxPoint(iLeft, iTop));
    // The following line of code works around an apparent bug in 
    // the Macintosh version of wxWidgets which fails to set the 
    // window size and position, though the Windows version does so.
    // It appears that SetSize() and SetPosition() don't set their 
    // corresponding instance variables on the Macintosh, but 
    // wxTopLevelWindowMac::DoMoveWindow() does successfully set 
    // these instance variables as well as actually adjust the window.
    wxTopLevelWindowMac::DoMoveWindow(iLeft, iTop, iWidth, iHeight);
#endif

    //
    // Restore Page(s) State
    //

    // Convert to a zero based index
    iPageCount = m_pNotebook->GetPageCount() - 1;

    for (iIndex = 0; iIndex <= iPageCount; iIndex++) {   

        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        strPreviousLocation = pConfig->GetPath();
        strConfigLocation = strPreviousLocation + pView->GetViewName();

        pConfig->SetPath(strConfigLocation);
        pView->FireOnRestoreState(pConfig);
        pConfig->SetPath(strPreviousLocation);

    }


    //
    // Restore Computer MRU list
    //
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);

    m_aSelectedComputerMRU.Clear();
    bKeepEnumerating = pConfig->GetFirstEntry(strBuffer, iIndex);
    while (bKeepEnumerating) {
        pConfig->Read(strBuffer, &strValue);

        m_aSelectedComputerMRU.Add(strValue);
        bKeepEnumerating = pConfig->GetNextEntry(strBuffer, iIndex);
    }

    pConfig->SetPath(strPreviousLocation);


    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::RestoreState - Function End"));
    return true;
}


void CMainFrame::OnHide(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHide - Function Begin"));

    Hide();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHide - Function End"));
}


void CMainFrame::OnActivitySelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnActivitySelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(event.GetId()) {
    case ID_ACTIVITYRUNALWAYS:
        pDoc->SetActivityRunMode(CMainDocument::MODE_ALWAYS);
        break;
    case ID_ACTIVITYSUSPEND:
        pDoc->SetActivityRunMode(CMainDocument::MODE_NEVER);
        break;
    case ID_ACTIVITYRUNBASEDONPREPERENCES:
        pDoc->SetActivityRunMode(CMainDocument::MODE_AUTO);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnActivitySelection - Function End"));
}


void CMainFrame::OnNetworkSelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNetworkSelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();
    int        iCurrentNetworkMode = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(event.GetId()) {
        case ID_NETWORKSUSPEND:
            pDoc->GetNetworkRunMode(iCurrentNetworkMode);

            if (iCurrentNetworkMode == CMainDocument::MODE_ALWAYS)
                pDoc->SetNetworkRunMode(CMainDocument::MODE_NEVER);
            else
                pDoc->SetNetworkRunMode(CMainDocument::MODE_ALWAYS);

            break;
        case ID_NETWORKRUNALWAYS:
        case ID_NETWORKRUNBASEDONPREPERENCES:
        default:
            pDoc->SetNetworkRunMode(CMainDocument::MODE_ALWAYS);
            break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNetworkSelection - Function End"));
}

   
void CMainFrame::OnRunBenchmarks(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnRunBenchmarks - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(NULL != m_pNotebook);
    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
    pDoc->RunBenchmarks();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnRunBenchmarks - Function End"));
}


void CMainFrame::OnSelectComputer(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnSelectComputer - Function Begin"));

    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CDlgSelectComputer* pDlg = new CDlgSelectComputer(this);
    long                lAnswer = 0;
    size_t              lIndex = 0;
    long                lRetVal = -1;
    wxArrayString       aComputerNames;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pDlg);


    // Lets copy the template store in the system state
    aComputerNames = m_aSelectedComputerMRU;

    // Lets populate the combo control with the MRU list
    pDlg->m_ComputerNameCtrl->Clear();
    for (lIndex = 0; lIndex < aComputerNames.Count(); lIndex++) {
        pDlg->m_ComputerNameCtrl->Append(aComputerNames.Item(lIndex));
    }

    lAnswer = pDlg->ShowModal();
    if (wxID_OK == lAnswer) {
        lRetVal = pDoc->Connect(
            pDlg->m_ComputerNameCtrl->GetValue(), 
            pDlg->m_ComputerPasswordCtrl->GetValue(),
            TRUE
        );
        if (lRetVal) {
            ::wxMessageBox(
                _("Connection failed."),
                _("Connection failed."),
                wxICON_ERROR
           );
        }

        // Insert a copy of the current combo box value to the head of the
        //   computer names string array
        aComputerNames.Insert(pDlg->m_ComputerNameCtrl->GetValue(), 0);

        // Loops through the computer names and remove any duplicates that
        //   might exist with the new head value
        for (lIndex = 1; lIndex < aComputerNames.Count(); lIndex++) {
            if (aComputerNames.Item(lIndex) == aComputerNames.Item(0))
                aComputerNames.Remove(lIndex);
        }

        // Store the modified computer name MRU list back to the system state
        m_aSelectedComputerMRU = aComputerNames;
    }

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnSelectComputer - Function End"));
}


void CMainFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnExit - Function Begin"));

    Close(true);

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnExit - Function End"));
}


void CMainFrame::OnToolsUpdateAccounts(wxCommandEvent& WXUNUSED(event))
{
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsUpdateAccounts - Function Begin"));

    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CDlgAccountManager* pDlg = new CDlgAccountManager(this);
    int             iAnswer = 0;
    wxString            strLogin = wxEmptyString;
    wxString            strPassword = wxEmptyString;


    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pDlg);

    if (!pDoc->IsAccountManagerLoginFound()) {
        iAnswer = pDlg->ShowModal();
        if (wxID_OK == iAnswer) {
            strLogin = pDlg->m_AcctManagerUsernameCtrl->GetValue();
            strPassword = pDlg->m_AcctManagerUsernameCtrl->GetValue();

            pDoc->InitializeAccountManagerLogin(strLogin, strPassword);            
        }
    }

    pDoc->UpdateAccountManagerAccounts();

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsUpdateAccounts - Function End"));
}


void CMainFrame::OnToolsOptions(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsOptions - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CDlgOptions*   pDlg = new CDlgOptions(this);
    int        iAnswer = 0;
    bool           bProxyInformationConfigured = false;
    bool           bBuffer = false;
    int        iBuffer = 0;
    wxString       strBuffer = wxEmptyString;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pDlg);


    bProxyInformationConfigured = (0 == pDoc->GetProxyConfiguration());
    if (bProxyInformationConfigured) {
        pDlg->m_bProxySectionConfigured = true;
        if (0 == pDoc->GetProxyHTTPProxyEnabled(bBuffer))
            pDlg->m_EnableHTTPProxyCtrl->SetValue(bBuffer);
        if (0 == pDoc->GetProxyHTTPServerName(strBuffer)) 
            pDlg->m_HTTPAddressCtrl->SetValue(strBuffer);
        if (0 == pDoc->GetProxyHTTPServerPort(iBuffer)) {
            strBuffer.Printf(wxT("%d"), iBuffer);
            pDlg->m_HTTPPortCtrl->SetValue(strBuffer);
        }
        if (0 == pDoc->GetProxyHTTPUserName(strBuffer)) 
            pDlg->m_HTTPUsernameCtrl->SetValue(strBuffer);
        if (0 == pDoc->GetProxyHTTPPassword(strBuffer)) 
            pDlg->m_HTTPPasswordCtrl->SetValue(strBuffer);

        if (0 == pDoc->GetProxySOCKSProxyEnabled(bBuffer))
            pDlg->m_EnableSOCKSProxyCtrl->SetValue(bBuffer);
        if (0 == pDoc->GetProxySOCKSServerName(strBuffer)) 
            pDlg->m_SOCKSAddressCtrl->SetValue(strBuffer);
        if (0 == pDoc->GetProxySOCKSServerPort(iBuffer)) {
            strBuffer.Printf(wxT("%d"), iBuffer);
            pDlg->m_SOCKSPortCtrl->SetValue(strBuffer);
        }
        if (0 == pDoc->GetProxySOCKSUserName(strBuffer)) 
            pDlg->m_SOCKSUsernameCtrl->SetValue(strBuffer);
        if (0 == pDoc->GetProxySOCKSPassword(strBuffer)) 
            pDlg->m_SOCKSPasswordCtrl->SetValue(strBuffer);

        pDlg->m_LanguageSelectionCtrl->SetSelection(m_iSelectedLanguage);
    }

    iAnswer = pDlg->ShowModal();
    if (wxID_OK == iAnswer) {
        bBuffer = pDlg->m_EnableHTTPProxyCtrl->GetValue();
        pDoc->SetProxyHTTPProxyEnabled(bBuffer);
        strBuffer = pDlg->m_HTTPAddressCtrl->GetValue();
        pDoc->SetProxyHTTPServerName(strBuffer);
        strBuffer = pDlg->m_HTTPPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->SetProxyHTTPServerPort(iBuffer);
        strBuffer = pDlg->m_HTTPUsernameCtrl->GetValue();
        pDoc->SetProxyHTTPUserName(strBuffer);
        strBuffer = pDlg->m_HTTPPasswordCtrl->GetValue();
        pDoc->SetProxyHTTPPassword(strBuffer);

        bBuffer = pDlg->m_EnableSOCKSProxyCtrl->GetValue();
        pDoc->SetProxySOCKSProxyEnabled(bBuffer);
        strBuffer = pDlg->m_SOCKSAddressCtrl->GetValue();
        pDoc->SetProxySOCKSServerName(strBuffer);
        strBuffer = pDlg->m_SOCKSPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->SetProxySOCKSServerPort(iBuffer);
        strBuffer = pDlg->m_SOCKSUsernameCtrl->GetValue();
        pDoc->SetProxySOCKSUserName(strBuffer);
        strBuffer = pDlg->m_SOCKSPasswordCtrl->GetValue();
        pDoc->SetProxySOCKSPassword(strBuffer);

        pDoc->SetProxyConfiguration();

        if (m_iSelectedLanguage != pDlg->m_LanguageSelectionCtrl->GetSelection()) {
            ::wxMessageBox(
                _("The BOINC Managers default language has been changed, in order for this change to take affect you must restart the manager."),
                _("Language Selection..."),
                wxICON_INFORMATION
           );
        }

        m_iSelectedLanguage = pDlg->m_LanguageSelectionCtrl->GetSelection();
    }

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsOptions - Function End"));
}


void CMainFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAbout - Function Begin"));

    CDlgAbout* pDlg = new CDlgAbout(this);
    wxASSERT(NULL != pDlg);

    pDlg->ShowModal();

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAbout - Function End"));
}


void CMainFrame::OnClose(wxCloseEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnClose - Function Begin"));

#if defined(__WXMSW__) || defined(__WXMAC__)
    if (!event.CanVeto())
        Destroy();
    else
        Hide();
#else
	event.Skip();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnClose - Function End"));
}


void CMainFrame::OnChar(wxKeyEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnChar - Function Begin"));

    if (IsShown()) {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage(event.GetId() - ID_LIST_BASE);
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnChar(event);
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnChar - Function End"));
}


void CMainFrame::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelp - Function Begin"));

    if (IsShown()) {
        if (ID_FRAME == event.GetId()) {
            ExecuteBrowserLink(wxT("http://boinc.berkeley.edu/manager.php"));
        } else {
            event.Skip();
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelp - Function End"));
}


void CMainFrame::OnConnect(CMainFrameEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnect - Function Begin"));
    
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CDlgAttachProject* pDlg = new CDlgAttachProject(this);
    int            iAnswer = 0;
    long               lProjectCount = 0;

    wxASSERT(m_pNotebook);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pDlg);


    // Only present the attach to project dialog if no projects are currently
    //   detected.
    lProjectCount = pDoc->GetProjectCount();
    if (0 == lProjectCount) {

        iAnswer = pDlg->ShowModal();
        if (wxID_OK == iAnswer) {
            pDoc->ProjectAttach(
                pDlg->GetProjectAddress(), 
                pDlg->GetProjectAccountKey()
            );
        }

        m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
    }


    if (pDlg) {
        pDlg->Destroy();
    }
   
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnect - Function End"));
}


void CMainFrame::OnConnectError(CMainFrameEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnectError - Function Begin"));

    ::wxMessageBox(
        _("Connection failed."),
        _("Connection failed."),
        wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnectError - Function End"));
}


void CMainFrame::OnConnectErrorAuthentication(CMainFrameEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnectErrorAuthentication - Function Begin"));

    ::wxMessageBox(
        _("The password you have provided is incorrect, please try again."),
        _("Connection Error"),
        wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnectErrorAuthentication - Function End"));
}


void CMainFrame::OnInitialized(CMainFrameEvent&) {
    fprintf(stderr, "CMainFrame::OnInitialized - Function Begin");

    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsConnected())
        pDoc->Connect(wxEmptyString, wxEmptyString, TRUE);

    fprintf(stderr, "CMainFrame::OnInitialized - Function End");
}


void CMainFrame::OnRefreshView(CMainFrameEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnRefreshView - Function Begin"));

    static bool bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        if (IsShown()) {
            wxWindow*       pwndNotebookPage = NULL;
            CBOINCBaseView* pView = NULL;
            wxTimerEvent    timerEvent;

            wxASSERT(NULL != m_pNotebook);

            pwndNotebookPage = m_pNotebook->GetPage(m_pNotebook->GetSelection());
            wxASSERT(pwndNotebookPage);

            pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
            wxASSERT(NULL != pView);

            pView->FireOnListRender(timerEvent);
        }

        bAlreadyRunningLoop = false;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnRefreshView - Function End"));
}


void CMainFrame::OnRefreshState(wxTimerEvent &event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnRefreshState - Function Begin"));

    static bool bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        // Write a snapshot of the current state to the config
        //   module, on Win9x systems we don't always shutdown
        //   in a nice way, if we are terminated by the user
        //   we still want the UI state to have been stored
        //   for their next use
        SaveState();

        bAlreadyRunningLoop = false;
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnRefreshState - Function End"));
}


void CMainFrame::OnFrameRender(wxTimerEvent &event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnFrameRender - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    static bool bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        wxGetApp().UpdateSystemIdleDetection();

        if (IsShown()) {
            if (NULL != pDoc) {
                wxASSERT(wxDynamicCast(pDoc, CMainDocument));

                // Update the menu bar
                wxMenuBar*     pMenuBar      = GetMenuBar();
                int        iActivityMode = -1;
                int        iNetworkMode  = -1;

                wxASSERT(NULL != pMenuBar);
                wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

                if (NULL != pMenuBar->FindItem(ID_ACTIVITYRUNALWAYS, NULL))
                    pMenuBar->Check(ID_ACTIVITYRUNALWAYS, false);

                if (NULL != pMenuBar->FindItem(ID_ACTIVITYSUSPEND, NULL))
                    pMenuBar->Check(ID_ACTIVITYSUSPEND, false);

                if (NULL != pMenuBar->FindItem(ID_ACTIVITYRUNBASEDONPREPERENCES, NULL))
                    pMenuBar->Check(ID_ACTIVITYRUNBASEDONPREPERENCES, false);

                if ((pDoc->IsConnected()) && (0 == pDoc->GetActivityRunMode(iActivityMode))) {
                    if (CMainDocument::MODE_ALWAYS == iActivityMode)
                        pMenuBar->Check(ID_ACTIVITYRUNALWAYS, true);

                    if (CMainDocument::MODE_NEVER == iActivityMode)
                        pMenuBar->Check(ID_ACTIVITYSUSPEND, true);
                    if (CMainDocument::MODE_AUTO == iActivityMode)
                        pMenuBar->Check(ID_ACTIVITYRUNBASEDONPREPERENCES, true);
                }

#if 0
                if (NULL != pMenuBar->FindItem(ID_NETWORKRUNALWAYS, NULL))
                    pMenuBar->Check(ID_NETWORKRUNALWAYS, false);
#endif
                if (NULL != pMenuBar->FindItem(ID_NETWORKSUSPEND, NULL))
                    pMenuBar->Check(ID_NETWORKSUSPEND, false);

#if 0
                if (NULL != pMenuBar->FindItem(ID_NETWORKRUNBASEDONPREPERENCES, NULL))
                    pMenuBar->Check(ID_NETWORKRUNBASEDONPREPERENCES, false);
#endif

                if ((pDoc->IsConnected()) && (0 == pDoc->GetNetworkRunMode(iNetworkMode))) {
#if 0
                    if (CMainDocument::MODE_ALWAYS == iNetworkMode)
                        pMenuBar->Check(ID_NETWORKRUNALWAYS, true);
#endif

                    if (CMainDocument::MODE_NEVER == iNetworkMode)
                        pMenuBar->Check(ID_NETWORKSUSPEND, true);

#if 0
                    if (CMainDocument::MODE_AUTO == iNetworkMode)
                        pMenuBar->Check(ID_NETWORKRUNBASEDONPREPERENCES, true);
#endif
                }

                // Update the statusbar
                wxASSERT(wxDynamicCast(m_pStatusbar, CStatusBar));
                if (pDoc->IsConnected() || pDoc->IsReconnecting()) {
                    m_pStatusbar->m_pbmpConnected->Show();
                    m_pStatusbar->m_ptxtConnected->Show();
                    m_pStatusbar->m_pbmpDisconnect->Hide();
                    m_pStatusbar->m_ptxtDisconnect->Hide();

                    wxString strBuffer = wxEmptyString;
                    wxString strComputerName = wxEmptyString;
                    wxString strStatusText = wxEmptyString;
                    wxString strTitle = m_strBaseTitle;
                    wxString strLocale = setlocale(LC_NUMERIC, NULL);
     
                    if (pDoc->IsReconnecting())
                        pDoc->GetConnectingComputerName(strComputerName);
                    else
                        pDoc->GetConnectedComputerName(strComputerName);

                    if (strComputerName.empty()) {
                        strTitle += wxT(" - (localhost)");
                        strComputerName += wxT("localhost");
                    } else {
                        strStatusText += strComputerName;
                    }

                    if (pDoc->IsReconnecting()) {
                        strTitle.Printf(_("%s - (%s)"), m_strBaseTitle.c_str(), strComputerName.c_str());
                        strStatusText.Printf(_("Connecting to %s"), strComputerName.c_str());
                    } else {
                        strTitle.Printf(_("%s - (%s)"), m_strBaseTitle.c_str(), strComputerName.c_str());
                        strStatusText.Printf(_("Connected to %s"), strComputerName.c_str());
                    }

                    SetTitle(strTitle);
                    m_pStatusbar->m_ptxtConnected->SetLabel(strStatusText);
                } else {
                    m_pStatusbar->m_pbmpConnected->Hide();
                    m_pStatusbar->m_ptxtConnected->Hide();
                    m_pStatusbar->m_pbmpDisconnect->Show();
                    m_pStatusbar->m_ptxtDisconnect->Show();

                    SetTitle(m_strBaseTitle);
                }
            }
        }

        bAlreadyRunningLoop = false;
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnFrameRender - Function End"));
}


void CMainFrame::OnListPanelRender(wxTimerEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnListPanelRender - Function Begin"));

    FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnListPanelRender - Function End"));
}


void CMainFrame::OnDocumentPoll(wxTimerEvent& event) {
    fprintf(stderr, "CMainFrame::OnDocumentPoll - Function Begin");
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->OnPoll();
    fprintf(stderr, "CMainFrame::OnDocumentPoll - Function End");
}


void CMainFrame::OnNotebookSelectionChanged(wxNotebookEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNotebookSelectionChanged - Function Begin"));

    if ((-1 != event.GetSelection()) && IsShown()) {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;
        wxTimerEvent    timerEvent;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage(event.GetSelection());
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnListRender(timerEvent);
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNotebookSelectionChanged - Function End"));
}


void CMainFrame::UpdateStatusText(const wxChar* szStatus) {
    wxString strStatus = szStatus;
    m_pStatusbar->SetStatusText(strStatus);
    ::wxSleep(0);
}


void CMainFrame::FireConnect() {
    CMainFrameEvent event(wxEVT_MAINFRAME_CONNECT, this);
    AddPendingEvent(event);
}


void CMainFrame::FireConnectError() {
    CMainFrameEvent event(wxEVT_MAINFRAME_CONNECT_ERROR, this);
    AddPendingEvent(event);
}


void CMainFrame::FireConnectErrorAuthentication() {
    CMainFrameEvent event(wxEVT_MAINFRAME_CONNECT_ERROR_AUTHENTICATION, this);
    AddPendingEvent(event);
}


void CMainFrame::FireRefreshView() {
    CMainFrameEvent event(wxEVT_MAINFRAME_REFRESHVIEW, this);
    AddPendingEvent(event);
}


void CMainFrame::ProcessRefreshView() {
    CMainFrameEvent event(wxEVT_MAINFRAME_REFRESHVIEW, this);
    ProcessEvent(event);
}


void CMainFrame::ExecuteBrowserLink(const wxString &strLink) {
    wxString strMimeType = wxEmptyString;

    if      (strLink.StartsWith(wxT("http://")))
        strMimeType = wxT("text/html");
    else if (strLink.StartsWith(wxT("ftp://")))
        strMimeType = wxT("text/html");
    else if (strLink.StartsWith(wxT("mailto:")))
        strMimeType = wxT("message/rfc822");
    else
        return;

    wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromMimeType(strMimeType);
    if (ft) {
        wxString cmd;
        if (ft->GetOpenCommand(&cmd, wxFileType::MessageParameters(strLink))) {
            cmd.Replace(wxT("file://"), wxEmptyString);
            ::wxExecute(cmd);
        }

        delete ft;
    }
}



const char *BOINC_RCSID_d881a56dc5 = "$Id$";
