// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
    EVT_TASKBAR_BALLOON_TIMEOUT(CTaskBarIcon::OnNotificationTimeout)
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

#ifdef __WXMAC__
    // wxMac-2.6.3 "helpfully" converts wxID_ABOUT to kHICommandAbout, wxID_EXIT to kHICommandQuit, 
    //  wxID_PREFERENCES to kHICommandPreferences
    EVT_MENU(kHICommandAbout, CTaskBarIcon::OnAbout)
#endif

END_EVENT_TABLE()


CTaskBarIcon::CTaskBarIcon(wxString title, wxIcon* icon, wxIcon* iconDisconnected, wxIcon* iconSnooze) : 
#ifdef __WXMAC__
    wxTaskBarIcon(DOCK)
#else 
    wxTaskBarIconEx(wxT("BOINCManagerSystray"), 1)
#endif
{
    m_iconTaskBarNormal = *icon;
    m_iconTaskBarDisconnected = *iconDisconnected;
    m_iconTaskBarSnooze = *iconSnooze;
    m_SnoozeGPUMenuItem = NULL;

    m_bTaskbarInitiatedShutdown = false;

    m_bMouseButtonPressed = false;

    m_dtLastNotificationAlertExecuted = wxDateTime((time_t)0);
    m_iLastNotificationUnreadMessageCount = 0;
#ifdef __WXMAC__
    m_pNotificationRequest = NULL;
#endif
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
#endif
    
    wxGetApp().ShowApplication(true);

    ResetTaskBar();

    CDlgAbout dlg(NULL);
    dlg.ShowModal();

    if (!bWasVisible) {
        wxGetApp().ShowApplication(false);
    }
    
#ifdef __WXMAC__
    if (bEventLogWasShown) eventLog->Show(true);
#endif
}


void CTaskBarIcon::OnExit(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function Begin"));

#ifndef __WXMAC__
    if (wxGetApp().ConfirmExit()) 
#endif
    {
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

    m_iconTaskBarNormal = *pSkinAdvanced->GetApplicationIcon();
    m_iconTaskBarDisconnected = *pSkinAdvanced->GetApplicationDisconnectedIcon();
    m_iconTaskBarSnooze = *pSkinAdvanced->GetApplicationSnoozeIcon();

#ifdef __WXMAC__
    // For unknown reasons, menus won't work if we call BuildMenu() here 
    wxGetApp().GetMacSystemMenu()->SetNeedToRebuildMenu();
#endif
}


void CTaskBarIcon::FireReloadSkin() {
    CTaskbarEvent event(wxEVT_TASKBAR_RELOADSKIN, this);
    AddPendingEvent(event);
}


void CTaskBarIcon::ResetTaskBar() {
    SetIcon(m_iconTaskBarNormal);
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
bool CTaskBarIcon::SetIcon(const wxIcon& icon, const wxString& ) {
    CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
    if (pTaskbar) {
        return pTaskbar->SetMacTaskBarIcon(icon);
    }
    return false;
}


bool CTaskBarIcon::SetMacTaskBarIcon(const wxIcon& icon) {
    wxIcon macIcon;
    bool result;
    OSStatus err = noErr ;
    int w, h, x, y;

    if (icon.IsSameAs(m_iconCurrentIcon))
        return true;
    
    m_iconCurrentIcon = icon;
    
    CMacSystemMenu* sysMenu = wxGetApp().GetMacSystemMenu();
    if (sysMenu == NULL) return 0;
    
    result = sysMenu->SetMacMenuIcon(icon);

    RestoreApplicationDockTileImage();      // Remove any previous badge

    if (m_iconTaskBarDisconnected.IsSameAs(icon))
        macIcon = macdisconnectbadge;
    else if (m_iconTaskBarSnooze.IsSameAs(icon))
        macIcon = macsnoozebadge;
    else
        return result;
    
    // Convert the wxIcon into a wxBitmap so we can perform some
    // wxBitmap operations with it
    wxBitmap bmp( macIcon ) ;
    
    // wxMac's XMP image format always uses 32-bit pixels but allows only 
    // 1-bit masks, so we use a separate XMP file for the 8-bit mask to 
    // allow us to do proper anti-aliasing of the badges.  This code assumes 
    // that all badges are the same size circle and at the same position so 
    // that they can share a single mask.
    wxBitmap mask_bmp( macbadgemask ) ;
    h = bmp.GetHeight();
    w = bmp.GetWidth();

    wxASSERT(h == mask_bmp.GetHeight());
    wxASSERT(w == mask_bmp.GetWidth());

    unsigned char * iconBuffer = (unsigned char *)bmp.GetRawAccess();
    unsigned char * maskBuffer = (unsigned char *)mask_bmp.GetRawAccess() + 1;

    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            *iconBuffer = 255 - *maskBuffer;
            iconBuffer += 4;
            maskBuffer += 4;
        }
    }

    CGImageRef pImage = (CGImageRef) bmp.CGImageCreate(); 
    
    // Actually set the dock image    
    err = OverlayApplicationDockTileImage(pImage);
    
    wxASSERT(err == 0);
    
    // Free the CGImage
    if (pImage != NULL)
        CGImageRelease(pImage);

    return result;
}


// wxTopLevel::RequestUserAttention() doesn't have an API to cancel 
// after a timeout, so we must call Notification Manager directly on Mac
void CTaskBarIcon::MacRequestUserAttention()
{
    m_pNotificationRequest = (NMRecPtr) NewPtrClear( sizeof( NMRec) ) ;
    m_pNotificationRequest->qType = nmType ;
    m_pNotificationRequest->nmMark = 1;

    NMInstall(m_pNotificationRequest);
}

void CTaskBarIcon::MacCancelUserAttentionRequest()
{
    if (m_pNotificationRequest) {
        NMRemove(m_pNotificationRequest);
        DisposePtr((Ptr)m_pNotificationRequest);
        m_pNotificationRequest = NULL;
    }
}

#endif  // ! __WXMAC__


void CTaskBarIcon::DisplayContextMenu() {
    ResetTaskBar();

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
        // Account managers have a different menu arrangement
        pDoc->rpc.acct_mgr_info(ami);
        is_acct_mgr_detected = ami.acct_mgr_url.size() ? true : false;
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
    if (pDoc->state.have_nvidia || pDoc->state.have_ati) {
        m_SnoozeGPUMenuItem = pMenu->AppendCheckItem(ID_TB_SUSPEND_GPU, _("Snooze GPU"), wxEmptyString);
    }

    pMenu->AppendSeparator();

    menuName.Printf(
        _("&About %s..."),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    pMenu->Append(wxID_ABOUT, menuName, wxEmptyString);

#ifndef __WXMAC__
    // These should be in Windows Task Bar Menu but not in Mac's Dock menu
    pMenu->AppendSeparator();

    pMenu->Append(wxID_EXIT, _("E&xit"), wxEmptyString);
#endif

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
        
    for (loc = 0; loc < pMenu->GetMenuItemCount(); loc++) {
        pMenuItem = pMenu->FindItemByPosition(loc);
        if (is_dialog_detected && (pMenuItem->GetId() != wxID_OPEN)) {
            pMenuItem->Enable(false);
        } else {
            pMenuItem->Enable(!(pMenuItem->IsSeparator()));
        }
    }

#ifdef __WXMSW__
    // Wierd things happen with menus and wxWidgets on Windows when you try
    //   to change the font and use the system default as the baseline, so 
    //   instead of fighting the system get the original font and tweak it 
    //   a bit. It shouldn't hurt other platforms.
    for (loc = 0; loc < pMenu->GetMenuItemCount(); loc++) {
        pMenuItem = pMenu->FindItemByPosition(loc);
        if (!pMenuItem->IsSeparator() && pMenuItem->IsEnabled()) {
            pMenu->Remove(pMenuItem);

            font = pMenuItem->GetFont();
            if (pMenuItem->GetId() != ID_OPENBOINCMANAGER) {
                font.SetWeight(wxFONTWEIGHT_NORMAL);
            } else {
                font.SetWeight(wxFONTWEIGHT_BOLD);
            }
            pMenuItem->SetFont(font);

            pMenu->Insert(loc, pMenuItem);
        }
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
            if (!is_dialog_detected) {
                m_SnoozeMenuItem->Enable(true);
            }
            break;
        default:
            m_SnoozeMenuItem->SetItemLabel(_("Snooze"));
            m_SnoozeMenuItem->Check(true);
            if (!is_dialog_detected) {
                m_SnoozeMenuItem->Enable(true);
            }
        }
        break;
    default:
        m_SnoozeMenuItem->SetItemLabel(_("Snooze"));
        m_SnoozeMenuItem->Check(false);
        if (!is_dialog_detected) {
            m_SnoozeMenuItem->Enable(true);
        }
    }
    
    if (pDoc->state.have_nvidia || pDoc->state.have_ati) {
        switch (status.gpu_mode) {
        case RUN_MODE_NEVER:
            switch (status.gpu_mode_perm) {
            case RUN_MODE_NEVER:
                m_SnoozeGPUMenuItem->SetItemLabel(_("Resume GPU"));
                m_SnoozeGPUMenuItem->Check(false);
                if (!is_dialog_detected) {
                    m_SnoozeGPUMenuItem->Enable(true);
                }
                break;
            default:
                m_SnoozeGPUMenuItem->SetItemLabel(_("Snooze GPU"));
                m_SnoozeGPUMenuItem->Check(true);
                if (!is_dialog_detected) {
                    m_SnoozeGPUMenuItem->Enable(true);
                }
            }
            break;
        default:
            m_SnoozeGPUMenuItem->SetItemLabel(_("Snooze GPU"));
            m_SnoozeGPUMenuItem->Check(false);
            if (!is_dialog_detected) {
                m_SnoozeGPUMenuItem->Enable(true);
            }
            break;
        }
        if (pDoc->state.have_nvidia || pDoc->state.have_ati) {
            if (status.task_mode == RUN_MODE_NEVER) {
                m_SnoozeGPUMenuItem->Check(false);
                m_SnoozeGPUMenuItem->Enable(false);
            }
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
        if (RUN_MODE_NEVER == status.task_mode) {
            SetIcon(m_iconTaskBarSnooze);
        } else {
            SetIcon(m_iconTaskBarNormal);
        }
    }
#else
    wxString       strMachineName       = wxEmptyString;
    wxString       strMessage           = wxEmptyString;
    wxString       strBuffer            = wxEmptyString;
    wxIcon         icnIcon;

    pDoc->GetConnectedComputerName(strMachineName);

    if (!pDoc->IsComputerNameLocal(strMachineName)) {
        strMessage += strMachineName;
        strMessage += wxT("\n");
    }

    if (pDoc->IsConnected()) {
        icnIcon = m_iconTaskBarNormal;
        if (RUN_MODE_NEVER == status.task_mode) {
            icnIcon = m_iconTaskBarSnooze;
        }
        bool comp_suspended = false;
        switch(status.task_suspend_reason) {
        case SUSPEND_REASON_CPU_THROTTLE:
        case 0:
            strMessage += _("Computing is enabled");
            break;
        default:
            strMessage += _("Computing is suspended - ");
            strMessage += suspend_reason_wxstring(status.task_suspend_reason);
            comp_suspended = true;
            break;
        }
        strMessage += wxT(".\n");

        if (!comp_suspended && (pDoc->state.have_nvidia || pDoc->state.have_ati)) {
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
