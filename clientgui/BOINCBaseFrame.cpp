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
#pragma implementation "BOINCBaseFrame.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCClientManager.h"
#include "BOINCTaskBar.h"
#include "BOINCBaseFrame.h"
#include "BOINCDialupManager.h"
#include "Events.h"
#include "DlgEventLog.h"
#include "DlgSelectComputer.h"
#include "wizardex.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"

DEFINE_EVENT_TYPE(wxEVT_FRAME_ALERT)
DEFINE_EVENT_TYPE(wxEVT_FRAME_CONNECT)
DEFINE_EVENT_TYPE(wxEVT_FRAME_INITIALIZED)
DEFINE_EVENT_TYPE(wxEVT_FRAME_REFRESHVIEW)
DEFINE_EVENT_TYPE(wxEVT_FRAME_UPDATESTATUS)
DEFINE_EVENT_TYPE(wxEVT_FRAME_RELOADSKIN)
DEFINE_EVENT_TYPE(wxEVT_FRAME_NOTIFICATION)


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseFrame, wxFrame)

BEGIN_EVENT_TABLE (CBOINCBaseFrame, wxFrame)
    EVT_TIMER(ID_DOCUMENTPOLLTIMER, CBOINCBaseFrame::OnDocumentPoll)
    EVT_TIMER(ID_ALERTPOLLTIMER, CBOINCBaseFrame::OnAlertPoll)
    EVT_TIMER(ID_PERIODICRPCTIMER, CBOINCBaseFrame::OnPeriodicRPC)
    EVT_FRAME_INITIALIZED(CBOINCBaseFrame::OnInitialized)
    EVT_FRAME_ALERT(CBOINCBaseFrame::OnAlert)
    EVT_FRAME_REFRESH(CBOINCBaseFrame::OnRefreshView)
    EVT_ACTIVATE(CBOINCBaseFrame::OnActivate)
    EVT_CLOSE(CBOINCBaseFrame::OnClose)
    EVT_MENU(ID_CLOSEWINDOW, CBOINCBaseFrame::OnCloseWindow)
    EVT_MENU(wxID_EXIT, CBOINCBaseFrame::OnExit)
#if SUPPORTDARKMODE
    EVT_SYS_COLOUR_CHANGED(CBOINCBaseFrame::OnDarkModeChanged)
#endif
END_EVENT_TABLE ()


CBOINCBaseFrame::CBOINCBaseFrame()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Default Constructor Function End"));
}


CBOINCBaseFrame::CBOINCBaseFrame(wxWindow* parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style) :
    wxFrame(parent, id, title, pos, size, style)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Function Begin"));

    // Configuration Settings
    m_iReminderFrequency = 0;
    m_strNetworkDialupConnectionName = wxEmptyString;
    m_aSelectedComputerMRU.Clear();
    m_bShowConnectionFailedAlert = false;


    m_pDialupManager = new CBOINCDialUpManager();
    wxASSERT(m_pDialupManager->IsOk());


    m_pDocumentPollTimer = new wxTimer(this, ID_DOCUMENTPOLLTIMER);
    wxASSERT(m_pDocumentPollTimer);

    m_pDocumentPollTimer->Start(250);               // Send event every 250 milliseconds

    m_pAlertPollTimer = new wxTimer(this, ID_ALERTPOLLTIMER);
    wxASSERT(m_pAlertPollTimer);

    m_pAlertPollTimer->Start(1000);                 // Send event every 1000 milliseconds

    m_pPeriodicRPCTimer = new wxTimer(this, ID_PERIODICRPCTIMER);
    wxASSERT(m_pPeriodicRPCTimer);

    m_pPeriodicRPCTimer->Start(1000);               // Send event every 1000 milliseconds
    m_iFrameRefreshRate = 1000;                     // Refresh frame every 1000 milliseconds

    // Limit the number of times the UI can update itself to two times a second
    //   NOTE: Linux and Mac were updating several times a second and eating
    //         CPU time
    wxUpdateUIEvent::SetUpdateInterval(500);

    m_ptFramePos = wxPoint(0, 0);

    // The second half of the initialization process picks up in the OnFrameRender()
    //   routine since the menus' and status bars' are drawn in the frameworks
    //   on idle routines, on idle events are sent in between the end of the
    //   constructor and the first call to OnFrameRender
    //
    // Look for the 'if (!bAlreadyRunOnce) {' statement

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Function End"));
}


CBOINCBaseFrame::~CBOINCBaseFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::~CBOINCBaseFrame - Function Begin"));

    wxASSERT(m_pPeriodicRPCTimer);
    wxASSERT(m_pAlertPollTimer);
    wxASSERT(m_pDocumentPollTimer);

    if (m_pPeriodicRPCTimer) {
        m_pPeriodicRPCTimer->Stop();
        delete m_pPeriodicRPCTimer;
    }

    if (m_pAlertPollTimer) {
        m_pAlertPollTimer->Stop();
        delete m_pAlertPollTimer;
    }

    if (m_pDocumentPollTimer) {
        m_pDocumentPollTimer->Stop();
        delete m_pDocumentPollTimer;
    }

    if (m_pDialupManager) {
        delete m_pDialupManager;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::~CBOINCBaseFrame - Function End"));
}


void CBOINCBaseFrame::OnPeriodicRPC(wxTimerEvent& WXUNUSED(event)) {
    static bool        bAlreadyRunningLoop = false;
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

#ifdef __WXMAC__
    static bool first = true;
    if (first) {
        first = false;
        wxGetApp().OnFinishInit();
    }

//    wxGetApp().CheckPartialActivation();
#endif

    if (!bAlreadyRunningLoop && m_pPeriodicRPCTimer->IsRunning()) {
        bAlreadyRunningLoop = true;

        pDoc->RunPeriodicRPCs(m_iFrameRefreshRate);

        bAlreadyRunningLoop = false;
    }
}


void CBOINCBaseFrame::OnDocumentPoll(wxTimerEvent& WXUNUSED(event)) {
    static bool        bAlreadyRunOnce = false;
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Timer events are handled while the RPC Wait dialog is shown
    // which may cause unintended recursion and repeatedly posting
    // the same RPC requests from timer routines.
    if (pDoc->WaitingForRPC()) return;

    if (!bAlreadyRunOnce && m_pDocumentPollTimer->IsRunning()) {
        // Complete any remaining initialization that has to happen after we are up
        //   and running
        FireInitialize();
        bAlreadyRunOnce = true;
    }

    pDoc->OnPoll();
}


void CBOINCBaseFrame::OnAlertPoll(wxTimerEvent& WXUNUSED(event)) {
    static bool       bAlreadyRunningLoop = false;
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    if (!bAlreadyRunningLoop && m_pAlertPollTimer->IsRunning()) {
        bAlreadyRunningLoop = true;

        // Update idle detection if needed.
        wxGetApp().UpdateSystemIdleDetection();

        // Check to see if there is anything that we need to do from the
        //   dial up user perspective.
        if (pDoc && m_pDialupManager) {
            // Timer events are handled while the RPC Wait dialog is shown
            // which may cause unintended recursion and repeatedly posting
            // the same RPC requests from timer routines.
            if (pDoc->IsConnected() && !pDoc->WaitingForRPC()) {
                m_pDialupManager->OnPoll();
            }
        }

        if (m_bShowConnectionFailedAlert && IsShown()) {
            m_bShowConnectionFailedAlert = false;
            ShowConnectionFailedAlert();
        }

        bAlreadyRunningLoop = false;
    }
}


void CBOINCBaseFrame::OnInitialized(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnInitialized - Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnInitialized - Function End"));
}


void CBOINCBaseFrame::OnRefreshView(CFrameEvent& ) {
}


void CBOINCBaseFrame::OnAlert(CFrameAlertEvent&
#ifndef __WXGTK__
        event
#endif
        ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnAlert - Function Begin"));
    static bool       bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

#ifdef __WXMSW__
        CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
        wxASSERT(pTaskbar);

        if ((IsShown() && !event.m_notification_only) || (IsShown() && !pTaskbar->IsBalloonsSupported())) {
            if (!event.m_notification_only) {
                int retval = 0;

                if (!IsShown()) {
                    Show();
                }

                retval = wxGetApp().SafeMessageBox(event.m_message, event.m_title, event.m_style, this);
                if (event.m_alert_event_type == AlertProcessResponse) {
                    event.ProcessResponse(retval);
                }
            }
        } else {
            // If the main window is hidden or minimzed use the system tray ballon
            //   to notify the user instead.  This keeps dialogs from interfering
            //   with people typing email messages or any other activity where they
            //   do not want keyboard focus changed to another window while typing.
            unsigned int  icon_type;

            if (wxICON_ERROR & event.m_style) {
                icon_type = NIIF_ERROR;
            } else if (wxICON_WARNING & event.m_style) {
                icon_type = NIIF_WARNING;
            } else if (wxICON_INFORMATION & event.m_style) {
                icon_type = NIIF_INFO;
            } else {
                icon_type = NIIF_NONE;
            }

            pTaskbar->SetBalloon(
                pTaskbar->m_iconTaskBarNormal,
                event.m_title,
                event.m_message,
                icon_type
            );
        }
#elif defined (__WXMAC__)
        // SafeMessageBox() / ProcessResponse() hangs the Manager if hidden.
        // Currently, the only non-notification-only alert is Connection Failed,
        // which is now has logic to be displayed when Manager is maximized.

        // Notification only events on platforms other than Windows are
        //   currently discarded.  Otherwise the application would be restored
        //   and input focus set on the notification which interrupts whatever
        //   the user was doing.
        if (IsShown() && !event.m_notification_only) {
            int retval = 0;

            retval = wxGetApp().SafeMessageBox(event.m_message, event.m_title, event.m_style, this);
            if (event.m_alert_event_type == AlertProcessResponse) {
                event.ProcessResponse(retval);
            }
        }
#endif

        bAlreadyRunningLoop = false;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnAlert - Function End"));
}


void CBOINCBaseFrame::OnActivate(wxActivateEvent& event) {
    bool isActive = event.GetActive();
    if (isActive) wxGetApp().SetEventLogWasActive(false);
    event.Skip();
}


void CBOINCBaseFrame::OnClose(wxCloseEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnClose - Function Begin"));

    if (
#if defined(__WXMAC__)
        IsIconized()
#elif defined(__WXGTK__)
        true
#else
        false
#endif
        || !event.CanVeto()) {
        // Destroy the top-level window (which will cause the Manager to exit)
        wxGetApp().FrameClosed();
        Destroy();
    } else {
        // Hide the top-level window (and keep the Manager running)
        Hide();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnClose - Function End"));
}


void CBOINCBaseFrame::OnCloseWindow(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnCloseWindow - Function Begin"));

	Close();

	wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnCloseWindow - Function End"));
}


void CBOINCBaseFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnExit - Function Begin"));

    if (wxGetApp().ConfirmExit()) {

        // Save state before exiting
        SaveState();

        CDlgEventLog*   eventLog = wxGetApp().GetEventLog();
        if (eventLog) {
            eventLog->Destroy();
        }

        Close(true);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnExit - Function End"));
}

void CBOINCBaseFrame::OnDarkModeChanged( wxSysColourChangedEvent& WXUNUSED(event) ) {
}


int CBOINCBaseFrame::GetCurrentViewPage() {
    return _GetCurrentViewPage();
}


void CBOINCBaseFrame::FireInitialize() {
    CFrameEvent event(wxEVT_FRAME_INITIALIZED, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireRefreshView() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->RefreshRPCs();
    pDoc->RunPeriodicRPCs(0);
}


void CBOINCBaseFrame::FireConnect() {
    CFrameEvent event(wxEVT_FRAME_CONNECT, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireReloadSkin() {
    CFrameEvent event(wxEVT_FRAME_RELOADSKIN, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireNotification() {
    CFrameEvent event(wxEVT_FRAME_NOTIFICATION, this);
    AddPendingEvent(event);
}


bool CBOINCBaseFrame::SelectComputer(wxString& hostName, int& portNum, wxString& password, bool required) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SelectComputer - Function Begin"));

    CDlgSelectComputer  dlg(this, required);
    size_t              lIndex = 0;
    wxArrayString       aComputerNames;
    bool                bResult = false;

    // Lets copy the template store in the system state
    aComputerNames = m_aSelectedComputerMRU;

    // Lets populate the combo control with the MRU list
    dlg.m_ComputerNameCtrl->Clear();
    for (lIndex = 0; lIndex < aComputerNames.Count(); lIndex++) {
        dlg.m_ComputerNameCtrl->Append(aComputerNames.Item(lIndex));
    }

    if (wxID_OK == dlg.ShowModal()) {
        hostName = dlg.m_ComputerNameCtrl->GetValue();
        // Make a null hostname be the same thing as localhost
        if (wxEmptyString == hostName) {
            hostName = wxT("localhost");
            portNum = GUI_RPC_PORT;
            password = wxEmptyString;
        } else {
            // Parse the remote machine info
            wxString sHost = dlg.m_ComputerNameCtrl->GetValue();
            long lPort = GUI_RPC_PORT;
            int iPos = sHost.Find(wxT(":"));
            if (iPos != wxNOT_FOUND) {
                wxString sPort = sHost.substr(iPos + 1);
                if (!sPort.ToLong(&lPort)) lPort = GUI_RPC_PORT;
                sHost.erase(iPos);
            }
            hostName = sHost;
            portNum = (int)lPort;
            password = dlg.m_ComputerPasswordCtrl->GetValue();
        }

        // Insert a copy of the current combo box value to the head of the
        //   computer names string array
        if (wxEmptyString != dlg.m_ComputerNameCtrl->GetValue()) {
            aComputerNames.Insert(dlg.m_ComputerNameCtrl->GetValue(), 0);
        }

        // Loops through the computer names and remove any duplicates that
        //   might exist with the new head value
        for (lIndex = 1; lIndex < aComputerNames.Count(); lIndex++) {
            if (aComputerNames.Item(lIndex) == aComputerNames.Item(0))
                aComputerNames.RemoveAt(lIndex);
        }

        // Store the modified computer name MRU list back to the system state
        m_aSelectedComputerMRU = aComputerNames;
        bResult = true;
    } else {
        bResult = false;        // User cancelled
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SelectComputer - Function End"));
    return bResult;
}


void CBOINCBaseFrame::ShowConnectionBadPasswordAlert(
    bool bUsedDefaultPassword, int /*iReadGUIRPCAuthFailure*/, std::string password_msg
) {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            strDialogTitle = wxEmptyString;


    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionBadPasswordAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Error"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    if ( bUsedDefaultPassword ) {
        if (password_msg.empty()) {
            password_msg = "Invalid client RPC password.  Try reinstalling BOINC.";
        }
    } else {
        password_msg = "Invalid client RPC password.  Try reinstalling BOINC.";
    }
    wxMessageBox(wxString(password_msg), strDialogTitle, wxOK | wxICON_ERROR);
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionBadPasswordAlert - Function End"));
}


void CBOINCBaseFrame::ShowConnectionFailedAlert() {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    wxString            strConnectedCompter = wxEmptyString;
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionFailedAlert - Function Begin"));

    // Did BOINC crash on local computer? If so restart it and reconnect.
    pDoc->GetConnectedComputerName(strConnectedCompter);
    if (pDoc->IsComputerNameLocal(strConnectedCompter)) {
        if (pDoc->m_pClientManager->AutoRestart()) {
            boinc_sleep(0.5);       // Allow time for Client to restart
            if (pDoc->m_pClientManager->IsBOINCCoreRunning()) {
                pDoc->Reconnect();
                return;
            }
        } else {
            // Don't ask whether to reconnect to local client if it is not running
            if (!pDoc->m_pClientManager->IsBOINCCoreRunning()) {
                return;
            }
        }
    } else {
        // Don't ask whether to reconnect to remote client
        pDoc->Reconnect();
        return;
    }

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Failed"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2st %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not able to connect to a %s client.\nWould you like to try to connect again?"),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );

    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxYES_NO | wxICON_QUESTION,
        false,
        AlertProcessResponse
    );

    // If we are minimized, set flag to show alert when maximized
    m_bShowConnectionFailedAlert = !IsShown();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionFailedAlert - Function End"));
}


void CBOINCBaseFrame::ShowDaemonStartFailedAlert() {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;


    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowDaemonStartFailedAlert - Function Begin"));


    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Daemon Start Failed"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2st %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
#ifdef __WXMSW__
    strDialogMessage.Printf(
        _("%s is not able to start a %s client.\nPlease launch the Control Panel->Administrative Tools->Services applet and start the BOINC service."),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
#else
    strDialogMessage.Printf(
        _("%s is not able to start a %s client.\nPlease run the client and try again."),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
#endif

    wxMessageBox(strDialogMessage, strDialogTitle, wxOK | wxICON_ERROR);
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowDaemonStartFailedAlert - Function End"));
}


void CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert() {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    wxString            strConnectedCompter = wxEmptyString;
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert - Function Begin"));

    // Did BOINC crash on local computer? If so restart it and reconnect.
    pDoc->GetConnectedComputerName(strConnectedCompter);
    if (pDoc->IsComputerNameLocal(strConnectedCompter)) {
        if (pDoc->m_pClientManager->AutoRestart()) {
            boinc_sleep(0.5);       // Allow time for Client to restart
            if (pDoc->m_pClientManager->IsBOINCCoreRunning()) {
                pDoc->Reconnect();
                return;
            }
        } else {
            // Don't ask whether to reconnect to local client if it is not running
            if (!pDoc->m_pClientManager->IsBOINCCoreRunning()) {
                return;
            }
        }
    }

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Status"),
        pSkinAdvanced->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    // 3nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not currently connected to a %s client.\nPlease use the 'Advanced\\Select Computer...' menu option to connect up to a %s client.\nTo connect up to your local computer please use 'localhost' as the host name."),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    wxMessageBox(strDialogMessage, strDialogTitle, wxOK | wxICON_ERROR);
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert - Function End"));
}


void CBOINCBaseFrame::StartTimersBase() {
    wxASSERT(m_pAlertPollTimer);
    wxASSERT(m_pPeriodicRPCTimer);
    wxASSERT(m_pDocumentPollTimer);
    m_pAlertPollTimer->Start();
    m_pPeriodicRPCTimer->Start();
    m_pDocumentPollTimer->Start();
}


void CBOINCBaseFrame::StopTimersBase() {
    wxASSERT(m_pAlertPollTimer);
    wxASSERT(m_pPeriodicRPCTimer);
    wxASSERT(m_pDocumentPollTimer);
    m_pAlertPollTimer->Stop();
    m_pPeriodicRPCTimer->Stop();
    m_pDocumentPollTimer->Stop();
}


void CBOINCBaseFrame::UpdateRefreshTimerInterval() {
}

#if 0
void CBOINCBaseFrame::UpdateStatusText(const wxChar* szStatus) {
    CFrameEvent event(wxEVT_FRAME_UPDATESTATUS, this, szStatus);
    ProcessEvent(event);
}
#endif

void CBOINCBaseFrame::ShowAlert( const wxString title, const wxString message, const int style, const bool notification_only, const FrameAlertEventType alert_event_type ) {
    CFrameAlertEvent event(wxEVT_FRAME_ALERT, this, title, message, style, notification_only, alert_event_type);
    AddPendingEvent(event);
}


bool CBOINCBaseFrame::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SaveState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    size_t          iIndex;
    wxArrayString   existingComputers;


    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("ReminderFrequencyV3"), m_iReminderFrequency);
    pConfig->Write(wxT("NetworkDialupConnectionName"), m_strNetworkDialupConnectionName);


    //
    // Save Computer MRU list
    //
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);


    // Retrieve existing computers in the config because
    // some computers may have been added by other BOINC manager windows.
    iIndex = 0;
    strBuffer.Printf(wxT("%zu"), iIndex);
    while (pConfig->Exists(strBuffer)) {
        wxString computer = pConfig->Read(strBuffer, wxEmptyString);
        if (computer != wxEmptyString && existingComputers.Index(computer) == wxNOT_FOUND) {
            existingComputers.Add(computer);
        }
        strBuffer.Printf(wxT("%zu"), ++iIndex);
    }

    for (iIndex = 0; iIndex < m_aSelectedComputerMRU.GetCount(); iIndex++) {
        strBuffer.Printf(wxT("%zu"), iIndex);
        pConfig->Write(
            strBuffer,
            m_aSelectedComputerMRU.Item(iIndex)
        );
    }

    // Write existing computers that are not in the MRU list into the config.
    for (const wxString& computer : existingComputers) {
        if (m_aSelectedComputerMRU.Index(computer) == wxNOT_FOUND) {
            strBuffer.Printf(wxT("%zu"), iIndex++);
            pConfig->Write(strBuffer, computer);
        }
    }

    // Remove any remaining MRU computer entries in the config to avoid duplicates.
    strBuffer.Printf(wxT("%zu"), iIndex);
    while (pConfig->Exists(strBuffer)) {
        pConfig->DeleteEntry(strBuffer);
        strBuffer.Printf(wxT("%zu"), ++iIndex);
    }

    pConfig->SetPath(strPreviousLocation);
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SaveState - Function End"));
    return true;
}


bool CBOINCBaseFrame::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    wxString        strValue;
    long            iIndex;
    bool            bKeepEnumerating = false;


    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("ReminderFrequencyV3"), &m_iReminderFrequency, 360L);
    pConfig->Read(wxT("NetworkDialupConnectionName"), &m_strNetworkDialupConnectionName, wxEmptyString);


    //
    // Restore Computer MRU list
    //
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);

    m_aSelectedComputerMRU.Clear();
    bKeepEnumerating = pConfig->GetFirstEntry(strBuffer, iIndex);
    while (bKeepEnumerating) {
        pConfig->Read(strBuffer, &strValue);
        if (m_aSelectedComputerMRU.Index(strValue) == wxNOT_FOUND) {
            m_aSelectedComputerMRU.Add(strValue);
        }

        bKeepEnumerating = pConfig->GetNextEntry(strBuffer, iIndex);
    }

    pConfig->SetPath(strPreviousLocation);


    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::RestoreState - Function End"));
    return true;
}

bool CBOINCBaseFrame::Show(bool bShow) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::Show - Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::Show - Show: '%d'"), (int)bShow);

    bool    retval;

    if (bShow) {
        wxGetApp().ShowApplication(true);
    } else {
        if ( this == wxGetApp().GetFrame() ) {
            if (wxGetApp().IsApplicationVisible()) {
                wxGetApp().ShowApplication(false);
            }
        }
    }

    CDlgEventLog* pEventLog = wxGetApp().GetEventLog();
    if (pEventLog) {
#ifdef __WXMAC__
        if (bShow) {
            pEventLog->Show(bShow);
        }
#else
        pEventLog->Show(bShow);
#endif
    }

#ifdef __WXMAC__
    retval = (wxGetApp().IsApplicationVisible() != bShow);
    if (bShow) {
        retval = wxFrame::Show(bShow);
    }
#else
    retval = wxFrame::Show(bShow);
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::Show - Function End"));
    return retval;
}

int CBOINCBaseFrame::_GetCurrentViewPage() {
    wxASSERT(false);
    return 0;
}


void CFrameAlertEvent::ProcessResponse(const int response) const {
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if ((AlertProcessResponse == m_alert_event_type) && (wxYES == response)) {
        pDoc->Reconnect();
    }
}

// bring up the attach-project dialog
//
void CBOINCBaseFrame::OnWizardAttachProject( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnWizardAttachProject - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized()) {
        return;
    }

    if (pDoc->IsConnected()) {

        // Stop all timers so that the wizard is the only thing doing anything
        StopTimers();

        CWizardAttach* pWizard = new CWizardAttach(this);

        pWizard->Run(
            wxEmptyString,
            wxEmptyString,
            wxEmptyString,
            wxEmptyString,
            wxEmptyString,
            wxEmptyString,
            false,
            false
        );

        pWizard->Destroy();

        // Restart timers to continue normal operations.
        StartTimers();

        pDoc->ForceCacheUpdate();
        FireRefreshView();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnWizardAttachProject - Function End"));
}


void CBOINCBaseFrame::OnWizardUpdate(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnWizardUpdate - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        // Stop all timers so that the wizard is the only thing doing anything
        StopTimers();

        CWizardAttach* pWizard = new CWizardAttach(this);

        pWizard->SyncToAccountManager();
        pWizard->Destroy();

        CreateMenus();
        pDoc->ForceCacheUpdate();
        FireRefreshView();
        ResetReminderTimers();

        // Restart timers to continue normal operations.
        StartTimers();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnWizardUpdate - Function End"));
}

// detach from account manager
//
void CBOINCBaseFrame::OnWizardDetach(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnWizardDetach - Function Begin"));

    CMainDocument* pDoc           = wxGetApp().GetDocument();
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxInt32        iAnswer        = 0;
    wxString       strTitle       = wxEmptyString;
    wxString       strMessage     = wxEmptyString;
    ACCT_MGR_INFO  ami;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    if (!pDoc->IsUserAuthorized()) {
        return;
    }

    if (pDoc->IsConnected()) {
        pDoc->rpc.acct_mgr_info(ami);

        strTitle.Printf(
            _("%s - Stop using %s"),
            pSkinAdvanced->GetApplicationName().c_str(),
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        strMessage.Printf(
            _("If you stop using %s,\nyou'll keep all your current projects,\nbut you'll have to manage projects manually.\n\nDo you want to stop using %s?"),
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str(),
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );

        iAnswer = wxGetApp().SafeMessageBox(
            strMessage,
            strTitle,
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->rpc.acct_mgr_rpc("", "", "", false);
        }

        CreateMenus();
        pDoc->ForceCacheUpdate();
        FireRefreshView();

    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnWizardDetach - Function End"));
}

void CBOINCBaseFrame::ResetReminderTimers() {
#ifdef __WXMSW__
    wxASSERT(m_pDialupManager);
    wxASSERT(wxDynamicCast(m_pDialupManager, CBOINCDialUpManager));

    m_pDialupManager->ResetReminderTimers();
#endif
}
