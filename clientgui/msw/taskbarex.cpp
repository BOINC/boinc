/////////////////////////////////////////////////////////////////////////
// File:        taskbar.cpp
// Purpose:     Implements wxTaskBarIconEx class for manipulating icons on
//              the Windows task bar.
// Author:      Julian Smart
// Modified by: Rom Walton
// Created:     24/3/98
// RCS-ID:      $Id$
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#pragma implementation "taskbarex.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "msw/taskbarex.h"
#include "BOINCTaskBar.h"


// Add items new to Windows Vista and Windows 7
//
#ifndef NIF_GUID
#define NIF_GUID                    0x00000020
#endif
#ifndef NIF_REALTIME
#define NIF_REALTIME                0x00000040
#endif
#ifndef NIF_SHOWTIP
#define NIF_SHOWTIP                 0x00000080
#endif
#ifndef NIIF_LARGE_ICON
#define NIIF_LARGE_ICON             0x00000010
#endif
#ifndef NIIF_RESPECT_QUIET_TIME
#define NIIF_RESPECT_QUIET_TIME     0x00000010
#endif


LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

wxChar* wxTaskBarExWindowClass = (wxChar*) wxT("wxTaskBarExWindowClass");
wxChar* wxTaskBarExWindow      = (wxChar*) wxT("wxTaskBarExWindow");

const UINT WM_TASKBARCREATED    = ::RegisterWindowMessage(wxT("TaskbarCreated"));
const UINT WM_TASKBARMESSAGE    = ::RegisterWindowMessage(wxT("TaskbarMessage"));
const UINT WM_TASKBARSHUTDOWN   = ::RegisterWindowMessage(wxT("TaskbarShutdown"));
const UINT WM_TASKBARAPPRESTORE = ::RegisterWindowMessage(wxT("TaskbarAppRestore"));

DEFINE_EVENT_TYPE( wxEVT_TASKBAR_CREATED )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_CONTEXT_MENU )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_SELECT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_KEY_SELECT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_SHOW )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_HIDE )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_USERTIMEOUT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_USERCLICK )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_SHUTDOWN )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_APPRESTORE )

IMPLEMENT_DYNAMIC_CLASS(wxTaskBarIconEx, wxEvtHandler)

BEGIN_EVENT_TABLE (wxTaskBarIconEx, wxEvtHandler)
    EVT_CLOSE(wxTaskBarIconEx::OnClose)
    EVT_TASKBAR_CREATED(wxTaskBarIconEx::OnTaskBarCreated)
END_EVENT_TABLE ()


wxTaskBarIconEx::wxTaskBarIconEx()
{
    m_pTaskbarMutex = new wxMutex();
    m_iTaskbarID = 0;
    m_iconAdded = FALSE;
    m_hWnd = CreateTaskBarWindow( wxTaskBarExWindow );
}

wxTaskBarIconEx::wxTaskBarIconEx( wxChar* szWindowTitle, wxInt32 iTaskbarID )
{
    m_pTaskbarMutex = new wxMutex();
    m_iTaskbarID = iTaskbarID;
    m_iconAdded = FALSE;
    m_hWnd = CreateTaskBarWindow( szWindowTitle );
}

wxTaskBarIconEx::~wxTaskBarIconEx()
{
    if (m_iconAdded)
    {
        RemoveIcon();
    }

    if (m_hWnd)
    {
        ::DestroyWindow((HWND) m_hWnd);
        m_hWnd = 0;
    }
}

// Events
void wxTaskBarIconEx::OnClose(wxCloseEvent& WXUNUSED(event))
{
    ::DestroyWindow((HWND) m_hWnd);
    m_hWnd = 0;
}

void wxTaskBarIconEx::OnTaskBarCreated(wxTaskBarIconExEvent& WXUNUSED(event))
{
    m_iconAdded = false;
    UpdateIcon();
}

// Operations
bool wxTaskBarIconEx::SetIcon(const wxIcon& icon, const wxString& message)
{
    if (!IsOK())
        return false;

    if (!icon.Ok())
        return false;

    memset(&notifyData, 0, sizeof(NOTIFYICONDATA));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = m_iTaskbarID;
    notifyData.uCallbackMessage = WM_TASKBARMESSAGE;
    notifyData.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_REALTIME;
    notifyData.uVersion         = NOTIFYICON_VERSION;
    notifyData.hIcon            = (HICON) icon.GetHICON();

    if (!message.empty()) {
        notifyData.uFlags       |= NIF_TIP;
        lstrcpyn(notifyData.szTip, message.c_str(), ARRAYSIZE(notifyData.szTip));
    }

    UpdateIcon();
    return m_iconAdded;
}

bool wxTaskBarIconEx::SetBalloon(const wxIcon& icon, const wxString title, const wxString message, unsigned int iconballoon)
{
    if (!IsOK())
        return false;

    if (!icon.Ok())
        return false;

    memset(&notifyData, 0, sizeof(NOTIFYICONDATA));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = m_iTaskbarID;
    notifyData.uCallbackMessage = WM_TASKBARMESSAGE;
    notifyData.uFlags           = NIF_MESSAGE | NIF_INFO | NIF_ICON | NIF_REALTIME;
    notifyData.dwInfoFlags      = iconballoon | NIIF_NOSOUND | NIIF_RESPECT_QUIET_TIME;
    notifyData.uVersion         = NOTIFYICON_VERSION;
    notifyData.hIcon            = (HICON) icon.GetHICON();

    lstrcpyn(notifyData.szInfoTitle, title.c_str(), ARRAYSIZE(notifyData.szInfoTitle));
    lstrcpyn(notifyData.szInfo, message.c_str(), ARRAYSIZE(notifyData.szInfo));

    UpdateIcon();
    return m_iconAdded;
}

bool wxTaskBarIconEx::QueueBalloon(const wxIcon& icon, const wxString title, const wxString message, unsigned int iconballoon)
{
    if (!IsOK())
        return false;

    if (!icon.Ok())
        return false;

    memset(&notifyData, 0, sizeof(NOTIFYICONDATA));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = m_iTaskbarID;
    notifyData.uCallbackMessage = WM_TASKBARMESSAGE;
    notifyData.uFlags           = NIF_MESSAGE | NIF_INFO | NIF_ICON;
    notifyData.dwInfoFlags      = iconballoon | NIIF_RESPECT_QUIET_TIME;
    notifyData.uVersion         = NOTIFYICON_VERSION;
    notifyData.hIcon            = (HICON) icon.GetHICON();

    lstrcpyn(notifyData.szInfoTitle, title.c_str(), ARRAYSIZE(notifyData.szInfoTitle));
    lstrcpyn(notifyData.szInfo, message.c_str(), ARRAYSIZE(notifyData.szInfo));

    UpdateIcon();
    return m_iconAdded;
}

bool wxTaskBarIconEx::RemoveIcon()
{
    if (!m_iconAdded)
        return FALSE;

    memset(&notifyData, 0, sizeof(NOTIFYICONDATA));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = m_iTaskbarID;

    m_iconAdded = FALSE;

    return (Shell_NotifyIcon(NIM_DELETE, &notifyData) != 0);
}

void wxTaskBarIconEx::UpdateIcon()
{
    if (m_iconAdded)
    {
        Shell_NotifyIcon(NIM_MODIFY, &notifyData);
    }
    else
    {
        m_iconAdded = (Shell_NotifyIcon(NIM_ADD, &notifyData) != 0);
        if (IsBalloonsSupported())
        {
            Shell_NotifyIcon(NIM_SETVERSION, &notifyData);
        }
    }
}

bool wxTaskBarIconEx::PopupMenu(wxMenu *menu)
{
    wxMutexLocker lock(*m_pTaskbarMutex);
    if (!lock.IsOk()) return false;

    bool        rval = FALSE;
    wxWindow*   win;
    int         x, y;
    wxGetMousePosition(&x, &y);

    // is wxFrame the best window type to use???
    win = new wxFrame(NULL, -1, wxEmptyString, wxPoint(x,y), wxSize(-1,-1), 0);
    win->PushEventHandler(this);

    // Remove from record of top-level windows, or will confuse wxWindows
    // if we try to exit right now.
    wxTopLevelWindows.DeleteObject(win);

    menu->UpdateUI();

    // Work around a WIN32 bug
    ::SetForegroundWindow ((HWND) win->GetHWND());

    rval = win->PopupMenu(menu, 0, 0);

    // Work around a WIN32 bug
    ::PostMessage ((HWND) win->GetHWND(), WM_NULL, 0, 0L);

    win->PopEventHandler(FALSE);
    win->Destroy();
    delete win;

    return rval;
}

bool wxTaskBarIconEx::FireAppRestore()
{
    HWND hWnd = ::FindWindow(wxTaskBarExWindowClass, NULL);
    if (hWnd) {
        ::SendMessage(hWnd, WM_TASKBARAPPRESTORE, NULL, NULL);
        return true;
    }
    return false;
}

WXHWND wxTaskBarIconEx::CreateTaskBarWindow( wxChar* szWindowTitle )
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HWND hWnd = NULL;
    WNDCLASS wc;

    if (!::GetClassInfo( hInstance, wxTaskBarExWindowClass, &wc )) {
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = (WNDPROC) wxTaskBarIconExWindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = 0;
        wc.hCursor = 0;
        wc.hbrBackground = 0;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = wxTaskBarExWindowClass;
        ::RegisterClass(&wc);
    }

    hWnd = ::CreateWindowEx (
        0,
        wxTaskBarExWindowClass,
        szWindowTitle,
        WS_OVERLAPPED,
        0,
        0,
        10,
        10,
        NULL,
        (HMENU)0,
        hInstance,
        NULL
    );

    return (WXHWND)hWnd;
}

bool wxTaskBarIconEx::IsBalloonsSupported()
{
    return true;
}

LRESULT wxTaskBarIconEx::WindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    wxEventType eventType = 0;

    if      ( WM_CLOSE == msg )
    {
        wxCloseEvent eventClose(wxEVT_CLOSE_WINDOW);
        ProcessEvent(eventClose);

        if ( !eventClose.GetSkipped() ) {
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }

        return 0;
    }
    else if ( WM_TASKBARCREATED == msg )
    {
        eventType = wxEVT_TASKBAR_CREATED;
    }
    else if ( WM_TASKBARSHUTDOWN == msg )
    {
        eventType = wxEVT_TASKBAR_SHUTDOWN;
    }
    else if ( WM_TASKBARAPPRESTORE == msg )
    {
        eventType = wxEVT_TASKBAR_APPRESTORE;
    }
    else if ( WM_TASKBARMESSAGE == msg )
    {
        switch (lParam)
        {
            case WM_LBUTTONDOWN:
                eventType = wxEVT_TASKBAR_LEFT_DOWN;
                break;

            case WM_LBUTTONUP:
                eventType = wxEVT_TASKBAR_LEFT_UP;
                break;

            case WM_RBUTTONDOWN:
                eventType = wxEVT_TASKBAR_RIGHT_DOWN;
                break;

            case WM_RBUTTONUP:
                eventType = wxEVT_TASKBAR_RIGHT_UP;
                break;

            case WM_LBUTTONDBLCLK:
                eventType = wxEVT_TASKBAR_LEFT_DCLICK;
                break;

            case WM_RBUTTONDBLCLK:
                eventType = wxEVT_TASKBAR_RIGHT_DCLICK;
                break;

            case WM_MOUSEMOVE:
                eventType = wxEVT_TASKBAR_MOVE;
                break;

            case WM_CONTEXTMENU:
                eventType = wxEVT_TASKBAR_CONTEXT_MENU;
                break;

            case NIN_SELECT:
                eventType = wxEVT_TASKBAR_SELECT;
                break;

            case NIN_KEYSELECT:
                eventType = wxEVT_TASKBAR_KEY_SELECT;
                break;

            case NIN_BALLOONSHOW:
                eventType = wxEVT_TASKBAR_BALLOON_SHOW;
                break;

            case NIN_BALLOONHIDE:
                eventType = wxEVT_TASKBAR_BALLOON_HIDE;
                break;

            case NIN_BALLOONTIMEOUT:
                eventType = wxEVT_TASKBAR_BALLOON_USERTIMEOUT;
                break;

            case NIN_BALLOONUSERCLICK:
                eventType = wxEVT_TASKBAR_BALLOON_USERCLICK;
                break;
        }
    }
    else
    {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    if (eventType)
    {
        wxTaskBarIconExEvent event(eventType, this);
        return ProcessEvent(event);
    }

    return 0;
}

LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return wxGetApp().GetTaskBarIcon()->WindowProc(hWnd, msg, wParam, lParam);
}

