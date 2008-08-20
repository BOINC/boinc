// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
#pragma implementation "AsyncRPC.h"
#endif

#include <vector>

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "AsyncRPC.h"
#include "BOINCBaseFrame.h"

// Delay in milliseconds before showing AsyncRPCDlg
#define RPC_WAIT_DLG_DELAY 1500

ASYNC_RPC_REQUEST::ASYNC_RPC_REQUEST() {
    clear();
}


ASYNC_RPC_REQUEST::~ASYNC_RPC_REQUEST() {
    clear();
}


void ASYNC_RPC_REQUEST::clear() {
    which_rpc = (RPC_SELECTOR) 0;
    arg1 = NULL;
    exchangeBuf = NULL;
    arg2 = NULL;
    arg3 = NULL;
    arg4 = NULL;
    event = NULL;
    eventHandler = NULL;
    completionTime = NULL;
    resultPtr = NULL;
    isActive = false;
}


bool ASYNC_RPC_REQUEST::isSameAs(ASYNC_RPC_REQUEST& otherRequest) {
    if (which_rpc != otherRequest.which_rpc) return false;
    if (arg1 != otherRequest.arg1) return false;
    if (exchangeBuf != otherRequest.exchangeBuf) return false;
    if (arg2 != otherRequest.arg2) return false;
    if (arg3 != otherRequest.arg3) return false;
    if (arg4 != otherRequest.arg4) return false;
    if (event != otherRequest.event) {
        if (event->GetEventType() != (otherRequest.event)->GetEventType()) return false;
        if (event->GetId() != (otherRequest.event)->GetId()) return false;
        if (event->GetEventObject() != (otherRequest.event)->GetEventObject()) return false;
    }
    if (eventHandler != otherRequest.eventHandler) return false;
    if (completionTime != otherRequest.completionTime) return false;
    if (resultPtr != otherRequest.resultPtr) return false;
    // OK if isActive doesn't match.
    return true;
}


AsyncRPC::AsyncRPC(CMainDocument *pDoc) {
    m_Doc = pDoc;
}


AsyncRPC::~AsyncRPC() {}


int AsyncRPC::RPC_Wait(RPC_SELECTOR which_rpc, void *arg1, void *arg2, 
    void *arg3, void *arg4, bool hasPriority
) {
    ASYNC_RPC_REQUEST request;
    int retval = 0;

    request.which_rpc = which_rpc;
    request.arg1 = arg1;
    request.arg2 = arg2;
    request.arg3 = arg3;
    request.arg4 = arg4;

    retval = m_Doc->RequestRPC(request, hasPriority);
    return retval;
}


RPCThread::RPCThread(CMainDocument *pDoc)
        : wxThread() {
    m_Doc = pDoc;
}


void RPCThread::OnExit() {
    // Tell CMainDocument that thread has gracefully ended 
    m_Doc->m_RPCThread = NULL;
}


// We don't need critical sections because:
// 1. CMainDocument never modifies mDoc->current_rpc_request while the 
// async RPC thread is using it.
// 2. The async RPC thread never modifies either mDoc->current_rpc_request 
// or the vector of requests mDoc->RPC_requests.

void *RPCThread::Entry() {
    int retval;
    CRPCFinishedEvent RPC_done_event( wxEVT_RPC_FINISHED );

    while(true) {
        // check if we were asked to exit
        if ( TestDestroy() )
            break;

        if (! m_Doc->GetCurrentRPCRequest()->isActive) {
            // Wait until CMainDocument issues next RPC request
#ifdef __WXMSW__       // Until we can suspend the thread without Deadlock on Windows
            Sleep(1);
#else
            Yield();
#endif
            continue;
        }

       if (! m_Doc->IsConnected()) {
            Yield();
        }

        retval = ProcessRPCRequest();
        wxPostEvent( wxTheApp, RPC_done_event );
        }

#ifndef __WXMSW__       // Deadlocks on Windows

    // Use a critical section to prevent a crash during 
    // manager shutdown due to a rare race condition 
    m_Doc->m_critsect.Enter();
    m_Doc->m_critsect.Leave();
#endif //  !!__WXMSW__       // Deadlocks on Windows

    return NULL;
}


int RPCThread::ProcessRPCRequest() {
    int                     retval = 0;
    ASYNC_RPC_REQUEST       *current_request;
    
    current_request = m_Doc->GetCurrentRPCRequest();
    switch (current_request->which_rpc) {
    // RPC_SELECTORS with no arguments
    case RPC_RUN_BENCHMARKS:
    case RPC_QUIT:
    case RPC_NETWORK_AVAILABLE:
    case RPC_PROJECT_ATTACH_FROM_FILE:
    case RPC_READ_GLOBAL_PREFS_OVERRIDE:
    case RPC_READ_CC_CONFIG:
        break;
    default:
        // All others must have at least one argument
        if (current_request->arg1 == NULL) {
            wxASSERT(false);
            return -1;
        }
        break;
    }
    switch (current_request->which_rpc) {
    case RPC_AUTHORIZE:
        retval = (m_Doc->rpcClient).authorize((const char*)(current_request->arg1));
        break;
    case RPC_EXCHANGE_VERSIONS:
        retval = (m_Doc->rpcClient).exchange_versions(*(VERSION_INFO*)(current_request->arg1));
        break;
    case RPC_GET_STATE:
        retval = (m_Doc->rpcClient).get_state(*(CC_STATE*)(current_request->arg1));
        break;
    case RPC_GET_RESULTS:
        retval = (m_Doc->rpcClient).get_results(*(RESULTS*)(current_request->arg1));
        break;
    case RPC_GET_FILE_TRANSFERS:
        retval = (m_Doc->rpcClient).get_file_transfers(*(FILE_TRANSFERS*)(current_request->arg1));
        break;
    case RPC_GET_SIMPLE_GUI_INFO1:
        retval = (m_Doc->rpcClient).get_simple_gui_info(*(SIMPLE_GUI_INFO*)(current_request->arg1));
        break;
    case RPC_GET_SIMPLE_GUI_INFO2:
        retval = (m_Doc->rpcClient).get_simple_gui_info(
            *(CC_STATE*)(current_request->arg1), 
            *(RESULTS*)(current_request->arg2)
        );
        break;
    case RPC_GET_PROJECT_STATUS1:
        if (current_request->exchangeBuf) {
            ((CC_STATE*)(current_request->arg1))->projects.clear();
            int n = (int)((CC_STATE*)(current_request->exchangeBuf))->projects.size();
            for (int i=0; i<n; i++) {
                PROJECT* p = new PROJECT();
                // get_project_status RPC needs master_url and will fill in everything else
                p->master_url = ((CC_STATE*)(current_request->exchangeBuf))->projects[i]->master_url;
                ((CC_STATE*)(current_request->arg1))->projects.push_back(p);
            }
        }
        retval = (m_Doc->rpcClient).get_project_status(*(CC_STATE*)(current_request->arg1));
        break;
    case RPC_GET_PROJECT_STATUS2:
        retval = (m_Doc->rpcClient).get_project_status(*(PROJECTS*)(current_request->arg1));
        break;
    case RPC_GET_ALL_PROJECTS_LIST:
        retval = (m_Doc->rpcClient).get_all_projects_list(*(ALL_PROJECTS_LIST*)(current_request->arg1));
        break;
    case RPC_GET_DISK_USAGE:
        retval = (m_Doc->rpcClient).get_disk_usage(*(DISK_USAGE*)(current_request->arg1));
        break;
    case RPC_SHOW_GRAPHICS:
        retval = (m_Doc->rpcClient).show_graphics(
            (const char*)(current_request->arg1), 
            (const char*)(current_request->arg2), 
            *(int*)(current_request->arg3), 
            *(DISPLAY_INFO*)(current_request->arg4)
        );
        break;
    case RPC_PROJECT_OP:
        retval = (m_Doc->rpcClient).project_op(
            *(PROJECT*)(current_request->arg1), 
            (const char*)(current_request->arg2)
        );
        break;
    case RPC_SET_RUN_MODE:
        retval = (m_Doc->rpcClient).set_run_mode(
            *(int*)(current_request->arg1), 
            *(double*)(current_request->arg2)
        );
        break;
    case RPC_SET_NETWORK_MODE:
        retval = (m_Doc->rpcClient).set_network_mode(
            *(int*)(current_request->arg1),
            *(double*)(current_request->arg2)
        );
        break;
    case RPC_GET_SCREENSAVER_TASKS:
        retval = (m_Doc->rpcClient).get_screensaver_tasks(
            *(int*)(current_request->arg1),
            *(RESULTS*)(current_request->arg2)
        );
        break;
    case RPC_RUN_BENCHMARKS:
        retval = (m_Doc->rpcClient).run_benchmarks();
        break;
    case RPC_SET_PROXY_SETTINGS:
        retval = (m_Doc->rpcClient).set_proxy_settings(*(GR_PROXY_INFO*)(current_request->arg1));
        break;
    case RPC_GET_PROXY_SETTINGS:
        retval = (m_Doc->rpcClient).get_proxy_settings(*(GR_PROXY_INFO*)(current_request->arg1));
        break;
    case RPC_GET_MESSAGES:
        retval = (m_Doc->rpcClient).get_messages(
            *(int*)(current_request->arg1), 
            *(MESSAGES*)(current_request->arg2)
        );
        break;
    case RPC_FILE_TRANSFER_OP:
        retval = (m_Doc->rpcClient).file_transfer_op(
            *(FILE_TRANSFER*)(current_request->arg1), 
            (const char*)(current_request->arg2)
        );
        break;
    case RPC_RESULT_OP:
        retval = (m_Doc->rpcClient).result_op(
            *(RESULT*)(current_request->arg1),
            (const char*)(current_request->arg2)
        );
        break;
    case RPC_GET_HOST_INFO:
        retval = (m_Doc->rpcClient).get_host_info(*(HOST_INFO*)(current_request->arg1));
        break;
    case RPC_QUIT:
        retval = (m_Doc->rpcClient).quit();
        break;
    case RPC_ACCT_MGR_INFO:
        retval = (m_Doc->rpcClient).acct_mgr_info(*(ACCT_MGR_INFO*)(current_request->arg1));
        break;
    case RPC_GET_STATISTICS:
        retval = (m_Doc->rpcClient).get_statistics(*(PROJECTS*)(current_request->arg1));
        break;
    case RPC_NETWORK_AVAILABLE:
        retval = (m_Doc->rpcClient).network_available();
        break;
    case RPC_GET_PROJECT_INIT_STATUS:
        retval = (m_Doc->rpcClient).get_project_init_status(*(PROJECT_INIT_STATUS*)(current_request->arg1));
        break;
    case RPC_GET_PROJECT_CONFIG:
        retval = (m_Doc->rpcClient).get_project_config(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_PROJECT_CONFIG_POLL:
        retval = (m_Doc->rpcClient).get_project_config_poll(*(PROJECT_CONFIG*)(current_request->arg1));
        break;
    case RPC_LOOKUP_ACCOUNT:
        retval = (m_Doc->rpcClient).lookup_account(*(ACCOUNT_IN*)(current_request->arg1));
        break;
    case RPC_LOOKUP_ACCOUNT_POLL:
        retval = (m_Doc->rpcClient).lookup_account_poll(*(ACCOUNT_OUT*)(current_request->arg1));
        break;
    case RPC_CREATE_ACCOUNT:
        retval = (m_Doc->rpcClient).create_account(*(ACCOUNT_IN*)(current_request->arg1));
        break;
    case RPC_CREATE_ACCOUNT_POLL:
        retval = (m_Doc->rpcClient).create_account_poll(*(ACCOUNT_OUT*)(current_request->arg1));
        break;
    case RPC_PROJECT_ATTACH:
        retval = (m_Doc->rpcClient).project_attach(
            (const char*)(current_request->arg1), 
            (const char*)(current_request->arg2), 
            (const char*)(current_request->arg3)
        );
        break;
    case RPC_PROJECT_ATTACH_FROM_FILE:
        retval = (m_Doc->rpcClient).project_attach_from_file();
        break;
    case RPC_PROJECT_ATTACH_POLL:
        retval = (m_Doc->rpcClient).project_attach_poll(*(PROJECT_ATTACH_REPLY*)(current_request->arg1));
        break;
    case RPC_ACCT_MGR_RPC:
        retval = (m_Doc->rpcClient).acct_mgr_rpc(
            (const char*)(current_request->arg1), 
            (const char*)(current_request->arg2), 
            (const char*)(current_request->arg3),
            (bool*)(current_request->arg4)
        );
        break;
    case RPC_ACCT_MGR_RPC_POLL:
        retval = (m_Doc->rpcClient).acct_mgr_rpc_poll(*(ACCT_MGR_RPC_REPLY*)(current_request->arg1));
        break;
    case RPC_GET_NEWER_VERSION:
        retval = (m_Doc->rpcClient).get_newer_version(*(std::string*)(current_request->arg1));
        break;
    case RPC_READ_GLOBAL_PREFS_OVERRIDE:
        retval = (m_Doc->rpcClient).read_global_prefs_override();
        break;
    case RPC_READ_CC_CONFIG:
        retval = (m_Doc->rpcClient).read_cc_config();
        break;
    case RPC_GET_CC_STATUS:
        retval = (m_Doc->rpcClient).get_cc_status(*(CC_STATUS*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_FILE:
        retval = (m_Doc->rpcClient).get_global_prefs_file(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_WORKING:
        retval = (m_Doc->rpcClient).get_global_prefs_working(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_WORKING_STRUCT:
        retval = (m_Doc->rpcClient).get_global_prefs_working_struct(
            *(GLOBAL_PREFS*)(current_request->arg1), 
            *(GLOBAL_PREFS_MASK*)(current_request->arg2)
        );
        break;
    case RPC_GET_GLOBAL_PREFS_OVERRIDE:
        retval = (m_Doc->rpcClient).get_global_prefs_override(*(std::string*)(current_request->arg1));
        break;
    case RPC_SET_GLOBAL_PREFS_OVERRIDE:
         retval = (m_Doc->rpcClient).set_global_prefs_override(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_OVERRIDE_STRUCT:
        retval = (m_Doc->rpcClient).get_global_prefs_override_struct(
            *(GLOBAL_PREFS*)(current_request->arg1), 
            *(GLOBAL_PREFS_MASK*)(current_request->arg2)
        );
        break;
    case RPC_SET_GLOBAL_PREFS_OVERRIDE_STRUCT:
        retval = (m_Doc->rpcClient).set_global_prefs_override_struct(
            *(GLOBAL_PREFS*)(current_request->arg1), 
            *(GLOBAL_PREFS_MASK*)(current_request->arg2)
        );
        break;
    case RPC_SET_DEBTS:
        retval = (m_Doc->rpcClient).set_debts(*(std::vector<PROJECT>*)(current_request->arg1));
        break;
    default:
        break;
    }

    // Deactivation is an atomic operation
    current_request->retval = retval;
    current_request->isActive = false;

    return retval;
}


// We don't need critical sections (except when exiting Manager) because:
// 1. CMainDocument never modifies mDoc->current_rpc_request while the 
// async RPC thread is using it.
// 2. The async RPC thread never modifies either mDoc->current_rpc_request 
// or the vector of requests mDoc->RPC_requests.

// TODO: combine RPC requests for different buffers, then just copy the buffer.

int CMainDocument::RequestRPC(ASYNC_RPC_REQUEST& request, bool hasPriority) {
    std::vector<ASYNC_RPC_REQUEST>::iterator iter;
    int retval = 0, retval2 = 0;
    
    // If we are quitting, cancel any pending RPCs
    if (request.which_rpc == RPC_QUIT) {
        RPC_requests.clear();
    }
    
    // Check if a duplicate request is already on the queue
    for (iter=RPC_requests.begin(); iter!=RPC_requests.end(); iter++) {
        if (iter->isSameAs(request)) {
            return 0;
        }
    }

    if ((request.event == 0) && (request.resultPtr == NULL)) {
        request.resultPtr = &retval;
    }
    
    if (hasPriority) {
        // We may want to set hasPriority for some user-initiated events. 
        // Since the user is waiting, insert this at head of request queue.
        // As of 8/14/08, hasPriority is never set true, so hasn't been tested.
        iter = RPC_requests.insert(RPC_requests.begin(), request);
    } else {
           RPC_requests.push_back(request);
    }
    
    // Start this RPC if no other RPC is already in progress.
    if (RPC_requests.size() == 1) {
        // Make sure activation is an atomic operation
        request.isActive = false;
        current_rpc_request = request;
        current_rpc_request.isActive = true;
    }
#ifndef __WXMSW__       // Deadlocks on Windows
    if (current_rpc_request.isActive && m_RPCThread->IsPaused()) {
        m_RPCThread->Resume();
    }
#endif //  !!__WXMSW__       // Deadlocks on Windows

    // If no completion event specified, this is a user-initiated event so 
    // wait for completion but show a dialog allowing the user to cancel.
    if (request.event == 0) {
    // TODO: proper handling if a second user request is received while first is pending ??
        if (m_bWaitingForRPC) {
            wxLogMessage(wxT("Second user RPC request while another was pending"));
            wxASSERT(false);
            return -1;
        }
        m_bWaitingForRPC = true;
        // Don't show dialog if RPC completes before RPC_WAIT_DLG_DELAY
        wxStopWatch Dlgdelay = wxStopWatch();        
        m_RPCWaitDlg = new AsyncRPCDlg();
        do {
        // Simulate handling of CRPCFinishedEvent but don't allow any other events (so no user activity)
        // Allow RPC thread to run while we wait for it
#ifdef __WXMSW__
            SwitchToThread();
#else
            // TODO: is there a way for main UNIX thread to yield wih no minimum delay?
            timespec ts = {0, 1};   /// 1 nanosecond
            nanosleep(&ts, NULL);   /// 1 nanosecond or less 
#endif
            if (!current_rpc_request.isActive) {
                HandleCompletedRPC();
#ifndef __WXMSW__       // Deadlocks on Windows
                } else {
                // for safety
                if (m_RPCThread->IsPaused()) {
                    m_RPCThread->Resume();
               }
#endif //  !!__WXMSW__       // Deadlocks on Windows
            }

            // OnRPCComplete() clears m_bWaitingForRPC if RPC completed 
            if (! m_bWaitingForRPC) {
                return retval;
            }
        } while (Dlgdelay.Time() < RPC_WAIT_DLG_DELAY);
//      GetCurrentProcess(&psn);    // Mac only
//      SetFrontProcess(&psn);  // Mac only: Shows process if hidden
        if (m_RPCWaitDlg) {
            if (m_RPCWaitDlg->ShowModal() != wxID_OK) {
                // TODO: If user presses Cancel in Please Wait dialog but request 
                // has not yet been started, should we just remove it from queue? 
                // If we make that change, should we also add a separate menu item  
                // to reset the RPC connection (or does one already exist)?

                retval = -1;
                // If the RPC continues to get data after we return to 
                // our caller, it may try to write into a buffer or struct
                // which the caller has already deleted.  To prevent this, 
                // we close the socket (disconnect) and kill the RPC thread.
                // This is ugly but necessary.  We must then reconnect and 
                // start a new RPC thread.
                if (current_rpc_request.isActive) {
                    current_rpc_request.isActive = false;
                    m_RPCThread->Pause();   // Needed on Windows
                    rpcClient.close();
                    m_RPCThread->Kill();
#ifdef __WXMSW__
                    m_RPCThread->Delete();  // Needed on Windows, crashes on Mac/Linux
#endif
                    m_RPCThread = NULL;
                    RPC_requests.clear();
                    current_rpc_request.clear();
                    // We will be reconnected to the same client (if possible) by 
                    // CBOINCDialUpManager::OnPoll() and CNetworkConnection::Poll().
                    m_pNetworkConnection->SetStateDisconnected();
                    m_RPCThread = new RPCThread(this);
                    wxASSERT(m_RPCThread);
                    retval2 = m_RPCThread->Create();
                    wxASSERT(!retval2);
                    retval2 = m_RPCThread->Run();
                    wxASSERT(!retval2);
//                    m_RPCThread->Pause();
                }
            }
            if (m_RPCWaitDlg) {
                m_RPCWaitDlg->Destroy();
            }
            m_RPCWaitDlg = NULL;
            m_bWaitingForRPC = false;
        }
    }
    return retval;
}


void CMainDocument::OnRPCComplete(CRPCFinishedEvent&) {
    HandleCompletedRPC();
}   
    
void CMainDocument::HandleCompletedRPC() {
    int retval;
    int i, n, requestIndex = -1;
    bool stillWaitingForPendingRequests = false;
    
    if(current_rpc_request.isActive) return;
    // We can get here either via a CRPCFinishedEvent event posted 
    // by the RPC thread or by a call from RequestRPC.  If we were 
    // called from RequestRPC, the CRPCFinishedEvent will still be 
    // on the event queue, so we get called twice.  Check for this here.
    if (current_rpc_request.which_rpc == 0) return; // already handled by a call from RequestRPC
//    m_RPCThread->Pause();

    // Find our completed request in the queue
    n = RPC_requests.size();
    for (i=0; i<n; ++i) {
        if (RPC_requests[i].isSameAs(current_rpc_request)) {
            requestIndex = i;
        } else {
            if (RPC_requests[i].event == 0) {
                stillWaitingForPendingRequests = true;
            }
        }
    }
    
    if (requestIndex >= 0) {
        // Remove completed request from the queue
        RPC_requests[requestIndex].event = NULL;  // Is this needed to prevent calling the event's destructor?
        RPC_requests.erase(RPC_requests.begin()+requestIndex);
    }
    
    retval = current_rpc_request.retval;

    if (current_rpc_request.completionTime) {
        *(current_rpc_request.completionTime) = wxDateTime::Now();
    }
    if (current_rpc_request.resultPtr) {
        *(current_rpc_request.resultPtr) = retval;
    }

    // Post-processing
    if (! retval) {
        switch (current_rpc_request.which_rpc) {
        case RPC_GET_STATE:
            if (current_rpc_request.exchangeBuf && !retval) {
                CC_STATE* arg1 = (CC_STATE*)current_rpc_request.arg1;
                CC_STATE* exchangeBuf = (CC_STATE*)current_rpc_request.exchangeBuf;
                arg1->projects.swap(exchangeBuf->projects);
                arg1->apps.swap(exchangeBuf->apps);
                arg1->app_versions.swap(exchangeBuf->app_versions);
                arg1->wus.swap(exchangeBuf->wus);
                arg1->results.swap(exchangeBuf->results);
                exchangeBuf->global_prefs = arg1->global_prefs;
                exchangeBuf->version_info = arg1->version_info;
                exchangeBuf->executing_as_daemon = arg1->executing_as_daemon;
            }
            break;
        case RPC_GET_RESULTS:
            if (current_rpc_request.exchangeBuf && !retval) {
                RESULTS* arg1 = (RESULTS*)current_rpc_request.arg1;
                RESULTS* exchangeBuf = (RESULTS*)current_rpc_request.exchangeBuf;
                arg1->results.swap(exchangeBuf->results);
            }
            break;
        case RPC_GET_FILE_TRANSFERS:
            if (current_rpc_request.exchangeBuf && !retval) {
                FILE_TRANSFERS* arg1 = (FILE_TRANSFERS*)current_rpc_request.arg1;
                FILE_TRANSFERS* exchangeBuf = (FILE_TRANSFERS*)current_rpc_request.exchangeBuf;
                arg1->file_transfers.swap(exchangeBuf->file_transfers);
            }
            break;
        case RPC_GET_SIMPLE_GUI_INFO2:
            if (current_rpc_request.exchangeBuf && !retval) {
                CC_STATE* arg1 = (CC_STATE*)current_rpc_request.arg1;
                CC_STATE* exchangeBuf = (CC_STATE*)current_rpc_request.exchangeBuf;
                arg1->projects.swap(exchangeBuf->projects);
            }
            if (current_rpc_request.arg3) {
                RESULTS* arg2 = (RESULTS*)current_rpc_request.arg2;
                RESULTS* arg3 = (RESULTS*)current_rpc_request.arg3;
                arg2->results.swap(arg3->results);
            }
            break;
        case RPC_GET_PROJECT_STATUS1:
            if (current_rpc_request.exchangeBuf && !retval) {
                CC_STATE* arg1 = (CC_STATE*)current_rpc_request.arg1;
                CC_STATE* exchangeBuf = (CC_STATE*)current_rpc_request.exchangeBuf;
                arg1->projects.swap(exchangeBuf->projects);
            }
            break;
        case RPC_GET_ALL_PROJECTS_LIST:
            if (current_rpc_request.exchangeBuf && !retval) {
                ALL_PROJECTS_LIST* arg1 = (ALL_PROJECTS_LIST*)current_rpc_request.arg1;
                ALL_PROJECTS_LIST* exchangeBuf = (ALL_PROJECTS_LIST*)current_rpc_request.exchangeBuf;
                arg1->projects.swap(exchangeBuf->projects);
            }
            break;
        case RPC_GET_DISK_USAGE:
            if (current_rpc_request.exchangeBuf && !retval) {
                DISK_USAGE* arg1 = (DISK_USAGE*)current_rpc_request.arg1;
                DISK_USAGE* exchangeBuf = (DISK_USAGE*)current_rpc_request.exchangeBuf;
                arg1->projects.swap(exchangeBuf->projects);
                exchangeBuf->d_total = arg1->d_total;
                exchangeBuf->d_free = arg1->d_free;
                exchangeBuf->d_boinc = arg1->d_boinc;
                exchangeBuf->d_allowed = arg1->d_allowed;
            }
            break;
        case RPC_GET_MESSAGES:
            if (current_rpc_request.exchangeBuf && !retval) {
                MESSAGES* arg2 = (MESSAGES*)current_rpc_request.arg2;
                MESSAGES* exchangeBuf = (MESSAGES*)current_rpc_request.exchangeBuf;
                arg2->messages.swap(exchangeBuf->messages);
            }
            break;
        case RPC_GET_HOST_INFO:
            if (current_rpc_request.exchangeBuf && !retval) {
                HOST_INFO* arg1 = (HOST_INFO*)current_rpc_request.arg1;
                HOST_INFO* exchangeBuf = (HOST_INFO*)current_rpc_request.exchangeBuf;
                *exchangeBuf = *arg1;
            }
            break;
        case RPC_GET_STATISTICS:
            if (current_rpc_request.exchangeBuf && !retval) {
                PROJECTS* arg1 = (PROJECTS*)current_rpc_request.arg1;
                PROJECTS* exchangeBuf = (PROJECTS*)current_rpc_request.exchangeBuf;
                arg1->projects.swap(exchangeBuf->projects);
            }
            break;
            
        case RPC_GET_CC_STATUS:
            if (current_rpc_request.exchangeBuf && !retval) {
                CC_STATUS* arg1 = (CC_STATUS*)current_rpc_request.arg1;
                CC_STATUS* exchangeBuf = (CC_STATUS*)current_rpc_request.exchangeBuf;
                *exchangeBuf = *arg1;
            }
            break;
        case RPC_ACCT_MGR_INFO:
            if (current_rpc_request.exchangeBuf && !retval) {
                ACCT_MGR_INFO* arg1 = (ACCT_MGR_INFO*)current_rpc_request.arg1;
                ACCT_MGR_INFO* exchangeBuf = (ACCT_MGR_INFO*)current_rpc_request.exchangeBuf;
                *exchangeBuf = *arg1;
           }
            break;
        default:
            // We don't support double buffering for other RPC calls 
            wxASSERT(current_rpc_request.exchangeBuf == NULL);
            break;
        }
    }
    
    if ( (current_rpc_request.event) && (current_rpc_request.event != (wxEvent*)-1) ) {
        if (! retval) {
            if (current_rpc_request.eventHandler) {
                current_rpc_request.eventHandler->AddPendingEvent(*current_rpc_request.event);
            } else {
                // We must get the frame immediately before using it, 
                // since it may have been changed by SetActiveGUI().
                CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                if (pFrame) {
                    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
                    pFrame->AddPendingEvent(*current_rpc_request.event);
                }
            }
        }
        delete current_rpc_request.event;
        current_rpc_request.event = NULL;
    }

    current_rpc_request.clear();

    // Start the next RPC request.
    if (RPC_requests.size() > 0) {
        // Make sure activation is an atomic operation
        RPC_requests[0].isActive = false;
        current_rpc_request = RPC_requests[0];
        current_rpc_request.isActive = true;
#ifndef __WXMSW__       // Deadlocks on Windows
        if (m_RPCThread->IsPaused()) {
            m_RPCThread->Resume();
        }
    } else {
        m_RPCThread->Pause();
        while (!m_RPCThread->IsPaused()) {
#ifdef __WXMSW__
            SwitchToThread();
#else
            // TODO: is there a way for main UNIX thread to yield wih no minimum delay?
            timespec ts = {0, 1};   /// 1 nanosecond
            nanosleep(&ts, NULL);   /// 1 nanosecond or less 
#endif
        }
#endif  // ! __WXMSW__       // Deadlocks on Windows
    }


    if (! stillWaitingForPendingRequests) {
        if (m_RPCWaitDlg) {
            if (m_RPCWaitDlg->IsShown()) {
                m_RPCWaitDlg->EndModal(wxID_OK);
            }
                m_RPCWaitDlg->Destroy();
                m_RPCWaitDlg = NULL;
        }
        m_bWaitingForRPC = false;
    }
}


IMPLEMENT_CLASS(AsyncRPCDlg, wxDialog)

AsyncRPCDlg::AsyncRPCDlg() : wxDialog( NULL, wxID_ANY, wxT(""), wxDefaultPosition ) {

    wxString message = wxString(_("Communicating with BOINC client.  Please wait ..."));

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );

    icon_text->Add( CreateTextSizer( message ), 0, wxALIGN_CENTER | wxLEFT, 10 );
    topsizer->Add( icon_text, 1, wxCENTER | wxLEFT|wxRIGHT|wxTOP, 10 );
    
    int center_flag = wxEXPAND;
    wxSizer *sizerBtn = CreateStdDialogButtonSizer(wxCANCEL|wxNO_DEFAULT);
    if ( sizerBtn )
        topsizer->Add(sizerBtn, 0, center_flag | wxALL, 10 );

    SetAutoLayout( true );
    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );
    wxSize size( GetSize() );
    if (size.x < size.y*3/2)
    {
        size.x = size.y*3/2;
        SetSize( size );
    }

    Centre( wxBOTH | wxCENTER_FRAME);
}


#if 0

/// For testing: triggered by Advanced / Options menu item.
void CMainDocument::TestAsyncRPC() {
    ALL_PROJECTS_LIST pl;
    ASYNC_RPC_REQUEST request;
    wxDateTime completionTime = wxDateTime((time_t)0);
    int req_retval = 0, rpc_result = 0;

    completionTime.ResetTime();

    request.which_rpc = RPC_GET_ALL_PROJECTS_LIST;
    request.arg1 = &pl;
    request.exchangeBuf = NULL;
    request.arg2 = NULL;
    request.arg3 = NULL;
    request.arg4 = NULL;
    request.event = NULL;
    request.eventHandler = NULL;
    request.completionTime = &completionTime;
//    request.result = NULL;
    request.resultPtr = &rpc_result;        // For testing async RPCs
    request.isActive = false;
    
//retval = rpcClient.get_all_projects_list(pl);

    req_retval = RequestRPC(request, true);

    wxString s = completionTime.FormatTime();
    wxLogMessage(wxT("Completion time = %s"), s.c_str());
    wxLogMessage(wxT("RequestRPC returned %d\n"), req_retval);
    ::wxSafeYield(NULL, true);  // Allow processing of RPC_FINISHED event
    wxLogMessage(wxT("rpcClient.get_all_projects_list returned %d\n"), rpc_result);
}

#endif
