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
END_EVENT_TABLE ()


CTaskBarIcon::CTaskBarIcon() : 
    wxTaskBarIcon()
{
    wxIcon icon( boinc_xpm );
    SetIcon( icon, wxGetApp().GetAppName() );
}


CTaskBarIcon::~CTaskBarIcon()
{
    RemoveIcon();
}


void CTaskBarIcon::OnMouseMove( wxEvent& event )
{
    ShowBalloon( wxGetApp().GetAppName(), wxT("This is a test, this is only a test.") );
}


void CTaskBarIcon::OnLButtonDown( wxEvent& event )
{
}


void CTaskBarIcon::OnLButtonUp( wxEvent& event )
{
}


void CTaskBarIcon::OnRButtonDown( wxEvent& event )
{
}


void CTaskBarIcon::OnRButtonUp( wxEvent& event )
{
}


void CTaskBarIcon::OnLButtonDClick( wxEvent& event )
{
}


void CTaskBarIcon::OnRButtonDClick( wxEvent& event )
{
}


bool CTaskBarIcon::ShowBalloon(wxString title, wxString message, unsigned int timeout, int icon)
{
   if (!IsOK())
      return false;

#ifdef __WXMSW__

   NOTIFYICONDATA notifyData;

   memset(&notifyData, 0, sizeof(notifyData));
   notifyData.cbSize = sizeof(notifyData);
   notifyData.hWnd = (HWND) m_hWnd;
   notifyData.uCallbackMessage = sm_taskbarMsg;
   notifyData.uFlags = NIF_MESSAGE;

   notifyData.uFlags |= NIF_INFO;
   lstrcpyn(notifyData.szInfo, WXSTRINGCAST message, sizeof(notifyData.szInfo));
   lstrcpyn(notifyData.szInfoTitle, WXSTRINGCAST title, sizeof(notifyData.szInfoTitle));
   notifyData.dwInfoFlags = icon | NIIF_NOSOUND;
   notifyData.uTimeout = timeout;

   notifyData.uID = 99;

   if (m_iconAdded)
      return (Shell_NotifyIcon(NIM_MODIFY, &notifyData) != 0);
   else
      return false;

#endif

}
