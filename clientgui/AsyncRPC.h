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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _ASYNCRPC_H_
#define _ASYNCRPC_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AsyncRPC.cpp"
#endif

#include "wx/thread.h"

#include "BOINCBaseFrame.h"

//#include "common_defs.h"
//#include "gui_rpc_client.h"

#define USE_RPC_DLG_TIMER 0
#define USE_CRITICAL_SECTIONS_FOR_ASYNC_RPCS 0

class CMainDocument;    // Forward declaration


enum RPC_SELECTOR {
    RPC_INIT_POLL = 1,
    RPC_AUTHORIZE,
    RPC_INIT_ASYNCH,
    RPC_EXCHANGE_VERSIONS,
    RPC_GET_STATE,
    RPC_GET_HOST_INFO,
    RPC_GET_CC_STATUS,
    RPC_GET_PROJECT_STATUS,
    RPC_GET_RESULTS,
    RPC_GET_MESSAGES,
    RPC_GET_FILE_TRANSFERS,
    RPC_GET_DISK_USAGE,
    RPC_GET_STATISTICS,
    RPC_GET_PROXY_SETTINGS,
    RPC_GET_SIMPLE_GUI_INFO,
    RPC_SET_RUN_MODE,
    RPC_SET_NETWORK_MODE,
    RPC_RUN_BENCHMARKS,
    RPC_QUIT,
    RPC_SET_PROXY_SETTINGS,
    RPC_PROJECT_OP,
    RPC_RESULT_OP,
    RPC_SHOW_GRAPHICS,
    RPC_FILE_TRANSFER_OP,
    RPC_ACCT_MGR_RPC,
    RPC_ACCT_MGR_RPC_POLL,
    RPC_GET_PROJECT_CONFIG,
    RPC_GET_PROJECT_CONFIG_POLL,
    RPC_ACCT_MGR_INFO,
    RPC_READ_GLOBAL_PREFS_OVERRIDE,
    RPC_READ_CC_CONFIG,
    RPC_NETWORK_AVAILABLE,
    RPC_GET_PROJECT_INIT_STATUS,
    RPC_INIT,
    RPC_GET_GLOBAL_PREFS_WORKING_STRUCT,
    RPC_GET_GLOBAL_PREFS_OVERRIDE_STRUCT,
    RPC_SET_GLOBAL_PREFS_OVERRIDE_STRUCT,
    RPC_CREATE_ACCOUNT,
    RPC_CREATE_ACCOUNT_POLL,
    RPC_LOOKUP_ACCOUNT,
    RPC_LOOKUP_ACCOUNT_POLL,
    RPC_PROJECT_ATTACH,
    RPC_PROJECT_ATTACH_FROM_FILE,
    RPC_PROJECT_ATTACH_POLL,
    RPC_GET_ALL_PROJECTS_LIST
};


struct ASYNC_RPC_REQUEST {
    RPC_SELECTOR which_rpc;
    void *inBuf;
    void *exchangeBuf;
    void *outBuf;
    wxEvent *event;
    wxEvtHandler *eventHandler;
    wxDateTime *completionTime;
    bool isActive;

    ASYNC_RPC_REQUEST();
    ~ASYNC_RPC_REQUEST();

    void clear();
    bool isSameAs(ASYNC_RPC_REQUEST& otherRequest);
};


class RPCThread : public wxThread
{
public:
    RPCThread(CMainDocument *pDoc);
    virtual void *Entry();
    virtual void OnExit();
    
private:
    int ProcessRPCRequest();
    
    CMainDocument           *m_Doc;
};


class AsyncRPCDlg : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( AsyncRPCDlg )

public:
    AsyncRPCDlg();
    void    OnRPCDlgTimer(wxTimerEvent &event);

#if USE_RPC_DLG_TIMER
    ~AsyncRPCDlg();
private:
    wxTimer*        m_pDlgDelayTimer;

    DECLARE_EVENT_TABLE()
#endif  // USE_RPC_DLG_TIMER
};


class CRPCFinishedEvent : public wxEvent
{
public:
    CRPCFinishedEvent(wxEventType evtType, CBOINCBaseFrame *pFrame)
        : wxEvent(-1, evtType)
        {
            SetEventObject(pFrame);
        }

    virtual wxEvent *       Clone() const { return new CRPCFinishedEvent(*this); }

    void SetInt(int i) { m_retval = i; }
    int GetInt() const { return m_retval; }
private:
    int m_retval;
};

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_RPC_FINISHED, -1 )
END_DECLARE_EVENT_TYPES()

#define EVT_RPC_FINISHED(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_RPC_FINISHED, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),



#endif // _ASYNCRPC_H_
