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
#pragma implementation "BOINCClientManager.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "AdvancedFrame.h"
#include "BOINCClientManager.h"
#include "error_numbers.h"

#include "filesys.h"
#include "util.h"

#ifdef __WXMAC__
enum {
    NewStyleDaemon = 1,
    OldStyleDaemon
};
#endif

#ifdef __WXMSW__
EXTERN_C BOOL  IsBOINCServiceInstalled();
EXTERN_C BOOL  IsBOINCServiceStarting();
EXTERN_C BOOL  IsBOINCServiceRunning();
EXTERN_C BOOL  IsBOINCServiceStopping();
EXTERN_C BOOL  IsBOINCServiceStopped();
EXTERN_C BOOL  StartBOINCService();
EXTERN_C BOOL  StopBOINCService();
#endif

CBOINCClientManager::CBOINCClientManager() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::CBOINCClientManager - Function Begin"));

    m_bBOINCStartedByManager = false;
    m_lBOINCCoreProcessId = 0;

#ifdef __WXMSW__
    m_hBOINCCoreProcess = NULL;
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::CBOINCClientManager - Function End"));
}


CBOINCClientManager::~CBOINCClientManager() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::~CBOINCClientManager - Function Begin"));

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::~CBOINCClientManager - Function End"));
}


bool CBOINCClientManager::AutoRestart() {
    if (IsBOINCCoreRunning()) return true;
#ifndef __WXMAC__       // Mac can restart Client as a daemon
    if (! m_bBOINCStartedByManager) return false;
#endif
    m_lBOINCCoreProcessId = 0;
    StartupBOINCCore();
    return true;
}


bool CBOINCClientManager::IsSystemBooting() {
    bool bReturnValue = false;
#if   defined(__WXMSW__)
    if (GetTickCount() < (1000*60*5)) bReturnValue = true;  // If system has been up for less than 5 minutes 
#elif defined(__WXMAC__)
    if (TickCount() < (120*60)) bReturnValue = true;        // If system has been up for less than 2 minutes 
#endif
    return bReturnValue;
}


int CBOINCClientManager::IsBOINCConfiguredAsDaemon() {
    bool bReturnValue = false;
#if   defined(__WXMSW__)
    if (IsBOINCServiceInstalled()) bReturnValue = 1;
#elif defined(__WXMAC__)
    if ( boinc_file_exists("/Library/LaunchDaemons/edu.berkeley.boinc.plist")) {
        bReturnValue = NewStyleDaemon;                      // New-style daemon uses launchd
    }
    if (boinc_file_exists("/Library/StartupItems/boinc/boinc") ) {
        bReturnValue = OldStyleDaemon;                      // Old-style daemon uses StartupItem
    }
#endif
    return bReturnValue;
}


bool CBOINCClientManager::IsBOINCCoreRunning() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Function Begin"));

    int retval=0;
    bool running = false;
    RPC_CLIENT rpc;

#ifdef __WXMSW__
    if (IsBOINCServiceInstalled()) {
        running = (FALSE != IsBOINCServiceStarting()) || (FALSE != IsBOINCServiceRunning());
    } else {
#endif
    // If set up to run as a daemon, allow time for daemon to start up
    for (int i=0; i<10; i++) {
        retval = rpc.init("localhost");  // synchronous is OK since local
        wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Connecting to core client returned '%d'"), retval);
        retval = rpc.authorize("");      // Do not use an RPC that uses the SET_LOCALE class, this
                                         //   function is typically called from the UI thread.  If the
                                         //   UI thread and the async thread happen to use SET_LOCALE
                                         //   at the same time there is a 50% chance that the UI will
                                         //   be partially suck using the "C" locale which is needed to
                                         //   parse the data coming back from the CC.
        wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Requesting host info... retval '%d'"), retval);
        running = (retval != ERR_CONNECT);
        rpc.close();
        if (running) break;
        if (!IsBOINCConfiguredAsDaemon()) break;
            wxSleep(1);
    }
#ifdef __WXMSW__
    }
#endif

    wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Returning '%d'"), retval);
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Function End"));
    return running;
}


bool CBOINCClientManager::StartupBOINCCore() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::StartupBOINCCore - Function Begin"));

    bool                bReturnValue = false;
    wxString            strExecute = wxEmptyString;

    if (IsBOINCCoreRunning()) return true;

#if   defined(__WXMSW__)
    LPTSTR  szExecute = NULL;
    LPTSTR  szDataDirectory = NULL;

    if (IsBOINCConfiguredAsDaemon()) {
        StartBOINCService();

        m_bBOINCStartedByManager = true;
        bReturnValue = IsBOINCCoreRunning();
    } else {

        // Append boinc.exe to the end of the strExecute string and get ready to rock
        strExecute.Printf(
            wxT("\"%s\\boinc.exe\" --redirectio --launched_by_manager %s"),
            wxGetApp().GetRootDirectory().c_str(),
            wxGetApp().GetArguments().c_str()
        );

        PROCESS_INFORMATION pi;
        STARTUPINFO         si;
        BOOL                bProcessStarted;

        memset(&pi, 0, sizeof(pi));
        memset(&si, 0, sizeof(si));

        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        szExecute = (LPTSTR)strExecute.c_str();
        if (wxGetApp().GetDataDirectory().empty()) {
            szDataDirectory = NULL;
        } else {
            szDataDirectory = (LPTSTR)wxGetApp().GetDataDirectory().c_str();
        }

        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - szExecute '%s'\n"), szExecute);
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - szDataDirectory '%s'\n"), szDataDirectory);

        bProcessStarted = CreateProcess(
            NULL,
            szExecute,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW,
            NULL,
            szDataDirectory,
            &si,
            &pi
        );

        if (bProcessStarted) {
            m_lBOINCCoreProcessId = pi.dwProcessId;
            m_hBOINCCoreProcess = pi.hProcess;
        }
    }

#elif defined(__WXMAC__)

#if 0   // The Mac version of wxExecute(wxString& ...) crashes if there is a space in the path
    wxChar buf[1024];
    wxChar *argv[5];
#else
    char buf[1024];
    char *argv[5];
#endif
    ProcessSerialNumber ourPSN;
    FSRef ourFSRef;
    OSErr err;

    if (IsBOINCConfiguredAsDaemon() == NewStyleDaemon) {
        system ("launchctl load /Library/LaunchDaemons/edu.berkeley.boinc.plist");
        system ("launchctl start edu.berkeley.boinc");
        bReturnValue = IsBOINCCoreRunning();
    } else {
        
        // Get the full path to core client inside this application's bundle
        err = GetCurrentProcess (&ourPSN);
        if (err == noErr) {
            err = GetProcessBundleLocation(&ourPSN, &ourFSRef);
        }
        if (err == noErr) {
            err = FSRefMakePath (&ourFSRef, (UInt8*)buf, sizeof(buf));
        }
        if (err == noErr) {
#if 0   // The Mac version of wxExecute(wxString& ...) crashes if there is a space in the path
            strExecute = wxT("\"");            
            strExecute += wxT(buf);
            strExecute += wxT("/Contents/Resources/boinc\" --redirectio --launched_by_manager");
            m_lBOINCCoreProcessId = ::wxExecute(strExecute);
#else   // Use wxExecute(wxChar **argv ...) instead of wxExecute(wxString& ...)
            strcat(buf, "/Contents/Resources/boinc");
            argv[0] = buf;
            argv[1] = "--redirectio";
            argv[2] = "--launched_by_manager";
            argv[3] = NULL;
#ifdef SANDBOX
            if (!g_use_sandbox) {
                argv[3] = "--insecure";
                argv[4] = NULL;
            }
#endif
            // Under wxMac-2.8.0, wxExecute starts a separate thread
            // to wait for child's termination.
            // That wxProcessTerminationThread uses a huge amount of processor
            // time (about 11% of one CPU on 2GHz Intel Dual-Core Mac).
//                m_lBOINCCoreProcessId = ::wxExecute(argv);
            run_program(
                "/Library/Application Support/BOINC Data",
                buf, argv[3] ? 4 : 3, argv, 0.0, m_lBOINCCoreProcessId
            );
#endif
        } else {
            buf[0] = '\0';
        }
    }

#else   // Unix based systems

    // Append boinc.exe to the end of the strExecute string and get ready to rock
    strExecute = ::wxGetCwd() + wxT("/boinc --redirectio --launched_by_manager");
#ifdef SANDBOX
    if (!g_use_sandbox) {
        strExecute += wxT(" --insecure");
    }
#endif

    wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - szExecute '%s'\n"), strExecute.c_str());
    wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - szDataDirectory '%s'\n"), ::wxGetCwd().c_str());

    m_lBOINCCoreProcessId = ::wxExecute(strExecute);
    
#endif

    if (0 != m_lBOINCCoreProcessId) {
        m_bBOINCStartedByManager = true;
        bReturnValue = true;
        // Allow time for daemon to start up so we don't keep relaunching it
        for (int i=0; i<100; i++) {     // Wait up to 1 seccond in 10 ms increments
            boinc_sleep(0.01);
            if (IsBOINCCoreRunning()) break;
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::StartupBOINCCore - Function End"));
    return bReturnValue;
}


#if defined(__WXMSW__)

void CBOINCClientManager::ShutdownBOINCCore() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::ShutdownBOINCCore - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    wxInt32            iCount = 0;
    DWORD              dwExitCode = 0;
    bool               bClientQuit = false;
    wxString           strConnectedCompter = wxEmptyString;
    wxString           strPassword = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (m_bBOINCStartedByManager) {
        if (IsBOINCConfiguredAsDaemon()) {
            StopBOINCService();
            bClientQuit = true;
        } else {

            pDoc->GetConnectedComputerName(strConnectedCompter);
            if (!pDoc->IsComputerNameLocal(strConnectedCompter)) {
                RPC_CLIENT rpc;
                if (!rpc.init("localhost")) {
                    pDoc->m_pNetworkConnection->GetLocalPassword(strPassword);
                    rpc.authorize((const char*)strPassword.mb_str());
                    if (GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                        if (STILL_ACTIVE == dwExitCode) {
                            rpc.quit();
                            for (iCount = 0; iCount <= 10; iCount++) {
                                if (!bClientQuit && GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                                    if (STILL_ACTIVE != dwExitCode) {
                                        wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - (localhost) Application Exit Detected"));
                                        bClientQuit = true;
                                        break;
                                    }
                                }
                                wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - (localhost) Application Exit NOT Detected, Sleeping..."));
                                ::wxSleep(1);
                            }
                        }
                    }
                }
                rpc.close();
            } else {
                if (GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                    if (STILL_ACTIVE == dwExitCode) {
                        pDoc->CoreClientQuit();
                        for (iCount = 0; iCount <= 10; iCount++) {
                            if (!bClientQuit && GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                                if (STILL_ACTIVE != dwExitCode) {
                                    wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - Application Exit Detected"));
                                    bClientQuit = true;
                                    break;
                                }
                            }
                            wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - Application Exit NOT Detected, Sleeping..."));
                            ::wxSleep(1);
                        }
                    }
                }
            }
        }

        if (!bClientQuit) {
            ::wxKill(m_lBOINCCoreProcessId);
        }
        m_lBOINCCoreProcessId = 0;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::ShutdownBOINCCore - Function End"));
}

#elif defined(__WXMAC__)

static char * PersistentFGets(char *buf, size_t buflen, FILE *f) {
    char *p = buf;
    size_t len = buflen;
    size_t datalen = 0;

    *buf = '\0';
    while (datalen < (buflen - 1)) {
        fgets(p, len, f);
        if (feof(f)) break;
        if (ferror(f) && (errno != EINTR)) break;
        if (strchr(buf, '\n')) break;
        datalen = strlen(buf);
        p = buf + datalen;
        len -= datalen;
    }
    return (buf[0] ? buf : NULL);
}

bool CBOINCClientManager::ProcessExists(pid_t thePID)
{
    FILE *f;
    char buf[256];
    pid_t aPID;

    f = popen("ps -a -x -c -o pid,state", "r");
    if (f == NULL)
        return false;
    
    while (PersistentFGets(buf, sizeof(buf), f)) {
        aPID = atol(buf);
        if (aPID == thePID) {
            if (strchr(buf, 'Z'))   // A 'zombie', stopped but waiting
                break;              // for us (its parent) to quit
            pclose(f);
            return true;
        }
    }
    pclose(f);
    return false;
}

// wxProcess::Exists and wxKill are unimplemented in WxMac-2.6.0
void CBOINCClientManager::ShutdownBOINCCore() {
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    wxInt32            iCount = 0;
    wxString           strConnectedCompter = wxEmptyString;
    wxString           strPassword = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (m_bBOINCStartedByManager) {
        pDoc->GetConnectedComputerName(strConnectedCompter);
        if (!pDoc->IsComputerNameLocal(strConnectedCompter)) {
            RPC_CLIENT rpc;
            if (!rpc.init("localhost")) {
                pDoc->m_pNetworkConnection->GetLocalPassword(strPassword);
                rpc.authorize((const char*)strPassword.mb_str());
                if (ProcessExists(m_lBOINCCoreProcessId)) {
                    rpc.quit();
                    for (iCount = 0; iCount <= 10; iCount++) {
                        if (!ProcessExists(m_lBOINCCoreProcessId))
                            return;
                        ::wxSleep(1);
                    }
                }
            }
            rpc.close();
        } else {
            if (ProcessExists(m_lBOINCCoreProcessId)) {
                pDoc->CoreClientQuit();
                for (iCount = 0; iCount <= 10; iCount++) {
                    if (!ProcessExists(m_lBOINCCoreProcessId))
                        return;

                    ::wxSleep(1);
                }
            }
        }
        
        // Client did not quit after 10 seconds so kill it
        kill(m_lBOINCCoreProcessId, SIGKILL);
    }
    m_lBOINCCoreProcessId = 0;
}

#else

void CBOINCClientManager::ShutdownBOINCCore() {
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    wxInt32            iCount = 0;
    bool               bClientQuit = false;
    wxString           strConnectedCompter = wxEmptyString;
    wxString           strPassword = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (m_bBOINCStartedByManager) {
        pDoc->GetConnectedComputerName(strConnectedCompter);
        if (!pDoc->IsComputerNameLocal(strConnectedCompter)) {
            RPC_CLIENT rpc;
            if (!rpc.init("localhost")) {
                pDoc->m_pNetworkConnection->GetLocalPassword(strPassword);
                rpc.authorize((const char*)strPassword.mb_str());
                if (wxProcess::Exists(m_lBOINCCoreProcessId)) {
                    rpc.quit();
                    for (iCount = 0; iCount <= 10; iCount++) {
                        if (!bClientQuit && !wxProcess::Exists(m_lBOINCCoreProcessId)) {
                            bClientQuit = true;
                            break;
                        }
                        ::wxSleep(1);
                    }
                }
            }
            rpc.close();
        } else {
            if (wxProcess::Exists(m_lBOINCCoreProcessId)) {
                pDoc->CoreClientQuit();
                for (iCount = 0; iCount <= 10; iCount++) {
                    if (!bClientQuit && !wxProcess::Exists(m_lBOINCCoreProcessId)) {
                        bClientQuit = true;
                        break;
                    }
                    ::wxSleep(1);
                }
            }
        }

        if (!bClientQuit) {
            ::wxKill(m_lBOINCCoreProcessId);
        }
    }
}

#endif

