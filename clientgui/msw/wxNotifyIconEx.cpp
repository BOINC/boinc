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

#ifdef __GNUG__
#pragma implementation "taskbarex.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
//#include "msw/taskbarex.h"
#include "BOINCTaskBar.h"


LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, unsigned msg, UINT wParam, LONG lParam );

wxChar* wxTaskBarExWindowClass = (wxChar*) wxT("wxTaskBarExWindowClass");
wxChar* wxTaskBarExWindow = (wxChar*) wxT("wxTaskBarExWindow");

const UINT WM_TASKBARCREATED   = ::RegisterWindowMessage(wxT("TaskbarCreated"));
const UINT WM_TASKBARSHUTDOWN  = ::RegisterWindowMessage(wxT("TaskbarShutdown"));

bool   wxNotifyIcon::sm_registeredClass = FALSE;
UINT   wxNotifyIcon::sm_taskbarMsg = 0;

DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_CREATED )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_SHUTDOWN )

IMPLEMENT_DYNAMIC_CLASS(wxNotifyIcon, wxNotifyIconBase)

BEGIN_EVENT_TABLE (wxNotifyIcon, wxNotifyIconBase)
    EVT_CLOSE(wxNotifyIcon::OnClose)
    EVT_NOTIFYICON_TASKBAR_CREATED(wxNotifyIcon::OnTaskBarCreated)
END_EVENT_TABLE ()


wxNotifyIcon::wxNotifyIcon() {
    m_hWnd = 0;
    m_iconAdded = FALSE;

    if (RegisterWindowClass())
        m_hWnd = CreateTaskBarWindow( wxTaskBarExWindow );
}

wxNotifyIcon::wxNotifyIcon(const wxString& title) {
    m_hWnd = 0;
    m_iconAdded = FALSE;

    if (RegisterWindowClass())
        m_hWnd = CreateTaskBarWindow(title.c_str());
}

wxNotifyIcon::~wxNotifyIcon(void)
{
    if (m_iconAdded) {
        RemoveIcon();
    }

    if (m_hWnd) {
        ::DestroyWindow((HWND) m_hWnd);
        m_hWnd = 0;
    }
}

// Events
void wxNotifyIcon::OnClose(wxCloseEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::OnClose - Function Begin"));

    ::DestroyWindow((HWND) m_hWnd);
    m_hWnd = 0;

    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::OnClose - Function End"));
}

void wxNotifyIcon::OnTaskBarCreated(wxNotifyIconEvent& WXUNUSED(event)) {
    m_iconAdded = false;
    UpdateIcon();
}

// Operations
bool wxNotifyIcon::SetIcon(const wxIcon& icon, const wxString& tooltip) {
    if (!IsOK())
        return FALSE;

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = 99;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags           = NIF_MESSAGE;
    notifyData.uVersion         = NOTIFYICON_VERSION;

    if (icon.Ok()) {
        notifyData.uFlags |= NIF_ICON;
        notifyData.hIcon = (HICON) icon.GetHICON();
    }

    if (((const wxChar*) tooltip != NULL) && (tooltip != wxT(""))) {
        notifyData.uFlags |= NIF_TIP ;
        lstrcpyn(notifyData.szTip, WXSTRINGCAST tooltip, sizeof(notifyData.szTip));
    }

    return UpdateIcon();
}

// timeout is clamped between 10 seconds and 30 seconds by the OS.
bool wxNotifyIcon::SetBalloon(const wxIcon& icon,
                              const wxString title,
                              const wxString message,
                              unsigned int timeout,
                              unsigned int iconballoon)
{
    if (!IsOK())
        return false;

    wxString strTip = wxEmptyString;

    if (!IsBalloonsSupported())
        strTip = title + wxT(" - ") + message;

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = 99;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags           = NIF_MESSAGE;
    notifyData.dwInfoFlags      = iconballoon;
    notifyData.uTimeout         = timeout;

    if (icon.Ok()) {
        // XPSP2 behaviour: NIIF_USER and hIcon.
        notifyData.dwInfoFlags = NIIF_USER;
        notifyData.hIcon = (HICON) icon.GetHICON();
    }

    if (IsBalloonsSupported()) {
        notifyData.uFlags |= NIF_INFO | NIF_TIP;
        lstrcpyn(notifyData.szInfo, WXSTRINGCAST message, sizeof(notifyData.szInfo));
        lstrcpyn(notifyData.szInfoTitle, WXSTRINGCAST title, sizeof(notifyData.szInfoTitle));
        lstrcpyn(notifyData.szTip, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szTip));
    }
    else {
        notifyData.uFlags |= NIF_TIP;
        lstrcpyn(notifyData.szTip, WXSTRINGCAST strTip, sizeof(notifyData.szTip));
    }

    return UpdateIcon();
}

// Kills a visible balloon immediately. Ought to kill a queued balloon, too.
bool wxNotifyIcon::CancelBalloon() {

    if (!IsOK() || !IsBalloonsSupported()) {
        return false;
    }

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = 99;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags           = NIF_INFO;
    notifyData.uVersion         = NOTIFYICON_VERSION;

    lstrcpyn(notifyData.szInfo, WXSTRINGCAST wxEmptyString, sizeof(notifyData.szInfo));

    return UpdateIcon();
}

bool wxNotifyIcon::SetTooltip(const wxString tip) {
	if (!IsOK())
	    return false;

    memset(&notifyData, 0, sizeof(notifyData));
    notifyData.cbSize           = sizeof(notifyData);
    notifyData.hWnd             = (HWND) m_hWnd;
    notifyData.uID              = 99;
    notifyData.uCallbackMessage = sm_taskbarMsg;
    notifyData.uFlags           = NIF_TIP;

    lstrcpyn(notifyData.szTip, WXSTRINGCAST tip, sizeof(notifyData.szTip));

    return UpdateIcon();
}


bool wxNotifyIcon::RemoveIcon(void) {
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

bool wxNotifyIcon::UpdateIcon() {
    if (m_iconAdded) {
        return (Shell_NotifyIcon(NIM_MODIFY, &notifyData) != 0);
    } else {
        m_iconAdded = (Shell_NotifyIcon(NIM_ADD, &notifyData) != 0);
        if (IsBalloonsSupported()) {
            memset(&notifyData, 0, sizeof(notifyData));
            notifyData.cbSize           = sizeof(notifyData);
            notifyData.hWnd             = (HWND) m_hWnd;
            notifyData.uID              = 99;
            notifyData.uCallbackMessage = sm_taskbarMsg;
            notifyData.uVersion         = NOTIFYICON_VERSION;

            Shell_NotifyIcon(NIM_SETVERSION, &notifyData);
        }
    }
    return m_iconAdded;
}

bool wxNotifyIcon::PopupMenu(wxMenu *menu) {
    CancelBalloon();
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


bool wxNotifyIcon::RegisterWindowClass() {
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

WXHWND wxNotifyIcon::CreateTaskBarWindow(const wxChar* szWindowTitle) {
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

bool wxNotifyIcon::IsBalloonsSupported() {
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

long wxNotifyIcon::WindowProc( WXHWND hWnd, unsigned int msg, unsigned int wParam, long lParam ) {
    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::WindowProc - Function Begin"));

    wxEventType eventType = 0;
    long        lReturnValue = 0;     

    if (WM_CLOSE == msg) {
        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_CLOSE Detected"));
 
        wxCloseEvent eventClose(wxEVT_CLOSE_WINDOW);
        ProcessEvent(eventClose);

        if ( !eventClose.GetSkipped() )
            lReturnValue = DefWindowProc((HWND) hWnd, msg, wParam, lParam);
        else
            lReturnValue = 0;
    }
    else if (WM_TASKBARCREATED == msg) {
        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_TASKBARCREATED Detected"));
        eventType = wxEVT_NOTIFYICON_TASKBAR_CREATED;
    }
    else if (WM_TASKBARSHUTDOWN == msg) {
        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_TASKBARSHUTDOWN Detected"));
        eventType = wxEVT_NOTIFYICON_TASKBAR_SHUTDOWN;
    }
    if (msg != sm_taskbarMsg)
        lReturnValue = DefWindowProc((HWND) hWnd, msg, wParam, lParam);

    if (0 == eventType) {
        switch (lParam) {
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
                eventType = wxEVT_NOTIFYICON_CONTEXT_MENU;
                break;

            case NIN_SELECT:
                eventType = wxEVT_NOTIFYICON_SELECT;
                break;

            case NIN_KEYSELECT:
                eventType = wxEVT_NOTIFYICON_KEY_SELECT;
                break;

            case NIN_BALLOONSHOW:
                eventType = wxEVT_NOTIFYICON_BALLOON_SHOW;
                break;

            case NIN_BALLOONHIDE:
                eventType = wxEVT_NOTIFYICON_BALLOON_HIDE;
                break;

            case NIN_BALLOONTIMEOUT:
                eventType = wxEVT_NOTIFYICON_BALLOON_TIMEOUT;
                break;

            case NIN_BALLOONUSERCLICK:
                eventType = wxEVT_NOTIFYICON_BALLOON_USERCLICK;
                break;
        }
    }

    if (eventType) {
        wxNotifyIconEvent event(eventType, this);
        ProcessEvent(event);

        lReturnValue = 0;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::WindowProc - Function End"));
    return lReturnValue;
}


LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, unsigned msg, UINT wParam, LONG lParam ) {
    return wxGetApp().GetTaskBarIcon()->WindowProc((WXHWND) hWnd, msg, wParam, lParam);
}

