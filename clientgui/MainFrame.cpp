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
#include "DlgAbout.h"
#include "DlgOptions.h"

#include "res/BOINCGUIApp.xpm"


IMPLEMENT_DYNAMIC_CLASS(CMainFrame, wxFrame)

BEGIN_EVENT_TABLE (CMainFrame, wxFrame)
    EVT_CLOSE(CMainFrame::OnClose)
    EVT_MENU(wxID_EXIT, CMainFrame::OnExit)
    EVT_MENU(ID_TOOLSOPTIONS, CMainFrame::OnToolsOptions)
    EVT_MENU(wxID_ABOUT, CMainFrame::OnAbout)
    EVT_IDLE(CMainFrame::OnIdle)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_FRAMENOTEBOOK, CMainFrame::OnNotebookSelectionChanged)
    EVT_LIST_CACHE_HINT(wxID_ANY, CMainFrame::OnListCacheHint)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, CMainFrame::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, CMainFrame::OnListDeselected)
    EVT_TIMER(ID_FRAMELISTRENDERTIMER, CMainFrame::OnListPanelRender)
    EVT_TIMER(ID_FRAMETASKRENDERTIMER, CMainFrame::OnTaskPanelRender)
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


    m_pFrameTaskPanelRenderTimer = new wxTimer(this, ID_FRAMETASKRENDERTIMER);
    wxASSERT(NULL != m_pFrameTaskPanelRenderTimer);

    m_pFrameListPanelRenderTimer = new wxTimer(this, ID_FRAMELISTRENDERTIMER);
    wxASSERT(NULL != m_pFrameListPanelRenderTimer);

    m_pFrameTaskPanelRenderTimer->Start(1000);       // Send event every 1 second
    m_pFrameListPanelRenderTimer->Start(5000);       // Send event every 5 seconds

    SetStatusBarPane(0);

    RestoreState();
}


CMainFrame::~CMainFrame()
{
    wxASSERT(NULL != m_pFrameTaskPanelRenderTimer);
    wxASSERT(NULL != m_pFrameListPanelRenderTimer);
    wxASSERT(NULL != m_pMenubar);
    wxASSERT(NULL != m_pNotebook);
    wxASSERT(NULL != m_pStatusbar);


    SaveState();


    if (m_pFrameTaskPanelRenderTimer) {
        m_pFrameTaskPanelRenderTimer->Stop();
        delete m_pFrameTaskPanelRenderTimer;
    }

    if (m_pFrameListPanelRenderTimer) {
        m_pFrameListPanelRenderTimer->Stop();
        delete m_pFrameListPanelRenderTimer;
    }

    if (m_pStatusbar)
        wxCHECK_RET(DeleteStatusbar(), _T("Failed to delete status bar."));

    if (m_pNotebook)
        wxCHECK_RET(DeleteNotebook(), _T("Failed to delete notebook."));

    if (m_pMenubar)
        wxCHECK_RET(DeleteMenu(), _T("Failed to delete menu bar."));
}


bool CMainFrame::CreateMenu()
{
    // File menu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(
        wxID_EXIT,
        _("E&xit"),
        _("Exit the BOINC Manager")
    );

    // Tools menu
    wxMenu *menuTools = new wxMenu;
    menuTools->Append( 
        ID_TOOLSOPTIONS, 
        _("&Options"),
        _("Configure GUI options and proxy settings")
    );

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(  
        wxID_ABOUT,
        _("&About BOINC..."), 
        _("Displays general information about BOINC and BOINC Manager")
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

    return true;
}


bool CMainFrame::CreateNotebook()
{
    // create frame panel
    wxPanel *pPanel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize,
                                 wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxNO_BORDER);

    // initialize notebook
    m_pNotebook = new wxNotebook(pPanel, ID_FRAMENOTEBOOK, wxDefaultPosition, wxDefaultSize,
                                wxNB_FIXEDWIDTH|wxCLIP_CHILDREN);

    wxNotebookSizer *pNotebookSizer = new wxNotebookSizer(m_pNotebook);

    // layout frame panel
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    pPanelSizer->Add(new wxStaticLine(pPanel, -1), 0, wxEXPAND);
    pPanelSizer->Add(0, 4);
    pPanelSizer->Add(pNotebookSizer, 1, wxEXPAND);

    pPanel->SetAutoLayout(true);
    pPanel->SetSizerAndFit(pPanelSizer);

    CreateNotebookPage( new CViewProjects( m_pNotebook ) );
    CreateNotebookPage( new CViewWork( m_pNotebook ) );
    CreateNotebookPage( new CViewTransfers( m_pNotebook ) );
    CreateNotebookPage( new CViewMessages( m_pNotebook ) );
    CreateNotebookPage( new CViewResources( m_pNotebook ) );

    return true;
}


template < class T >
bool CMainFrame::CreateNotebookPage(T pwndNewNotebookPage)
{
    wxImageList*    pImageList;
    wxInt32         iImageIndex = 0;

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

    return true;
}


bool CMainFrame::CreateStatusbar()
{
    if (m_pStatusbar)
        return true;

    wxInt32 ch = GetCharWidth();

    const wxInt32 widths[] = {-1, 20*ch, 15};

    m_pStatusbar = CreateStatusBar(WXSIZEOF(widths), wxST_SIZEGRIP, ID_STATUSBAR);
    wxASSERT(NULL != m_pStatusbar);

    m_pStatusbar->SetStatusWidths(WXSIZEOF(widths), widths);

    SetStatusBar(m_pStatusbar);
    SendSizeEvent();

    return true;
}


bool CMainFrame::DeleteMenu()
{
    return true;
}


bool CMainFrame::DeleteNotebook()
{
    wxImageList*    pImageList;

    wxASSERT(NULL != m_pNotebook);

    pImageList = m_pNotebook->GetImageList();

    wxASSERT(NULL != pImageList);

    if (pImageList)
        delete pImageList;

    return true;
}


bool CMainFrame::DeleteStatusbar()
{
    if (!m_pStatusbar)
        return true;

    SetStatusBar(NULL);

    delete m_pStatusbar;

    m_pStatusbar = NULL;
    SendSizeEvent();

    return true;
}


bool CMainFrame::UpdateStatusbar( const wxString& strStatusbarText )
{
    if (!m_pStatusbar)
        return true;

    if ( NULL != m_pStatusbar )
    {
        if ( m_pStatusbar->GetStatusText(0) != strStatusbarText )
        {
            SetStatusText(strStatusbarText, 0);
        }
    }

    ::wxSafeYield( NULL, true );

    return true;
}


bool CMainFrame::SaveState()
{
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    wxString        strConfigLocation = wxString(wxT(""));
    wxInt32         iIndex = 0;
    wxInt32         iPageCount = 0;


    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pNotebook);


    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("CurrentPage"), m_pNotebook->GetSelection());


    //
    // Save Page(s) State
    //
 
    // Convert to a zero based index
    iPageCount = m_pNotebook->GetPageCount() - 1;

    for ( iIndex = 0; iIndex <= iPageCount; iIndex++ ) {   

        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CViewWork), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CViewResources), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireSaveStateEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), pConfig);
        }
    }

    return true;
}


template < class T >
bool CMainFrame::FireSaveStateEvent( T pPage, wxConfigBase* pConfig )
{
    wxString strPreviousLocation = wxString(wxT(""));
    wxString strConfigLocation = wxString(wxT(""));

    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + pPage->GetViewName();

    pConfig->SetPath(strConfigLocation);
    pPage->OnSaveState( pConfig );
    pConfig->SetPath(strPreviousLocation);

    return true;
}


bool CMainFrame::RestoreState()
{
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    wxString        strConfigLocation = wxString(wxT(""));
    wxInt32         iIndex = 0;
    wxInt32         iPageCount = 0;


    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pNotebook);


    //
    // Restore Frame State
    //
    wxInt32         iCurrentPage;


    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("CurrentPage"), &iCurrentPage, 1);
    m_pNotebook->SetSelection(iCurrentPage);


    //
    // Restore Page(s) State
    //

    // Convert to a zero based index
    iPageCount = m_pNotebook->GetPageCount() - 1;

    for ( iIndex = 0; iIndex <= iPageCount; iIndex++ ) {   

        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CViewWork), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CViewResources), pConfig);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireRestoreStateEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), pConfig);
        }
    }

    return true;
}


template < class T >
bool CMainFrame::FireRestoreStateEvent( T pPage, wxConfigBase* pConfig ) 
{
    wxString strPreviousLocation = wxString(wxT(""));
    wxString strConfigLocation = wxString(wxT(""));

    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + pPage->GetViewName();

    pConfig->SetPath(strConfigLocation);
    pPage->OnRestoreState( pConfig );
    pConfig->SetPath(strPreviousLocation);

    return true;
}


void CMainFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    Close(true);
}


void CMainFrame::OnClose( wxCloseEvent& event )
{
    if ( !event.CanVeto() )
        Destroy();
    else
    {
        Hide();
        event.Veto();
    }
}


void CMainFrame::OnToolsOptions( wxCommandEvent& WXUNUSED(event) )
{
    CDlgOptions* pDlg = new CDlgOptions(this);
    wxASSERT(NULL != pDlg);

    pDlg->ShowModal();

    if (pDlg)
        pDlg->Destroy();
}


void CMainFrame::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
    CDlgAbout* pDlg = new CDlgAbout(this);
    wxASSERT(NULL != pDlg);

    pDlg->ShowModal();

    if (pDlg)
        pDlg->Destroy();
}


void CMainFrame::OnIdle( wxIdleEvent& event )
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if ( NULL != pDoc )
    {
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));

        pDoc->OnIdle();
    }

    event.Skip();
}


void CMainFrame::OnNotebookSelectionChanged( wxNotebookEvent& event )
{
    if ( (-1 != event.GetSelection()) && IsShown() )
    {
        wxWindow*       pwndNotebookPage;
        wxTimerEvent    timerEvent;

        wxASSERT(NULL != m_pNotebook);


        pwndNotebookPage = m_pNotebook->GetPage( event.GetSelection() );
        wxASSERT(NULL != pwndNotebookPage);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), timerEvent);
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), timerEvent);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewWork), timerEvent);
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewWork), timerEvent);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), timerEvent);
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), timerEvent);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), timerEvent);
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), timerEvent);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewResources), timerEvent);
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewResources), timerEvent);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), timerEvent);
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), timerEvent);
        }
    }

    event.Skip();
}


void CMainFrame::OnListCacheHint( wxListEvent& event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage;

        wxASSERT(NULL != m_pNotebook);


        pwndNotebookPage = m_pNotebook->GetPage( event.GetId() - ID_LIST_BASE );
        wxASSERT(NULL != pwndNotebookPage);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireListOnCacheHintEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireListOnCacheHintEvent(wxDynamicCast(pwndNotebookPage, CViewWork), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireListOnCacheHintEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireListOnCacheHintEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireListOnCacheHintEvent(wxDynamicCast(pwndNotebookPage, CViewResources), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireListOnCacheHintEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), event);
        }
    }

    event.Skip();
}


void CMainFrame::OnListSelected( wxListEvent& event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage;

        wxASSERT(NULL != m_pNotebook);


        pwndNotebookPage = m_pNotebook->GetPage( event.GetId() - ID_LIST_BASE );
        wxASSERT(NULL != pwndNotebookPage);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireListOnSelectedEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireListOnSelectedEvent(wxDynamicCast(pwndNotebookPage, CViewWork), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireListOnSelectedEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireListOnSelectedEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireListOnSelectedEvent(wxDynamicCast(pwndNotebookPage, CViewResources), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireListOnSelectedEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), event);
        }
    }

    event.Skip();
}


void CMainFrame::OnListDeselected( wxListEvent& event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage;

        wxASSERT(NULL != m_pNotebook);


        pwndNotebookPage = m_pNotebook->GetPage( event.GetId() - ID_LIST_BASE );
        wxASSERT(NULL != pwndNotebookPage);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireListOnDeselectedEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireListOnDeselectedEvent(wxDynamicCast(pwndNotebookPage, CViewWork), event);
        } 
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireListOnDeselectedEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireListOnDeselectedEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireListOnDeselectedEvent(wxDynamicCast(pwndNotebookPage, CViewResources), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireListOnDeselectedEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), event);
        }
    }

    event.Skip();
}


void CMainFrame::OnListPanelRender ( wxTimerEvent &event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage;

        wxASSERT(NULL != m_pNotebook);


        pwndNotebookPage = m_pNotebook->GetPage( m_pNotebook->GetSelection() );
        wxASSERT(NULL != pwndNotebookPage);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewWork), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewResources), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireListPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), event);
        }
    }

    event.Skip();
}


void CMainFrame::OnTaskPanelRender ( wxTimerEvent &event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage;

        wxASSERT(NULL != m_pNotebook);


        pwndNotebookPage = m_pNotebook->GetPage( m_pNotebook->GetSelection() );
        wxASSERT(NULL != pwndNotebookPage);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        if      (wxDynamicCast(pwndNotebookPage, CViewProjects))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewProjects), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewWork))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewWork), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewTransfers))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewTransfers), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewMessages))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewMessages), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CViewResources))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CViewResources), event);
        }
        else if (wxDynamicCast(pwndNotebookPage, CBOINCBaseView))
        {
            FireTaskPanelRenderEvent(wxDynamicCast(pwndNotebookPage, CBOINCBaseView), event);
        }
    }

    event.Skip();
}


template < class T >
void CMainFrame::FireListOnCacheHintEvent( T pPage, wxListEvent& event )
{
    pPage->OnListCacheHint( event );
}


template < class T >
void CMainFrame::FireListOnSelectedEvent( T pPage, wxListEvent& event )
{
    pPage->OnListSelected( event );
}


template < class T >
void CMainFrame::FireListOnDeselectedEvent( T pPage, wxListEvent& event )
{
    pPage->OnListDeselected( event );
}


template < class T >
void CMainFrame::FireListPanelRenderEvent( T pPage, wxTimerEvent& event )
{
    pPage->OnListRender( event );
}


template < class T >
void CMainFrame::FireTaskPanelRenderEvent( T pPage, wxTimerEvent& event )
{
    pPage->OnTaskRender( event );
}

