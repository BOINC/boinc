/////////////////////////////////////////////////////////////////////////
// File:        wx/gtk/taskbarex.h
// Purpose:     wxTaskBarIconEx
// Author:      Vaclav Slavik
// Modified by: Paul Cornett / Rom Walton
// Created:     2004/05/29
// RCS-ID:      $Id$
// Copyright:   (c) Vaclav Slavik, 2004
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "taskbarex.cpp"
#endif

#ifndef _TASKBAREX_H_
#define _TASKBAREX_H_

// ----------------------------------------------------------------------------
// wxTaskBarIconEx Balloon Types
// ----------------------------------------------------------------------------

#define BALLOONTYPE_INFO        0x00000001
#define BALLOONTYPE_WARNING     0x00000002
#define BALLOONTYPE_ERROR       0x00000003

// ----------------------------------------------------------------------------
// wxTaskBarIconEx
// ----------------------------------------------------------------------------

class wxTaskBarIconExEvent;

class wxTaskBarIconEx: public wxEvtHandler
{
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(wxTaskBarIconEx)
    DECLARE_NO_COPY_CLASS(wxTaskBarIconEx)
public:

    wxTaskBarIconEx();
    wxTaskBarIconEx( wxChar* szWindowTitle, wxInt32 iTaskbarID );

    virtual ~wxTaskBarIconEx();

// Accessors
    bool IsOK() const { return true; }
    bool IsIconInstalled() const;

// Event Callbacks
    void ClearEvents();
    void FireUserClickedEvent();
    bool IsUserClicked();

// Operations
    virtual bool SetIcon(
        const wxIcon& icon,
        const wxString& message = wxEmptyString
    );

    virtual bool SetBalloon(
        const wxIcon& icon,
        const wxString title = wxEmptyString,
        const wxString message = wxEmptyString,
        unsigned int iconballoon = BALLOONTYPE_INFO
    );

    virtual bool QueueBalloon(
        const wxIcon& icon,
        const wxString title = wxEmptyString,
        const wxString message = wxEmptyString,
        unsigned int iconballoon = BALLOONTYPE_INFO
    );

    virtual bool RemoveIcon();

    bool PopupMenu(wxMenu *menu);

// Implementation
    bool IsBalloonsSupported() const { return false; };

// Data members
protected:
    wxWindow*           m_pWnd;
    wxInt32             m_iTaskbarID;
    bool                m_bUserClicked;
};


// ----------------------------------------------------------------------------
// wxTaskBarIconEx events
// ----------------------------------------------------------------------------

class wxTaskBarIconExEvent : public wxEvent
{
public:
    wxTaskBarIconExEvent(wxEventType evtType, wxTaskBarIconEx *tbIcon)
        : wxEvent(wxID_ANY, evtType)
        {
            SetEventObject(tbIcon);
        }

    virtual wxEvent *Clone() const { return new wxTaskBarIconExEvent(*this); }
};

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_CREATED, 1557 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_CONTEXT_MENU, 1558 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_SELECT, 1559 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_KEY_SELECT, 1560 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_SHOW, 1561 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_HIDE, 1562 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_USERTIMEOUT, 1563 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_USERCLICK, 1564 )
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_SHUTDOWN, 1565 )
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxTaskBarIconExEventFunction)(wxTaskBarIconExEvent&);

#define wxTaskBarIconExEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxTaskBarIconExEventFunction, &func)

#define wx__DECLARE_TASKBAREXEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_TASKBAR_ ## evt, wxTaskBarIconExEventHandler(fn))

#define EVT_TASKBAR_CREATED(fn)              wx__DECLARE_TASKBAREXEVT(CREATED, fn)
#define EVT_TASKBAR_CONTEXT_MENU(fn)         wx__DECLARE_TASKBAREXEVT(CONTEXT_MENU, fn)
#define EVT_TASKBAR_SELECT(fn)               wx__DECLARE_TASKBAREXEVT(SELECT, fn)
#define EVT_TASKBAR_KEY_SELECT(fn)           wx__DECLARE_TASKBAREXEVT(KEY_SELECT, fn)
#define EVT_TASKBAR_BALLOON_SHOW(fn)         wx__DECLARE_TASKBAREXEVT(BALLOON_SHOW, fn)
#define EVT_TASKBAR_BALLOON_HIDE(fn)         wx__DECLARE_TASKBAREXEVT(BALLOON_HIDE, fn)
#define EVT_TASKBAR_BALLOON_USERTIMEOUT(fn)  wx__DECLARE_TASKBAREXEVT(BALLOON_USERTIMEOUT, fn)
#define EVT_TASKBAR_CONTEXT_USERCLICK(fn)    wx__DECLARE_TASKBAREXEVT(BALLOON_USERCLICK, fn)
#define EVT_TASKBAR_SHUTDOWN(fn)             wx__DECLARE_TASKBAREXEVT(SHUTDOWN, fn)


#endif
    // _TASKBAREX_H_
