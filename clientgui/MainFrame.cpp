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

#ifdef __APPLE__
#include "mac/MacGUI.pch"
#endif

#include "stdwx.h"
#include "hyperlink.h"
#include "BOINCGUIApp.h"
#include "MainFrame.h"
#include "Events.h"
#include "BOINCBaseView.h"
#include "BOINCDialUpManager.h"
#include "ViewProjects.h"
#include "ViewWork.h"
#include "ViewTransfers.h"
#include "ViewMessages.h"
#include "ViewStatistics.h"
#include "ViewResources.h"
#include "DlgAbout.h"
#include "DlgGenericMessage.h"
#include "DlgOptions.h"
#include "DlgSelectComputer.h"
#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"

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
    EVT_MENU(ID_FILERUNBENCHMARKS, CMainFrame::OnRunBenchmarks)
    EVT_MENU(ID_FILESELECTCOMPUTER, CMainFrame::OnSelectComputer)
    EVT_MENU(wxID_EXIT, CMainFrame::OnExit)
    EVT_MENU_RANGE(ID_FILEACTIVITYRUNALWAYS, ID_FILEACTIVITYSUSPEND, CMainFrame::OnActivitySelection)
    EVT_MENU_RANGE(ID_FILENETWORKRUNALWAYS, ID_FILENETWORKSUSPEND, CMainFrame::OnNetworkSelection)
    EVT_MENU(ID_PROJECTSATTACHACCOUNTMANAGER, CMainFrame::OnProjectsAttachToAccountManager)
    EVT_MENU(ID_TOOLSAMUPDATENOW, CMainFrame::OnAccountManagerUpdate)
    EVT_MENU(ID_ADVANCEDAMDEFECT, CMainFrame::OnAccountManagerDetach)
    EVT_MENU(ID_PROJECTSATTACHPROJECT, CMainFrame::OnProjectsAttachToProject)
    EVT_MENU(ID_COMMANDSRETRYCOMMUNICATIONS, CMainFrame::OnCommandsRetryCommunications)
    EVT_MENU(ID_OPTIONSOPTIONS, CMainFrame::OnOptionsOptions)
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


CMainFrame::CMainFrame(wxString title, wxIcon* icon) : 
    wxFrame ((wxFrame *)NULL, ID_FRAME, title, wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::CMainFrame - Function Begin"));
    
    m_pMenubar = NULL;
    m_pNotebook = NULL;
    m_pStatusbar = NULL;

    // Configuration Settings
    m_iSelectedLanguage = 0;
    m_iReminderFrequency = 0;
    m_iDisplayExitWarning = 1;

    m_strNetworkDialupConnectionName = wxEmptyString;


    // Working Variables
    m_strBaseTitle = title;

    m_aSelectedComputerMRU.Clear();


    // Initialize Application
    SetIcon(*icon);

    wxCHECK_RET(CreateMenu(), _T("Failed to create menu bar."));
    wxCHECK_RET(CreateNotebook(), _T("Failed to create notebook."));
    wxCHECK_RET(CreateStatusbar(), _T("Failed to create status bar."));

    RestoreState();

    SetStatusBarPane(0);


    m_pDialupManager = new CBOINCDialUpManager();
    wxASSERT(m_pDialupManager->IsOk());

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
    m_pFrameListPanelRenderTimer->Start(1000);       // Send event every 1 second
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

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    ACCT_MGR_INFO      ami;
    bool               is_acct_mgr_detected = false;
    wxString           strMenuName;
    wxString           strMenuDescription;
    
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Account managers have a different menu arrangement
    pDoc->rpc.acct_mgr_info(ami);
    is_acct_mgr_detected = ami.acct_mgr_url.size() ? true : false;

    // File menu
    wxMenu *menuFile = new wxMenu;

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Exit the %s"), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuFile->Append(
        wxID_EXIT,
        _("E&xit"),
        strMenuDescription
    );

    // Tools menu
    wxMenu *menuTools = new wxMenu;

    if (!is_acct_mgr_detected) {
        menuTools->Append(
            ID_PROJECTSATTACHPROJECT, 
            _("Attach to &project"),
            _("Attach to a project to begin processing work")
        );
        menuTools->Append(
            ID_PROJECTSATTACHACCOUNTMANAGER, 
            _("&Account manager"),
            _("Attach to an account manager")
        );
    } else {
        // %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        strMenuName.Printf(
            _("&Synchronize with %s"), 
            ami.acct_mgr_name.c_str()
        );
        // %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        strMenuDescription.Printf(
            _("Get current settings from %s"), 
            ami.acct_mgr_name.c_str()
        );
        menuTools->Append(
            ID_TOOLSAMUPDATENOW, 
            strMenuName,
            strMenuDescription
        );
    }

    // Activity menu
    wxMenu *menuActivity = new wxMenu;

    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYRUNALWAYS,
        _("&Run always"),
        _("Do work regardless of preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYRUNBASEDONPREPERENCES,
        _("Run based on &preferences"),
        _("Do work according to your preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYSUSPEND,
        _("&Suspend"),
        _("Stop work regardless of preferences")
    );

    menuActivity->AppendSeparator();

    menuActivity->AppendRadioItem(
        ID_FILENETWORKRUNALWAYS,
        _("&Network activity always available"),
        _("Do network activity regardless of preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILENETWORKRUNBASEDONPREPERENCES,
        _("Network activity based on &preferences"),
        _("Do network activity according to your preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILENETWORKSUSPEND,
        _("&Network activity suspended"),
        _("Stop BOINC network activity")
    );

    // Advanced menu
    wxMenu *menuAdvanced = new wxMenu;
    menuAdvanced->Append(
        ID_OPTIONSOPTIONS, 
        _("&Options"),
        _("Configure GUI options and proxy settings")
    );
    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strMenuDescription.Printf(
        _("Connect to another computer running %s"), 
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    menuAdvanced->Append(
        ID_FILESELECTCOMPUTER, 
        _("Select computer..."),
        strMenuDescription
    );
    menuAdvanced->Append(
        ID_FILERUNBENCHMARKS, 
        _("Run CPU &benchmarks"),
        _("Runs BOINC CPU benchmarks")
    );
    menuAdvanced->Append(
        ID_COMMANDSRETRYCOMMUNICATIONS, 
        _("Retry &communications"),
        _("Report completed work, get latest credit, "
          "get latest preferences, and possibly get more work.")
    );
    if (is_acct_mgr_detected) {
        // %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        strMenuName.Printf(
            _("&Defect from %s"), 
            ami.acct_mgr_name.c_str()
        );
        menuAdvanced->Append(
            ID_ADVANCEDAMDEFECT, 
            strMenuName,
            _("Remove client from account manager control.")
        );
        menuAdvanced->Append(
            ID_PROJECTSATTACHPROJECT, 
            _("Attach to &project"),
            _("Attach to a project to begin processing work")
        );
    }


    // Help menu
    wxMenu *menuHelp = new wxMenu;

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuName.Printf(
        _("&%s\tF1"), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about the %s"), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINCMANAGER,
        strMenuName, 
        strMenuDescription
    );

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strMenuName.Printf(
        _("%s &website"), 
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about BOINC and %s"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINC,
        strMenuName, 
        strMenuDescription
    );

    menuHelp->AppendSeparator();

    // %s is the project name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuName.Printf(
        _("&About %s..."), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuHelp->Append(
        wxID_ABOUT,
        strMenuName, 
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
        menuActivity,
        _("&Activity")
    );
    m_pMenubar->Append(
        menuAdvanced,
        _("A&dvanced")
    );
    m_pMenubar->Append(
        menuHelp,
        _("&Help")
    );

    wxMenuBar* m_pOldMenubar = GetMenuBar();
    SetMenuBar(m_pMenubar);
    if (m_pOldMenubar) {
        delete m_pOldMenubar;
    }

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
    
    iImageIndex = pImageList->Add(wxBitmap(pwndNewNotebookPage->GetViewIcon()));
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
    pConfig->Write(wxT("DisplayExitWarning"), m_iDisplayExitWarning);

    pConfig->Write(wxT("NetworkDialupConnectionName"), m_strNetworkDialupConnectionName);

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
    pConfig->Read(wxT("DisplayExitWarning"), &m_iDisplayExitWarning, 1L);

    pConfig->Read(wxT("NetworkDialupConnectionName"), &m_strNetworkDialupConnectionName, wxEmptyString);

    if (wxGetApp().GetBrand()->IsBranded() && 
        wxGetApp().GetBrand()->IsDefaultTabSpecified()) {
        m_pNotebook->SetSelection(wxGetApp().GetBrand()->GetDefaultTab());
    } else {
        pConfig->Read(wxT("CurrentPage"), &iCurrentPage, (ID_LIST_WORKVIEW - ID_LIST_BASE));
        m_pNotebook->SetSelection(iCurrentPage);
    }

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


void CMainFrame::OnActivitySelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnActivitySelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(event.GetId()) {
    case ID_FILEACTIVITYRUNALWAYS:
        pDoc->SetActivityRunMode(RUN_MODE_ALWAYS);
        break;
    case ID_FILEACTIVITYSUSPEND:
        pDoc->SetActivityRunMode(RUN_MODE_NEVER);
        break;
    case ID_FILEACTIVITYRUNBASEDONPREPERENCES:
        pDoc->SetActivityRunMode(RUN_MODE_AUTO);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnActivitySelection - Function End"));
}


void CMainFrame::OnNetworkSelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNetworkSelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(event.GetId()) {
    case ID_FILENETWORKRUNALWAYS:
        pDoc->SetNetworkRunMode(RUN_MODE_ALWAYS);
        break;
    case ID_FILENETWORKSUSPEND:
        pDoc->SetNetworkRunMode(RUN_MODE_NEVER);
        break;
    case ID_FILENETWORKRUNBASEDONPREPERENCES:
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

        // Make a null hostname be the same thing as localhost
        if (wxEmptyString == pDlg->m_ComputerNameCtrl->GetValue()) {
            lRetVal = pDoc->Connect(
                wxT("localhost"),
                wxEmptyString,
                TRUE,
                TRUE
            );
        } else {
            // Connect up to the remote machine
            lRetVal = pDoc->Connect(
                pDlg->m_ComputerNameCtrl->GetValue(), 
                pDlg->m_ComputerPasswordCtrl->GetValue(),
                TRUE,
                FALSE
            );
        }
        if (lRetVal) {
            ShowConnectionFailedAlert();
        }

        // Insert a copy of the current combo box value to the head of the
        //   computer names string array
        if (wxEmptyString != pDlg->m_ComputerNameCtrl->GetValue()) {
            aComputerNames.Insert(pDlg->m_ComputerNameCtrl->GetValue(), 0);
        }

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

    if (m_iDisplayExitWarning &&
        wxGetApp().GetBrand()->IsBranded() && 
        !wxGetApp().GetBrand()->GetExitMessage().IsEmpty()) {

        CDlgGenericMessage* pDlg = new CDlgGenericMessage(this);
        long                lAnswer = 0;

        wxString strMessage;
        if (wxGetApp().GetBrand()->IsBranded() && 
            !wxGetApp().GetBrand()->GetExitMessage().IsEmpty()) {
            strMessage = wxGetApp().GetBrand()->GetExitMessage();
        } else {
            strMessage = 
                _("This will shut down your tasks until it restarts automatically\n"
                  "following your user preferences. Close window to close the manager\n"
                  "without stopping the tasks.");
        }

        pDlg->SetTitle(_("Close Confirmation"));
        pDlg->m_DialogMessage->SetLabel(strMessage);
        pDlg->Fit();
        pDlg->Centre();

        lAnswer = pDlg->ShowModal();
        if (wxID_OK == lAnswer) {
            if (pDlg->m_DialogDisableMessage->GetValue()) {
                m_iDisplayExitWarning = 0;
            }
            Close(true);
        }

        if (pDlg)
            pDlg->Destroy();

    } else {
        Close(true);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnExit - Function End"));
}


void CMainFrame::OnProjectsAttachToAccountManager(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnProjectsAttachToAccountManager - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

#ifdef __WXMAC__
    if (!Mac_Authorize())
        return;
#endif

    if (pDoc->IsConnected()) {
        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAccountManager* pWizard = new CWizardAccountManager(this);

        pWizard->Run(ACCOUNTMANAGER_ATTACH);

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();
        FireRefreshView();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnProjectsAttachToAccountManager - Function End"));
}


void CMainFrame::OnAccountManagerUpdate(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAccountManagerUpdate - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

#ifdef __WXMAC__
    if (!Mac_Authorize())
        return;
#endif

    if (pDoc->IsConnected()) {
        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAccountManager* pWizard = new CWizardAccountManager(this);

        pWizard->Run(ACCOUNTMANAGER_UPDATE);

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();
        FireRefreshView();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAccountManagerUpdate - Function End"));
}


void CMainFrame::OnAccountManagerDetach(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAccountManagerDetach - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

#ifdef __WXMAC__
    if (!Mac_Authorize())
        return;
#endif

    if (pDoc->IsConnected()) {
        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAccountManager* pWizard = new CWizardAccountManager(this);

        pWizard->Run(ACCOUNTMANAGER_DETACH);

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();
        FireRefreshView();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAccountManagerDetach - Function End"));
}


void CMainFrame::OnProjectsAttachToProject( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnProjectsAttachToProject - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

#ifdef __WXMAC__
    if (!Mac_Authorize())
        return;
#endif

    if (pDoc->IsConnected()) {
        UpdateStatusText(_("Attaching to project..."));

        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAttachProject* pWizard = new CWizardAttachProject(this);

        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        pWizard->Run( strName, strURL, false );

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();

        UpdateStatusText(wxT(""));

        FireRefreshView();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnProjectsAttachToProject - Function End"));
}


void CMainFrame::OnCommandsRetryCommunications( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnCommandsRetryCommunications - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    UpdateStatusText(_("Retrying communications for project(s)..."));
    pDoc->rpc.network_available();
    UpdateStatusText(wxT(""));

    FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnCommandsRetryCommunications - Function End"));
}


void CMainFrame::OnOptionsOptions(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnOptionsOptions - Function Begin"));

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


    // General Tab
    pDlg->m_LanguageSelectionCtrl->Append(wxGetApp().GetSupportedLanguages());

    pDlg->m_LanguageSelectionCtrl->SetSelection(m_iSelectedLanguage);
    pDlg->m_ReminderFrequencyCtrl->SetValue(m_iReminderFrequency);

    // Connection Tab
    if (m_pDialupManager) {
        m_pDialupManager->GetISPNames(astrDialupConnections);

        pDlg->m_DialupConnectionsCtrl->Append(astrDialupConnections);
    } else {
        pDlg->m_DialupConnectionsCtrl->Append(astrDialupConnections);

        pDlg->m_DialupSetDefaultCtrl->Disable();
        pDlg->m_DialupClearDefaultCtrl->Disable();
    }

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
            wxString strDialogTitle;
            wxString strDialogMessage;

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogTitle.Printf(
                _("%s - Language Selection"),
                wxGetApp().GetBrand()->GetApplicationName().c_str()
            );

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogMessage.Printf(
                _("The %s's default language has been changed, in order for this "
                  "change to take affect you must restart the %s."),
                wxGetApp().GetBrand()->GetApplicationName().c_str(),
                wxGetApp().GetBrand()->GetApplicationName().c_str()
            );

            ShowAlert(
                strDialogTitle,
                strDialogMessage,
                wxICON_INFORMATION
           );
        }

        m_iSelectedLanguage = pDlg->m_LanguageSelectionCtrl->GetSelection();
        m_iReminderFrequency = pDlg->m_ReminderFrequencyCtrl->GetValue();

        // Connection Tab
        m_strNetworkDialupConnectionName = pDlg->GetDefaultDialupConnection();

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

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnOptionsOptions - Function End"));
}


void CMainFrame::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function Begin"));

    if (IsShown()) {
        if (ID_FRAME == event.GetId()) {
            wxString url = wxGetApp().GetBrand()->GetCompanyWebsite().c_str();
            url += wxT("manager.php");
            ExecuteBrowserLink(url);
        } else {
            event.Skip();
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function End"));
}


void CMainFrame::OnHelpBOINCManager(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function Begin"));

    if (IsShown()) {
        wxString url = wxGetApp().GetBrand()->GetCompanyWebsite().c_str();
        url += wxT("manager.php");
        ExecuteBrowserLink(url);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCManager - Function End"));
}


void CMainFrame::OnHelpBOINCWebsite(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnHelpBOINCWebsite - Function Begin"));

    if (IsShown()) {
        wxString url = wxGetApp().GetBrand()->GetCompanyWebsite().c_str();
        ExecuteBrowserLink(url);
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
    if (!event.CanVeto()) {
        Destroy();
    } else {
        Hide();
    }
#else
	event.Skip();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnClose - Function End"));
}


void CMainFrame::OnAlert(CMainFrameAlertEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAlert - Function Begin"));

#ifdef __WXMSW__
    CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
    wxASSERT(pTaskbar);

    if ((IsShown() && !event.m_notification_only) || (IsShown() && !pTaskbar->IsBalloonsSupported())) {
        if (!event.m_notification_only) {
            int retval = 0;
            retval = ::wxMessageBox(event.m_message, event.m_title, event.m_style, this);
            if (event.m_alert_event_type == AlertProcessResponse) {
                event.ProcessResponse(retval);
            }
        }
    } else {
        // If the main window is hidden or minimzed use the system tray ballon
        //   to notify the user instead.  This keeps dialogs from interfering
        //   with people typing email messages or any other activity where they
        //   do not want keyboard focus changed to another window while typing.
        unsigned int  icon_type;

        if (wxICON_ERROR & event.m_style) {
            icon_type = NIIF_ERROR;
        } else if (wxICON_WARNING & event.m_style) {
            icon_type = NIIF_WARNING;
        } else if (wxICON_INFORMATION & event.m_style) {
            icon_type = NIIF_INFO;
        } else {
            icon_type = NIIF_NONE;
        }

        pTaskbar->SetBalloon(
            pTaskbar->m_iconTaskBarIcon,
            event.m_title,
            event.m_message,
            5000,
            icon_type
        );
    }
#else
    ::wxMessageBox(event.m_message, event.m_title, event.m_style, this);
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnAlert - Function End"));
}


void CMainFrame::OnConnect(CMainFrameEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnect - Function Begin"));
    
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(m_pNotebook);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Update the menus
    DeleteMenu();
    CreateMenu();

    // Only present one of the wizards if no projects are currently
    //   detected.
    if (0 >= pDoc->GetProjectCount()) {
        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAttachProject* pAPWizard = NULL;
        CWizardAccountManager* pAMWizard = NULL;
        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        bool bCachedCredentials = false;
        ACCT_MGR_INFO ami;
        PROJECT_INIT_STATUS pis;

        pDoc->rpc.acct_mgr_info(ami);
        if (ami.acct_mgr_url.size()) {
            pAMWizard = new CWizardAccountManager(this);
            if (pAMWizard->Run()) {
                // If successful, hide the main window
                Hide();
            } else {
                // If failure, display the messages tab
                m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
            }
        } else {
            pAPWizard = new CWizardAttachProject(this);
            pDoc->rpc.get_project_init_status(pis);
            strName = pis.name.c_str();
            strURL = pis.url.c_str();
            bCachedCredentials = pis.url.length() && pis.has_account_key;
            if (pAPWizard->Run(strName, strURL, bCachedCredentials)) {
                // If successful, display the work tab
                m_pNotebook->SetSelection(ID_LIST_WORKVIEW - ID_LIST_BASE);
            } else {
                // If failure, display the messages tab
                m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
            }
        }

        if (pAMWizard)
            pAMWizard->Destroy();
        if (pAPWizard)
            pAPWizard->Destroy();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnConnect - Function End"));
}


void CMainFrame::OnInitialized(CMainFrameEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnInitialized - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsConnected()) {
        pDoc->Connect(wxT("localhost"), wxEmptyString, TRUE, TRUE);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnInitialized - Function End"));
}


void CMainFrame::OnRefreshView(CMainFrameEvent&) {
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
}


void CMainFrame::OnRefreshState(wxTimerEvent &event) {
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
}


void CMainFrame::OnFrameRender(wxTimerEvent &event) {
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

        // Check to see if there is anything that we need to do from the
        //   dial up user perspective.
        if (pDoc->IsConnected()) {
            if (m_pDialupManager) {
                m_pDialupManager->poll();
            }
        }

        if (IsShown()) {
            if (pDoc) {
                wxASSERT(wxDynamicCast(pDoc, CMainDocument));

                // Update the menu bar
                wxMenuBar* pMenuBar      = GetMenuBar();
                int        iActivityMode = -1;
                int        iNetworkMode  = -1;

                wxASSERT(pMenuBar);
                wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

                pMenuBar->Check(ID_FILEACTIVITYRUNALWAYS, false);
                pMenuBar->Check(ID_FILEACTIVITYSUSPEND, false);
                pMenuBar->Check(ID_FILEACTIVITYRUNBASEDONPREPERENCES, false);
                if ((pDoc->IsConnected()) && (0 == pDoc->GetActivityRunMode(iActivityMode))) {
                    if (iActivityMode == RUN_MODE_ALWAYS)
                        pMenuBar->Check(ID_FILEACTIVITYRUNALWAYS, true);
                    if (iActivityMode == RUN_MODE_NEVER)
                        pMenuBar->Check(ID_FILEACTIVITYSUSPEND, true);
                    if (iActivityMode == RUN_MODE_AUTO)
                        pMenuBar->Check(ID_FILEACTIVITYRUNBASEDONPREPERENCES, true);
                }

                pMenuBar->Check(ID_FILENETWORKRUNALWAYS, false);
                pMenuBar->Check(ID_FILENETWORKSUSPEND, false);
                pMenuBar->Check(ID_FILENETWORKRUNBASEDONPREPERENCES, false);
                if ((pDoc->IsConnected()) && (0 == pDoc->GetNetworkRunMode(iNetworkMode))) {
                    if (RUN_MODE_ALWAYS == iNetworkMode)
                        pMenuBar->Check(ID_FILENETWORKRUNALWAYS, true);
                    if (RUN_MODE_NEVER == iNetworkMode)
                        pMenuBar->Check(ID_FILENETWORKSUSPEND, true);
                    if (RUN_MODE_AUTO == iNetworkMode)
                        pMenuBar->Check(ID_FILENETWORKRUNBASEDONPREPERENCES, true);
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

                    if (pDoc->IsComputerNameLocal(strComputerName)) {
                        strTitle += wxT(" - (localhost)");
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

                    // The Mac takes a huge performance hit redrawing this window, 
                    //   window, so don't change the text unless we really have too.
                    if (GetTitle() != strTitle)
                        SetTitle(strTitle);
                        
                    if (strStatusText != strCachedStatusText) {
                        strCachedStatusText = strStatusText;
                        m_pStatusbar->m_ptxtConnected->SetLabel(strStatusText);
                    }
                } else {
                    m_pStatusbar->m_pbmpConnected->Hide();
                    m_pStatusbar->m_ptxtConnected->Hide();
                    m_pStatusbar->m_pbmpDisconnect->Show();
                    m_pStatusbar->m_ptxtDisconnect->Show();

                    if (GetTitle() != m_strBaseTitle)
                        SetTitle(m_strBaseTitle);
                }
            }
        }

        bAlreadyRunningLoop = false;
    }

    event.Skip();
}


void CMainFrame::OnListPanelRender(wxTimerEvent&) {
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    FireRefreshView();
    pDoc->CachedMessageUpdate();
    SetFrameListPanelRenderTimerRate();   // Set to refresh every 5 or 60 seconds
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

        SetFrameListPanelRenderTimerRate();
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::OnNotebookSelectionChanged - Function End"));
}


void CMainFrame::SetFrameListPanelRenderTimerRate() {
    static wxWindowID   previousPane = -1;
    static int          connectedCount = 0;
    wxWindowID          currentPane;

    CMainDocument*      pDoc = wxGetApp().GetDocument();
   
    wxASSERT(m_pNotebook);
    wxASSERT(m_pFrameListPanelRenderTimer);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Keep timer at faster rate until we have been connected > 10 seconds
    if (!pDoc->IsConnected())
        connectedCount = 0;
        
    if (connectedCount < 3) {
        connectedCount++;
        previousPane = -1;  // Ensure an update when connectedCount reaches 3
        
        if (m_pFrameListPanelRenderTimer->IsRunning())
            m_pFrameListPanelRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Start(5000);  // Refresh every 5 seconds
        return;
    }
    
    currentPane = m_pNotebook->GetSelection() + ID_TASK_BASE;
    if (currentPane == previousPane) 
        return;
        
    previousPane = currentPane;
    if (m_pFrameListPanelRenderTimer->IsRunning())
        m_pFrameListPanelRenderTimer->Stop();

    switch (currentPane) {
    case ID_TASK_STATISTICSVIEW: 
        m_pFrameListPanelRenderTimer->Start(60000); // Refresh every 1 minute
        break;
    default:
        m_pFrameListPanelRenderTimer->Start(5000);  // Refresh every 5 seconds
        break;
    }
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


void CMainFrame::ShowConnectionBadPasswordAlert() {
    wxString            strDialogTitle = wxEmptyString;

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::ShowConnectionBadPasswordAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Error"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );

    ShowAlert(
        strDialogTitle,
        _("The password you have provided is incorrect, please try again."),
        wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::ShowConnectionBadPasswordAlert - Function End"));
}


void CMainFrame::ShowConnectionFailedAlert() {
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::ShowConnectionFailedAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Failed"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2st %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not able to connect to a %s client.\n"
          "Would you like to try to connect again?"),
        wxGetApp().GetBrand()->GetApplicationName().c_str(),
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );

    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxYES_NO | wxICON_QUESTION,
        false,
        AlertProcessResponse
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::ShowConnectionFailedAlert - Function End"));
}


void CMainFrame::ShowNotCurrentlyConnectedAlert() {
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::ShowNotCurrentlyConnectedAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Status"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    // 3nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not currently connected to a %s client.\n"
          "Please use the 'File\\Select Computer...' menu option to connect up to a %s client.\n"
          "To connect up to your local computer please use 'localhost' as the host name."),
        wxGetApp().GetBrand()->GetApplicationName().c_str(),
        wxGetApp().GetBrand()->GetProjectName().c_str(),
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CMainFrame::ShowNotCurrentlyConnectedAlert - Function End"));
}


void CMainFrame::ShowAlert( const wxString title, const wxString message, const int style, const bool notification_only, const MainFrameAlertEventType alert_event_type ) {
    CMainFrameAlertEvent event(wxEVT_MAINFRAME_ALERT, this, title, message, style, notification_only, alert_event_type);
    AddPendingEvent(event);
}


void CMainFrame::ExecuteBrowserLink(const wxString &strLink) {
    wxHyperLink::ExecuteLink(strLink);
}


#ifdef __WXMAC__

bool CMainFrame::Show(bool show) {
    ProcessSerialNumber psn;

    GetCurrentProcess(&psn);
    if (show) {
        SetFrontProcess(&psn);  // Shows process if hidden
    } else
        if (IsProcessVisible(&psn))
            ShowHideProcess(&psn, false);
    
    return wxFrame::Show(show);
}

#endif // __WXMAC__


void CMainFrameAlertEvent::ProcessResponse(const int response) const {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
   
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if ((AlertProcessResponse == m_alert_event_type) && (wxYES == response)) {
        pDoc->Reconnect();
    }
}


const char *BOINC_RCSID_d881a56dc5 = "$Id$";
