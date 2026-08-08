// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOINC_BOINCTASKBAR_H
#define BOINC_BOINCTASKBAR_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCTaskBar.cpp"
#endif

#ifdef __APPLE__
#define NSInteger int
#endif

#if   defined(__WXMSW__)
#include "msw/taskbarex.h"
#elif defined(__WXGTK__)
#include "gtk/taskbarex.h"
#else
#define wxTaskBarIconEx         wxTaskBarIcon
#define wxTaskBarIconExEvent    wxTaskBarIconEvent
#endif

class CTaskbarEvent;

class CTaskBarIcon : public wxTaskBarIconEx {
public:
    CTaskBarIcon(wxIconBundle* icon, wxIconBundle* iconDisconnected, wxIconBundle* iconSnooze
#ifdef __WXMAC__
                , wxTaskBarIconType iconType
#endif
                );
    ~CTaskBarIcon();

    void OnOpenWebsite(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSuspendResume(wxCommandEvent& event);
    void OnSuspendResumeGPU(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    void OnIdle(wxIdleEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnRefresh(CTaskbarEvent& event);
    void OnReloadSkin(CTaskbarEvent& event);

    void OnNotificationClick(wxTaskBarIconExEvent& event);
    void OnNotificationTimeout(wxTaskBarIconExEvent& event);
    void OnAppRestore(wxTaskBarIconExEvent& event);
    void OnShutdown(wxTaskBarIconExEvent& event);
    void OnLButtonDClick(wxTaskBarIconEvent& event);
    void OnRButtonDown(wxTaskBarIconEvent& event);
    void OnRButtonUp(wxTaskBarIconEvent& event);

    void FireReloadSkin();

    wxMenu *BuildContextMenu();
    void AdjustMenuItems(wxMenu* menu);

    wxSize GetBestIconSize();

#ifdef __WXMAC__
private:
    NSInteger m_pNotificationRequest;
    wxTaskBarIconType m_iconType;
    void MacRequestUserAttention();
    void MacCancelUserAttentionRequest();
    bool SetMacTaskBarIcon(const wxIcon& icon);
    int SetDockBadge(wxBitmap* bmp);

public:
    wxMenu *CreatePopupMenu();
#if wxCHECK_VERSION(3,1,6)
    bool SetIcon(const wxBitmapBundle& icon, const wxString& message = wxEmptyString);
#else
    bool SetIcon(const wxIcon& icon, const wxString& message = wxEmptyString);
#endif

#define BALLOONTYPE_INFO 0
    bool IsBalloonsSupported();

    bool QueueBalloon(
        const wxIcon& icon,
        const wxString title = wxEmptyString,
        const wxString message = wxEmptyString,
        unsigned int iconballoon = BALLOONTYPE_INFO
    );
#endif  // __WXMAC__

    wxIcon          m_iconTaskBarNormal;
    wxIcon          m_iconTaskBarDisconnected;
    wxIcon          m_iconTaskBarSnooze;

    wxIcon          m_iconCurrentIcon;

    bool            m_bTaskbarInitiatedShutdown;

private:
    bool            m_bMouseButtonPressed;
    wxMenuItem*     m_SnoozeMenuItem;
    wxMenuItem*     m_SnoozeGPUMenuItem;

    wxDateTime      m_dtLastNotificationAlertExecuted;
    int             m_iLastNotificationUnreadMessageCount;

    void            ResetTaskBar();
    void            DisplayContextMenu();

    void            UpdateTaskbarStatus();
    void            UpdateNoticeStatus();

    DECLARE_EVENT_TABLE()
};


class CTaskbarEvent : public wxEvent
{
public:
    CTaskbarEvent(wxEventType evtType, CTaskBarIcon *taskbar)
        : wxEvent(-1, evtType)
        {
            SetEventObject(taskbar);
        }

    CTaskbarEvent(wxEventType evtType, CTaskBarIcon *taskbar, wxString message)
        : wxEvent(-1, evtType), m_message(message)
        {
            SetEventObject(taskbar);
        }

    virtual wxEvent *Clone() const { return new CTaskbarEvent(*this); }

    wxString                m_message;
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_TASKBAR_RELOADSKIN, 10100 )
DECLARE_EVENT_TYPE( wxEVT_TASKBAR_REFRESH, 10101 )
END_DECLARE_EVENT_TYPES()

#define EVT_TASKBAR_RELOADSKIN(fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASKBAR_RELOADSKIN, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_TASKBAR_REFRESH(fn)  DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASKBAR_REFRESH, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

#endif
