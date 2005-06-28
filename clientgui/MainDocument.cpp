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
#pragma implementation "MainDocument.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "error_numbers.h"


CNetworkConnection::CNetworkConnection(CMainDocument* pDocument) :
    wxObject() {
    m_pDocument = pDocument;

    m_strConnectedComputerName = wxEmptyString;
    m_strConnectedComputerPassword = wxEmptyString;
    m_strNewComputerName = wxEmptyString;
    m_strNewComputerPassword = wxEmptyString;
    m_bFrameShutdownDetected = false;
    m_bConnectEvent = false;
    m_bConnected = false;
    m_bReconnecting = false;
    m_bForceReconnect = false;
    m_bReconnectOnError = false;
}


CNetworkConnection::~CNetworkConnection() {
}


// TODO: get rid of "reconnecting" stuff

void* CNetworkConnection::Poll() {
    int retval;
    std::string strComputer;
    std::string strComputerPassword;

    if (IsReconnecting()) {
        wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Reconnection Detected"));
        retval = m_pDocument->rpc.init_poll();
        if (!retval) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - init_poll() returned ERR_CONNECT, now authorizing..."));
            retval = m_pDocument->rpc.authorize(m_strNewComputerPassword.c_str());
            if (!retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Connection Success"));
                std::string host = m_strNewComputerName.c_str();
                std::string pwd = m_strNewComputerPassword.c_str();
                SetStateSuccess(host, pwd);
            } else if (ERR_AUTHENTICATOR == retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Authorization - ERR_AUTHENTICATOR"));
                SetStateErrorAuthentication();
            } else {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Authorization Failed '%d'"), retval);
                SetStateError();
            }
        } else if (ERR_RETRY != retval) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Connection Failed '%d'"), retval);
            SetStateError();
        }
    } else if (IsConnectEventSignaled() || m_bReconnectOnError) {
        if ((m_bForceReconnect) ||
             (!IsConnected() && m_bReconnectOnError) 
        ) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Resetting Document State"));
            m_pDocument->ResetState();
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Setting connection state to reconnecting"));
            SetStateReconnecting();
        }

        if (!IsConnected()) {
            // determine computer name and password to use.
            if (!m_strNewComputerName.empty()) {
                strComputer = m_strNewComputerName;
                strComputerPassword = m_strNewComputerPassword;
            } else {
                if (!m_strConnectedComputerName.empty()) {
                    strComputer = m_strConnectedComputerName.c_str();
                    strComputerPassword = m_strConnectedComputerPassword.c_str();
                }
            }

            if (strComputer.empty()) {
                retval = m_pDocument->rpc.init_asynch(NULL, 60., true);
            } else {
                retval = m_pDocument->rpc.init_asynch(strComputer.c_str(), 60., false);
            }

            if (!retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Initialization Called"));
            } else {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Initialization Failed '%d'"), retval);
                SetStateError();
            }
        }
    }

    return NULL;
}


int CNetworkConnection::FrameShutdownDetected() {
    m_bFrameShutdownDetected = true;
    return 0;
}

int CNetworkConnection::GetConnectedComputerName(wxString& strMachine) {
    strMachine = m_strConnectedComputerName;
    return 0;
}


int CNetworkConnection::GetConnectingComputerName(wxString& strMachine) {
    strMachine = m_strNewComputerName;
    return 0;
}


int CNetworkConnection::SetNewComputerName(const wxChar* szComputer) {
    m_strNewComputerName = szComputer;
    return 0;
}


int CNetworkConnection::SetNewComputerPassword(const wxChar* szPassword) {
    m_strNewComputerPassword = szPassword;
    return 0;
}


void CNetworkConnection::SetStateErrorAuthentication() {
    CMainFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        m_bConnected = false;
        m_bReconnecting = false;
        m_bReconnectOnError = false;

        m_bConnectEvent = false;

        pFrame->ShowAlert(
            _("BOINC Manager - Connection Error"),
            _("The password you have provided is incorrect, please try again."),
            wxICON_ERROR
        );
    }
}


void CNetworkConnection::SetStateError() {
    CMainFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        m_bConnected = false;
        m_bReconnecting = false;
        m_bReconnectOnError = false;

        m_bConnectEvent = false;

        pFrame->ShowAlert(
            _("BOINC Manager - Connection failed"),
            _("Connection failed."),
            wxICON_ERROR
        );
    }
}


void CNetworkConnection::SetStateReconnecting() {
    CMainFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        m_bConnected = false;
        m_bReconnectOnError = false;
        m_bForceReconnect = false;
        m_bReconnecting = true;
    }
}


void CNetworkConnection::SetStateSuccess(std::string& strComputer, std::string& strComputerPassword) {
    CMainFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        m_bConnected = true;
        m_bReconnecting = false;
        m_bReconnectOnError = true;
        m_strConnectedComputerName = strComputer.c_str();
        m_strConnectedComputerPassword = strComputerPassword.c_str();
        m_strNewComputerName = wxEmptyString;
        m_strNewComputerPassword = wxEmptyString;

        m_bConnectEvent = false;

        pFrame->FireConnect();
    }
}


void CNetworkConnection::SetStateDisconnected() {
    CMainFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        m_bConnected = false;
    }
}


IMPLEMENT_DYNAMIC_CLASS(CMainDocument, wxObject)


CMainDocument::CMainDocument() {
#ifdef __WIN32__
    int retval;
    WSADATA wsdata;

    retval = WSAStartup(MAKEWORD(1, 1), &wsdata);
    if (retval) {
        wxLogTrace(wxT("Function Status"), "CMainDocument::CMainDocument - Winsock Initialization Failure '%d'", retval);
    }
#endif

    m_iCachedActivityRunMode = 0;
    m_iCachedNetworkRunMode = 0;

    m_fProjectTotalResourceShare = 0.0;

    m_iMessageSequenceNumber = 0;

    m_bCachedStateLocked = false;
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    m_dtCachedActivityStateTimestamp = wxDateTime((time_t)0);
    m_dtCachedActivityRunModeTimestamp = wxDateTime((time_t)0);
    m_dtCachedNetworkRunModeTimestamp = wxDateTime((time_t)0);
}


CMainDocument::~CMainDocument() {
    // ??? huh?
    m_dtCachedNetworkRunModeTimestamp = wxDateTime::Now();
    m_dtCachedActivityRunModeTimestamp = wxDateTime::Now();
    m_dtCachedActivityStateTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime::Now();
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_bCachedStateLocked = false;

    m_iMessageSequenceNumber = 0;

    m_fProjectTotalResourceShare = 0.0;

    m_iCachedActivityRunMode = 0;
    m_iCachedNetworkRunMode = 0;

#ifdef __WIN32__
    WSACleanup();
#endif
}


int CMainDocument::CachedStateUpdate() {
    CMainFrame* pFrame = wxGetApp().GetFrame();
    int     retval = 0;

    wxTimeSpan ts(m_dtCachedStateLockTimestamp - m_dtCachedStateTimestamp);
    if (!m_bCachedStateLocked && IsConnected() && (ts.GetSeconds() > 3600)) {
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));
        pFrame->UpdateStatusText(_("Retrieving system state; please wait..."));

        m_dtCachedStateTimestamp = m_dtCachedStateLockTimestamp;
        retval = rpc.get_state(state);
        if (retval) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedStateUpdate - Get State Failed '%d'", retval);
            m_pNetworkConnection->SetStateDisconnected();
        }

        pFrame->UpdateStatusText(_("Retrieving host information; please wait..."));

        retval = rpc.get_host_info(host);
        if (retval) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedStateUpdate - Get Host Information Failed '%d'", retval);
            m_pNetworkConnection->SetStateDisconnected();
        }

        pFrame->UpdateStatusText(wxEmptyString);
    }

    return retval;
}


int CMainDocument::OnInit() {
    int iRetVal = -1;

    // start the connect management thread
    m_pNetworkConnection = new CNetworkConnection(this);

    return iRetVal;
}


int CMainDocument::OnExit() {
    int iRetVal = 0;

    if (m_pNetworkConnection) {
        delete m_pNetworkConnection;
        m_pNetworkConnection = NULL;
    }

    return iRetVal;
}


int CMainDocument::OnPoll() {
    int iRetVal = 0;

    if (m_pNetworkConnection) {
        m_pNetworkConnection->Poll();
    }

    return iRetVal;
}


int CMainDocument::OnRefreshState() {
    if (IsConnected())
        CachedStateUpdate();

    return 0;
}


int CMainDocument::ResetState() {
    rpc.close();
    state.clear();
    host.clear();
    project_status.clear();
    results.clear();
    messages.clear();
    ft.clear();
    resource_status.clear();
    proxy_info.clear();
    
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime((time_t)0);

    m_iMessageSequenceNumber = 0;
    return 0;
}


int CMainDocument::Connect(const wxChar* szComputer, const wxChar* szComputerPassword, bool bDisconnect) {

    if (bDisconnect) {
        m_pNetworkConnection->ForceReconnect();
    }

    m_pNetworkConnection->SetNewComputerName(szComputer);
    m_pNetworkConnection->SetNewComputerPassword(szComputerPassword);

    m_pNetworkConnection->FireReconnectEvent();
    return 0;
}


int CMainDocument::GetConnectedComputerName(wxString& strMachine) {
    m_pNetworkConnection->GetConnectedComputerName(strMachine);
    return 0;
}


int CMainDocument::GetConnectingComputerName(wxString& strMachine) {
    m_pNetworkConnection->GetConnectingComputerName(strMachine);
    return 0;
}


bool CMainDocument::IsConnected() {
    return m_pNetworkConnection->IsConnected();
}


bool CMainDocument::IsReconnecting() {
    return m_pNetworkConnection->IsReconnecting();
}


int CMainDocument::CachedStateLock() {
    m_bCachedStateLocked = true;
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    return 0;
}


int CMainDocument::CachedStateUnlock() {
    m_bCachedStateLocked = false;
    return 0;
}


int CMainDocument::FrameShutdownDetected() {
    return m_pNetworkConnection->FrameShutdownDetected();
}

int CMainDocument::GetCoreClientVersion() {
    return rpc.client_version;
}


int CMainDocument::GetActivityRunMode(int& iMode) {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtCachedActivityRunModeTimestamp);
        if (ts.GetSeconds() > 10) {
            m_dtCachedActivityRunModeTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_run_mode(iMode);
            if (0 == iRetVal) {
                m_iCachedActivityRunMode = iMode;
            }
        } else {
            iMode = m_iCachedActivityRunMode;
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


int CMainDocument::SetActivityRunMode(int iMode) {
    int     iRetVal = 0;

    if (IsConnected()) {
        iRetVal = rpc.set_run_mode(iMode);
        if (0 == iRetVal) {
            m_dtCachedActivityRunModeTimestamp = wxDateTime::Now();
            m_iCachedActivityRunMode = iMode;
        }
    }

    return iRetVal;
}


int CMainDocument::GetNetworkRunMode(int& iMode) {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtCachedNetworkRunModeTimestamp);
        if (ts.GetSeconds() > 10) {
            m_dtCachedNetworkRunModeTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_network_mode(iMode);
            if (0 == iRetVal) {
                m_iCachedNetworkRunMode = iMode;
            }
        } else {
            iMode = m_iCachedNetworkRunMode;
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


int CMainDocument::SetNetworkRunMode(int iMode) {
    int     iRetVal = 0;

    if (IsConnected()) {
        iRetVal = rpc.set_network_mode(iMode);
        if (0 == iRetVal) {
            m_dtCachedNetworkRunModeTimestamp = wxDateTime::Now();
            m_iCachedNetworkRunMode = iMode;
        }
    }

    return iRetVal;
}


int CMainDocument::GetActivityState(bool& bActivitiesSuspended, bool& bNetworkSuspended) {
    int     iRetVal = 0;

    wxTimeSpan ts(wxDateTime::Now() - m_dtCachedActivityStateTimestamp);
    if (ts.GetSeconds() > 10) {
        m_dtCachedActivityStateTimestamp = wxDateTime::Now();

        if (IsConnected()) {
            iRetVal = rpc.get_activity_state(bActivitiesSuspended, bNetworkSuspended);
            if (0 == iRetVal) {
                m_iCachedActivitiesSuspended = bActivitiesSuspended;
                m_iCachedNetworkSuspended = bNetworkSuspended;
            }
        }
    } else {
        bActivitiesSuspended = m_iCachedActivitiesSuspended;
        bNetworkSuspended = m_iCachedNetworkSuspended;
    }

    return iRetVal;
}


int CMainDocument::ForceCacheUpdate() {
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    return 0;
}


int CMainDocument::RunBenchmarks() {
    return rpc.run_benchmarks();
}


int CMainDocument::CoreClientQuit() {
    return rpc.quit();
}


int CMainDocument::CachedProjectStatusUpdate() {
    int     iRetVal = 0;
    int i = 0;

    if (IsConnected()) {
        iRetVal = rpc.get_project_status(project_status);
        if (iRetVal) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedProjectStatusUpdate - Get Project Status Failed '%d'", iRetVal);
            m_pNetworkConnection->SetStateDisconnected();
        }

        m_fProjectTotalResourceShare = 0.0;
        for (i=0; i < (long)project_status.projects.size(); i++) {
            m_fProjectTotalResourceShare += project_status.projects.at(i)->resource_share;
        }
    }

    return iRetVal;
}


PROJECT* CMainDocument::project(unsigned int i) {
    PROJECT* pProject = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!project_status.projects.empty())
            pProject = project_status.projects.at(i);
    }
    catch (std::out_of_range e) {
        pProject = NULL;
    }

    return pProject;
}


int CMainDocument::GetProjectCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedProjectStatusUpdate();

    if (!project_status.projects.empty())
        iCount = project_status.projects.size();

    return iCount;
}


int CMainDocument::ProjectAttach(const wxString& strURL, const wxString& strAccountKey) {
    return rpc.project_attach(strURL.c_str(), strAccountKey.c_str());
}


int CMainDocument::ProjectDetach(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("detach"));

    return iRetVal;
}


int CMainDocument::ProjectUpdate(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("update"));

    return iRetVal;
}


int CMainDocument::ProjectReset(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("reset"));

    return iRetVal;
}


int CMainDocument::ProjectSuspend(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("suspend"));

    return iRetVal;
}


int CMainDocument::ProjectResume(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("resume"));

    return iRetVal;
}

int CMainDocument::ProjectNoMoreWork(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("nomorework"));

    return iRetVal;
}

int CMainDocument::ProjectAllowMoreWork(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("allowmorework"));

    return iRetVal;
}


int CMainDocument::CachedResultsStatusUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        iRetVal = rpc.get_results(results);
        if (iRetVal) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedResultsStatusUpdate - Get Result Status Failed '%d'", iRetVal);
            m_pNetworkConnection->SetStateDisconnected();
        }
    }

    return iRetVal;
}


RESULT* CMainDocument::result(unsigned int i) {
    RESULT* pResult = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!results.results.empty())
            pResult = results.results.at(i);
    }
    catch (std::out_of_range e) {
        pResult = NULL;
    }

    return pResult;
}


int CMainDocument::GetWorkCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedResultsStatusUpdate();

    if (!results.results.empty())
        iCount = results.results.size();

    return iCount;
}


int CMainDocument::WorkSuspend(int iIndex) {
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    int iRetVal = 0;

    pResult = result(iIndex);

    if (pResult) {
        pStateResult = state.lookup_result(pResult->project_url, pResult->name);
        if (pStateResult) {
            iRetVal = rpc.result_op((*pStateResult), wxT("suspend"));
        } else {
            ForceCacheUpdate();
        }
    }

    return iRetVal;
}


int CMainDocument::WorkResume(int iIndex) {
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    int iRetVal = 0;

    pResult = result(iIndex);

    if (pResult) {
        pStateResult = state.lookup_result(pResult->project_url, pResult->name);
        if (pStateResult) {
            iRetVal = rpc.result_op((*pStateResult), wxT("resume"));
        } else {
            ForceCacheUpdate();
        }
    }

    return iRetVal;
}


int CMainDocument::WorkShowGraphics(int iIndex, bool bFullScreen,
    std::string WindowStation, std::string Desktop, std::string Display) {
    RESULT* pResult = NULL;
    int iRetVal = 0;

    pResult = result(iIndex);

    if (pResult) {
        DISPLAY_INFO di;
        strcpy(di.window_station, WindowStation.c_str());
        strcpy(di.desktop, Desktop.c_str());
        strcpy(di.display, Display.c_str());

        iRetVal = rpc.show_graphics(
            pResult->project_url.c_str(), pResult->name.c_str(),
            bFullScreen, di
      );
    }

    return iRetVal;
}


int CMainDocument::WorkAbort(int iIndex) {
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    int iRetVal = 0;

    pResult = result(iIndex);

    if (pResult) {
        pStateResult = state.lookup_result(pResult->project_url, pResult->name);
        if (pStateResult) {
            iRetVal = rpc.result_op((*pStateResult), wxT("abort"));
        } else {
            ForceCacheUpdate();
        }
    }

    return iRetVal;
}


int CMainDocument::CachedMessageUpdate() {
    int retval;
    static bool in_this_func = false;

    if (in_this_func) return 0;
    in_this_func = true;

    if (IsConnected()) {
        CMainFrame* pFrame = wxGetApp().GetFrame();
        MESSAGES new_msgs;

        wxASSERT(pFrame);
        wxASSERT(wxDynamicCast(pFrame, CMainFrame));

        retval = rpc.get_messages(m_iMessageSequenceNumber, new_msgs);
        if (retval) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'", retval);
            m_pNetworkConnection->SetStateDisconnected();
            goto done;
        }
        std::vector<MESSAGE*>::iterator mi = new_msgs.messages.begin();
        while (mi != new_msgs.messages.end()) {
            MESSAGE* mp = *mi;
            m_iMessageSequenceNumber = mp->seqno;
            switch(mp->priority) {
            case MSG_PRIORITY_ALERT_ERROR:
                if (m_iMessageSequenceNumber) {
                    pFrame->ShowAlert(
                        _("BOINC error"),
                        mp->body.c_str(),
                        wxICON_ERROR
                    );
                }
                delete mp;
                break;
            case MSG_PRIORITY_ALERT_INFO:
                if (m_iMessageSequenceNumber) {
                    pFrame->ShowAlert(
                        _("BOINC info"),
                        mp->body.c_str(),
                        wxICON_INFORMATION
                    );
                }
                delete mp;
                break;
            default:
                messages.messages.push_back(mp);
            }
            new_msgs.messages.erase(mi);
        }
    }
done:
    in_this_func = false;
    return 0;
}


MESSAGE* CMainDocument::message(unsigned int i) {
    MESSAGE* pMessage = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!messages.messages.empty())
            pMessage = messages.messages.at(i);
    }
    catch (std::out_of_range e) {
        pMessage = NULL;
    }

    return pMessage;
}


int CMainDocument::GetMessageCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedMessageUpdate();

    if (!messages.messages.empty())
        iCount = messages.messages.size();

    return iCount;
}


int CMainDocument::ResetMessageState() {
    messages.clear();
    m_iMessageSequenceNumber = 0;
    return 0;
}


int CMainDocument::CachedFileTransfersUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        iRetVal = rpc.get_file_transfers(ft);
        if (iRetVal) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedFileTransfersUpdate - Get File Transfers Failed '%d'", iRetVal);
            m_pNetworkConnection->SetStateDisconnected();
        }
    }

    return iRetVal;
}


FILE_TRANSFER* CMainDocument::file_transfer(unsigned int i) {
    FILE_TRANSFER* pFT = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!ft.file_transfers.empty())
            pFT = ft.file_transfers.at(i);
    }
    catch (std::out_of_range e) {
        pFT = NULL;
    }

    return pFT;
}


int CMainDocument::GetTransferCount() {
    int iCount = 0;

    CachedStateUpdate();
    CachedFileTransfersUpdate();

    if (!ft.file_transfers.empty())
        iCount = ft.file_transfers.size();

    return iCount;
}


int CMainDocument::TransferRetryNow(int iIndex) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(iIndex);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), wxT("retry"));

    return iRetVal;
}


int CMainDocument::TransferAbort(int iIndex) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(iIndex);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), wxT("abort"));

    return iRetVal;
}


int CMainDocument::CachedResourceStatusUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        iRetVal = rpc.get_disk_usage(resource_status);
        if (iRetVal) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedResourceStatusUpdate - Get Disk Usage Failed '%d'", iRetVal);
            m_pNetworkConnection->SetStateDisconnected();
        }
    }

    return iRetVal;
}


PROJECT* CMainDocument::resource(unsigned int i) {
    PROJECT* pProject = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!resource_status.projects.empty()) {
            pProject = resource_status.projects.at(i);
        }
    }
    catch (std::out_of_range e) {
        pProject = NULL;
    }

    return pProject;
}


int CMainDocument::GetResourceCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedResourceStatusUpdate();

    if (!resource_status.projects.empty())
        iCount = resource_status.projects.size();

    return iCount;
}


int CMainDocument::CachedStatisticsStatusUpdate() {
    int     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    if (IsConnected()) {
        iRetVal = rpc.get_statistics(statistics_status);
        if (iRetVal) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedStatisticsStatusUpdate - Get Statistics Failed '%d'", iRetVal);
            m_pNetworkConnection->SetStateDisconnected();
        }
    }

    return iRetVal;
}


PROJECT* CMainDocument::statistic(unsigned int i) {
    PROJECT* pProject = NULL;


    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!statistics_status.projects.empty())
            pProject = statistics_status.projects.at(i);
    }
    catch (std::out_of_range e) {
        pProject = NULL;
    }


    return pProject;
}


int CMainDocument::GetStatisticsCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedStatisticsStatusUpdate();

    if (!statistics_status.projects.empty())
        iCount = statistics_status.projects.size();

    return iCount;
}


int CMainDocument::GetProxyConfiguration() {
    int     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_proxy_settings(proxy_info);
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), "CMainDocument::GetProxyInfo - Get Proxy Info Failed '%d'", iRetVal);
    }

    return iRetVal;
}


int CMainDocument::SetProxyConfiguration() {
    int     iRetVal = 0;

    if (!proxy_info.http_user_name.empty() || !proxy_info.http_user_passwd.empty())
        proxy_info.use_http_authentication = true;

    proxy_info.socks_version = 4;
    if (!proxy_info.socks5_user_name.empty() || !proxy_info.socks5_user_passwd.empty())
        proxy_info.socks_version = 5;

    iRetVal = rpc.set_proxy_settings(proxy_info);
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), "CMainDocument::SetProxyInfo - Set Proxy Info Failed '%d'", iRetVal);
    }

    return iRetVal;
}


const char *BOINC_RCSID_aa03a835ba = "$Id$";
