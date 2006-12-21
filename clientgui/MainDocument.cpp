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

#include "error_numbers.h"
#include "util.h"

#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"

#ifdef SANDBOX
#include <grp.h>
#include "util.h"        // For g_use_sandbox
#endif

using std::string;

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


void CNetworkConnection::GetLocalPassword(wxString& strPassword){
    char buf[256];

    FILE* f = fopen("gui_rpc_auth.cfg", "r");
    if (!f) return;
    strcpy(buf, "");
    fgets(buf, 256, f);
    fclose(f);
    int n = (int)strlen(buf);
    if (n) {
        n--;
        if (buf[n]=='\n') {
            buf[n] = 0;
        }
    }
    strPassword = wxString(buf, wxConvUTF8);
}


void CNetworkConnection::Poll() {
    int retval;
    wxString strComputer = wxEmptyString;
    wxString strComputerPassword = wxEmptyString;

    if (IsReconnecting()) {
        wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Reconnection Detected"));
        retval = m_pDocument->rpc.init_poll();
        if (!retval) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - init_poll() returned ERR_CONNECT, now authorizing..."));

            // Wait until we can establish a connection to the core client before reading
            //   the password so that the client has time to create one when it needs to.
            if (m_bUseDefaultPassword) {
                GetLocalPassword(m_strNewComputerPassword);
                m_bUseDefaultPassword = FALSE;
            }

            retval = m_pDocument->rpc.authorize(m_strNewComputerPassword.mb_str());
            if (!retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Connection Success"));
                SetStateSuccess(m_strNewComputerName, m_strNewComputerPassword);
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
        if ((m_bForceReconnect) || (!IsConnected() && m_bReconnectOnError)) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Resetting Document State"));
            m_pDocument->ResetState();
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Setting connection state to reconnecting"));
            SetStateReconnecting();
        }

        if (!IsConnected()) {
            // determine computer name and password to use.
            // NOTE: Initial connection case.
            if (!m_strNewComputerName.empty()) {
                strComputer = m_strNewComputerName;
                strComputerPassword = m_strNewComputerPassword;
            } else {
                // NOTE: Reconnect after a disconnect case.
                //       Values are stored after the first successful connect to the host.
                //       See: SetStateSuccess()
                if (!m_strConnectedComputerName.empty()) {
                    strComputer = m_strConnectedComputerName;
                    strComputerPassword = m_strConnectedComputerPassword;
                }
            }

            // a host value of NULL is special cased as binding to the localhost and
            //   if we are connecting to the localhost we need to retry the connection
            //   for awhile so that the users can respond to firewall prompts.
            if (IsComputerNameLocal(strComputer)) {
                retval = m_pDocument->rpc.init_asynch(NULL, 60., true);
            } else {
                retval = m_pDocument->rpc.init_asynch(strComputer.mb_str(), 60., false);
            }

            if (!retval) {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Initialization Called"));
            } else {
                wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - RPC Initialization Failed '%d'"), retval);
                SetStateError();
            }
        }
    }
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


bool CNetworkConnection::IsComputerNameLocal(const wxString& strMachine) {
    if (strMachine.empty()) {
        return true;
    } else if (wxT("localhost") == strMachine.Lower()) {
        return true;
    } else if (wxT("localhost.localdomain") == strMachine.Lower()) {
        return true;
    } else if (::wxGetHostName().Lower() == strMachine.Lower()) {
        return true;
    } else if (::wxGetFullHostName().Lower() == strMachine.Lower()) {
        return true;
    }
    return false;
}


int CNetworkConnection::SetComputer(const wxChar* szComputer, const wxChar* szPassword,  const bool bUseDefaultPassword) {
    m_strNewComputerName.Empty();
    m_strNewComputerPassword.Empty();
    m_bUseDefaultPassword = FALSE;

    m_strNewComputerName = szComputer;
    m_strNewComputerPassword = szPassword;
    m_bUseDefaultPassword = bUseDefaultPassword;
    return 0;
}


void CNetworkConnection::SetStateErrorAuthentication() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnecting = false;
        m_bReconnectOnError = false;

        m_bConnectEvent = false;

        pFrame->ShowConnectionBadPasswordAlert();
    }
}


void CNetworkConnection::SetStateError() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnecting = false;
        m_bReconnectOnError = false;

        m_bConnectEvent = false;

        pFrame->ShowConnectionFailedAlert();
    }
}


void CNetworkConnection::SetStateReconnecting() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnectOnError = false;
        m_bForceReconnect = false;
        m_bReconnecting = true;
    }
}


void CNetworkConnection::SetStateSuccess(wxString& strComputer, wxString& strComputerPassword) {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = true;
        m_bReconnecting = false;
        m_bReconnectOnError = true;
        m_strConnectedComputerName = strComputer;
        m_strConnectedComputerPassword = strComputerPassword;
        m_strNewComputerName = wxEmptyString;
        m_strNewComputerPassword = wxEmptyString;

        m_bConnectEvent = false;

        pFrame->FireConnect();
    }
}


void CNetworkConnection::SetStateDisconnected() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnecting = false;
    }
}


IMPLEMENT_DYNAMIC_CLASS(CMainDocument, wxObject)


CMainDocument::CMainDocument() {
#ifdef __WIN32__
    int retval;
    WSADATA wsdata;

    retval = WSAStartup(MAKEWORD(1, 1), &wsdata);
    if (retval) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CMainDocument - Winsock Initialization Failure '%d'"), retval);
    }
#endif

    m_fProjectTotalResourceShare = 0.0;

    m_iMessageSequenceNumber = 0;

    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    m_dtCachedCCStatusTimestamp = wxDateTime((time_t)0);
    m_dtProjecStatusTimestamp = wxDateTime((time_t)0);
    m_dtResultsTimestamp = wxDateTime((time_t)0);
    m_dtFileTransfersTimestamp = wxDateTime((time_t)0);
    m_dtDiskUsageTimestamp = wxDateTime((time_t)0);
    m_dtStatisticsStatusTimestamp = wxDateTime((time_t)0);
	m_dtCachedSimpleGUITimestamp = wxDateTime((time_t)0);
}


CMainDocument::~CMainDocument() {
#ifdef __WIN32__
    WSACleanup();
#endif
}


int CMainDocument::CachedStateUpdate() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::CachedStateUpdate - Function Begin"));

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    int     retval = 0;

    wxTimeSpan ts(wxDateTime::Now() - m_dtCachedStateTimestamp);
    if (IsConnected() && (ts.GetSeconds() > 3600)) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        pFrame->UpdateStatusText(_("Retrieving system state; please wait..."));

        m_dtCachedStateTimestamp = wxDateTime::Now();
        retval = rpc.get_state(state);
        if (retval) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedStateUpdate - Get State Failed '%d'"), retval);
            m_pNetworkConnection->SetStateDisconnected();
        }
        pFrame->UpdateStatusText(_("Retrieving host information; please wait..."));

        retval = rpc.get_host_info(host);
        if (retval) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedStateUpdate - Get Host Information Failed '%d'"), retval);
            m_pNetworkConnection->SetStateDisconnected();
        }

        pFrame->UpdateStatusText(wxEmptyString);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::CachedStateUpdate - Function End"));
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
    if (IsConnected()) {
        CachedStateUpdate();
    }

    return 0;
}


int CMainDocument::ResetState() {
    rpc.close();
    state.clear();
    host.clear_host_info();
    results.clear();
    messages.clear();
    ft.clear();
    disk_usage.clear();
    proxy_info.clear();
    
    ForceCacheUpdate();

    m_iMessageSequenceNumber = 0;
    return 0;
}


int CMainDocument::Connect(const wxChar* szComputer, const wxChar* szComputerPassword, const bool bDisconnect, const bool bUseDefaultPassword) {
    if (bDisconnect) {
        m_pNetworkConnection->ForceReconnect();
    }

    m_pNetworkConnection->SetComputer(szComputer, szComputerPassword, bUseDefaultPassword);
    m_pNetworkConnection->FireReconnectEvent();
    return 0;
}


int CMainDocument::Reconnect() {
    m_pNetworkConnection->ForceReconnect();
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


bool CMainDocument::IsComputerNameLocal(const wxString strMachine) {
    return m_pNetworkConnection->IsComputerNameLocal(strMachine);
}


bool CMainDocument::IsConnected() {
    return m_pNetworkConnection->IsConnected();
}


bool CMainDocument::IsReconnecting() {
    return m_pNetworkConnection->IsReconnecting();
}


int CMainDocument::FrameShutdownDetected() {
    return m_pNetworkConnection->FrameShutdownDetected();
}


int CMainDocument::GetCoreClientStatus(CC_STATUS& ccs, bool bForce) {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtCachedCCStatusTimestamp);
        if ((ts.GetSeconds() > 0) || bForce) {
            m_dtCachedCCStatusTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_cc_status(ccs);
            if (0 == iRetVal) {
                status = ccs;

                if (ccs.manager_must_quit) {
                    // TODO:
                    // if client is remote, don't do anything
                    // otherwise notify user and exit
                    exit(0);
                }
            }
        } else {
            ccs = status;
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


int CMainDocument::SetActivityRunMode(int iMode, int iTimeout) {
    int       iRetVal = 0;
    CC_STATUS ccs;

    if (IsConnected()) {
        iRetVal = rpc.set_run_mode(iMode, iTimeout);
        if (0 == iRetVal) {
            if (RUN_MODE_RESTORE == iMode) {
                GetCoreClientStatus(ccs, true);
            } else {
                status.task_mode = iMode;
            }
        }
    }

    return iRetVal;
}


int CMainDocument::SetNetworkRunMode(int iMode, int iTimeout) {
    int       iRetVal = 0;
    CC_STATUS ccs;

    if (IsConnected()) {
        iRetVal = rpc.set_network_mode(iMode, iTimeout);
        if (0 == iRetVal) {
            if (RUN_MODE_RESTORE == iMode) {
                GetCoreClientStatus(ccs, true);
            } else {
                status.network_mode = iMode;
            }
        }
    }

    return iRetVal;
}


int CMainDocument::ForceCacheUpdate() {
    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    CachedStateUpdate();
    return 0;
}


int CMainDocument::RunBenchmarks() {
    return rpc.run_benchmarks();
}


int CMainDocument::CoreClientQuit() {
    return rpc.quit();
}


bool CMainDocument::IsUserAuthorized() {
#ifdef _WIN32
    return true;
#else       // !  _WIN32
    static bool         sIsAuthorized = false;

    if (sIsAuthorized)
        return true;            // We already checked and OK'd current user

#ifdef SANDBOX
    group               *grp;
    gid_t               rgid, boinc_master_gid;
    char                *userName, *groupMember;
    int                 i;

    if (g_use_sandbox) {
            
        grp = getgrnam(BOINC_MASTER_GROUP_NAME);
        if (grp) {
            boinc_master_gid = grp->gr_gid;

            rgid = getgid();
            if (rgid == boinc_master_gid) {
                sIsAuthorized = true;           // User's primary group is boinc_master
                return true;
            }

            userName = getlogin();
            if (userName) {
                for (i=0; ; i++) {              // Step through all users in group boinc_master
                    groupMember = grp->gr_mem[i];
                    if (groupMember == NULL)
                        break;                  // User is not a member of group boinc_master
                    if (strcmp(userName, groupMember) == 0) {
                        sIsAuthorized = true;   // User is a member of group boinc_master
                        return true;
                    }
                }       // for (i)
            }           // if (userName)
        }               // if grp
    }
#endif      // SANDBOX
#endif      // !  _WIN32

#ifdef __WXMAC__
    if (Mac_Authorize()) {          // Run Mac Authentication dialog
        sIsAuthorized = true;       // Authenticated by password
        return true;
    }
#endif      // __WXMAC__
    
#ifdef SANDBOX
    return false;
#else
    return true;
#endif
}


int CMainDocument::CachedProjectStatusUpdate() {
    int     iRetVal = 0;
    int     i = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtProjecStatusTimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtProjecStatusTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_project_status(state);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedProjectStatusUpdate - Get Project Status Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }

            m_fProjectTotalResourceShare = 0.0;
            for (i=0; i < (long)state.projects.size(); i++) {
                m_fProjectTotalResourceShare += state.projects.at(i)->resource_share;
            }
        }
    } else {
        iRetVal = -1;
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
        if (!state.projects.empty())
            pProject = state.projects.at(i);
    }
    catch (std::out_of_range e) {
        pProject = NULL;
    }

    return pProject;
}


int CMainDocument::GetProjectCount() {
    int iCount = -1;

    CachedProjectStatusUpdate();
    CachedStateUpdate();

    if (!state.projects.empty())
        iCount = (int)state.projects.size();

    return iCount;
}


int CMainDocument::ProjectAttach(const wxString& strURL, const wxString& strAccountKey) {
    return rpc.project_attach((const char*)strURL.mb_str(), (const char*)strAccountKey.mb_str());
}


int CMainDocument::ProjectDetach(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "detach");

    return iRetVal;
}


int CMainDocument::ProjectUpdate(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "update");

    return iRetVal;
}


int CMainDocument::ProjectReset(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "reset");

    return iRetVal;
}


int CMainDocument::ProjectSuspend(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "suspend");

    return iRetVal;
}


int CMainDocument::ProjectResume(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "resume");

    return iRetVal;
}

int CMainDocument::ProjectNoMoreWork(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "nomorework");

    return iRetVal;
}

int CMainDocument::ProjectAllowMoreWork(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "allowmorework");

    return iRetVal;
}


int CMainDocument::CachedResultsStatusUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtResultsTimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtResultsTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_results(results);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedResultsStatusUpdate - Get Result Status Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
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

    CachedResultsStatusUpdate();
    CachedStateUpdate();

    if (!results.results.empty())
        iCount = (int)results.results.size();

    return iCount;
}


int CMainDocument::WorkSuspend(std::string& strProjectURL, std::string& strName) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(strProjectURL, strName);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "suspend");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


int CMainDocument::WorkResume(std::string& strProjectURL, std::string& strName) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(strProjectURL, strName);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "resume");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


int CMainDocument::WorkShowGraphics(
    std::string& strProjectURL, std::string& strName, int iGraphicsMode,
    std::string& strWindowStation, std::string& strDesktop, std::string& strDisplay)
{
    int iRetVal = 0;
    DISPLAY_INFO di;

    strcpy(di.window_station, strWindowStation.c_str());
    strcpy(di.desktop, strDesktop.c_str());
    strcpy(di.display, strDisplay.c_str());

    iRetVal = rpc.show_graphics(
        strProjectURL.c_str(),
        strName.c_str(),
        iGraphicsMode,
        di
    );

    return iRetVal;
}


int CMainDocument::WorkAbort(std::string& strProjectURL, std::string& strName) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(strProjectURL, strName);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "abort");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


int CMainDocument::CachedMessageUpdate() {
    int retval;
    static bool in_this_func = false;

    if (in_this_func) return 0;
    in_this_func = true;

    if (IsConnected()) {
        retval = rpc.get_messages(m_iMessageSequenceNumber, messages);
        if (retval) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'"), retval);
            m_pNetworkConnection->SetStateDisconnected();
            goto done;
        }
        if (messages.messages.size() != 0) {
            m_iMessageSequenceNumber = messages.messages.at(messages.messages.size()-1)->seqno;
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

    CachedMessageUpdate();
    CachedStateUpdate();

    if (!messages.messages.empty())
        iCount = (int)messages.messages.size();

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
        wxTimeSpan ts(wxDateTime::Now() - m_dtFileTransfersTimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtFileTransfersTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_file_transfers(ft);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedFileTransfersUpdate - Get File Transfers Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
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

    CachedFileTransfersUpdate();
    CachedStateUpdate();

    if (!ft.file_transfers.empty())
        iCount = (int)ft.file_transfers.size();

    return iCount;
}


int CMainDocument::TransferRetryNow(int iIndex) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(iIndex);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), "retry");

    return iRetVal;
}


int CMainDocument::TransferAbort(int iIndex) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(iIndex);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), "abort");

    return iRetVal;
}


int CMainDocument::CachedDiskUsageUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtDiskUsageTimestamp);

		// don't get disk usage more than once per minute
		//
        if (ts.GetSeconds() > 60) {
            m_dtDiskUsageTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_disk_usage(disk_usage);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("Get Disk Usage Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


PROJECT* CMainDocument::DiskUsageProject(unsigned int i) {
    PROJECT* pProject = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!disk_usage.projects.empty()) {
            pProject = disk_usage.projects.at(i);
        }
    }
    catch (std::out_of_range e) {
        pProject = NULL;
    }

    return pProject;
}

int CMainDocument::CachedStatisticsStatusUpdate() {
    int     iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtStatisticsStatusTimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtStatisticsStatusTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_statistics(statistics_status);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedStatisticsStatusUpdate - Get Statistics Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }
        }
    } else {
        iRetVal = -1;
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

    CachedStatisticsStatusUpdate();
    CachedStateUpdate();

    if (!statistics_status.projects.empty())
        iCount = (int)statistics_status.projects.size();

    return iCount;
}


int CMainDocument::GetProxyConfiguration() {
    int     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_proxy_settings(proxy_info);
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::GetProxyInfo - Get Proxy Info Failed '%d'"), iRetVal);
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
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::SetProxyInfo - Set Proxy Info Failed '%d'"), iRetVal);
    }

    return iRetVal;
}


int CMainDocument::CachedSimpleGUIUpdate() {
    int     iRetVal = 0;
    int     i = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtCachedSimpleGUITimestamp);
        if (ts.GetSeconds() > 0) {
            m_dtCachedSimpleGUITimestamp = wxDateTime::Now();

            iRetVal = rpc.get_simple_gui_info(state, results);
            if (iRetVal) {
                wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedSimpleGUIUpdate - Get Simple GUI Failed '%d'"), iRetVal);
                ForceCacheUpdate();
            }

            m_fProjectTotalResourceShare = 0.0;
            for (i=0; i < (long)state.projects.size(); i++) {
                m_fProjectTotalResourceShare += state.projects.at(i)->resource_share;
            }
        }
    } else {
        iRetVal = -1;
    }

    return iRetVal;
}


int CMainDocument::GetSimpleGUIWorkCount() {
    int iCount = 0;
    unsigned int i = 0;

    CachedSimpleGUIUpdate();
    CachedStateUpdate();

	for(i=0; i<results.results.size(); i++) {
		if (results.results[i]->active_task) {
			iCount++;
		}
	}
    return iCount;
}


const char *BOINC_RCSID_aa03a835ba = "$Id$";
