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
#include "hyperlink.h"
#include "BOINCGUIApp.h"
#include "MainFrame.h"
#include "Events.h"
#include "BOINCBaseView.h"
#include "ViewProjects.h"
#include "ViewWork.h"
#include "ViewTransfers.h"
#include "ViewMessages.h"
#include "ViewStatistics.h"
#include "ViewResources.h"
#include "DlgAbout.h"
#include "DlgOptions.h"
#include "DlgAttachProject.h"
#include "DlgAccountManagerSignup.h"
#include "DlgAccountManagerStatus.h"
#include "DlgDialupCredentials.h"
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
    wxASSERT(m_pbmpConnected);
    m_pbmpConnected->Hide();

    m_ptxtConnected = new wxStaticText(this, -1, _("Connected"), wxPoint(0, 0), wxDefaultSize, wxALIGN_LEFT);
    wxASSERT(m_ptxtConnected);
    m_ptxtConnected->Hide();

    m_pbmpDisconnect = new wxStaticBitmap(this, -1, wxIcon(disconnect_xpm));
    wxASSERT(m_pbmpDisconnect);
    m_pbmpDisconnect->Hide();

    m_ptxtDisconnect = new wxStaticText(this, -1, _("Disconnected"), wxPoint(0, 0), wxDefaultSize, wxALIGN_LEFT);
    wxASSERT(m_ptxtDisconnect);
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


DEFINE_EVENT_TYPE(wxEVT_MAINFRAME_ALERT)
DEFINE_EVENT_TYPE(wxEVT_MAINFRAME_CONNECT)
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
    EVT_MENU(ID_TOOLSMANAGEACCOUNTS, CMainFrame::OnToolsManageAccounts)
    EVT_MENU(ID_TOOLSOPTIONS, CMainFrame::OnToolsOptions)
    EVT_HELP(ID_FRAME, CMainFrame::OnHelp)
    EVT_MENU(ID_HELPBOINCMANAGER, CMainFrame::OnHelpBOINCManager)
    EVT_MENU(ID_HELPBOINC, CMainFrame::OnHelpBOINCWebsite)
    EVT_MENU(wxID_ABOUT, CMainFrame::OnHelpAbout)
    EVT_CLOSE(CMainFrame::OnClose)
    EVT_MAINFRAME_ALERT(CMainFrame::OnAlert)
    EVT_MAINFRAME_CONNECT(CMainFrame::OnConnect)
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

    // Configuration Settings
    m_iSelectedLanguage = 0;
    m_iReminderFrequency = 0;

    m_iNetworkConnectionType = ID_NETWORKAUTODETECT;
    m_strNetworkDialupConnectionName = wxEmptyString;
    m_bNetworkDialupPromptCredentials = false;


    // Working Variables
    m_strBaseTitle = strTitle;

    m_aSelectedComputerMRU.Clear();


    // Initialize Application
    SetIcon(wxICON(APP_ICON));

    wxCHECK_RET(CreateMenu(), _T("Failed to create menu bar."));
    wxCHECK_RET(CreateNotebook(), _T("Failed to create notebook."));
    wxCHECK_RET(CreateStatusbar(), _T("Failed to create status bar."));

    RestoreState();

    SetStatusBarPane(0);


#ifdef __WXMSW__
    m_pDialupManager = wxDialUpManager::Create();
    wxASSERT(m_pDialupManager->IsOk());
#endif

    m_pRefreshStateTimer = new wxTimer(this, ID_REFRESHSTATETIMER);
    wxASSERT(m_pRefreshStateTimer);

    m_pFrameRenderTimer = new wxTimer(this, ID_FRAMERENDERTIMER);
    wxASSERT(m_pFrameRenderTimer);

    m_pFrameListPanelRenderTimer = new wxTimer(this, ID_FRAMELISTRENDERTIMER);
    wxASSERT(m_pFrameListPanelRenderTimer);

    m_pDocumentPollTimer = new wxTimer(this, ID_DOCUMENTPOLLTIMER);
    wxASSERT(m_pDocumentPollTimer);

    m_pRefreshStateTimer->Start(300000);             // Send event every 5 minutes
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second
    m_pFrameListPanelRenderTimer->Start(5000);       // Send event every 5 seconds
    m_pDocumentPollTimer->Start(250);                // Send event every 250 milliseconds


    // Limit the number of times the UI can update itself to two times a second
    //   NOTE: Linux and Mac were updating several times a second and eating
    //         CPU time
    wxUpdateUIEvent::SetUpdateInterval(500);

    // The second half of the initialization process picks up in the OnFrameRender()
    //   routine since the menus' and status bars' are drawn in the frameworks
    //   on idle routines, on idle events are sent in between the end of the
    //   constructor and the first call to OnFrameRender
    //
    // Look for the 'if (!bAlreadyRunOnce) {' statement

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CMainFrame - Function End"));
}


CMainFrame::~CMainFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::~CMainFrame - Function Begin"));

    wxASSERT(m_pRefreshStateTimer);
    wxASSERT(m_pFrameRenderTimer);
    wxASSERT(m_pFrameListPanelRenderTimer);
    wxASSERT(m_pDocumentPollTimer);
    wxASSERT(m_pMenubar);
    wxASSERT(m_pNotebook);
    wxASSERT(m_pStatusbar);
#ifdef __WXMSW__
    wxASSERT(m_pDialupManager);
#endif

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

#ifdef __WXMSW__
    if (m_pDialupManager)
        delete m_pDialupManager;
#endif


    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::~CMainFrame - Function End"));
}


bool CMainFrame::CreateMenu() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateMenu - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
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

    menuFile->AppendRadioItem(
        ID_NETWORKRUNALWAYS,
        _("&Network activity always available"),
        _("Does network activity regardless of preferences")
    );
    menuFile->AppendRadioItem(
        ID_NETWORKRUNBASEDONPREPERENCES,
        _("Network activity based on &preferences"),
        _("Does network activity according to your preferences")
    );
    menuFile->AppendRadioItem(
        ID_NETWORKSUSPEND,
        _("&Network activity suspended"),
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
    menuTools->Append(
        ID_TOOLSMANAGEACCOUNTS, 
        _("&Manage Accounts"),
        _("Connect to an account manager website and update all of your accounts")
    );

    menuTools->AppendSeparator();

    menuTools->Append(
        ID_TOOLSOPTIONS, 
        _("&Options"),
        _("Configure GUI options and proxy settings")
    );

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(
        ID_HELPBOINCMANAGER,
        _("&BOINC Manager\tF1"), 
        _("Show information about the BOINC Manager")
    );

    menuHelp->Append(
        ID_HELPBOINC,
        _("BOINC &Website"), 
        _("Show information about BOINC and BOINC Manager")
    );

    menuHelp->AppendSeparator();

    menuHelp->Append(
        wxID_ABOUT,
        _("&About BOINC Manager..."), 
        _("Licensing and copyright information.")
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

    // layout frame panel
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    pPanelSizer->Add(new wxStaticLine(pPanel, -1), 0, wxEXPAND);
	pPanelSizer->Add(0, 5);
    pPanelSizer->Add(m_pNotebook, 1, wxEXPAND);


    // create the various notebook pages
    CreateNotebookPage(new CViewProjects(m_pNotebook));
    CreateNotebookPage(new CViewWork(m_pNotebook));
    CreateNotebookPage(new CViewTransfers(m_pNotebook));
    CreateNotebookPage(new CViewMessages(m_pNotebook));
	CreateNotebookPage(new CViewStatistics(m_pNotebook));
    CreateNotebookPage(new CViewResources(m_pNotebook));


    pPanel->SetSizer(pPanelSizer);
    pPanel->Layout();


    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateNotebook - Function End"));
    return true;
}


template < class T >
bool CMainFrame::CreateNotebookPage(T pwndNewNotebookPage) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CreateNotebookPage - Function Begin"));

    wxImageList*    pImageList;
    int         iImageIndex = 0;

    wxASSERT(pwndNewNotebookPage);
    wxASSERT(m_pNotebook);
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
    wxASSERT(m_pStatusbar);

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

    wxASSERT(m_pNotebook);

    pImageList = m_pNotebook->GetImageList();

    wxASSERT(pImageList);

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

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("Language"), m_iSelectedLanguage);
    pConfig->Write(wxT("ReminderFrequency"), m_iReminderFrequency);

    pConfig->Write(wxT("NetworkConnectionType"), m_iNetworkConnectionType);
    pConfig->Write(wxT("NetworkDialupConnectionName"), m_strNetworkDialupConnectionName);
    pConfig->Write(wxT("NetworkDialupPromptCredentials"), m_bNetworkDialupPromptCredentials);

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
        wxASSERT(pView);

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


    wxASSERT(pConfig);
    wxASSERT(m_pNotebook);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    int         iCurrentPage;


    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("Language"), &m_iSelectedLanguage, 0L);
    pConfig->Read(wxT("ReminderFrequency"), &m_iReminderFrequency, 60L);

    pConfig->Read(wxT("NetworkConnectionType"), &m_iNetworkConnectionType, ID_NETWORKAUTODETECT);
    pConfig->Read(wxT("NetworkDialupConnectionName"), &m_strNetworkDialupConnectionName, wxEmptyString);
    pConfig->Read(wxT("NetworkDialupPromptCredentials"), &m_bNetworkDialupPromptCredentials, false);

    pConfig->Read(wxT("CurrentPage"), &iCurrentPage, 1);
    m_pNotebook->SetSelection(iCurrentPage);


    pConfig->Read(wxT("WindowIconized"), &bWindowIconized, false);
#if defined(__WXMSW__) || defined(__WXMAC__)
    pConfig->Read(wxT("WindowMaximized"), &bWindowMaximized, false);
#endif
    pConfig->Read(wxT("Width"), &iWidth, 800);
    pConfig->Read(wxT("Height"), &iHeight, 600);

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

    SetSize(iLeft, iTop, iWidth, iHeight);
#else       // ! __WXMAC__
    SetSize(-1, -1, iWidth, iHeight);
    Iconize(bWindowIconized);
#endif

#ifdef __WXMSW__ 
    Maximize(bWindowMaximized);
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
        wxASSERT(pView);

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

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(event.GetId()) {
    case ID_ACTIVITYRUNALWAYS:
        pDoc->SetActivityRunMode(RUN_MODE_ALWAYS);
        break;
    case ID_ACTIVITYSUSPEND:
        pDoc->SetActivityRunMode(RUN_MODE_NEVER);
        break;
    case ID_ACTIVITYRUNBASEDONPREPERENCES:
        pDoc->SetActivityRunMode(RUN_MODE_AUTO);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnActivitySelection - Function End"));
}


void CMainFrame::OnNetworkSelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNetworkSelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();
    int        iCurrentNetworkMode = -1;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(event.GetId()) {
    case ID_NETWORKRUNALWAYS:
        pDoc->SetNetworkRunMode(RUN_MODE_ALWAYS);
        break;
    case ID_NETWORKSUSPEND:
        pDoc->SetNetworkRunMode(RUN_MODE_NEVER);
        break;
    case ID_NETWORKRUNBASEDONPREPERENCES:
        pDoc->SetNetworkRunMode(RUN_MODE_AUTO);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNetworkSelection - Function End"));
}

   
void CMainFrame::OnRunBenchmarks(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnRunBenchmarks - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(m_pNotebook);
    wxASSERT(pDoc);
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

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pDlg);


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
            ShowAlert(
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
                aComputerNames.RemoveAt(lIndex);
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


void CMainFrame::OnToolsManageAccounts(wxCommandEvent& WXUNUSED(event))
{
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsManageAccounts - Function Begin"));

    int                       iAnswer = 0;
    wxString                  strTitle = wxEmptyString;
    CMainDocument*            pDoc = wxGetApp().GetDocument();
    CDlgAccountManagerSignup* pDlgSignup = new CDlgAccountManagerSignup(this);
    CDlgAccountManagerStatus* pDlgStatus = new CDlgAccountManagerStatus(this);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pDlgSignup);
    wxASSERT(pDlgStatus);

    ACCT_MGR_INFO ami;
    pDoc->rpc.acct_mgr_info(ami);
    if (ami.acct_mgr_url.size()) {
        strTitle = pDlgSignup->GetTitle();
        strTitle += wxT(" - ") + wxString(ami.acct_mgr_name.c_str());
        char buf[256];
        sprintf(buf, "Name: %s", ami.acct_mgr_name.c_str());
        pDlgStatus->SetAcctManagerName(wxT(buf));
        sprintf(buf, "URL: %s", ami.acct_mgr_url.c_str());
        pDlgStatus->SetAcctManagerURL(wxT(buf));
        pDlgStatus->SetTitle(strTitle);

        iAnswer = pDlgStatus->ShowModal();
    } else {
        if (ami.login_name.empty()) {
            iAnswer = ID_CHANGE;
        }
    }

    if (ID_CHANGE == iAnswer) {
        strTitle = pDlgSignup->GetTitle();
        if (!ami.acct_mgr_name.empty()) {
            strTitle += wxT(" - ") + wxString(ami.acct_mgr_name.c_str());
        } else if (!ami.acct_mgr_url.empty()) {
            strTitle += wxT(" - ") + wxString(ami.acct_mgr_url.c_str());
        }
        pDlgSignup->SetTitle(strTitle);
        pDlgSignup->m_strAcctManagerURL = ami.acct_mgr_url.c_str();
        pDlgSignup->m_strAcctManagerUsername = ami.login_name.c_str();
        pDlgSignup->m_strAcctManagerPassword = ami.password.c_str();
        iAnswer = pDlgSignup->ShowModal();
        if (wxID_OK == iAnswer) {
            ami.acct_mgr_url = pDlgSignup->m_strAcctManagerURL;
            ami.login_name = pDlgSignup->m_strAcctManagerUsername.c_str();
            ami.password = pDlgSignup->m_strAcctManagerPassword.c_str();
        }
    }

    if (wxID_CANCEL != iAnswer) {
        pDoc->rpc.acct_mgr_rpc(
            ami.acct_mgr_url.c_str(),
            ami.login_name.c_str(),
            ami.password.c_str()
        );
    }

    if (pDlgSignup)
        pDlgSignup->Destroy();
    if (pDlgStatus)
        pDlgStatus->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsManageAccounts - Function End"));
}


void CMainFrame::OnToolsOptions(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsOptions - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CDlgOptions*   pDlg = new CDlgOptions(this);
    int            iAnswer = 0;
    int            iBuffer = 0;
    wxString       strBuffer = wxEmptyString;
    wxArrayString  astrDialupConnections;
    bool           bRetrievedProxyConfiguration = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pDlg);

#ifdef __WXMSW__
    wxASSERT(m_pDialupManager);
#endif


    // General Tab
    pDlg->m_LanguageSelectionCtrl->Append(wxGetApp().GetSupportedLanguages());

    pDlg->m_LanguageSelectionCtrl->SetSelection(m_iSelectedLanguage);
    pDlg->m_ReminderFrequencyCtrl->SetValue(m_iReminderFrequency);

#ifdef __WXMSW__
    // Connection Tab
    m_pDialupManager->GetISPNames(astrDialupConnections);

    pDlg->m_DialupConnectionsCtrl->Append(astrDialupConnections);
    pDlg->SetDefaultConnectionType(m_iNetworkConnectionType);
    pDlg->SetDefaultDialupConnection(m_strNetworkDialupConnectionName);
    pDlg->SetDefaultDialupPromptCredentials(m_bNetworkDialupPromptCredentials);
#endif

    // Proxy Tabs
    bRetrievedProxyConfiguration = (0 == pDoc->GetProxyConfiguration());
    if(!bRetrievedProxyConfiguration) {
        // We were unable to get the proxy configuration, so disable
        //   the controls
        pDlg->m_EnableHTTPProxyCtrl->Enable(false);
        pDlg->m_EnableSOCKSProxyCtrl->Enable(false);
    } else {
        pDlg->m_EnableHTTPProxyCtrl->Enable(true);
        pDlg->m_EnableSOCKSProxyCtrl->Enable(true);
    }

    pDlg->m_EnableHTTPProxyCtrl->SetValue(pDoc->proxy_info.use_http_proxy);
    pDlg->m_HTTPAddressCtrl->SetValue(pDoc->proxy_info.http_server_name.c_str());
    pDlg->m_HTTPUsernameCtrl->SetValue(pDoc->proxy_info.http_user_name.c_str());
    pDlg->m_HTTPPasswordCtrl->SetValue(pDoc->proxy_info.http_user_passwd.c_str());

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
    pDlg->m_HTTPPortCtrl->SetValue(strBuffer);

    pDlg->m_EnableSOCKSProxyCtrl->SetValue(pDoc->proxy_info.use_socks_proxy);
    pDlg->m_SOCKSAddressCtrl->SetValue(pDoc->proxy_info.socks_server_name.c_str());
    pDlg->m_SOCKSUsernameCtrl->SetValue(pDoc->proxy_info.socks5_user_name.c_str());
    pDlg->m_SOCKSPasswordCtrl->SetValue(pDoc->proxy_info.socks5_user_passwd.c_str());

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
    pDlg->m_SOCKSPortCtrl->SetValue(strBuffer);

    iAnswer = pDlg->ShowModal();
    if (wxID_OK == iAnswer) {
        // General Tab
        if (m_iSelectedLanguage != pDlg->m_LanguageSelectionCtrl->GetSelection()) {
            ShowAlert(
                _("The BOINC Managers default language has been changed, in order for this change to take affect you must restart the manager."),
                _("Language Selection..."),
                wxICON_INFORMATION
           );
        }

        m_iSelectedLanguage = pDlg->m_LanguageSelectionCtrl->GetSelection();
        m_iReminderFrequency = pDlg->m_ReminderFrequencyCtrl->GetValue();

#ifdef __WXMSW__
        // Connection Tab
        m_iNetworkConnectionType = pDlg->GetDefaultConnectionType();
        m_strNetworkDialupConnectionName = pDlg->GetDefaultDialupConnection();
        m_bNetworkDialupPromptCredentials = pDlg->GetDefaultDialupPromptCredentials();
#endif

        // Proxy Tabs
        if (bRetrievedProxyConfiguration) {
            pDoc->proxy_info.use_http_proxy = pDlg->m_EnableHTTPProxyCtrl->GetValue();
            pDoc->proxy_info.http_server_name = pDlg->m_HTTPAddressCtrl->GetValue().c_str();
            pDoc->proxy_info.http_user_name = pDlg->m_HTTPUsernameCtrl->GetValue().c_str();
            pDoc->proxy_info.http_user_passwd = pDlg->m_HTTPPasswordCtrl->GetValue().c_str();

            strBuffer = pDlg->m_HTTPPortCtrl->GetValue();
            strBuffer.ToLong((long*)&iBuffer);
            pDoc->proxy_info.http_server_port = iBuffer;

            pDoc->proxy_info.use_socks_proxy = pDlg->m_EnableSOCKSProxyCtrl->GetValue();
            pDoc->proxy_info.socks_server_name = pDlg->m_SOCKSAddressCtrl->GetValue().c_str();
            pDoc->proxy_info.socks5_user_name = pDlg->m_SOCKSUsernameCtrl->GetValue().c_str();
            pDoc->proxy_info.socks5_user_passwd = pDlg->m_SOCKSPasswordCtrl->GetValue().c_str();

            strBuffer = pDlg->m_SOCKSPortCtrl->GetValue();
            strBuffer.ToLong((long*)&iBuffer);
            pDoc->proxy_info.socks_server_port = iBuffer;

            pDoc->SetProxyConfiguration();
        }
    }

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnToolsOptions - Function End"));
}


void CMainFrame::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function Begin"));

    if (IsShown()) {
        if (ID_FRAME == event.GetId()) {
            ExecuteBrowserLink(wxT("http://boinc.berkeley.edu/manager.php"));
        } else {
            event.Skip();
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function End"));
}


void CMainFrame::OnHelpBOINCManager(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function Begin"));

    if (IsShown()) {
        ExecuteBrowserLink(wxT("http://boinc.berkeley.edu/manager.php"));
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function End"));
}


void CMainFrame::OnHelpBOINCWebsite(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCWebsite - Function Begin"));

    if (IsShown()) {
        ExecuteBrowserLink(wxT("http://boinc.berkeley.edu"));
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCWebsite - Function End"));
}


void CMainFrame::OnHelpAbout(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpAbout - Function Begin"));

    CDlgAbout* pDlg = new CDlgAbout(this);
    wxASSERT(pDlg);

    pDlg->ShowModal();

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpAbout - Function End"));
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


void CMainFrame::OnAlert(CMainFrameAlertEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAlert - Function Begin"));

#ifdef __WXMSW__
    if (IsShown() && !event.notification_only) {
        ::wxMessageBox(event.message, event.title, event.style, this);
    } else {
        // If the main window is hidden or minimzed use the system tray ballon
        //   to notify the user instead.  This keeps dialogs from interfering
        //   with people typing email messages or any other activity where they
        //   do not want keyboard focus changed to another window while typing.
        CTaskBarIcon* taskbar = wxGetApp().GetTaskBarIcon();
        unsigned int  icon_type;
        wxASSERT(taskbar);

        if (wxICON_ERROR & event.style) {
            icon_type = NIIF_ERROR;
        } else if (wxICON_WARNING & event.style) {
            icon_type = NIIF_WARNING;
        } else if (wxICON_INFORMATION & event.style) {
            icon_type = NIIF_INFO;
        } else {
            icon_type = NIIF_NONE;
        }

        taskbar->SetBalloon(
            taskbar->m_iconTaskBarIcon,
            event.title,
            event.message,
            5000,
            icon_type
        );
    }
#else
    ::wxMessageBox(event.message, event.title, event.style, this);
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAlert - Function End"));
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


void CMainFrame::OnInitialized(CMainFrameEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnInitialized - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsConnected()) {
        pDoc->Connect(wxEmptyString, wxEmptyString, TRUE);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnInitialized - Function End"));
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

            wxASSERT(m_pNotebook);

            pwndNotebookPage = m_pNotebook->GetPage(m_pNotebook->GetSelection());
            wxASSERT(pwndNotebookPage);

            pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
            wxASSERT(pView);

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

    static bool       bAlreadyRunningLoop = false;
    static bool       bAlreadyRunOnce = false;
    static wxString   strCachedStatusText = wxEmptyString;

    CMainDocument*    pDoc = wxGetApp().GetDocument();

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        wxGetApp().UpdateSystemIdleDetection();

        if (!bAlreadyRunOnce) {
            // Complete any remaining initialization that has to happen after we are up
            //   and running
            FireInitialize();
            bAlreadyRunOnce = true;
        }

#ifdef __WXMSW__
        // Determine if we need to connect to the Internet
        //   Executes every ten seconds
        static wxDateTime   dtLastDialupAlertSent = wxDateTime((time_t)0);
        static wxDateTime   dtLastDialupRequest = wxDateTime((time_t)0);
        static wxDateTime   dtFirstDialupDisconnectEvent = wxDateTime((time_t)0);
        static bool         already_notified_update_all_projects = false;
        static bool         connected_successfully = false;
        static bool         reset_timers = false;
        static bool         was_dialing = false;
        bool                should_check_connection = false;
        bool                is_dialing = false;
        bool                is_online = false;
        int                 want_network = 0;
        int                 answer = 0;
        wxString            strConnectionName = wxEmptyString;
        wxString            strConnectionUsername = wxEmptyString;
        wxString            strConnectionPassword = wxEmptyString;
        wxString            strBuffer = wxEmptyString;
        wxTimeSpan          tsLastDialupAlertSent;
        wxTimeSpan          tsLastDialupRequest;
        wxTimeSpan          tsFirstDialupDisconnectEvent;

        wxASSERT(m_pDialupManager->IsOk());
        if (m_pDialupManager && pDoc) {
            // Are we configured to detect a network or told one already exists?
            if (ID_NETWORKLAN != m_iNetworkConnectionType) {
                if (ID_NETWORKDIALUP == m_iNetworkConnectionType) {
                    should_check_connection = true;
                }
                if (ID_NETWORKAUTODETECT == m_iNetworkConnectionType) {
                    if (!m_pDialupManager->IsAlwaysOnline()) {
                        should_check_connection = true;
                    }
                }
            }

            if (should_check_connection) {
                wxASSERT(wxDynamicCast(pDoc, CMainDocument));

                // cache the various states
                is_dialing = m_pDialupManager->IsDialing();
                is_online = m_pDialupManager->IsOnline();
                pDoc->rpc.network_query(want_network);

                wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Dialup Flags"));
                wxLogTrace(wxT("Function Status"), 
                    wxT("CMainFrame::OnFrameRender - -- is_online = '%d', is_dialing = '%d', was_dialing = '%d', want_network = '%d'"),
                    is_online, is_dialing, was_dialing, want_network
                );
                wxLogTrace(wxT("Function Status"),
                    wxT("CMainFrame::OnFrameRender - -- reset_timers = '%d', already_notified_update_all_projects = '%d', connected_successfully = '%d'"),
                    reset_timers, already_notified_update_all_projects, connected_successfully
                );

                // If we have received any connection event, then we should reset the
                //   dtLastDialupAlertSent and dtLastDialupRequest variables
                //   so that if we are disconnected without completing the user will
                //   be notifed in a prompt fashion.
                if (reset_timers) {
                    wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Resetting dial-up notification timers"));

                    reset_timers = false;
                    dtLastDialupAlertSent = wxDateTime((time_t)0);
                    dtLastDialupRequest = wxDateTime((time_t)0);
                }

                if (!is_online && !is_dialing && !was_dialing && want_network)
                {
                    wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Internet connection needed"));
                    if (!IsShown()) {
                        // BOINC Manager is hidden and displaying a dialog might interupt what they
                        //   are doing.
                        tsLastDialupAlertSent = wxDateTime::Now() - dtLastDialupAlertSent;
                        if (tsLastDialupAlertSent.GetSeconds() >= (m_iReminderFrequency * 60)) {
                            wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Manager not shown, notify instead"));

                            dtLastDialupAlertSent = wxDateTime::Now();

                            ShowAlert(
                                _("BOINC Manager - Network Status"),
                                _("BOINC needs a connection to the Internet to perform some maintenance, open the BOINC Manager to connect up and perform the needed work."),
                                wxICON_INFORMATION,
                                true
                            );
                        }
                    } else {
                        // BOINC Manager is visable and can process user input.
                        tsLastDialupRequest = wxDateTime::Now() - dtLastDialupRequest;
                        if (tsLastDialupRequest.GetSeconds() >= (m_iReminderFrequency * 60)) {
                            wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Begin connection process"));

                            dtLastDialupRequest = wxDateTime::Now();

                            if(pDoc->state.global_prefs.confirm_before_connecting) {
                                answer = ::wxMessageBox(
                                    _("BOINC needs to connect to the network.\nMay it do so now?"),
                                    _("BOINC Manager - Network Status"),
                                    wxYES_NO | wxICON_QUESTION,
                                    this
                                );
                            } else {
                                ShowAlert(
                                    _("BOINC Manager - Network Status"),
                                    _("BOINC is connecting to the internet."),
                                    wxICON_INFORMATION,
                                    true
                                );
                                answer = wxYES;
                            }

                            // Are we allow to connect?
                            if (wxYES == answer) {
                                if (m_strNetworkDialupConnectionName.size())
                                    strConnectionName = m_strNetworkDialupConnectionName;

                                if (m_bNetworkDialupPromptCredentials) {
                                    CDlgDialupCredentials* pDlgDialupCredentials = new CDlgDialupCredentials(this);

                                    answer = pDlgDialupCredentials->ShowModal();
                                    if (wxID_OK == answer) {
                                        strConnectionUsername = pDlgDialupCredentials->GetUsername();
                                        strConnectionPassword = pDlgDialupCredentials->GetPassword();
                                    }

                                    if (pDlgDialupCredentials) {
                                        pDlgDialupCredentials->Destroy();
                                    }
                                }

                                already_notified_update_all_projects = false;
                                connected_successfully = false;
                                m_pDialupManager->Dial(strConnectionName, strConnectionUsername, strConnectionPassword, true);
                            }
                        }
                    }
                } else if (!is_dialing && !was_dialing) {
                    if (is_online && want_network && !already_notified_update_all_projects) {
                        wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Connection Detected, notifing user of update to all projects"));

                        already_notified_update_all_projects = true;

                        // We are already online but BOINC for some reason is in a state
                        //   where it belives it has some pending work to do, so give it
                        //   a nudge
                        ShowAlert(
                            _("BOINC Manager - Network Status"),
                            _("BOINC has detected it is now connected to the internet, updating all projects and retrying all transfers."),
                            wxICON_INFORMATION,
                            true
                        );

                        // Sleep for a couple of seconds to let the network interface finish
                        //   initializing.
                        ::wxSleep(2);

                        // Signal BOINC to update all projects and transfers.
                        pDoc->rpc.network_available();

                    } else if (is_online && !want_network && connected_successfully) {
                        wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Connection Detected, Don't need the network, We successfully connected."));

                        // Should we disconnect now? The first time we see the disconnect event
                        //   we should ignore it and wait for 5 seconds to see if it is really
                        //   safe to disconnect.
                        if (wxDateTime((time_t)0) == dtFirstDialupDisconnectEvent) {
                            dtFirstDialupDisconnectEvent = wxDateTime::Now();
                        }
                        tsFirstDialupDisconnectEvent = wxDateTime::Now() - dtFirstDialupDisconnectEvent;
                        if (tsFirstDialupDisconnectEvent.GetSeconds() >= 5) {
                            if (pDoc->state.global_prefs.hangup_if_dialed) {
                                wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - Connection Detected, Don't need the network, Hanging up."));
                                if (m_pDialupManager->HangUp()) {
                                    ShowAlert(
                                        _("BOINC Manager - Network Status"),
                                        _("BOINC has successfully disconnected from the internet."),
                                        wxICON_INFORMATION,
                                        true
                                    );

                                    connected_successfully = false;
                                } else {
                                    ShowAlert(
                                        _("BOINC Manager - Network Status"),
                                        _("BOINC failed to disconnected from the internet."),
                                        wxICON_ERROR
                                    );
                                }
                            }
                        }
                    }
                } else if (!is_dialing && was_dialing) {
                    wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - We were dialing and now we are not, detect success or failure of the connection."));
                    was_dialing = false;
                    reset_timers = true;
                    if (is_online) {
                        ShowAlert(
                            _("BOINC Manager - Network Status"),
                            _("BOINC has successfully connected to the internet."),
                            wxICON_INFORMATION,
                            true
                        );
                        connected_successfully = true;
                    } else {
                        ShowAlert(
                            _("BOINC Manager - Network Status"),
                            _("BOINC failed to connect to the internet."),
                            wxICON_ERROR,
                            true
                        );
                        connected_successfully = false;
                    }
                } else if (is_dialing && !was_dialing) {
                    wxLogTrace(wxT("Function Status"), wxT("CMainFrame::OnFrameRender - We are now dialing, where before we were not."));
                    was_dialing = true;
                }
            }
        }
#endif

        if (IsShown()) {
            if (pDoc) {
                wxASSERT(wxDynamicCast(pDoc, CMainDocument));

                // Update the menu bar
                wxMenuBar* pMenuBar      = GetMenuBar();
                int        iActivityMode = -1;
                int        iNetworkMode  = -1;

                wxASSERT(pMenuBar);
                wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

                pMenuBar->Check(ID_ACTIVITYRUNALWAYS, false);
                pMenuBar->Check(ID_ACTIVITYSUSPEND, false);
                pMenuBar->Check(ID_ACTIVITYRUNBASEDONPREPERENCES, false);
                if ((pDoc->IsConnected()) && (0 == pDoc->GetActivityRunMode(iActivityMode))) {
                    if (iActivityMode == RUN_MODE_ALWAYS)
                        pMenuBar->Check(ID_ACTIVITYRUNALWAYS, true);
                    if (iActivityMode == RUN_MODE_NEVER)
                        pMenuBar->Check(ID_ACTIVITYSUSPEND, true);
                    if (iActivityMode == RUN_MODE_AUTO)
                        pMenuBar->Check(ID_ACTIVITYRUNBASEDONPREPERENCES, true);
                }

                pMenuBar->Check(ID_NETWORKRUNALWAYS, false);
                pMenuBar->Check(ID_NETWORKSUSPEND, false);
                pMenuBar->Check(ID_NETWORKRUNBASEDONPREPERENCES, false);
                if ((pDoc->IsConnected()) && (0 == pDoc->GetNetworkRunMode(iNetworkMode))) {
                    if (RUN_MODE_ALWAYS == iNetworkMode)
                        pMenuBar->Check(ID_NETWORKRUNALWAYS, true);
                    if (RUN_MODE_NEVER == iNetworkMode)
                        pMenuBar->Check(ID_NETWORKSUSPEND, true);
                    if (RUN_MODE_AUTO == iNetworkMode)
                        pMenuBar->Check(ID_NETWORKRUNBASEDONPREPERENCES, true);
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
                    // The Mac takes a huge performance hit changing the text of a floating
                    //   window, so don't change the text unless we really have too.
                    if (strStatusText != strCachedStatusText) {
                        strCachedStatusText = strStatusText;
                        m_pStatusbar->m_ptxtConnected->SetLabel(strStatusText);
                    }
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

    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    FireRefreshView();
    pDoc->CachedMessageUpdate();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnListPanelRender - Function End"));
}


void CMainFrame::OnDocumentPoll(wxTimerEvent& /*event*/) {
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->OnPoll();
}


void CMainFrame::OnNotebookSelectionChanged(wxNotebookEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNotebookSelectionChanged - Function Begin"));

    if ((-1 != event.GetSelection()) && IsShown()) {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;
        wxTimerEvent    timerEvent;

        wxASSERT(m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage(event.GetSelection());
        wxASSERT(pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        FireRefreshView();
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNotebookSelectionChanged - Function End"));
}


void CMainFrame::UpdateStatusText(const wxChar* szStatus) {
    wxString strStatus = szStatus;
    m_pStatusbar->SetStatusText(strStatus);
    ::wxSleep(0);
}


void CMainFrame::FireInitialize() {
    CMainFrameEvent event(wxEVT_MAINFRAME_INITIALIZED, this);
    AddPendingEvent(event);
}


void CMainFrame::FireRefreshView() {
    CMainFrameEvent event(wxEVT_MAINFRAME_REFRESHVIEW, this);
    AddPendingEvent(event);
}


void CMainFrame::FireConnect() {
    CMainFrameEvent event(wxEVT_MAINFRAME_CONNECT, this);
    AddPendingEvent(event);
}


void CMainFrame::ShowAlert( const wxString title, const wxString message, const int style, const bool notification_only ) {
    CMainFrameAlertEvent event(wxEVT_MAINFRAME_ALERT, this, title, message, style, notification_only);
    AddPendingEvent(event);
}


void CMainFrame::ExecuteBrowserLink(const wxString &strLink) {
    wxHyperLink::ExecuteLink(strLink);
}


const char *BOINC_RCSID_d881a56dc5 = "$Id$";
