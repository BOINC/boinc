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

#ifndef _BOINCTASKBAR_H_
#define _BOINCTASKBAR_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCTaskBar.cpp"
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef __WXMSW__
#include "msw/taskbarex.h"
#else
#define wxTaskBarIconEx     wxTaskBarIcon
#endif


class CTaskBarIcon : public wxTaskBarIconEx {
public:
    CTaskBarIcon(wxString title, wxIcon* icon);
    ~CTaskBarIcon();

    void OnOpenWebsite(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSuspend(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

#ifdef __WXMSW__
    void OnShutdown(wxTaskBarIconExEvent& event);
#endif

    void OnIdle(wxIdleEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnTimer(wxTimerEvent& event);

#ifdef __WXMSW__
    void OnMouseMove(wxTaskBarIconEvent& event);
#endif
    void OnLButtonDClick(wxTaskBarIconEvent& event);

#ifdef __WXMSW__
    void OnContextMenu(wxTaskBarIconExEvent& event);
#else
    void OnContextMenu(wxTaskBarIconEvent& event);
#endif

    void OnRButtonDown(wxTaskBarIconEvent& event);
    void OnRButtonUp(wxTaskBarIconEvent& event);

    wxMenu *BuildContextMenu();
    void AdjustMenuItems(wxMenu* menu);

#ifdef __APPLE__
    wxMenu *CreatePopupMenu();
#endif

#ifndef __WXMSW__
    inline bool CTaskBarIcon::IsBalloonsSupported() {
        return false;
    }
#endif

    wxIcon     m_iconTaskBarIcon;

private:
    wxDateTime m_dtLastHoverDetected;
    wxDateTime m_dtLastBalloonDisplayed;

    wxTimer*   m_pTimer;

    bool       m_bButtonPressed;

    int        m_iSuspendId;
    int        m_iPreviousActivityMode;
    int        m_iPreviousNetworkMode;

    void       ResetSuspendState();
    void       ResetTaskBar();

    void       CreateContextMenu();
    
    DECLARE_EVENT_TABLE()

};


#endif

