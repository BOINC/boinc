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

#ifndef _MAINDOCUMENT_H_
#define _MAINDOCUMENT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "MainDocument.cpp"
#endif

#include "gui_rpc_client.h"
#include "acct_mgr_client.h"

class CMainDocument;

class CNetworkConnection : public wxObject {
public:
    CNetworkConnection(CMainDocument* pDocument);
    ~CNetworkConnection();

    virtual void*  Poll();
    void           FireReconnectEvent() { m_bConnectEvent = true; };
    void           ForceReconnect() { m_bForceReconnect = true; };
    int            FrameShutdownDetected();
    int            GetConnectedComputerName(wxString& strMachine);
    int            GetConnectingComputerName(wxString& strMachine);
    int            SetNewComputerName(const wxChar* szComputer);
    int            SetNewComputerPassword(const wxChar* szPassword);
    void           SetStateError();
    void           SetStateErrorAuthentication();
    void           SetStateReconnecting();
    void           SetStateSuccess(std::string& strComputer, std::string& strComputerPassword);
    void           SetStateDisconnected();
    bool           IsConnectEventSignaled() { return m_bConnectEvent; };
    bool           IsConnected() { return m_bConnected; };
    bool           IsReconnecting() { return m_bReconnecting; };

private:
    CMainDocument* m_pDocument;
    bool           m_bFrameShutdownDetected;
    bool           m_bConnectEvent;
    bool           m_bForceReconnect;
    bool           m_bReconnectOnError;
    bool           m_bConnected;
    bool           m_bReconnecting;
    wxString       m_strNewComputerName;
    wxString       m_strNewComputerPassword;
    wxString       m_strConnectedComputerName;
    wxString       m_strConnectedComputerPassword;
};


class CMainDocument : public wxObject {
    DECLARE_DYNAMIC_CLASS(CMainDocument)

public:
    CMainDocument();
    ~CMainDocument();

    enum RESULTTYPES {
        NEW = RESULT_NEW,
        FILES_DOWNLOADING = RESULT_FILES_DOWNLOADING,
        FILES_DOWNLOADED = RESULT_FILES_DOWNLOADED,
        COMPUTE_ERROR = RESULT_COMPUTE_ERROR,
        FILES_UPLOADING = RESULT_FILES_UPLOADING,
        FILES_UPLOADED = RESULT_FILES_UPLOADED
    };

    enum CPUSCHEDTYPES {
        SCHED_UNINITIALIZED = CPU_SCHED_UNINITIALIZED,
        SCHED_PREEMPTED = CPU_SCHED_PREEMPTED,
        SCHED_SCHEDULED = CPU_SCHED_SCHEDULED
    };

    enum RUNMODETYPES {
        MODE_ALWAYS = RUN_MODE_ALWAYS,
        MODE_NEVER = RUN_MODE_NEVER,
        MODE_AUTO = RUN_MODE_AUTO
    };


    //
    // Global
    //
private:

    CNetworkConnection*         m_pNetworkConnection;

    bool                        m_bCachedStateLocked;

	wxDateTime                  m_dtCachedActivityRunModeTimestamp;
    wxDateTime                  m_dtCachedNetworkRunModeTimestamp;
    wxDateTime                  m_dtCachedActivityStateTimestamp;
    int                         m_iCachedActivityRunMode;
    int                         m_iCachedNetworkRunMode;
    bool                        m_iCachedActivitiesSuspended;
    bool                        m_iCachedNetworkSuspended;

    int                         CachedStateUpdate();
    int                         ForceCacheUpdate();

public:

    int                         OnInit();
    int                         OnExit();
    int                         OnPoll();

    int                         OnRefreshState();
    int                         ResetState();

    int                         Connect(const wxChar* szComputer, const wxChar* szComputerPassword = wxEmptyString, bool bDisconnect = FALSE);

    int                         CachedStateLock();
    int                         CachedStateUnlock();

    int                         FrameShutdownDetected();
    int                         GetCoreClientVersion();
    int                         CoreClientQuit();

    int                         GetConnectedComputerName(wxString& strMachine);
    int                         GetConnectingComputerName(wxString& strMachine);
    bool                        IsConnected();
    bool                        IsReconnecting();

    int                         GetActivityRunMode(int& iMode);
    int                         SetActivityRunMode(int iMode);
    int                         GetNetworkRunMode(int& iMode);
    int                         SetNetworkRunMode(int iMode);
    int                         GetActivityState(bool& bActivitiesSuspended, bool& bNetworkSuspended);

    int                         RunBenchmarks();

    RPC_CLIENT                  rpc;
    CC_STATE                    state;
    HOST_INFO                   host;
    wxDateTime                  m_dtCachedStateTimestamp;
    wxDateTime                  m_dtCachedStateLockTimestamp;


    //
    // Project Tab
    //
private:

    float                       m_fProjectTotalResourceShare;

    int                         CachedProjectStatusUpdate();

public:
    int                         GetProjectCount();
    int                         GetProjectProjectName(int iIndex, wxString& strBuffer);
    int                         GetProjectProjectURL(int iIndex, wxString& strBuffer);
    int                         GetProjectAccountName(int iIndex, wxString& strBuffer);
    int                         GetProjectTeamName(int iIndex, wxString& strBuffer);
    int                         GetProjectTotalCredit(int iIndex, float& fBuffer);
    int                         GetProjectAvgCredit(int iIndex, float& fBuffer);
    int                         GetProjectResourceShare(int iIndex, float& fBuffer);
    int                         GetProjectTotalResourceShare(int iIndex, float& fBuffer);
    int                         GetProjectMinRPCTime(int iIndex, int& iBuffer);
    int                         GetProjectWebsiteCount(int iIndex);
    int                         GetProjectWebsiteName(int iProjectIndex, int iWebsiteIndex, wxString& strBuffer);
    int                         GetProjectWebsiteDescription(int iProjectIndex, int iWebsiteIndex, wxString& strBuffer);
    int                         GetProjectWebsiteLink(int iProjectIndex, int iWebsiteIndex, wxString& strBuffer);
    bool                        IsProjectSuspended(int iIndex);
    bool                        IsProjectRPCPending(int iIndex);
    bool                        IsProjectAllowedToGetWork(int iIndex);


    int                         ProjectNoMoreWork(int iIndex);
    int                         ProjectAllowMoreWork(int iIndex);
    int                         ProjectAttach(const wxString& strURL, const wxString& strAccountKey);
    int                         ProjectDetach(int iIndex);
    int                         ProjectUpdate(int iIndex);
    int                         ProjectReset(int iIndex);
    int                         ProjectSuspend(int iIndex);
    int                         ProjectResume(int iIndex);

    PROJECT*                    project(int);
    PROJECTS                    project_status;


    //
    // Work Tab
    //
private:

    int                         CachedResultsStatusUpdate();

public:

    int                         GetWorkCount();
    int                         GetWorkProjectName(int iIndex, wxString& strBuffer);
    int                         GetWorkProjectURL(int iIndex, wxString& strBuffer);
    int                         GetWorkApplicationName(int iIndex, wxString& strBuffer);
    int                         GetWorkApplicationVersion(int iIndex, int& iBuffer);
    int                         GetWorkName(int iIndex, wxString& strBuffer);
    int                         GetWorkCurrentCPUTime(int iIndex, float& fBuffer);
    int                         GetWorkEstimatedCPUTime(int iIndex, float& fBuffer);
    int                         GetWorkFinalCPUTime(int iIndex, float& fBuffer);
    int                         GetWorkFractionDone(int iIndex, float& fBuffer);
    int                         GetWorkReportDeadline(int iIndex, int& iBuffer);
    int                         GetWorkState(int iIndex);
    int                         GetWorkSchedulerState(int iIndex);
    bool                        IsWorkAborted(int iIndex);
    bool                        IsWorkAcknowledged(int iIndex);
    bool                        IsWorkActive(int iIndex);
    bool                        IsWorkReadyToReport(int iIndex);
    bool                        IsWorkSuspended(int iIndex);
    bool                        IsWorkGraphicsSupported(int iIndex);

    int                         WorkSuspend(int iIndex);
    int                         WorkResume(int iIndex);
    int                         WorkShowGraphics(int iIndex, bool bFullScreen, std::string, std::string, std::string);
    int                         WorkAbort(int iIndex);

    RESULT*                     result(int);
    RESULTS                     results;


    //
    // Messages Tab
    //
private:

    int                         CachedMessageUpdate();

public:

    int                         GetMessageCount();
    int                         GetMessageProjectName(int iIndex, wxString& strBuffer);
    int                         GetMessageTime(int iIndex, wxDateTime& dtBuffer);
    int                         GetMessagePriority(int iIndex, int& iBuffer);
    int                         GetMessageMessage(int iIndex, wxString& strBuffer);

    int                         ResetMessageState();

    MESSAGE*                    message(int);
    MESSAGES                    messages;
    int                         m_iMessageSequenceNumber;


    //
    // Transfers Tab
    //
private:

    int                         CachedFileTransfersUpdate();

public:

    int                         GetTransferCount();
    int                         GetTransferProjectName(int iIndex, wxString& strBuffer);
    int                         GetTransferFileName(int iIndex, wxString& strBuffer);
    int                         GetTransferFileSize(int iIndex, float& fBuffer);
    int                         GetTransferBytesXfered(int iIndex, float& fBuffer);
    int                         GetTransferSpeed(int iIndex, float& fBuffer);
    int                         GetTransferTime(int iIndex, float& fBuffer);
    int                         GetTransferNextRequestTime(int iIndex, int& iBuffer);
    int                         GetTransferStatus(int iIndex, int& iBuffer);
    bool                        IsTransferActive(int iIndex);
    bool                        IsTransferGeneratedLocally(int iIndex);

    int                         TransferRetryNow(int iIndex);
    int                         TransferAbort(int iIndex);

    FILE_TRANSFER*              file_transfer(int);
    FILE_TRANSFERS              ft;


    //
    // Resources Tab
    //
private:

    int                         CachedResourceStatusUpdate();

public:

    int                         GetResourceCount();
    int                         GetResourceProjectName(int iIndex, wxString& strBuffer);
    int                         GetResourceDiskspace(int iIndex, float& fBuffer);

    PROJECT*                    resource(int);
    PROJECTS                    resource_status;


	//
	// Statistics Tab
	//
private:

    int                         CachedStatisticsStatusUpdate();

public:

    int                         GetStatisticsCount();
    int                         GetStatisticsProjectName(int iIndex, wxString& strBuffer);
	
    PROJECT*                    statistic(int);
	PROJECTS                    statistics_status;


	//
	// Proxy Configuration
	//
private:

public:

    int                         GetProxyConfiguration();
    int                         GetProxyHTTPProxyEnabled(bool& bEnabled);
    int                         GetProxyHTTPServerName(wxString& strServerName);
    int                         GetProxyHTTPServerPort(int& iPortNumber);
    int                         GetProxyHTTPUserName(wxString& strUserName);
    int                         GetProxyHTTPPassword(wxString& strPassword);
    int                         GetProxySOCKSProxyEnabled(bool& bEnabled);
    int                         GetProxySOCKSServerName(wxString& strServerName);
    int                         GetProxySOCKSServerPort(int& iPortNumber);
    int                         GetProxySOCKSUserName(wxString& strUserName);
    int                         GetProxySOCKSPassword(wxString& strPassword);

    int                         SetProxyConfiguration();
    int                         SetProxyHTTPProxyEnabled(const bool bEnabled);
    int                         SetProxyHTTPServerName(const wxString& strServerName);
    int                         SetProxyHTTPServerPort(const int iPortNumber);
    int                         SetProxyHTTPUserName(const wxString& strUserName);
    int                         SetProxyHTTPPassword(const wxString& strPassword);
    int                         SetProxySOCKSProxyEnabled(const bool bEnabled);
    int                         SetProxySOCKSServerName(const wxString& strServerName);
    int                         SetProxySOCKSServerPort(const int iPortNumber);
    int                         SetProxySOCKSUserName(const wxString& strUserName);
    int                         SetProxySOCKSPassword(const wxString& strPassword);

	PROXY_INFO					proxy_info;


    //
    // Account Management
    //
private:

public:
    int                         GetAccountManagerName(wxString& strName);

    int                         InitializeAccountManagerLogin(const wxString& strLogin, const wxString& strPassword);
    int                         UpdateAccountManagerAccounts();

    bool                        IsAccountManagerFound();
    bool                        IsAccountManagerLoginFound();

    ACCT_MGR_CLIENT             acct_mgr;

};

#endif

