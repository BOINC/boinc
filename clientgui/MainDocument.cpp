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
#ifdef _WIN32
#else
#include <sys/wait.h>
#endif
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
    m_bNewConnection = false;
    m_bUsedDefaultPassword = false;
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
                m_bUsedDefaultPassword = true;
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
            m_bUsedDefaultPassword = false;
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
            //
            // use a timeout of 60 seconds so that slow machines do not get a
            //   timeout event right after boot-up.
            //
            if (IsComputerNameLocal(strComputer)) {
                retval = m_pDocument->rpc.init_asynch(NULL, 60.0, true);
            } else {
                retval = m_pDocument->rpc.init_asynch(strComputer.mb_str(), 60.0, false);
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


int CNetworkConnection::GetConnectedComputerVersion(wxString& strVersion) {
    strVersion = m_strConnectedComputerVersion;
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

    m_bNewConnection = true;
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

        pFrame->ShowConnectionBadPasswordAlert(m_bUsedDefaultPassword);
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
        if (!m_bNewConnection) {
            m_strNewComputerName = m_strConnectedComputerName;
            m_strNewComputerPassword = m_strConnectedComputerPassword;
        }
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
        m_bNewConnection = false;

        // Get the version of the client and cache it
        VERSION_INFO vi;
        m_pDocument->rpc.exchange_versions(vi);
        m_strConnectedComputerVersion.Printf(
            wxT("%d.%d.%d"),
            vi.major, vi.minor, vi.release
        );

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
    m_dtKillInactiveGfxTimestamp = wxDateTime((time_t)0);
    m_dtFileTransfersTimestamp = wxDateTime((time_t)0);
    m_dtDiskUsageTimestamp = wxDateTime((time_t)0);
    m_dtStatisticsStatusTimestamp = wxDateTime((time_t)0);
	m_dtCachedSimpleGUITimestamp = wxDateTime((time_t)0);
}


CMainDocument::~CMainDocument() {
    KillAllRunningGraphicsApps();
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

    // Every 10 seconds, kill any running graphics apps 
    // whose associated worker tasks are no longer running
    wxTimeSpan ts(wxDateTime::Now() - m_dtKillInactiveGfxTimestamp);
    if (ts.GetSeconds() > 10) {
        m_dtKillInactiveGfxTimestamp = wxDateTime::Now();
        KillInactiveGraphicsApps();
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


int CMainDocument::GetConnectedComputerVersion(wxString& strVersion) {
    m_pNetworkConnection->GetConnectedComputerVersion(strVersion);
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
    wxString         strMachine = wxEmptyString;
    int              iRetVal = 0;

    if (IsConnected()) {
        wxTimeSpan ts(wxDateTime::Now() - m_dtCachedCCStatusTimestamp);
        if ((ts.GetSeconds() > 0) || bForce) {
            m_dtCachedCCStatusTimestamp = wxDateTime::Now();

            iRetVal = rpc.get_cc_status(ccs);
            if (0 == iRetVal) {
                status = ccs;
                if (ccs.manager_must_quit) {
                    GetConnectedComputerName(strMachine);
                    if (IsComputerNameLocal(strMachine)) {
                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
                        pFrame->Close(true);
                    }
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
#ifndef _WIN32
#ifdef SANDBOX
    static bool         sIsAuthorized = false;
    group               *grp;
    gid_t               rgid, boinc_master_gid;
    char                *userName, *groupMember;
    int                 i;

    if (g_use_sandbox) {
        if (sIsAuthorized)
            return true;            // We already checked and OK'd current user

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

#ifdef __WXMAC__
        if (Mac_Authorize()) {          // Run Mac Authentication dialog
            sIsAuthorized = true;       // Authenticated by password
            return true;
        }
#endif      // __WXMAC__

        return false;

    }       // if (g_use_sandbox)
#endif      // SANDBOX
#endif      // #ifndef _WIN32

    return true;
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

PROJECT* CMainDocument::project(const wxString& projectname) {
	for (unsigned int i=0; i< state.projects.size(); i++) {
		PROJECT* tp = state.projects[i];
		wxString t1(tp->project_name.c_str(), wxConvUTF8);
		if(t1.IsSameAs(projectname)) return tp;
		wxString t2(tp->master_url.c_str(), wxConvUTF8);
		if(t2.IsSameAs(projectname)) return tp;
	}
    return NULL;
}


int CMainDocument::GetProjectCount() {
    int iCount = -1;

    CachedProjectStatusUpdate();
    CachedStateUpdate();

    if (!state.projects.empty())
        iCount = (int)state.projects.size();

    return iCount;
}

int CMainDocument::ProjectDetach(int iIndex) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(iIndex);

    if (pProject)
        iRetVal = rpc.project_op((*pProject), "detach");

    return iRetVal;
}


int CMainDocument::ProjectDetach(const wxString& projectname) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(projectname);

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

int CMainDocument::ProjectUpdate(const wxString& projectname) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(projectname);

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

int CMainDocument::ProjectReset(const wxString& projectname) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(projectname);

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

int CMainDocument::ProjectSuspend(const wxString& projectname) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(projectname);

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

int CMainDocument::ProjectResume(const wxString& projectname) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(projectname);

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

int CMainDocument::ProjectNoMoreWork(const wxString& projectname) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(projectname);

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

int CMainDocument::ProjectAllowMoreWork(const wxString& projectname) {
    PROJECT* pProject = NULL;
    int iRetVal = -1;

    pProject = project(projectname);

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

/* get the result not by index, but by name */
RESULT* CMainDocument::result(const wxString& name) {
    RESULT* pResult = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
        if (!results.results.empty())
			//iterating over the vector and find the right result
			for(unsigned int i=0; i< results.results.size();i++) {
				RESULT* tResult = results.results.at(i);
				wxString resname(tResult->name.c_str(),wxConvUTF8);
				if(resname.IsSameAs(name)){
					pResult = tResult;
					break;
				}
			}
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


// If the graphics application for the current task is already 
// running, return a pointer to its RUNNING_GFX_APP struct.
RUNNING_GFX_APP* CMainDocument::GetRunningGraphicsApp(RESULT* result, int slot)
{
    bool exited = false;
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;
    
    for( gfx_app_iter = m_running_gfx_apps.begin(); 
        gfx_app_iter != m_running_gfx_apps.end(); 
        gfx_app_iter++
    ) {
         if ( (slot >= 0) && ((*gfx_app_iter).slot != slot) ) continue;

#ifdef _WIN32
        unsigned long exit_code;
        if (GetExitCodeProcess((*gfx_app_iter).pid, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                exited = true;
            }
        }
#else
        if (waitpid((*gfx_app_iter).pid, 0, WNOHANG) == (*gfx_app_iter).pid) {
            exited = true;
        }
#endif
        if (! exited) {
            if ( (result->name == (*gfx_app_iter).name) &&
                (result->project_url == (*gfx_app_iter).project_url) ) {
                return &(*gfx_app_iter);
            }
    
            // Graphics app is still running but the slot no longer has a different task
            kill_program((*gfx_app_iter).pid);
        }

        // Either the graphics app had already exited or we just killed it
        (*gfx_app_iter).name.clear();
        (*gfx_app_iter).project_url.clear();
        m_running_gfx_apps.erase(gfx_app_iter);
        return NULL;
    }
    return NULL;
}


// Kill any running graphics apps whose worker tasks aren't running
void CMainDocument::KillInactiveGraphicsApps()
{
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;
    unsigned int i;
    bool bStillRunning;

    if (m_running_gfx_apps.size() <= 0) return;
    
    // If none of the Tasks displays are visible, we need to update 
    // the results vector.  This call does nothing if recently updated
    // by a call from CViewWork, CViewWorkGrid or CViewTabPage.
    CachedResultsStatusUpdate();
    
    // Step through in reverse order so we can erase vector items
    gfx_app_iter = m_running_gfx_apps.end();
    do {
        gfx_app_iter--;
        bStillRunning = false;
        
        for(i=0; i<results.results.size(); i++) {
            if ((results.results.at(i))->state != RESULT_FILES_DOWNLOADED) continue;
            if (! (results.results.at(i))->active_task) continue;
            if  ((results.results.at(i))->scheduler_state != CPU_SCHED_SCHEDULED) continue;

            if ((results.results.at(i))->name != (*gfx_app_iter).name) continue;
            if ((results.results.at(i))->project_url != (*gfx_app_iter).project_url) continue;
                    
            bStillRunning =  true;
            break;
        }
        
        if (!bStillRunning) {
            kill_program((*gfx_app_iter).pid);

            (*gfx_app_iter).name.clear();
            (*gfx_app_iter).project_url.clear();
            m_running_gfx_apps.erase(gfx_app_iter);
        }
    } while (gfx_app_iter != m_running_gfx_apps.begin());
}


void CMainDocument::KillAllRunningGraphicsApps()
{
    int i, n;
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;

    n = m_running_gfx_apps.size();
    for (i=0; i<n; i++) {
        gfx_app_iter = m_running_gfx_apps.begin(); 
        kill_program((*gfx_app_iter).pid);
        (*gfx_app_iter).name.clear();
        (*gfx_app_iter).project_url.clear();
        m_running_gfx_apps.erase(gfx_app_iter);
    }
}


int CMainDocument::WorkShowGraphics(RESULT* result)
{
    int iRetVal = 0;
    
    if (!result->graphics_exec_path.empty()) {
        // V6 Graphics
        RUNNING_GFX_APP gfx_app;
        RUNNING_GFX_APP* previous_gfx_app;
        char *p;
        int slot;
#ifdef __WXMSW__
        HANDLE   id;
#else
        int      id;
#endif

        p = strrchr((char*)result->slot_path.c_str(), '/');
        if (!p) return ERR_INVALID_PARAM;
        slot = atoi(p+1);
        
        // See if we are already running the grapics application for this task
        previous_gfx_app = GetRunningGraphicsApp(result, slot);

#ifdef __WXMAC__
        ProcessSerialNumber gfx_app_psn;
        char* argv[5];

        if (previous_gfx_app) {
            // If this graphics app is already running, just bring it to the front
            if (! GetProcessForPID(previous_gfx_app->pid, &gfx_app_psn)) {
                SetFrontProcess(&gfx_app_psn);
            }
            return 0;
        }
        // For unknown reasons, the graphics application exits with 
        // "RegisterProcess failed (error = -50)" unless we pass its 
        // full path twice in the argument list to execv.
        argv[0] = "switcher";
        argv[1] = (char *)result->graphics_exec_path.c_str();
        argv[2] = (char *)result->graphics_exec_path.c_str();
        argv[3] = "--graphics";
        argv[4] = 0;
    
         if (g_use_sandbox) {
            iRetVal = run_program(
                result->slot_path.c_str(),
               "../../switcher/switcher",
                4,
                argv,
                0,
                id
            );
        } else {        
            iRetVal = run_program(
                result->slot_path.c_str(),
                result->graphics_exec_path.c_str(),
                3,
                &argv[1],
                0,
                id
            );
        }
#else
        char* argv[2];

        // If graphics app is already running, don't launch a second instance
        if (previous_gfx_app) return 0;
        argv[0] = "--graphics";
        argv[1] = 0;
        
        iRetVal = run_program(
            result->slot_path.c_str(),
            result->graphics_exec_path.c_str(),
            1,
            argv,
            0,
            id
        );
#endif
        if (! iRetVal) {
            gfx_app.slot = slot;
            gfx_app.project_url = result->project_url;
            gfx_app.name = result->name;
            gfx_app.pid = id;
            m_running_gfx_apps.push_back(gfx_app);
        }

    } else {
        // V5 and Older
        DISPLAY_INFO di;

        strcpy(di.window_station, (char*)wxGetApp().m_strDefaultWindowStation.c_str());
        strcpy(di.desktop, (char*)wxGetApp().m_strDefaultDesktop.c_str());
        strcpy(di.display, (char*)wxGetApp().m_strDefaultDisplay.c_str());

        iRetVal = rpc.show_graphics(
            result->project_url.c_str(),
            result->name.c_str(),
            MODE_WINDOW,
            di
        );
    }

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
            int last_ind = messages.messages.size()-1;
            m_iMessageSequenceNumber = messages.messages[last_ind]->seqno;
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

FILE_TRANSFER* CMainDocument::file_transfer(const wxString& fileName) {
    FILE_TRANSFER* pFT = NULL;

    // It is not safe to assume that the vector actually contains the data,
    //   doing so will lead to those annoying dialogs about the list control
    //   not being able to find list item such and such.  In the worst case
    //   scenario it'll lead to a crash, so for now we'll use the at() function
    //   which will cause an exception which can be trapped and return a NULL
    //   pointer when the exception is thrown.
    try {
		if (!ft.file_transfers.empty()) {
			for(unsigned int i=0; i< ft.file_transfers.size();i++) {
				FILE_TRANSFER* tFT = ft.file_transfers.at(i);
				wxString fname(tFT->name.c_str(),wxConvUTF8);
				if(fname.IsSameAs(fileName)) {
					pFT = tFT;
					break;
				}
			}
		}
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

int CMainDocument::TransferRetryNow(const wxString& fileName) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(fileName);

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

int CMainDocument::TransferAbort(const wxString& fileName) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(fileName);

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
