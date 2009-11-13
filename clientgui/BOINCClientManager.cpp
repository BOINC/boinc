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
#include "procinfo.h"
#include "filesys.h"
#include "daemonmgt.h"
#include "util.h"

#ifdef __WXMAC__
enum {
    NewStyleDaemon = 1,
    OldStyleDaemon
};

#elif defined(__WXMSW__)

#include "win_util.h"
#include "diagnostics_win.h"

extern int diagnostics_get_process_information(PVOID* ppBuffer, PULONG pcbBuffer);

#else
#include <sys/wait.h>
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
    if (!m_bBOINCStartedByManager) return false;
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
    if (is_daemon_installed()) bReturnValue = 1;
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
    bool running = false;

#ifdef __WXMSW__
    char buf[MAX_PATH] = "";
    
    if (is_daemon_installed()) {
        running = (FALSE != is_daemon_starting()) || (FALSE != is_daemon_running());
    } else {
        // Global mutex on Win2k and later
        //
        if (IsWindows2000Compatible()) {
            strcpy(buf, "Global\\");
        }
        strcat( buf, RUN_MUTEX);

        HANDLE h = CreateMutexA(NULL, true, buf);
        DWORD err = GetLastError();
        if ((h==0) || (err == ERROR_ALREADY_EXISTS)) {
            running = true;
        }
        if (h) {
            CloseHandle(h);
        }
    }
#elif defined(__WXMAC__)
    char path[1024];
    static FILE_LOCK file_lock;
    
    sprintf(path, "%s/%s", (const char *)wxGetApp().GetDataDirectory().mb_str(), LOCK_FILE_NAME);
    if (boinc_file_exists(path)) {   // If there is no lock file, core is not running
        if (file_lock.lock(path)) {
            running = true;
        } else {
            file_lock.unlock(path);
        }
    }
#else
    std::vector<PROCINFO> piv;
    int retval;

    if (m_lBOINCCoreProcessId) {
        // Prevent client from being a zombie
        if (waitpid(m_lBOINCCoreProcessId, 0, WNOHANG) == m_lBOINCCoreProcessId) {
            m_lBOINCCoreProcessId = 0;
        }
    }

    // Look for BOINC Client in list of all running processes
    retval = procinfo_setup(piv);
    if (retval) return false;     // Should never happen
    
    for (unsigned int i=0; i<piv.size(); i++) {
        PROCINFO& pi = piv[i];
        if (!strcmp(pi.command, "boinc")) {
            running = true;
            break;
        }
        if (!strcmp(pi.command, "boinc_client")) {
            running = true;
            break;
        }
    }
#endif
    wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Returning '%d'"), (int)running);
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Function End"));
    return running;
}


bool CBOINCClientManager::StartupBOINCCore() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::StartupBOINCCore - Function Begin"));

    bool                bReturnValue = false;
    wxString            strExecute = wxEmptyString;

    if (IsBOINCCoreRunning()) return true;

#if defined(__WXMSW__)
    LPTSTR  szExecute = NULL;
    LPTSTR  szDataDirectory = NULL;

    if (IsBOINCConfiguredAsDaemon()) {
        start_daemon_via_daemonctrl();

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
    wxString savedWD = ::wxGetCwd();
    
    wxSetWorkingDirectory(wxGetApp().GetDataDirectory());
    
    // Append boinc.exe to the end of the strExecute string and get ready to rock
    strExecute = wxGetApp().GetRootDirectory() + wxT("boinc --redirectio --launched_by_manager");
#ifdef SANDBOX
    if (!g_use_sandbox) {
        strExecute += wxT(" --insecure");
    }
#endif

    wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - szExecute '%s'\n"), strExecute.c_str());
    wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - szDataDirectory '%s'\n"), wxGetApp().GetDataDirectory().c_str());

    m_lBOINCCoreProcessId = ::wxExecute(strExecute);
    
    wxSetWorkingDirectory(savedWD);
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


#ifdef __WXMSW__
static tstring downcase_string(tstring& orig) {
    tstring retval = orig;
    for (size_t i=0; i < retval.length(); i++) {
        retval[i] = tolower(retval[i]);
    }
    return retval;
}


void CBOINCClientManager::KillClient() {
    ULONG                   cbBuffer = 128*1024;    // 128k initial buffer
    PVOID                   pBuffer = NULL;
    PSYSTEM_PROCESSES       pProcesses = NULL;

    if (m_hBOINCCoreProcess != NULL) {
        kill_program(m_hBOINCCoreProcess);
        return;
    }

    // Get a snapshot of the process and thread information.
    diagnostics_get_process_information(&pBuffer, &cbBuffer);

    // Lets start walking the structures to find the good stuff.
    pProcesses = (PSYSTEM_PROCESSES)pBuffer;
    do {
        if (pProcesses->ProcessId) {
            tstring strProcessName = pProcesses->ProcessName.Buffer;
            if (downcase_string(strProcessName) == tstring(_T("boinc.exe"))) {
                TerminateProcessById(pProcesses->ProcessId);
                break;
           }
        }

        // Move to the next structure if one exists
        if (!pProcesses->NextEntryDelta) {
            break;
        }
        pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses) + pProcesses->NextEntryDelta);
    } while (pProcesses);

    // Release resources
    if (pBuffer) HeapFree(GetProcessHeap(), NULL, pBuffer);
}

#else

void CBOINCClientManager::KillClient() {
    std::vector<PROCINFO> piv;
    int retval;
    
    if (m_lBOINCCoreProcessId) {
        kill_program(m_lBOINCCoreProcessId);
        return;
    }

    retval = procinfo_setup(piv);
	if (retval) return;     // Should never happen
    
    for (unsigned int i=0; i<piv.size(); i++) {
        PROCINFO& pi = piv[i];
        if (!strcmp(pi.command, "boinc")) {
            kill_program(pi.id);
            break;
        }
    }
}
#endif


void CBOINCClientManager::ShutdownBOINCCore() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::ShutdownBOINCCore - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    wxInt32            iCount = 0;
    bool               bClientQuit = false;
    wxString           strConnectedCompter = wxEmptyString;
    wxString           strPassword = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (m_bBOINCStartedByManager) {
#ifdef __WXMSW__
        if (IsBOINCConfiguredAsDaemon()) {
            stop_daemon_via_daemonctrl();
            bClientQuit = true;
        } else
#endif
        {
            pDoc->GetConnectedComputerName(strConnectedCompter);
            if (!pDoc->IsComputerNameLocal(strConnectedCompter)) {
                RPC_CLIENT rpc;
                if (!rpc.init("localhost")) {
                    pDoc->m_pNetworkConnection->GetLocalPassword(strPassword);
                    rpc.authorize((const char*)strPassword.mb_str());
                    if (IsBOINCCoreRunning()) {
                        rpc.quit();
                        for (iCount = 0; iCount <= 10; iCount++) {
                            if (!bClientQuit && !IsBOINCCoreRunning()) {
                                wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - (localhost) Application Exit Detected"));
                                bClientQuit = true;
                                break;
                            }
                            wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - (localhost) Application Exit NOT Detected, Sleeping..."));
                            ::wxSleep(1);
                        }
                    } else {
                        bClientQuit = true;
                    }
                }
                rpc.close();
            } else {
                if (IsBOINCCoreRunning()) {
                    pDoc->CoreClientQuit();
                    for (iCount = 0; iCount <= 10; iCount++) {
                        if (!bClientQuit && !IsBOINCCoreRunning()) {
                            wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - Application Exit Detected"));
                            bClientQuit = true;
                            break;
                        }
                        wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - Application Exit NOT Detected, Sleeping..."));
                        ::wxSleep(1);
                    }
                } else {
                    bClientQuit = true;
                }
            }
        }

        if (!bClientQuit) {
            KillClient();
        }
        m_lBOINCCoreProcessId = 0;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::ShutdownBOINCCore - Function End"));
}
