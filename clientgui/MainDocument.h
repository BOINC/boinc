// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#ifndef BOINC_MAINDOCUMENT_H
#define BOINC_MAINDOCUMENT_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "MainDocument.cpp"
#endif

#include <vector>
#include "common_defs.h"
#include "gui_rpc_client.h"
#include "AsyncRPC.h"

typedef struct {
    int slot;
    std::string project_url;
    std::string name;
#ifdef _WIN32
    HANDLE pid;
#else
    int pid;
#ifdef __APPLE__
    int gfx_pid;  // Used only on Mac
#endif
#endif
} RUNNING_GFX_APP;


///
/// Bitmask values for GetCurrentViewPage()
/// Used by CMainDocument::RunPeriodicRPCs() and Mac Accessibility
///
#define VW_PROJ   1
#define VW_TASK   2
#define VW_XFER   4
#define VW_STAT   8
#define VW_DISK   16
#define VW_NOTIF  128
#define VW_SGUI   256
#define VW_SMSG   2048
#define VW_SNOTIF 4096


extern bool g_use_sandbox;

class CMainDocument;
class CBOINCClientManager;

// GUI RPC connection to a client (should change the name)
//
class CNetworkConnection : public wxObject {
public:
    CNetworkConnection(CMainDocument* pDocument);
    ~CNetworkConnection();

    void           Poll();
    void           FireReconnectEvent() { m_bConnectEvent = true; };
    void           ForceDisconnect() { m_bForceReconnect = false; m_bReconnectOnError = false; m_bConnectEvent = false; SetStateDisconnected(); };
    void           ForceReconnect() { m_bForceReconnect = true; SetStateDisconnected(); };
    int            FrameShutdownDetected();
    int            GetConnectedComputerName(wxString& strMachine);
    int            GetConnectedComputerVersion(wxString& strVersion);
    int            GetConnectingComputerName(wxString& strMachine);
    bool           IsComputerNameLocal(const wxString& strMachine);
    int            GetLocalPassword(wxString& strPassword);
    int SetComputer(
        const wxString& szComputer, const int iPort, const wxString& szPassword, const bool bUseDefaultPassword
    );
    void           SetStateError();
    void           SetStateErrorAuthentication();
    void           SetStateReconnecting();
    void           SetStateSuccess(wxString& strComputer, wxString& strComputerPassword);
    void           SetStateDisconnected();
    bool           IsConnectEventSignaled() { return m_bConnectEvent; };
    bool           IsConnected() { return m_bConnected; };
    bool           IsReconnecting() { return m_bReconnecting; };
    std::string    password_msg;

private:
    CMainDocument* m_pDocument;
    bool           m_bFrameShutdownDetected;
    bool           m_bConnectEvent;
    bool           m_bForceReconnect;
    bool           m_bReconnectOnError;
    bool           m_bConnected;
    bool           m_bReconnecting;
    bool           m_bUseDefaultPassword;
    bool           m_bUsedDefaultPassword;
    int            m_iReadGUIRPCAuthFailure;
    bool           m_bNewConnection;
    wxString       m_strNewComputerName;
    wxString       m_strNewComputerPassword;
    wxString       m_strConnectedComputerName;
    wxString       m_strConnectedComputerPassword;
    wxString       m_strConnectedComputerVersion;
    int m_iPort;
};


class CMainDocument : public wxObject {
    DECLARE_DYNAMIC_CLASS(CMainDocument)

public:
    CMainDocument();
    ~CMainDocument();

    //
    // Global
    //
private:
    char                        m_szLanguage[256];
    wxDateTime                  m_dtCachedCCStatusTimestamp;
    bool                        m_bClientStartCheckCompleted;


public:
    int                         OnInit();
    int                         OnExit();
    int                         OnPoll();

    int                         OnRefreshState();
    int                         CachedStateUpdate();
    int                         ResetState();

    int                         Connect(
                                    const wxString& szComputer,
                                    const int iPort,
                                    const wxString& szComputerPassword = wxEmptyString,
                                    const bool bDisconnect = FALSE,
                                    const bool bUseDefaultPassword = FALSE
                                );
    int                         Reconnect();

    void                        ForceDisconnect();
    int                         FrameShutdownDetected();
    int                         CoreClientQuit();

    int                         GetConnectedComputerName(wxString& strMachine);
    int                         GetConnectedComputerVersion(wxString& strVersion);
    int                         GetConnectingComputerName(wxString& strMachine);
    bool                        IsComputerNameLocal(const wxString& strMachine);
    bool                        IsConnected();
    bool                        IsReconnecting();

    int                         GetCoreClientStatus(CC_STATUS&, bool bForce = false);
    int                         SetActivityRunMode(int iMode, int iTimeout);
    int                         SetGPURunMode(int iMode, int iTimeout);
    int                         SetNetworkRunMode(int iMode, int iTimeout);

    void                        RefreshRPCs(bool fullReset = false);
    void                        RunPeriodicRPCs(int frameRefreshRate);
    int                         ForceCacheUpdate(bool immediate = true);
    int                         RunBenchmarks();

    bool                        IsUserAuthorized();

    void                        CheckForVersionUpdate(bool showMessage = false);

    CNetworkConnection*         m_pNetworkConnection;
    CBOINCClientManager*        m_pClientManager;
    AsyncRPC                    rpc;
    RPC_CLIENT                  rpcClient;
    PROJECTS                    async_projects_update_buf;

    CC_STATE                    state;
    CC_STATE                    async_state_buf;
    int                         m_iGet_state_rpc_result;

    CC_STATUS                   status;
    CC_STATUS                   async_status_buf;
    int                         m_iGet_status_rpc_result;
    wxDateTime                  m_dtCachedStateTimestamp;

    //
    // Async RPC support
    //
public:
    int                         RequestRPC(ASYNC_RPC_REQUEST& request, bool hasPriority = false);
    void                        OnRPCComplete(CRPCFinishedEvent& event);
    ASYNC_RPC_REQUEST*          GetCurrentRPCRequest() { return &current_rpc_request; }
    bool                        WaitingForRPC() { return m_bWaitingForRPC; }
    wxDialog*                   GetRPCWaitDialog() { return m_RPCWaitDlg; }
//    void                      TestAsyncRPC();      // For testing Async RPCs
    RPCThread*                  m_RPCThread;
    bool                        m_bRPCThreadIsReady;
    bool                        m_bShutDownRPCThread;

private:
    void                        HandleCompletedRPC();
    void                        KillRPCThread();
    int                         CopyProjectsToStateBuffer(PROJECTS& p, CC_STATE& state);
    ASYNC_RPC_REQUEST           current_rpc_request;
    AsyncRPCDlg*                m_RPCWaitDlg;
    std::vector<ASYNC_RPC_REQUEST> RPC_requests;
    bool                        m_bWaitingForRPC;
    bool                        m_bNeedRefresh;
    bool                        m_bNeedTaskBarRefresh;
    BOINC_Mutex*                m_pRPC_Thread_Mutex;
    BOINC_Condition*            m_pRPC_Thread_Condition;
    BOINC_Mutex*                m_pRPC_Request_Mutex;
    BOINC_Condition*            m_pRPC_Request_Condition;
    wxDateTime                  m_dtLasAsyncRPCDlgTime;
    wxDateTime                  m_dtLastFrameViewRefreshRPCTime;
    bool                        m_bAutoAttaching;

    //
    // Projects Tab
    //
private:
    int                         m_iGet_project_status1_rpc_result;
    wxDateTime                  m_dtProjectsStatusTimestamp;

public:
    int                         CachedProjectStatusUpdate(bool bForce = false);
    PROJECT*                    project(unsigned int);
	PROJECT*                    project(char* url);
    double                       m_fProjectTotalResourceShare;

    int                         GetProjectCount();

    int                         ProjectNoMoreWork(int iIndex);
    int                         ProjectAllowMoreWork(int iIndex);
    int                         ProjectDetach(int iIndex);
    int                         ProjectUpdate(int iIndex);
    int                         ProjectReset(int iIndex);
    int                         ProjectSuspend(int iIndex);
    int                         ProjectResume(int iIndex);
    RUNNING_GFX_APP*            GetRunningGraphicsApp(RESULT* result);
#ifdef _WIN32
    void                        KillGraphicsApp(HANDLE pid);
#else
    void                        KillGraphicsApp(int tpid);
#endif
#ifdef __APPLE__
    int                         GetGFXPIDFromForkedPID(RUNNING_GFX_APP* gfx_app);
    int                         fix_slot_file_owners(int slot);
#endif

    //
    // Work Tab
    //
private:
    int                         CachedResultsStatusUpdate();
    wxDateTime                  m_dtResultsTimestamp;
    double                      m_fResultsRPCExecutionTime;
    wxDateTime                  m_dtKillInactiveGfxTimestamp;
    std::vector<RUNNING_GFX_APP> m_running_gfx_apps;
    void                        KillAllRunningGraphicsApps();
    void                        KillInactiveGraphicsApps();

public:
    RESULTS                     results;
    RESULTS                     async_results_buf;
    int                         m_iGet_results_rpc_result;
    bool                        m_ActiveTasksOnly;

    RESULT*                     result(unsigned int);
    RESULT*                     result(const wxString& name, const wxString& project_url);

    int                         GetWorkCount();

    int                         WorkSuspend(const char* url, const char* name);
    int                         WorkResume(const char* url, const char* name);
    int                         WorkShowGraphics(RESULT* result);
    int                         WorkShowVMConsole(RESULT* result);
    int                         WorkAbort(const char* url, const char* name);
    CC_STATE*                   GetState() { return &state; };


    //
    // Notices Tab
    //
private:
    wxDateTime                  m_dtNoticesTimeStamp;

    int                         m_iNoticeSequenceNumber;
    int                         m_iLastReadNoticeSequenceNumber;
    double                      m_dLastReadNoticeArrivalTime;
    bool                        m_bWaitingForGetNoticesRPC;

public:
    NOTICES                     notices;
    int                         m_iGet_notices_rpc_result;

    NOTICE*                     notice(unsigned int);
    int                         CachedNoticeUpdate();

    int                         GetNoticeCount();
    int                         GetUnreadNoticeCount();

    void                        SaveUnreadNoticeInfo();
    void                        RestoreUnreadNoticeInfo();

    void                        UpdateUnreadNoticeState();
    int                         ResetNoticeState();


    //
    // Messages Tab
    //
private:


public:
    MESSAGES                    messages;
    int                         m_iGet_messages_rpc_result;

    MESSAGE*                    message(unsigned int);
    int                         CachedMessageUpdate();

    int                         GetMessageCount();

    int                         ResetMessageState();

    int                         m_iFirstMessageSequenceNumber;
    int                         m_iLastMessageSequenceNumber;

    int                         GetFirstMsgSeqNum() { return m_iFirstMessageSequenceNumber; }
    int                         GetLastMsgSeqNum() { return m_iLastMessageSequenceNumber; }

    //
    // Transfers Tab
    //
private:
    int                         CachedFileTransfersUpdate();
    wxDateTime                  m_dtFileTransfersTimestamp;

public:
    FILE_TRANSFERS              ft;
    FILE_TRANSFERS              async_ft_buf;
    int                         m_iGet_file_transfers_rpc_result;

    FILE_TRANSFER*              file_transfer(unsigned int);
    FILE_TRANSFER*              file_transfer(const wxString& fileName, const wxString& project_url);

    int                         GetTransferCount();

    int                         TransferRetryNow(int iIndex);
    int                         TransferRetryNow(const wxString& fileName, const wxString& project_url);
    int                         TransferAbort(int iIndex);
    int                         TransferAbort(const wxString& fileName, const wxString& project_url);


    //
    // Disk Tab
    //
private:
    wxDateTime                  m_dtDiskUsageTimestamp;

public:
    DISK_USAGE                  disk_usage;
    DISK_USAGE                  async_disk_usage_buf;
    int                         m_iGet_dsk_usage_rpc_result;

    PROJECT*                    DiskUsageProject(unsigned int);
    int                         CachedDiskUsageUpdate();

	//
	// Statistics Tab
	//
private:
    int                         CachedStatisticsStatusUpdate();
    wxDateTime                  m_dtStatisticsStatusTimestamp;

public:
    PROJECTS                    statistics_status;
    PROJECTS                    async_statistics_status_buf;
    PROJECT*                    statistic(unsigned int);
    int                         m_iGet_statistics_rpc_result;

    int                         GetStatisticsCount();


	//
	// Proxy Configuration
	//
private:

public:
	GR_PROXY_INFO   			proxy_info;
    int                         GetProxyConfiguration();
    int                         SetProxyConfiguration();


    //
    // Simple GUI Updates
    //
    int                         m_iGet_simple_gui2_rpc_result;
    int                         m_iAcct_mgr_info_rpc_result;
    int                         CachedSimpleGUIUpdate(bool bForce = false);
private:
    wxDateTime                  m_dtCachedSimpleGUITimestamp;
    wxDateTime                  m_dtCachedAcctMgrInfoTimestamp;

public:
    ACCT_MGR_INFO               ami;
    ACCT_MGR_INFO               async_ami_buf;
    int                         GetSimpleProjectCount();
    int                         GetSimpleGUIWorkCount();

};

extern wxString suspend_reason_wxstring(int reason);
extern wxString result_description(RESULT*, bool show_resources=true);
extern wxString process_client_message(const char*);
extern void localize(wxString& strMessage);
extern void eol_to_br(wxString& strMessage);
extern void remove_eols(wxString& strMessage);
extern void https_to_http(wxString& strMessage);
extern void color_cycle(int i, int n, wxColour& color);
extern wxString FormatTime(double secs);
extern wxTimeSpan convert_to_timespan(double secs);
extern bool autoattach_in_progress();


wxString format_number(double x, int nprec);

#ifdef SANDBOX
#define BOINC_MASTER_GROUP_NAME "boinc_master"
#endif

#endif
