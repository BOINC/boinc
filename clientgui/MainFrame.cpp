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

#include "stdwx.h"
#include "boincguiapp.h"
#include "mainframe.h"
#include "baselistctrlview.h"
#include "basewindowview.h"
#include "messagesview.h"
#include "projectsview.h"
#include "resourceutilizationview.h"
#include "transfersview.h"
#include "workview.h"
#include "events.h"


IMPLEMENT_DYNAMIC_CLASS(CMainFrame, wxFrame)


BEGIN_EVENT_TABLE (CMainFrame, wxFrame)
    EVT_CLOSE       (                       CMainFrame::OnClose)
    EVT_IDLE        (                       CMainFrame::OnIdle)
    EVT_MENU        (wxID_EXIT,             CMainFrame::OnExit)
    EVT_MENU        (wxID_ABOUT,            CMainFrame::OnAbout)
    EVT_MENU        (ID_STATUSBAR,          CMainFrame::OnStatusbar)
    EVT_UPDATE_UI   (ID_STATUSBAR,          CMainFrame::OnStatusbarUI)
END_EVENT_TABLE ()


CMainFrame::CMainFrame()
{
}


CMainFrame::CMainFrame(wxString strTitle) : 
    wxFrame ((wxFrame *)NULL, -1, strTitle, wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    m_pMenubar = NULL;
    m_pNotebook = NULL;
    m_pStatusbar = NULL;

    SetIcon(wxICON(APP_ICON));

    wxCHECK_RET(CreateMenu(), _T("Failed to create menu bar."));
    wxCHECK_RET(CreateNotebook(), _T("Failed to create notebook."));
    wxCHECK_RET(CreateStatusbar(), _T("Failed to create status bar."));
}


CMainFrame::~CMainFrame(void)
{
    if (m_pMenubar)
        wxCHECK_RET(DeleteMenu(), _T("Failed to delete menu bar."));

    if (m_pNotebook)
        wxCHECK_RET(DeleteNotebook(), _T("Failed to delete notebook."));

    if (m_pStatusbar)
        wxCHECK_RET(DeleteStatusbar(), _T("Failed to delete status bar."));
}


bool CMainFrame::CreateMenu() {

    // File menu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT, _("E&xit"));

    // Commands menu
    wxMenu *menuCommands = new wxMenu;
    menuCommands->Append(wxID_EXIT, _("&Attach to Project..."));

    // Tools menu
    wxMenu *menuTools = new wxMenu;
    menuTools->Append(wxID_EXIT, _("&Options"));

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

    return true;
}


bool CMainFrame::CreateNotebook() {

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
    pPanel->SetSizerAndFit(pPanelSizer);

    CreateNotebookPage(new CProjectsView(m_pNotebook));
    CreateNotebookPage(new CWorkView(m_pNotebook));
    CreateNotebookPage(new CTransfersView(m_pNotebook));
    CreateNotebookPage(new CMessagesView(m_pNotebook));
    CreateNotebookPage(new CResourceUtilizationView(m_pNotebook));

    return true;
}


bool CMainFrame::CreateNotebookPage(wxWindow* pwndNewNotebookPage) {

    wxASSERT(pwndNewNotebookPage->IsKindOf(CLASSINFO(CBaseListCtrlView)) ||
             pwndNewNotebookPage->IsKindOf(CLASSINFO(CBaseWindowView)));

    if(pwndNewNotebookPage->IsKindOf(CLASSINFO(CBaseListCtrlView)))
    {
        CBaseListCtrlView* pPage = (CBaseListCtrlView*)pwndNewNotebookPage;
        m_pNotebook->AddPage(pPage, pPage->GetViewName(), true, -1);
    }
    else
    {
        if(pwndNewNotebookPage->IsKindOf(CLASSINFO(CBaseWindowView)))
        {
            CBaseWindowView* pPage = (CBaseWindowView*)pwndNewNotebookPage;
            m_pNotebook->AddPage(pPage, pPage->GetViewName(), true, -1);
        }
    }

    return true;
}


bool CMainFrame::CreateStatusbar() {

    if (m_pStatusbar)
        return true;

    int ch = GetCharWidth();

    const int widths[] = {-1, 20*ch, 15};

    m_pStatusbar = CreateStatusBar(WXSIZEOF(widths), wxST_SIZEGRIP, ID_STATUSBAR);
    m_pStatusbar->SetStatusWidths(WXSIZEOF(widths), widths);

    SetStatusBar(m_pStatusbar);
    SendSizeEvent();

    return true;
}


bool CMainFrame::DeleteMenu() {
    return true;
}


bool CMainFrame::DeleteNotebook() {
    return true;
}


bool CMainFrame::DeleteStatusbar() {

    if (!m_pStatusbar)
        return true;

    SetStatusBar (NULL);

    delete m_pStatusbar;

    m_pStatusbar = NULL;
    SendSizeEvent();

    return true;
}


void CMainFrame::OnAbout(wxCommandEvent &WXUNUSED(event)) {
}


void CMainFrame::OnClose(wxCloseEvent &event) {
    Destroy();
}


void CMainFrame::OnExit(wxCommandEvent &WXUNUSED(event)) {
    Close(true);
}


void CMainFrame::OnIdle (wxIdleEvent &event) {
}


void CMainFrame::OnStatusbar (wxCommandEvent &WXUNUSED(event)) {

    if (!m_pStatusbar)
        CreateStatusbar();
    else
        DeleteStatusbar();

}


void CMainFrame::OnStatusbarUI (wxUpdateUIEvent &event) {
    event.Check(m_pStatusbar != NULL);
}
