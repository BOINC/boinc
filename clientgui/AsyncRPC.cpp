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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AsyncRPC.h"
#endif

#include <vector>

#include "stdwx.h"
//#include "wx/artprov.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "AsyncRPC.h"
//#include "BOINCClientManager.h"

// Delay in milliseconds before showing AsyncRPCDlg
#define RPC_WAIT_DLG_DELAY 250

ASYNC_RPC_REQUEST::ASYNC_RPC_REQUEST() {
    clear();
}


ASYNC_RPC_REQUEST::~ASYNC_RPC_REQUEST() {
    clear();
}


void ASYNC_RPC_REQUEST::clear() {
    which_rpc = (RPC_SELECTOR) 0;
    inBuf = NULL;
    exchangeBuf = NULL;
    outBuf = NULL;
    event = NULL;
    eventHandler = NULL;
    completionTime = NULL;
    isActive = false;
}


bool ASYNC_RPC_REQUEST::isSameAs(ASYNC_RPC_REQUEST& otherRequest) {
    if (which_rpc != otherRequest.which_rpc) return false;
    if (inBuf != otherRequest.inBuf) return false;
    if (exchangeBuf != otherRequest.exchangeBuf) return false;
    if (outBuf != otherRequest.outBuf) return false;
    if (event != otherRequest.event) {
        if (event->GetEventType() != (otherRequest.event)->GetEventType()) return false;
        if (event->GetId() != (otherRequest.event)->GetId()) return false;
        if (event->GetEventObject() != (otherRequest.event)->GetEventObject()) return false;
    }
    if (eventHandler != otherRequest.eventHandler) return false;
    if (completionTime != otherRequest.completionTime) return false;
    // OK if isActive doesn't match.
    return true;
}


RPCThread::RPCThread(CMainDocument *pDoc)
        : wxThread() {
    m_Doc = pDoc;
}


void RPCThread::OnExit() {
}


// We don't need critical sections because:
// 1. CMainDocument never modifies mDoc->current_rpc_request while the 
// async RPC thread is using it.
// 2. The async RPC thread never modifies either mDoc->current_rpc_request 
// or the vector of requests mDoc->RPC_requests.

void *RPCThread::Entry() {
    int retval;

    while(true) {
        // check if we were asked to exit
        if ( TestDestroy() )
            break;

        if (! m_Doc->GetCurrentRPCRequest()->isActive) {
            // Wait until CMainDocument issues next RPC request
            Yield();
            continue;
        }

       if (! m_Doc->IsConnected()) {
            Yield();
            continue;
        }

        retval = ProcessRPCRequest();
 
// TODO: Do we need a critical section here and in CBOINCGUIApp::SetActiveGUI()?
        // We need to get the frame each time, in case it was 
        // changed by a call to CBOINCGUIApp::SetActiveGUI().
        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
        wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

        CRPCFinishedEvent RPC_done_event( wxEVT_RPC_FINISHED, pFrame );
        RPC_done_event.SetInt(retval);

        wxPostEvent( pFrame, RPC_done_event );
    }

    return NULL;
}


int RPCThread::ProcessRPCRequest() {
    int                     retval = 0;
    ASYNC_RPC_REQUEST       *current_request;
    
    current_request = m_Doc->GetCurrentRPCRequest();
//Sleep(5000);     // TEMPORARY FOR TESTING ASYNC RPCs -- CAF

    switch (current_request->which_rpc) {
    case RPC_GET_STATE:
        if (current_request->inBuf == NULL) return -1;
        retval = (m_Doc->rpc).get_state((CC_STATE&)*(CC_STATE*)(current_request->inBuf));
        break;
    
    case RPC_GET_RESULTS:
        if (current_request->inBuf == NULL) return -1;
        // TODO: Confirm if the following is correct
        retval = (m_Doc->rpc).get_results((RESULTS&)*(RESULTS*)(current_request->inBuf));
        break;
    case RPC_GET_ALL_PROJECTS_LIST:
        if (current_request->inBuf == NULL) return -1;
        retval = (m_Doc->rpc).get_all_projects_list((ALL_PROJECTS_LIST&)*(ALL_PROJECTS_LIST*)(current_request->inBuf));
        break;
    default:
        break;
    }

#if USE_CRITICAL_SECTIONS_FOR_ASYNC_RPCS
    m_Doc->m_critsect.Enter();
    current_request->isActive = false;
    m_Doc->m_critsect.Leave();
#else
    // Deactivation is an atomic operation
    current_request->isActive = false;
#endif

    return retval;
}


// We don't need critical sections because:
// 1. CMainDocument never modifies mDoc->current_rpc_request while the 
// async RPC thread is using it.
// 2. The async RPC thread never modifies either mDoc->current_rpc_request 
// or the vector of requests mDoc->RPC_requests.

// TODO: combine RPC requests for different buffers, then just copy the buffer.

int CMainDocument::RequestRPC(ASYNC_RPC_REQUEST& request) {
    static bool inUserRequest = false;
    std::vector<ASYNC_RPC_REQUEST>::iterator iter;
    int retval = 0, retval2 = 0;
    
    // Check if a duplicate request is already on the queue
    for (iter=RPC_requests.begin(); iter!=RPC_requests.end(); iter++) {
        if (iter->isSameAs(request)) return 0;
    }

    if (request.event == NULL) {
        // If no completion event specified, this is a user-initiated event. 
        // Since the user is waiting, insert this at head of request queue
        iter = RPC_requests.insert(RPC_requests.begin(), request);
    } else {
           RPC_requests.push_back(request);
    }
    
    // Start this RPC if no other RPC is already in progress.
    if (RPC_requests.size() == 1) {
#if USE_CRITICAL_SECTIONS_FOR_ASYNC_RPCS
        m_critsect.Enter();
        current_rpc_request = request;
        current_rpc_request.isActive = true;
        m_critsect.Leave();
#else
        // Make sure activation is an atomic operation
        request.isActive = false;
        current_rpc_request = request;
        current_rpc_request.isActive = true;
#endif
    }

    // If no completion event specified, this is a user-initiated event so 
    // wait for completion but show a dialog allowing the user to cancel.
    if ((request.event == NULL) && !inUserRequest) {
        inUserRequest = true;
        // Don't show dialog if RPC completes before RPC_WAIT_DLG_DELAY
        wxStopWatch Dlgdelay = wxStopWatch();
        m_RPCWaitDlg = new AsyncRPCDlg();
        do {
            // OnRPCComplete() sets m_RPCWaitDlg to NULL if RPC completed 
            if (! m_RPCWaitDlg) {
                inUserRequest = false;
                return retval;
            }
            ::wxSafeYield(NULL, true);  // Continue processing events
        } while (Dlgdelay.Time() < RPC_WAIT_DLG_DELAY);
//      GetCurrentProcess(&psn);
//      SetFrontProcess(&psn);  // Shows process if hidden
        if (m_RPCWaitDlg) {
            if (m_RPCWaitDlg->ShowModal() != wxID_OK) {
                retval = -1;
                // If the RPC continues to get data after we return to 
                // our caller, it may try to write into a buffer or struct
                // which the caller has already deleted.  To prevent this, 
                // we close the socket (disconnect) and kill the RPC thread.
                // This is ugly but necessary.  We must then reconnect and 
                // start a new RPC thread.
                if (current_rpc_request.isActive) {
                    current_rpc_request.isActive = false;
                    rpc.close();
                    m_RPCThread->Kill();
                    m_RPCThread = NULL;
                    RPC_requests.clear();
                    current_rpc_request.clear();
                    // We will be reconnected to the same client (if possible) by 
                    // CBOINCDialUpManager::OnPoll() and CNetworkConnection::Poll().
//                wxString strComputer = wxEmptyString;
//                GetConnectedComputerName(strComputer);
//                retval2 = rpc.init(strComputer.mb_str(), m_pNetworkConnection->GetGUI_RPC_Port());
                
                    m_RPCThread = new RPCThread(this);
                    wxASSERT(m_RPCThread);
                    retval2 = m_RPCThread->Create();
                    wxASSERT(retval2);
                    m_RPCThread->Run();
                }
            }
            if (m_RPCWaitDlg) {
                m_RPCWaitDlg->Destroy();
            }
            m_RPCWaitDlg = NULL;
        }
        inUserRequest = false;
        }
    
    return retval;
}


void CMainDocument::OnRPCComplete(CRPCFinishedEvent& event) {
    int retval = event.GetInt();
    int i, n;
    std::vector<ASYNC_RPC_REQUEST> completed_RPC_requests;

    // Move all requests for the completed RPC to our local vector
    // We do this in reverse order so we can remove them from queue
    n = RPC_requests.size();
    for (i=n-1; i>=0; --i) {
        if (RPC_requests[i].which_rpc == current_rpc_request.which_rpc) {
            completed_RPC_requests.push_back(RPC_requests[i]);
            RPC_requests[i].event = NULL;  // Is this needed to prevent calling event's destructor?
            RPC_requests.erase(RPC_requests.begin()+i);

        }
    }

    current_rpc_request.clear();

    // Start the next RPC request while we process the one just completed.
    if (RPC_requests.size() > 0) {
#if USE_CRITICAL_SECTIONS_FOR_ASYNC_RPCS
        m_critsect.Enter();
        current_rpc_request = RPC_requests[0];
        current_rpc_request.isActive = true;
        m_critsect.Leave();
#else
        // Make sure activation is an atomic operation
        RPC_requests[0].isActive = false;
        current_rpc_request = RPC_requests[0];
        current_rpc_request.isActive = true;
#endif
    }

    // Now process the requests we have satisfied.
    n = completed_RPC_requests.size();
    for (i=n-1; i>=0; --i) {
        if (completed_RPC_requests[i].completionTime) {
            *(completed_RPC_requests[i].completionTime) = wxDateTime::Now();
        }
    
        switch (completed_RPC_requests[i].which_rpc) {
        case RPC_GET_STATE:
            m_iGet_state_RPC_retval = retval;
#if 0
            if (completed_RPC_requests[i].exchangeBuf) {
                CC_STATE* inBuf = (CC_STATE*)completed_RPC_requests[i].inBuf;
                CC_STATE* exchangeBuf = (CC_STATE*)completed_RPC_requests[i].exchangeBuf;
                inBuf->results.swap(exchangeBuf->results);
            }
#endif
            break;
        case RPC_GET_RESULTS:
            m_iGet_results_RPC_retval = retval;
            if (completed_RPC_requests[i].exchangeBuf) {
                RESULTS* inBuf = (RESULTS*)completed_RPC_requests[i].inBuf;
                RESULTS* exchangeBuf = (RESULTS*)completed_RPC_requests[i].exchangeBuf;
                inBuf->results.swap(exchangeBuf->results);
            }
            break;
        case RPC_GET_ALL_PROJECTS_LIST:
//            m_iGet_all_projects_list_RPC_retval = retval;
            if (completed_RPC_requests[i].exchangeBuf) {
                ALL_PROJECTS_LIST* inBuf = (ALL_PROJECTS_LIST*)completed_RPC_requests[i].inBuf;
                ALL_PROJECTS_LIST* exchangeBuf = (ALL_PROJECTS_LIST*)completed_RPC_requests[i].exchangeBuf;
                inBuf->projects.swap(exchangeBuf->projects);
            }
            break;
        default:
            break;
        }

        if (completed_RPC_requests[i].event) {
            if (completed_RPC_requests[i].eventHandler) {
                completed_RPC_requests[i].eventHandler->ProcessEvent(*completed_RPC_requests[i].event);
            } else {
                CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
                pFrame->ProcessEvent(*completed_RPC_requests[i].event);
            }
            delete completed_RPC_requests[i].event;
            completed_RPC_requests[i].event = NULL;
        } else {

            if (m_RPCWaitDlg) {
                if (m_RPCWaitDlg->IsShown()) {
                    m_RPCWaitDlg->EndModal(wxID_OK);
                } else {
                    m_RPCWaitDlg->Destroy();
                    m_RPCWaitDlg = NULL;
                }
            }
        }
    }
}


IMPLEMENT_CLASS(AsyncRPCDlg, wxDialog)

AsyncRPCDlg::AsyncRPCDlg() : wxDialog( NULL, wxID_ANY, wxT(""), wxDefaultPosition ) {

    wxString message = wxString(_("Communicating with BOINC client.  Please wait ..."));

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );

//    wxBitmap bitmap = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_MESSAGE_BOX);
//    wxStaticBitmap *icon = new wxStaticBitmap(this, wxID_ANY, bitmap);
//    icon_text->Add( icon, 0, wxCENTER );

    icon_text->Add( CreateTextSizer( message ), 0, wxALIGN_CENTER | wxLEFT, 10 );
    topsizer->Add( icon_text, 1, wxCENTER | wxLEFT|wxRIGHT|wxTOP, 10 );
    
    int center_flag = wxEXPAND;
    wxSizer *sizerBtn = CreateSeparatedButtonSizer(wxCANCEL|wxNO_DEFAULT);
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
    
#if USE_RPC_DLG_TIMER
    m_pDlgDelayTimer = new wxTimer(this, wxID_ANY);
    wxASSERT(m_pDlgDelayTimer);

    m_pDlgDelayTimer->Start(100, false); 
#endif  // USE_RPC_DLG_TIMER
}


#if USE_RPC_DLG_TIMER
BEGIN_EVENT_TABLE(AsyncRPCDlg, wxMessageDialog)
    EVT_TIMER(wxID_ANY, AsyncRPCDlg::OnRPCDlgTimer)
END_EVENT_TABLE()

AsyncRPCDlg::~AsyncRPCDlg() {
    if (m_pDlgDelayTimer) {
        m_pDlgDelayTimer->Stop();
        delete m_pDlgDelayTimer;
        m_pDlgDelayTimer = NULL;
    }
}


void AsyncRPCDlg::OnRPCDlgTimer(wxTimerEvent& WXUNUSED(event)) {
    ::wxWakeUpIdle();
}
#endif  // USE_RPC_DLG_TIMER

/// For testing: triggered by Advanced / Options menu item.
ALL_PROJECTS_LIST pl;

void CMainDocument::TestAsyncRPC() {        // TEMPORARY FOR TESTING ASYNC RPCs -- CAF
    ASYNC_RPC_REQUEST request;
    wxDateTime completionTime;
    int retval = 0;

    completionTime.ResetTime();

    request.which_rpc = RPC_GET_ALL_PROJECTS_LIST;
    request.inBuf = &pl;
    request.exchangeBuf = NULL;
    request.outBuf = NULL;
    request.event = NULL;
    request.eventHandler = NULL;
    request.completionTime = &completionTime;
    request.isActive = false;
    
//retval = rpc.get_all_projects_list(pl);

    retval = RequestRPC(request);

    wxString s = completionTime.Format("%T");
    printf("Completion time = %s\n", s.c_str());
    printf("RequestRPC returned %d\n", retval);
}

