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

        pFrame->FireConnectErrorAuthentication();
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

        pFrame->FireConnectError();
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


int CMainDocument::ForceCacheUpdate() {
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    return 0;
}


int CMainDocument::OnInit() {
    int iRetVal = -1;

    // attempt to lookup account management information
    acct_mgr.init();

    // start the connect management thread
    m_pNetworkConnection = new CNetworkConnection(this);

    return iRetVal;
}


int CMainDocument::OnExit() {
    int iRetVal = 0;

    // attempt to cleanup the account management information
    acct_mgr.close();

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


int CMainDocument::GetProjectCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedProjectStatusUpdate();

    if (!project_status.projects.empty())
        iCount = project_status.projects.size();

    return iCount;
}


int CMainDocument::GetProjectProjectURL(int iIndex, wxString& strBuffer) {
    PROJECT* pProject = NULL;

    pProject = project(iIndex);

    if (pProject)
        strBuffer = pProject->master_url.c_str();

    return 0;
}


int CMainDocument::GetProjectAccountName(int iIndex, wxString& strBuffer) {
    PROJECT* pProject = NULL;

    pProject = project(iIndex);

    if (pProject)
        strBuffer = pProject->user_name.c_str();

    return 0;
}



int CMainDocument::GetProjectTotalCredit(int iIndex, float& fBuffer) {
    PROJECT* pProject = NULL;

    pProject = project(iIndex);

    if (pProject)
        fBuffer = pProject->user_total_credit;

    return 0;
}


int CMainDocument::GetProjectAvgCredit(int iIndex, float& fBuffer) {
    PROJECT* pProject = NULL;

    pProject = project(iIndex);

    if (pProject)
        fBuffer = pProject->user_expavg_credit;

    return 0;
}


int CMainDocument::GetProjectResourceShare(int iIndex, float& fBuffer) {
    PROJECT* pProject = NULL;

    pProject = project(iIndex);

    if (pProject)
        fBuffer = pProject->resource_share;

    return 0;
}


int CMainDocument::GetProjectTotalResourceShare(int WXUNUSED(iIndex), float& fBuffer) {
    fBuffer = m_fProjectTotalResourceShare;
    return 0;
}


int CMainDocument::GetProjectMinRPCTime(int iIndex, int& iBuffer) {
    PROJECT* pProject = NULL;

    pProject = project(iIndex);

    if (pProject)
        iBuffer = (int)pProject->min_rpc_time;

    return 0;
}


int CMainDocument::GetProjectWebsiteCount(int iIndex) {
    int     iCount = 0;
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject = NULL;

    CachedStateUpdate();

    GetProjectProjectURL(iIndex, strProjectURL);
    str = strProjectURL.c_str();
    pProject = state.lookup_project(str);

    if (pProject)
        iCount = pProject->gui_urls.size();

    return iCount;
}


int CMainDocument::GetProjectWebsiteName(int iProjectIndex, int iWebsiteIndex, wxString& strBuffer) {
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject      = NULL;
    GUI_URL     Url;

    CachedStateUpdate();

    GetProjectProjectURL(iProjectIndex, strProjectURL);
    str = strProjectURL.c_str();
    pProject = state.lookup_project(str);

    if (pProject) {
        Url = pProject->gui_urls.at(iWebsiteIndex);
        strBuffer = Url.name.c_str();
    }

    return 0;
}


int CMainDocument::GetProjectWebsiteDescription(int iProjectIndex, int iWebsiteIndex, wxString& strBuffer) {
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject      = NULL;
    GUI_URL     Url;

    CachedStateUpdate();

    GetProjectProjectURL(iProjectIndex, strProjectURL);
    str = strProjectURL.c_str();
    pProject = state.lookup_project(str);

    if (pProject) {
        Url = pProject->gui_urls.at(iWebsiteIndex);
        strBuffer = Url.description.c_str();
    }

    return 0;
}


int CMainDocument::GetProjectWebsiteLink(int iProjectIndex, int iWebsiteIndex, wxString& strBuffer) {
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject      = NULL;
    GUI_URL     Url;

    CachedStateUpdate();

    GetProjectProjectURL(iProjectIndex, strProjectURL);
    str = strProjectURL.c_str();
    pProject = state.lookup_project(str);

    if (pProject) {
        Url = pProject->gui_urls.at(iWebsiteIndex);
        strBuffer = Url.url.c_str();
    }

    return 0;
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
    PROJECT* pStateProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("suspend"));

    return iRetVal;
}


int CMainDocument::ProjectResume(int iIndex) {
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("resume"));

    return iRetVal;
}

int CMainDocument::ProjectNoMoreWork(int iIndex) {
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("nomorework"));

    return iRetVal;
}

int CMainDocument::ProjectAllowMoreWork(int iIndex) {
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), wxT("allowmorework"));

    return iRetVal;
}


PROJECT* CMainDocument::project(int i) {
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


int CMainDocument::GetWorkCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedResultsStatusUpdate();

    if (!results.results.empty())
        iCount = results.results.size();

    return iCount;
}


int CMainDocument::GetWorkProjectName(int iIndex, wxString& strBuffer) {
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    PROJECT* pProject = NULL;

    pResult = result(iIndex);

    if (pResult) {
        pStateResult = state.lookup_result(pResult->project_url, pResult->name);
        if (pStateResult) {
            pProject = pStateResult->project;
            if (pProject) {
                strBuffer = pProject->project_name.c_str();
            }
        } else {
            ForceCacheUpdate();
        }
    }

    return 0;
}


int CMainDocument::GetWorkProjectURL(int iIndex, wxString& strBuffer) {
    RESULT* pResult = NULL;

    pResult = result(iIndex);

    if (pResult)
        strBuffer = pResult->project_url.c_str();

    return 0;
}


int CMainDocument::GetWorkApplicationName(int iIndex, wxString& strBuffer) {
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    WORKUNIT* pWorkunit = NULL;
    APP_VERSION* pAppVersion = NULL;

    pResult = result(iIndex);

    if (pResult) {
        pStateResult = state.lookup_result(pResult->project_url, pResult->name);
        if (pStateResult) {
            pWorkunit = pStateResult->wup;
            if (pWorkunit) {
                pAppVersion = pWorkunit->avp;
                if (pAppVersion) {
                    strBuffer = pAppVersion->app_name.c_str();
                }
            }
        }
        else
            ForceCacheUpdate();
    }

    return 0;
}


int CMainDocument::GetWorkApplicationVersion(int iIndex, int& iBuffer) {
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    WORKUNIT* pWorkunit = NULL;
    APP_VERSION* pAppVersion = NULL;

    pResult = result(iIndex);

    if (pResult) {
        pStateResult = state.lookup_result(pResult->project_url, pResult->name);
        if (pStateResult) {
            pWorkunit = pStateResult->wup;
            if (pWorkunit) {
                pAppVersion = pWorkunit->avp;
                if (pAppVersion) {
                    iBuffer = pAppVersion->version_num;
                }
            }
        } else {
            ForceCacheUpdate();
        }
    }

    return 0;
}


int CMainDocument::GetWorkName(int iIndex, wxString& strBuffer) {
    RESULT* pResult = NULL;

    pResult = result(iIndex);

    if (pResult)
        strBuffer = pResult->name.c_str();

    return 0;
}


int CMainDocument::GetWorkCurrentCPUTime(int iIndex, float& fBuffer) {
    RESULT* pResult = NULL;

    pResult = result(iIndex);

    if (pResult)
        fBuffer = pResult->current_cpu_time;

    return 0;
}


int CMainDocument::GetWorkEstimatedCPUTime(int iIndex, float& fBuffer) {
    RESULT* pResult = NULL;

    pResult = result(iIndex);

    if (pResult)
        fBuffer = pResult->estimated_cpu_time_remaining;

    return 0;
}


int CMainDocument::GetWorkFinalCPUTime(int iIndex, float& fBuffer) {
    RESULT* pResult = NULL;

    pResult = result(iIndex);

    if (pResult)
        fBuffer = pResult->final_cpu_time;

    return 0;
}


int CMainDocument::GetWorkFractionDone(int iIndex, float& fBuffer) {
    RESULT* pResult = NULL;

    pResult = result(iIndex);

    if (pResult)
        fBuffer = pResult->fraction_done;

    return 0;
}


int CMainDocument::GetWorkReportDeadline(int iIndex, int& iBuffer) {
    RESULT* pResult = NULL;

    pResult = result(iIndex);

    if (pResult)
        iBuffer = pResult->report_deadline;

    return 0;
}

bool CMainDocument::IsWorkGraphicsSupported(int iIndex) {
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    pResult = result(iIndex);

    if (pResult)
        bRetVal = pResult->supports_graphics;

    return bRetVal;
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


RESULT* CMainDocument::result(int i) {
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


int CMainDocument::CachedMessageUpdate() {
    int     retval = 0;

    if (IsConnected()) {
        MESSAGES new_msgs;
        retval = rpc.get_messages(m_iMessageSequenceNumber, new_msgs);
        if (retval) {
            wxLogTrace(wxT("Function Status"), "CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'", retval);
            m_pNetworkConnection->SetStateDisconnected();
            return retval;
        }
        std::vector<MESSAGE*>::iterator mi = new_msgs.messages.begin();
        while (mi != new_msgs.messages.end()) {
            MESSAGE* mp = *mi;
            m_iMessageSequenceNumber = mp->seqno;
            if (mp->priority == MSG_PRIORITY_ALERT) {
                wxString foo = mp->body.c_str();
                wxMessageBox(foo, _("BOINC error"), wxOK | wxICON_ERROR);
                delete mp;
            } else {
                messages.messages.push_back(mp);
            }
            new_msgs.messages.erase(mi);
        }
    }

    return retval;
}


int CMainDocument::GetMessageCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedMessageUpdate();

    if (!messages.messages.empty())
        iCount = messages.messages.size();

    return iCount;
}


int CMainDocument::GetMessageProjectName(int iIndex, wxString& strBuffer) {
    MESSAGE* pMessage = NULL;

    pMessage = message(iIndex);

    if (pMessage)
        strBuffer = pMessage->project.c_str();

    return 0;
}


int CMainDocument::GetMessageTime(int iIndex, wxDateTime& dtBuffer) {
    MESSAGE* pMessage = NULL;

    pMessage = message(iIndex);

    if (pMessage) {
        wxDateTime dtTemp((time_t)pMessage->timestamp);
        dtBuffer = dtTemp;
    }

    return 0;
}


int CMainDocument::GetMessagePriority(int iIndex, int& iBuffer) {
    MESSAGE* pMessage = NULL;

    pMessage = message(iIndex);

    if (pMessage)
        iBuffer = pMessage->priority;

    return 0;
}


int CMainDocument::GetMessageMessage(int iIndex, wxString& strBuffer) {
    MESSAGE* pMessage = NULL;

    pMessage = message(iIndex);

    if (pMessage)
        strBuffer = pMessage->body.c_str();

    return 0;
}


int CMainDocument::ResetMessageState() {
    messages.clear();
    m_iMessageSequenceNumber = 0;
    return 0;
}


MESSAGE* CMainDocument::message(int i) {
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


int CMainDocument::GetTransferCount() {
    int iCount = 0;

    CachedStateUpdate();
    CachedFileTransfersUpdate();

    if (!ft.file_transfers.empty())
        iCount = ft.file_transfers.size();

    return iCount;
}


int CMainDocument::GetTransferProjectName(int iIndex, wxString& strBuffer) {
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        strBuffer = pFT->project_name.c_str();

    return 0;
}


int CMainDocument::GetTransferFileName(int iIndex, wxString& strBuffer) {
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        strBuffer = pFT->name.c_str();

    return 0;
}


int CMainDocument::GetTransferFileSize(int iIndex, float& fBuffer) {
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        fBuffer = pFT->nbytes;

    return 0;
}


int CMainDocument::GetTransferBytesXfered(int iIndex, float& fBuffer)
{
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        fBuffer = pFT->bytes_xferred;

    return 0;
}


int CMainDocument::GetTransferSpeed(int iIndex, float& fBuffer) {
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        fBuffer = pFT->xfer_speed;

    return 0;
}


int CMainDocument::GetTransferTime(int iIndex, float& fBuffer) {
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        fBuffer = pFT->time_so_far;

    return 0;
}


int CMainDocument::GetTransferNextRequestTime(int iIndex, int& iBuffer) {
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        iBuffer = pFT->next_request_time;

    return 0;
}


int CMainDocument::GetTransferStatus(int iIndex, int& iBuffer) {
    FILE_TRANSFER* pFT = NULL;

    pFT = file_transfer(iIndex);

    if (pFT)
        iBuffer = pFT->status;

    return 0;
}


bool CMainDocument::IsTransferActive(int iIndex) {
    FILE_TRANSFER* pFT = NULL;
    bool bRetVal    = false;

    pFT = file_transfer(iIndex);

    if (pFT)
        bRetVal = pFT->pers_xfer_active;

    return bRetVal;
}

bool CMainDocument::IsTransferGeneratedLocally(int iIndex) {
    FILE_TRANSFER* pFT = NULL;
    bool bRetVal    = false;

    pFT = file_transfer(iIndex);

    if (pFT)
        bRetVal = pFT->generated_locally;

    return bRetVal;
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


FILE_TRANSFER* CMainDocument::file_transfer(int i) {
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


int CMainDocument::GetResourceCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedResourceStatusUpdate();

    if (!resource_status.projects.empty())
        iCount = resource_status.projects.size();

    return iCount;
}


int CMainDocument::GetResourceProjectName(int iIndex, wxString& strBuffer) {
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;

    pProject = resource(iIndex);

    if (pProject) {
        pStateProject = state.lookup_project(pProject->master_url);
        if (pStateProject) {
            strBuffer = pStateProject->project_name.c_str();
        } else {
            ForceCacheUpdate();
        }
    }

    return 0;
}


int CMainDocument::GetResourceDiskspace(int iIndex, float& fBuffer) {
    PROJECT* pProject = NULL;

    pProject = resource(iIndex);

    if (pProject)
        fBuffer = pProject->disk_usage;

    return 0;
}


PROJECT* CMainDocument::resource(int i) {
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


int CMainDocument::GetStatisticsCount() {
    int iCount = -1;

    CachedStateUpdate();
    CachedStatisticsStatusUpdate();

    if (!statistics_status.projects.empty())
        iCount = statistics_status.projects.size();

    return iCount;
}


int CMainDocument::GetStatisticsProjectName(int iIndex, wxString& strBuffer)
{
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;

    pProject = statistic(iIndex);

    if (pProject) {
        pStateProject = state.lookup_project(pProject->master_url);
        if (pStateProject) {
            strBuffer = pStateProject->project_name.c_str();
        } else {
            ForceCacheUpdate();
        }
    }

    return 0;
}


PROJECT* CMainDocument::statistic(int i) {
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


int CMainDocument::GetProxyConfiguration() {
    int     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_proxy_settings(proxy_info);
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), "CMainDocument::GetProxyInfo - Get Proxy Info Failed '%d'", iRetVal);
    }

    return iRetVal;
}


int CMainDocument::GetProxyHTTPProxyEnabled(bool& bEnabled) {
    bEnabled = proxy_info.use_http_proxy;
    return 0;
}


int CMainDocument::GetProxyHTTPServerName(wxString& strServerName) {
    strServerName.Clear();
    strServerName = proxy_info.http_server_name.c_str();
    return 0;
}


int CMainDocument::GetProxyHTTPServerPort(int& iPortNumber) {
    iPortNumber = proxy_info.http_server_port;
    return 0;
}


int CMainDocument::GetProxyHTTPUserName(wxString& strUserName) {
    strUserName.Clear();
    strUserName = proxy_info.http_user_name.c_str();
    return 0;
}


int CMainDocument::GetProxyHTTPPassword(wxString& strPassword) {
    strPassword.Clear();
    strPassword = proxy_info.http_user_passwd.c_str();
    return 0;
}


int CMainDocument::GetProxySOCKSProxyEnabled(bool& bEnabled) {
    bEnabled = proxy_info.use_socks_proxy;
    return 0;
}


int CMainDocument::GetProxySOCKSServerName(wxString& strServerName) {
    strServerName.Clear();
    strServerName = proxy_info.socks_server_name.c_str();
    return 0;
}


int CMainDocument::GetProxySOCKSServerPort(int& iPortNumber) {
    iPortNumber = proxy_info.socks_server_port;
    return 0;
}


int CMainDocument::GetProxySOCKSUserName(wxString& strUserName) {
    strUserName.Clear();
    strUserName = proxy_info.socks5_user_name.c_str();
    return 0;
}


int CMainDocument::GetProxySOCKSPassword(wxString& strPassword) {
    strPassword.Clear();
    strPassword = proxy_info.socks5_user_passwd.c_str();
    return 0;
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


int CMainDocument::SetProxyHTTPProxyEnabled(const bool bEnabled) {
    proxy_info.use_http_proxy = bEnabled;
    return 0;
}


int CMainDocument::SetProxyHTTPServerName(const wxString& strServerName) {
    proxy_info.http_server_name = strServerName.c_str();
    return 0;
}


int CMainDocument::SetProxyHTTPServerPort(const int iPortNumber) {
    proxy_info.http_server_port = iPortNumber;
    return 0;
}


int CMainDocument::SetProxyHTTPUserName(const wxString& strUserName) {
    proxy_info.http_user_name = strUserName.c_str();
    return 0;
}


int CMainDocument::SetProxyHTTPPassword(const wxString& strPassword) {
    proxy_info.http_user_passwd = strPassword.c_str();
    return 0;
}


int CMainDocument::SetProxySOCKSProxyEnabled(const bool bEnabled) {
    proxy_info.use_socks_proxy = bEnabled;
    return 0;
}


int CMainDocument::SetProxySOCKSServerName(const wxString& strServerName) {
    proxy_info.socks_server_name = strServerName.c_str();
    return 0;
}


int CMainDocument::SetProxySOCKSServerPort(const int iPortNumber) {
    proxy_info.socks_server_port = iPortNumber;
    return 0;
}


int CMainDocument::SetProxySOCKSUserName(const wxString& strUserName) {
    proxy_info.socks5_user_name = strUserName.c_str();
    return 0;
}


int CMainDocument::SetProxySOCKSPassword(const wxString& strPassword) {
    proxy_info.socks5_user_passwd = strPassword.c_str();
    return 0;
}


int CMainDocument::GetAccountManagerName(wxString& strName) {
    strName.Clear();
    strName = acct_mgr.acct_mgr.name.c_str();
    return 0;
}


int CMainDocument::InitializeAccountManagerLogin(const wxString& strLogin, const wxString& strPassword) {
    acct_mgr.acct_mgr_login_initialized = true;
    acct_mgr.acct_mgr_login.login = strLogin.c_str();
    acct_mgr.acct_mgr_login.password = strPassword.c_str();
    return 0;
}


int CMainDocument::UpdateAccountManagerAccounts() {
    int     iRetVal = 0;

    iRetVal = rpc.acct_mgr_rpc(
        acct_mgr.acct_mgr.url.c_str(),
        acct_mgr.acct_mgr_login.login.c_str(),
        acct_mgr.acct_mgr_login.password.c_str()
    );
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), "CMainDocument::UpdateAccountManagerAccounts - Account Manager RPC Failed '%d'", iRetVal);
        m_pNetworkConnection->SetStateDisconnected();
    }

    return iRetVal;
}


bool CMainDocument::IsAccountManagerFound() {
    return acct_mgr.acct_mgr_found;    
}


bool CMainDocument::IsAccountManagerLoginFound() {
    return acct_mgr.acct_mgr_login_found;
}


const char *BOINC_RCSID_aa03a835ba = "$Id$";
