// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCTaskBar.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCTaskBar.h"
#include "DlgAbout.h"
#include "Events.h"


BEGIN_EVENT_TABLE (CTaskBarIcon, wxTaskBarIconEx)
    EVT_IDLE(CTaskBarIcon::OnIdle)
    EVT_CLOSE(CTaskBarIcon::OnClose)
    EVT_TIMER(ID_TB_TIMER, CTaskBarIcon::OnTimer)
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLButtonDClick)
    EVT_MENU(wxID_OPEN, CTaskBarIcon::OnOpen)
    EVT_MENU(ID_OPENWEBSITE, CTaskBarIcon::OnOpenWebsite)
    EVT_MENU(ID_TB_SUSPEND, CTaskBarIcon::OnSuspend)
    EVT_MENU(wxID_ABOUT, CTaskBarIcon::OnAbout)
    EVT_MENU(wxID_EXIT, CTaskBarIcon::OnExit)

#ifdef __WXMSW__
    EVT_TASKBAR_SHUTDOWN(CTaskBarIcon::OnShutdown)
    EVT_TASKBAR_MOVE(CTaskBarIcon::OnMouseMove)
    EVT_TASKBAR_CONTEXT_MENU(CTaskBarIcon::OnContextMenu)
    EVT_TASKBAR_RIGHT_DOWN(CTaskBarIcon::OnRButtonDown)
    EVT_TASKBAR_RIGHT_UP(CTaskBarIcon::OnRButtonUp)
#endif
END_EVENT_TABLE ()


CTaskBarIcon::CTaskBarIcon(wxString title, wxIcon* icon) : 
#if   defined(__WXMAC__)
    wxTaskBarIcon(DOCK)
#elif defined(__WXMSW__)
    wxTaskBarIconEx(wxT("BOINCManagerSystray"))
#else
    wxTaskBarIcon()
#endif
{
    m_iconTaskBarIcon = *icon;
    m_dtLastHoverDetected = wxDateTime((time_t)0);
    m_dtLastBalloonDisplayed = wxDateTime((time_t)0);

    m_iPreviousActivityMode = RUN_MODE_AUTO;
    m_iPreviousNetworkMode = RUN_MODE_AUTO;

    m_pTimer = new wxTimer(this, ID_TB_TIMER);

#ifndef __WXMAC__
    SetIcon(m_iconTaskBarIcon, title);
#endif
}


CTaskBarIcon::~CTaskBarIcon() {
    RemoveIcon();

    if (m_pTimer) {
        m_pTimer->Stop();
        delete m_pTimer;
    }
}


void CTaskBarIcon::OnIdle(wxIdleEvent& event) {
    wxGetApp().UpdateSystemIdleDetection();
    event.Skip();
}


void CTaskBarIcon::OnClose(wxCloseEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function Begin"));

    ResetTaskBar();

    CMainFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame) {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        pFrame->Close(true);
    }

    event.Skip();
    
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnClose - Function End"));
}


void CTaskBarIcon::OnTimer(wxTimerEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnTimer - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    m_pTimer->Stop();
    pDoc->SetActivityRunMode(m_iPreviousActivityMode);
    pDoc->SetNetworkRunMode(m_iPreviousNetworkMode);

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnTimer - Function End"));
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

    CMainFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    if (pFrame) {
        pFrame->Show();
#ifndef __WXMAC__
        if (pFrame->IsMaximized()) {
            pFrame->Maximize(true);
        } else {
            pFrame->Maximize(false);
        }
#endif
        pFrame->SendSizeEvent();

#ifdef __WXMSW__
        ::SetForegroundWindow((HWND)pFrame->GetHandle());
#endif
	}
}


void CTaskBarIcon::OnOpenWebsite(wxCommandEvent& WXUNUSED(event)) {
    ResetTaskBar();

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CMainFrame*        pFrame = wxGetApp().GetFrame();
    ACCT_MGR_INFO      ami;
    wxString           url;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pDoc->rpc.acct_mgr_info(ami);

    url = ami.acct_mgr_url.c_str();

    pFrame->ExecuteBrowserLink(url);
}


void CTaskBarIcon::OnSuspend(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnSuspend - Function Begin"));
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    wxInt32        iActivityMode = -1;
    wxInt32        iNetworkMode  = -1;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    ResetTaskBar();

    pDoc->GetActivityRunMode(iActivityMode);
    pDoc->GetNetworkRunMode(iNetworkMode);

    if ((RUN_MODE_NEVER == iActivityMode) && (RUN_MODE_NEVER == iNetworkMode) &&
        (RUN_MODE_NEVER == m_iPreviousActivityMode) && (RUN_MODE_NEVER == m_iPreviousActivityMode)) {

        m_pTimer->Stop();

        pDoc->SetActivityRunMode(RUN_MODE_AUTO);
        pDoc->SetNetworkRunMode(RUN_MODE_AUTO);

    } else if ((RUN_MODE_NEVER == iActivityMode) && (RUN_MODE_NEVER == iNetworkMode)) {

        m_pTimer->Stop();

        pDoc->SetActivityRunMode(m_iPreviousActivityMode);
        pDoc->SetNetworkRunMode(m_iPreviousNetworkMode);

    } else {

        m_pTimer->Start(60*60*1000);  // Send event every 60 minutes

        m_iPreviousActivityMode = iActivityMode;
        m_iPreviousNetworkMode = iNetworkMode;

        pDoc->SetActivityRunMode(RUN_MODE_NEVER);
        pDoc->SetNetworkRunMode(RUN_MODE_NEVER);

    }

    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnSuspend - Function End"));
}


void CTaskBarIcon::OnAbout(wxCommandEvent& WXUNUSED(event)) {
#ifdef __WXMAC__
    ProcessSerialNumber psn;

    GetCurrentProcess(&psn);
    bool wasVisible = IsProcessVisible(&psn);
    SetFrontProcess(&psn);  // Shows process if hidden
#endif

    ResetTaskBar();

    CDlgAbout* pDlg = new CDlgAbout(NULL);
    wxASSERT(pDlg);

    pDlg->ShowModal();

    if (pDlg) {
        pDlg->Destroy();

#ifdef __WXMAC__
    if (! wasVisible)
        ShowHideProcess(&psn, false);
#endif
    }
}


void CTaskBarIcon::OnExit(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTaskBarIcon::OnExit - Function Begin"));

    wxCloseEvent eventClose;

    OnClose(eventClose);

    if (eventClose.GetSkipped()) event.Skip();

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


void CTaskBarIcon::OnMouseMove(wxTaskBarIconEvent& WXUNUSED(event)) {
    wxTimeSpan ts(wxDateTime::Now() - m_dtLastHoverDetected);
    if (ts.GetSeconds() >= 10)
        m_dtLastHoverDetected = wxDateTime::Now();

    wxTimeSpan tsLastHover(wxDateTime::Now() - m_dtLastHoverDetected);
    wxTimeSpan tsLastBalloon(wxDateTime::Now() - m_dtLastBalloonDisplayed);
    if ((tsLastHover.GetSeconds() >= 2) && (tsLastBalloon.GetSeconds() >= 10)) {
        m_dtLastBalloonDisplayed = wxDateTime::Now();

        wxString strTitle             = wxGetApp().GetBrand()->GetApplicationName();
        wxString strMachineName       = wxEmptyString;
        wxString strMessage           = wxEmptyString;
        wxString strBuffer            = wxEmptyString;
        wxString strProjectName       = wxEmptyString;
        float    fProgress            = 0;
        bool     bIsActive            = false;
        bool     bIsExecuting         = false;
        bool     bIsDownloaded        = false;
        wxInt32  iResultCount         = 0;
        wxInt32  iIndex               = 0;
        bool     bActivitiesSuspended = false;
        bool     bNetworkSuspended    = false;
        CMainDocument* pDoc           = wxGetApp().GetDocument();

        wxASSERT(pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));

        if (pDoc->IsConnected()) {
            pDoc->GetConnectedComputerName(strMachineName);
            strTitle = strTitle + wxT(" - (") + strMachineName + wxT(")");

            pDoc->GetActivityState(bActivitiesSuspended, bNetworkSuspended);
            if (bActivitiesSuspended) {
                // 1st %s is the previous instance of the message
                // 2nd %s is the project name
                //    i.e. 'BOINC', 'GridRepublic'
                strBuffer.Printf(
                    _("%s is currently suspended...\n"),
                    wxGetApp().GetBrand()->GetProjectName().c_str()
                );
                strMessage += strBuffer;
            }

            if (bNetworkSuspended) {
                // 1st %s is the previous instance of the message
                // 2nd %s is the project name
                //    i.e. 'BOINC', 'GridRepublic'
                strBuffer.Printf(
                    _("%s networking is currently suspended...\n"),
                    wxGetApp().GetBrand()->GetProjectName().c_str()
                );
                strMessage += strBuffer;
            }

            if (strMessage.Length() > 0) {
                strMessage += wxT("\n");
            }

            iResultCount = pDoc->results.results.size();
            for (iIndex = 0; iIndex < iResultCount; iIndex++) {
                RESULT* result = wxGetApp().GetDocument()->result(iIndex);
                RESULT* state_result = NULL;
                std::string project_name;

                bIsDownloaded = (result->state == RESULT_FILES_DOWNLOADED);
                bIsActive     = result->active_task;
                bIsExecuting  = (result->scheduler_state == CPU_SCHED_SCHEDULED);
                if (!(bIsActive) || !(bIsDownloaded) || !(bIsExecuting)) continue;

                if (result) {
                    state_result = pDoc->state.lookup_result(result->project_url, result->name);
                    if (state_result) {
                        state_result->project->get_name(project_name);
                        strProjectName = wxString(project_name.c_str());
                    }
                    fProgress = floor(result->fraction_done*10000)/100;
                }

                strBuffer.Printf(wxT("%s: %.2f%%\n"), strProjectName.c_str(), fProgress );
                strMessage += strBuffer;
            }
        } else if (pDoc->IsReconnecting()) {
            // 1st %s is the previous instance of the message
            // 2nd %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            // 3rd %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strBuffer.Printf(
                _("%s is currently reconnecting to a %s client...\n"),
                wxGetApp().GetBrand()->GetApplicationName().c_str(),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );
            strMessage += strBuffer;
        } else {
            // 1st %s is the previous instance of the message
            // 2nd %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            // 3rd %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strBuffer.Printf(
                _("%s is not currently connected to a %s client...\n"),
                wxGetApp().GetBrand()->GetApplicationName().c_str(),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );
            strMessage += strBuffer;
        }

        SetBalloon(m_iconTaskBarIcon, strTitle, strMessage);
    }
}

#endif // __WXMSW__


#ifndef __WXMAC__
#ifdef __WXMSW__
void CTaskBarIcon::OnContextMenu(wxTaskBarIconExEvent& WXUNUSED(event)) {
    CreateContextMenu();
}
#else
void CTaskBarIcon::OnContextMenu(wxTaskBarIconEvent& WXUNUSED(event)) {
    CreateContextMenu();
}
#endif


#ifdef __WXMSW__
void CTaskBarIcon::OnRButtonDown(wxTaskBarIconEvent& WXUNUSED(event)) {
    if (!IsBalloonsSupported()) {
        m_bButtonPressed = true;
    }
}


void CTaskBarIcon::OnRButtonUp(wxTaskBarIconEvent& WXUNUSED(event)) {
    if (!IsBalloonsSupported()) {
        if (m_bButtonPressed) {
            CreateContextMenu();
            m_bButtonPressed = false;
        }
    }
}
#endif
#endif  // !__WXMAC__


void CTaskBarIcon::ResetSuspendState() {
    m_pTimer->Stop();
}


void CTaskBarIcon::ResetTaskBar() {
#ifdef __WXMSW___
    SetBalloon(m_iconTaskBarIcon, wxT(""), wxT(""));
#else
#ifndef __WXMAC__
    SetIcon(m_iconTaskBarIcon, wxT(""));
#endif
#endif

    m_dtLastBalloonDisplayed = wxDateTime::Now();
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

#else  // ! __WXMAC__

void CTaskBarIcon::CreateContextMenu() {
    ResetTaskBar();

    wxMenu*     menu     = BuildContextMenu();
    wxMenuItem* menuItem = NULL;
    wxFont      font     = wxNullFont;

    // These should be in Windows Task Bar Menu but not in Mac's Dock menu
    menu->AppendSeparator();
    menu->Append(wxID_EXIT, _("E&xit"), wxEmptyString);

#ifdef __WXMSW__
    menuItem = menu->FindItem(wxID_EXIT);
    menu->Remove(menuItem);

    font = menuItem->GetFont();
    font.SetWeight(wxFONTWEIGHT_NORMAL);
    menuItem->SetFont(font);

    menu->Append(menuItem);
#endif

    PopupMenu(menu);
    delete menu;
}

#endif  // ! __WXMAC__

wxMenu *CTaskBarIcon::BuildContextMenu() {

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    ACCT_MGR_INFO      ami;
    bool               is_acct_mgr_detected = false;
    wxMenu*            pMenu         = new wxMenu;
    wxMenuItem*        pMenuItem     = NULL;
    wxString           menuName      = wxEmptyString;
    wxFont             font          = wxNullFont;
#ifdef __WXMSW__
    wxMenuItem*        menuItem      = NULL;
    size_t             loc = 0;
#endif

    wxASSERT(pMenu);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Account managers have a different menu arrangement
    pDoc->rpc.acct_mgr_info(ami);
    is_acct_mgr_detected = ami.acct_mgr_url.size() ? true : false;

    if (is_acct_mgr_detected) {
        menuName.Printf(
            _("Open %s Web..."),
            wxGetApp().GetBrand()->GetProjectName().c_str()
        );
        pMenu->Append(ID_OPENWEBSITE, menuName, wxEmptyString);
    }

    menuName.Printf(
        _("Open %s..."),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    pMenu->Append(wxID_OPEN, menuName, wxEmptyString);

    pMenu->AppendCheckItem(ID_TB_SUSPEND, _("Snooze"), wxEmptyString);

    menuName.Printf(
        _("&About %s..."),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );

    pMenu->Append(wxID_ABOUT, menuName, wxEmptyString);

#ifdef __WXMSW__
    for (loc = 0; loc < pMenu->GetMenuItemCount(); loc++) {
        pMenuItem = pMenu->FindItemByPosition(loc);
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
#endif

    if (is_acct_mgr_detected) {
        pMenu->InsertSeparator(3);
        pMenu->InsertSeparator(2);
    } else {
        pMenu->InsertSeparator(2);
        pMenu->InsertSeparator(1);
    }

    AdjustMenuItems(pMenu);

    return pMenu;
}

void CTaskBarIcon::AdjustMenuItems(wxMenu* menu) {
    CMainDocument* pDoc          = wxGetApp().GetDocument();
    wxInt32        iActivityMode = -1;
    wxInt32        iNetworkMode  = -1;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->GetActivityRunMode(iActivityMode);
    pDoc->GetNetworkRunMode(iNetworkMode);

    if ((RUN_MODE_NEVER == iActivityMode) && (RUN_MODE_NEVER == iActivityMode)) {
        menu->Check(ID_TB_SUSPEND, true);
    } else {
        menu->Check(ID_TB_SUSPEND, false);
    }
    
#ifdef __WXMAC__
//    WindowRef win = ActiveNonFloatingWindow();
    WindowRef win = FrontWindow();
    WindowModality modality = kWindowModalityNone;
    wxMenuItem *item;
    unsigned int i;
    
    if (win)
        GetWindowModality(win, &modality, NULL);
    for (i = 0; i <menu->GetMenuItemCount() ; i++) {
        item = menu->FindItemByPosition(i);
        if (modality == kWindowModalityAppModal)
            item->Enable(false);
        else
            item->Enable(!(item->IsSeparator()));
    }
#endif // __WXMAC__
}

const char *BOINC_RCSID_531575eeaa = "$Id$";
