// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//

#ifndef _MAINDOCUMENT_H_
#define _MAINDOCUMENT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "MainDocument.cpp"
#endif

#include "gui_rpc_client.h"


class CMainDocument : public wxObject
{
    DECLARE_DYNAMIC_CLASS(CMainDocument)

public:
    CMainDocument();
    ~CMainDocument();

    enum RESULTTYPES
    {
        RESULT_NEW = 0,
        RESULT_FILES_DOWNLOADING = 1,
        RESULT_FILES_DOWNLOADED = 2,
        RESULT_COMPUTE_ERROR = 3,
        RESULT_FILES_UPLOADING = 4,
        RESULT_FILES_UPLOADED = 5
    };

    enum CPUSCHEDTYPES
    {
        CPU_SCHED_UNINITIALIZED = 0,
        CPU_SCHED_PREEMPTED = 1,
        CPU_SCHED_SCHEDULED = 2
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
    wxDateTime                  m_dtCachedActivityRunModeTimestamp;
    wxDateTime                  m_dtCachedNetworkRunModeTimestamp;
    bool                        m_bCachedStateLocked;

    bool                        m_bIsConnected;

    wxInt32                     m_iCachedActivityRunMode;
    wxInt32                     m_iCachedNetworkRunMode;

    wxInt32                     CachedStateUpdate();

public:

    wxInt32                     OnInit();
    wxInt32                     OnExit();
    wxInt32                     OnIdle();

    wxInt32                     CachedStateLock();
    wxInt32                     CachedStateUnlock();

    wxInt32                     GetActivityRunMode( wxInt32& iMode );
    wxInt32                     SetActivityRunMode( wxInt32 iMode );
    wxInt32                     GetNetworkRunMode( wxInt32& iMode );
    wxInt32                     SetNetworkRunMode( wxInt32 iMode );


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

    wxInt32                     ProjectAttach( wxString& strURL, wxString& strAccountKey );
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
    bool                        IsWorkAcknowledged( wxInt32 iIndex );
    bool                        IsWorkActive( wxInt32 iIndex );
    bool                        IsWorkReadyToReport( wxInt32 iIndex );
    bool                        IsWorkSuspended( wxInt32 iIndex );

    wxInt32                     WorkSuspend( wxInt32 iIndex );
    wxInt32                     WorkResume( wxInt32 iIndex );
    wxInt32                     WorkShowGraphics( wxInt32 iIndex, bool bFullScreen );
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

    wxInt32                     GetProxyInfo();
    wxInt32                     SetProxyInfo();

    bool use_http_proxy;
    bool use_socks_proxy;
    bool use_http_authentication;
    int socks_version;
    std::string socks_server_name;
    std::string http_server_name;
    int socks_server_port;
    int http_server_port;
    std::string http_user_name;
    std::string http_user_passwd;
    std::string socks5_user_name;
    std::string socks5_user_passwd;

};

#endif

