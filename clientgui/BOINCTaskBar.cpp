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

#ifdef __WXMSW__
    EVT_TASKBAR_SHUTDOWN(CTaskBarIcon::OnShutdown)
#endif

    EVT_IDLE(CTaskBarIcon::OnIdle)
    EVT_CLOSE(CTaskBarIcon::OnClose)
#ifdef __WXMSW__
    EVT_TASKBAR_MOVE(CTaskBarIcon::OnMouseMove)
#endif
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLButtonDClick)

#ifdef __WXMSW__
    EVT_TASKBAR_CONTEXT_MENU(CTaskBarIcon::OnContextMenu)
    
    EVT_TASKBAR_RIGHT_DOWN(CTaskBarIcon::OnRButtonDown)
    EVT_TASKBAR_RIGHT_UP(CTaskBarIcon::OnRButtonUp)
#endif

END_EVENT_TABLE ()


CTaskBarIcon::CTaskBarIcon() : 
#ifdef __WXMAC__
    wxTaskBarIcon( DOCK )
#else
    wxTaskBarIconEx( wxT("BOINCManagerSystray") )
#endif
{
    m_iconTaskBarIcon = wxIcon( boinc_xpm );
    m_dtLastHoverDetected = wxDateTime( (time_t)0 );
    m_dtLastBalloonDisplayed = wxDateTime( (time_t)0 );

#ifndef __WXMAC__
    SetIcon( m_iconTaskBarIcon, wxEmptyString );
#endif
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
	{
        pFrame->Show();
        pFrame->SendSizeEvent();
	}
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
                pDoc->SetNetworkRunMode( CMainDocument::MODE_NEVER );
            else
                pDoc->SetNetworkRunMode( CMainDocument::MODE_ALWAYS );
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


void CTaskBarIcon::OnExit( wxCommandEvent& event )
{
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function Begin"));

    wxCloseEvent eventClose;

    OnClose( eventClose );

    if ( eventClose.GetSkipped() ) event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function End"));
}


#ifdef __WXMSW__


void CTaskBarIcon::OnShutdown( wxTaskBarIconExEvent& event )
{
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnShutdown - Function Begin"));

    wxCloseEvent eventClose;

    OnClose( eventClose );

    if ( eventClose.GetSkipped() ) event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnShutdown - Function End"));
}


#endif


void CTaskBarIcon::OnIdle( wxIdleEvent& event )
{
    wxGetApp().UpdateSystemIdleDetection();
    event.Skip();
}


void CTaskBarIcon::OnClose( wxCloseEvent& event )
{
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function Begin"));

    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    if ( NULL != pFrame )
    {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        pFrame->Close(true);
    }

    event.Skip();
    
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function End"));
}


#ifdef __WXMSW__

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
            bIsDownloaded = ( CMainDocument::FILES_DOWNLOADED == pDoc->GetWorkState( iIndex ) );
            bIsActive     = ( pDoc->IsWorkActive( iIndex ) );
            bIsExecuting  = ( CMainDocument::SCHED_SCHEDULED == pDoc->GetWorkSchedulerState( iIndex ) );
            if ( !( bIsActive ) || !( bIsDownloaded ) || !( bIsExecuting ) ) continue;

            pDoc->GetWorkProjectName( iIndex, strProjectName );
            pDoc->GetWorkFractionDone( iIndex, fProgress );

            strBuffer.Printf(wxT( "%s: %.2f%%\n"), strProjectName.c_str(), fProgress * 100 );
            strMessage += strBuffer;
        }

        SetBalloon( m_iconTaskBarIcon, strTitle, strMessage );
    }
}

#endif // __WXMSW__


void CTaskBarIcon::OnLButtonDClick( wxTaskBarIconEvent& event )
{
    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    if ( NULL != pFrame )
	{
        pFrame->Show();
        pFrame->SendSizeEvent();
	}
}


#ifdef __WXMSW__
void CTaskBarIcon::OnContextMenu( wxTaskBarIconExEvent& event )
{
    CreateContextMenu();
}
#else
void CTaskBarIcon::OnContextMenu( wxTaskBarIconEvent& event )
{
    CreateContextMenu();
}
#endif


void CTaskBarIcon::OnRButtonDown( wxTaskBarIconEvent& event )
{
    if (!IsBalloonsSupported())
    {
        m_bButtonPressed = true;
    }
}


void CTaskBarIcon::OnRButtonUp( wxTaskBarIconEvent& event )
{
    if (!IsBalloonsSupported())
    {
        if (m_bButtonPressed)
        {
            CreateContextMenu();
            m_bButtonPressed = false;
        }
    }
}


void CTaskBarIcon::ResetTaskBar()
{
#ifdef __WXMSW___
    SetBalloon( m_iconTaskBarIcon, wxT(""), wxT("") );
#else
#ifndef __WXMAC__
    SetIcon( m_iconTaskBarIcon, wxT("") );
#endif
#endif

    m_dtLastBalloonDisplayed = wxDateTime::Now();
}


#ifdef __WXMAC__

// The mac version of WxWidgets will delete this menu when 
//  done with it; we must not delete it.  See the comments
//  in wxTaskBarIcon::PopupMenu() and DoCreatePopupMenu() 
//  in WxMac/src/mac/carbon/taskbar.cpp for details

// Overridables
wxMenu *CTaskBarIcon::CreatePopupMenu()
{
    wxMenu *menu = BuildContextMenu();
    return menu;
}

#endif

void CTaskBarIcon::CreateContextMenu()
{
    ResetTaskBar();

    wxMenu *menu = BuildContextMenu();

    // These should be in Windows Task Bar Menu but not in Mac's Dock menu
    menu->AppendSeparator();
    menu->Append( wxID_EXIT, _("E&xit"), wxEmptyString );

    PopupMenu( menu );

    delete menu;
}


wxMenu *CTaskBarIcon::BuildContextMenu()
{
    wxMenu*        menu          = new wxMenu;
    wxASSERT(NULL != menu);

#ifdef __WXMSW__

    wxMenuItem*    menuItem      = NULL;

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
    menu->AppendCheckItem( ID_TB_NETWORKSUSPEND, _("&Disable BOINC network access"), wxEmptyString );
    menu->AppendSeparator();
    menu->Append( wxID_ABOUT, _("&About BOINC Manager..."), wxEmptyString );

    AdjustMenuItems(menu);
    
    return menu;
}

void CTaskBarIcon::AdjustMenuItems(wxMenu* menu)
{
    CMainDocument* pDoc          = wxGetApp().GetDocument();
    wxInt32        iActivityMode = -1;
    wxInt32        iNetworkMode  = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

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
}

const char *BOINC_RCSID_531575eeaa = "$Id$";
