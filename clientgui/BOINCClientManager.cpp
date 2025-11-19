// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#include "miofile.h"
#include "str_replace.h"
#include "util.h"

#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "AdvancedFrame.h"
#include "BOINCClientManager.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "filesys.h"
#include "daemonmgt.h"
#include "Events.h"
#include "version.h"

// Alert user if Client crashes 3 times in 30 minutes
#define CLIENT_3_CRASH_MAX_TIME 30

#ifdef __WXMAC__
#include "mac_util.h"
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
#include <cstdio>
#endif

CBOINCClientManager::CBOINCClientManager() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::CBOINCClientManager - Function Begin"));

    m_bBOINCStartedByManager = false;
    m_lBOINCCoreProcessId = 0;
    m_fAutoRestart1Time = 0;
    m_fAutoRestart2Time = 0;

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
    double timeNow, timeDiff;
    if (IsBOINCCoreRunning()) return true;
#if ! (defined(__WXMAC__) || defined(__WXMSW__))
// Mac and Windows can restart Client as a daemon, but
// Linux may not know Client's location if it didn't start the Client
    if (!m_bBOINCStartedByManager) return false;
#endif
    // Alert user if Client crashes 3 times in CLIENT_3_CRASH_MAX_TIME
    timeNow = dtime();
    timeDiff = timeNow - m_fAutoRestart1Time;
    if ((timeDiff) < (CLIENT_3_CRASH_MAX_TIME * 60)) {
        int                 response;
        ClientCrashDlg      *dlg = new ClientCrashDlg(timeDiff);
        if (dlg) {
            CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
            if (!pFrame->IsShown()) {
                pFrame->Show();
            }
            response = dlg->ShowModal();
            dlg->Destroy();
            if (response == wxID_CANCEL) return false;
            timeNow = 0;
            m_fAutoRestart1Time = 0;
            m_fAutoRestart2Time = 0;
        }
    }
    m_lBOINCCoreProcessId = 0;
    m_fAutoRestart1Time = m_fAutoRestart2Time;
    m_fAutoRestart2Time = timeNow;
    StartupBOINCCore();
    return true;
}


bool CBOINCClientManager::IsSystemBooting() {
    bool bReturnValue = false;
#if   defined(__WXMSW__)
    if (GetTickCount() < (1000*60*5)) bReturnValue = true;  // If system has been up for less than 5 minutes
#elif defined(__WXMAC__)
    if (getTimeSinceBoot() < (120)) bReturnValue = true;    // If system has been up for less than 2 minutes
#endif
    return bReturnValue;
}


int CBOINCClientManager::IsBOINCConfiguredAsDaemon() {
    int bReturnValue = 0;
#if   defined(__WXMSW__)
    if (is_daemon_installed()) bReturnValue = 1;
#elif defined(__WXMAC__)
    if (boinc_file_exists("/Library/LaunchDaemons/edu.berkeley.boinc.plist")) {
        bReturnValue = NewStyleDaemon;                      // New-style daemon uses launchd
    }
    if (boinc_file_exists("/Library/StartupItems/boinc/boinc") ) {
        bReturnValue = OldStyleDaemon;                      // Old-style daemon uses StartupItem
    }
#else
    FILE *f = popen("systemctl -q is-active boinc-client.service", "r");
    if (f) {
        if (pclose(f) == 0) {
            bReturnValue = 1;
        }
    }
#endif
    return bReturnValue;
}


bool CBOINCClientManager::IsBOINCCoreRunning() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Function Begin"));
    bool running = false;

    PROC_MAP pm;
    int retval;

#ifndef __WXMSW__
    if (m_lBOINCCoreProcessId) {
        // Prevent client from being a zombie
        if (waitpid(m_lBOINCCoreProcessId, 0, WNOHANG) == m_lBOINCCoreProcessId) {
            m_lBOINCCoreProcessId = 0;
        }
    }
#endif

    // Look for BOINC Client in list of all running processes
    retval = procinfo_setup(pm);
    if (retval) return false;     // Should never happen

    PROC_MAP::iterator i;
    for (i=pm.begin(); i!=pm.end(); ++i) {
        PROCINFO& pi = i->second;
#ifdef __WXMSW__
        if (!strcmp(pi.command, "boinc.exe"))
#else
        if (!strcmp(pi.command, "boinc"))
#endif
        {
            running = true;
            break;
        }
        if (!strcmp(pi.command, "boinc_client")) {
            running = true;
            break;
        }
    }

    wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Returning '%d'"), (int)running);
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::IsBOINCCoreRunning - Function End"));
    return running;
}


bool CBOINCClientManager::StartupBOINCCore() {
    wxLogTrace(wxT("Function Start/End"), wxT("CMainDocument::StartupBOINCCore - Function Begin"));

    bool                bReturnValue = false;
#ifndef __WXMAC__
    wxString            strExecute = wxEmptyString;
#endif

    if (IsBOINCCoreRunning()) return true;

#if defined(__WXMSW__)
    wxString     strDataDir = wxEmptyString;
    const char*  pszExecute = NULL;
    const char*  pszDataDirectory = NULL;

    if (IsBOINCConfiguredAsDaemon()) {
        start_daemon_via_daemonctrl();

        m_bBOINCStartedByManager = true;
        bReturnValue = IsBOINCCoreRunning();
    } else {

        // Append boinc.exe to the end of the strExecute string and get ready to rock
        strExecute.Printf(
            wxT("\"%sboinc.exe\" --redirectio --launched_by_manager %s"),
            wxGetApp().GetRootDirectory().c_str(),
            wxGetApp().GetArguments().c_str()
        );

        PROCESS_INFORMATION pi;
        STARTUPINFOA        si;
        BOOL                bProcessStarted;

        memset(&pi, 0, sizeof(pi));
        memset(&si, 0, sizeof(si));

        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        pszExecute = (const char*)strExecute.mb_str();
        if (wxGetApp().GetDataDirectory().empty()) {
            pszDataDirectory = NULL;
        } else {
            strDataDir = wxGetApp().GetDataDirectory();
            pszDataDirectory = (const char*)strDataDir.mb_str();
        }

        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - pszExecute '%s'\n"), pszExecute);
        wxLogTrace(wxT("Function Status"), wxT("CMainDocument::StartupBOINCCore - pszDataDirectory '%s'\n"), pszDataDirectory);

        bProcessStarted = CreateProcessA(
            NULL,
            (LPSTR)pszExecute,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW,
            NULL,
            (LPSTR)pszDataDirectory,
            &si,
            &pi
        );

        if (bProcessStarted) {
            m_lBOINCCoreProcessId = pi.dwProcessId;
            m_hBOINCCoreProcess = pi.hProcess;
        }
    }

#elif defined(__WXMAC__)

    char buf[1024];
    char *argv[5];

    if (IsBOINCConfiguredAsDaemon() == NewStyleDaemon) {
        system ("launchctl load /Library/LaunchDaemons/edu.berkeley.boinc.plist");
        system ("launchctl start edu.berkeley.boinc");
        for (int i=0; i<100; i++) {     // Wait up to 1 seccond in 10 ms increments
            boinc_sleep(0.01);
            if (IsBOINCCoreRunning()) {
                bReturnValue = true;
                break;
            }
        }
    } else {

        // Get the full path to core client inside this application's bundle
        getPathToThisApp(buf, sizeof(buf));
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
            buf, argv[3] ? 4 : 3, argv, m_lBOINCCoreProcessId
        );
#endif
    }

#else   // Unix based systems
    // if BOINC client is installed as a systemd service - skip starting it
    if (!IsBOINCConfiguredAsDaemon()) {
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
    }
#endif

    if (0 != m_lBOINCCoreProcessId) {
        m_bBOINCStartedByManager = true;
        // Allow time for daemon to start up so we don't keep relaunching it
        for (int i=0; i<100; i++) {     // Wait up to 1 seccond in 10 ms increments
            boinc_sleep(0.01);
            if (IsBOINCCoreRunning()) {
                bReturnValue = true;
                break;
            }
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
        kill_process(m_hBOINCCoreProcess);
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
    PROC_MAP pm;
    int retval;

    if (m_lBOINCCoreProcessId) {
        kill_process(m_lBOINCCoreProcessId);
        return;
    }

    retval = procinfo_setup(pm);
	if (retval) return;     // Should never happen

    PROC_MAP::iterator i;
    for (i=pm.begin(); i!=pm.end(); ++i) {
        PROCINFO& procinfo = i->second;
        if (!strcmp(procinfo.command, "boinc")) {
            kill_process(procinfo.id);
            break;
        }
    }
}
#endif


void CBOINCClientManager::ShutdownBOINCCore(bool ShuttingDownManager) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::ShutdownBOINCCore - Function Begin"));

    CMainDocument*      pDoc = wxGetApp().GetDocument();
    wxInt32             iCount = 0;
    bool                bClientQuit = false;
    wxString            strConnectedCompter = wxEmptyString;
    wxString            strPassword = wxEmptyString;
    double              startTime = 0;
    wxDateTime          zeroTime = wxDateTime((time_t)0);
    wxDateTime          rpcCompletionTime = zeroTime;
    ASYNC_RPC_REQUEST   request;
    int                 quit_result;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

#ifdef __WXMAC__
    // Mac Manager shuts down client only if Manager started client
    if (!m_bBOINCStartedByManager) return;
#endif

#ifdef __WXGTK__
    // Linux Manager shuts down client only if Manager started client or client is not a daemon
    if (!m_bBOINCStartedByManager && IsBOINCConfiguredAsDaemon()) return;
#endif

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
                if (ShuttingDownManager) {
                    // Set event filtering to allow RPC completion
                    // events but not events which start new RPCs
                    wxGetApp().SetEventFiltering(true);
                }
                quit_result = -1;
                request.clear();
                request.which_rpc = RPC_QUIT;
                request.rpcType = RPC_TYPE_ASYNC_NO_REFRESH;
                request.completionTime = &rpcCompletionTime;
                request.resultPtr = &quit_result;
                pDoc->RequestRPC(request);  // Issue an asynchronous Quit RPC

                // Client needs time to shut down project applications, so don't wait
                // for it to shut down; assume it will exit OK if Quit RPC succeeds.
                startTime = dtime();
                while ((dtime() - startTime) < 10.0) {  // Allow 10 seconds
                    boinc_sleep(0.25);          // Check 4 times per second
                    wxSafeYield(NULL, true);    // To allow handling RPC completion events
                    if (!bClientQuit && (rpcCompletionTime != zeroTime)) {
                        // If Quit RPC finished, check its returned value
                        if (quit_result) {
                            break;  // Quit RPC returned an error
                        }
                        wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - Application Exit Detected"));
                        bClientQuit = true;
                        break;
                    }
                    wxLogTrace(wxT("Function Status"), wxT("CBOINCClientManager::ShutdownBOINCCore - Application Exit NOT Detected, Sleeping..."));
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

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCClientManager::ShutdownBOINCCore - Function End"));
}


BEGIN_EVENT_TABLE(ClientCrashDlg, wxDialog)
    EVT_BUTTON(wxID_HELP, ClientCrashDlg::OnHelp)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ClientCrashDlg, wxDialog)

ClientCrashDlg::ClientCrashDlg(double timeDiff) : wxDialog( NULL, wxID_ANY, wxT(""), wxDefaultPosition ) {
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;
    int                 minutes = wxMax((int)((timeDiff + 59.) / 60.), 2);
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Unexpected Exit"),
        pSkinAdvanced->GetApplicationName().c_str()
    );
    SetTitle(strDialogTitle.c_str());

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2st %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("The %s client has exited unexpectedly 3 times within the last %d minutes.\nWould you like to restart it again?"),
        pSkinAdvanced->GetApplicationShortName().c_str(),
        minutes
    );

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );

    icon_text->Add( CreateTextSizer( strDialogMessage ), 0, wxALIGN_CENTER | wxLEFT, 10 );
    topsizer->Add( icon_text, 1, wxCENTER | wxLEFT|wxRIGHT|wxTOP, 10 );

    wxStdDialogButtonSizer *sizerBtn = CreateStdDialogButtonSizer(wxYES | wxNO | wxHELP);
    SetEscapeId(wxID_NO);   // Changes return value of NO button to wxID_CANCEL

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


void ClientCrashDlg::OnHelp(wxCommandEvent& WXUNUSED(eventUnused)) {
    wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

    wxString wxurl;
    wxurl.Printf(
        wxT("%s?target=crash_detection&version=%s&controlid=%d"),
        strURL.c_str(),
        wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
        ID_HELPBOINC
    );
    wxLaunchDefaultBrowser(wxurl);
}
