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

#include <vector>
#include "common_defs.h"
#include "gui_rpc_client.h"

typedef struct {
    int slot;
    std::string project_url;
    std::string name;
#ifdef _WIN32
    HANDLE pid;
#else
    int pid;
#endif
} RUNNING_GFX_APP;


extern bool g_use_sandbox;

class CMainDocument;

class CNetworkConnection : public wxObject {
public:
    CNetworkConnection(CMainDocument* pDocument);
    ~CNetworkConnection();

    void           Poll();
    void           FireReconnectEvent() { m_bConnectEvent = true; };
    void           ForceReconnect() { m_bForceReconnect = true; SetStateDisconnected(); };
    int            FrameShutdownDetected();
    int            GetConnectedComputerName(wxString& strMachine);
    int            GetConnectedComputerVersion(wxString& strVersion);
    int            GetConnectingComputerName(wxString& strMachine);
    bool           IsComputerNameLocal(const wxString& strMachine);
    void           GetLocalPassword(wxString& strPassword);
    int            SetComputer(const wxChar* szComputer, const wxChar* szPassword, const bool bUseDefaultPassword);
    void           SetStateError();
    void           SetStateErrorAuthentication();
    void           SetStateReconnecting();
    void           SetStateSuccess(wxString& strComputer, wxString& strComputerPassword);
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
    bool           m_bUseDefaultPassword;
    bool           m_bUsedDefaultPassword;
    bool           m_bNewConnection;
    wxString       m_strNewComputerName;
    wxString       m_strNewComputerPassword;
    wxString       m_strConnectedComputerName;
    wxString       m_strConnectedComputerPassword;
    wxString       m_strConnectedComputerVersion;
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

    wxDateTime                  m_dtCachedCCStatusTimestamp;


public:
    int                         CachedStateUpdate();

    CNetworkConnection*         m_pNetworkConnection;

    int                         OnInit();
    int                         OnExit();
    int                         OnPoll();

    int                         OnRefreshState();
    int                         ResetState();

    int                         Connect(
                                    const wxChar* szComputer,
                                    const wxChar* szComputerPassword = wxEmptyString,
                                    const bool bDisconnect = FALSE,
                                    const bool bUseDefaultPassword = FALSE
                                );
    int                         Reconnect();

    int                         CachedStateLock();
    int                         CachedStateUnlock();

    int                         FrameShutdownDetected();
    int                         CoreClientQuit();

    int                         GetConnectedComputerName(wxString& strMachine);
    int                         GetConnectedComputerVersion(wxString& strVersion);
    int                         GetConnectingComputerName(wxString& strMachine);
    bool                        IsComputerNameLocal(const wxString strMachine);
    bool                        IsConnected();
    bool                        IsReconnecting();

    int                         GetCoreClientStatus(CC_STATUS&, bool bForce = false);
    int                         SetActivityRunMode(int iMode, int iTimeout);
    int                         SetNetworkRunMode(int iMode, int iTimeout);

    int                         ForceCacheUpdate();
    int                         RunBenchmarks();

    bool                        IsUserAuthorized();

    RPC_CLIENT                  rpc;
    CC_STATE                    state;
    CC_STATUS                   status;
    HOST_INFO                   host;
    wxDateTime                  m_dtCachedStateTimestamp;


    //
    // Project Tab
    //
private:
    int                         CachedProjectStatusUpdate();
    wxDateTime                  m_dtProjecStatusTimestamp;

public:
    PROJECT*                    project(unsigned int);
	PROJECT*                    project(const wxString& projectname);
    float                       m_fProjectTotalResourceShare;

    int                         GetProjectCount();

    int                         ProjectNoMoreWork(int iIndex);
	int                         ProjectNoMoreWork(const wxString& projectname);
    int                         ProjectAllowMoreWork(int iIndex);
	int                         ProjectAllowMoreWork(const wxString& projectname);
    int                         ProjectAttach(const wxString& strURL, const wxString& strAccountKey);
    int                         ProjectDetach(int iIndex);
	int                         ProjectDetach(const wxString& projectname);
    int                         ProjectUpdate(int iIndex);
	int                         ProjectUpdate(const wxString& projectname);
    int                         ProjectReset(int iIndex);
	int                         ProjectReset(const wxString& projectname);
    int                         ProjectSuspend(int iIndex);
	int                         ProjectSuspend(const wxString& projectname);
    int                         ProjectResume(int iIndex);
	int                         ProjectResume(const wxString& projectname);


    //
    // Work Tab
    //
private:
    int                         CachedResultsStatusUpdate();
    wxDateTime                  m_dtResultsTimestamp;
    wxDateTime                  m_dtKillInactiveGfxTimestamp;
    std::vector<RUNNING_GFX_APP> m_running_gfx_apps;
    RUNNING_GFX_APP*            GetRunningGraphicsApp(RESULT* result, int slot);
    void                        KillAllRunningGraphicsApps();
    void                        KillInactiveGraphicsApps();

public:
    RESULTS                     results;
    RESULT*                     result(unsigned int);
	RESULT*						result(const wxString& name);

    int                         GetWorkCount();

    int                         WorkSuspend(
                                    std::string& strProjectURL,
                                    std::string& strName
                                );
    int                         WorkResume(
                                    std::string& strProjectURL,
                                    std::string& strName
                                );
    int                         WorkShowGraphics(RESULT* result);
    int                         WorkAbort(
                                    std::string& strProjectURL,
                                    std::string& strName
                                );


    //
    // Messages Tab
    //
private:


public:
    MESSAGES                    messages;
    MESSAGE*                    message(unsigned int);
    int                         CachedMessageUpdate();

    int                         GetMessageCount();

    int                         ResetMessageState();

    int                         m_iMessageSequenceNumber;


    //
    // Transfers Tab
    //
private:
    int                         CachedFileTransfersUpdate();
    wxDateTime                  m_dtFileTransfersTimestamp;

public:
    FILE_TRANSFERS              ft;
    FILE_TRANSFER*              file_transfer(unsigned int);
	FILE_TRANSFER*              file_transfer(const wxString& fileName);

    int                         GetTransferCount();

    int                         TransferRetryNow(int iIndex);
	int                         TransferRetryNow(const wxString& fileName);
    int                         TransferAbort(int iIndex);
	int                         TransferAbort(const wxString& fileName);


    //
    // Disk Tab
    //
private:
    wxDateTime                  m_dtDiskUsageTimestamp;

public:
    DISK_USAGE                  disk_usage;
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
    PROJECT*                    statistic(unsigned int);

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
private:
    wxDateTime                  m_dtCachedSimpleGUITimestamp;
    int                         CachedSimpleGUIUpdate();

public:
    int                         GetSimpleGUIWorkCount();

};

#endif

#ifdef SANDBOX
#define BOINC_MASTER_GROUP_NAME "boinc_master"
#endif
