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


class CMainDocument : public wxObject
{
    DECLARE_DYNAMIC_CLASS(CMainDocument)

public:
    CMainDocument();
    ~CMainDocument();

    enum RESULTTYPES
    {
        NEW = RESULT_NEW,
        FILES_DOWNLOADING = RESULT_FILES_DOWNLOADING,
        FILES_DOWNLOADED = RESULT_FILES_DOWNLOADED,
        COMPUTE_ERROR = RESULT_COMPUTE_ERROR,
        FILES_UPLOADING = RESULT_FILES_UPLOADING,
        FILES_UPLOADED = RESULT_FILES_UPLOADED
    };

    enum CPUSCHEDTYPES
    {
        SCHED_UNINITIALIZED = CPU_SCHED_UNINITIALIZED,
        SCHED_PREEMPTED = CPU_SCHED_PREEMPTED,
        SCHED_SCHEDULED = CPU_SCHED_SCHEDULED
    };

    enum RUNMODETYPES
    {
        MODE_ALWAYS = RUN_MODE_ALWAYS,
        MODE_NEVER = RUN_MODE_NEVER,
        MODE_AUTO = RUN_MODE_AUTO
    };


    //
    // Global
    //
private:

    RPC_CLIENT                  rpc;
    CC_STATE                    state;
    HOST_INFO                   host;
    wxDateTime                  m_dtCachedStateTimestamp;
    wxDateTime                  m_dtCachedStateLockTimestamp;
    bool                        m_bCachedStateLocked;

    bool                        m_bIsConnected;
    wxString                    m_strConnectedComputerName;
    wxString                    m_strConnectedComputerPassword;

	wxDateTime                  m_dtCachedActivityRunModeTimestamp;
    wxDateTime                  m_dtCachedNetworkRunModeTimestamp;
    wxDateTime                  m_dtCachedActivityStateTimestamp;
    wxInt32                     m_iCachedActivityRunMode;
    wxInt32                     m_iCachedNetworkRunMode;
    bool                        m_iCachedActivitiesSuspended;
    bool                        m_iCachedNetworkSuspended;

    wxInt32                     CachedStateUpdate();
    wxInt32                     ForceCacheUpdate();

public:

    wxInt32                     OnInit();
    wxInt32                     OnExit();
    wxInt32                     OnRefreshState();

    wxInt32                     Connect( const wxChar* szComputer, const wxChar* szComputerPassword = wxEmptyString, bool bDisconnect = TRUE );
    wxInt32                     Disconnect();
    wxInt32                     GetConnectedComputerName( wxString& strMachine );
    bool                        IsConnected();

    wxInt32                     CachedStateLock();
    wxInt32                     CachedStateUnlock();

    wxInt32                     GetCoreClientVersion();

    wxInt32                     GetActivityRunMode( wxInt32& iMode );
    wxInt32                     SetActivityRunMode( wxInt32 iMode );
    wxInt32                     GetNetworkRunMode( wxInt32& iMode );
    wxInt32                     SetNetworkRunMode( wxInt32 iMode );
    wxInt32                     GetActivityState( bool& bActivitiesSuspended, bool& bNetworkSuspended );

    wxInt32                     RunBenchmarks();
    wxInt32                     CoreClientQuit();


    //
    // Project Tab
    //
private:

    PROJECTS                    project_status;
    float                       m_fProjectTotalResourceShare;

    wxInt32                     CachedProjectStatusUpdate();

public:

    wxInt32                     GetProjectCount();
    wxInt32                     GetProjectProjectName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetProjectProjectURL( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetProjectAccountName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetProjectTeamName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetProjectTotalCredit( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetProjectAvgCredit( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetProjectResourceShare( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetProjectTotalResourceShare( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetProjectMinRPCTime( wxInt32 iIndex, wxInt32& iBuffer );
    wxInt32                     GetProjectWebsiteCount( wxInt32 iIndex );
    wxInt32                     GetProjectWebsiteName( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer );
    wxInt32                     GetProjectWebsiteDescription( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer );
    wxInt32                     GetProjectWebsiteLink( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer );
    bool                        IsProjectSuspended( wxInt32 iIndex );
    bool                        IsProjectRPCPending( wxInt32 iIndex );

    wxInt32                     ProjectAttach(const wxString& strURL, const wxString& strAccountKey );
    wxInt32                     ProjectDetach( wxInt32 iIndex );
    wxInt32                     ProjectUpdate( wxInt32 iIndex );
    wxInt32                     ProjectReset( wxInt32 iIndex );
    wxInt32                     ProjectSuspend( wxInt32 iIndex );
    wxInt32                     ProjectResume( wxInt32 iIndex );


    //
    // Work Tab
    //
private:

    RESULTS                     results;

    wxInt32                     CachedResultsStatusUpdate();

public:

    wxInt32                     GetWorkCount();
    wxInt32                     GetWorkProjectName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetWorkProjectURL( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetWorkApplicationName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetWorkApplicationVersion(wxInt32 iIndex, wxInt32& iBuffer);
    wxInt32                     GetWorkName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetWorkCurrentCPUTime( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetWorkEstimatedCPUTime( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetWorkFinalCPUTime( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetWorkFractionDone( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetWorkReportDeadline( wxInt32 iIndex, wxInt32& iBuffer );
    wxInt32                     GetWorkState( wxInt32 iIndex );
    wxInt32                     GetWorkSchedulerState( wxInt32 iIndex );
    bool                        IsWorkAborted( wxInt32 iIndex );
    bool                        IsWorkAcknowledged( wxInt32 iIndex );
    bool                        IsWorkActive( wxInt32 iIndex );
    bool                        IsWorkReadyToReport( wxInt32 iIndex );
    bool                        IsWorkSuspended( wxInt32 iIndex );
    bool                        IsWorkGraphicsSupported( wxInt32 iIndex );

    wxInt32                     WorkSuspend( wxInt32 iIndex );
    wxInt32                     WorkResume( wxInt32 iIndex );
    wxInt32                     WorkShowGraphics( wxInt32 iIndex, bool bFullScreen, std::string, std::string, std::string );
    wxInt32                     WorkAbort( wxInt32 iIndex );


    //
    // Messages Tab
    //
private:

    MESSAGES                    messages;
    wxInt32                     m_iMessageSequenceNumber;

    wxInt32                     CachedMessageUpdate();

public:

    wxInt32                     GetMessageCount();
    wxInt32                     GetMessageProjectName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetMessageTime( wxInt32 iIndex, wxDateTime& dtBuffer );
    wxInt32                     GetMessagePriority( wxInt32 iIndex, wxInt32& iBuffer );
    wxInt32                     GetMessageMessage( wxInt32 iIndex, wxString& strBuffer );

    wxInt32                     ResetMessageState();


    //
    // Transfers Tab
    //
private:

    FILE_TRANSFERS              ft;

    wxInt32                     CachedFileTransfersUpdate();

public:

    wxInt32                     GetTransferCount();
    wxInt32                     GetTransferProjectName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetTransferFileName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetTransferFileSize( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetTransferBytesXfered( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetTransferSpeed( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetTransferTime( wxInt32 iIndex, float& fBuffer );
    wxInt32                     GetTransferNextRequestTime( wxInt32 iIndex, wxInt32& iBuffer );
    wxInt32                     GetTransferStatus( wxInt32 iIndex, wxInt32& iBuffer );
    bool                        IsTransferActive( wxInt32 iIndex );
    bool                        IsTransferGeneratedLocally( wxInt32 iIndex );

    wxInt32                     TransferRetryNow( wxInt32 iIndex );
    wxInt32                     TransferAbort( wxInt32 iIndex );


    //
    // Resources Tab
    //
private:

    PROJECTS                    resource_status;

    wxInt32                     CachedResourceStatusUpdate();

public:

    wxInt32                     GetResourceCount();
    wxInt32                     GetResourceProjectName( wxInt32 iIndex, wxString& strBuffer );
    wxInt32                     GetResourceDiskspace( wxInt32 iIndex, float& fBuffer );


	//
	// Proxy Configuration
	//
private:

	PROXY_INFO					proxy_info;

public:

    wxInt32                     GetProxyConfiguration();
    wxInt32                     GetProxyHTTPProxyEnabled( bool& bEnabled );
    wxInt32                     GetProxyHTTPServerName( wxString& strServerName );
    wxInt32                     GetProxyHTTPServerPort( wxInt32& iPortNumber );
    wxInt32                     GetProxyHTTPUserName( wxString& strUserName );
    wxInt32                     GetProxyHTTPPassword( wxString& strPassword );
    wxInt32                     GetProxySOCKSProxyEnabled( bool& bEnabled );
    wxInt32                     GetProxySOCKSServerName( wxString& strServerName );
    wxInt32                     GetProxySOCKSServerPort( wxInt32& iPortNumber );
    wxInt32                     GetProxySOCKSUserName( wxString& strUserName );
    wxInt32                     GetProxySOCKSPassword( wxString& strPassword );

    wxInt32                     SetProxyConfiguration();
    wxInt32                     SetProxyHTTPProxyEnabled( const bool bEnabled );
    wxInt32                     SetProxyHTTPServerName( const wxString& strServerName );
    wxInt32                     SetProxyHTTPServerPort( const wxInt32 iPortNumber );
    wxInt32                     SetProxyHTTPUserName( const wxString& strUserName );
    wxInt32                     SetProxyHTTPPassword( const wxString& strPassword );
    wxInt32                     SetProxySOCKSProxyEnabled( const bool bEnabled );
    wxInt32                     SetProxySOCKSServerName( const wxString& strServerName );
    wxInt32                     SetProxySOCKSServerPort( const wxInt32 iPortNumber );
    wxInt32                     SetProxySOCKSUserName( const wxString& strUserName );
    wxInt32                     SetProxySOCKSPassword( const wxString& strPassword );


    //
    // Account Management
    //
private:

    ACCT_MGR_CLIENT             acct_mgr;

public:
    wxInt32                     GetAccountManagerName( wxString& strName );

    wxInt32                     InitializeAccountManagerLogin( const wxString& strLogin, const wxString& strPassword );
    wxInt32                     UpdateAccountManagerAccounts();

    bool                        IsAccountManagerFound();
    bool                        IsAccountManagerLoginFound();

};

#endif

