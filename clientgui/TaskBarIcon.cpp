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
#pragma implementation "TaskBarIcon.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "TaskBarIcon.h"
#include "DlgAbout.h"
#include "Events.h"

#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CTaskBarIcon, wxTaskBarIcon)

BEGIN_EVENT_TABLE (CTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(wxID_OPEN, CTaskBarIcon::OnOpen)
    EVT_MENU_RANGE(ID_TB_ACTIVITYRUNALWAYS, ID_TB_ACTIVITYSUSPEND, CTaskBarIcon::OnActivitySelection)
    EVT_MENU_RANGE(ID_TB_NETWORKRUNALWAYS, ID_TB_NETWORKSUSPEND, CTaskBarIcon::OnNetworkSelection)
    EVT_MENU(wxID_ABOUT, CTaskBarIcon::OnAbout)
    EVT_MENU(wxID_EXIT, CTaskBarIcon::OnExit)
    EVT_CLOSE(CTaskBarIcon::OnClose)
END_EVENT_TABLE ()


CTaskBarIcon::CTaskBarIcon() : 
    wxTaskBarIcon()
{
    iconTaskBarIcon = wxIcon( boinc_xpm );
    dtLastMouseCaptureTime = wxDateTime( (time_t)0 );

    SetIcon( iconTaskBarIcon, wxEmptyString );
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
            {
                pDoc->SetNetworkRunMode( CMainDocument::MODE_ALWAYS );
            }
            else
            {
                pDoc->SetNetworkRunMode( CMainDocument::MODE_NEVER );
            }
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
}


void CTaskBarIcon::OnMouseMove( wxEvent& event )
{
   wxTimeSpan ts(wxDateTime::Now() - dtLastMouseCaptureTime);

    if ( ts.GetSeconds() >= 5 )
    {
        dtLastMouseCaptureTime = wxDateTime::Now();

        wxString strTitle        = wxGetApp().GetAppName();
        wxString strMessage      = wxEmptyString;
        wxString strBuffer       = wxEmptyString;
        wxString strProjectName  = wxEmptyString;
        wxString strResultName   = wxEmptyString;
        float    fProgress       = 0;
        bool     bIsActive       = false;
        bool     bIsExecuting    = false;
        bool     bIsDownloaded   = false;
        wxInt32  iResultCount    = 0;
        wxInt32  iIndex          = 0;
        CMainDocument* pDoc      = wxGetApp().GetDocument();

        wxASSERT(NULL != pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));

        iResultCount = pDoc->GetWorkCount();

        for ( iIndex = 0; iIndex < iResultCount; iIndex++ )
        {
            bIsDownloaded = ( CMainDocument::RESULT_FILES_DOWNLOADED == pDoc->GetWorkState( iIndex ) );
            bIsActive     = ( pDoc->IsWorkActive( iIndex ) );
            bIsExecuting  = ( CMainDocument::CPU_SCHED_SCHEDULED == pDoc->GetWorkSchedulerState( iIndex ) );
            if ( !( bIsActive ) || !( bIsDownloaded ) || !( bIsExecuting ) ) continue;

            pDoc->GetWorkProjectName( iIndex, strProjectName );
            pDoc->GetWorkName( iIndex, strResultName );
            pDoc->GetWorkFractionDone( iIndex, fProgress );

            strBuffer.Printf(wxT( "%s: %s: %.2f%%\n"), strProjectName.c_str(), strResultName.c_str(), fProgress * 100 );
            strMessage += strBuffer;
        }

        ShowBalloon( strTitle, strMessage );
    }
}


void CTaskBarIcon::OnRButtonDown( wxEvent& event )
{
    ResetTaskBar();

    CMainDocument* pDoc          = wxGetApp().GetDocument();
    wxMenu*        menu          = new wxMenu;
    wxInt32        iActivityMode = -1;
    wxInt32        iNetworkMode  = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != menu);

    menu->Append( wxID_OPEN, _("&Open"), wxEmptyString );
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
    switch( iNetworkMode )
    {
        case CMainDocument::MODE_NEVER:
            menu->Check( ID_TB_NETWORKSUSPEND, true );
            break;
        default:
            menu->Check( ID_TB_NETWORKSUSPEND, false );
            break;
    }

    PopupMenu( menu );

    delete menu;
}


void CTaskBarIcon::OnLButtonDClick( wxEvent& event )
{
    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    if ( NULL != pFrame )
        pFrame->Show();
}


bool CTaskBarIcon::ShowBalloon( wxString title, wxString message, unsigned int timeout, ICONTYPES icon )
{
   if (!IsOK())
      return false;

   wxInt32 iPlatform = 0;
   wxInt32 iMajorVersion = 0;
   wxInt32 iMinorVersion = 0;
   bool    bRetVal = false;

   iPlatform = wxGetOsVersion( &iMajorVersion, &iMinorVersion );

#ifdef __WXMSW__

   if ( ( wxWINDOWS_NT == iPlatform ) && ( 5 >= iMajorVersion ) )
   {
        NOTIFYICONDATA notifyData;

        memset(&notifyData, 0, sizeof(notifyData));

        notifyData.cbSize           = sizeof(notifyData);
        notifyData.hWnd             = (HWND) m_hWnd;
        notifyData.uID              = 99;
        notifyData.uCallbackMessage = sm_taskbarMsg;
        notifyData.uFlags           = NIF_MESSAGE | NIF_INFO;
        notifyData.dwInfoFlags      = icon | NIIF_NOSOUND;
        notifyData.uTimeout         = timeout;
        lstrcpyn(notifyData.szInfo, WXSTRINGCAST message, sizeof(notifyData.szInfo));
        lstrcpyn(notifyData.szInfoTitle, WXSTRINGCAST title, sizeof(notifyData.szInfoTitle));

        if (m_iconAdded)
            bRetVal = (Shell_NotifyIcon(NIM_MODIFY, &notifyData) != 0);
   }
   else
   {
       wxString strMessage;
       strMessage = title + wxT("\n") + message;
       SetIcon( iconTaskBarIcon, strMessage );
   }

#endif

   return bRetVal;
}


void CTaskBarIcon::ResetTaskBar()
{
    ShowBalloon( wxT(""), wxT("") );
    dtLastMouseCaptureTime = wxDateTime::Now();
}

