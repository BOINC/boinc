// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// Incorporates code from wxWidgets by Julian Smart.

#ifndef _MSW_NOTIFYICON_H_
#define _MSW_NOTIFYICON_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "wxNotifyIcon.cpp"
#endif


class wxNotifyIcon: public wxNotifyIconBase {

    DECLARE_DYNAMIC_CLASS(wxNotifyIcon)
    DECLARE_EVENT_TABLE()

public:
    wxNotifyIcon();
    wxNotifyIcon(const wxString& title);

    virtual ~wxNotifyIcon();

// Events
    virtual void OnClose( wxCloseEvent& event );
    virtual void OnTaskBarCreated( wxNotifyIconEvent& event );

// Accessors
    inline WXHWND GetHWND() const { return m_hWnd; }
    inline bool IsOK() const { return (m_hWnd != 0) ; }
    inline bool IsIconInstalled() const { return m_iconAdded; }

// Operations

    bool SetIcon(
        const wxIcon& icon,
        const wxString& tooltip = wxEmptyString
    );

    bool SetBalloon(
        const wxIcon& icon, 
        const wxString title = wxEmptyString,
        const wxString message = wxEmptyString,
        unsigned int timeout = 15000,
        unsigned int iconballoon = NIIF_INFO
    );

    bool CancelBalloon();

	bool SetTooltip(const wxString tip);

    bool RemoveIcon();

    bool PopupMenu(wxMenu *menu);

    // temp stuff
    void ShowPopupTooltip() {}

// Implementation
    static bool RegisterWindowClass();
    static WXHWND CreateTaskBarWindow(const wxChar* szWindowTitle );
    static bool IsBalloonsSupported();
    long WindowProc( WXHWND hWnd, unsigned int msg, unsigned int wParam, long lParam );

// Data members
protected:
    WXHWND          m_hWnd;
    bool            m_iconAdded;
    NOTIFYICONDATA  notifyData;
    static bool     sm_registeredClass;
    static unsigned int sm_taskbarMsg;

private:
    bool UpdateIcon();
};

// Additional events
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_CREATED, 1590 )
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_SHUTDOWN, 1591 )

#define EVT_NOTIFYICON_TASKBAR_CREATED(fn)      wx__DECLARE_NOTIFYICONEVT(TASKBAR_CREATED, fn)
#define EVT_NOTIFYICON_TASKBAR_SHUTDOWN(fn)     wx__DECLARE_NOTIFYICONEVT(TASKBAR_SHUTDOWN, fn)

#endif // _MSW_NOTIFYICON_H_