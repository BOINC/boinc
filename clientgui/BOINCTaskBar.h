// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#ifdef __APPLE__
#define NSInteger int
#endif

class CTaskbarEvent;

class CTaskBarIcon : public wxTaskBarIcon {
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

#ifndef __WXMAC__
    void OnNotificationClick(wxTaskBarIconEvent& event);
    void OnNotificationTimeout(wxTaskBarIconEvent& event);
#endif
#ifdef __WXMSW__
    void OnShutdown(CTaskbarEvent& event);
#endif
    void OnLButtonDClick(wxTaskBarIconEvent& event);
#ifdef __WXGTK__
    void OnRButtonDown(wxTaskBarIconEvent& event);
#endif
#ifdef __WXMSW__
    void OnRButtonUp(wxTaskBarIconEvent& event);
    void FireShutdown();
#endif

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
#endif  // __WXMAC__


#define BALLOONTYPE_INFO 0
    bool IsBalloonsSupported();

    bool QueueBalloon(
        const wxIcon& icon,
        const wxString& title = wxEmptyString,
        const wxString& message = wxEmptyString,
        unsigned int iconballoon = BALLOONTYPE_INFO
    );

    wxIcon          m_iconTaskBarNormal;
    wxIcon          m_iconTaskBarDisconnected;
    wxIcon          m_iconTaskBarSnooze;

    wxIcon          m_iconCurrentIcon;

private:
    wxMenuItem*     m_SnoozeMenuItem = nullptr;
    wxMenuItem*     m_SnoozeGPUMenuItem = nullptr;

    wxDateTime      m_dtLastNotificationAlertExecuted;
    int             m_iLastNotificationUnreadMessageCount = 0;

    void            ResetTaskBar();
    void            DisplayContextMenu();

    void            UpdateTaskbarStatus();
    void            UpdateNoticeStatus();
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

wxDECLARE_EVENT(wxEVT_TASKBAR_RELOADSKIN, CTaskbarEvent);
wxDECLARE_EVENT(wxEVT_TASKBAR_REFRESH, CTaskbarEvent);
#ifdef __WXMSW__
wxDECLARE_EVENT(wxEVT_TASKBAR_SHUTDOWN, CTaskbarEvent);
#endif // __WXMSW__

#endif
