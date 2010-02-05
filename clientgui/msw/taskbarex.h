/////////////////////////////////////////////////////////////////////////
// File:        wx/msw/taskbar.h
// Purpose:     Defines wxTaskBarIcon class for manipulating icons on the
//              Windows task bar.
// Author:      Julian Smart
// Modified by:
// Created:     24/3/98
// RCS-ID:      $Id$
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////

#ifndef _TASKBAREX_H_
#define _TASKBAREX_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "taskbarex.cpp"
#endif


// ----------------------------------------------------------------------------
// wxTaskBarIconEx 
// ----------------------------------------------------------------------------

class wxTaskBarIconExEvent;

class wxTaskBarIconEx: public wxEvtHandler {
    DECLARE_DYNAMIC_CLASS(wxTaskBarIconEx)
public:

    wxTaskBarIconEx();
    wxTaskBarIconEx( wxChar* szWindowTitle, wxInt32 iTaskbarID );

    virtual ~wxTaskBarIconEx();

// Events
    virtual void OnClose( wxCloseEvent& event );
    virtual void OnTaskBarCreated( wxTaskBarIconExEvent& event );

// Accessors
    inline WXHWND GetHWND() const { return m_hWnd; }
    inline bool IsOK() const { return (m_hWnd != 0) ; }
    inline bool IsIconInstalled() const { return m_iconAdded; }

// Operations

    virtual bool SetIcon( const wxIcon& icon, const wxString& message = wxEmptyString );

    virtual bool SetBalloon(
        const wxIcon& icon, 
        const wxString title = wxEmptyString,
        const wxString message = wxEmptyString,
        unsigned int iconballoon = NIIF_INFO
    );

    virtual bool QueueBalloon(
        const wxIcon& icon, 
        const wxString title = wxEmptyString,
        const wxString message = wxEmptyString,
        unsigned int iconballoon = NIIF_INFO
    );

    virtual bool RemoveIcon();
    virtual void UpdateIcon();

    bool PopupMenu(wxMenu *menu);

// Implementation
    WXHWND CreateTaskBarWindow( wxChar* szWindowTitle );
    static bool IsBalloonsSupported();
    long WindowProc( WXHWND hWnd, unsigned int msg, unsigned int wParam, long lParam );

// Data members
protected:
    wxMutex*        m_pTaskbarMutex;
    WXHWND          m_hWnd;
    wxInt32         m_iTaskbarID;
    bool            m_iconAdded;
    NOTIFYICONDATA  notifyData;


private:
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxTaskBarIconEx)
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
    DECLARE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_TIMEOUT, 1563 )
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
#define EVT_TASKBAR_BALLOON_TIMEOUT(fn)      wx__DECLARE_TASKBAREXEVT(BALLOON_TIMEOUT, fn)
#define EVT_TASKBAR_CONTEXT_USERCLICK(fn)    wx__DECLARE_TASKBAREXEVT(BALLOON_USERCLICK, fn)
#define EVT_TASKBAR_SHUTDOWN(fn)             wx__DECLARE_TASKBAREXEVT(SHUTDOWN, fn)


#endif
    // _TASKBAR_H_





