// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//
// $Log$
// Revision 1.13  2004/05/27 06:17:57  rwalton
// *** empty log message ***
//
// Revision 1.12  2004/05/24 23:50:14  rwalton
// *** empty log message ***
//
// Revision 1.11  2004/05/22 01:36:36  rwalton
// *** empty log message ***
//
// Revision 1.10  2004/05/21 06:27:15  rwalton
// *** empty log message ***
//
// Revision 1.9  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "MainFrame.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainFrame.h"
#include "BaseListCtrlView.h"
#include "BaseWindowView.h"
#include "MessagesView.h"
#include "ProjectsView.h"
#include "ResourceUtilizationView.h"
#include "TransfersView.h"
#include "WorkView.h"
#include "DlgAbout.h"
#include "DlgOptions.h"
#include "DlgAttachProject.h"

#include "res/BOINCGUIApp.xpm"


IMPLEMENT_DYNAMIC_CLASS(CMainFrame, wxFrame)

BEGIN_EVENT_TABLE (CMainFrame, wxFrame)
    EVT_CLOSE       (                           CMainFrame::OnClose)

    EVT_MENU        (wxID_EXIT,                 CMainFrame::OnExit)
    EVT_MENU        (ID_COMMANDSATTACHPROJECT,  CMainFrame::OnCommandsAttachProject)
    EVT_MENU        (ID_TOOLSOPTIONS,           CMainFrame::OnToolsOptions)
    EVT_MENU        (wxID_ABOUT,                CMainFrame::OnAbout)

    EVT_TIMER       (ID_FRAMERENDERTIMER,       CMainFrame::OnFrameRender)
END_EVENT_TABLE ()

CMainFrame::CMainFrame()
{
    wxLogTrace("CMainFrame::CMainFrame - Function Begining");

    wxLogTrace("CMainFrame::CMainFrame - Function Ending");
}


CMainFrame::CMainFrame(wxString strTitle) : 
    wxFrame ((wxFrame *)NULL, -1, strTitle, wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace("CMainFrame::CMainFrame - Function Begining");


    m_pMenubar = NULL;
    m_pNotebook = NULL;
    m_pStatusbar = NULL;


    SetIcon(wxICON(APP_ICON));


    wxCHECK_RET(CreateMenu(), _T("Failed to create menu bar."));
    wxCHECK_RET(CreateNotebook(), _T("Failed to create notebook."));
    wxCHECK_RET(CreateStatusbar(), _T("Failed to create status bar."));


    m_pFrameRenderTimer = new wxTimer(this, ID_FRAMERENDERTIMER);
    wxASSERT(NULL != m_pFrameRenderTimer);

    m_pFrameRenderTimer->Start(1000);       // Send event every second


    RestoreState();


    wxLogTrace("CMainFrame::CMainFrame - Function Ending");
}


CMainFrame::~CMainFrame(){
    wxLogTrace("CMainFrame::~CMainFrame - Function Begining");


    wxASSERT(NULL != m_pFrameRenderTimer);
    wxASSERT(NULL != m_pMenubar);
    wxASSERT(NULL != m_pNotebook);
    wxASSERT(NULL != m_pStatusbar);


    SaveState();


    if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
    }

    if (m_pStatusbar)
        wxCHECK_RET(DeleteStatusbar(), _T("Failed to delete status bar."));

    if (m_pNotebook)
        wxCHECK_RET(DeleteNotebook(), _T("Failed to delete notebook."));

    if (m_pMenubar)
        wxCHECK_RET(DeleteMenu(), _T("Failed to delete menu bar."));


    wxLogTrace("CMainFrame::~CMainFrame - Function Ending");
}


bool CMainFrame::CreateMenu() {
    wxLogTrace("CMainFrame::CreateMenu - Function Begining");


    // File menu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT, _("E&xit"));

    // Commands menu
    wxMenu *menuCommands = new wxMenu;
    menuCommands->Append(ID_COMMANDSATTACHPROJECT, _("&Attach to Project..."));

    // Tools menu
    wxMenu *menuTools = new wxMenu;
    menuTools->Append(ID_TOOLSOPTIONS, _("&Options"));

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, _("&About BOINC..."));

    // construct menu
    m_pMenubar = new wxMenuBar;
    m_pMenubar->Append(menuFile,      _("&File"));
    m_pMenubar->Append(menuCommands,  _("&Commands"));
    m_pMenubar->Append(menuTools,     _("&Tools"));
    m_pMenubar->Append(menuHelp,      _("&Help"));
    SetMenuBar(m_pMenubar);


    wxLogTrace("CMainFrame::CreateMenu - Function Ending");
    return true;
}


bool CMainFrame::CreateNotebook() {
    wxLogTrace("CMainFrame::CreateNotebook - Function Begining");


    // create frame panel
    wxPanel *pPanel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize,
                                 wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxNO_BORDER);

    // initialize notebook
    m_pNotebook = new wxNotebook(pPanel, -1, wxDefaultPosition, wxDefaultSize,
                                wxNB_FIXEDWIDTH|wxCLIP_CHILDREN);

    wxNotebookSizer *pNotebookSizer = new wxNotebookSizer(m_pNotebook);

    // layout frame panel
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    pPanelSizer->Add(new wxStaticLine(pPanel, -1), 0, wxEXPAND);
    pPanelSizer->Add(0, 4);
    pPanelSizer->Add(pNotebookSizer, 1, wxEXPAND);
    pPanel->SetAutoLayout(true);
    pPanel->SetSizerAndFit(pPanelSizer);

    CreateNotebookPage(new CProjectsView(m_pNotebook));
    CreateNotebookPage(new CWorkView(m_pNotebook));
    CreateNotebookPage(new CTransfersView(m_pNotebook));
    CreateNotebookPage(new CMessagesView(m_pNotebook));
    CreateNotebookPage(new CResourceUtilizationView(m_pNotebook));


    wxLogTrace("CMainFrame::CreateNotebook - Function Ending");
    return true;
}


template < class T >
bool CMainFrame::CreateNotebookPage(T pwndNewNotebookPage) {
    wxLogTrace("CMainFrame::CreateNotebookPage - Function Begining");


    wxImageList*    pImageList;
    int             iImageIndex = 0;

    wxASSERT(NULL != pwndNewNotebookPage);
    wxASSERT(NULL != m_pNotebook);
    wxASSERT(wxDynamicCast(pwndNewNotebookPage, CBaseListCtrlView) ||
             wxDynamicCast(pwndNewNotebookPage, CBaseWindowView));


    pImageList = m_pNotebook->GetImageList();
    if (!pImageList) {
        pImageList = new wxImageList(16, 16, true, 0);
        wxASSERT(pImageList != NULL);
        m_pNotebook->SetImageList(pImageList);
    }
    
    iImageIndex = pImageList->Add(wxBitmap(pwndNewNotebookPage->GetViewIcon()), wxColour(255, 0, 255));
    m_pNotebook->AddPage(pwndNewNotebookPage, pwndNewNotebookPage->GetViewName(), TRUE, iImageIndex);


    wxLogTrace("CMainFrame::CreateNotebookPage - Function Ending");
    return true;
}


bool CMainFrame::CreateStatusbar() {
    wxLogTrace("CMainFrame::CreateStatusbar - Function Begining");


    if (m_pStatusbar)
        return true;

    int ch = GetCharWidth();

    const int widths[] = {-1, 20*ch, 15};

    m_pStatusbar = CreateStatusBar(WXSIZEOF(widths), wxST_SIZEGRIP, ID_STATUSBAR);
    wxASSERT(NULL != m_pStatusbar);

    m_pStatusbar->SetStatusWidths(WXSIZEOF(widths), widths);

    SetStatusBar(m_pStatusbar);
    SendSizeEvent();


    wxLogTrace("CMainFrame::CreateStatusbar - Function Ending");
    return true;
}


bool CMainFrame::DeleteMenu() {
    wxLogTrace("CMainFrame::DeleteMenu - Function Begining");

    wxLogTrace("CMainFrame::DeleteMenu - Function Ending");
    return true;
}


bool CMainFrame::DeleteNotebook() {
    wxLogTrace("CMainFrame::DeleteNotebook - Function Begining");


    wxImageList*    pImageList;

    wxASSERT(NULL != m_pNotebook);

    pImageList = m_pNotebook->GetImageList();

    wxASSERT(NULL != pImageList);

    if (pImageList)
        delete pImageList;


    wxLogTrace("CMainFrame::DeleteNotebook - Function Ending");
    return true;
}


bool CMainFrame::DeleteStatusbar() {
    wxLogTrace("CMainFrame::DeleteStatusbar - Function Begining");


    if (!m_pStatusbar)
        return true;

    SetStatusBar(NULL);

    delete m_pStatusbar;

    m_pStatusbar = NULL;
    SendSizeEvent();


    wxLogTrace("CMainFrame::DeleteStatusbar - Function Ending");
    return true;
}


bool CMainFrame::SaveState() {
    wxLogTrace("CMainFrame::SaveState - Function Begining");

    wxString        strBaseConfigLocation = wxString(_T("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);


    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pNotebook);


    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write("CurrentPage", m_pNotebook->GetSelection());


    //
    // Save Page(s) State
    //

    wxWindow*       pwndNotebookPage = NULL;
    wxString        strConfigLocation = wxString(_T(""));
    int             iIndex;
    int             iPageCount;

    iPageCount = m_pNotebook->GetPageCount() - 1;

    for ( iIndex = 0; iIndex <= iPageCount; iIndex++ ) {   

        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBaseListCtrlView) ||
                 wxDynamicCast(pwndNotebookPage, CBaseWindowView));

        if        (wxDynamicCast(pwndNotebookPage, CProjectsView)) {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CProjectsView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CWorkView)) {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CWorkView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CTransfersView)) {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CTransfersView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CMessagesView)) {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CMessagesView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CResourceUtilizationView)) {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CResourceUtilizationView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CBaseListCtrlView)) {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CBaseListCtrlView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CBaseWindowView)) {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CBaseWindowView), pConfig);
        }
    }


    wxLogTrace("CMainFrame::SaveState - Function Ending");
    return true;
}


template < class T >
bool CMainFrame::FireSaveStateEvent( T pPage, wxConfigBase* pConfig ) {
    wxLogTrace("CMainFrame::FireSaveStateEvent - Function Begining");

    wxString strPreviousLocation = wxString(_T(""));
    wxString strConfigLocation = wxString(_T(""));

    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + pPage->GetViewName();

    pConfig->SetPath(strConfigLocation);
    pPage->OnSaveState();
    pConfig->SetPath(strPreviousLocation);

    wxLogTrace("CMainFrame::FireSaveStateEvent - Function Ending");
    return true;
}


bool CMainFrame::RestoreState() {
    wxLogTrace("CMainFrame::RestoreState - Function Begining");

    wxString        strBaseConfigLocation = wxString(_T("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);


    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pNotebook);


    //
    // Restore Frame State
    //
    long            lCurrentPage;


    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read("CurrentPage", &lCurrentPage);
    m_pNotebook->SetSelection(lCurrentPage);


    //
    // Restore Page(s) State
    //

    wxWindow*       pwndNotebookPage = NULL;
    wxString        strConfigLocation = wxString(_T(""));
    int             iIndex;
    int             iPageCount;

    iPageCount = m_pNotebook->GetPageCount() - 1;

    for ( iIndex = 0; iIndex <= iPageCount; iIndex++ ) {   

        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBaseListCtrlView) ||
                 wxDynamicCast(pwndNotebookPage, CBaseWindowView));

        if        (wxDynamicCast(pwndNotebookPage, CProjectsView)) {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CProjectsView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CWorkView)) {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CWorkView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CTransfersView)) {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CTransfersView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CMessagesView)) {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CMessagesView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CResourceUtilizationView)) {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CResourceUtilizationView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CBaseListCtrlView)) {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CBaseListCtrlView), pConfig);
        } else if (wxDynamicCast(pwndNotebookPage, CBaseWindowView)) {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CBaseWindowView), pConfig);
        }
    }


    wxLogTrace("CMainFrame::RestoreState - Function Ending");
    return true;
}


template < class T >
bool CMainFrame::FireRestoreStateEvent( T pPage, wxConfigBase* pConfig ) {
    wxLogTrace("CMainFrame::FireRestoreStateEvent - Function Begining");

    wxString strPreviousLocation = wxString(_T(""));
    wxString strConfigLocation = wxString(_T(""));

    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + pPage->GetViewName();

    pConfig->SetPath(strConfigLocation);
    pPage->OnRestoreState();
    pConfig->SetPath(strPreviousLocation);

    wxLogTrace("CMainFrame::FireRestoreStateEvent - Function Ending");
    return true;
}


void CMainFrame::OnExit(wxCommandEvent &WXUNUSED(event)) {
    wxLogTrace("CMainFrame::OnExit - Function Begining");

    Close(true);

    wxLogTrace("CMainFrame::OnExit - Function Ending");
}


void CMainFrame::OnClose(wxCloseEvent &WXUNUSED(event)) {
    wxLogTrace("CMainFrame::OnClose - Function Begining");

    Destroy();

    wxLogTrace("CMainFrame::OnClose - Function Ending");
}


void CMainFrame::OnCommandsAttachProject(wxCommandEvent &WXUNUSED(event)) {
    wxLogTrace("CMainFrame::OnCommandsAttachProject - Function Begining");


    CDlgAttachProject* pDlg = new CDlgAttachProject(this);
    wxASSERT(NULL != pDlg);

    pDlg->ShowModal();

    if (pDlg)
        delete pDlg;


    wxLogTrace("CMainFrame::OnCommandsAttachProject - Function Ending");
}


void CMainFrame::OnToolsOptions(wxCommandEvent &WXUNUSED(event)) {
    wxLogTrace("CMainFrame::OnToolsOptions - Function Begining");


    CDlgOptions* pDlg = new CDlgOptions(this);
    wxASSERT(NULL != pDlg);

    pDlg->ShowModal();

    if (pDlg)
        delete pDlg;


    wxLogTrace("CMainFrame::OnToolsOptions - Function Ending");
}


void CMainFrame::OnAbout(wxCommandEvent &WXUNUSED(event)) {
    wxLogTrace("CMainFrame::OnAbout - Function Begining");


    CDlgAbout* pDlg = new CDlgAbout(this);
    wxASSERT(NULL != pDlg);

    pDlg->ShowModal();

    if (pDlg)
        delete pDlg;


    wxLogTrace("CMainFrame::OnAbout - Function Ending");
}


void CMainFrame::OnFrameRender (wxTimerEvent &event) {
    wxLogTrace("CMainFrame::OnFrameRender - Function Begining");

    wxWindow*       pwndNotebookPage;

    wxASSERT(NULL != m_pNotebook);


    pwndNotebookPage = m_pNotebook->GetPage(m_pNotebook->GetSelection());
    wxASSERT(NULL != pwndNotebookPage);
    wxASSERT(wxDynamicCast(pwndNotebookPage, CBaseListCtrlView) ||
             wxDynamicCast(pwndNotebookPage, CBaseWindowView));

    if        (wxDynamicCast(pwndNotebookPage, CProjectsView)) {
        FireRenderEvent(wxDynamicCast(pwndNotebookPage, CProjectsView), event);
    } else if (wxDynamicCast(pwndNotebookPage, CWorkView)) {
        FireRenderEvent(wxDynamicCast(pwndNotebookPage, CWorkView), event);
    } else if (wxDynamicCast(pwndNotebookPage, CTransfersView)) {
        FireRenderEvent(wxDynamicCast(pwndNotebookPage, CTransfersView), event);
    } else if (wxDynamicCast(pwndNotebookPage, CMessagesView)) {
        FireRenderEvent(wxDynamicCast(pwndNotebookPage, CMessagesView), event);
    } else if (wxDynamicCast(pwndNotebookPage, CResourceUtilizationView)) {
        FireRenderEvent(wxDynamicCast(pwndNotebookPage, CResourceUtilizationView), event);
    } else if (wxDynamicCast(pwndNotebookPage, CBaseListCtrlView)) {
        FireRenderEvent(wxDynamicCast(pwndNotebookPage, CBaseListCtrlView), event);
    } else if (wxDynamicCast(pwndNotebookPage, CBaseWindowView)) {
        FireRenderEvent(wxDynamicCast(pwndNotebookPage, CBaseWindowView), event);
    }


    wxLogTrace("CMainFrame::OnFrameRender - Function Ending");
}


template < class T >
void CMainFrame::FireRenderEvent( T pPage, wxTimerEvent &event ) {
    wxLogTrace("CMainFrame::FireRenderEvent - Function Begining");

    pPage->OnRender(event);

    wxLogTrace("CMainFrame::FireRenderEvent - Function Ending");
}

