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

class wxNotifyIcon: public wxNotifyIconBase {

    DECLARE_DYNAMIC_CLASS_NO_COPY(wxNotifyIcon)
//    DECLARE_EVENT_TABLE()

public:
    wxNotifyIcon();
    virtual ~wxNotifyIcon();

// Accessors
    inline bool IsOk() const { return true; }
    inline bool IsIconInstalled() const { return m_iconAdded; }

// Operations
    bool SetIcon(const wxIcon& icon, const wxString& tooltip = wxEmptyString);
    bool RemoveIcon();
    bool PopupMenu(wxMenu* menu);

    bool SetBalloon(
        const wxIcon& icon, 
        const wxString title = wxEmptyString,
        const wxString message = wxEmptyString,
        unsigned int timeout = 15000,
        unsigned int iconballoon = NIIF_INFO
    );

    bool CancelBalloon();

	bool SetTooltip(const wxString tip);


    static bool IsBalloonsSupported();

// Data members
protected:

    struct NotifyIconData : public NOTIFYICONDATA
    {
        NotifyIconData(WXHWND hwnd);
    };

    // NB: this class serves two purposes:
    //     1. win32 needs a HWND associated with taskbar icon, this provides it
    //     2. we need wxTopLevelWindow so that the app doesn't exit when
    //        last frame is closed but there still is a taskbar icon
    class wxNotifyIconWindow : public wxFrame {

    public:
        wxNotifyIconWindow(wxNotifyIcon *icon);

        WXLRESULT MSWWindowProc(WXUINT msg,
                                WXWPARAM wParam, WXLPARAM lParam);

    private:
        wxNotifyIcon *m_icon;
    };

    long WindowProc(unsigned int msg, unsigned int wParam, long lParam);
    void RegisterWindowMessages();
// Data members
    wxNotifyIconWindow*  m_win;
    bool                 m_iconAdded;
    wxIcon               m_icon;
    wxString             m_strTooltip;

    static UINT   s_msgTaskbar;
    static UINT   s_msgRestartTaskbar;

private:
    //bool UpdateIcon();
};

// Additional events
//DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_CREATED, 1590 )
//DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_SHUTDOWN, 1591 )
//
//#define EVT_NOTIFYICON_TASKBAR_CREATED(fn)      wx__DECLARE_NOTIFYICONEVT(TASKBAR_CREATED, fn)
//#define EVT_NOTIFYICON_TASKBAR_SHUTDOWN(fn)     wx__DECLARE_NOTIFYICONEVT(TASKBAR_SHUTDOWN, fn)

#endif // _MSW_NOTIFYICON_H_