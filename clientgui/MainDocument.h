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

    int                         ForceCacheUpdate();
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
    int                         CachedProjectStatusUpdate();

public:
    PROJECTS                    project_status;
    PROJECT*                    project(int);
    float                       m_fProjectTotalResourceShare;

    int                         GetProjectCount();

    int                         ProjectNoMoreWork(int iIndex);
    int                         ProjectAllowMoreWork(int iIndex);
    int                         ProjectAttach(const wxString& strURL, const wxString& strAccountKey);
    int                         ProjectDetach(int iIndex);
    int                         ProjectUpdate(int iIndex);
    int                         ProjectReset(int iIndex);
    int                         ProjectSuspend(int iIndex);
    int                         ProjectResume(int iIndex);


    //
    // Work Tab
    //
private:
    int                         CachedResultsStatusUpdate();

public:
    RESULTS                     results;
    RESULT*                     result(int);

    int                         GetWorkCount();

    int                         WorkSuspend(int iIndex);
    int                         WorkResume(int iIndex);
    int                         WorkShowGraphics(int iIndex, bool bFullScreen, std::string, std::string, std::string);
    int                         WorkAbort(int iIndex);


    //
    // Messages Tab
    //
private:


public:
    MESSAGES                    messages;
    MESSAGE*                    message(int);
    int                         CachedMessageUpdate();

    int                         GetMessageCount();

    int                         ResetMessageState();

    int                         m_iMessageSequenceNumber;


    //
    // Transfers Tab
    //
private:

    int                         CachedFileTransfersUpdate();

public:
    FILE_TRANSFERS              ft;
    FILE_TRANSFER*              file_transfer(int);

    int                         GetTransferCount();

    int                         TransferRetryNow(int iIndex);
    int                         TransferAbort(int iIndex);


    //
    // Resources Tab
    //
private:

    int                         CachedResourceStatusUpdate();

public:
    PROJECTS                    resource_status;
    PROJECT*                    resource(int);

    int                         GetResourceCount();


	//
	// Statistics Tab
	//
private:

    int                         CachedStatisticsStatusUpdate();

public:
	PROJECTS                    statistics_status;
    PROJECT*                    statistic(int);

    int                         GetStatisticsCount();
	

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
    ACCT_MGR_CLIENT             acct_mgr;

    int                         UpdateAccountManagerAccounts();
};

#endif

