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

#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CTaskBarIcon, wxTaskBarIcon)

BEGIN_EVENT_TABLE (CTaskBarIcon, wxTaskBarIcon)
    EVT_CLOSE(CTaskBarIcon::OnClose)
    EVT_MENU(wxID_EXIT, CTaskBarIcon::OnExit)
END_EVENT_TABLE ()


CTaskBarIcon::CTaskBarIcon() : 
    wxTaskBarIcon()
{
    iconTaskBarIcon = wxIcon( boinc_xpm );
    dtLastMouseCaptureTime = wxDateTime::Now();

    SetIcon( iconTaskBarIcon, wxEmptyString );
}


CTaskBarIcon::~CTaskBarIcon()
{
    RemoveIcon();
}


void CTaskBarIcon::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    CMainFrame* pFrame = NULL;
    pFrame = wxGetApp().GetFrame();
    if ( NULL != pFrame )
        pFrame->Close(true);
}


void CTaskBarIcon::OnClose( wxCloseEvent& event )
{
    CMainFrame* pFrame = NULL;
    pFrame = wxGetApp().GetFrame();
    if ( NULL != pFrame )
        pFrame->Close(true);
}


void CTaskBarIcon::OnMouseMove( wxEvent& event )
{
    wxTimeSpan ts(wxDateTime::Now() - dtLastMouseCaptureTime);

    if ( ts.GetSeconds() > 5 )
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
    wxMenu* menu = new wxMenu;
    wxASSERT( NULL != menu );

    menu->Append(
        wxID_EXIT,
        _("E&xit"),
        wxEmptyString
    );

    PopupMenu( menu );

    delete menu;
}


void CTaskBarIcon::OnLButtonDClick( wxEvent& event )
{
    CMainFrame* pFrame = NULL;
    pFrame = wxGetApp().GetFrame();
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
