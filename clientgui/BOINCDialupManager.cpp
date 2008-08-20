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
#pragma implementation "BOINCDialupManager.h"
#endif

#include "stdwx.h"
#include "network.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCTaskBar.h"
#include "BOINCBaseFrame.h"
#include "BOINCDialupManager.h"
#include "DlgOptions.h"


CBOINCDialUpManager::CBOINCDialUpManager() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCDialUpManager::CBOINCDialUpManager - Function Begin"));

#ifdef __WXMSW__
    m_pDialupManager = wxDialUpManager::Create();
    wxASSERT(m_pDialupManager->IsOk());
#endif
    ResetReminderTimers();
    m_bSetConnectionTimer = false;
    m_bNotifyConnectionAvailable = false;
    m_bConnectedSuccessfully = false;
    m_bResetTimers = false;
    m_bWasDialing = false;
    m_iNetworkStatus = 0;
    m_iConnectAttemptRetVal = 0;


    // Construct the default dialog title for dial-up messages
    //
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    m_strDialogTitle.Printf(
        _("%s - Network Status"),
        wxGetApp().GetSkinManager()->GetAdvanced()->GetApplicationName().c_str()
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCDialUpManager::CBOINCDialUpManager - Function End"));
}


CBOINCDialUpManager::~CBOINCDialUpManager() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCDialUpManager::~CBOINCDialUpManager - Function Begin"));

#ifdef __WXMSW__
    delete m_pDialupManager;
#endif
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCDialUpManager::~CBOINCDialUpManager - Function End"));
}


bool CBOINCDialUpManager::IsOk() {
#ifdef __WXMSW__
    return m_pDialupManager->IsOk();
#else
    return true;
#endif
}


size_t CBOINCDialUpManager::GetISPNames(wxArrayString& names) {
    return m_pDialupManager->GetISPNames(names);
}


void CBOINCDialUpManager::OnPoll() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    static bool         bAlreadyRunningLoop = false;
    bool                bIsOnline = false;
    bool                bWantConnection = false;
    bool                bWantDisconnect = false;
    wxString            strDialogMessage = wxEmptyString;
    CC_STATUS           cc_status;


    // We are ready to rock and roll.
    if (!bAlreadyRunningLoop && pDoc && pFrame) {
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

        bAlreadyRunningLoop = true;

        // cache the various states

        // The dialup manager tells us if we are still dialing or if we have
        //   successfully connected.  IsNetworkAlive/IsOnline both report the
        //   success or failure of the dialup device to establish a connection
        //   to the outside world.
        pDoc->GetCoreClientStatus(cc_status);

        bIsOnline = (cc_status.network_status == NETWORK_STATUS_ONLINE);
        bWantConnection = (cc_status.network_status == NETWORK_STATUS_WANT_CONNECTION);
        bWantDisconnect = (cc_status.network_status == NETWORK_STATUS_WANT_DISCONNECT);

        // The timers are used to keep from spamming the user with the same
        //   messages over each iteration of the poll loop.  we only need to
        //   reset them during a connect event in case we randomly loose
        //   a connection.
        if (m_bResetTimers) {
            wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - Resetting dial-up notification timers"));

            m_bResetTimers = false;
            m_bSetConnectionTimer = false;
            ResetReminderTimers();
        }

        // Log out the trace information for debugging purposes.
        /*
        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - Dialup Flags"));
        wxLogTrace(wxT("Function Status"), 
            wxT("CBOINCDialUpManager::poll - -- bIsOnline = '%d', bIsDialing = '%d', m_bWasDialing = '%d', iNetworkStatus = '%d', bWantConnection = '%d'"),
            bIsOnline, bIsDialing, m_bWasDialing, iNetworkStatus, bWantConnection
        );
        wxLogTrace(wxT("Function Status"),
            wxT("CBOINCDialUpManager::poll - -- m_bResetTimers = '%d', m_bNotifyConnectionAvailable = '%d', m_bConnectedSuccessfully = '%d'"),
            m_bResetTimers, m_bNotifyConnectionAvailable, m_bConnectedSuccessfully
        );
        wxLogTrace(wxT("Function Status"),
            wxT("CBOINCDialUpManager::poll - -- confirm_before_connecting = '%d', hangup_if_dialed = '%d'"),
            pDoc->state.global_prefs.confirm_before_connecting, pDoc->state.global_prefs.hangup_if_dialed
        );
        */

#ifndef __WXMSW__           // notification logic for non MS-Windows systems
        if (!bIsOnline && bWantConnection) {
            // Make sure window is visable and active, don't want the message box
            // to pop up on top of anything else
            wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - Notify need Internet Connection"));
            if (pFrame->IsShown() && wxGetApp().IsActive()) {
                NotifyUserNeedConnection(false);
            } 
/*
// this section commented out until taskbar/systray notification alerts are implemented
              else {
                // window is not visible, show alert
                if (pTaskbar && pTaskbar->IsBalloonsSupported()) {
                    NotifyUserNeedConnection(true);
                }
            }
*/
        } 
#else               // dialer stuff for MS-Windows systems
        bool  bIsDialing = m_pDialupManager->IsDialing();
        if (!bIsOnline && !bIsDialing && !m_bWasDialing && bWantConnection) {
            wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - !bIsOnline && !bIsDialing && !m_bWasDialing && bWantConnection"));
            if (!pFrame->IsShown()) {
                // BOINC Manager is hidden and displaying a dialog might interupt what they
                //   are doing.
                NotifyUserNeedConnection(true);
            } else {
                // BOINC Manager is visable and can process user input.
                m_bSetConnectionTimer = true;
                Connect();
            }
        } else if (!bIsDialing && !m_bWasDialing) {
            // We are not doing anything now, were we up to something before?
            if (bIsOnline && m_bConnectedSuccessfully && m_bNotifyConnectionAvailable) {
                // Ah ha, we are online and we initiated the connection, so we need to
                //   notify the CC that the network is available.
                wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - bIsOnline && m_bConnectedSuccessfully && m_bNotifyConnectionAvailable"));
                NetworkAvailable();
            } else if (bWantDisconnect && m_bConnectedSuccessfully) {
                // We are online, and the CC says it is safe to disconnect.  Since we
                //   initiated the connection we need to disconnect now.
                wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - bWantDisconnect && m_bConnectedSuccessfully"));
                Disconnect();
            }
        } else if (!bIsDialing && m_bWasDialing) {
            // We initiated a connection attempt and now we are either online or failed to
            //   connect because of a modem error or a users credentials were wrong.
            wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - !bIsDialing && m_bWasDialing"));

            if (m_bSetConnectionTimer) {
                m_bSetConnectionTimer = false;
                m_dtDialupConnectionTimeout = wxDateTime::Now();
                m_iConnectAttemptRetVal = ERR_NO_NETWORK_CONNECTION;
            }

            wxTimeSpan tsTimeout = wxDateTime::Now() - m_dtDialupConnectionTimeout;
            if (30 > tsTimeout.GetSeconds()) {
                if(m_iConnectAttemptRetVal != BOINC_SUCCESS) {
                    return;
                }
            }

            m_bWasDialing = false;
            m_bResetTimers = true;
            if (!m_iConnectAttemptRetVal) {
                ConnectionSucceeded();
            } else {
                ConnectionFailed();
            }
        } else if (bIsDialing && !m_bWasDialing) {
            // Setup the state machine so that it knows when we have finished the connection
            //   attempt.
            wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - bIsDialing && !m_bWasDialing"));
            m_bWasDialing = true;
        }
#endif
        bAlreadyRunningLoop = false;
    }
}


int CBOINCDialUpManager::NotifyUserNeedConnection(bool bNotificationOnly) {
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxTimeSpan          tsLastDialupAlertSent;
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pFrame);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    tsLastDialupAlertSent = wxDateTime::Now() - m_dtLastDialupAlertSent;
    if ((tsLastDialupAlertSent.GetMinutes() >= pFrame->GetReminderFrequency()) && (pFrame->GetReminderFrequency() != 0)) {
        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::NotifyUserNeedConnection - Manager not shown, notify instead"));
        m_dtLastDialupAlertSent = wxDateTime::Now();

#ifdef __WXWIN__
        // 1st %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        // 2st %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogMessage.Printf(
            _("%s needs to connect to the Internet.  Please click to open %s."),
            pSkinAdvanced->GetApplicationShortName().c_str(),
            pSkinAdvanced->GetApplicationName().c_str()
        );
#else
        // %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        strDialogMessage.Printf(
            _("%s is unable to communicate with a project and needs an Internet connection.\nPlease connect to the Internet, then select the 'Do network communications' item from the Advanced menu."),
            pSkinAdvanced->GetApplicationShortName().c_str()
        );
#endif
        pFrame->ShowAlert(
            m_strDialogTitle,
            strDialogMessage,
            wxOK | wxICON_INFORMATION,
            bNotificationOnly
        );
    }

    return 0;
}


int CBOINCDialUpManager::Connect() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxTimeSpan          tsLastDialupRequest;
    int                 iAnswer;
    wxString            strDialogMessage = wxEmptyString;
    GLOBAL_PREFS_MASK   mask;


    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    tsLastDialupRequest = wxDateTime::Now() - m_dtLastDialupRequest;
    if ((tsLastDialupRequest.GetMinutes() >= pFrame->GetReminderFrequency()) && (pFrame->GetReminderFrequency() != 0)) {
        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::Connect - Begin connection process"));
        m_dtLastDialupRequest = wxDateTime::Now();

        if(pFrame->GetDialupConnectionName().size()) {
            // We have a valid connection name that we can dial.
            // Update current working preferences (including any overrides) from client
            pDoc->rpc.get_global_prefs_working_struct(pDoc->state.global_prefs, mask);
            if(pDoc->state.global_prefs.confirm_before_connecting) {
                // %s is the project name
                //    i.e. 'BOINC', 'GridRepublic'
                strDialogMessage.Printf(
                    _("%s needs to connect to the Internet.\nMay it do so now?"),
                    pSkinAdvanced->GetApplicationShortName().c_str()
                );
                iAnswer = wxGetApp().SafeMessageBox(
                    strDialogMessage,
                    m_strDialogTitle,
                    wxYES_NO | wxICON_QUESTION,
                    pFrame
                );
            } else {
                // %s is the project name
                //    i.e. 'BOINC', 'GridRepublic'
                strDialogMessage.Printf(
                    _("%s is connecting to the Internet."),
                    pSkinAdvanced->GetApplicationShortName().c_str()
                );
                pFrame->ShowAlert(
                    m_strDialogTitle,
                    strDialogMessage,
                    wxOK | wxICON_INFORMATION,
                    true
                );
                iAnswer = wxYES;
            }

            // Are we allow to connect?
            if (wxYES == iAnswer) {
                m_bNotifyConnectionAvailable = true;
                m_bConnectedSuccessfully = false;
                m_pDialupManager->Dial(
                    pFrame->GetDialupConnectionName(),
                    wxEmptyString,
                    wxEmptyString,
                    true
                );
            }
        } else {
            // The user hasn't given us a valid connection to dial.  Inform them
            //   that we need a connection and that they may need to set a default
            //   connection.

            // %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strDialogMessage.Printf(
                _("%s couldn't do Internet communication, and no default connection is selected.\nPlease connect to the Internet, or select a default connection\nusing Advanced/Options/Connections."),
                pSkinAdvanced->GetApplicationShortName().c_str()
            );

            pFrame->ShowAlert(
                m_strDialogTitle,
                strDialogMessage,
                wxOK | wxICON_INFORMATION,
                false
            );
        }
    }

    return 0;
}


int CBOINCDialUpManager::ConnectionSucceeded() {
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            strDialogMessage = wxEmptyString;


    wxASSERT(pFrame);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s has successfully connected to the Internet."),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    pFrame->ShowAlert(
        m_strDialogTitle,
        strDialogMessage,
        wxOK | wxICON_INFORMATION,
        true
    );
    m_bConnectedSuccessfully = true;

    return 0;
}


int CBOINCDialUpManager::ConnectionFailed() {
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pFrame);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s failed to connect to the Internet."),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    pFrame->ShowAlert(
        m_strDialogTitle,
        strDialogMessage,
        wxOK | wxICON_ERROR,
        true
    );
    m_bConnectedSuccessfully = false;

    return 0;
}


int CBOINCDialUpManager::NetworkAvailable() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            strDialogMessage = wxEmptyString;


    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::NetworkAvailable - Connection Detected, notifing user of update to all projects"));

    m_bNotifyConnectionAvailable = false;

    // We are already online but BOINC for some reason is in a state
    //   where it belives it has some pending work to do, so give it
    //   a nudge

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s has detected it is now connected to the Internet.\nUpdating all projects and retrying all transfers."),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );

    pFrame->ShowAlert(
        m_strDialogTitle,
        strDialogMessage,
        wxOK | wxICON_INFORMATION,
        true
    );

    // Signal BOINC to update all projects and transfers.
    pDoc->rpc.network_available();

    return 0;
}


int CBOINCDialUpManager::Disconnect() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString            strDialogMessage = wxEmptyString;
    GLOBAL_PREFS_MASK   mask;


    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::Disconnect - Connection Detected, disconnect requested via the CC."));

    // Update current working preferences (including any overrides) from client
    pDoc->rpc.get_global_prefs_working_struct(pDoc->state.global_prefs, mask);
    if (pDoc->state.global_prefs.hangup_if_dialed) {
        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::Disconnect - Connection Detected, Don't need the network, Hanging up."));
        if (m_pDialupManager->HangUp()) {

            // %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strDialogMessage.Printf(
                _("%s has successfully disconnected from the Internet."),
                pSkinAdvanced->GetApplicationShortName().c_str()
            );
            pFrame->ShowAlert(
                m_strDialogTitle,
                strDialogMessage,
                wxOK | wxICON_INFORMATION,
                true
            );
            m_bConnectedSuccessfully = false;

        } else {

            // %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strDialogMessage.Printf(
                _("%s failed to disconnected from the Internet."),
                pSkinAdvanced->GetApplicationShortName().c_str()
            );
            pFrame->ShowAlert(
                m_strDialogTitle,
                strDialogMessage,
                wxOK | wxICON_ERROR
            );
        }
    }

    return 0;
}


void CBOINCDialUpManager::ResetReminderTimers() {
    m_dtLastDialupAlertSent = wxDateTime((time_t)0);
    m_dtLastDialupRequest = wxDateTime((time_t)0);
    m_dtDialupConnectionTimeout = wxDateTime((time_t)0);
}

