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

#include <wx/event.h>
#include <wx/list.h>
#include <wx/icon.h>

class wxTaskBarIconEx: public wxEvtHandler {
    DECLARE_DYNAMIC_CLASS(wxTaskBarIconEx)
public:
    wxTaskBarIconEx(void);
    virtual ~wxTaskBarIconEx(void);

// Accessors
    inline WXHWND GetHWND() const { return m_hWnd; }
    inline bool IsOK() const { return (m_hWnd != 0) ; }
    inline bool IsIconInstalled() const { return m_iconAdded; }

// Operations
    bool SetIcon(const wxIcon& icon, const wxString& tooltip = wxEmptyString);
    bool RemoveIcon(void);
    bool PopupMenu(wxMenu *menu); //, int x, int y);

// Overridables
    virtual void OnMouseMove(wxEvent&);
    virtual void OnLButtonDown(wxEvent&);
    virtual void OnLButtonUp(wxEvent&);
    virtual void OnRButtonDown(wxEvent&);
    virtual void OnRButtonUp(wxEvent&);
    virtual void OnLButtonDClick(wxEvent&);
    virtual void OnRButtonDClick(wxEvent&);

// Implementation
    static wxTaskBarIconEx* FindObjectForHWND(WXHWND hWnd);
    static void AddObject(wxTaskBarIconEx* obj);
    static void RemoveObject(wxTaskBarIconEx* obj);
    static bool RegisterWindowClass();
    static WXHWND CreateTaskBarWindow();
    long WindowProc( WXHWND hWnd, unsigned int msg, unsigned int wParam, long lParam );

// Data members
protected:
    WXHWND          m_hWnd;
    bool            m_iconAdded;
    static wxList   sm_taskBarIcons;
    static bool     sm_registeredClass;
    static unsigned int sm_taskbarMsg;

    // non-virtual default event handlers to forward events to the virtuals
    void _OnMouseMove(wxEvent&);
    void _OnLButtonDown(wxEvent&);
    void _OnLButtonUp(wxEvent&);
    void _OnRButtonDown(wxEvent&);
    void _OnRButtonUp(wxEvent&);
    void _OnLButtonDClick(wxEvent&);
    void _OnRButtonDClick(wxEvent&);


    DECLARE_EVENT_TABLE()
};


// ----------------------------------------------------------------------------
// wxTaskBarIcon events
// ----------------------------------------------------------------------------

class wxTaskBarIconExEvent : public wxEvent
{
public:
    wxTaskBarIconExEvent(wxEventType evtType, wxTaskBarIconEx *tbIcon)
        : wxEvent(-1, evtType)
        {
            SetEventObject(tbIcon);
        }

    virtual wxEvent *Clone() const { return new wxTaskBarIconExEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_TASKBAR_CONTEXT_MENU, 1557 )
END_DECLARE_EVENT_TYPES()

#define EVT_TASKBAR_CONTEXT_MENU(fn)         DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASKBAR_CONTEXT_MENU, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


#endif
    // _TASKBAR_H_





