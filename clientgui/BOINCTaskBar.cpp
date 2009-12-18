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
#include "Events.h"

#ifdef __WXMAC__
#include "res/macsnoozebadge.xpm"
#include "res/macdisconnectbadge.xpm"
#include "res/macbadgemask.xpm"
#endif


DEFINE_EVENT_TYPE(wxEVT_TASKBAR_RELOADSKIN)
DEFINE_EVENT_TYPE(wxEVT_TASKBAR_REFRESH)

BEGIN_EVENT_TABLE(CTaskBarIcon, wxTaskBarIconEx)
    EVT_IDLE(CTaskBarIcon::OnIdle)
    EVT_CLOSE(CTaskBarIcon::OnClose)
    EVT_TASKBAR_REFRESH(CTaskBarIcon::OnRefresh)
    EVT_TASKBAR_RELOADSKIN(CTaskBarIcon::OnReloadSkin)
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLButtonDClick)
    EVT_MENU(wxID_OPEN, CTaskBarIcon::OnOpen)
    EVT_MENU(ID_OPENWEBSITE, CTaskBarIcon::OnOpenWebsite)
    EVT_MENU(ID_TB_SUSPEND, CTaskBarIcon::OnSuspendResume)
    EVT_MENU(ID_TB_SUSPEND_GPU, CTaskBarIcon::OnSuspendResumeGPU)
    EVT_MENU(wxID_ABOUT, CTaskBarIcon::OnAbout)
    EVT_MENU(wxID_EXIT, CTaskBarIcon::OnExit)

#ifdef __WXMSW__
    EVT_TASKBAR_SHUTDOWN(CTaskBarIcon::OnShutdown)
    EVT_TASKBAR_MOVE(CTaskBarIcon::OnMouseMove)
    EVT_TASKBAR_CONTEXT_MENU(CTaskBarIcon::OnContextMenu)
    EVT_TASKBAR_RIGHT_DOWN(CTaskBarIcon::OnRButtonDown)
    EVT_TASKBAR_RIGHT_UP(CTaskBarIcon::OnRButtonUp)
#endif
#ifdef __WXMAC__
    // wxMac-2.6.3 "helpfully" converts wxID_ABOUT to kHICommandAbout, wxID_EXIT to kHICommandQuit, 
    //  wxID_PREFERENCES to kHICommandPreferences
    EVT_MENU(kHICommandAbout, CTaskBarIcon::OnAbout)
#endif
END_EVENT_TABLE()


CTaskBarIcon::CTaskBarIcon(wxString title, wxIcon* icon, wxIcon* iconDisconnected, wxIcon* iconSnooze) : 
#if   defined(__WXMAC__)
    wxTaskBarIcon(DOCK)
#elif defined(__WXMSW__)
    wxTaskBarIconEx(wxT("BOINCManagerSystray"))
#else
    wxTaskBarIcon()
#endif
{
    m_iconTaskBarNormal = *icon;
    m_iconTaskBarDisconnected = *iconDisconnected;
    m_iconTaskBarSnooze = *iconSnooze;
    m_strDefaultTitle = title;
    m_bTaskbarInitiatedShutdown = false;

    m_dtLastHoverDetected = wxDateTime((time_t)0);

    m_bMouseButtonPressed = false;
}


CTaskBarIcon::~CTaskBarIcon() {
    RemoveIcon();
}


void CTaskBarIcon::OnIdle(wxIdleEvent& event) {
    wxGetApp().UpdateSystemIdleDetection();
    event.Skip();
}


void CTaskBarIcon::OnClose(wxCloseEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function Begin"));

    RemoveIcon();
    m_bTaskbarInitiatedShutdown = true;

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        pFrame->Close(true);
    }

    event.Skip();
    
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function End"));
}


void CTaskBarIcon::OnRefresh(CTaskbarEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnRefresh - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CC_STATUS      status;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // What is the current status of the client?
    pDoc->GetCoreClientStatus(status);

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

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnRefresh - Function End"));
}


void CTaskBarIcon::OnLButtonDClick(wxTaskBarIconEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnLButtonDClick - Function Begin"));

    wxCommandEvent eventCommand;
    OnOpen(eventCommand);
    if (eventCommand.GetSkipped()) event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnLButtonDClick - Function End"));
}


void CTaskBarIcon::OnOpen(wxCommandEvent& WXUNUSED(event)) {
    ResetTaskBar();

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

    if (pFrame) {
        pFrame->Show();

#ifdef __WXMAC__
        if (pFrame->IsIconized()) {
            pFrame->Iconize(false);
        }
#else
        if (pFrame->IsMaximized()) {
            pFrame->Maximize(true);
        } else {
            pFrame->Maximize(false);
        }
#endif
        pFrame->SendSizeEvent();
        pFrame->Update();

#ifdef __WXMSW__
        ::SetForegroundWindow((HWND)pFrame->GetHandle());
#endif
    }
#ifdef __WXMAC__
    else {
        wxGetApp().ShowCurrentGUI();
    }
#endif
}


void CTaskBarIcon::OnOpenWebsite(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnOpenWebsite - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame*   pFrame = wxGetApp().GetFrame();
    ACCT_MGR_INFO      ami;
    wxString           url;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

    ResetTaskBar();

    pDoc->rpc.acct_mgr_info(ami);
    url = wxString(ami.acct_mgr_url.c_str(), wxConvUTF8);
    pFrame->ExecuteBrowserLink(url);

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
    if (status.task_mode_perm != status.task_mode) {
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
    if (status.gpu_mode_perm != status.gpu_mode) {
        pDoc->SetGPURunMode(RUN_MODE_RESTORE, 0);
    } else {
        pDoc->SetGPURunMode(RUN_MODE_NEVER, 3600);
    }
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnSuspendResumeGPU - Function End"));
}
void CTaskBarIcon::OnAbout(wxCommandEvent& WXUNUSED(event)) {
    bool bWasVisible;

    bWasVisible = wxGetApp().IsApplicationVisible();
    wxGetApp().ShowApplication(true);

    ResetTaskBar();

    CDlgAbout dlg(NULL);
    dlg.ShowModal();

    if (!bWasVisible) {
        wxGetApp().ShowApplication(false);
    }
}


void CTaskBarIcon::OnExit(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function Begin"));

#ifndef __WXMAC__
    if (wxGetApp().ConfirmExit()) 
#endif
    {
#ifdef __WXMSW__
        CMainDocument* pDoc = wxGetApp().GetDocument();

        wxASSERT(pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));

        if (wxGetApp().ShouldShutdownCoreClient()) {
            pDoc->m_pClientManager->EnableBOINCStartedByManager();
        } else {
            pDoc->m_pClientManager->DisableBOINCStartedByManager();
        }
#endif

        wxCloseEvent eventClose;
        OnClose(eventClose);
        if (eventClose.GetSkipped()) event.Skip();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function End"));
}


#ifdef __WXMSW__
void CTaskBarIcon::OnShutdown(wxTaskBarIconExEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnShutdown - Function Begin"));

    wxCloseEvent eventClose;
    OnClose(eventClose);
    if (eventClose.GetSkipped()) event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnShutdown - Function End"));
}


// Note: tooltip must not have a trailing linebreak. 
void CTaskBarIcon::OnMouseMove(wxTaskBarIconEvent& WXUNUSED(event)) {

    wxTimeSpan tsLastHover(wxDateTime::Now() - m_dtLastHoverDetected);
    if (tsLastHover.GetSeconds() >= 2) {
        m_dtLastHoverDetected = wxDateTime::Now();

        CMainDocument* pDoc                 = wxGetApp().GetDocument();
       wxString       strMachineName       = wxEmptyString;
        wxString       strMessage           = wxEmptyString;
        wxString       strProjectName       = wxEmptyString;
        wxString       strBuffer            = wxEmptyString;
        wxString       strActiveTaskBuffer  = wxEmptyString;
        float          fProgress            = 0;
        bool           bIsActive            = false;
        bool           bIsExecuting         = false;
        bool           bIsDownloaded        = false;
        wxInt32        iResultCount         = 0;
        wxInt32        iActiveTaskCount     = 0;
        wxInt32        iIndex               = 0;
        CC_STATUS      status;

        if (!pDoc) return;

        if (pDoc->IsConnected()) {
            pDoc->GetConnectedComputerName(strMachineName);

            // Only show machine name if connected to remote machine.
            if (!pDoc->IsComputerNameLocal(strMachineName)) {
                strMessage += strMachineName;
            }

            pDoc->GetCoreClientStatus(status);
            if (status.task_suspend_reason && !(status.task_suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT)) {
                strBuffer.Printf(
                    _("Computation is suspended.")
                );
                if (strMessage.Length() > 0) strMessage += wxT("\n");
                strMessage += strBuffer;
            }

            if (status.network_suspend_reason && !(status.network_suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT)) {
                strBuffer.Printf(
                    _("Network activity is suspended.")
                );
                if (strMessage.Length() > 0) strMessage += wxT("\n");
                strMessage += strBuffer;
            }

            iResultCount = pDoc->GetWorkCount();
            for (iIndex = 0; iIndex < iResultCount; iIndex++) {
                RESULT* result = pDoc->result(iIndex);
                RESULT* state_result = NULL;
                std::string project_name;

                bIsDownloaded = (result->state == RESULT_FILES_DOWNLOADED);
                bIsActive     = result->active_task;
                bIsExecuting  = (result->scheduler_state == CPU_SCHED_SCHEDULED);
                if (!(bIsActive) || !(bIsDownloaded) || !(bIsExecuting)) continue;

                // Increment the active task counter
                iActiveTaskCount++;

                // If we have more then two active tasks then we'll just be displaying
                //   the total number of active tasks anyway, so just look at the rest
                //   of the result records.
                if (iActiveTaskCount > 2) continue;

                if (result) {
                    state_result = pDoc->state.lookup_result(result->project_url, result->name);
                    if (state_result) {
                        state_result->project->get_name(project_name);
                        strProjectName = wxString(project_name.c_str(), wxConvUTF8);
                    }
                    fProgress = floor(result->fraction_done*10000)/100;
                }

                strBuffer.Printf(_("%s: %.2f%% completed."), strProjectName.c_str(), fProgress );
                if (strActiveTaskBuffer.Length() > 0) strActiveTaskBuffer += wxT("\n");
                strActiveTaskBuffer += strBuffer;
            }

            if (iActiveTaskCount <= 2) {
                if (strMessage.Length() > 0) strMessage += wxT("\n");
                strMessage += strActiveTaskBuffer;
            } else {
                // More than two active tasks are running on the system, we don't have
                //   enough room to display them all, so just tell the user how many are
                //   currently running.
                strBuffer.Printf(
                    _("%d tasks running."),
                    iActiveTaskCount
                );
                if (strMessage.Length() > 0) strMessage += wxT("\n");
                strMessage += strBuffer;
            }

        } else if (pDoc->IsReconnecting()) {
            strBuffer.Printf(
                _("Reconnecting to client.")
            );
            if (strMessage.Length() > 0) strMessage += wxT("\n");
            strMessage += strBuffer;
        } else {
            strBuffer.Printf(
                _("Not connected to a client.")
            );
            if (strMessage.Length() > 0) strMessage += wxT("\n");
            strMessage += strBuffer;
        }

        SetTooltip(strMessage);
    }
}


void CTaskBarIcon::OnContextMenu(wxTaskBarIconExEvent& WXUNUSED(event)) {
    DisplayContextMenu();
}


void CTaskBarIcon::OnRButtonDown(wxTaskBarIconEvent& WXUNUSED(event)) {
    if (!IsBalloonsSupported()) {
        m_bMouseButtonPressed = true;
    }
}


void CTaskBarIcon::OnRButtonUp(wxTaskBarIconEvent& WXUNUSED(event)) {
    if (!IsBalloonsSupported()) {
        if (m_bMouseButtonPressed) {
            DisplayContextMenu();
            m_bMouseButtonPressed = false;
        }
    }
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
    wxGetApp().GetMacSystemMenu()->BuildMenu();
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
bool CTaskBarIcon::SetIcon(const wxIcon& icon) {
    wxIcon macIcon;
    bool result;
    OSStatus err = noErr ;
    static const wxIcon* currentIcon = NULL;
    int w, h, x, y;

    if (&icon == currentIcon)
        return true;
    
    currentIcon = &icon;
    
    CMacSystemMenu* sysMenu = wxGetApp().GetMacSystemMenu();
    if (sysMenu == NULL) return 0;
    
    result = sysMenu->SetIcon(icon);

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
    pMenu->Append(wxID_OPEN, menuName, wxEmptyString);

    pMenu->AppendSeparator();

    pMenu->AppendCheckItem(ID_TB_SUSPEND, _("Snooze"), wxEmptyString);
    if (pDoc->state.have_cuda || pDoc->state.have_ati) {
        pMenu->AppendCheckItem(ID_TB_SUSPEND_GPU, _("Snooze GPU"), wxEmptyString);
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
            if (pMenuItem->GetId() != wxID_OPEN) {
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
            pMenu->Check(ID_TB_SUSPEND, false);
            pMenu->Enable(ID_TB_SUSPEND, false);
            break;
        default:
            pMenu->Check(ID_TB_SUSPEND, true);
            if (!is_dialog_detected) {
                pMenu->Enable(ID_TB_SUSPEND, true);
            }
        }
        if (pDoc->state.have_cuda || pDoc->state.have_ati) {
            pMenu->Check(ID_TB_SUSPEND_GPU, false);
            pMenu->Enable(ID_TB_SUSPEND_GPU, false);
        }
        break;
    default:
        pMenu->Check(ID_TB_SUSPEND, false);
        if (!is_dialog_detected) {
            pMenu->Enable(ID_TB_SUSPEND, true);
        }
        if (pDoc->state.have_cuda || pDoc->state.have_ati) {
            switch (status.gpu_mode) {
            case RUN_MODE_NEVER:
                switch (status.gpu_mode_perm) {
                case RUN_MODE_NEVER:
                    pMenu->Check(ID_TB_SUSPEND_GPU, false);
                    pMenu->Enable(ID_TB_SUSPEND_GPU, false);
                    break;
                default:
                    pMenu->Check(ID_TB_SUSPEND_GPU, true);
                    if (!is_dialog_detected) {
                        pMenu->Enable(ID_TB_SUSPEND_GPU, true);
                    }
                }
                break;
            default:
                pMenu->Check(ID_TB_SUSPEND_GPU, false);
                if (!is_dialog_detected) {
                    pMenu->Enable(ID_TB_SUSPEND_GPU, true);
                }
                break;
            }
        }
    }
}

const char *BOINC_RCSID_531575eeaa = "$Id$";
