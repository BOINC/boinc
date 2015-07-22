/////////////////////////////////////////////////////////////////////////
// File:        src/gtk/taskbarex.cpp
// Purpose:     wxTaskBarIconEx
// Author:      Vaclav Slavik
// Modified by: Paul Cornett / Rom Walton
// Created:     2004/05/29
// RCS-ID:      $Id$
// Copyright:   (c) Vaclav Slavik, 2004
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#pragma implementation "taskbarex.h"
#endif

#define GSocket GlibGSocket
#include <gtk/gtk.h>
#undef GSocket

#include "stdwx.h"

#include <libnotify/notify.h>
#include <glib.h>
#include <dlfcn.h>

#include "BOINCGUIApp.h"
#include "gtk/taskbarex.h"
#include "BOINCTaskBar.h"


// Old Style
typedef NotifyNotification* (*__notify_notification_new_with_status_icon)
(
	const gchar *summary,
    const gchar *body,
	const gchar *icon,
    GtkStatusIcon *status_icon
);

// New Style
typedef NotifyNotification* (*__notify_notification_new)
(
     const char *summary,
     const char *body,
     const char *icon
);

static void* notify_lib = NULL;
static __notify_notification_new_with_status_icon my_notify_notification_new_with_status_icon = NULL;
static __notify_notification_new my_notify_notification_new = NULL;


static GtkStatusIcon* g_pStatusIcon = NULL;
static NotifyNotification* g_pNotification = NULL;


//-----------------------------------------------------------------------------

extern "C" {
    static void
    status_icon_activate(GtkStatusIcon*, wxTaskBarIconEx* taskBarIcon)
    {
        wxTaskBarIconExEvent eventLeftDClick(wxEVT_TASKBAR_LEFT_DCLICK, taskBarIcon);
        taskBarIcon->AddPendingEvent(eventLeftDClick);
    }

    static void
    status_icon_popup_menu(GtkStatusIcon*, guint, guint, wxTaskBarIconEx* taskBarIcon)
    {
        wxTaskBarIconExEvent eventDown(wxEVT_TASKBAR_RIGHT_DOWN, taskBarIcon);
        taskBarIcon->AddPendingEvent(eventDown);
        wxTaskBarIconExEvent eventUp(wxEVT_TASKBAR_RIGHT_UP, taskBarIcon);
        taskBarIcon->AddPendingEvent(eventUp);
    }

    static void
    status_icon_notification_actions(NotifyNotification* notification, gchar *action, wxTaskBarIconEx* taskBarIcon)
    {
        if (strcmp(action, "default") == 0) {
            taskBarIcon->FireUserClickedEvent();
        }
    }

    static void
    status_icon_notification_closed(NotifyNotification* notification, wxTaskBarIconEx* taskBarIcon)
    {
        if (taskBarIcon->IsUserClicked()) {
            wxTaskBarIconExEvent eventUserClicked(wxEVT_TASKBAR_BALLOON_USERCLICK, taskBarIcon);
            taskBarIcon->AddPendingEvent(eventUserClicked);
        } else {
            wxTaskBarIconExEvent eventTimeout(wxEVT_TASKBAR_BALLOON_USERTIMEOUT, taskBarIcon);
            taskBarIcon->AddPendingEvent(eventTimeout);
        }

        taskBarIcon->ClearEvents();
    }
}


//-----------------------------------------------------------------------------


static wxChar* wxTaskBarExWindow      = (wxChar*) wxT("wxTaskBarExWindow");


DEFINE_EVENT_TYPE( wxEVT_TASKBAR_CREATED )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_CONTEXT_MENU )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_SELECT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_KEY_SELECT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_SHOW )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_HIDE )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_USERTIMEOUT )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_BALLOON_USERCLICK )
DEFINE_EVENT_TYPE( wxEVT_TASKBAR_SHUTDOWN )

IMPLEMENT_DYNAMIC_CLASS(wxTaskBarIconEx, wxEvtHandler)

BEGIN_EVENT_TABLE (wxTaskBarIconEx, wxEvtHandler)
END_EVENT_TABLE ()


wxTaskBarIconEx::wxTaskBarIconEx()
{
    wxTaskBarIconEx((wxChar*)wxTaskBarExWindow, 1);
}

wxTaskBarIconEx::wxTaskBarIconEx( wxChar* szWindowTitle, wxInt32 iTaskbarID )
{
    m_pWnd = NULL;
    m_iTaskbarID = iTaskbarID;
    g_pStatusIcon = NULL;
    g_pNotification = NULL;
    m_bUserClicked = false;

    notify_lib = dlopen("libnotify.so", RTLD_NOW);
    if (notify_lib) {
        my_notify_notification_new_with_status_icon = (__notify_notification_new_with_status_icon)dlsym(notify_lib, "notify_notification_new_with_status_icon");
        my_notify_notification_new = (__notify_notification_new)dlsym(notify_lib, "notify_notification_new");
    }

    notify_init((const char*)wxString(szWindowTitle).mb_str());
}

wxTaskBarIconEx::~wxTaskBarIconEx()
{
    m_bUserClicked = false;

    if (m_pWnd)
    {
        m_pWnd->PopEventHandler();
        m_pWnd->Destroy();
        m_pWnd = NULL;
    }

    if (g_pNotification)
    {
        notify_notification_close(g_pNotification, NULL);
        g_pNotification = NULL;
    }

    if (g_pStatusIcon)
    {
        g_object_unref(g_pStatusIcon);
        g_pStatusIcon = NULL;
    }

    if (notify_lib) {
        my_notify_notification_new_with_status_icon = NULL;
        my_notify_notification_new = NULL;
        dlclose(notify_lib);
    }
}

bool wxTaskBarIconEx::IsIconInstalled() const {
    return (g_pStatusIcon != NULL);
}

void wxTaskBarIconEx::ClearEvents() {
    m_bUserClicked = false;
}

void wxTaskBarIconEx::FireUserClickedEvent() {
    m_bUserClicked = true;
}

bool wxTaskBarIconEx::IsUserClicked() {
    return m_bUserClicked;
}

// Operations
bool wxTaskBarIconEx::SetIcon(const wxIcon& icon, const wxString& message)
{
    if (!IsOK())
        return false;

    if (!icon.Ok())
        return false;

    wxBitmap bitmap = icon;

    if (!g_pStatusIcon)
    {
        g_pStatusIcon = gtk_status_icon_new_from_pixbuf(bitmap.GetPixbuf());
        g_signal_connect(g_pStatusIcon, "activate", G_CALLBACK(status_icon_activate), this);
        g_signal_connect(g_pStatusIcon, "popup_menu", G_CALLBACK(status_icon_popup_menu), this);
    }

    gtk_status_icon_set_from_pixbuf(g_pStatusIcon, bitmap.GetPixbuf());
    if (!message.empty())
    {
        gtk_status_icon_set_tooltip_text(g_pStatusIcon, message.mb_str());
    }
    gtk_status_icon_set_visible(g_pStatusIcon, TRUE);

    return true;
}

static const char* SetBalloon__returnIcon(const unsigned int iconballoon) {
    switch(iconballoon)
    {
        case BALLOONTYPE_INFO:
            return(GTK_STOCK_DIALOG_INFO);
            break;
        case BALLOONTYPE_WARNING:
            return(GTK_STOCK_DIALOG_WARNING);
            break;
        case BALLOONTYPE_ERROR:
        default:
            return(GTK_STOCK_DIALOG_ERROR);
            break;
    }
    return(NULL);
}

bool wxTaskBarIconEx::SetBalloon(const wxIcon& icon, const wxString title, const wxString message, unsigned int iconballoon)
{
    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::SetBalloon - Function Begin"));

    bool retval = false;
    GError* error = NULL;

    if (!IsOK())
        return false;

    if (!icon.Ok())
        return false;

    if (!SetIcon(icon, wxEmptyString))
        return false;

    const char* desired_icon = SetBalloon__returnIcon(iconballoon);

    if (!g_pNotification)
    {
        // Old Style
        if (my_notify_notification_new_with_status_icon) {
            g_pNotification =
                (*my_notify_notification_new_with_status_icon)(
                    title.mb_str(),
                    message.mb_str(),
                    desired_icon,
                    g_pStatusIcon
                );
        }

        // New Style
        if (my_notify_notification_new) {
            g_pNotification =
                (*my_notify_notification_new)(
                    title.mb_str(),
                    message.mb_str(),
                    gtk_status_icon_get_icon_name(g_pStatusIcon)
            );
        }

        g_signal_connect(
            g_pNotification,
            "closed",
            G_CALLBACK(status_icon_notification_closed),
            this
        );

        notify_notification_add_action(
            g_pNotification,
            "default",
            "Do Default Action",
            NOTIFY_ACTION_CALLBACK(status_icon_notification_actions),
            this,
            NULL
        );
    }
    else
    {
        notify_notification_update(
            g_pNotification,
            title.mb_str(),
            message.mb_str(),
            desired_icon
        );
    }

    retval = notify_notification_show(g_pNotification, &error);
    g_clear_error(&error);

    wxLogTrace(wxT("Function Start/End"), wxT("wxTaskBarIconEx::SetBalloon - Function End"));
    return retval;
}

bool wxTaskBarIconEx::QueueBalloon(const wxIcon& icon, const wxString title, const wxString message, unsigned int iconballoon)
{
    // There isn't two classifications of notifications on Linux as there is on Windows
    return SetBalloon(icon, title, message, iconballoon);
}

bool wxTaskBarIconEx::RemoveIcon()
{
    if (!IsOK())
        return false;

    if (g_pNotification)
    {
        notify_notification_close(g_pNotification, NULL);
        g_pNotification = NULL;
    }

    if (g_pStatusIcon)
    {
        g_object_unref(g_pStatusIcon);
        g_pStatusIcon = NULL;
    }

    return true;
}

bool wxTaskBarIconEx::PopupMenu(wxMenu* menu)
{
#if wxUSE_MENUS

    if (m_pWnd == NULL)
    {
        m_pWnd = new wxTopLevelWindow(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
        m_pWnd->PushEventHandler(this);
    }

    wxPoint point(-1, -1);
#ifdef __WXUNIVERSAL__
    point = wxGetMousePosition();
#endif

    m_pWnd->PopupMenu(menu, point);

#endif // wxUSE_MENUS
    return true;
}

