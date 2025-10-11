// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCTaskBar.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCTaskBar.h"
#include "BOINCBaseFrame.h"
#include "BOINCClientManager.h"
#include "DlgAbout.h"
#include "DlgEventLog.h"
#include "Events.h"

#ifdef __WXMAC__
#include "res/macsnoozebadge.xpm"
#include "res/macdisconnectbadge.xpm"
#include "res/macbadgemask.xpm"

#define MIN_IDLE_TIME_FOR_NOTIFICATION 45
// How long to bounce Dock icon on Mac
#define MAX_NOTIFICATION_DURATION 5
#endif

DEFINE_EVENT_TYPE(wxEVT_TASKBAR_RELOADSKIN)
DEFINE_EVENT_TYPE(wxEVT_TASKBAR_REFRESH)

BEGIN_EVENT_TABLE(CTaskBarIcon, wxTaskBarIconEx)

    EVT_IDLE(CTaskBarIcon::OnIdle)
    EVT_CLOSE(CTaskBarIcon::OnClose)
    EVT_TASKBAR_REFRESH(CTaskBarIcon::OnRefresh)
    EVT_TASKBAR_RELOADSKIN(CTaskBarIcon::OnReloadSkin)
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLButtonDClick)
#ifndef __WXMAC__
    EVT_TASKBAR_RIGHT_DOWN(CTaskBarIcon::OnRButtonDown)
    EVT_TASKBAR_RIGHT_UP(CTaskBarIcon::OnRButtonUp)
    EVT_TASKBAR_CONTEXT_USERCLICK(CTaskBarIcon::OnNotificationClick)
    EVT_TASKBAR_BALLOON_USERTIMEOUT(CTaskBarIcon::OnNotificationTimeout)
#endif
    EVT_MENU(ID_OPENBOINCMANAGER, CTaskBarIcon::OnOpen)
    EVT_MENU(ID_OPENWEBSITE, CTaskBarIcon::OnOpenWebsite)
    EVT_MENU(ID_TB_SUSPEND, CTaskBarIcon::OnSuspendResume)
    EVT_MENU(ID_TB_SUSPEND_GPU, CTaskBarIcon::OnSuspendResumeGPU)
    EVT_MENU(wxID_ABOUT, CTaskBarIcon::OnAbout)
    EVT_MENU(wxID_EXIT, CTaskBarIcon::OnExit)

#ifdef __WXMSW__
    EVT_TASKBAR_SHUTDOWN(CTaskBarIcon::OnShutdown)
    EVT_TASKBAR_APPRESTORE(CTaskBarIcon::OnAppRestore)
#endif

END_EVENT_TABLE()


CTaskBarIcon::CTaskBarIcon(wxIconBundle* icon, wxIconBundle* iconDisconnected, wxIconBundle* iconSnooze
#ifdef __WXMAC__
, wxTaskBarIconType iconType
#endif
) :
#ifdef __WXMAC__
    wxTaskBarIcon(iconType)
#else
    wxTaskBarIconEx(wxT("BOINCManagerSystray"), 1)
#endif
{
#ifdef __WXMAC__
    m_iconType = iconType;
    m_pNotificationRequest = NULL;
    if (iconType == wxTBI_DOCK) {
        // This code expects the wxTBI_CUSTOM_STATUSITEM CTaskBarIcon
        // to be constructed before the wxTBI_DOCK CTaskBarIcon.
        //
        // Ensure that m_pTaskBarIcon and m_pMacDockIcon use same copy of each icon.
        m_iconTaskBarNormal = wxGetApp().GetTaskBarIcon()->m_iconTaskBarNormal;
        m_iconTaskBarDisconnected = wxGetApp().GetTaskBarIcon()->m_iconTaskBarDisconnected;
        m_iconTaskBarSnooze = wxGetApp().GetTaskBarIcon()->m_iconTaskBarSnooze;
    } else
#endif
    {

#ifdef __WXMAC__
        m_iconTaskBarNormal = icon->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_SYSTEM);
        m_iconTaskBarDisconnected = iconDisconnected->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_SYSTEM);
        m_iconTaskBarSnooze = iconSnooze->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_SYSTEM);
#else
        m_iconTaskBarNormal = icon->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_NEAREST_LARGER);
        m_iconTaskBarDisconnected = iconDisconnected->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_NEAREST_LARGER);
        m_iconTaskBarSnooze = iconSnooze->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_NEAREST_LARGER);
#endif

    }
    m_SnoozeGPUMenuItem = NULL;

    m_bTaskbarInitiatedShutdown = false;

    m_bMouseButtonPressed = false;

    m_dtLastNotificationAlertExecuted = wxDateTime((time_t)0);
    m_iLastNotificationUnreadMessageCount = 0;
}


CTaskBarIcon::~CTaskBarIcon() {
    RemoveIcon();
}


void CTaskBarIcon::OnIdle(wxIdleEvent& event) {
    wxGetApp().UpdateSystemIdleDetection();
    event.Skip();
}


void CTaskBarIcon::OnClose(wxCloseEvent& ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function Begin"));

    RemoveIcon();
    m_bTaskbarInitiatedShutdown = true;

    CDlgEventLog* pEventLog = wxGetApp().GetEventLog();
    if (pEventLog) {
        wxLogTrace(wxT("Function Status"), wxT("CTaskBarIcon::OnClose - Closing Event Log"));
        pEventLog->Destroy();
    }

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame) {
        wxLogTrace(wxT("Function Status"), wxT("CTaskBarIcon::OnClose - Closing Current Frame"));
        pFrame->Close(true);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function End"));
}


void CTaskBarIcon::OnRefresh(CTaskbarEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnRefresh - Function Begin"));

    UpdateTaskbarStatus();
    UpdateNoticeStatus();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnRefresh - Function End"));
}


void CTaskBarIcon::OnLButtonDClick(wxTaskBarIconEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnLButtonDClick - Function Begin"));

    wxCommandEvent eventCommand;
    OnOpen(eventCommand);
    if (eventCommand.GetSkipped()) event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnLButtonDClick - Function End"));
}


void CTaskBarIcon::OnNotificationClick(wxTaskBarIconExEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnNotificationClick - Function Begin"));

    ResetTaskBar();
    wxGetApp().ShowNotifications();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnNotificationClick - Function End"));
}


void CTaskBarIcon::OnNotificationTimeout(wxTaskBarIconExEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnNotificationTimeout - Function Begin"));

    ResetTaskBar();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnNotificationTimeout - Function End"));
}


void CTaskBarIcon::OnOpen(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnOpen - Function Begin"));

    ResetTaskBar();
    wxGetApp().ShowInterface();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnOpen - Function End"));
}


void CTaskBarIcon::OnOpenWebsite(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnOpenWebsite - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    ACCT_MGR_INFO      ami;
    wxString           url;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ResetTaskBar();

    pDoc->rpc.acct_mgr_info(ami);
    url = wxString(ami.acct_mgr_url.c_str(), wxConvUTF8);
    wxLaunchDefaultBrowser(url);

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnOpenWebsite - Function End"));
}


void CTaskBarIcon::OnSuspendResume(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnSuspendResume - Function Begin"));
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    CC_STATUS      status;
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ResetTaskBar();
    pDoc->GetCoreClientStatus(status);
    if (status.task_mode_perm == RUN_MODE_NEVER) {
        pDoc->SetActivityRunMode(RUN_MODE_RESTORE, 0);
    } else if (status.task_mode_perm != status.task_mode) {
        pDoc->SetActivityRunMode(RUN_MODE_RESTORE, 0);
    } else {
        pDoc->SetActivityRunMode(RUN_MODE_NEVER, 3600);
    }
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnSuspendResume - Function End"));
}

void CTaskBarIcon::OnSuspendResumeGPU(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnSuspendResumeGPU - Function Begin"));
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    CC_STATUS      status;
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ResetTaskBar();
    pDoc->GetCoreClientStatus(status);
    if (status.gpu_mode_perm == RUN_MODE_NEVER) {
        pDoc->SetGPURunMode(RUN_MODE_RESTORE, 0);
    } else if (status.gpu_mode_perm != status.gpu_mode) {
        pDoc->SetGPURunMode(RUN_MODE_RESTORE, 0);
    } else {
        pDoc->SetGPURunMode(RUN_MODE_NEVER, 3600);
    }
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnSuspendResumeGPU - Function End"));
}

void CTaskBarIcon::OnAbout(wxCommandEvent& WXUNUSED(event)) {
    bool bWasVisible = wxGetApp().IsApplicationVisible();
#ifdef __WXMAC__
    bool bEventLogWasShown = false;

    CDlgEventLog* eventLog = wxGetApp().GetEventLog();
    if (eventLog) {
        bEventLogWasShown = eventLog->IsShown();
        if (bEventLogWasShown && !bWasVisible) eventLog->Show(false);
    }

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame) {
        if (!bWasVisible) {
            // We really do need to hide the frame here
            // See comment in CBOINCGUIApp::OnFinishInit()
            pFrame->wxFrame::Show(false);
        }
    }
#endif

    wxGetApp().ShowApplication(true);

    ResetTaskBar();

    CDlgAbout dlg(NULL);
    wxGetApp().SetAboutDialogIsOpen(true);
    dlg.ShowModal();
    wxGetApp().SetAboutDialogIsOpen(false);

    if (!bWasVisible) {
        wxGetApp().ShowApplication(false);
    }

#ifdef __WXMAC__
    // See comment in CBOINCGUIApp::OnFinishInit()
    pFrame->wxWindow::Show(true);
#endif
}


void CTaskBarIcon::OnExit(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function Begin"));

    if (wxGetApp().ConfirmExit()) {
        wxCloseEvent eventClose;
        OnClose(eventClose);
        if (eventClose.GetSkipped()) event.Skip();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function End"));
}


#ifndef __WXMAC__
void CTaskBarIcon::OnRButtonDown(wxTaskBarIconEvent& WXUNUSED(event)) {
    m_bMouseButtonPressed = true;
}


void CTaskBarIcon::OnRButtonUp(wxTaskBarIconEvent& WXUNUSED(event)) {
    if (m_bMouseButtonPressed) {
        DisplayContextMenu();
        m_bMouseButtonPressed = false;
    }
}
#endif  // #ifndef __WXMAC__


#ifdef __WXMSW__

void CTaskBarIcon::OnShutdown(wxTaskBarIconExEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnShutdown - Function Begin"));

    wxCloseEvent eventClose;
    OnClose(eventClose);
    if (eventClose.GetSkipped()) event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnShutdown - Function End"));
}

void CTaskBarIcon::OnAppRestore(wxTaskBarIconExEvent&) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnAppRestore - Function Begin"));

    ResetTaskBar();
    wxGetApp().ShowInterface();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnAppRestore - Function End"));
}

#endif


void CTaskBarIcon::OnReloadSkin(CTaskbarEvent& WXUNUSED(event)) {
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

#ifdef __WXMAC__
    m_iconTaskBarNormal = pSkinAdvanced->GetApplicationIcon()->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_SYSTEM);
    m_iconTaskBarDisconnected = pSkinAdvanced->GetApplicationDisconnectedIcon()->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_SYSTEM);
    m_iconTaskBarSnooze = pSkinAdvanced->GetApplicationSnoozeIcon()->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_SYSTEM);

    // Ensure that m_pTaskBarIcon and m_pMacDockIcon use same copy of each icon.
    wxGetApp().GetMacDockIcon()->m_iconTaskBarNormal = m_iconTaskBarNormal;
    wxGetApp().GetMacDockIcon()->m_iconTaskBarDisconnected = m_iconTaskBarDisconnected;
    wxGetApp().GetMacDockIcon()->m_iconTaskBarSnooze = m_iconTaskBarSnooze;
#else
    m_iconTaskBarNormal = pSkinAdvanced->GetApplicationIcon()->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_NEAREST_LARGER);
    m_iconTaskBarDisconnected = pSkinAdvanced->GetApplicationDisconnectedIcon()->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_NEAREST_LARGER);
    m_iconTaskBarSnooze = pSkinAdvanced->GetApplicationSnoozeIcon()->GetIcon(GetBestIconSize(), wxIconBundle::FALLBACK_NEAREST_LARGER);
#endif
}


void CTaskBarIcon::FireReloadSkin() {
    CTaskbarEvent event(wxEVT_TASKBAR_RELOADSKIN, this);
    AddPendingEvent(event);
}


void CTaskBarIcon::ResetTaskBar() {
    SetIcon(m_iconTaskBarNormal);
}


wxSize CTaskBarIcon::GetBestIconSize() {
    wxSize size;

#ifdef _WIN32
    size = wxSize(wxSystemSettings::GetMetric(wxSYS_SMALLICON_X), wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y));
#elif defined(__WXMAC__)
    size = wxDefaultSize;
#else
    size = wxSize(16, 16);
#endif

    return size;
}


#ifdef __WXMAC__

// The mac version of WxWidgets will delete this menu when
//  done with it; we must not delete it.  See the comments
//  in wxTaskBarIcon::PopupMenu() and DoCreatePopupMenu()
//  in WxMac/src/mac/carbon/taskbar.cpp for details

// Overridables
wxMenu *CTaskBarIcon::CreatePopupMenu() {
    wxMenu *menu = BuildContextMenu();
    return menu;
}


// Override the standard wxTaskBarIcon::SetIcon() because we are only providing a
// 16x16 icon for the menubar, while the Dock needs a 128x128 icon.
// Rather than using an entire separate icon, overlay the Dock icon with a badge
// so we don't need additional Snooze and Disconnected icons for branding.
#if wxCHECK_VERSION(3,1,6)
bool CTaskBarIcon::SetIcon(const wxBitmapBundle& newIcon, const wxString& )
#else
bool CTaskBarIcon::SetIcon(const wxIcon& icon, const wxString& )
#endif
{
    wxImage macIcon;
#if wxDEBUG_LEVEL
    int err = noErr;
#endif
    int w, h, x, y;

#if wxCHECK_VERSION(3,1,6)
    wxIcon icon = newIcon.GetIcon(wxDefaultSize);
#endif

    if (m_iconType != wxTBI_DOCK) {
        if (wxGetApp().GetBOINCMGRHideMenuBarIcon()) {
            RemoveIcon();
            return true;
        }
        return (wxGetApp().GetMacDockIcon()->SetIcon(icon) && wxTaskBarIcon::SetIcon(icon));
    }

    if (icon.IsSameAs(m_iconCurrentIcon))
        return true;

    m_iconCurrentIcon = icon;

    if (m_iconTaskBarDisconnected.IsSameAs(icon))
        macIcon = wxImage(macdisconnectbadge);
    else if (m_iconTaskBarSnooze.IsSameAs(icon))
        macIcon = wxImage(macsnoozebadge);
    else {
#if wxDEBUG_LEVEL
        err =
#endif
        SetDockBadge(NULL);
        return true;
    }

    unsigned char* iconBuffer = macIcon.GetData();

    wxImage mac_badge_mask = wxImage(macbadgemask);
    unsigned char* maskBuffer = mac_badge_mask.GetData();
    h = macIcon.GetHeight();
    w = macIcon.GetWidth();
    wxASSERT(h == mac_badge_mask.GetHeight());
    wxASSERT(w == mac_badge_mask.GetWidth());

    void * maskedIconData = malloc(h*w*4);
    unsigned char * maskedIconDataChars = (unsigned char*)maskedIconData;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            *maskedIconDataChars++ = *iconBuffer++;
            *maskedIconDataChars++ = *iconBuffer++;
            *maskedIconDataChars++ = *iconBuffer++;
            *maskedIconDataChars++ = 255 - *maskBuffer;
            maskBuffer += 3;
        }
    }

    wxImage maskedIcon = wxImage(w, h);
    maskedIcon.SetDataRGBA((unsigned char*)maskedIconData);
    wxBitmap bmp = wxBitmap(maskedIcon);
    if (!bmp.IsOk()) {
        return false;
    }

    // Actually set the dock image
#if wxDEBUG_LEVEL
    err =
#endif
    SetDockBadge(&bmp);

#if wxDEBUG_LEVEL
    wxASSERT(err == 0);
#endif
    return true;
}

#endif  // __WXMAC__


void CTaskBarIcon::DisplayContextMenu() {
    wxMenu* pMenu = BuildContextMenu();
    PopupMenu(pMenu);
    delete pMenu;
}


wxMenu *CTaskBarIcon::BuildContextMenu() {
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxMenu*            pMenu = new wxMenu;
    wxString           menuName = wxEmptyString;
    ACCT_MGR_INFO      ami;
    bool               is_acct_mgr_detected = false;

    wxASSERT(pMenu);
    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    // Prevent recursive entry of CMainDocument::RequestRPC()
    if (!pDoc->WaitingForRPC()) {
        if (pDoc->IsConnected()) {
            // Account managers have a different menu arrangement
            pDoc->rpc.acct_mgr_info(ami);
            is_acct_mgr_detected = ami.acct_mgr_url.size() ? true : false;
        }
    }

    if (is_acct_mgr_detected) {
        menuName.Printf(
            _("Open %s Web..."),
            pSkinAdvanced->GetApplicationShortName().c_str()
        );
        pMenu->Append(ID_OPENWEBSITE, menuName, wxEmptyString);
    }

    menuName.Printf(
        _("Open %s..."),
        pSkinAdvanced->GetApplicationName().c_str()
    );
    pMenu->Append(ID_OPENBOINCMANAGER, menuName, wxEmptyString);

    pMenu->AppendSeparator();

    m_SnoozeMenuItem = pMenu->AppendCheckItem(ID_TB_SUSPEND, _("Snooze"), wxEmptyString);
    if (pDoc->state.have_gpu()) {
        m_SnoozeGPUMenuItem = pMenu->AppendCheckItem(ID_TB_SUSPEND_GPU, _("Snooze GPU"), wxEmptyString);
    }

    pMenu->AppendSeparator();

    menuName.Printf(
        _("&About %s..."),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    pMenu->Append(wxID_ABOUT, menuName, wxEmptyString);

#ifdef __WXMAC__
    // These should be in Windows Task Bar Menu but not in Mac's Dock menu
    if (m_iconType != wxTBI_DOCK)
#endif
    {
        pMenu->AppendSeparator();
        pMenu->Append(wxID_EXIT, _("E&xit"), wxEmptyString);
    }

    AdjustMenuItems(pMenu);

    return pMenu;
}


void CTaskBarIcon::AdjustMenuItems(wxMenu* pMenu) {
    CC_STATUS      status;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxMenuItem*    pMenuItem = NULL;
    wxFont         font = wxNullFont;
    size_t         loc = 0;
    bool           is_dialog_detected = false;
    bool           enableSnoozeItems = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // BOINC Manager crashes if user selects "Exit" from taskbar menu while
    //  a dialog is open, so we must disable the "Exit" menu item if a dialog
    //  is open.
    // On the Mac, the user can open multiple instances of the About dialog
    //  by repeatedly selecting "About" menu item from the taskbar, so we
    //  must also disable that item.  For consistency with the Mac standard,
    //  we disable the entire taskbar menu when a modal dialog is open.
    if (wxGetApp().IsModalDialogDisplayed()) {
        is_dialog_detected = true;
    }

    enableSnoozeItems = (!is_dialog_detected) && pDoc->IsConnected();

    for (loc = 0; loc < pMenu->GetMenuItemCount(); loc++) {
        pMenuItem = pMenu->FindItemByPosition(loc);
        if (is_dialog_detected && (pMenuItem->GetId() != wxID_OPEN)) {
            pMenuItem->Enable(false);
        } else {
            pMenuItem->Enable(!(pMenuItem->IsSeparator()));
        }
    }

#ifdef __WXMSW__
    // Weird things happen with menus and wxWidgets on Windows when you try
    //   to change the font and use the system default as the baseline, so
    //   instead of fighting the system get the original font and tweak it
    //   a bit. It shouldn't hurt other platforms.
    for (loc = 0; loc < pMenu->GetMenuItemCount(); loc++) {
        pMenuItem = pMenu->FindItemByPosition(loc);
        pMenu->Remove(pMenuItem);

        font = pMenuItem->GetFont();
        font.SetPointSize(8);
        if (pMenuItem->GetId() != ID_OPENBOINCMANAGER) {
            font.SetWeight(wxFONTWEIGHT_NORMAL);
        } else {
            font.SetWeight(wxFONTWEIGHT_BOLD);
        }
        pMenuItem->SetFont(font);

        pMenu->Insert(loc, pMenuItem);
    }
#endif

    // Prevent recursive entry of CMainDocument::RequestRPC()
    if (pDoc->WaitingForRPC()) return;

    pDoc->GetCoreClientStatus(status);
    switch (status.task_mode) {
    case RUN_MODE_NEVER:
        switch (status.task_mode_perm) {
        case RUN_MODE_NEVER:
            m_SnoozeMenuItem->SetItemLabel(_("Resume"));
            m_SnoozeMenuItem->Check(false);
            m_SnoozeMenuItem->Enable(enableSnoozeItems);
            break;
        default:
            m_SnoozeMenuItem->SetItemLabel(_("Snooze"));
            m_SnoozeMenuItem->Check(true);
        }
        break;
    default:
        m_SnoozeMenuItem->SetItemLabel(_("Snooze"));
        m_SnoozeMenuItem->Check(false);
    }
    m_SnoozeMenuItem->Enable(enableSnoozeItems);

    if (pDoc->state.have_gpu()) {
        switch (status.gpu_mode) {
        case RUN_MODE_NEVER:
            switch (status.gpu_mode_perm) {
            case RUN_MODE_NEVER:
                m_SnoozeGPUMenuItem->SetItemLabel(_("Resume GPU"));
                m_SnoozeGPUMenuItem->Check(false);
                break;
            default:
                m_SnoozeGPUMenuItem->SetItemLabel(_("Snooze GPU"));
                m_SnoozeGPUMenuItem->Check(true);
            }
            break;
        default:
            m_SnoozeGPUMenuItem->SetItemLabel(_("Snooze GPU"));
            m_SnoozeGPUMenuItem->Check(false);
            break;
        }
        if (status.task_mode == RUN_MODE_NEVER) {
            bool check_gpu_snooze = false;
            if (status.task_mode_perm != RUN_MODE_NEVER) check_gpu_snooze = true;
            if ((status.gpu_mode == RUN_MODE_NEVER) && (status.gpu_mode_perm != RUN_MODE_NEVER)) {
                check_gpu_snooze = true;
            }
            m_SnoozeGPUMenuItem->Check(check_gpu_snooze);
            m_SnoozeGPUMenuItem->Enable(false);
        } else {
           m_SnoozeGPUMenuItem->Enable(enableSnoozeItems);
        }
    }
}


void CTaskBarIcon::UpdateTaskbarStatus() {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::UpdateTaskbarStatus - Function Begin"));

    CMainDocument* pDoc                 = wxGetApp().GetDocument();
    CC_STATUS      status;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->GetCoreClientStatus(status);

#ifdef __WXMAC__    // Mac Taskbar Icon does not support tooltips
    // Which icon should be displayed?
    if (!pDoc->IsConnected()) {
        SetIcon(m_iconTaskBarDisconnected);
    } else {
        switch(status.task_suspend_reason) {
        case SUSPEND_REASON_CPU_THROTTLE:
        case 0:
            SetIcon(m_iconTaskBarNormal);
            break;
        default:
            SetIcon(m_iconTaskBarSnooze);
        }
    }
#else
    wxString       strMachineName       = wxEmptyString;
    wxString       strMessage           = wxEmptyString;
    wxIcon         icnIcon;

    pDoc->GetConnectedComputerName(strMachineName);

    if (!pDoc->IsComputerNameLocal(strMachineName)) {
        strMessage += strMachineName;
        strMessage += wxT("\n");
    }

    if (pDoc->IsConnected()) {
        icnIcon = m_iconTaskBarNormal;
        bool comp_suspended = false;
        switch(status.task_suspend_reason) {
        case SUSPEND_REASON_CPU_THROTTLE:
        case 0:
            strMessage += _("Computing is enabled");
            break;
        default:
            icnIcon = m_iconTaskBarSnooze;
            strMessage += _("Computing is suspended - ");
            strMessage += suspend_reason_wxstring(status.task_suspend_reason);
            comp_suspended = true;
            break;
        }
        strMessage += wxT(".\n");

        if (!comp_suspended && pDoc->state.have_gpu()) {
            switch(status.gpu_suspend_reason) {
            case 0:
                strMessage += _("GPU computing is enabled");
                break;
            default:
                strMessage += _("GPU computing is suspended - ");
                strMessage += suspend_reason_wxstring(status.gpu_suspend_reason);
                break;
            }
            strMessage += wxT(".\n");
        }

        switch(status.network_suspend_reason) {
        case 0:
            strMessage += _("Network is enabled");
            break;
        default:
            strMessage += _("Network is suspended - ");
            strMessage += suspend_reason_wxstring(status.network_suspend_reason);
            break;
        }
        strMessage += wxT(".\n");
    } else {
        icnIcon = m_iconTaskBarDisconnected;
        if (pDoc->IsReconnecting()) {
            strMessage += _("Reconnecting to client.");
        } else {
            strMessage += _("Not connected to a client.");
        }
    }

    strMessage.Trim();
    SetIcon(icnIcon, strMessage);
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::UpdateTaskbarStatus - Function End"));
}


void CTaskBarIcon::UpdateNoticeStatus() {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::UpdateNoticeStatus - Function Begin"));

    CMainDocument*   pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*   pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString         strTitle;

    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    if (!pFrame) return;

    // Repeat notification for unread notices at user-selected reminder frequency
    wxTimeSpan tsLastNotificationDisplayed = wxDateTime::Now() - m_dtLastNotificationAlertExecuted;
    if (
        (tsLastNotificationDisplayed.GetMinutes() >= pFrame->GetReminderFrequency())
        && (pFrame->GetReminderFrequency() != 0)
    ) {

        if (pDoc->GetUnreadNoticeCount()
            && (pDoc->GetUnreadNoticeCount() != m_iLastNotificationUnreadMessageCount)
        ) {
#ifdef __WXMAC__
            // Delay notification while user is inactive
            // NOTE: This API requires OS 10.4 or later
            double idleTime = CGEventSourceSecondsSinceLastEventType (
                                kCGEventSourceStateCombinedSessionState,
                                kCGAnyInputEventType
                                );
            if (idleTime > MIN_IDLE_TIME_FOR_NOTIFICATION) return;
#endif
            // Update cached info
            m_dtLastNotificationAlertExecuted = wxDateTime::Now();
            m_iLastNotificationUnreadMessageCount = pDoc->GetUnreadNoticeCount();

            if (IsBalloonsSupported()) {
                // Display balloon
                strTitle.Printf(
                    _("%s Notices"),
                    pSkinAdvanced->GetApplicationName().c_str()
                );
                QueueBalloon(
                    m_iconTaskBarNormal,
                    strTitle,
                    _("There are new notices - click to view."),
                    BALLOONTYPE_INFO
                );
#ifdef __WXMAC__
            } else {
                // For platforms that do not support balloons
                // If Manager is hidden or in background, request user attention.
                if (! (wxGetApp().IsActive())) {
                    MacRequestUserAttention();  // Bounce BOINC Dock icon
                }
#else
                pFrame->RequestUserAttention();
#endif
            }
        }
#ifdef __WXMAC__
    } else {
        // Stop bouncing BOINC Dock icon after MAX_NOTIFICATION_DURATION seconds
        if (m_pNotificationRequest) {
            if (wxGetApp().IsActive() ||
                (tsLastNotificationDisplayed.GetSeconds() >= MAX_NOTIFICATION_DURATION)
            ) {
                MacCancelUserAttentionRequest();
            }
        }
#endif
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::UpdateNoticeStatus - Function End"));
}
