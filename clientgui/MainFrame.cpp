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
    EVT_MENU(ID_HIDE, CMainFrame::OnHide)
    EVT_MENU_RANGE(ID_ACTIVITYRUNALWAYS, ID_ACTIVITYSUSPEND, CMainFrame::OnActivitySelection)
    EVT_MENU_RANGE(ID_NETWORKRUNALWAYS, ID_NETWORKSUSPEND, CMainFrame::OnNetworkSelection)
    EVT_MENU(wxID_EXIT, CMainFrame::OnExit)
    EVT_MENU(ID_TOOLSOPTIONS, CMainFrame::OnToolsOptions)
    EVT_MENU(wxID_ABOUT, CMainFrame::OnAbout)
    EVT_UPDATE_UI_RANGE(ID_ACTIVITYRUNALWAYS, ID_ACTIVITYSUSPEND, CMainFrame::OnUpdateActivitySelection)
    EVT_UPDATE_UI_RANGE(ID_NETWORKRUNALWAYS, ID_NETWORKSUSPEND, CMainFrame::OnUpdateNetworkSelection)
    EVT_IDLE(CMainFrame::OnIdle)
    EVT_CLOSE(CMainFrame::OnClose)
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
        ID_HIDE, 
        _("&Hide"),
        _("Hides the main BOINC Manager window")
    );

    menuFile->AppendSeparator();

    menuFile->AppendRadioItem(
        ID_ACTIVITYRUNALWAYS,
        _("&Run always"),
        _("Runs BOINC without regards to the configured preferences for the computer")
    );
    menuFile->AppendRadioItem(
        ID_ACTIVITYRUNBASEDONPREPERENCES,
        _("Run based on &preferences"),
        _("Runs BOINC according to the preferences configured for the computer")
    );
    menuFile->AppendRadioItem(
        ID_ACTIVITYSUSPEND,
        _("&Suspend"),
        _("Suspends processing and network activity without regards to the configured preferences")
    );

    menuFile->AppendSeparator();

    menuFile->AppendCheckItem(
        ID_NETWORKSUSPEND,
        _("&Disable BOINC Network Access"),
        _("Disables network activity without suspending BOINC")
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
        ID_TOOLSOPTIONS, 
        _("&Options"),
        _("Configure GUI options and proxy settings")
    );

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(  
        wxID_ABOUT,
        _("&About BOINC Manager..."), 
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
    CBOINCBaseView* pView = NULL;
    wxString        strConfigLocation = wxEmptyString;
    wxString        strPreviousLocation = wxEmptyString;
    wxInt32         iIndex = 0;
    wxInt32         iPageCount = 0;


    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pNotebook);

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("CurrentPage"), m_pNotebook->GetSelection());
    pConfig->Write(wxT("Width"), GetSize().GetWidth());
    pConfig->Write(wxT("Height"), GetSize().GetHeight());


    //
    // Save Page(s) State
    //
 
    // Convert to a zero based index
    iPageCount = m_pNotebook->GetPageCount() - 1;

    for ( iIndex = 0; iIndex <= iPageCount; iIndex++ )
    {   
        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        strPreviousLocation = pConfig->GetPath();
        strConfigLocation = strPreviousLocation + pView->GetViewName();

        pConfig->SetPath(strConfigLocation);
        pView->FireOnSaveState( pConfig );
        pConfig->SetPath(strPreviousLocation);
    }

    return true;
}


bool CMainFrame::RestoreState()
{
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    CBOINCBaseView* pView = NULL;
    wxString        strConfigLocation = wxEmptyString;
    wxString        strPreviousLocation = wxEmptyString;
    wxInt32         iIndex = 0;
    wxInt32         iPageCount = 0;
    wxInt32         iHeight = 0;
    wxInt32         iWidth = 0;


    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pNotebook);


    //
    // Restore Frame State
    //
    wxInt32         iCurrentPage;


    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("CurrentPage"), &iCurrentPage, 1);
    m_pNotebook->SetSelection(iCurrentPage);

    pConfig->Read(wxT("Width"), &iWidth, 800);
    pConfig->Read(wxT("Height"), &iHeight, 600);
    SetSize( -1, -1, iWidth, iHeight );


    //
    // Restore Page(s) State
    //

    // Convert to a zero based index
    iPageCount = m_pNotebook->GetPageCount() - 1;

    for ( iIndex = 0; iIndex <= iPageCount; iIndex++ ) {   

        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        strPreviousLocation = pConfig->GetPath();
        strConfigLocation = strPreviousLocation + pView->GetViewName();

        pConfig->SetPath(strConfigLocation);
        pView->FireOnRestoreState( pConfig );
        pConfig->SetPath(strPreviousLocation);

    }

    return true;
}


void CMainFrame::OnHide( wxCommandEvent& event )
{
    Hide();
}


void CMainFrame::OnActivitySelection( wxCommandEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch( event.GetId() )
    {
        case ID_ACTIVITYRUNALWAYS:
            pDoc->SetActivityRunMode( CMainDocument::MODE_ALWAYS );
            break;
        case ID_ACTIVITYSUSPEND:
            pDoc->SetActivityRunMode( CMainDocument::MODE_NEVER );
            break;
        case ID_ACTIVITYRUNBASEDONPREPERENCES:
            pDoc->SetActivityRunMode( CMainDocument::MODE_AUTO );
            break;
    }
}


void CMainFrame::OnNetworkSelection( wxCommandEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch( event.GetId() )
    {
        case ID_NETWORKSUSPEND:
            if ( event.IsChecked() )
            {
                pDoc->SetNetworkRunMode( CMainDocument::MODE_ALWAYS );
            }
            else
            {
                pDoc->SetNetworkRunMode( CMainDocument::MODE_NEVER );
            }
            break;
        case ID_NETWORKRUNALWAYS:
        case ID_NETWORKRUNBASEDONPREPERENCES:
        default:
            pDoc->SetNetworkRunMode( CMainDocument::MODE_ALWAYS );
            break;
    }
}

   
void CMainFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    Close(true);
}


void CMainFrame::OnToolsOptions( wxCommandEvent& WXUNUSED(event) )
{
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CDlgOptions*   pDlg = new CDlgOptions(this);
    wxInt32        iAnswer = 0;
    bool           bProxyInformationConfigured = false;
    bool           bBuffer = false;
    wxInt32        iBuffer = 0;
    wxString       strBuffer = wxEmptyString;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pDlg);


    bProxyInformationConfigured = ( 0 == pDoc->GetProxyConfiguration() );
    if ( bProxyInformationConfigured )
    {
        pDlg->m_bProxySectionConfigured = true;
        if ( 0 == pDoc->GetProxyHTTPProxyEnabled( bBuffer ) )
            pDlg->m_EnableHTTPProxyCtrl->SetValue( bBuffer );
        if ( 0 == pDoc->GetProxyHTTPServerName( strBuffer ) ) 
            pDlg->m_HTTPAddressCtrl->SetValue( strBuffer );
        if ( 0 == pDoc->GetProxyHTTPServerPort( iBuffer ) ) 
        {
            strBuffer.Printf( wxT("%d"), iBuffer );
            pDlg->m_HTTPPortCtrl->SetValue( strBuffer );
        }
        if ( 0 == pDoc->GetProxyHTTPUserName( strBuffer ) ) 
            pDlg->m_HTTPUsernameCtrl->SetValue( strBuffer );
        if ( 0 == pDoc->GetProxyHTTPPassword( strBuffer ) ) 
            pDlg->m_HTTPPasswordCtrl->SetValue( strBuffer );

        if ( 0 == pDoc->GetProxySOCKSProxyEnabled( bBuffer ) )
            pDlg->m_EnableSOCKSProxyCtrl->SetValue( bBuffer );
        if ( 0 == pDoc->GetProxySOCKSServerName( strBuffer ) ) 
            pDlg->m_SOCKSAddressCtrl->SetValue( strBuffer );
        if ( 0 == pDoc->GetProxySOCKSServerPort( iBuffer ) ) 
        {
            strBuffer.Printf( wxT("%d"), iBuffer );
            pDlg->m_SOCKSPortCtrl->SetValue( strBuffer );
        }
        if ( 0 == pDoc->GetProxySOCKSUserName( strBuffer ) ) 
            pDlg->m_SOCKSUsernameCtrl->SetValue( strBuffer );
        if ( 0 == pDoc->GetProxySOCKSPassword( strBuffer ) ) 
            pDlg->m_SOCKSPasswordCtrl->SetValue( strBuffer );
    }

    iAnswer = pDlg->ShowModal();
    if ( wxID_OK == iAnswer )
    {
        bBuffer = pDlg->m_EnableHTTPProxyCtrl->GetValue();
        pDoc->SetProxyHTTPProxyEnabled( bBuffer );
        strBuffer = pDlg->m_HTTPAddressCtrl->GetValue();
        pDoc->SetProxyHTTPServerName( strBuffer );
        strBuffer = pDlg->m_HTTPPortCtrl->GetValue();
        strBuffer.ToLong( (long*)&iBuffer );
        pDoc->SetProxyHTTPServerPort( iBuffer );
        strBuffer = pDlg->m_HTTPUsernameCtrl->GetValue();
        pDoc->SetProxyHTTPUserName( strBuffer );
        strBuffer = pDlg->m_HTTPPasswordCtrl->GetValue();
        pDoc->SetProxyHTTPPassword( strBuffer );
        bBuffer = pDlg->m_EnableHTTPProxyCtrl->GetValue();

        pDoc->SetProxySOCKSProxyEnabled( bBuffer );
        strBuffer = pDlg->m_SOCKSAddressCtrl->GetValue();
        pDoc->SetProxySOCKSServerName( strBuffer );
        strBuffer = pDlg->m_SOCKSPortCtrl->GetValue();
        strBuffer.ToLong( (long*)&iBuffer );
        pDoc->SetProxySOCKSServerPort( iBuffer );
        strBuffer = pDlg->m_SOCKSUsernameCtrl->GetValue();
        pDoc->SetProxySOCKSUserName( strBuffer );
        strBuffer = pDlg->m_SOCKSPasswordCtrl->GetValue();
        pDoc->SetProxySOCKSPassword( strBuffer );

        pDoc->SetProxyConfiguration();
    }

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


void CMainFrame::OnUpdateActivitySelection( wxUpdateUIEvent& WXUNUSED(event) )
{
    CMainDocument* pDoc          = wxGetApp().GetDocument();
    wxMenuBar*     pMenuBar      = GetMenuBar();
    wxInt32        iActivityMode = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pMenuBar);
    wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

    pDoc->GetActivityRunMode( iActivityMode );
    switch( iActivityMode )
    {
        case CMainDocument::MODE_ALWAYS:
            pMenuBar->Check( ID_ACTIVITYRUNALWAYS, true );
            break;
        case CMainDocument::MODE_NEVER:
            pMenuBar->Check( ID_ACTIVITYSUSPEND, true );
            break;
        case CMainDocument::MODE_AUTO:
            pMenuBar->Check( ID_ACTIVITYRUNBASEDONPREPERENCES, true );
            break;
    }
}


void CMainFrame::OnUpdateNetworkSelection( wxUpdateUIEvent& WXUNUSED(event) )
{
    CMainDocument* pDoc          = wxGetApp().GetDocument();
    wxMenuBar*     pMenuBar      = GetMenuBar();
    wxInt32        iNetworkMode  = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pMenuBar);
    wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

    pDoc->GetNetworkRunMode( iNetworkMode );
    switch( iNetworkMode )
    {
        case CMainDocument::MODE_NEVER:
            pMenuBar->Check( ID_NETWORKSUSPEND, true );
            break;
        default:
            pMenuBar->Check( ID_NETWORKSUSPEND, false );
            break;
    }
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


void CMainFrame::OnNotebookSelectionChanged( wxNotebookEvent& event )
{
    if ( (-1 != event.GetSelection()) && IsShown() )
    {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;
        wxTimerEvent    timerEvent;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage( event.GetSelection() );
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnTaskRender( timerEvent );
        pView->FireOnListRender( timerEvent );
    }

    event.Skip();
}


void CMainFrame::OnListCacheHint( wxListEvent& event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage( event.GetId() - ID_LIST_BASE );
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnListCacheHint( event );
    }

    event.Skip();
}


void CMainFrame::OnListSelected( wxListEvent& event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage( event.GetId() - ID_LIST_BASE );
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnListSelected( event );
    }

    event.Skip();
}


void CMainFrame::OnListDeselected( wxListEvent& event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage( event.GetId() - ID_LIST_BASE );
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnListDeselected( event );
    }

    event.Skip();
}


void CMainFrame::OnListPanelRender ( wxTimerEvent &event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage( m_pNotebook->GetSelection() );
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnListRender( event );
    }

    event.Skip();
}


void CMainFrame::OnTaskPanelRender ( wxTimerEvent &event )
{
    if ( IsShown() )
    {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        wxASSERT(NULL != m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage( m_pNotebook->GetSelection() );
        wxASSERT(NULL != pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(NULL != pView);

        pView->FireOnTaskRender( event );
    }

    event.Skip();
}

