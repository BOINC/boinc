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
#pragma implementation "MainDocument.h"
#endif

#include "stdwx.h"
#include <wx/numformatter.h>

#include "error_numbers.h"
#include "str_replace.h"
#include "util.h"

#ifdef __WXMAC__
#include "mac_util.h"
#endif

#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "AdvancedFrame.h"
#include "BOINCClientManager.h"
#include "BOINCTaskBar.h"
#include "DlgEventLog.h"
#include "Events.h"
#include "SkinManager.h"
#include "version.h"
#include "DlgGenericMessage.h"

#ifndef _WIN32
#include <sys/wait.h>
#endif

#ifdef SANDBOX
#include <grp.h>
#endif

#define USE_CACHE_TIMEOUTS 0

// If get_results RPC takes x seconds, do it no more often than
// once every (x * GET_RESULTS_FREQUENCY_FACTOR) seconds
#define GET_RESULTS_FREQUENCY_FACTOR 10

// *** RPC update intervals in seconds ***
// m_dtCachedCCStatusTimestamp
#define CCSTATUS_RPC_INTERVAL 1

//m_dtCachedStateTimestamp
#define STATERPC_INTERVAL 3600

//m_dtNoticesTimeStamp
#define NOTICESBACKGROUNDRPC_INTERVAL 60

//m_dtProjectsStatusTimestamp
#define PROJECTSTATUSRPC_INTERVAL 1

//m_dtResultsTimestamp
#define RESULTSRPC_INTERVAL 1

//m_dtFileTransfersTimestamp
#define FILETRANSFERSRPC_INTERVAL 1

// m_dtStatisticsStatusTimestamp
#define STATISTICSSTATUSRPC_INTERVAL 60

// m_dtDiskUsageTimestamp
#define DISKUSAGERPC_INTERVAL 60

// m_dtCachedSimpleGUITimestamp
#define CACHEDSIMPLEGUIRPC_INTERVAL 1

// m_dtCachedAcctMgrInfoTimestamp
#define CACHEDACCTMGRINFORPC_INTERVAL 600

// m_dtLasAsyncRPCDlgTime
#define DELAYAFTERASYNCRPC_DLG 1

bool g_use_sandbox = false;

extern bool s_bSkipExitConfirmation;


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
    m_bUseDefaultPassword = false;
    m_bUsedDefaultPassword = false;
    m_iPort = GUI_RPC_PORT,
    m_iReadGUIRPCAuthFailure = 0;
}


CNetworkConnection::~CNetworkConnection() {
}


int CNetworkConnection::GetLocalPassword(wxString& strPassword){
    char buf[256];

    int retval = read_gui_rpc_password(buf, password_msg);
    strPassword = wxString(buf, wxConvUTF8);
    return retval;
}


void CNetworkConnection::Poll() {
    int retval;
    wxString strComputer = wxEmptyString;

    if (IsReconnecting()) {
        wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - Reconnection Detected"));
        retval = m_pDocument->rpcClient.init_poll();
        if (!retval) {
            wxLogTrace(wxT("Function Status"), wxT("CNetworkConnection::Poll - init_poll() returned ERR_CONNECT, now authorizing..."));

            // Wait until we can establish a connection to the core client before reading
            //   the password so that the client has time to create one when it needs to.
            if (m_bUseDefaultPassword) {
                m_iReadGUIRPCAuthFailure = 0;
                m_bUseDefaultPassword = FALSE;
                m_bUsedDefaultPassword = true;

                m_iReadGUIRPCAuthFailure = GetLocalPassword(m_strNewComputerPassword);
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
            } else {
                // NOTE: Reconnect after a disconnect case.
                //       Values are stored after the first successful connect to the host.
                //       See: SetStateSuccess()
                if (!m_strConnectedComputerName.empty()) {
                    strComputer = m_strConnectedComputerName;
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
                retval = m_pDocument->rpcClient.init_asynch(NULL, 60.0, true, m_iPort);
            } else {
                retval = m_pDocument->rpcClient.init_asynch(strComputer.mb_str(), 60.0, false, m_iPort);
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
    static wxString strHostName = wxEmptyString;
    static wxString strFullHostName = wxEmptyString;

    if (strHostName.empty()) {
        strHostName = ::wxGetHostName().Lower();
    }
    if (strFullHostName.empty()) {
        strFullHostName = ::wxGetFullHostName().Lower();
    }

    if (strMachine.empty()) {
        return true;
    }

    const wxString& strMachineLower = strMachine.Lower();

    if (wxT("localhost") == strMachineLower) {
        return true;
    }
    if (wxT("localhost.localdomain") == strMachineLower) {
        return true;
    }
    if (strHostName == strMachineLower) {
        return true;
    }
    if (strFullHostName == strMachineLower) {
        return true;
    }
    if (wxT("127.0.0.1") == strMachineLower) {
        return true;
    }

    return false;
}


int CNetworkConnection::SetComputer(
    const wxString& szComputer, const int iPort, const wxString& szPassword,
    const bool bUseDefaultPassword
) {
    m_strNewComputerName.Empty();
    m_strNewComputerPassword.Empty();
    m_bUseDefaultPassword = FALSE;

    m_bNewConnection = true;
    m_strNewComputerName = szComputer;
    m_iPort = iPort;
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

        pFrame->ShowConnectionBadPasswordAlert(m_bUsedDefaultPassword, m_iReadGUIRPCAuthFailure, password_msg);
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
        pFrame->FireRefreshView();
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

        // Prevent a race condition where OnFrameRender() causes SetStateDisconnected()
        // to be called due to a previous RPC error before we reconnected.
        m_pDocument->RefreshRPCs(true);

        // Get the version of the client and cache it
        VERSION_INFO vi;
        string rpc_client_name = "BOINC Manager " BOINC_VERSION_STRING;
        m_pDocument->rpc.exchange_versions(rpc_client_name, vi);
        m_strConnectedComputerVersion.Printf(
            wxT("%d.%d.%d"),
            vi.major, vi.minor, vi.release
        );

        m_bConnectEvent = false;
        m_pDocument->ResetMessageState();

        pFrame->FireConnect();
    }
}


void CNetworkConnection::SetStateDisconnected() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame && !m_bFrameShutdownDetected) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
        m_bConnected = false;
        m_bReconnecting = false;
        m_pDocument->results.clear();
    }
}


IMPLEMENT_DYNAMIC_CLASS(CMainDocument, wxObject)


CMainDocument::CMainDocument() : rpc(this) {

#ifdef __WIN32__
    int retval;
    WSADATA wsdata;

    retval = WSAStartup(MAKEWORD(1, 1), &wsdata);
    if (retval) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CMainDocument - Winsock Initialization Failure '%d'"), retval);
    }
#endif

    safe_strcpy(m_szLanguage, "");

    m_bClientStartCheckCompleted = false;

    m_ActiveTasksOnly = false;

    m_fProjectTotalResourceShare = 0.0;

    m_iLastMessageSequenceNumber = 0;
    m_iFirstMessageSequenceNumber = -1;

    m_iNoticeSequenceNumber = 0;
    m_iLastReadNoticeSequenceNumber = -1;
    m_dLastReadNoticeArrivalTime = 0.0;
    m_bWaitingForGetNoticesRPC = false;

    m_dtCachedStateTimestamp = wxDateTime((time_t)0);
    m_iGet_state_rpc_result = 0;

    m_dtCachedCCStatusTimestamp = wxDateTime((time_t)0);
    m_iGet_status_rpc_result = 0;

    m_dtNoticesTimeStamp = wxDateTime((time_t)0);;
    m_iGet_notices_rpc_result = -1;

    m_dtProjectsStatusTimestamp = wxDateTime((time_t)0);
    m_iGet_project_status1_rpc_result = -1;

    m_dtResultsTimestamp = wxDateTime((time_t)0);
    m_iGet_results_rpc_result = -1;

    m_fResultsRPCExecutionTime = 0;

    m_dtKillInactiveGfxTimestamp = wxDateTime((time_t)0);
    m_dtFileTransfersTimestamp = wxDateTime((time_t)0);
    m_iGet_file_transfers_rpc_result = 0;

    m_iGet_messages_rpc_result = -1;

    m_dtDiskUsageTimestamp = wxDateTime((time_t)0);
    m_iGet_dsk_usage_rpc_result = -1;

    m_dtStatisticsStatusTimestamp = wxDateTime((time_t)0);
    m_iGet_statistics_rpc_result = -1;

    m_dtCachedSimpleGUITimestamp = wxDateTime((time_t)0);
    m_iGet_simple_gui2_rpc_result = -1;

    m_dtCachedAcctMgrInfoTimestamp = wxDateTime((time_t)0);
    m_iAcct_mgr_info_rpc_result = -1;

    m_dtLasAsyncRPCDlgTime = wxDateTime((time_t)0);
    m_dtLastFrameViewRefreshRPCTime = wxDateTime((time_t)0);

    status.max_event_log_lines = 0;
}


CMainDocument::~CMainDocument() {
    KillAllRunningGraphicsApps();
#ifdef __WIN32__
    WSACleanup();
#endif
}


int CMainDocument::OnInit() {
    int iRetVal = -1;

    m_pNetworkConnection = new CNetworkConnection(this);
    wxASSERT(m_pNetworkConnection);

    m_pClientManager = new CBOINCClientManager();
    wxASSERT(m_pClientManager);

    // client may auto-attach only when first launched
    m_bAutoAttaching = autoattach_in_progress();

    m_RPCWaitDlg = NULL;
    m_bWaitingForRPC = false;
    m_bNeedRefresh = false;
    m_bNeedTaskBarRefresh = false;
    m_bRPCThreadIsReady = false;
    m_bShutDownRPCThread = false;
    current_rpc_request.clear();

    m_pRPC_Thread_Mutex = new BOINC_Mutex();
    wxASSERT(m_pRPC_Thread_Mutex);

    m_pRPC_Thread_Condition = new BOINC_Condition(*m_pRPC_Thread_Mutex);
    wxASSERT(m_pRPC_Thread_Condition);

    m_pRPC_Request_Mutex = new BOINC_Mutex();
    wxASSERT(m_pRPC_Request_Mutex);

    m_pRPC_Request_Condition = new BOINC_Condition(*m_pRPC_Request_Mutex);
    wxASSERT(m_pRPC_Request_Condition);

    m_RPCThread = new RPCThread(this,
        m_pRPC_Thread_Mutex,
        m_pRPC_Thread_Condition,
        m_pRPC_Request_Mutex,
        m_pRPC_Request_Condition
    );
    wxASSERT(m_RPCThread);

    iRetVal = m_RPCThread->Create();
    wxASSERT(!iRetVal);

    m_RPCThread->Run();
    for (int i=0; i<100; i++) {
        if (!m_bRPCThreadIsReady) {
            boinc_sleep(0.01);  // Allow RPC thread to initialize itself
        }
    }

    return iRetVal;
}


int CMainDocument::OnExit() {
    int              iRetVal = 0;

    if (m_pClientManager) {
        if (wxGetApp().ShouldShutdownCoreClient()) {
            // Shut down only local clients on Manager exit
            if (!wxGetApp().IsMgrMultipleInstance()) {
                m_pClientManager->ShutdownBOINCCore(true);
            }
        }

        delete m_pClientManager;
        m_pClientManager = NULL;
    }

    if (m_RPCThread) {
        KillRPCThread();
        m_RPCThread = NULL;
    }

    delete m_pRPC_Thread_Mutex;
    m_pRPC_Thread_Mutex = NULL;

    delete m_pRPC_Thread_Condition;
    m_pRPC_Thread_Condition = NULL;

    delete m_pRPC_Request_Mutex;
    m_pRPC_Request_Mutex = NULL;

    delete m_pRPC_Request_Condition;
    m_pRPC_Request_Condition = NULL;

    rpcClient.close();

    if (m_pNetworkConnection) {
        delete m_pNetworkConnection;
        m_pNetworkConnection = NULL;
    }

    return iRetVal;
}


int CMainDocument::OnPoll() {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    int iRetVal = 0;
    wxString hostName = wxGetApp().GetClientHostNameArg();
    wxString password = wxGetApp().GetClientPasswordArg();
    bool isHostnamePasswordSet = wxGetApp().IsHostnamePasswordSet();
    int portNum = wxGetApp().GetClientRPCPortArg();

    wxASSERT(wxDynamicCast(m_pClientManager, CBOINCClientManager));
    wxASSERT(wxDynamicCast(m_pNetworkConnection, CNetworkConnection));

    if (!m_bClientStartCheckCompleted && pFrame) {
        m_bClientStartCheckCompleted = true;

        if (IsComputerNameLocal(hostName)) {
            if (wxGetApp().IsAnotherInstanceRunning() && !isHostnamePasswordSet) {
                if (!pFrame->SelectComputer(hostName, portNum, password, true)) {
                    s_bSkipExitConfirmation = true;
                    wxCommandEvent event;
                    pFrame->OnExit(event); // Exit if Select Computer dialog cancelled
                }
            }
        }

        if (wxGetApp().GetNeedRunDaemon() && IsComputerNameLocal(hostName)) {
            if (m_pClientManager->StartupBOINCCore()) {
                Connect(wxT("localhost"), portNum, password, TRUE, password.IsEmpty());
            }
            else {
                m_pNetworkConnection->ForceDisconnect();
                pFrame->ShowDaemonStartFailedAlert();
            }
        } else {
            Connect(hostName, portNum, password, TRUE, password.IsEmpty());
        }
    }

    // Check connection state, connect if needed.
    m_pNetworkConnection->Poll();

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


int CMainDocument::CachedStateUpdate() {
    // Most of this is now handled by RunPeriodicRPCs() and ForceCacheUpdate()
    int     retval = 0;

    if (m_iGet_state_rpc_result) retval = m_iGet_state_rpc_result;

    if (retval) m_pNetworkConnection->SetStateDisconnected();

    return retval;
}


int CMainDocument::ResetState() {
    rpcClient.close();
    state.clear();
    results.clear();
    ft.clear();
    statistics_status.clear();
    disk_usage.clear();
    proxy_info.clear();

    ForceCacheUpdate();
    return 0;
}


int CMainDocument::Connect(const wxString& szComputer, int iPort, const wxString& szComputerPassword, const bool bDisconnect, const bool bUseDefaultPassword) {
    if (wxGetApp().GetNeedRunDaemon() && IsComputerNameLocal(szComputer)) {
        // Restart client if not already running
        m_pClientManager->AutoRestart();
    }

    if (bDisconnect) {
        m_pNetworkConnection->ForceReconnect();
    }

    m_pNetworkConnection->SetComputer(szComputer, iPort, szComputerPassword, bUseDefaultPassword);
    m_pNetworkConnection->FireReconnectEvent();

    ResetMessageState();
    ResetNoticeState();
    return 0;
}


int CMainDocument::Reconnect() {
    m_pNetworkConnection->ForceReconnect();
    m_pNetworkConnection->FireReconnectEvent();
    ResetMessageState();
    ResetNoticeState();
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


bool CMainDocument::IsComputerNameLocal(const wxString& strMachine) {
    return m_pNetworkConnection->IsComputerNameLocal(strMachine);
}


bool CMainDocument::IsConnected() {
    return m_pNetworkConnection->IsConnected();
}


bool CMainDocument::IsReconnecting() {
    return m_pNetworkConnection->IsReconnecting();
}


void CMainDocument::ForceDisconnect() {
    return m_pNetworkConnection->ForceDisconnect();
}


int CMainDocument::FrameShutdownDetected() {
    return m_pNetworkConnection->FrameShutdownDetected();
}


// It is _not_ enough to just reset m_dtCachedCCStatusTimestamp
// and let RunPeriodicRPCs() update the state for these routines
// (which need immediate results):
//      CMainDocument::SetActivityRunMode()
//      CMainDocument::SetNetworkRunMode()
// Otherwise the Snooze task bar menu item and SimpleGUI Pause
// button do not work properly.
//
int CMainDocument::GetCoreClientStatus(CC_STATUS& ccs, bool bForce) {
    wxString         strMachine = wxEmptyString;
    int              iRetVal = 0;

    if (IsConnected()) {
        if (!m_bWaitingForRPC) {    // Prevent recursive entry of RequestRPC()
#if USE_CACHE_TIMEOUTS
            wxTimeSpan ts(wxDateTime::Now() - m_dtCachedCCStatusTimestamp);
            if (ts.GetSeconds() >= (10 * CCSTATUS_RPC_INTERVAL)) bForce = true;
#endif
            if (m_dtCachedCCStatusTimestamp.IsEqualTo(wxDateTime((time_t)0))) bForce = true;
        }
        if (bForce) {
            m_dtCachedCCStatusTimestamp = wxDateTime::Now();

            m_iGet_status_rpc_result = rpc.get_cc_status(ccs);
            if (0 == iRetVal) {
                status = ccs;
            } else {
                iRetVal = m_iGet_status_rpc_result;
            }
        } else {
            ccs = status;
            iRetVal = m_iGet_status_rpc_result;
        }

        if (m_iGet_status_rpc_result) {
            m_pNetworkConnection->SetStateDisconnected();
        } else {
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


int CMainDocument::SetGPURunMode(int iMode, int iTimeout) {
    CC_STATUS ccs;

    if (IsConnected()) {
        int retval = rpc.set_gpu_mode(iMode, iTimeout);
        if (retval) return retval;
        if (iMode == RUN_MODE_RESTORE) {
            GetCoreClientStatus(ccs, true);
        } else {
            status.gpu_mode = iMode;
        }
    }
    return 0;
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


// We use 0 to indicate that the RPC has never been called yet, so
// set last update time to (time_t)1 here rather than to (time_t)0,
// and only if it is currently not zero.
void CMainDocument::RefreshRPCs(bool fullReset) {
    wxDateTime t = fullReset ? wxDateTime((time_t)0) : wxDateTime((time_t)1);

    if (!m_dtCachedCCStatusTimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtCachedCCStatusTimestamp = t;
//      m_iGet_status_rpc_result = -1;
    }

    if (!m_dtProjectsStatusTimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtProjectsStatusTimestamp = t;
//      m_iGet_project_status1_rpc_result = -1;
    }

    if (!m_dtResultsTimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtResultsTimestamp = t;
//      m_iGet_results_rpc_result = -1;
    }
    m_fResultsRPCExecutionTime = 0;

    if (!m_dtFileTransfersTimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtFileTransfersTimestamp = t;
//      m_iGet_file_transfers_rpc_result = 0;
    }

//  m_iGet_messages_rpc_result = -1;

    if (!m_dtDiskUsageTimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtDiskUsageTimestamp = t;
//      m_iGet_dsk_usage_rpc_result = -1;
    }

    if (!m_dtStatisticsStatusTimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtStatisticsStatusTimestamp = t;
//      m_iGet_statistics_rpc_result = -1;
    }

    if (!m_dtCachedSimpleGUITimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtCachedSimpleGUITimestamp = t;
//      m_iGet_simple_gui2_rpc_result = -1;
    }

    if (!m_dtCachedAcctMgrInfoTimestamp.IsEqualTo(wxDateTime((time_t)0))) {
        m_dtCachedAcctMgrInfoTimestamp = t;
        m_iAcct_mgr_info_rpc_result = -1;
    }

//  m_iGet_state_rpc_result = -1;
}


void CMainDocument::RunPeriodicRPCs(int frameRefreshRate) {
    ASYNC_RPC_REQUEST request;
    wxTimeSpan ts;

    // Timer events are handled while the RPC Wait dialog is shown
    // which may cause unintended recursion and repeatedly posting
    // the same RPC requests from timer routines.
    if (WaitingForRPC()) return;

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (!pFrame) return;

    int currentTabView = pFrame->GetCurrentViewPage();

    // If the client is heavily loaded (e.g, very many tasks), the
    // RPC Wait dialog could appear continuously.  To prevent this,
    // delay periodic RPCs for 1 second after the dialog closes.
    wxDateTime dtNow(wxDateTime::Now());
    if ((currentTabView & (VW_STAT | VW_DISK)) == 0) {
        ts = dtNow - m_dtLasAsyncRPCDlgTime;
        if (ts.GetSeconds()<= DELAYAFTERASYNCRPC_DLG) {
            return;
        }
    }

    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

    if (!IsConnected()) {
        CFrameEvent event(wxEVT_FRAME_REFRESHVIEW, pFrame);
        pFrame->GetEventHandler()->AddPendingEvent(event);
#ifndef __WXGTK__
        CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
        if (pTaskbar) {
            CTaskbarEvent event2(wxEVT_TASKBAR_REFRESH, pTaskbar);
            pTaskbar->AddPendingEvent(event2);
        }
#endif
        CDlgEventLog* eventLog = wxGetApp().GetEventLog();
        if (eventLog) {
            eventLog->OnRefresh();
        }
        return;
    }

    // Several functions (such as Abort, Reset, Detach) display an
    // "Are you sure?" dialog before passing a pointer to a result
    // or project in a demand RPC call.  If Periodic RPCs continue
    // to run during these dialogs, that pointer may no longer be
    // valid by the time the demand RPC is executed.  So we suspend
    // periodic RPCs during certain modal dialogs.
    //
    // Note that this depends on using wxGetApp().SafeMessageBox()
    // instead of wxMessageBox in all tab views and anywhere else
    // where a periodic RPC could cause a similar problem.
    if (wxGetApp().IsSafeMesageBoxDisplayed()) {
        return;
    }

    // SET_LANGUAGE

    static bool first = true;
    if (first) {
        first = false;
        safe_strcpy(m_szLanguage, wxGetApp().GetISOLanguageCode().mb_str());
        request.clear();
        request.which_rpc = RPC_SET_LANGUAGE;
        request.arg1 = (void*)(const char*)&m_szLanguage;
        request.rpcType = RPC_TYPE_ASYNC_NO_REFRESH;
        RequestRPC(request);
    }

    // *********** RPC_GET_CC_STATUS **************

    ts = dtNow - m_dtCachedCCStatusTimestamp;
    if (ts.GetSeconds() >= CCSTATUS_RPC_INTERVAL) {
        request.clear();
        request.which_rpc = RPC_GET_CC_STATUS;
        request.arg1 = &async_status_buf;
        request.exchangeBuf = &status;
        request.rpcType = RPC_TYPE_ASYNC_WITH_UPDATE_TASKBAR_ICON_AFTER;
        request.completionTime = &m_dtCachedCCStatusTimestamp;
        request.resultPtr = &m_iGet_status_rpc_result;

        RequestRPC(request);
    }

    // *********** RPC_GET_MESSAGES **************

    // We must keep getting messages even if the Event Log is not open
    // due to the limited size of the client's buffer, or some may be
    // lost, causing gaps when the Event Log is later opened.
    //
    request.clear();
    request.which_rpc = RPC_GET_MESSAGES;
    // m_iLastMessageSequenceNumber could change between request and execution
    // of RPC, so pass in a pointer rather than its value
    request.arg1 = &m_iLastMessageSequenceNumber;
    request.arg2 = &messages;
    static bool _true = true;
    request.arg3 = &_true;
    request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_EVENT_LOG_AFTER;
    request.completionTime = NULL;
    request.resultPtr = &m_iGet_messages_rpc_result;

    RequestRPC(request);

    // *********** RPC_GET_NOTICES **************

    // We must keep getting notices even if the Notices Tab is not open
    // so we can notify the user when new notices become available.
    ts = dtNow - m_dtNoticesTimeStamp;
    if ((currentTabView & VW_NOTIF) ||
        (ts.GetSeconds() >= NOTICESBACKGROUNDRPC_INTERVAL)) {
        // Don't request another get_notices RPC until we have
        // updated m_iNoticeSequenceNumber from the previous
        // one; otherwise we will get duplicate notices
        if (!m_bWaitingForGetNoticesRPC) {
            m_bWaitingForGetNoticesRPC =  true;
            request.clear();
            request.which_rpc = RPC_GET_NOTICES;
            // m_iNoticeSequenceNumber could change between request and execution
            // of RPC, so pass in a pointer rather than its value
            request.arg1 = &m_iNoticeSequenceNumber;
            request.arg2 = &notices;
            request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_AFTER;
            if (!pFrame->IsShown()
#ifdef __WXMAC__
                || (!wxGetApp().IsApplicationVisible())
#endif
            ) {
                request.rpcType = RPC_TYPE_ASYNC_NO_REFRESH;
            }

            request.completionTime = &m_dtNoticesTimeStamp;
            request.resultPtr = &m_iGet_notices_rpc_result;

            RequestRPC(request);
        }
    }

    ts = dtNow - m_dtCachedStateTimestamp;
    if (ts.GetSeconds() >= STATERPC_INTERVAL) {

        // *********** RPC_GET_STATE **************

        request.clear();
        request.which_rpc = RPC_GET_STATE;
        request.arg1 = &async_state_buf;
        request.exchangeBuf = &state;
        request.rpcType = RPC_TYPE_ASYNC_NO_REFRESH;
        request.completionTime = &m_dtCachedStateTimestamp;
        request.resultPtr = &m_iGet_state_rpc_result;

        RequestRPC(request);
    }

    // **** All periodic RPCs after this point are used only
    // **** when refreshing Advanced Frame or Simple Frame views.
    // **** If the Event Log is shown, the Periodic RPC Timer is
    // **** set for 1 second even though the Frame View may need
    // **** less frequent update.
    // **** The argument frameRefreshRate is 0 if an immediate
    // **** update is needed due to some user action, etc.
    // **** Otherwise frameRefreshRate is the rate at which the
    // **** the current Frame View should be updated.
    ts = dtNow - m_dtLastFrameViewRefreshRPCTime;
    if (ts.GetMilliseconds() < (frameRefreshRate - 500)) return;

    // Don't do periodic RPC calls when hidden / minimized
    if (!pFrame->IsShown()) return;
#ifdef __WXMAC__
    if (!wxGetApp().IsApplicationVisible()) return;
#endif

    m_dtLastFrameViewRefreshRPCTime = dtNow;

    // *********** RPC_GET_PROJECT_STATUS1 **************

    if (currentTabView & VW_PROJ) {
        ts = dtNow - m_dtProjectsStatusTimestamp;
        if (ts.GetSeconds() >= PROJECTSTATUSRPC_INTERVAL) {
            request.clear();
            request.which_rpc = RPC_GET_PROJECT_STATUS1;
            request.arg1 = &async_projects_update_buf;
            request.arg2 = &state;
            request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_AFTER;
            request.completionTime = &m_dtProjectsStatusTimestamp;
            request.resultPtr = &m_iGet_project_status1_rpc_result;

            RequestRPC(request);
        }
    }

    // *********** RPC_GET_RESULTS **************

    if (currentTabView & VW_TASK) {
        ts = dtNow - m_dtResultsTimestamp;
        wxLongLong secondsSinceLastRPC = ts.GetSeconds();
        if (secondsSinceLastRPC >= RESULTSRPC_INTERVAL) {
            if (secondsSinceLastRPC >= (m_fResultsRPCExecutionTime * GET_RESULTS_FREQUENCY_FACTOR)) {
                request.clear();
                request.which_rpc = RPC_GET_RESULTS;
                request.arg1 = &async_results_buf;
                request.arg2 = &m_ActiveTasksOnly;
                request.exchangeBuf = &results;
                request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_AFTER;
                request.completionTime = &m_dtResultsTimestamp;
                request.RPCExecutionTime = &m_fResultsRPCExecutionTime;
                request.resultPtr = &m_iGet_results_rpc_result;

                RequestRPC(request);
            }
        }
    }

    // *********** RPC_GET_FILE_TRANSFERS **************

    if (currentTabView & VW_XFER) {
        ts = dtNow - m_dtFileTransfersTimestamp;
        if (ts.GetSeconds() >= FILETRANSFERSRPC_INTERVAL) {
            request.clear();
            request.which_rpc = RPC_GET_FILE_TRANSFERS;
            request.arg1 = &async_ft_buf;
            request.exchangeBuf = &ft;
            request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_AFTER;
            request.completionTime = &m_dtFileTransfersTimestamp;
            request.resultPtr = &m_iGet_file_transfers_rpc_result;

            RequestRPC(request);
        }
    }

    // *********** RPC_GET_STATISTICS **************

    if (currentTabView & VW_STAT) {
        ts = dtNow - m_dtStatisticsStatusTimestamp;
        if (ts.GetSeconds() >= STATISTICSSTATUSRPC_INTERVAL) {
            request.clear();
            request.which_rpc = RPC_GET_STATISTICS;
            request.arg1 = &async_statistics_status_buf;
            request.exchangeBuf = &statistics_status;
            request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_AFTER;
            request.completionTime = &m_dtStatisticsStatusTimestamp;
            request.resultPtr = &m_iGet_statistics_rpc_result;

            RequestRPC(request);
        }
    }

    // *********** RPC_GET_DISK_USAGE **************

    if (currentTabView & VW_DISK) {
        ts = dtNow - m_dtDiskUsageTimestamp;
        if ((ts.GetSeconds() >= DISKUSAGERPC_INTERVAL) || disk_usage.projects.empty()) {
            request.clear();
            request.which_rpc = RPC_GET_DISK_USAGE;
            request.arg1 = &async_disk_usage_buf;
            request.exchangeBuf = &disk_usage;
            request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_AFTER;
            request.completionTime = &m_dtDiskUsageTimestamp;
            request.resultPtr = &m_iGet_dsk_usage_rpc_result;

            RequestRPC(request);
        }
    }

    // *********** GET_SIMPLE_GUI_INFO2 **************
    if (currentTabView & VW_SGUI) {
        ts = dtNow - m_dtCachedSimpleGUITimestamp;
        if (ts.GetSeconds() >= CACHEDSIMPLEGUIRPC_INTERVAL) {
            request.clear();
            request.which_rpc = RPC_GET_SIMPLE_GUI_INFO2;
            request.arg1 = &async_projects_update_buf;
            request.arg2 = &state;
            request.arg3 = &async_results_buf;
            request.arg4 = &m_ActiveTasksOnly;
            request.exchangeBuf = &results;
            request.rpcType = RPC_TYPE_ASYNC_WITH_REFRESH_AFTER;
            request.completionTime = &m_dtCachedSimpleGUITimestamp;
            request.resultPtr = &m_iGet_simple_gui2_rpc_result;

            RequestRPC(request);
        }
    }
    // *********** RPC_ACCT_MGR_INFO **************

    if (currentTabView & VW_SGUI) {
        ts = dtNow - m_dtCachedAcctMgrInfoTimestamp;
        if (ts.GetSeconds() >= CACHEDACCTMGRINFORPC_INTERVAL) {
            request.clear();
            request.which_rpc = RPC_ACCT_MGR_INFO;
            request.arg1 = &async_ami_buf;
            request.exchangeBuf = &ami;
            request.rpcType = RPC_TYPE_ASYNC_NO_REFRESH;
            request.completionTime = &m_dtCachedAcctMgrInfoTimestamp;
            request.resultPtr = &m_iAcct_mgr_info_rpc_result;

            RequestRPC(request);
        }
    }
}


// TODO: Is it enough to just reset m_dtCachedStateTimestamp
// and let RunPeriodicRPCs() update the state?  This would avoid
// displaying the "Please wait" dialog on multi-processor computers.
// Possible exceptions might be when ForceCacheUpdate() is called
// from these routines (which may need immediate results):
//      CAdvancedFrame::OnConnect()
//      CDlgItemProperties::FormatApplicationName()
//      WorkunitNotebook::AddTab()
//      CMainDocument::CachedProjectStatusUpdate()
//      CMainDocument::CachedSimpleGUIUpdate()
//
// Currently, no calls to ForceCacheUpdate pass false as the arg.
//
int CMainDocument::ForceCacheUpdate(bool immediate) {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::ForceCacheUpdate - Function Begin"));

    if (!immediate) {
        m_dtCachedStateTimestamp = wxDateTime((time_t)0);
        return m_iGet_state_rpc_result;
    }

    int     retval = 0;

    if (IsConnected()) {
        m_dtCachedStateTimestamp = wxDateTime::Now();
        m_iGet_state_rpc_result = rpc.get_state(state);
        if (m_iGet_state_rpc_result) {
            retval = m_iGet_state_rpc_result;
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::ForceCacheUpdate - Get State Failed '%d'"), retval);
            m_pNetworkConnection->SetStateDisconnected();
        }

    } else {
        retval = -1;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::ForceCacheUpdate - Function End"));
    return retval;
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
#ifndef __WXMAC__   // Currently unauthorized users can't run Manager, so this would be redundant
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

            userName = getenv("USER");
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
#endif      // #ifndef __WXMAC__

    return true;
}

void CMainDocument::CheckForVersionUpdate(bool showMessage) {
    std::string version, url;
    wxString message, title;
    title.Printf(_("Version Update"));
    wxString applicationName = wxGetApp().GetSkinManager()->GetAdvanced()->GetApplicationName();
    bool newVersionAvailable = false;
    if (IsConnected()) {
        rpc.get_newer_version(version, url);

        if (!showMessage)
            return;

        if (!version.empty() && !url.empty()) {
            message.Printf(_("A new version of %s is available.\nYou can download it here: %s"), applicationName, url);
            newVersionAvailable = true;
        }
        else {
            message.Printf(_("There is no new version of %s available for download."), applicationName);
        }
    }
    else {
        message.Printf(_("%s is not connected to the client"), applicationName);
    }
    if (showMessage) {
        CDlgGenericMessageParameters params;
        params.caption = title;
        params.message = message;
        params.showDisableMessage = false;
        params.button1 = CDlgGenericMessageButton(newVersionAvailable, wxID_OK, _("Go to download page"));
        params.button2 = CDlgGenericMessageButton(true, wxID_CANCEL, _("Close"));
        CDlgGenericMessage dlg(wxGetApp().GetFrame(), &params);
        if (dlg.ShowModal() == wxID_OK) {
            wxLaunchDefaultBrowser(url);
        }
    }
}

int CMainDocument::CachedProjectStatusUpdate(bool bForce) {
    int     i = 0;

    if (! IsConnected()) return -1;

#if USE_CACHE_TIMEOUTS
    wxTimeSpan ts(wxDateTime::Now() - m_dtProjectsStatusTimestamp);
    if (ts.GetSeconds() >= (2 * PROJECTSTATUSRPC_INTERVAL)) bForce = true;
#endif
    if (m_dtProjectsStatusTimestamp.IsEqualTo(wxDateTime((time_t)0))) bForce = true;

    if (bForce) {
        m_dtProjectsStatusTimestamp = wxDateTime::Now();
        m_iGet_project_status1_rpc_result = rpc.get_project_status(async_projects_update_buf, state);
    }

    if (m_iGet_project_status1_rpc_result) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedProjectStatusUpdate - Get Project Status Failed '%d'"), m_iGet_project_status1_rpc_result);
        ForceCacheUpdate();
//        return m_iGet_project_status1_rpc_result;
    }

    m_fProjectTotalResourceShare = 0.0;
    for (i=0; i < (long)state.projects.size(); i++) {
        m_fProjectTotalResourceShare += state.projects.at(i)->resource_share;
    }
    if (!m_fProjectTotalResourceShare) m_fProjectTotalResourceShare = 1;

    return m_iGet_project_status1_rpc_result;
}


PROJECT* CMainDocument::project(unsigned int i) {
    PROJECT* pProject = NULL;

    try {
        if (!state.projects.empty())
            pProject = state.projects.at(i);
    }
    catch (std::out_of_range&) {
        pProject = NULL;
    }

    return pProject;
}


PROJECT* CMainDocument::project(char* url) {
    for (unsigned int i=0; i< state.projects.size(); i++) {
        PROJECT* tp = state.projects[i];
        if (!strcmp(url, tp->master_url)) return tp;
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
    if (! IsConnected()) return -1;
    bool active_tasks_only = false;
    bool immediate = false;

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame) {
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

        if (pFrame->GetCurrentViewPage() & VW_TASK) {
            active_tasks_only = m_ActiveTasksOnly;
        }
    }

#if USE_CACHE_TIMEOUTS
    wxTimeSpan ts(wxDateTime::Now() - m_dtResultsTimestamp);
    if (ts.GetSeconds() >= (2 * RESULTSRPC_INTERVAL)) immediate = true;
#endif
    if (m_dtResultsTimestamp.IsEqualTo(wxDateTime((time_t)0))) immediate = true;

    if (immediate) {
        m_dtResultsTimestamp = wxDateTime::Now();
        m_iGet_results_rpc_result = rpc.get_results(results, active_tasks_only);
    }

    if (m_iGet_results_rpc_result) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedResultsStatusUpdate - Get Result Status Failed '%d'"), m_iGet_results_rpc_result);
        ForceCacheUpdate();
    }

    return m_iGet_results_rpc_result;
}


RESULT* CMainDocument::result(unsigned int i) {
    RESULT* pResult = NULL;

    try {
        if (!results.results.empty())
            pResult = results.results.at(i);
    }
    catch (std::out_of_range&) {
        pResult = NULL;
    }

    return pResult;
}

// get the result not by index, but by name
//
RESULT* CMainDocument::result(const wxString& name, const wxString& project_url) {
    RESULT* pResult = NULL;

    try {
        if (!results.results.empty()) {
            //iterating over the vector and find the right result
            for(unsigned int i=0; i< results.results.size();i++) {
                RESULT* tResult = results.results.at(i);
                wxString resname(tResult->name, wxConvUTF8);
                if(resname.IsSameAs(name)){
                    wxString resurl(tResult->project_url, wxConvUTF8);
                    if(resurl.IsSameAs(project_url)){
                        pResult = tResult;
                        break;
                    }
                }
            }
        }
    }
    catch (std::out_of_range&) {
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


int CMainDocument::WorkSuspend(const char* url, const char* name) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(url, name);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "suspend");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


int CMainDocument::WorkResume(const char* url, const char* name) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(url, name);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "resume");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


// If the graphics application for the current task is already
// running, return a pointer to its RUNNING_GFX_APP struct.
//
RUNNING_GFX_APP* CMainDocument::GetRunningGraphicsApp(RESULT* rp) {
    bool exited = false;
    int slot = -1;
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;

    if (m_running_gfx_apps.empty()) return NULL;

    char *p = strrchr((char*)rp->slot_path, '/');
    if (!p) return NULL;
    slot = atoi(p+1);


    for( gfx_app_iter = m_running_gfx_apps.begin();
        gfx_app_iter != m_running_gfx_apps.end();
        ++gfx_app_iter
    ) {
        if ((slot >= 0) && ((*gfx_app_iter).slot != slot)) continue;

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
            if ((rp->name == (*gfx_app_iter).name) &&
                (rp->project_url == (*gfx_app_iter).project_url)
            ) {
                return &(*gfx_app_iter);
            }

            // Graphics app is still running but the slot now has a different task
#ifdef __WXMAC__
            // For some graphics apps (including Einstein), we must explicitly delete both
            // our forked process and its child graphics app, or the other will remain. A
            // race condition can occur because of the time it takes for our forked process
            // to launch the graphics app, so we periodically poll for the child's PID until
            // it is available. If we don't have it yet, we defer killing our forked process.
            if (g_use_sandbox) {
                if (!GetGFXPIDFromForkedPID(&(*gfx_app_iter))) {
                    return 0;    // Ignore "Stop graphics" button until we have graphics app's pid
                }
                KillGraphicsApp((*gfx_app_iter).gfx_pid);
           }
#endif
        KillGraphicsApp((*gfx_app_iter).pid);
       }

        // Either the graphics app had already exited or we just killed it
#ifdef __WXMAC__
        // Graphics app wrote files in slot directory as logged in user, not boinc_master
        fix_slot_file_owners((*gfx_app_iter).slot);
#endif
        (*gfx_app_iter).name.clear();
        (*gfx_app_iter).project_url.clear();
        m_running_gfx_apps.erase(gfx_app_iter);
        return NULL;
    }
    return NULL;
}


// Kill any running graphics apps whose worker tasks aren't running
void CMainDocument::KillInactiveGraphicsApps() {
/*
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;
    unsigned int i;
    bool bStillRunning;

    if (m_running_gfx_apps.size() <= 0) return;

    // If none of the Tasks displays are visible, we need to update
    // the results vector.  This call does nothing if recently updated
    // by a call from CViewWork or CViewTabPage.
    CachedResultsStatusUpdate();

    gfx_app_iter = m_running_gfx_apps.begin();
    while (gfx_app_iter != m_running_gfx_apps.end()) {
        bStillRunning = false;

        for (i=0; i<results.results.size(); i++) {
            if ((results.results.at(i))->state != RESULT_FILES_DOWNLOADED) continue;
            if (!(results.results.at(i))->active_task) continue;
            if ((results.results.at(i))->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            if ((results.results.at(i))->name != (*gfx_app_iter).name) continue;
            if ((results.results.at(i))->project_url != (*gfx_app_iter).project_url) continue;
            bStillRunning =  true;
            break;
        }

        if (!bStillRunning) {
#ifdef __WXMAC__
            // For some graphics apps (including Einstein), we must explicitly delete both
            // our forked process and its child graphics app, or the other will remain. A
            // race condition can occur because of the time it takes for our forked process
            // to launch the graphics app, so we periodically poll for the child's PID until
            // it is available. If we don't have it yet, we defer killing our forked process.
            if (g_use_sandbox) {
                if (!GetGFXPIDFromForkedPID(&(*gfx_app_iter))) {
                    return;    // Ignore "Stop graphics" button until we have graphics app's pid
                }
                KillGraphicsApp((*gfx_app_iter).gfx_pid);
                KillGraphicsApp((*gfx_app_iter).pid);

                // Graphics app wrote files in slot directory as logged in user, not boinc_master
                fix_slot_file_owners((*gfx_app_iter).slot);
           }
#else
            KillGraphicsApp((*gfx_app_iter).pid);
#endif
            gfx_app_iter = m_running_gfx_apps.erase(gfx_app_iter);
        } else {
            gfx_app_iter++;
        }
    }
*/
}


// KillAllRunningGraphicsApps() is called only from our destructor
void CMainDocument::KillAllRunningGraphicsApps()
{
    size_t i, n;
    std::vector<RUNNING_GFX_APP>::iterator gfx_app_iter;

    n = m_running_gfx_apps.size();
    for (i=0; i<n; i++) {
        gfx_app_iter = m_running_gfx_apps.begin();
#ifdef __WXMAC__
        // For some graphics apps (including Einstein), we must explicitly delete both
        // our forked process and its child graphics app, or the other will remain. A
        // race condition can occur because of the time it takes for our forked process
        // to launch the graphics app, so we periodically poll for the child's PID until
        // it is available. If we don't have it yet, we defer killing our forked process.
        if (g_use_sandbox) {
            for (int j=0; j<100; ++j) { // Wait 1 second max for gfx app's pid
                if (GetGFXPIDFromForkedPID(&(*gfx_app_iter))) {
                    KillGraphicsApp((*gfx_app_iter).gfx_pid);
                    break;
                }
                boinc_sleep(.01);
            }
            KillGraphicsApp((*gfx_app_iter).pid);

            // Graphics app wrote files in slot directory as logged in user, not boinc_master
            fix_slot_file_owners((*gfx_app_iter).slot);
        } else {
            KillGraphicsApp((*gfx_app_iter).pid);
        }
#else
        KillGraphicsApp((*gfx_app_iter).pid);
#endif
        (*gfx_app_iter).name.clear();
        (*gfx_app_iter).project_url.clear();
        m_running_gfx_apps.erase(gfx_app_iter);
    }
}


#ifdef _WIN32
void CMainDocument::KillGraphicsApp(HANDLE pid) {
    kill_process(pid);
}
#else
void CMainDocument::KillGraphicsApp(int pid) {
    // As of MacOS 13.0 Ventura IOSurface cannot be used to share graphics
    // between apps unless they are running as the same user, so we no
    // longer run the graphics apps as user boinc_master.
    kill_process(pid);
}
#endif


#ifdef __WXMAC__
// For some graphics apps (including Einstein), we must explicitly delete both
// our forked process and its child graphics app, or the other will remain. A
// race condition can occur because of the time it takes for our forked process
// to launch the graphics app, so we periodically call this method to poll for
// the child's PID until it is available..
//
int CMainDocument::GetGFXPIDFromForkedPID(RUNNING_GFX_APP* gfx_app) {
    char buf[256];

    if (gfx_app->gfx_pid) return gfx_app->gfx_pid;    // We already found it

    // The graphics app is the child of our forked process. Get its PID
    FILE *fp = (FILE*)popen("ps -xoppid,pid","r");
    fgets(buf, sizeof(buf), fp);    // Skip the header line
    int parent, child;

    // Find the process whose parent is our forked process
    while (fscanf(fp, "%d %d\n", &parent, &child) == 2) {
        if (parent == gfx_app->pid) {
            gfx_app->gfx_pid = child;
            break;
        }
    }
    pclose(fp);
    return gfx_app->gfx_pid;
}


int CMainDocument::fix_slot_file_owners(int slot) {
    if (g_use_sandbox) {
        // Graphics apps run by Manager write files in slot directory
        // as logged in user, not boinc_master. This ugly hack tells
        // BOINC client to fix all ownerships in this slot directory
        rpcClient.run_graphics_app("stop", slot, "");
    }
    return 0;
}
#endif


int CMainDocument::WorkShowGraphics(RESULT* rp) {
    int iRetVal = 0;

    if (strlen(rp->web_graphics_url)) {
        wxString url(rp->web_graphics_url, wxConvUTF8);
        wxLaunchDefaultBrowser(url);
        return 0;
    }
    if (strlen(rp->graphics_exec_path)) {
        // V6 Graphics
        RUNNING_GFX_APP gfx_app;
        RUNNING_GFX_APP* previous_gfx_app;
        char *p;
        int slot;
#ifdef __WXMSW__
        HANDLE   id;
#else
        int      id = 0;
#endif

        // See if we are already running the graphics application for this task
        previous_gfx_app = GetRunningGraphicsApp(rp);

        if (previous_gfx_app) {
#ifdef __WXMAC__
            if (g_use_sandbox) {
                // If graphics app is already running, the button has changed to
                // "Stop graphics", so we end the graphics app.
                //
                // For some graphics apps (including Einstein), we must explicitly delete both
                // our forked process and its child graphics app, or the other will remain. A
                // race condition can occur because of the time it takes for our forked process
                // to launch the graphics app, so we periodically poll for the child's PID until
                // it is available. If we don't have it yet, then defer responding to the "Stop
                // graphics" button.
                if (!GetGFXPIDFromForkedPID(previous_gfx_app)) {
                    return 0;    // Ignore "Stop graphics" button until we have graphics app's pid
                }
                KillGraphicsApp(previous_gfx_app->gfx_pid);
            }
#endif
            KillGraphicsApp(previous_gfx_app->pid); // User clicked on "Stop graphics" button
            GetRunningGraphicsApp(rp);  // Let GetRunningGraphicsApp() do necessary clean up
            return 0;
        }


        char* argv[2] = {
            rp->graphics_exec_path,
            NULL
        };

#ifdef __WXMAC__
        if (g_use_sandbox) {    // Used only by Mac
            int pid = fork();
            char path[MAXPATHLEN];
            char cmd[2048];

            // As of MacOS 13.0 Ventura IOSurface cannot be used to share graphics
            // between apps unless they are running as the same user, so we no
            // longer run the graphics apps as user boinc_master. To replace the
            // security that was provided by running as a different user, we use
            // sandbox-exec() to launch the graphics apps. Note that sandbox-exec()
            // is marked deprecated because it is an Apple private API so the syntax
            // of the security specifications is subject to change without notice.
            // But it is used widely in Apple's software, and the security profile
            // elements we use are very unlikely to change.
            if (pid == 0) {
                // For unknown reasons, the graphics application exits with
                // "RegisterProcess failed (error = -50)" unless we pass its
                // full path twice in the argument list to execv.
                strlcpy(cmd, "sandbox-exec -f \"", sizeof(cmd));
                getPathToThisApp(path, sizeof(path));
                strlcat(cmd, path, sizeof(cmd)); // path to this executable
                strlcat(cmd, "/Contents/Resources/mac_restrict_access.sb\" \"", sizeof(cmd)); // path to sandboxing profile
                strlcat(cmd, rp->graphics_exec_path, sizeof(cmd)); // path to graphics app
                strlcat(cmd, "\"", sizeof(cmd)); // path to sandboxing profile
                chdir(rp->slot_path);
                iRetVal = callPosixSpawn(cmd);

                exit(0);
            }
            id = pid;
            // The graphics app is the child of our forked process. Get its PID
            gfx_app.pid = id;
            gfx_app.gfx_pid = 0;    // Initialize GetGFXPIDFromForkedPID()
            gfx_app.gfx_pid = GetGFXPIDFromForkedPID(&gfx_app);
        } else  // !g_use_sandbox
#endif  // __WXMAC__
#ifndef __WXMSW__
        {
            iRetVal = run_program(
                rp->slot_path,
                rp->graphics_exec_path,
                1,
                argv,
                id
            );
        }
#else
        iRetVal = run_program(
            rp->slot_path,
            rp->graphics_exec_path,
            1,
            argv,
            id
        );
#endif
        if (!iRetVal) {
            p = strrchr((char*)rp->slot_path, '/');
            if (!p) return ERR_INVALID_PARAM;
            slot = atoi(p+1);

            gfx_app.slot = slot;
            gfx_app.project_url = rp->project_url;
            gfx_app.name = rp->name;
            gfx_app.pid = id;
            m_running_gfx_apps.push_back(gfx_app);
        }
    }
    return iRetVal;
}


int CMainDocument::WorkShowVMConsole(RESULT* res) {
    int iRetVal = 0;

    if (strlen(res->remote_desktop_addr)) {
        wxString strConnection(res->remote_desktop_addr, wxConvUTF8);
        wxString strCommand;

#if   defined(__WXMSW__)
        strCommand = wxT("mstsc.exe /v:") + strConnection;
        wxExecute(strCommand);
#elif defined(__WXGTK__)
        strCommand = wxT("rdesktop-vrdp ") + strConnection;
        int pid = wxExecute(strCommand);
        // newer versions of VirtualBox don't include rdesktop-vrdp;
        // try a standard version instead
        //
        if (pid == 0) {
            strCommand = wxT("rdesktop ") + strConnection;
            pid = wxExecute(strCommand);
        }
        if (pid == 0) {
            strCommand = wxT("xfreerdp ") + strConnection;
            pid = wxExecute(strCommand);
        }
        // show an error if all the above fail?
#elif defined(__WXMAC__)
        OSStatus status = noErr;
        char pathToCoRD[MAXPATHLEN];

        // I have found no reliable way to pass the IP address and port to Microsoft's
        // Remote Desktop Connection application for the Mac, so I'm using CoRD.
        // Unfortunately, CoRD does not seem as reliable as I would like either.
        //
        // First try to find the CoRD application by Bundle ID and Creator Code
        status = GetPathToAppFromID('RDC#', CFSTR("net.sf.cord"), pathToCoRD, MAXPATHLEN);
        if (status != noErr) {
            CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
            if (pFrame) {
                pFrame->ShowAlert(
                    _("Missing application"),
                    _("Please download and install the CoRD application from http://cord.sourceforge.net"),
                    wxOK | wxICON_INFORMATION,
                    false
                );
            }
            return ERR_FILE_MISSING;
        }

        strCommand = wxT("osascript -e 'tell application \"CoRD\"' -e 'activate' -e 'open location \"rdp://") + strConnection + wxT("\"' -e 'end tell'");
        strCommand.Replace(wxT("localhost"), wxT("127.0.0.1"));
        system(strCommand.char_str());
#endif
    }

    return iRetVal;
}


int CMainDocument::WorkAbort(const char* url, const char* name) {
    int iRetVal = 0;

    RESULT* pStateResult = state.lookup_result(url, name);
    if (pStateResult) {
        iRetVal = rpc.result_op((*pStateResult), "abort");
    } else {
        ForceCacheUpdate();
    }

    return iRetVal;
}


// Call this only when notice buffer is stable
// Note: This must not call any rpcs.
// This is now called after each get_notices RPC from
//   CMainDocument::HandleCompletedRPC() .
int CMainDocument::CachedNoticeUpdate() {
    static bool in_this_func = false;

    if (in_this_func) return 0;
    in_this_func = true;

    if (IsConnected()) {
        // Can't look up previous last read message until we know machine name
        if (!strlen(state.host_info.domain_name)) {
            goto done;
        }

        // rpc.get_notices is now called from RunPeriodicRPCs()
        if (m_iGet_notices_rpc_result) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedNoticeUpdate - Get Notices Failed '%d'"), m_iGet_notices_rpc_result);
            m_pNetworkConnection->SetStateDisconnected();
            goto done;
        }

        if (notices.notices.size() > 0) {
            m_iNoticeSequenceNumber = notices.notices[0]->seqno;

            if (m_iLastReadNoticeSequenceNumber < 0) {
                m_iLastReadNoticeSequenceNumber = 0;    // Do this only once
                RestoreUnreadNoticeInfo();
            }
        }
    }

done:
    in_this_func = false;
    return 0;
}


void CMainDocument::SaveUnreadNoticeInfo() {
    static double   dLastSavedArrivalTime = 0.0;

    if (dLastSavedArrivalTime != m_dLastReadNoticeArrivalTime) {
        wxString        strBaseConfigLocation = wxString(wxT("/Notices/"));
        wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
        wxString        strDomainName = wxString(state.host_info.domain_name, wxConvUTF8, strlen(state.host_info.domain_name));
        wxString        strArrivalTime = wxEmptyString;

        pConfig->SetPath(strBaseConfigLocation + strDomainName);
        // wxConfigBase::Write(const wxString& key, double value) has
        // insufficient precision so write a wxString.
        strArrivalTime.Printf(wxT("%f"), m_dLastReadNoticeArrivalTime);
        pConfig->Write(wxT("lastReadNoticeTime"), strArrivalTime);
        dLastSavedArrivalTime = m_dLastReadNoticeArrivalTime;
    }
}


void CMainDocument::RestoreUnreadNoticeInfo() {
    wxString        strBaseConfigLocation = wxString(wxT("/Notices/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strDomainName = wxString(state.host_info.domain_name, wxConvUTF8, strlen(state.host_info.domain_name));
    double          dLastReadNoticeTime;
    wxString        strArrivalTime = wxEmptyString;
    int             i, n = (int)notices.notices.size();

    pConfig->SetPath(strBaseConfigLocation + strDomainName);

    if (pConfig->Read(wxT("LastReadNoticeTime"), &strArrivalTime)) {
        strArrivalTime.ToDouble(&dLastReadNoticeTime);
        // To avoid problems caused by rounding in save & restore operation, test in
        // reverse order (oldest first) and for arrival time <= dLastReadNoticeTime
        for (i=n-1; i>=0; --i) {
            if (notices.notices[i]->arrival_time <= dLastReadNoticeTime) {
                m_iLastReadNoticeSequenceNumber = notices.notices[i]->seqno;
                m_dLastReadNoticeArrivalTime = notices.notices[i]->arrival_time;
            }
        }
    }
}


NOTICE* CMainDocument::notice(unsigned int i) {
    NOTICE* pNotice = NULL;

    try {
        if (!notices.notices.empty())
            pNotice = notices.notices.at(i);
    }
    catch (std::out_of_range&) {
        pNotice = NULL;
    }

    return pNotice;
}


int CMainDocument::GetNoticeCount() {
    int iCount = -1;

    // CachedNoticeUpdate() is now called from CMainDocument::OnRPCComplete()
    // only after a get_notices RPC completes so notices buffer is stable.
    CachedStateUpdate();

    if (notices.received) {
        iCount = (int)notices.notices.size();
    }

    return iCount;
}


int CMainDocument::GetUnreadNoticeCount() {
    int iCount = 0;
    if (!notices.notices.empty()) {
        for (unsigned int i = 0; i < notices.notices.size(); i++) {
            if (notices.notices[i]->arrival_time > m_dLastReadNoticeArrivalTime) {
                iCount++;
            }
        }
    }
    return iCount;
}


void CMainDocument::UpdateUnreadNoticeState() {
    if (!notices.notices.empty()) {
        m_iLastReadNoticeSequenceNumber = notices.notices[0]->seqno;
        m_dLastReadNoticeArrivalTime = notices.notices[0]->arrival_time;
        SaveUnreadNoticeInfo();
    }
}


int CMainDocument::ResetNoticeState() {
    notices.clear();
    m_iNoticeSequenceNumber = 0;
    m_iLastReadNoticeSequenceNumber = -1;
    m_dLastReadNoticeArrivalTime = 0.0;
    return 0;
}


// Replace CRLFs and LFs with HTML breaks.
//
void eol_to_br(wxString& strMessage) {
    strMessage.Replace(wxT("\r\n"), wxT("<br>"));
    strMessage.Replace(wxT("\n"), wxT("<br>"));
    strMessage.Replace(wxT("<br />"), wxT("<br>"));
    strMessage.Replace(wxT("<br><br>"), wxT("<br>"));
}

// Remove CRLFs and LFs
//
void remove_eols(wxString& strMessage) {
    strMessage.Replace(wxT("\r\n"), wxT(""));
    strMessage.Replace(wxT("\n"), wxT(""));
}

// Replace https:// with http://
//
void https_to_http(wxString& strMessage) {
    strMessage.Replace(wxT("https://"), wxT("http://"));
}

// replace substrings of the form _("X") with the translation of X
//
void localize(wxString& strMessage) {
    wxString strBuffer = wxEmptyString;
    wxString strStart = wxString(wxT("_(\""));
    wxString strEnd = wxString(wxT("\")"));

    while (strMessage.Find(strStart.c_str()) != wxNOT_FOUND) {
        strBuffer = strMessage.SubString(
            strMessage.Find(strStart.c_str()) + strStart.Length(),
            strMessage.Find(strEnd.c_str()) - (strEnd.Length() - 1)
        );
        strMessage.Replace(
            wxString(strStart + strBuffer + strEnd).c_str(),
            wxGetTranslation(strBuffer.c_str())
        );
    }
}


// Call this only when message buffer is stable
// Note: This must not call any rpcs.
// This is now called after each get_messages RPC from
//   CMainDocument::HandleCompletedRPC() .
//
int CMainDocument::CachedMessageUpdate() {
    static bool in_this_func = false;
    size_t last_ind;

    if (in_this_func) return 0;
    in_this_func = true;

    if (IsConnected()) {
        // rpc.get_messages is now called from RunPeriodicRPCs()
        // retval = rpc.get_messages(m_iLastMessageSequenceNumber, messages);
        if (m_iGet_messages_rpc_result) {
            wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'"), m_iGet_messages_rpc_result);
            m_pNetworkConnection->SetStateDisconnected();
            goto done;
        }
        if (messages.messages.size() != 0) {
            last_ind = messages.messages.size()-1;
            m_iLastMessageSequenceNumber = messages.messages[last_ind]->seqno;

            // status.max_event_log_lines <= 0 means no limit
            if ((status.max_event_log_lines > 0) &&
                    (last_ind >= (unsigned)status.max_event_log_lines)
            ) {
                // Remove oldest messages if we have too many
                while (messages.messages.size() > (unsigned)status.max_event_log_lines) {
                    delete messages.messages.front();
                    messages.messages.pop_front();
                }
                m_iFirstMessageSequenceNumber = messages.messages[0]->seqno;
            } else if (m_iFirstMessageSequenceNumber < 0) {
                m_iFirstMessageSequenceNumber = messages.messages[0]->seqno;
            }
        }
    }

done:
    in_this_func = false;
    return 0;
}


MESSAGE* CMainDocument::message(unsigned int i) {
    if (messages.messages.empty() || messages.messages.size() <= i)
        return NULL;

    return messages.messages.at(i);
}


int CMainDocument::GetMessageCount() {
    int iCount = -1;

    // CachedMessageUpdate() is now called from CMainDocument::OnRPCComplete()
    // only after a get_messages RPC completes so messages buffer is stable.
    CachedStateUpdate();

    if (!messages.messages.empty()) {
        iCount = (int)messages.messages.size();
    }

    return iCount;
}


int CMainDocument::ResetMessageState() {
    messages.clear();
    m_iLastMessageSequenceNumber = 0;
    m_iFirstMessageSequenceNumber = -1;
    return 0;
}


int CMainDocument::CachedFileTransfersUpdate() {
    bool immediate = false;

    if (! IsConnected()) return -1;

#if USE_CACHE_TIMEOUTS
    wxTimeSpan ts(wxDateTime::Now() - m_dtFileTransfersTimestamp);
    if (ts.GetSeconds() >= (2* FILETRANSFERSRPC_INTERVAL)) immediate = true;
#endif
    if (m_dtFileTransfersTimestamp.IsEqualTo(wxDateTime((time_t)0))) immediate = true;

    if (immediate) {
        m_dtFileTransfersTimestamp = wxDateTime::Now();
        m_iGet_file_transfers_rpc_result = rpc.get_file_transfers(ft);
    }

    if (m_iGet_file_transfers_rpc_result) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedFileTransfersUpdate - Get File Transfers Failed '%d'"), m_iGet_file_transfers_rpc_result);
        ForceCacheUpdate();
    }

    return m_iGet_file_transfers_rpc_result;
}


FILE_TRANSFER* CMainDocument::file_transfer(unsigned int i) {
    FILE_TRANSFER* pFT = NULL;

    try {
        if (!ft.file_transfers.empty())
            pFT = ft.file_transfers.at(i);
    }
    catch (std::out_of_range&) {
        pFT = NULL;
    }

    return pFT;
}

FILE_TRANSFER* CMainDocument::file_transfer(const wxString& fileName, const wxString& project_url) {
    FILE_TRANSFER* pFT = NULL;

    try {
        if (!ft.file_transfers.empty()) {
            for(unsigned int i=0; i< ft.file_transfers.size();i++) {
                FILE_TRANSFER* tFT = ft.file_transfers.at(i);
                wxString fname(tFT->name.c_str(),wxConvUTF8);
                if(fname.IsSameAs(fileName)) {
                    wxString furl(tFT->project_url.c_str(),wxConvUTF8);
                    if(furl.IsSameAs(project_url)){
                        pFT = tFT;
                        break;
                    }
                }
            }
        }
    }
    catch (std::out_of_range&) {
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

int CMainDocument::TransferRetryNow(const wxString& fileName, const wxString& project_url) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(fileName, project_url);

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

int CMainDocument::TransferAbort(const wxString& fileName, const wxString& project_url) {
    FILE_TRANSFER* pFT = NULL;
    int iRetVal = 0;

    pFT = file_transfer(fileName, project_url);

    if (pFT)
        iRetVal = rpc.file_transfer_op((*pFT), "abort");

    return iRetVal;
}

int CMainDocument::CachedDiskUsageUpdate() {
    bool immediate = true;

    if (!IsConnected()) return -1;

    // don't get disk usage more than once per minute
    // unless we just connected to a client
    //
#if USE_CACHE_TIMEOUTS
    wxTimeSpan ts(wxDateTime::Now() - m_dtDiskUsageTimestamp);
    if (ts.GetSeconds() >= (2 * DISKUSAGERPC_INTERVAL)) immediate = true;
#endif
    if (disk_usage.projects.empty()) immediate = true;
    if (m_dtDiskUsageTimestamp.IsEqualTo(wxDateTime((time_t)0))) immediate = true;

    if (immediate) {
        m_dtDiskUsageTimestamp = wxDateTime::Now();
        m_iGet_dsk_usage_rpc_result = rpc.get_disk_usage(disk_usage);
    }

    if (m_iGet_dsk_usage_rpc_result) {
        wxLogTrace(wxT("Function Status"), wxT("Get Disk Usage Failed '%d'"), m_iGet_dsk_usage_rpc_result);
        ForceCacheUpdate();
    }

    return m_iGet_dsk_usage_rpc_result;
}


PROJECT* CMainDocument::DiskUsageProject(unsigned int i) {
    PROJECT* pProject = NULL;

    try {
        if (!disk_usage.projects.empty()) {
            pProject = disk_usage.projects.at(i);
        }
    }
    catch (std::out_of_range&) {
        pProject = NULL;
    }

    return pProject;
}

int CMainDocument::CachedStatisticsStatusUpdate() {
    bool immediate = false;

    if (! IsConnected()) return -1;

#if USE_CACHE_TIMEOUTS
    wxTimeSpan ts(wxDateTime::Now() - m_dtStatisticsStatusTimestamp);
    if (ts.GetSeconds() >= (2 * STATISTICSSTATUSRPC_INTERVAL)) immediate = true;
#endif
    if (statistics_status.projects.empty()) immediate = true;
    if (m_dtStatisticsStatusTimestamp.IsEqualTo(wxDateTime((time_t)0))) immediate = true;

    if (immediate) {
        m_dtStatisticsStatusTimestamp = wxDateTime::Now();
        m_dtStatisticsStatusTimestamp = rpc.get_statistics(statistics_status);
    }

    if (m_iGet_statistics_rpc_result) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedStatisticsStatusUpdate - Get Statistics Failed '%d'"), m_iGet_statistics_rpc_result);
        ForceCacheUpdate();
    }

    return m_iGet_state_rpc_result;
}


PROJECT* CMainDocument::statistic(unsigned int i) {
    PROJECT* pProject = NULL;


    try {
        if (!statistics_status.projects.empty())
            pProject = statistics_status.projects.at(i);
    }
    catch (std::out_of_range&) {
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

    iRetVal = rpc.set_proxy_settings(proxy_info);
    if (iRetVal) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::SetProxyInfo - Set Proxy Info Failed '%d'"), iRetVal);
    }

    return iRetVal;
}


int CMainDocument::CachedSimpleGUIUpdate(bool bForce) {
    int     i = 0;

    if (! IsConnected()) return -1;

#if USE_CACHE_TIMEOUTS
    wxTimeSpan ts(wxDateTime::Now() - m_dtCachedSimpleGUITimestamp);
    if (ts.GetSeconds() >= (2 * CACHEDSIMPLEGUIRPC_INTERVAL)) bForce = true;
#endif
    if (m_dtCachedSimpleGUITimestamp.IsEqualTo(wxDateTime((time_t)0))) bForce = true;
    if (bForce) {
        m_dtCachedSimpleGUITimestamp = wxDateTime::Now();
        m_iGet_simple_gui2_rpc_result = rpc.get_simple_gui_info(async_projects_update_buf, state, results, m_ActiveTasksOnly);
    }

    if (m_iGet_simple_gui2_rpc_result) {
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::CachedSimpleGUIUpdate - Get Simple GUI Failed '%d'"), m_iGet_simple_gui2_rpc_result);
        ForceCacheUpdate();
    }

    m_fProjectTotalResourceShare = 0.0;
    for (i=0; i < (long)state.projects.size(); i++) {
        m_fProjectTotalResourceShare += state.projects.at(i)->resource_share;
    }
    if (!m_fProjectTotalResourceShare) m_fProjectTotalResourceShare = 1;

    return m_iGet_simple_gui2_rpc_result;
}


int CMainDocument::GetSimpleProjectCount() {
    int iCount = -1;

    CachedSimpleGUIUpdate();
    CachedStateUpdate();

    if (!state.projects.empty())
        iCount = (int)state.projects.size();

    return iCount;
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

wxString suspend_reason_wxstring(int reason) {
    switch (reason) {
    case SUSPEND_REASON_BATTERIES: return _("on batteries");
    case SUSPEND_REASON_USER_ACTIVE: return _("computer is in use");
    case SUSPEND_REASON_USER_REQ: return _("user request");
    case SUSPEND_REASON_TIME_OF_DAY: return _("time of day");
    case SUSPEND_REASON_BENCHMARKS: return _("CPU benchmarks in progress");
    case SUSPEND_REASON_DISK_SIZE: return _("need disk space - check preferences");
    case SUSPEND_REASON_NO_RECENT_INPUT: return _("computer is not in use");
    case SUSPEND_REASON_INITIAL_DELAY: return _("starting up");
    case SUSPEND_REASON_EXCLUSIVE_APP_RUNNING: return _("an exclusive app is running");
    case SUSPEND_REASON_CPU_USAGE: return _("CPU is busy");
    case SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED: return _("network bandwidth limit exceeded");
    case SUSPEND_REASON_OS: return _("requested by operating system");
    case SUSPEND_REASON_PODMAN_INIT: return _("Podman initializing");
    }
    return _("unknown reason");
}

bool uses_gpu(RESULT* r) {
    // kludge.  But r->avp isn't populated.
    return (strstr(r->resources, "GPU") != NULL);
}

wxString result_description(RESULT* result, bool show_resources) {
    CMainDocument* doc = wxGetApp().GetDocument();
    PROJECT* project;
    CC_STATUS       status;
    int             retval;
    wxString strBuffer= wxEmptyString;

    strBuffer.Clear();
    retval = doc->GetCoreClientStatus(status);
    if (retval || !result) {
        return strBuffer;
    }

    if (result->coproc_missing) {
        strBuffer += _("GPU missing, ");
    }

    project = doc->state.lookup_project(result->project_url);
    int throttled = status.task_suspend_reason & SUSPEND_REASON_CPU_THROTTLE;
    switch(result->state) {
    case RESULT_NEW:
        strBuffer += _("New");
        break;
    case RESULT_FILES_DOWNLOADING:
        if (result->ready_to_report) {
            strBuffer += _("Download failed");
        } else {
            strBuffer += _("Downloading");
            if (status.network_suspend_reason) {
                strBuffer += _(" (suspended - ");
                strBuffer += suspend_reason_wxstring(status.network_suspend_reason);
                strBuffer += _(")");
            }
        }
        break;
    case RESULT_FILES_DOWNLOADED:
        if (result->project_suspended_via_gui) {
            strBuffer += _("Project suspended by user");
        } else if (result->suspended_via_gui) {
            strBuffer += _("Task suspended by user");
        } else if (status.task_suspend_reason && !throttled && result->active_task_state != PROCESS_EXECUTING) {
            // an NCI process can be running even though computation is suspended
            // (because of <dont_suspend_nci>
            //
            strBuffer += _("Suspended - ");
            strBuffer += suspend_reason_wxstring(status.task_suspend_reason);
        } else if (status.gpu_suspend_reason && uses_gpu(result)) {
            strBuffer += _("GPU suspended - ");
            strBuffer += suspend_reason_wxstring(status.gpu_suspend_reason);
        } else if (result->active_task) {
            if (result->too_large) {
                strBuffer += _("Waiting for memory");
            } else if (result->needs_shmem) {
                strBuffer += _("Waiting for shared memory");
            } else if (result->scheduler_state == CPU_SCHED_SCHEDULED) {
                strBuffer += _("Running");
                if (project && project->non_cpu_intensive) {
                    strBuffer += _(" (non-CPU-intensive)");
                }
            } else if (result->scheduler_state == CPU_SCHED_PREEMPTED) {
                strBuffer += _("Waiting to run");
            } else if (result->scheduler_state == CPU_SCHED_UNINITIALIZED) {
                strBuffer += _("Ready to start");
            }
        } else {
            strBuffer += _("Ready to start");
        }
        if (result->scheduler_wait) {
            if (strlen(result->scheduler_wait_reason)) {
                strBuffer = _("Postponed: ");
                strBuffer += wxString(result->scheduler_wait_reason, wxConvUTF8);
            } else {
                strBuffer = _("Postponed");
            }
        }
        if (result->network_wait) {
            strBuffer = _("Waiting for network access");
        }
        break;
    case RESULT_COMPUTE_ERROR:
        strBuffer += _("Computation error");
        break;
    case RESULT_FILES_UPLOADING:
        if (result->ready_to_report) {
            strBuffer += _("Upload failed");
        } else {
            strBuffer += _("Uploading");
            if (status.network_suspend_reason) {
                strBuffer += _(" (suspended - ");
                strBuffer += suspend_reason_wxstring(status.network_suspend_reason);
                strBuffer += _(")");
            }
        }
        break;
    case RESULT_ABORTED:
        switch(result->exit_status) {
        case EXIT_ABORTED_VIA_GUI:
            strBuffer += _("Aborted by user");
            break;
        case EXIT_ABORTED_BY_PROJECT:
            strBuffer += _("Aborted by project");
            break;
        case EXIT_UNSTARTED_LATE:
            strBuffer += _("Aborted: not started by deadline");
            break;
        case EXIT_DISK_LIMIT_EXCEEDED:
            strBuffer += _("Aborted: task disk limit exceeded");
            break;
        case EXIT_TIME_LIMIT_EXCEEDED:
            strBuffer += _("Aborted: run time limit exceeded");
            break;
        case EXIT_MEM_LIMIT_EXCEEDED:
            strBuffer += _("Aborted: memory limit exceeded");
            break;
        default:
            strBuffer += _("Aborted");
        }
        break;
    default:
        if (result->got_server_ack) {
            strBuffer += _("Acknowledged");
        } else if (result->ready_to_report) {
            strBuffer += _("Ready to report");
        } else {
            strBuffer.Format(_("Error: invalid state '%d'"), result->state);
        }
        break;
    }
    if (strlen(result->resources)>1 && show_resources) {
        strBuffer += wxString(wxT(" (")) + wxString(result->resources, wxConvUTF8) + wxString(wxT(")"));
    }
    return strBuffer;
}

static void hsv2rgb(
    double h, double s, double v, double& r, double& g, double& b
) {
    double m, n, f;
    int i = floor(h);
    f = h - i;
    if (!(i&1)) f = 1 - f;
    m = v * (1 - s);
    n = v * (1 - s*f);
    switch (i) {
    case 6:
    case 0: r = v; g = n; b = m; return;
    case 1: r = n; g = v; b = m; return;
    case 2: r = m; g = v; b = n; return;
    case 3: r = m; g = n; b = v; return;
    case 4: r = n; g = m; b = v; return;
    case 5: r = v; g = m; b = n; return;
    }
}

// return the ith out of n maximally distinct colors
//
void color_cycle(int i, int n, wxColour& color) {
    double h = (double)i/(double)n;
    double r = 0.0, g = 0.0, b = 0.0;
    double v = .75;
    if (n > 6) v = .6 + (i % 3)*.1;
        // cycle through 3 different brightnesses
    hsv2rgb(h*6, .5, v, r, g, b);
    unsigned char cr = (unsigned char) (r*256);
    unsigned char cg = (unsigned char) (g*256);
    unsigned char cb = (unsigned char) (b*256);
    color = wxColour(cr, cg, cb);
}

wxString FormatTime(double secs) {
    if (secs <= 0) {
        return wxT("---");
    }

    wxTimeSpan ts = convert_to_timespan(secs);
    return ts.Format((secs>=86400)?"%Dd %H:%M:%S":"%H:%M:%S");
}

wxString format_number(double x, int nprec) {
    return wxNumberFormatter::ToString(x, nprec);

}

wxTimeSpan convert_to_timespan(double secs) {
    wxInt32 iHour = (wxInt32)(secs / (60 * 60));
    wxInt32 iMin  = (wxInt32)(secs / 60) % 60;
    wxInt32 iSec  = (wxInt32)(secs) % 60;
    wxTimeSpan ts = wxTimeSpan(iHour, iMin, iSec);
    return (ts);
}

// the autoattach process deletes the installer filename file when done
//
bool autoattach_in_progress() {
    return boinc_file_exists(ACCOUNT_DATA_FILENAME) != 0;
}
