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

#ifndef _BOINCTASKBAR_H_
#define _BOINCTASKBAR_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCTaskBar.cpp"
#endif

#ifdef __WXMSW__
#include "msw/taskbarex.h"
#else
#define wxTaskBarIconEx     wxTaskBarIcon
#endif


class CTaskBarIcon : public wxTaskBarIconEx
{
    DECLARE_DYNAMIC_CLASS(CTaskBarIcon)

public:
    CTaskBarIcon();
    ~CTaskBarIcon();

    void OnOpen( wxCommandEvent& event );
    void OnActivitySelection( wxCommandEvent& event );
    void OnNetworkSelection( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );
    void OnExit( wxCommandEvent& event );

    void OnClose( wxCloseEvent& event );

    void OnMouseMove( wxTaskBarIconEvent& event );
    void OnLButtonDClick( wxTaskBarIconEvent& event );

#ifdef __WXMSW__
    void OnContextMenu( wxTaskBarIconExEvent& event );
#endif

    void OnRButtonDown( wxTaskBarIconEvent& event );
    void OnRButtonUp( wxTaskBarIconEvent& event );

private:

    wxIcon     m_iconTaskBarIcon;
    wxDateTime m_dtLastHoverDetected;
    wxDateTime m_dtLastBalloonDisplayed;

    bool       m_bButtonPressed;

    void       ResetTaskBar();
    void       CreateContextMenu();

    DECLARE_EVENT_TABLE()

};


#endif

