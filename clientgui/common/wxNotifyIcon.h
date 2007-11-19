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

#ifndef _NOTIFYICON_H_
#define _NOTIFYICON_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "wxNotifyIcon.cpp"
#endif

//// msw provides a complete native implementation, so we derive from wxTaskBarIconBase instead.
//#ifdef __WXMSW__
//    #define wxTaskBarIconNative wxTaskBarIconBase
//#else
    #define wxTaskBarIconNative wxTaskBarIcon
//#endif

class wxNotifyIconEvent;
class wxNotifyIcon;

class wxNotifyIconBase: public wxTaskBarIconNative {

    DECLARE_NO_COPY_CLASS(wxNotifyIconBase)

public:
    wxNotifyIconBase() {}
//    wxNotifyIconBase(const wxString& title) : wxTaskBarIcon(title) {}

    virtual ~wxNotifyIconBase() {}

// Operations

    virtual bool ShowBalloon(
        const wxIcon& WXUNUSED(icon), 
        const wxString& WXUNUSED(title = wxEmptyString),
        const wxString& WXUNUSED(message = wxEmptyString),
        unsigned int WXUNUSED(timeout = 10000),
        unsigned int WXUNUSED(iconballoon = NIIF_INFO)
    ) { return false; }

    virtual bool HideBalloon() { return false; }
	virtual bool SetTooltip(const wxString& WXUNUSED(tip)) { return false; }

//    virtual bool PopupMenu(wxMenu* WXUNUSED(menu)) { return false; }

//    virtual bool ProcessEvent(wxEvent& event);

protected:
    virtual bool ShowTooltipWindow() { return false; }
    virtual bool HideTooltipWindow() { return false; }

private:

};

#ifdef __WXMSW__
    #include "msw/wxNotifyIcon.h"
#else
    typedef wxNotifyIcon wxNotifyIconBase;
#endif

class wxNotifyIconEvent : public wxTaskBarIconEvent
{
public:
    wxNotifyIconEvent(wxEventType evtType, wxNotifyIcon* tbIcon)
        : wxTaskBarIconEvent(evtType, NULL) {
            SetEventObject(wxDynamicCast(tbIcon, wxObject));
        }

    virtual wxEvent *Clone() const { return new wxNotifyIconEvent(*this); }
};

typedef void (wxEvtHandler::*wxNotifyIconEventFunction)(wxNotifyIconEvent&);

#define wxNotifyIconEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxNotifyIconEventFunction, &func)

//// From standard (old) TaskBarIcon. Adapt for compatability with new TaskBarIcon.
//DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_CREATED, 1570 )
//DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_CONTEXT_MENU, 1571 )
//DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_SELECT, 1572 )
//DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_KEY_SELECT, 1573 )
// New events extending TaskBarIcon.
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_SHOW, 1580 )
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_HIDE, 1581 )
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_TIMEOUT, 1582 )
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_USERCLICK, 1583 )
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_POPUP_SHOW, 1584 )
DECLARE_EVENT_TYPE( wxEVT_NOTIFYICON_POPUP_HIDE, 1585 )

// Helper macro to simplify event macro definitions
#define wx__DECLARE_NOTIFYICONEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_NOTIFYICON_ ## evt, wxNotifyIconEventHandler(fn))

//#define EVT_NOTIFYICON_TASKBAR_CREATED(fn)      wx__DECLARE_NOTIFYICONEVT(TASKBAR_CREATED, fn)
//#define EVT_NOTIFYICON_CONTEXT_MENU(fn)         wx__DECLARE_NOTIFYICONEVT(CONTEXT_MENU, fn)
//#define EVT_NOTIFYICON_SELECT(fn)               wx__DECLARE_NOTIFYICONEVT(SELECT, fn)
//#define EVT_NOTIFYICON_KEY_SELECT(fn)           wx__DECLARE_NOTIFYICONEVT(KEY_SELECT, fn)

#define EVT_NOTIFYICON_BALLOON_SHOW(fn)         wx__DECLARE_NOTIFYICONEVT(BALLOON_SHOW, fn)
#define EVT_NOTIFYICON_BALLOON_HIDE(fn)         wx__DECLARE_NOTIFYICONEVT(BALLOON_HIDE, fn)
#define EVT_NOTIFYICON_BALLOON_TIMEOUT(fn)      wx__DECLARE_NOTIFYICONEVT(BALLOON_TIMEOUT, fn)
#define EVT_NOTIFYICON_BALLOON_USERCLICK(fn)    wx__DECLARE_NOTIFYICONEVT(BALLOON_USERCLICK, fn)
#define EVT_NOTIFYICON_POPUP_SHOW(fn)           wx__DECLARE_NOTIFYICONEVT(POPUP_SHOW, fn)
#define EVT_NOTIFYICON_POPUP_HIDE(fn)           wx__DECLARE_NOTIFYICONEVT(POPUP_HIDE, fn)

#endif // _NOTIFYICON_H_





