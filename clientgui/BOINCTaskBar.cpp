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
#pragma implementation "BOINCTaskBar.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCTaskBar.h"
#include "DlgAbout.h"
#include "Events.h"

#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CTaskBarIcon, wxTaskBarIconEx)

BEGIN_EVENT_TABLE (CTaskBarIcon, wxTaskBarIconEx)
    EVT_MENU(wxID_OPEN, CTaskBarIcon::OnOpen)
    EVT_MENU_RANGE(ID_TB_ACTIVITYRUNALWAYS, ID_TB_ACTIVITYSUSPEND, CTaskBarIcon::OnActivitySelection)
    EVT_MENU_RANGE(ID_TB_NETWORKRUNALWAYS, ID_TB_NETWORKSUSPEND, CTaskBarIcon::OnNetworkSelection)
    EVT_MENU(wxID_ABOUT, CTaskBarIcon::OnAbout)
    EVT_MENU(wxID_EXIT, CTaskBarIcon::OnExit)
    EVT_CLOSE(CTaskBarIcon::OnClose)
    EVT_TASKBAR_MOVE(CTaskBarIcon::OnMouseMove)
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLButtonDClick)

#ifdef __WXMSW__
    EVT_TASKBAR_CONTEXT_MENU(CTaskBarIcon::OnContextMenu)
#else
    EVT_TASKBAR_RIGHT_DOWN(CTaskBarIcon::OnRButtonDown)
#endif
END_EVENT_TABLE ()


CTaskBarIcon::CTaskBarIcon() : 
    wxTaskBarIconEx()
{
    m_iconTaskBarIcon = wxIcon( boinc_xpm );
    m_dtLastHoverDetected = wxDateTime( (time_t)0 );
    m_dtLastBalloonDisplayed = wxDateTime( (time_t)0 );

    SetIcon( m_iconTaskBarIcon, wxEmptyString );
}


CTaskBarIcon::~CTaskBarIcon()
{
    RemoveIcon();
}


void CTaskBarIcon::OnOpen( wxCommandEvent& WXUNUSED(event) )
{
    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    if ( NULL != pFrame )
        pFrame->Show();
}


void CTaskBarIcon::OnActivitySelection( wxCommandEvent& event )
{
    ResetTaskBar();

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch( event.GetId() )
    {
        case ID_TB_ACTIVITYRUNALWAYS:
            pDoc->SetActivityRunMode( CMainDocument::MODE_ALWAYS );
            break;
        case ID_TB_ACTIVITYSUSPEND:
            pDoc->SetActivityRunMode( CMainDocument::MODE_NEVER );
            break;
        case ID_TB_ACTIVITYRUNBASEDONPREPERENCES:
            pDoc->SetActivityRunMode( CMainDocument::MODE_AUTO );
            break;
    }
}


void CTaskBarIcon::OnNetworkSelection( wxCommandEvent& event )
{
    ResetTaskBar();

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch( event.GetId() )
    {
        case ID_TB_NETWORKSUSPEND:
            if ( event.IsChecked() )
                pDoc->SetNetworkRunMode( CMainDocument::MODE_ALWAYS );
            else
                pDoc->SetNetworkRunMode( CMainDocument::MODE_NEVER );
            break;
        case ID_TB_NETWORKRUNALWAYS:
        case ID_TB_NETWORKRUNBASEDONPREPERENCES:
        default:
            pDoc->SetNetworkRunMode( CMainDocument::MODE_ALWAYS );
            break;
    }
}


void CTaskBarIcon::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
    ResetTaskBar();

    CDlgAbout* pDlg = new CDlgAbout(NULL);
    wxASSERT(NULL != pDlg);

    pDlg->ShowModal();

    if (pDlg)
        pDlg->Destroy();
}


void CTaskBarIcon::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    if ( NULL != pFrame )
        pFrame->Close(true);
}


void CTaskBarIcon::OnClose( wxCloseEvent& event )
{
    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    if ( NULL != pFrame )
        pFrame->Close(true);

    event.Skip();
}


void CTaskBarIcon::OnMouseMove( wxTaskBarIconEvent& event )
{

    wxTimeSpan ts(wxDateTime::Now() - m_dtLastHoverDetected);
    if ( ts.GetSeconds() >= 10 )
        m_dtLastHoverDetected = wxDateTime::Now();

    wxTimeSpan tsLastHover(wxDateTime::Now() - m_dtLastHoverDetected);
    wxTimeSpan tsLastBalloon(wxDateTime::Now() - m_dtLastBalloonDisplayed);
    if ( (tsLastHover.GetSeconds() >= 2) && (tsLastBalloon.GetSeconds() >= 10) )
    {
        m_dtLastBalloonDisplayed = wxDateTime::Now();

        wxString strTitle        = wxGetApp().GetAppName();
        wxString strMachineName  = wxEmptyString;
        wxString strMessage      = wxEmptyString;
        wxString strBuffer       = wxEmptyString;
        wxString strProjectName  = wxEmptyString;
        float    fProgress       = 0;
        bool     bIsActive       = false;
        bool     bIsExecuting    = false;
        bool     bIsDownloaded   = false;
        wxInt32  iResultCount    = 0;
        wxInt32  iIndex          = 0;
        CMainDocument* pDoc      = wxGetApp().GetDocument();

        wxASSERT(NULL != pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));


        pDoc->GetConnectedComputerName( strMachineName );
        if ( strMachineName.empty() )
            strTitle = strTitle + wxT(" - (localhost)");
        else
            strTitle = strTitle + wxT(" - (") + strMachineName + wxT(")");


        iResultCount = pDoc->GetWorkCount();
        for ( iIndex = 0; iIndex < iResultCount; iIndex++ )
        {
            bIsDownloaded = ( CMainDocument::RESULT_FILES_DOWNLOADED == pDoc->GetWorkState( iIndex ) );
            bIsActive     = ( pDoc->IsWorkActive( iIndex ) );
            bIsExecuting  = ( CMainDocument::CPU_SCHED_SCHEDULED == pDoc->GetWorkSchedulerState( iIndex ) );
            if ( !( bIsActive ) || !( bIsDownloaded ) || !( bIsExecuting ) ) continue;

            pDoc->GetWorkProjectName( iIndex, strProjectName );
            pDoc->GetWorkFractionDone( iIndex, fProgress );

            strBuffer.Printf(wxT( "%s: %.2f%%\n"), strProjectName.c_str(), fProgress * 100 );
            strMessage += strBuffer;
        }

        SetBalloon( m_iconTaskBarIcon, strTitle, strMessage );
    }
}


void CTaskBarIcon::OnLButtonDClick( wxTaskBarIconEvent& event )
{
    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    if ( NULL != pFrame )
        pFrame->Show();
}


#ifdef __WXMSW__


void CTaskBarIcon::OnContextMenu( wxTaskBarIconExEvent& event )
{
    CreateContextMenu();
}


#else


void CTaskBarIcon::OnRButtonDown( wxTaskBarIconEvent& event )
{
    CreateContextMenu();
}


#endif


void CTaskBarIcon::ResetTaskBar()
{
#ifdef __WXMSW___
    SetBalloon( m_iconTaskBarIcon, wxT(""), wxT("") );
#else
    SetIcon( m_iconTaskBarIcon, wxT("") );
#endif

    m_dtLastBalloonDisplayed = wxDateTime::Now();
}


void CTaskBarIcon::CreateContextMenu()
{
    ResetTaskBar();

    CMainDocument* pDoc          = wxGetApp().GetDocument();
    wxMenu*        menu          = new wxMenu;
    wxMenuItem*    menuItem      = NULL;
    wxInt32        iActivityMode = -1;
    wxInt32        iNetworkMode  = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != menu);

#ifdef __WXMSW__

    wxFont font = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
    font.SetWeight( wxBOLD );

    menuItem = new wxMenuItem( menu, wxID_OPEN, _("&Open BOINC Manager..."), wxEmptyString );
    menuItem->SetFont( font );

    menu->Append( menuItem );

#else

    menu->Append( wxID_OPEN, _("&Open BOINC Manager..."), wxEmptyString );

#endif
    menu->AppendSeparator();
    menu->AppendRadioItem( ID_TB_ACTIVITYRUNALWAYS, _("&Run always"), wxEmptyString );
    menu->AppendRadioItem( ID_TB_ACTIVITYRUNBASEDONPREPERENCES, _("Run based on &preferences"), wxEmptyString );
    menu->AppendRadioItem( ID_TB_ACTIVITYSUSPEND, _("&Suspend"), wxEmptyString );
    menu->AppendSeparator();
    menu->AppendCheckItem( ID_TB_NETWORKSUSPEND, _("&Disable BOINC Network Access"), wxEmptyString );
    menu->AppendSeparator();
    menu->Append( wxID_ABOUT, _("&About BOINC Manager..."), wxEmptyString );
    menu->AppendSeparator();
    menu->Append( wxID_EXIT, _("E&xit"), wxEmptyString );

    pDoc->GetActivityRunMode( iActivityMode );
    switch( iActivityMode )
    {
        case CMainDocument::MODE_ALWAYS:
            menu->Check( ID_TB_ACTIVITYRUNALWAYS, true );
            break;
        case CMainDocument::MODE_NEVER:
            menu->Check( ID_TB_ACTIVITYSUSPEND, true );
            break;
        case CMainDocument::MODE_AUTO:
            menu->Check( ID_TB_ACTIVITYRUNBASEDONPREPERENCES, true );
            break;
    }

    pDoc->GetNetworkRunMode( iNetworkMode );
    if ( CMainDocument::MODE_NEVER == iNetworkMode )
        menu->Check( ID_TB_NETWORKSUSPEND, true );
    else
        menu->Check( ID_TB_NETWORKSUSPEND, false );

    PopupMenu( menu );

    delete menu;
}

