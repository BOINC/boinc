/////////////////////////////////////////////////////////////////////////
// File:        taskbar.cpp
// Purpose:     Implements wxTaskBarIconEx class for manipulating icons on
//              the Windows task bar.
// Author:      Julian Smart
// Modified by:
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


LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, unsigned msg, UINT wParam, LONG lParam );

wxChar* wxTaskBarExWindowClass = (wxChar*) wxT("wxTaskBarExWindowClass");
wxChar* wxTaskBarExWindow = (wxChar*) wxT("wxTaskBarExWindow");

const UINT WM_TASKBARCREATED   = ::RegisterWindowMessage(wxT("TaskbarCreated"));
const UINT WM_TASKBARSHUTDOWN  = ::RegisterWindowMessage(wxT("TaskbarShutdown"));

bool   wxTaskBarIconEx::sm_registeredClass = FALSE;
UINT   wxTaskBarIconEx::sm_taskbarMsg = 0;

DEFINE_EVENT_TYPE( wxEVT_TASKBAR_CREATED )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_CONTEXT_MENU )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_SELECT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_KEY_SELECT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_SHOW )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_HIDE )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_TIMEOUT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_USERCLICK )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_SHUTDOWN )

IMPLEMENT_DYNAMIC_CLASS(wxTaskBarIconEx, wxEvtHandler)

BEGIN_EVENT_TABLE (wxTaskBarIconEx, wxEvtHandler)
    EVT_CLOSE(wxTaskBarIconEx::OnClose)
    EVT_TASKBAR_CREATED(wxTaskBarIconEx::OnTaskBarCreated)
END_EVENT_TABLE ()


wxTaskBarIconEx::wxTaskBarIconEx(void)
{
    m_hWnd = 0;
    m_iconAdded = FALSE;

    if (RegisterWindowClass())
        m_hWnd = CreateTaskBarWindow( wxTaskBarExWindow );
}

wxTaskBarIconEx::wxTaskBarIconEx( wxChar* szWindowTitle )
{
    m_hWnd = 0;
    m_iconAdded = FALSE;

    if (RegisterWindowClass())
        m_hWnd = CreateTaskBarWindow( szWindowTitle );
}

wxTaskBarIconEx::~wxTaskBarIconEx(void)
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
    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::OnClose - Function Begin"));

    ::DestroyWindow((HWND) m_hWnd);
    m_hWnd = 0;

    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::OnClose - Function End"));
}

void wxTaskBarIconEx::OnTaskBarCreated(wxTaskBarIconExEvent& WXUNUSED(event))
{
    m_iconAdded = false;
    UpdateIcon();
}

// Operations
bool wxTaskBarIconEx::SetIcon(const wxIcon& icon)
{
    if (!IsOK())
        return false;

    if (!icon.Ok())
        return false;

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = 99;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags           = NIF_MESSAGE | NIF_ICON;
    notifyData.uVersion         = NOTIFYICON_VERSION;

    lstrcpyn(notifyData.szInfo, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szInfo));
    lstrcpyn(notifyData.szInfoTitle, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szInfoTitle));
    lstrcpyn(notifyData.szTip, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szTip));
    notifyData.hIcon            = (HICON) icon.GetHICON();

    UpdateIcon();
    return m_iconAdded;
}

bool wxTaskBarIconEx::SetTooltip(const wxString& message)
{
    if (!IsOK())
        return false;

    if (message.empty())
        return false;

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = 99;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags           = NIF_MESSAGE | NIF_TIP;
    notifyData.uVersion         = NOTIFYICON_VERSION;

    lstrcpyn(notifyData.szInfo, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szInfo));
    lstrcpyn(notifyData.szInfoTitle, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szInfoTitle));
    lstrcpyn(notifyData.szTip, WXSTRINGCAST message, sizeof(notifyData.szTip));
    notifyData.hIcon            = NULL;

    UpdateIcon();
    return m_iconAdded;
}

bool wxTaskBarIconEx::SetBalloon(const wxIcon& icon, const wxString title, const wxString message, unsigned int timeout, unsigned int iconballoon)
{
    if (!IsOK())
        return false;

    if (!icon.Ok())
        return false;

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = 99;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags           = NIF_MESSAGE | NIF_INFO | NIF_TIP | NIF_ICON;
    notifyData.dwInfoFlags      = iconballoon | NIIF_NOSOUND;
    notifyData.uTimeout         = timeout;
    notifyData.uVersion         = NOTIFYICON_VERSION;

    lstrcpyn(notifyData.szInfo, WXSTRINGCAST message, sizeof(notifyData.szInfo));
    lstrcpyn(notifyData.szInfoTitle, WXSTRINGCAST title, sizeof(notifyData.szInfoTitle));
    lstrcpyn(notifyData.szTip, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szTip));
    notifyData.hIcon            = (HICON) icon.GetHICON();

    UpdateIcon();
    return m_iconAdded;
}

bool wxTaskBarIconEx::RemoveIcon()
{
    if (!m_iconAdded)
        return FALSE;

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize = sizeof(notifyData);
    notifyData.hWnd = (HWND) m_hWnd;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags = NIF_MESSAGE;
    notifyData.hIcon = 0 ; // hIcon;
    notifyData.uID = 99;
    m_iconAdded = FALSE;

    return (Shell_NotifyIcon(NIM_DELETE, & notifyData) != 0);
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

bool wxTaskBarIconEx::PopupMenu(wxMenu *menu) //, int x, int y);
{
    // OK, so I know this isn't thread-friendly, but
    // what to do? We need this check.

    static bool s_inPopup = FALSE;

    if (s_inPopup)
        return FALSE;

    s_inPopup = TRUE;

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
    ::SetForegroundWindow ((HWND) win->GetHWND ());

    rval = win->PopupMenu(menu, 0, 0);

    // Work around a WIN32 bug
    ::PostMessage ((HWND) win->GetHWND(),WM_NULL,0,0L);

    win->PopEventHandler(FALSE);
    win->Destroy();
    delete win;

    s_inPopup = FALSE;

    return rval;
}


bool wxTaskBarIconEx::RegisterWindowClass()
{
    if (sm_registeredClass)
        return TRUE;

    // Also register the taskbar message here
    sm_taskbarMsg = ::RegisterWindowMessage(wxT("wxTaskBarIconExMessage"));

    WNDCLASS        wc;
    bool        rc;

    HINSTANCE hInstance = GetModuleHandle(NULL);

    /*
     * set up and register window class
     */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC) wxTaskBarIconExWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = 0;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wxTaskBarExWindowClass ;
    rc = (::RegisterClass( &wc ) != 0);

    sm_registeredClass = (rc != 0);

    return( (rc != 0) );
}

WXHWND wxTaskBarIconEx::CreateTaskBarWindow( wxChar* szWindowTitle )
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    HWND hWnd = CreateWindowEx (0, wxTaskBarExWindowClass,
            szWindowTitle,
            WS_OVERLAPPED,
            0,
            0,
            10,
            10,
            NULL,
            (HMENU) 0,
            hInstance,
            NULL);

    return (WXHWND) hWnd;
}

bool wxTaskBarIconEx::IsBalloonsSupported()
{
#ifdef __WXMSW__
    wxInt32 iMajor = 0, iMinor = 0;
    if ( wxWINDOWS_NT == wxGetOsVersion( &iMajor, &iMinor ) )
    {
        if ( (5 >= iMajor) && (0 <= iMinor) )
            return true;
    }
#endif
    return false;
}

long wxTaskBarIconEx::WindowProc( WXHWND hWnd, unsigned int msg, unsigned int wParam, long lParam )
{
    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::WindowProc - Function Begin"));

    wxEventType eventType = 0;
    long        lReturnValue = 0;     

    if      ( WM_CLOSE == msg )
    {
        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_CLOSE Detected"));
 
        wxCloseEvent eventClose(wxEVT_CLOSE_WINDOW);
        ProcessEvent(eventClose);

        if ( !eventClose.GetSkipped() )
            lReturnValue = DefWindowProc((HWND) hWnd, msg, wParam, lParam);
        else
            lReturnValue = 0;
    }
    else if ( WM_TASKBARCREATED == msg )
    {
        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_TASKBARCREATED Detected"));
        eventType = wxEVT_TASKBAR_CREATED;
    }
    else if ( WM_TASKBARSHUTDOWN == msg )
    {
        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_TASKBARSHUTDOWN Detected"));
        eventType = wxEVT_TASKBAR_SHUTDOWN;
    }
    if (msg != sm_taskbarMsg)
        lReturnValue = DefWindowProc((HWND) hWnd, msg, wParam, lParam);

    if ( 0 == eventType )
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
                eventType = wxEVT_TASKBAR_BALLOON_TIMEOUT;
                break;

            case NIN_BALLOONUSERCLICK:
                eventType = wxEVT_TASKBAR_BALLOON_USERCLICK;
                break;
        }
    }

    if (eventType)
    {
        wxTaskBarIconExEvent event(eventType, this);
        ProcessEvent(event);

        lReturnValue = 0;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::WindowProc - Function End"));
    return lReturnValue;
}

LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, unsigned msg, UINT wParam, LONG lParam )
{
    return wxGetApp().GetTaskBarIcon()->WindowProc((WXHWND) hWnd, msg, wParam, lParam);
}

