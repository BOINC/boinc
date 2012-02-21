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

#if !(defined(_WIN32) || (defined(__WXMAC__) && (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4)))
#include <xlocale.h>
//#include "gui_rpc_client.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "AsyncRPC.h"
#include "BOINCBaseFrame.h"
#include "BOINCTaskBar.h"
#include "error_numbers.h"
#include "SkinManager.h"
#include "DlgEventLog.h"
#include "util.h"

extern bool s_bSkipExitConfirmation;

#ifdef __WXMAC__

#ifdef HAVE_PTHREAD_MUTEXATTR_T
// on some systems pthread_mutexattr_settype() is not in the headers (but it is
// in the library, otherwise we wouldn't compile this code at all)
extern "C" int pthread_mutexattr_settype( pthread_mutexattr_t *, int );
#endif

BOINC_Mutex::BOINC_Mutex( wxMutexType mutexType )
{
    int err;
    switch ( mutexType )
    {
        case wxMUTEX_RECURSIVE:
            // support recursive locks like Win32, i.e. a thread can lock a
            // mutex which it had itself already locked
            //
            // unfortunately initialization of recursive mutexes is non
            // portable, so try several methods
#ifdef HAVE_PTHREAD_MUTEXATTR_T
            {
                pthread_mutexattr_t attr;
                pthread_mutexattr_init( &attr );
                pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

                err = pthread_mutex_init( &m_mutex, &attr );
            }
#elif defined(HAVE_PTHREAD_RECURSIVE_MUTEX_INITIALIZER)
            // we can use this only as initializer so we have to assign it
            // first to a temp var - assigning directly to m_mutex wouldn't
            // even compile
            {
                pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
                m_mutex = mutex;
            }
#else // no recursive mutexes
            err = EINVAL;
#endif // HAVE_PTHREAD_MUTEXATTR_T/...
            break;

        default:
            wxFAIL_MSG( wxT("unknown mutex type") );
            // fall through

        case wxMUTEX_DEFAULT:
            err = pthread_mutex_init( &m_mutex, NULL );
            break;
    }

    m_isOk = err == 0;
    if ( !m_isOk )
    {
        wxLogApiError( wxT("pthread_mutex_init()"), err );
    }
}

BOINC_Mutex::~BOINC_Mutex()
{
    if ( m_isOk )
    {
        int err = pthread_mutex_destroy( &m_mutex );
        if ( err != 0 )
        {
            wxLogApiError( wxT("pthread_mutex_destroy()"), err );
        }
    }
}

wxMutexError BOINC_Mutex::Lock()
{
    int err = pthread_mutex_lock( &m_mutex );
    switch ( err )
    {
        case EDEADLK:
            // only error checking mutexes return this value and so it's an
            // unexpected situation -- hence use assert, not wxLogDebug
            wxFAIL_MSG( wxT("mutex deadlock prevented") );
            return wxMUTEX_DEAD_LOCK;

        case EINVAL:
            wxLogDebug( wxT("pthread_mutex_lock(): mutex not initialized.") );
            break;

        case 0:
            return wxMUTEX_NO_ERROR;

        default:
            wxLogApiError( wxT("pthread_mutex_lock()"), err );
    }

    return wxMUTEX_MISC_ERROR;
}

wxMutexError BOINC_Mutex::TryLock()
{
    int err = pthread_mutex_trylock( &m_mutex );
    switch ( err )
    {
        case EBUSY:
            // not an error: mutex is already locked, but we're prepared for this case
            return wxMUTEX_BUSY;

        case EINVAL:
            wxLogDebug( wxT("pthread_mutex_trylock(): mutex not initialized.") );
            break;

        case 0:
            return wxMUTEX_NO_ERROR;

        default:
            wxLogApiError( wxT("pthread_mutex_trylock()"), err );
    }

    return wxMUTEX_MISC_ERROR;
}

wxMutexError BOINC_Mutex::Unlock()
{
    int err = pthread_mutex_unlock( &m_mutex );
    switch ( err )
    {
        case EPERM:
            // we don't own the mutex
            return wxMUTEX_UNLOCKED;

        case EINVAL:
            wxLogDebug( wxT("pthread_mutex_unlock(): mutex not initialized.") );
            break;

        case 0:
            return wxMUTEX_NO_ERROR;

        default:
            wxLogApiError( wxT("pthread_mutex_unlock()"), err );
    }

    return wxMUTEX_MISC_ERROR;
}


// wxMac wxCondition has bugs, so use native UNIX implementation

BOINC_Condition::BOINC_Condition(BOINC_Mutex& mutex) 
        : m_BOINC_Mutex(mutex) {
    int err;
    
    err = pthread_cond_init(&m_cond, NULL);
    mb_initOK = (err == 0);
}
    
BOINC_Condition::~BOINC_Condition() {
    pthread_cond_destroy(&m_cond);
    mb_initOK = false; 
}

wxCondError BOINC_Condition::Wait(){
    int err;
    
    err = pthread_cond_wait(&m_cond, &m_BOINC_Mutex.m_mutex);
    switch (err) {
    case 0:
        return wxCOND_NO_ERROR;
    case EINVAL:
        return wxCOND_INVALID;
    case ETIMEDOUT:
        return wxCOND_TIMEOUT;
    default:
        return wxCOND_MISC_ERROR;
    }
    return wxCOND_NO_ERROR;
}

wxCondError BOINC_Condition::WaitTimeout(unsigned long milliseconds) {
    int err;
    wxLongLong curtime = wxGetLocalTimeMillis();
    curtime += milliseconds;
    wxLongLong temp = curtime / 1000;
    int sec = temp.GetLo();
    temp *= 1000;
    temp = curtime - temp;
    int millis = temp.GetLo();

    timespec tspec;

    tspec.tv_sec = sec;
    tspec.tv_nsec = millis * 1000L * 1000L;
    
    err = pthread_cond_timedwait(&m_cond, &m_BOINC_Mutex.m_mutex, &tspec);
    switch (err) {
    case 0:
        return wxCOND_NO_ERROR;
    case EINVAL:
        return wxCOND_INVALID;
    case ETIMEDOUT:
        return wxCOND_TIMEOUT;
    default:
        return wxCOND_MISC_ERROR;
    }
    return wxCOND_NO_ERROR;
}

void BOINC_Condition::Signal() {
    pthread_cond_signal(&m_cond);
}

void BOINC_Condition::Broadcast() {
    pthread_cond_broadcast(&m_cond);
}

#endif      // __WXMAC__

// Delay in milliseconds before showing AsyncRPCDlg
#define RPC_WAIT_DLG_DELAY 1500
// How often to check for events when minimized and waiting for Demand RPC
#define DELAY_WHEN_MINIMIZED 500
// Delay in milliseconds to allow thread to exit before killing it
#define RPC_KILL_DELAY 2000

ASYNC_RPC_REQUEST::ASYNC_RPC_REQUEST() {
    clear();
}


ASYNC_RPC_REQUEST::~ASYNC_RPC_REQUEST() {
    clear();
}


void ASYNC_RPC_REQUEST::clear() {
    rpcType = (ASYNC_RPC_TYPE) 0;
    which_rpc = (RPC_SELECTOR) 0;
    exchangeBuf = NULL;
    arg1 = NULL;
    arg2 = NULL;
    arg3 = NULL;
    arg4 = NULL;
    completionTime = NULL;
    RPCExecutionTime = NULL;
    resultPtr = NULL;
    retval = 0;
    isActive = false;
}


bool ASYNC_RPC_REQUEST::isSameAs(ASYNC_RPC_REQUEST& otherRequest) {
    if (which_rpc != otherRequest.which_rpc) return false;
    if (arg1 != otherRequest.arg1) return false;
    if (exchangeBuf != otherRequest.exchangeBuf) return false;
    if (arg2 != otherRequest.arg2) return false;
    if (arg3 != otherRequest.arg3) return false;
    if (arg4 != otherRequest.arg4) return false;
    if (rpcType != otherRequest.rpcType) return false;
    if (completionTime != otherRequest.completionTime) return false;
    if (resultPtr != otherRequest.resultPtr) return false;
    // OK if isActive and retval don't match.
    return true;
}


AsyncRPC::AsyncRPC(CMainDocument *pDoc) {
    m_pDoc = pDoc;
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
    if (which_rpc == RPC_QUIT) {
        request.rpcType = RPC_TYPE_ASYNC_NO_REFRESH;
    } else {
        request.rpcType = RPC_TYPE_WAIT_FOR_COMPLETION;
    }
    request.RPCExecutionTime = NULL;
    retval = m_pDoc->RequestRPC(request, hasPriority);
    return retval;
}


RPCThread::RPCThread(CMainDocument *pDoc, 
                        BOINC_Mutex* pRPC_Thread_Mutex, 
                        BOINC_Condition* pRPC_Thread_Condition, 
                        BOINC_Mutex* pRPC_Request_Mutex, 
                        BOINC_Condition* pRPC_Request_Condition)
                        : wxThread() {
    m_pDoc = pDoc;
    m_pRPC_Thread_Mutex = pRPC_Thread_Mutex;
    m_pRPC_Thread_Condition = pRPC_Thread_Condition;
    m_pRPC_Request_Mutex = pRPC_Request_Mutex;
    m_pRPC_Request_Condition = pRPC_Request_Condition;
}

void *RPCThread::Entry() {
    int retval = 0;
    CRPCFinishedEvent RPC_done_event( wxEVT_RPC_FINISHED );
    ASYNC_RPC_REQUEST *current_request;
    double startTime = 0;
    wxMutexError mutexErr = wxMUTEX_NO_ERROR;
    wxCondError condErr = wxCOND_NO_ERROR;

#ifndef NO_PER_THREAD_LOCALE
#ifdef __WXMSW__
    // On Windows, set all locales for this thread on a per-thread basis
    _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
    setlocale(LC_ALL, "C");
#else
    // We initialize RPC_Thread_Locale to fix a compiler warning
    locale_t RPC_Thread_Locale = LC_GLOBAL_LOCALE;
#if defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4)
    if (uselocale)    // uselocale() is not available in Mac OS 10.3.9
#endif
    {
        // On Mac / Unix / Linux, set "C" locale for this thread only
        RPC_Thread_Locale = newlocale(LC_ALL_MASK, "C", NULL);
        uselocale(RPC_Thread_Locale);
    }
#endif      // ifndef __WXMSW__
#endif      // ifndef NO_PER_THREAD_LOCALE
   
    m_pRPC_Thread_Mutex->Lock();
    m_pDoc->m_bRPCThreadIsReady = true;
    while(true) {
        // Wait for main thread to wake us
        // This does the following:
        // (1) Unlocks the Mutex and puts the RPC thread to sleep as an atomic operation.
        // (2) On Signal from main thread: locks Mutex again and wakes the RPC thread.
        condErr = m_pRPC_Thread_Condition->Wait();
        wxASSERT(condErr == wxCOND_NO_ERROR);
        
        if (m_pDoc->m_bShutDownRPCThread) {
#if !defined(NO_PER_THREAD_LOCALE) && !defined(__WXMSW__)
#if defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4)
        if (uselocale)    // uselocale() is not available in Mac OS 10.3.9
#endif
        {
            uselocale(LC_GLOBAL_LOCALE);
            freelocale(RPC_Thread_Locale);
        }
#endif
            m_pRPC_Thread_Mutex->Unlock();  // Just for safety - not really needed
            // Tell CMainDocument that thread has gracefully ended 
            // We do this here because OnExit() is not called on Windows
            m_pDoc->m_RPCThread = NULL;
            return 0;
        }
        
        current_request = m_pDoc->GetCurrentRPCRequest();

        if (!current_request->isActive)  continue;       // Should never happen
        
        if (current_request->RPCExecutionTime) {
            startTime = dtime();
        }
        retval = ProcessRPCRequest();
        if (current_request->RPCExecutionTime) {
            *(current_request->RPCExecutionTime) = dtime() - startTime;
        }
        
        current_request->retval = retval;

        mutexErr = m_pRPC_Request_Mutex->Lock();
        wxASSERT(mutexErr == wxMUTEX_NO_ERROR);

        current_request->isActive = false;
        wxPostEvent( wxTheApp, RPC_done_event );

        // Signal() is ignored / discarded unless the main thread is 
        // currently blocked by m_pRPC_Request_Condition->Wait[Timeout]()
        m_pRPC_Request_Condition->Signal();
        
        mutexErr = m_pRPC_Request_Mutex->Unlock();
        wxASSERT(mutexErr == wxMUTEX_NO_ERROR);
    }

    return NULL;
}


int RPCThread::ProcessRPCRequest() {
    int                     retval = 0;
    ASYNC_RPC_REQUEST       *current_request = m_pDoc->GetCurrentRPCRequest();
    
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
        retval = (m_pDoc->rpcClient).authorize((const char*)(current_request->arg1));
        break;
    case RPC_EXCHANGE_VERSIONS:
        retval = (m_pDoc->rpcClient).exchange_versions(*(VERSION_INFO*)(current_request->arg1));
        break;
    case RPC_GET_STATE:
        retval = (m_pDoc->rpcClient).get_state(*(CC_STATE*)(current_request->arg1));
        break;
    case RPC_GET_RESULTS:
        retval = (m_pDoc->rpcClient).get_results(*(RESULTS*)(current_request->arg1), *(bool*)(current_request->arg2));
        break;
    case RPC_GET_FILE_TRANSFERS:
        retval = (m_pDoc->rpcClient).get_file_transfers(*(FILE_TRANSFERS*)(current_request->arg1));
        break;
    case RPC_GET_SIMPLE_GUI_INFO1:
        retval = (m_pDoc->rpcClient).get_simple_gui_info(*(SIMPLE_GUI_INFO*)(current_request->arg1));
        break;
    case RPC_GET_SIMPLE_GUI_INFO2:
        // RPC_GET_SIMPLE_GUI_INFO2 is equivalent to doing both 
        // RPC_GET_PROJECT_STATUS1 and RPC_GET_RESULTS
        retval = (m_pDoc->rpcClient).get_results(*(RESULTS*)(current_request->arg3));
        if (!retval) {
            retval = (m_pDoc->rpcClient).get_project_status(*(PROJECTS*)(current_request->arg1));
        }
        break;
    case RPC_GET_PROJECT_STATUS1:
        retval = (m_pDoc->rpcClient).get_project_status(*(PROJECTS*)(current_request->arg1));
        break;
    case RPC_GET_PROJECT_STATUS2:
        retval = (m_pDoc->rpcClient).get_project_status(*(PROJECTS*)(current_request->arg1));
        break;
    case RPC_GET_ALL_PROJECTS_LIST:
        retval = (m_pDoc->rpcClient).get_all_projects_list(*(ALL_PROJECTS_LIST*)(current_request->arg1));
        break;
    case RPC_GET_DISK_USAGE:
        retval = (m_pDoc->rpcClient).get_disk_usage(*(DISK_USAGE*)(current_request->arg1));
        break;
    case RPC_PROJECT_OP:
        retval = (m_pDoc->rpcClient).project_op(
            *(PROJECT*)(current_request->arg1), 
            (const char*)(current_request->arg2)
        );
        break;
    case RPC_SET_RUN_MODE:
        retval = (m_pDoc->rpcClient).set_run_mode(
            *(int*)(current_request->arg1), 
            *(double*)(current_request->arg2)
        );
        break;
    case RPC_SET_GPU_MODE:
        retval = (m_pDoc->rpcClient).set_gpu_mode(
            *(int*)(current_request->arg1), 
            *(double*)(current_request->arg2)
        );
        break;
    case RPC_SET_NETWORK_MODE:
        retval = (m_pDoc->rpcClient).set_network_mode(
            *(int*)(current_request->arg1),
            *(double*)(current_request->arg2)
        );
        break;
    case RPC_GET_SCREENSAVER_TASKS:
        retval = (m_pDoc->rpcClient).get_screensaver_tasks(
            *(int*)(current_request->arg1),
            *(RESULTS*)(current_request->arg2)
        );
        break;
    case RPC_RUN_BENCHMARKS:
        retval = (m_pDoc->rpcClient).run_benchmarks();
        break;
    case RPC_SET_PROXY_SETTINGS:
        retval = (m_pDoc->rpcClient).set_proxy_settings(*(GR_PROXY_INFO*)(current_request->arg1));
        break;
    case RPC_GET_PROXY_SETTINGS:
        retval = (m_pDoc->rpcClient).get_proxy_settings(*(GR_PROXY_INFO*)(current_request->arg1));
        break;
    case RPC_GET_NOTICES:
        retval = (m_pDoc->rpcClient).get_notices(
            *(int*)(current_request->arg1), 
            *(NOTICES*)(current_request->arg2)
        );
        break;
    case RPC_GET_MESSAGES:
        retval = (m_pDoc->rpcClient).get_messages(
            *(int*)(current_request->arg1), 
            *(MESSAGES*)(current_request->arg2),
            *(bool*)(current_request->arg3)
        );
        break;
    case RPC_FILE_TRANSFER_OP:
        retval = (m_pDoc->rpcClient).file_transfer_op(
            *(FILE_TRANSFER*)(current_request->arg1), 
            (const char*)(current_request->arg2)
        );
        break;
    case RPC_RESULT_OP:
        retval = (m_pDoc->rpcClient).result_op(
            *(RESULT*)(current_request->arg1),
            (const char*)(current_request->arg2)
        );
        break;
    case RPC_GET_HOST_INFO:
        retval = (m_pDoc->rpcClient).get_host_info(*(HOST_INFO*)(current_request->arg1));
        break;
    case RPC_QUIT:
        retval = (m_pDoc->rpcClient).quit();
        break;
    case RPC_ACCT_MGR_INFO:
        retval = (m_pDoc->rpcClient).acct_mgr_info(*(ACCT_MGR_INFO*)(current_request->arg1));
        break;
    case RPC_GET_STATISTICS:
        retval = (m_pDoc->rpcClient).get_statistics(*(PROJECTS*)(current_request->arg1));
        break;
    case RPC_NETWORK_AVAILABLE:
        retval = (m_pDoc->rpcClient).network_available();
        break;
    case RPC_GET_PROJECT_INIT_STATUS:
        retval = (m_pDoc->rpcClient).get_project_init_status(*(PROJECT_INIT_STATUS*)(current_request->arg1));
        break;
    case RPC_GET_PROJECT_CONFIG:
        retval = (m_pDoc->rpcClient).get_project_config(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_PROJECT_CONFIG_POLL:
        retval = (m_pDoc->rpcClient).get_project_config_poll(*(PROJECT_CONFIG*)(current_request->arg1));
        break;
    case RPC_LOOKUP_ACCOUNT:
        retval = (m_pDoc->rpcClient).lookup_account(*(ACCOUNT_IN*)(current_request->arg1));
        break;
    case RPC_LOOKUP_ACCOUNT_POLL:
        retval = (m_pDoc->rpcClient).lookup_account_poll(*(ACCOUNT_OUT*)(current_request->arg1));
        break;
    case RPC_CREATE_ACCOUNT:
        retval = (m_pDoc->rpcClient).create_account(*(ACCOUNT_IN*)(current_request->arg1));
        break;
    case RPC_CREATE_ACCOUNT_POLL:
        retval = (m_pDoc->rpcClient).create_account_poll(*(ACCOUNT_OUT*)(current_request->arg1));
        break;
    case RPC_PROJECT_ATTACH:
        retval = (m_pDoc->rpcClient).project_attach(
            (const char*)(current_request->arg1), 
            (const char*)(current_request->arg2), 
            (const char*)(current_request->arg3)
        );
        break;
    case RPC_PROJECT_ATTACH_FROM_FILE:
        retval = (m_pDoc->rpcClient).project_attach_from_file();
        break;
    case RPC_PROJECT_ATTACH_POLL:
        retval = (m_pDoc->rpcClient).project_attach_poll(*(PROJECT_ATTACH_REPLY*)(current_request->arg1));
        break;
    case RPC_ACCT_MGR_RPC:
        retval = (m_pDoc->rpcClient).acct_mgr_rpc(
            (const char*)(current_request->arg1), 
            (const char*)(current_request->arg2), 
            (const char*)(current_request->arg3),
            (bool)(current_request->arg4 != NULL)
        );
        break;
    case RPC_ACCT_MGR_RPC_POLL:
        retval = (m_pDoc->rpcClient).acct_mgr_rpc_poll(*(ACCT_MGR_RPC_REPLY*)(current_request->arg1));
        break;
    case RPC_GET_NEWER_VERSION:
        retval = (m_pDoc->rpcClient).get_newer_version(
            *(std::string*)(current_request->arg1),
            *(std::string*)(current_request->arg2)
        );
        break;
    case RPC_READ_GLOBAL_PREFS_OVERRIDE:
        retval = (m_pDoc->rpcClient).read_global_prefs_override();
        break;
    case RPC_READ_CC_CONFIG:
        retval = (m_pDoc->rpcClient).read_cc_config();
        break;
    case RPC_GET_CC_STATUS:
        retval = (m_pDoc->rpcClient).get_cc_status(*(CC_STATUS*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_FILE:
        retval = (m_pDoc->rpcClient).get_global_prefs_file(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_WORKING:
        retval = (m_pDoc->rpcClient).get_global_prefs_working(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_WORKING_STRUCT:
        retval = (m_pDoc->rpcClient).get_global_prefs_working_struct(
            *(GLOBAL_PREFS*)(current_request->arg1), 
            *(GLOBAL_PREFS_MASK*)(current_request->arg2)
        );
        break;
    case RPC_GET_GLOBAL_PREFS_OVERRIDE:
        retval = (m_pDoc->rpcClient).get_global_prefs_override(*(std::string*)(current_request->arg1));
        break;
    case RPC_SET_GLOBAL_PREFS_OVERRIDE:
         retval = (m_pDoc->rpcClient).set_global_prefs_override(*(std::string*)(current_request->arg1));
        break;
    case RPC_GET_GLOBAL_PREFS_OVERRIDE_STRUCT:
        retval = (m_pDoc->rpcClient).get_global_prefs_override_struct(
            *(GLOBAL_PREFS*)(current_request->arg1), 
            *(GLOBAL_PREFS_MASK*)(current_request->arg2)
        );
        break;
    case RPC_SET_GLOBAL_PREFS_OVERRIDE_STRUCT:
        retval = (m_pDoc->rpcClient).set_global_prefs_override_struct(
            *(GLOBAL_PREFS*)(current_request->arg1), 
            *(GLOBAL_PREFS_MASK*)(current_request->arg2)
        );
        break;
    case RPC_GET_CC_CONFIG:
         retval = (m_pDoc->rpcClient).get_cc_config(
            *(CONFIG*)(current_request->arg1),
            *(LOG_FLAGS*)(current_request->arg2)
        );
        break;
    case RPC_SET_CC_CONFIG:
         retval = (m_pDoc->rpcClient).set_cc_config(
            *(CONFIG*)(current_request->arg1),
            *(LOG_FLAGS*)(current_request->arg2)
        );
        break;
    default:
        break;
    }

    return retval;
}


// TODO: combine RPC requests for different buffers, then just copy the buffer.

int CMainDocument::RequestRPC(ASYNC_RPC_REQUEST& request, bool hasPriority) {
    std::vector<ASYNC_RPC_REQUEST>::iterator iter;
    int retval = 0;
    int response = wxID_OK;
    wxMutexError mutexErr = wxMUTEX_NO_ERROR;
    long delayTimeRemaining, timeToSleep;
    bool shown = false;
    
    if (!m_RPCThread) return -1;

    if ( (request.rpcType < RPC_TYPE_WAIT_FOR_COMPLETION) || 
            (request.rpcType >= NUM_RPC_TYPES) ) {
        wxASSERT(false);
        return -1;
    }
    
    // If we are quitting, cancel any pending RPCs
    if (request.which_rpc == RPC_QUIT) {
        if (current_rpc_request.isActive) {
            RPC_requests.erase(RPC_requests.begin()+1, RPC_requests.end());

        } else {
            RPC_requests.clear();
        }
    }
    
    // Check if a duplicate request is already on the queue
    for (iter=RPC_requests.begin(); iter!=RPC_requests.end(); iter++) {
        if (iter->isSameAs(request)) {
            return 0;
        }
    }

    if ((request.rpcType == RPC_TYPE_WAIT_FOR_COMPLETION) && (request.resultPtr == NULL)) {
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
        // Wait for thread to unlock mutex with m_pRPC_Thread_Condition->Wait()
        mutexErr = m_pRPC_Thread_Mutex->Lock();  // Blocks until thread unlocks the mutex
        wxASSERT(mutexErr == wxMUTEX_NO_ERROR);

        // Make sure activation is an atomic operation
        request.isActive = false;
        current_rpc_request = request;
        current_rpc_request.isActive = true;

        m_pRPC_Thread_Condition->Signal();  // Unblock the thread

        // m_pRPC_Thread_Condition->Wait() will Lock() the mutex upon receiving Signal(), 
        // causing it to block again if we still have our lock on the mutex.
        mutexErr = m_pRPC_Thread_Mutex->Unlock();
        wxASSERT(mutexErr == wxMUTEX_NO_ERROR);
    }

    // If this is a user-initiated event wait for completion but show 
    // a dialog allowing the user to cancel.
    if (request.rpcType == RPC_TYPE_WAIT_FOR_COMPLETION) {
    // TODO: proper handling if a second user request is received while first is pending ??
        if (m_bWaitingForRPC) {
            wxLogMessage(wxT("Second user RPC request while another was pending"));
            wxASSERT(false);
            return -1;
        }
        // Don't show dialog if RPC completes before RPC_WAIT_DLG_DELAY
        // or while BOINC is minimized
        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
        wxStopWatch Dlgdelay = wxStopWatch();        
        m_RPCWaitDlg = new AsyncRPCDlg();
        m_bWaitingForRPC = true;
        
        // Allow RPC_WAIT_DLG_DELAY seconds for Demand RPC to complete before 
        // displaying "Please Wait" dialog, but keep checking for completion.
        delayTimeRemaining = RPC_WAIT_DLG_DELAY;
        while (true) {
            if (delayTimeRemaining >= 0) {  // Prevent overflow if minimized for a very long time
                delayTimeRemaining = RPC_WAIT_DLG_DELAY - Dlgdelay.Time();
            }
            
            if (pFrame) {
                shown = pFrame->IsShown();
            } else {
                shown = false;
            }
            
            if (shown) {
                if (delayTimeRemaining <= 0) break; // Display the Please Wait dialog
                timeToSleep = delayTimeRemaining;
            } else {
                // Don't show dialog while Manager is minimized, but do 
                // process events so user can maximize the manager. 
                //
                // NOTE: CBOINCGUIApp::FilterEvent() discards those events 
                // which might cause posting of more RPC requests while 
                // we are in this loop, to prevent undesirable recursion.
                // Since the manager is minimized, we don't have to worry about 
                // discarding crucial drawing or command events. 
                // The filter does allow the the Open Manager menu item from 
                // the system tray icon and wxEVT_RPC_FINISHED event. 
                //
                timeToSleep = DELAY_WHEN_MINIMIZED; // Allow user to maximize Manager
                wxSafeYield(NULL, true);
            }
            
            // OnRPCComplete() clears m_bWaitingForRPC if RPC completed 
            if (! m_bWaitingForRPC) {
                return retval;
            }
            
            mutexErr = m_pRPC_Request_Mutex->Lock();
            wxASSERT(mutexErr == wxMUTEX_NO_ERROR);

            // Simulate handling of CRPCFinishedEvent but don't allow any other 
            // events (so no user activity) to prevent undesirable recursion.
            // Since we don't need to filter and discard events, they remain on 
            // the queue until it is safe to process them.
            // Allow RPC thread to run while we wait for it.
            if (!current_rpc_request.isActive) {
                mutexErr = m_pRPC_Request_Mutex->Unlock();
                wxASSERT(mutexErr == wxMUTEX_NO_ERROR);
                
                HandleCompletedRPC();
                continue;
            }

            // Wait for RPC thread to wake us
            // This does the following:
            // (1) Unlocks the Mutex and puts the main thread to sleep as an atomic operation.
            // (2) On Signal from RPC thread: locks Mutex again and wakes the main thread.
            m_pRPC_Request_Condition->WaitTimeout(timeToSleep);

            mutexErr = m_pRPC_Request_Mutex->Unlock();
            wxASSERT(mutexErr == wxMUTEX_NO_ERROR);
        }
        
        // Demand RPC has taken longer than RPC_WAIT_DLG_DELAY seconds and 
        // Manager is not minimized, so display the "Please Wait" dialog 
        // with a Cancel button.  If the RPC does complete while the dialog 
        // is up, HandleCompletedRPC() will call EndModal with wxID_OK.
        //
        // NOTE: the Modal dialog permits processing of all events, but 
        // CBOINCGUIApp::FilterEvent() blocks those events which might cause 
        // posting of more RPC requests while in this dialog, to prevent 
        // undesirable recursion.
        //
        if (m_RPCWaitDlg) {
            response = m_RPCWaitDlg->ShowModal();
            // Remember time the dialog was closed for use by RunPeriodicRPCs()
            m_dtLasAsyncRPCDlgTime = wxDateTime::Now();
            if (response != wxID_OK) {
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
                    rpcClient.close();
                    RPC_requests.clear();
                    current_rpc_request.clear();
                    m_bNeedRefresh = false;
                    m_bNeedTaskBarRefresh = false;

                    // We will be reconnected to the same client (if possible) by 
                    // CBOINCDialUpManager::OnPoll() and CNetworkConnection::Poll().
                    m_pNetworkConnection->SetStateDisconnected();
                }
                if (response == wxID_EXIT) {
                    pFrame = wxGetApp().GetFrame();
                    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_EXIT);
                    s_bSkipExitConfirmation = true;
                    pFrame->AddPendingEvent(evt);
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


void CMainDocument::KillRPCThread() {
    wxMutexError mutexErr = wxMUTEX_NO_ERROR;
    int i;

    if (!m_RPCThread) {
        return;
    }

    m_bNeedRefresh = false;
    m_bNeedTaskBarRefresh = false;
    
    rpcClient.close();  // Abort any async RPC in progress (in case hung)
   
    // On some platforms, Delete() takes effect only when thread calls TestDestroy()
    // Wait for thread to unlock mutex with m_pRPC_Thread_Condition->Wait()
    mutexErr = m_pRPC_Thread_Mutex->Lock();  // Blocks until thread unlocks the mutex
    wxASSERT(mutexErr == wxMUTEX_NO_ERROR);

    m_bShutDownRPCThread = true;
    m_pRPC_Thread_Condition->Signal();  // Unblock the thread

    mutexErr = m_pRPC_Thread_Mutex->Unlock(); // Release the mutex so thread can lock it
    wxASSERT(mutexErr == wxMUTEX_NO_ERROR);

    RPC_requests.clear();
    current_rpc_request.clear();

    // Wait up to RPC_KILL_DELAY milliseconds for thread to exit on its own
    for (i=0; i< RPC_KILL_DELAY; ++i) {
        boinc_sleep(.001);  // Defer to RPC thread for 1 millisecond
        if (!m_RPCThread) {
            return; // RPC thread sets m_RPCThread to NULL when it exits
        }
    }
    // Thread failed to exit, so forcefully kill it
    m_RPCThread->Kill();
}


void CMainDocument::OnRPCComplete(CRPCFinishedEvent&) {
    HandleCompletedRPC();
}   


void CMainDocument::HandleCompletedRPC() {
    int retval = 0;
    wxMutexError mutexErr = wxMUTEX_NO_ERROR;
    int i, n, requestIndex = -1;
    bool stillWaitingForPendingRequests = false;
    
    if (!m_RPCThread) return;
   
    if (current_rpc_request.isActive) return;
    
    // We can get here either via a CRPCFinishedEvent event posted 
    // by the RPC thread or by a call from RequestRPC.  If we were 
    // called from RequestRPC, the CRPCFinishedEvent will still be 
    // on the event queue, so we get called twice.  Check for this here.
    if (current_rpc_request.which_rpc == 0) return; // already handled by a call from RequestRPC

    // Find our completed request in the queue
    n = (int) RPC_requests.size();
    for (i=0; i<n; ++i) {
        if (RPC_requests[i].isSameAs(current_rpc_request)) {
            requestIndex = i;
        } else {
            if (RPC_requests[i].rpcType == RPC_TYPE_WAIT_FOR_COMPLETION) {
                stillWaitingForPendingRequests = true;
            }
        }
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

    if (requestIndex >= 0) {
        // Remove completed request from the queue
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
         if (current_rpc_request.rpcType == RPC_TYPE_ASYNC_WITH_REFRESH_AFTER) {
            if (!retval) {
                m_bNeedRefresh = true;
            }
        }
    
         if (current_rpc_request.rpcType == RPC_TYPE_ASYNC_WITH_UPDATE_TASKBAR_ICON_AFTER) {
            if (!retval) {
                m_bNeedTaskBarRefresh = true;
            }
        }
    
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
                exchangeBuf->have_nvidia = arg1->have_nvidia;
                exchangeBuf->have_ati = arg1->have_ati;
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
            if (!retval) {
                retval = CopyProjectsToStateBuffer(*(PROJECTS*)(current_rpc_request.arg1), *(CC_STATE*)(current_rpc_request.arg2));
            }
            if (current_rpc_request.exchangeBuf && !retval) {
                RESULTS* arg3 = (RESULTS*)current_rpc_request.arg3;
                RESULTS* exchangeBuf = (RESULTS*)current_rpc_request.exchangeBuf;
                arg3->results.swap(exchangeBuf->results);
            }
            break;
        case RPC_GET_PROJECT_STATUS1:
            if (!retval) {
                retval = CopyProjectsToStateBuffer(*(PROJECTS*)(current_rpc_request.arg1), *(CC_STATE*)(current_rpc_request.arg2));
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
        case RPC_GET_NOTICES:
            if (current_rpc_request.exchangeBuf && !retval) {
                NOTICES* arg2 = (NOTICES*)current_rpc_request.arg2;
                NOTICES* exchangeBuf = (NOTICES*)current_rpc_request.exchangeBuf;
                arg2->notices.swap(exchangeBuf->notices);
            }
            if (!retval) {
                CachedNoticeUpdate();  // Call this only when notice buffer is stable
            }
            break;
        case RPC_GET_MESSAGES:
            if (current_rpc_request.exchangeBuf && !retval) {
                MESSAGES* arg2 = (MESSAGES*)current_rpc_request.arg2;
                MESSAGES* exchangeBuf = (MESSAGES*)current_rpc_request.exchangeBuf;
                arg2->messages.swap(exchangeBuf->messages);
            }
            if (!retval) {
                CachedMessageUpdate();  // Call this only when message buffer is stable
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
    
    if (current_rpc_request.resultPtr) {
        // In case post-processing changed retval
        *(current_rpc_request.resultPtr) = retval;
    }

    // We must call ProcessEvent() rather than AddPendingEvent() here to 
    // guarantee integrity of data when other events are handled (such as 
    // Abort, Suspend/Resume, Show Graphics, Update, Detach, Reset, No 
    // New Work, etc.)  Otherwise, if one of those events is pending it 
    // might be processed first, and the data in the selected rows may not 
    // match the data which the user selected if any rows were added or 
    // deleted due to the RPC.  
    // The refresh event called here adjusts the selections to fix any 
    // such mismatch before other pending events are processed.  
    //
    // However, the refresh code may itself request a Demand RPC, which 
    // would cause undesirable recursion if we are already waiting for 
    // another Demand RPC to complete.  In that case, we defer the refresh 
    // until all pending Demand RPCs have been done.
    //
    if (m_bNeedRefresh && !m_bWaitingForRPC) {
        m_bNeedRefresh = false;
        // We must get the frame immediately before using it, 
        // since it may have been changed by SetActiveGUI().
        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
        if (pFrame) {
            CFrameEvent event(wxEVT_FRAME_REFRESHVIEW, pFrame);
            pFrame->ProcessEvent(event);
        }
    }

    if (m_bNeedTaskBarRefresh && !m_bWaitingForRPC) {
        m_bNeedTaskBarRefresh = false;
        CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
        if (pTaskbar) {
            CTaskbarEvent event(wxEVT_TASKBAR_REFRESH, pTaskbar);
            pTaskbar->ProcessEvent(event);
        }
    }

    if (current_rpc_request.rpcType == RPC_TYPE_ASYNC_WITH_REFRESH_EVENT_LOG_AFTER) {
        CDlgEventLog* eventLog = wxGetApp().GetEventLog();
        if (eventLog) {
            eventLog->OnRefresh();
        }
    }
    
    current_rpc_request.clear();

    // Start the next RPC request.  
    // We can't start this until finished processing the previous RPC's 
    // event because the two requests may write into the same buffer.
    if (RPC_requests.size() > 0) {
        // Wait for thread to unlock mutex with m_pRPC_Thread_Condition->Wait()
        mutexErr = m_pRPC_Thread_Mutex->Lock();  // Blocks until thread unlocks the mutex
        wxASSERT(mutexErr == wxMUTEX_NO_ERROR);

        // Make sure activation is an atomic operation
        RPC_requests[0].isActive = false;
        current_rpc_request = RPC_requests[0];
        current_rpc_request.isActive = true;

        m_pRPC_Thread_Condition->Signal();  // Unblock the thread

        // m_pRPC_Thread_Condition->Wait() will Lock() the mutex upon receiving Signal(), 
        // causing it to block again if we still have our lock on the mutex.
        mutexErr = m_pRPC_Thread_Mutex->Unlock();
        wxASSERT(mutexErr == wxMUTEX_NO_ERROR);
    }
}


int CMainDocument::CopyProjectsToStateBuffer(PROJECTS& p, CC_STATE& state) {
    int retval = 0;
    unsigned int i;
    PROJECT* state_project = NULL;

    // flag for delete
    for (i=0; i<state.projects.size(); i++) {
        state_project = state.projects[i];
        state_project->flag_for_delete = true;
    }

    for (i=0; i<p.projects.size(); i++) {
        state_project = state.lookup_project(p.projects[i]->master_url);
        if (state_project && (!strcmp(p.projects[i]->master_url, state_project->master_url))) {
            // Because the CC_STATE contains several pointers to each element of the 
            // CC_STATE::projects vector, we must update these elements in place.
            *state_project = *(p.projects[i]);
            state_project->flag_for_delete = false;
        } else {
            retval = ERR_NOT_FOUND;
        }
        continue;
    }

    // Anything need to be deleted?
    if (!retval) {
        for (i=0; i<state.projects.size(); i++) {
            state_project = state.projects[i];
            if (state_project->flag_for_delete) {
                retval = ERR_FILE_MISSING;
            }
        }
    }
    
    return retval;
}


BEGIN_EVENT_TABLE(AsyncRPCDlg, wxDialog)
    EVT_BUTTON(wxID_EXIT, AsyncRPCDlg::OnExit)
END_EVENT_TABLE()

IMPLEMENT_CLASS(AsyncRPCDlg, wxDialog)

AsyncRPCDlg::AsyncRPCDlg() : wxDialog( NULL, wxID_ANY, wxT(""), wxDefaultPosition ) {
    CSkinAdvanced*  pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString exit_label;
    wxASSERT(pSkinAdvanced);

    wxString message = wxString(_("Communicating with BOINC client.  Please wait ..."));
    
#ifdef __WXMAC__
    exit_label.Printf(_("&Quit %s"), pSkinAdvanced->GetApplicationName().c_str());
#else
    exit_label.Printf(_("E&xit %s"), pSkinAdvanced->GetApplicationName().c_str());
#endif

    wxString strCaption;
    strCaption.Printf(_("%s - Communication"), pSkinAdvanced->GetApplicationName().c_str());
    SetTitle(strCaption.c_str());

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );

    icon_text->Add( CreateTextSizer( message ), 0, wxALIGN_CENTER | wxLEFT, 10 );
    topsizer->Add( icon_text, 1, wxCENTER | wxLEFT|wxRIGHT|wxTOP, 10 );
    
    wxStdDialogButtonSizer *sizerBtn = CreateStdDialogButtonSizer(0);
    
    wxButton* exitbutton = new wxButton;
    exitbutton->Create( this, wxID_EXIT, exit_label, wxDefaultPosition, wxDefaultSize, 0 );
    sizerBtn->Add(exitbutton, 0, wxLEFT|wxRIGHT|wxALL, 5);

    wxButton* cancelbutton = new wxButton;
    cancelbutton->Create( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    sizerBtn->Add(cancelbutton, 0, wxLEFT|wxRIGHT|wxALL, 5);
    
    if ( sizerBtn )
        topsizer->Add(sizerBtn, 0, wxEXPAND | wxALL, 10 );

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


void AsyncRPCDlg::OnExit(wxCommandEvent& WXUNUSED(eventUnused)) {
    EndModal(wxID_EXIT);
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
    request.rpcType = RPC_TYPE_WAIT_FOR_COMPLETION;
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
