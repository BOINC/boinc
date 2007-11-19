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

#include "stdwx.h"
#include "common/wxNotifyIcon.h"
//#include "BOINCGUIApp.h"
//#include "BOINCTaskBar.h"

//
//LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, unsigned msg, UINT wParam, LONG lParam );
//
//wxChar* wxTaskBarExWindowClass = (wxChar*) wxT("wxTaskBarExWindowClass");
//wxChar* wxTaskBarExWindow = (wxChar*) wxT("wxTaskBarExWindow");
//
//const UINT WM_TASKBARCREATED   = ::RegisterWindowMessage(wxT("TaskbarCreated"));
//const UINT WM_TASKBARSHUTDOWN  = ::RegisterWindowMessage(wxT("TaskbarShutdown"));
//
// initialized on demand
//UINT wxNotifyIcon::s_msgTaskbar = 0;
//UINT wxNotifyIcon::s_msgRestartTaskbar = 0;

UINT s_msgTaskbar = 0;
UINT s_msgRestartTaskbar = 0;
//
//DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_CREATED )
//DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_TASKBAR_SHUTDOWN )

IMPLEMENT_DYNAMIC_CLASS(wxNotifyIcon, wxNotifyIconBase)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxTaskBarIconWindow: helper window
// ----------------------------------------------------------------------------

// NB: this class serves two purposes:
//     1. win32 needs a HWND associated with taskbar icon, this provides it
//     2. we need wxTopLevelWindow so that the app doesn't exit when
//        last frame is closed but there still is a taskbar icon
class wxNotifyIconWindow : public wxFrame
{
public:
    wxNotifyIconWindow(wxNotifyIcon *icon)
        : wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0),
          m_icon(icon)
    {
    }

    WXLRESULT MSWWindowProc(WXUINT msg,
                            WXWPARAM wParam, WXLPARAM lParam)
    {
        if (msg == s_msgRestartTaskbar || msg == s_msgTaskbar)
        {
            return m_icon->WindowProc(msg, wParam, lParam);
        }
        else
        {
            return wxFrame::MSWWindowProc(msg, wParam, lParam);
        }
    }

private:
    wxNotifyIcon *m_icon;
};


// ----------------------------------------------------------------------------
// NotifyIconData: wrapper around NOTIFYICONDATA
// ----------------------------------------------------------------------------

struct NotifyIconData : public NOTIFYICONDATA
{
    NotifyIconData(WXHWND hwnd)
    {
        memset(this, 0, sizeof(NOTIFYICONDATA));
        cbSize = sizeof(NOTIFYICONDATA);
        hWnd = (HWND) hwnd;
        uCallbackMessage = s_msgTaskbar;
        uFlags = NIF_MESSAGE;

        // we use the same id for all taskbar icons as we don't need it to
        // distinguish between them
        uID = 99;
    }
};

// ----------------------------------------------------------------------------
// wxTaskBarIcon
// ----------------------------------------------------------------------------

wxNotifyIcon::wxNotifyIcon()
{
    m_win = NULL;
    m_iconAdded = false;
    RegisterWindowMessages();
}

wxNotifyIcon::~wxNotifyIcon()
{
    if (m_iconAdded)
        RemoveIcon();

    if (m_win)
        m_win->Destroy();
}

// Operations
bool wxNotifyIcon::SetIcon(const wxIcon& icon, const wxString& tooltip)
{
    // NB: we have to create the window lazily because of backward compatibility,
    //     old applications may create a wxTaskBarIcon instance before wxApp
    //     is initialized (as samples/taskbar used to do)
    if (!m_win)
    {
        m_win = new wxNotifyIconWindow(this);
    }

    m_icon = icon;
    m_strTooltip = tooltip;

    NotifyIconData notifyData(GetHwndOf(m_win));

    if (icon.Ok())
    {
        notifyData.uFlags |= NIF_ICON;
        notifyData.hIcon = GetHiconOf(icon);
    }

    if ( !tooltip.empty() )
    {
        notifyData.uFlags |= NIF_TIP;
//        lstrcpyn(notifyData.szTip, tooltip.c_str(), WXSIZEOF(notifyData.szTip));
        wxStrncpy(notifyData.szTip, tooltip.c_str(), WXSIZEOF(notifyData.szTip));
    }

    bool ok = Shell_NotifyIcon(m_iconAdded ? NIM_MODIFY
                                           : NIM_ADD, &notifyData) != 0;

    if ( !m_iconAdded && ok )
        m_iconAdded = true;

    return ok;
}

bool wxNotifyIcon::RemoveIcon()
{
    if (!m_iconAdded)
        return false;

    m_iconAdded = false;

    NotifyIconData notifyData(GetHwndOf(m_win));

    return Shell_NotifyIcon(NIM_DELETE, &notifyData) != 0;
}

bool wxNotifyIcon::PopupMenu(wxMenu *menu)
{
    wxASSERT_MSG( m_win != NULL, _T("taskbar icon not initialized") );

    static bool s_inPopup = false;

    if (s_inPopup)
        return false;

    s_inPopup = true;

    int         x, y;
    wxGetMousePosition(&x, &y);

    m_win->Move(x, y);

    m_win->PushEventHandler(this);

    menu->UpdateUI();

    // the SetForegroundWindow() and PostMessage() calls are needed to work
    // around Win32 bug with the popup menus shown for the notifications as
    // documented at http://support.microsoft.com/kb/q135788/
    ::SetForegroundWindow(GetHwndOf(m_win));

    bool rval = m_win->PopupMenu(menu, 0, 0);

    ::PostMessage(GetHwndOf(m_win), WM_NULL, 0, 0L);

    m_win->PopEventHandler(false);

    s_inPopup = false;

    return rval;
}



void wxNotifyIcon::RegisterWindowMessages()
{
    static bool s_registered = false;

    if ( !s_registered )
    {
        // Taskbar restart msg will be sent to us if the icon needs to be redrawn
        s_msgRestartTaskbar = RegisterWindowMessage(wxT("TaskbarCreated"));

        // Also register the taskbar message here
        s_msgTaskbar = ::RegisterWindowMessage(wxT("wxTaskBarIconMessage"));

        s_registered = true;
    }
}

// ----------------------------------------------------------------------------
// wxTaskBarIcon window proc
// ----------------------------------------------------------------------------

long wxNotifyIcon::WindowProc(unsigned int msg,
                               unsigned int WXUNUSED(wParam),
                               long lParam)
{
    wxEventType eventType = 0;

    if (msg == s_msgRestartTaskbar)   // does the icon need to be redrawn?
    {
        m_iconAdded = false;
        SetIcon(m_icon, m_strTooltip);
    }

    // this function should only be called for gs_msg(Restart)Taskbar messages
    wxASSERT(msg == s_msgTaskbar);

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

        default:
            break;
    }

    if (eventType)
    {
        wxNotifyIconEvent event(eventType, this);

        ProcessEvent(event);
    }

    return 0;
}


//BEGIN_EVENT_TABLE (wxNotifyIcon, wxNotifyIconBase)
//    EVT_CLOSE(wxNotifyIcon::OnClose)
//    EVT_NOTIFYICON_TASKBAR_CREATED(wxNotifyIcon::OnTaskBarCreated)
//END_EVENT_TABLE ()

//// Events
//void wxNotifyIcon::OnClose(wxCloseEvent& WXUNUSED(event)) {
//    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::OnClose - Function Begin"));
//
//    ::DestroyWindow((HWND) m_hWnd);
//    m_hWnd = 0;
//
//    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::OnClose - Function End"));
//}
//
//void wxNotifyIcon::OnTaskBarCreated(wxNotifyIconEvent& WXUNUSED(event)) {
//    m_iconAdded = false;
//    UpdateIcon();
//}
//
//// Operations
//bool wxNotifyIcon::SetIcon(const wxIcon& icon, const wxString& tooltip) {
//    if (!IsOK())
//        return FALSE;
//
//    memset(&notifyData, 0, sizeof(notifyData));
//    notifyData.cbSize           = sizeof(notifyData);
//    notifyData.hWnd             = (HWND) m_hWnd;
//    notifyData.uID              = 99;
//    notifyData.uCallbackMessage = sm_taskbarMsg;
//    notifyData.uFlags           = NIF_MESSAGE;
//    notifyData.uVersion         = NOTIFYICON_VERSION;
//
//    if (icon.Ok()) {
//        notifyData.uFlags |= NIF_ICON;
//        notifyData.hIcon = (HICON) icon.GetHICON();
//    }
//
//    if (((const wxChar*) tooltip != NULL) && (tooltip != wxT(""))) {
//        notifyData.uFlags |= NIF_TIP ;
//        lstrcpyn(notifyData.szTip, WXSTRINGCAST tooltip, sizeof(notifyData.szTip));
//    }
//
//    return UpdateIcon();
//}

// timeout is clamped between 10 seconds and 30 seconds by the OS.
bool wxNotifyIcon::SetBalloon(const wxIcon& icon,
                              const wxString title,
                              const wxString message,
                              unsigned int timeout,
                              unsigned int iconballoon)
{
    if (!m_iconAdded)
        return false;

    NotifyIconData notifyData(GetHwndOf(m_win));
    //wxString strTip = wxEmptyString;

    //if (!IsBalloonsSupported())
    //    strTip = title + wxT(" - ") + message;

    notifyData.dwInfoFlags      = iconballoon;
    notifyData.uTimeout         = timeout;

    if (icon.Ok()) {
        // XPSP2 behaviour: NIIF_USER and hIcon.
        notifyData.dwInfoFlags = NIIF_USER;
        notifyData.hIcon = (HICON) icon.GetHICON();
    }

    //if (IsBalloonsSupported()) {
        notifyData.uFlags |= NIF_INFO | NIF_TIP;
        wxStrncpy(notifyData.szInfo, message.c_str(), sizeof(notifyData.szInfo));
        wxStrncpy(notifyData.szInfoTitle, title.c_str(), sizeof(notifyData.szInfoTitle));
        wxStrncpy(notifyData.szTip, wxEmptyString, sizeof(notifyData.szTip));
    //}
    //else {
    //    notifyData.uFlags |= NIF_TIP;
    //    lstrcpyn(notifyData.szTip, WXSTRINGCAST strTip, sizeof(notifyData.szTip));
    //}



    return Shell_NotifyIcon(NIM_MODIFY, &notifyData) != 0;
}

// Kills a visible balloon immediately. Ought to kill a queued balloon, too.
bool wxNotifyIcon::CancelBalloon() {

    if (!m_iconAdded)
        return false;

    NotifyIconData notifyData(GetHwndOf(m_win));

    notifyData.uFlags |= NIF_INFO;
    wxStrncpy(notifyData.szInfo, wxEmptyString, sizeof(notifyData.szInfo));

    return Shell_NotifyIcon(NIM_MODIFY, &notifyData) != 0;
}

bool wxNotifyIcon::SetTooltip(const wxString tip) {
	//if (!IsOK())
	    return false;

    //memset(&notifyData, 0, sizeof(notifyData));
    //notifyData.cbSize           = sizeof(notifyData);
    //notifyData.hWnd             = (HWND) m_hWnd;
    //notifyData.uID              = 99;
    //notifyData.uCallbackMessage = sm_taskbarMsg;
    //notifyData.uFlags           = NIF_TIP;

    //lstrcpyn(notifyData.szTip, WXSTRINGCAST tip, sizeof(notifyData.szTip));

    //return UpdateIcon();
}


//
//bool wxNotifyIcon::PopupMenu(wxMenu *menu) {
//    CancelBalloon();
//    // OK, so I know this isn't thread-friendly, but
//    // what to do? We need this check.
//
//    static bool s_inPopup = FALSE;
//
//    if (s_inPopup)
//        return FALSE;
//
//    s_inPopup = TRUE;
//
//    bool        rval = FALSE;
//    wxWindow*   win;
//    int         x, y;
//    wxGetMousePosition(&x, &y);
//
//    // is wxFrame the best window type to use???
//    win = new wxFrame(NULL, -1, wxEmptyString, wxPoint(x,y), wxSize(-1,-1), 0);
//    win->PushEventHandler(this);
//
//    // Remove from record of top-level windows, or will confuse wxWindows
//    // if we try to exit right now.
//    wxTopLevelWindows.DeleteObject(win);
//
//    menu->UpdateUI();
//
//    // Work around a WIN32 bug
//    ::SetForegroundWindow ((HWND) win->GetHWND ());
//
//    rval = win->PopupMenu(menu, 0, 0);
//
//    // Work around a WIN32 bug
//    ::PostMessage ((HWND) win->GetHWND(),WM_NULL,0,0L);
//
//    win->PopEventHandler(FALSE);
//    win->Destroy();
//    delete win;
//
//    s_inPopup = FALSE;
//
//    return rval;
//}


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

//long wxNotifyIcon::WindowProc( WXHWND hWnd, unsigned int msg, unsigned int wParam, long lParam ) {
//    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::WindowProc - Function Begin"));
//
//    wxEventType eventType = 0;
//    long        lReturnValue = 0;     
//
//    if (WM_CLOSE == msg) {
//        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_CLOSE Detected"));
// 
//        wxCloseEvent eventClose(wxEVT_CLOSE_WINDOW);
//        ProcessEvent(eventClose);
//
//        if ( !eventClose.GetSkipped() )
//            lReturnValue = DefWindowProc((HWND) hWnd, msg, wParam, lParam);
//        else
//            lReturnValue = 0;
//    }
//    else if (WM_TASKBARCREATED == msg) {
//        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_TASKBARCREATED Detected"));
//        eventType = wxEVT_NOTIFYICON_TASKBAR_CREATED;
//    }
//    else if (WM_TASKBARSHUTDOWN == msg) {
//        wxLogTrace(wxT("Function Status"), wxT("wxTaskBarIconEx::WindowProc - WM_TASKBARSHUTDOWN Detected"));
//        eventType = wxEVT_NOTIFYICON_TASKBAR_SHUTDOWN;
//    }
//    if (msg != sm_taskbarMsg)
//        lReturnValue = DefWindowProc((HWND) hWnd, msg, wParam, lParam);
//
//    if (0 == eventType) {
//        switch (lParam) {
//            case WM_LBUTTONDOWN:
//                eventType = wxEVT_TASKBAR_LEFT_DOWN;
//                break;
//
//            case WM_LBUTTONUP:
//                eventType = wxEVT_TASKBAR_LEFT_UP;
//                break;
//
//            case WM_RBUTTONDOWN:
//                eventType = wxEVT_TASKBAR_RIGHT_DOWN;
//                break;
//
//            case WM_RBUTTONUP:
//                eventType = wxEVT_TASKBAR_RIGHT_UP;
//                break;
//
//            case WM_LBUTTONDBLCLK:
//                eventType = wxEVT_TASKBAR_LEFT_DCLICK;
//                break;
//
//            case WM_RBUTTONDBLCLK:
//                eventType = wxEVT_TASKBAR_RIGHT_DCLICK;
//                break;
//
//            case WM_MOUSEMOVE:
//                eventType = wxEVT_TASKBAR_MOVE;
//                break;
//
//            case WM_CONTEXTMENU:
//                eventType = wxEVT_NOTIFYICON_CONTEXT_MENU;
//                break;
//
//            case NIN_SELECT:
//                eventType = wxEVT_NOTIFYICON_SELECT;
//                break;
//
//            case NIN_KEYSELECT:
//                eventType = wxEVT_NOTIFYICON_KEY_SELECT;
//                break;
//
//            case NIN_BALLOONSHOW:
//                eventType = wxEVT_NOTIFYICON_BALLOON_SHOW;
//                break;
//
//            case NIN_BALLOONHIDE:
//                eventType = wxEVT_NOTIFYICON_BALLOON_HIDE;
//                break;
//
//            case NIN_BALLOONTIMEOUT:
//                eventType = wxEVT_NOTIFYICON_BALLOON_TIMEOUT;
//                break;
//
//            case NIN_BALLOONUSERCLICK:
//                eventType = wxEVT_NOTIFYICON_BALLOON_USERCLICK;
//                break;
//        }
//    }
//
//    if (eventType) {
//        wxNotifyIconEvent event(eventType, this);
//        // Bypass NotifyIconBase special case handling.
//        wxTaskBarIconBase::ProcessEvent(event);
//
//        lReturnValue = 0;
//    }
//
//    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::WindowProc - Function End"));
//    return lReturnValue;
//}
//
//
//LRESULT APIENTRY wxTaskBarIconExWindowProc( HWND hWnd, unsigned msg, UINT wParam, LONG lParam ) {
//    return wxGetApp().GetTaskBarIcon()->WindowProc((WXHWND) hWnd, msg, wParam, lParam);
//}
//
