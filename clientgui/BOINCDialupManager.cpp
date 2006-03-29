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
#pragma implementation "BOINCDialupManager.h"
#endif

#include "stdwx.h"
#include "network.h"
#include "BOINCGUIApp.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "BOINCDialupManager.h"
#include "DlgOptions.h"
#include "DlgDialupCredentials.h"


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
        wxGetApp().GetBrand()->GetApplicationName().c_str()
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


void CBOINCDialUpManager::poll() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CMainFrame*         pFrame = wxGetApp().GetFrame();

    bool                bIsOnline = false;
    bool                bWantConnection = false;
    bool                bWantDisconnect = false;
    int                 iNetworkStatus = 0;
    wxString            strDialogMessage = wxEmptyString;


    // We are ready to rock and roll.
    if (pDoc) {
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));


        // cache the various states

        // The dialup manager tells us if we are still dialing or if we have
        //   successfully connected.  IsNetworkAlive/IsOnline both report the
        //   success or failure of the dialup device to establish a connection
        //   to the outside world.
        pDoc->rpc.network_status(iNetworkStatus);

        bIsOnline = iNetworkStatus == 0 ? true : false;
        bWantConnection = iNetworkStatus == 1 ? true : false;
        bWantDisconnect = iNetworkStatus == 2 ? true : false;

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
        wxTimeSpan          tsLastDialupAlertSent;

        if (!bIsOnline && bWantConnection) {
            wxTimeSpan          tsLastDialupAlertSent;

            // Make sure window is visable and active, don't want the message box
            // to pop up on top of anything else
            if (pFrame->IsShown() && pFrame->IsActive()) {
                tsLastDialupAlertSent = wxDateTime::Now() - m_dtLastDialupAlertSent;
                if (tsLastDialupAlertSent.GetSeconds() >= (pFrame->GetReminderFrequency() * 60)) {
                    wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - Notify need Internet Connection"));
                    m_dtLastDialupAlertSent = wxDateTime::Now();

                    // %s is the project name
                    //    i.e. 'BOINC', 'GridRepublic'
                    strDialogMessage.Printf(
                        _("%s is unable to communicate with a project and needs an Internet "
                          "connection.  Please connect to the Internet, then select the 'retry "
                          "communications' item off the advanced menu."),
                        wxGetApp().GetBrand()->GetProjectName().c_str()
                    );
                    
                    pFrame->ShowAlert(
                        m_strDialogTitle,
                        strDialogMessage,
                        wxICON_INFORMATION,      
                        false
                    );
                }
            } 
/*
// this section commented out until taskbar/systray notification alerts are implemented
              else {
                // window is not visible, show alert
                CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
                wxASSERT(pTaskbar);
                if (pTaskbar && pTaskbar->IsBalloonsSupported()) {
                    tsLastDialupAlertSent = wxDateTime::Now() - m_dtLastDialupAlertSent;
                    if (tsLastDialupAlertSent.GetSeconds() >= (pFrame->GetReminderFrequency() * 60)) {
                        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - Need Internet Connection Alert"));
                        m_dtLastDialupAlertSent = wxDateTime::Now();

                        // %s is the project name
                        //    i.e. 'BOINC', 'GridRepublic'
                        strDialogMessage.Printf(
                            _("%s is unable to communicate with a project and needs an Internet "
                              "connection.  Please connect to the Internet then open the %s and "
                              "'retry communications'."),
                            wxGetApp().GetBrand()->GetProjectName().c_str(),
                            wxGetApp().GetBrand()->GetApplicationName().c_str()
                        );
                    
                        pFrame->ShowAlert(
                            m_strDialogTitle,
                            strDialogMessage,
                            wxICON_INFORMATION,      
                            true
                        );
                    }
                }
            }
*/
        } 
#else               // dialer stuff for MS-Windows systems
        bool  bIsDialing = m_pDialupManager->IsDialing();
        if (!bIsOnline && !bIsDialing && !m_bWasDialing && bWantConnection)
        {
            wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::poll - !bIsOnline && !bIsDialing && !m_bWasDialing && bWantConnection"));
            if (!pFrame->IsShown()) {
                // BOINC Manager is hidden and displaying a dialog might interupt what they
                //   are doing.
                NotifyUserNeedConnection();
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
                    if (m_iConnectAttemptRetVal != ERR_IN_PROGRESS) {
                        // Attempt to successfully download the Google homepage
                        pDoc->rpc.lookup_website(LOOKUP_GOOGLE);
                    }
                    m_iConnectAttemptRetVal = pDoc->rpc.lookup_website_poll();
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
    }
}


int CBOINCDialUpManager::NotifyUserNeedConnection() {
    CMainFrame*         pFrame = wxGetApp().GetFrame();
    wxTimeSpan          tsLastDialupAlertSent;
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    tsLastDialupAlertSent = wxDateTime::Now() - m_dtLastDialupAlertSent;
    if (tsLastDialupAlertSent.GetSeconds() >= (pFrame->GetReminderFrequency() * 60)) {
        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::NotifyUserNeedConnection - Manager not shown, notify instead"));

        m_dtLastDialupAlertSent = wxDateTime::Now();

        // 1st %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        // 2st %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogMessage.Printf(
            _("%s needs a connection to the Internet to perform some "
                "maintenance, open the %s to connect up and "
                "perform the needed work."),
            wxGetApp().GetBrand()->GetProjectName().c_str(),
            wxGetApp().GetBrand()->GetApplicationName().c_str()
        );

        pFrame->ShowAlert(
            m_strDialogTitle,
            strDialogMessage,
            wxICON_INFORMATION,
            true
        );
    }

    return 0;
}


int CBOINCDialUpManager::Connect() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CMainFrame*         pFrame = wxGetApp().GetFrame();
    wxTimeSpan          tsLastDialupRequest;
    int                 iAnswer;
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    tsLastDialupRequest = wxDateTime::Now() - m_dtLastDialupRequest;
    if (tsLastDialupRequest.GetSeconds() >= (pFrame->GetReminderFrequency() * 60)) {
        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::Connect - Begin connection process"));

        m_dtLastDialupRequest = wxDateTime::Now();

        if(pFrame->GetDialupConnectionName().size()) {
            // We have a valid connection name that we can dial.
            if(pDoc->state.global_prefs.confirm_before_connecting) {
                // %s is the project name
                //    i.e. 'BOINC', 'GridRepublic'
                strDialogMessage.Printf(
                    _("%s needs to connect to the Internet.\nMay it do so now?"),
                    wxGetApp().GetBrand()->GetProjectName().c_str()
                );
                iAnswer = ::wxMessageBox(
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
                    wxGetApp().GetBrand()->GetProjectName().c_str()
                );
                pFrame->ShowAlert(
                    m_strDialogTitle,
                    strDialogMessage,
                    wxICON_INFORMATION,
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
                _("%s is unable to communicate with a project and no default connection is specified.\n"
                  "Please connect up to the Internet or specify a default connection via the connections\n"
                  "tab in the Options dialog off of the advanced menu."),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );

            pFrame->ShowAlert(
                m_strDialogTitle,
                strDialogMessage,
                wxICON_INFORMATION,
                false
            );
        }
    }

    return 0;
}


int CBOINCDialUpManager::ConnectionSucceeded() {
    CMainFrame*         pFrame = wxGetApp().GetFrame();
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s has successfully connected to the Internet."),
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    pFrame->ShowAlert(
        m_strDialogTitle,
        strDialogMessage,
        wxICON_INFORMATION,
        true
    );
    m_bConnectedSuccessfully = true;

    return 0;
}


int CBOINCDialUpManager::ConnectionFailed() {
    CMainFrame*         pFrame = wxGetApp().GetFrame();
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s failed to connect to the Internet."),
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    pFrame->ShowAlert(
        m_strDialogTitle,
        strDialogMessage,
        wxICON_ERROR,
        true
    );
    m_bConnectedSuccessfully = false;

    return 0;
}


int CBOINCDialUpManager::NetworkAvailable() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CMainFrame*         pFrame = wxGetApp().GetFrame();
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::NetworkAvailable - Connection Detected, notifing user of update to all projects"));

    m_bNotifyConnectionAvailable = false;

    // We are already online but BOINC for some reason is in a state
    //   where it belives it has some pending work to do, so give it
    //   a nudge

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s has detected it is now connected to the Internet. "
          "Updating all projects and retrying all transfers."),
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );

    pFrame->ShowAlert(
        m_strDialogTitle,
        strDialogMessage,
        wxICON_INFORMATION,
        true
    );

    // Signal BOINC to update all projects and transfers.
    pDoc->rpc.network_available();

    return 0;
}


int CBOINCDialUpManager::Disconnect() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CMainFrame*         pFrame = wxGetApp().GetFrame();
    wxString            strDialogMessage = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));


    wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::Disconnect - Connection Detected, disconnect requested via the CC."));

    if (pDoc->state.global_prefs.hangup_if_dialed) {
        wxLogTrace(wxT("Function Status"), wxT("CBOINCDialUpManager::Disconnect - Connection Detected, Don't need the network, Hanging up."));
        if (m_pDialupManager->HangUp()) {

            // %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strDialogMessage.Printf(
                _("%s has successfully disconnected from the Internet."),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );
            pFrame->ShowAlert(
                m_strDialogTitle,
                strDialogMessage,
                wxICON_INFORMATION,
                true
            );
            m_bConnectedSuccessfully = false;

        } else {

            // %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strDialogMessage.Printf(
                _("%s failed to disconnected from the Internet."),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );
            pFrame->ShowAlert(
                m_strDialogTitle,
                strDialogMessage,
                wxICON_ERROR
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

