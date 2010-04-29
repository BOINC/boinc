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

#include "boinc_win.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "url.h"
#include "util.h"
#include "win_util.h"
#include "prefs.h"
#include "filesys.h"
#include "network.h"

#include "client_state.h"
#include "log_flags.h"
#include "client_msgs.h"
#include "http_curl.h"
#include "sandbox.h"
#include "main.h"
#include "cs_proxy.h"

#include "sysmon_win.h"


static HANDLE g_hWindowsMonitorSystemThread = NULL;
static DWORD g_WindowsMonitorSystemThreadID = NULL;
static HWND g_hWndWindowsMonitorSystem = NULL;


// The following 3 functions are called in a separate thread,
// so we can't do anything directly.
// Set flags telling the main thread what to do.
//

// Quit operations
static void quit_client() {
    gstate.requested_exit = true;
    while (1) {
        boinc_sleep(1.0);
        if (gstate.cleanup_completed) break;
    }
}

// Suspend client operations
static void suspend_client(bool wait) {
    gstate.requested_suspend = true;
    if (wait) {
        while (1) {
            boinc_sleep(1.0);
            if (!gstate.active_tasks.is_task_executing()) break;
        }
    }
}

// Resume client operations
static void resume_client() {
    gstate.requested_resume = true;
}

// Process console messages sent by the system
static BOOL WINAPI console_control_handler( DWORD dwCtrlType ){
    BOOL bReturnStatus = FALSE;
    BOINCTRACE("***** Console Event Detected *****\n");
    switch( dwCtrlType ){
    case CTRL_LOGOFF_EVENT:
        BOINCTRACE("Event: CTRL-LOGOFF Event\n");
        if (!gstate.executing_as_daemon) {
           quit_client();
        }
        bReturnStatus =  TRUE;
        break;
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        BOINCTRACE("Event: CTRL-C or CTRL-BREAK Event\n");
        quit_client();
        bReturnStatus =  TRUE;
        break;
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        BOINCTRACE("Event: CTRL-CLOSE or CTRL-SHUTDOWN Event\n");
        quit_client();
        break;
    }
    return bReturnStatus;
}

// Detect any proxy configuration settings automatically.
static void windows_detect_autoproxy_settings() {
    if (log_flags.proxy_debug) {
        msg_printf(NULL, MSG_INFO, "[proxy] automatic proxy check in progress");
    }

    HMODULE hModWinHttp = LoadLibrary("winhttp.dll");
    if (!hModWinHttp) {
        return;
    }
    pfnWinHttpOpen pWinHttpOpen =
        (pfnWinHttpOpen)GetProcAddress(hModWinHttp, "WinHttpOpen");
    if (!pWinHttpOpen) {
        return;
    }
    pfnWinHttpCloseHandle pWinHttpCloseHandle =
        (pfnWinHttpCloseHandle)(GetProcAddress(hModWinHttp, "WinHttpCloseHandle"));
    if (!pWinHttpCloseHandle) {
        return;
    }
    pfnWinHttpGetProxyForUrl pWinHttpGetProxyForUrl =
        (pfnWinHttpGetProxyForUrl)(GetProcAddress(hModWinHttp, "WinHttpGetProxyForUrl"));
    if (!pWinHttpGetProxyForUrl) {
        return;
    }

    HINTERNET                 hWinHttp = NULL;
    WINHTTP_AUTOPROXY_OPTIONS autoproxy_options;
    WINHTTP_PROXY_INFO        proxy_info;
    PARSED_URL purl;
    std::wstring              network_test_url;
    size_t                    pos;


    memset(&autoproxy_options, 0, sizeof(autoproxy_options));
    memset(&proxy_info, 0, sizeof(proxy_info));

    autoproxy_options.dwFlags =
        WINHTTP_AUTOPROXY_AUTO_DETECT;
    autoproxy_options.dwAutoDetectFlags =
        WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    autoproxy_options.fAutoLogonIfChallenged = TRUE;

    network_test_url = A2W(config.network_test_url).c_str();

    hWinHttp = pWinHttpOpen(
        L"BOINC client",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        NULL
    );

    if (pWinHttpGetProxyForUrl(hWinHttp, network_test_url.c_str(), &autoproxy_options, &proxy_info)) {

        if (log_flags.proxy_debug) {
            msg_printf(NULL, MSG_INFO, "[proxy] successfully executed proxy check", hWinHttp);
        }

        // Apparently there are some conditions where WinHttpGetProxyForUrl can return
        //   success but where proxy_info.lpszProxy is null.  Maybe related to UPNP?
        //
        // For the time being check to see if proxy_info.lpszProxy is non-null.
        //
        if (proxy_info.lpszProxy) {
            std::string proxy(W2A(std::wstring(proxy_info.lpszProxy)));
            std::string new_proxy;

            if (log_flags.proxy_debug) {
                msg_printf(NULL, MSG_INFO, "[proxy] proxy list '%s'", proxy.c_str());
            }

            if (!proxy.empty()) {
                // Trim string if more than one proxy is defined
                // proxy list is defined as:
                //   ([<scheme>=][<scheme>"://"]<server>[":"<port>])
                
                // Find and erase first delimeter type.
                pos = proxy.find(';');
                if (pos != -1 ) {
                    new_proxy = proxy.erase(pos);
                    proxy = new_proxy;
                }

                // Find and erase second delimeter type.
                pos = proxy.find(' ');
                if (pos != -1 ) {
                    new_proxy = proxy.erase(pos);
                    proxy = new_proxy;
                }

                // Parse the remaining url
                parse_url(proxy.c_str(), purl);

                // Store the results for future use.
                working_proxy_info.autodetect_protocol = purl.protocol;
                strcpy(working_proxy_info.autodetect_server_name, purl.host);
                working_proxy_info.autodetect_port = purl.port;

                if (log_flags.proxy_debug) {
                    msg_printf(NULL, MSG_INFO,
                        "[proxy] automatic proxy detected %s:%d",
                        purl.host, purl.port
                    );
                }
            }
        }

        // Clean up
        if (proxy_info.lpszProxy) GlobalFree(proxy_info.lpszProxy);
        if (proxy_info.lpszProxyBypass) GlobalFree(proxy_info.lpszProxyBypass);
    } else {
        // We can get here if the user is switching from a network that
        // requires a proxy to one that does not require a proxy.
        working_proxy_info.autodetect_protocol = 0;
        strcpy(working_proxy_info.autodetect_server_name, "");
        working_proxy_info.autodetect_port = 0;
        if (log_flags.proxy_debug) {
            msg_printf(NULL, MSG_INFO, "[proxy] no automatic proxy detected");
        }
    }
    if (hWinHttp) pWinHttpCloseHandle(hWinHttp);
    FreeLibrary(hModWinHttp);
    if (log_flags.proxy_debug) {
        msg_printf(NULL, MSG_INFO, "[proxy] automatic proxy check finished");
    }
}

// Trap events on Windows so we can clean ourselves up.
static LRESULT CALLBACK WindowsMonitorSystemWndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
    switch(uMsg) {
        // Process Timer Events
        case WM_TIMER:
            switch(wParam) {

                // System Monitor 1 second timer
                case 1:
                    if (working_proxy_info.need_autodetect_proxy_settings) {
                        working_proxy_info.have_autodetect_proxy_settings = false;
                        windows_detect_autoproxy_settings();
                        working_proxy_info.need_autodetect_proxy_settings = false;
                        working_proxy_info.have_autodetect_proxy_settings = true;
                    }
                default:
                    break;
            }
            break;

        // On Windows power events are broadcast via the WM_POWERBROADCAST
        //   window message.  It has the following parameters:
        //     PBT_APMQUERYSUSPEND
        //     PBT_APMQUERYSUSPENDFAILED
        //     PBT_APMSUSPEND
        //     PBT_APMRESUMECRITICAL
        //     PBT_APMRESUMESUSPEND
        //     PBT_APMBATTERYLOW
        //     PBT_APMPOWERSTATUSCHANGE
        //     PBT_APMOEMEVENT
        //     PBT_APMRESUMEAUTOMATIC 
        case WM_POWERBROADCAST:
            switch(wParam) {
                // System is preparing to suspend.  This is valid on
                //   Windows versions older than Vista
                case PBT_APMQUERYSUSPEND:
                    return TRUE;
                    break;

                // System is resuming from a failed request to suspend
                //   activity.  This is only valid on Windows versions
                //   older than Vista
                case PBT_APMQUERYSUSPENDFAILED:
                    resume_client();
                    break;

                // System is critically low on battery power.  This is
                //   only valid on Windows versions older than Vista
                case PBT_APMBATTERYLOW:
                    msg_printf(NULL, MSG_INFO, "Critical battery alarm, Windows is suspending operations");
                    suspend_client(true);
                    break;

                // System is suspending
                case PBT_APMSUSPEND:
                    msg_printf(NULL, MSG_INFO, "Windows is suspending operations");
                    suspend_client(true);
                    break;

                // System is resuming from a normal power event
                case PBT_APMRESUMESUSPEND:
                    msg_printf(NULL, MSG_INFO, "Windows is resuming operations");

                    // Check for a proxy
                    working_proxy_info.need_autodetect_proxy_settings = true;

                    resume_client();
                    break;
            }
            break;
        default:
            break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

// Create a thread to monitor system events
static DWORD WINAPI WindowsMonitorSystemThread( LPVOID  ) {
    WNDCLASS wc;
    MSG msg;

    wc.style         = CS_GLOBALCLASS;
    wc.lpfnWndProc   = (WNDPROC)WindowsMonitorSystemWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = NULL;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
	wc.lpszClassName = "BOINCWindowsMonitorSystem";

    if (!RegisterClass(&wc)) {
        log_message_error("Failed to register the WindowsMonitorSystem window class.");
        return 1;
    }

    /* Create an invisible window */
    g_hWndWindowsMonitorSystem = CreateWindow(
        wc.lpszClassName,
		"BOINC Monitor System",
        WS_OVERLAPPEDWINDOW & ~WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        NULL,
        NULL);

    if (!g_hWndWindowsMonitorSystem) {
        log_message_error("Failed to create the WindowsMonitorSystem window.");
        return 0;
    }

    if (!SetTimer(g_hWndWindowsMonitorSystem, 1, 1000, NULL)) {
        log_message_error("Failed to create the WindowsMonitorSystem timer.");
    }

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// Setup the client software to monitor various system events
int initialize_system_monitor(int /*argc*/, char** /*argv*/) {

    // Windows: install console controls
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)console_control_handler, TRUE)){
        log_message_error("Failed to register the console control handler.");
        return ERR_IO;
    }

    // Create a window to receive system events that are
    //   not taken care of by the console APIs.  The console
    //   APIs haven't been updated to handle various power states.
    g_hWindowsMonitorSystemThread = CreateThread(
        NULL,
        0,
        WindowsMonitorSystemThread,
        NULL,
        0,
        &g_WindowsMonitorSystemThreadID);

    if (!g_hWindowsMonitorSystemThread) {
        g_hWindowsMonitorSystemThread = NULL;
        g_WindowsMonitorSystemThreadID = NULL;
        g_hWndWindowsMonitorSystem = NULL;
    }

    return 0;
}

// Cleanup the system event monitor
int cleanup_system_monitor() {

    KillTimer(g_hWndWindowsMonitorSystem, 1);

    if (g_WindowsMonitorSystemThreadID) {
	    PostThreadMessage(g_WindowsMonitorSystemThreadID, WM_QUIT, 0, 0);
        WaitForSingleObject(g_hWindowsMonitorSystemThread, 10000);
    }

    if (g_hWindowsMonitorSystemThread) {
        CloseHandle(g_hWindowsMonitorSystemThread);
        g_hWindowsMonitorSystemThread = NULL;
        g_WindowsMonitorSystemThreadID = NULL;
        g_hWndWindowsMonitorSystem = NULL;
    }

    return 0;
}

// internal variables for managing the service
SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
TCHAR                   szErr[1024];

SERVICE_TABLE_ENTRY     service_dispatch_table[] = {
    { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)BOINCServiceMain },
    { NULL, NULL }
};

// Inform the service control manager that the service is about to
// start.
int initialize_service_dispatcher(int /*argc*/, char** /*argv*/) {
    fprintf(stdout, "\nStartServiceCtrlDispatcher being called.\n");
    fprintf(stdout, "This may take several seconds.  Please wait.\n");

    if (!StartServiceCtrlDispatcher(service_dispatch_table)) {
        log_message_error("StartServiceCtrlDispatcher failed.");
        return ERR_IO;
    }
    return 0;
}

//
//  FUNCTION: BOINCServiceMain
//
//  PURPOSE: To perform actual initialization of the service
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This routine performs the service initialization and then calls
//    the user defined main() routine to perform majority
//    of the work.
//
void WINAPI BOINCServiceMain(DWORD /*dwArgc*/, LPTSTR * /*lpszArgv*/)
{
    // SERVICE_STATUS members that don't change in example
    //
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwControlsAccepted = SERVICE_ACCEPTED_ACTIONS;
    ssStatus.dwServiceSpecificExitCode = 0;


    // register our service control handler:
    //
    sshStatusHandle = RegisterServiceCtrlHandler( TEXT(SZSERVICENAME), BOINCServiceCtrl);
    if (!sshStatusHandle)
        goto cleanup;

    if (!ReportStatus(
        SERVICE_RUNNING,       // service state
        ERROR_SUCCESS,         // exit code
        0))                    // wait hint
        goto cleanup;

    dwErr = boinc_main_loop();

cleanup:

    // try to report the stopped status to the service control manager.
    //
    if (sshStatusHandle)
        (VOID)ReportStatus(
            SERVICE_STOPPED,
            dwErr,
            0);
}


//
//  FUNCTION: BOINCServiceCtrl
//
//  PURPOSE: This function is called by the SCM whenever
//           ControlService() is called on this service.
//
//  PARAMETERS:
//    dwCtrlCode - type of control requested
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID WINAPI BOINCServiceCtrl(DWORD dwCtrlCode)
{
    // Handle the requested control code.
    //
    switch(dwCtrlCode)
    {
        // Stop the service.
        //
        // SERVICE_STOP_PENDING should be reported before
        // setting the Stop Event - hServerStopEvent - in
        // ServiceStop().  This avoids a race condition
        // which may result in a 1053 - The Service did not respond...
        // error.
        case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
            ReportStatus(SERVICE_STOP_PENDING, ERROR_SUCCESS, 30000);
			quit_client();
            return;

        // Pause the service.
        //
		case SERVICE_CONTROL_PAUSE:
            ReportStatus(SERVICE_PAUSE_PENDING, ERROR_SUCCESS, 10000);
			suspend_client(true);
            ReportStatus(SERVICE_PAUSED, ERROR_SUCCESS, 10000);
            return;

        // Continue the service.
        //
		case SERVICE_CONTROL_CONTINUE:
            ReportStatus(SERVICE_CONTINUE_PENDING, ERROR_SUCCESS, 10000);
			resume_client();
            ReportStatus(SERVICE_RUNNING, ERROR_SUCCESS, 10000);
            return;

		// Update the service status.
        //
        case SERVICE_CONTROL_INTERROGATE:
            break;

        // invalid control code
        //
        default:
            break;

    }

    ReportStatus(ssStatus.dwCurrentState, ERROR_SUCCESS, 1000);
}


//
//  FUNCTION: ReportStatus()
//
//  PURPOSE: Sets the current status of the service and
//           reports it to the Service Control Manager
//
//  PARAMETERS:
//    dwCurrentState - the state of the service
//    dwWin32ExitCode - error code to report
//    dwWaitHint - worst case estimate to next checkpoint
//
//  RETURN VALUE:
//    TRUE  - success
//    FALSE - failure
//
//  COMMENTS:
//
BOOL ReportStatus(DWORD dwCurrentState,
                  DWORD dwWin32ExitCode,
                  DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;

    if (dwCurrentState == SERVICE_START_PENDING)
        ssStatus.dwControlsAccepted = 0;
    else
        ssStatus.dwControlsAccepted = SERVICE_ACCEPTED_ACTIONS;

    ssStatus.dwCurrentState = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwWaitHint = dwWaitHint;

    if ( ( dwCurrentState == SERVICE_RUNNING ) ||
            ( dwCurrentState == SERVICE_STOPPED ) )
        ssStatus.dwCheckPoint = 0;
    else
        ssStatus.dwCheckPoint = dwCheckPoint++;


    // Report the status of the service to the service control manager.
    //
	fResult = SetServiceStatus( sshStatusHandle, &ssStatus);
    if (!fResult) {
        LogEventErrorMessage(TEXT("SetServiceStatus"));
    }
    return fResult;
}



//
//  FUNCTION: LogEventErrorMessage(LPTSTR lpszMsg)
//
//  PURPOSE: Allows any thread to log an error message
//
//  PARAMETERS:
//    lpszMsg - text for message
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID LogEventErrorMessage(LPTSTR lpszMsg)
{
    TCHAR   szMsg[1024];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];

    dwErr = GetLastError();

    // Use event logging to log the error.
    //
    hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));

    _stprintf(szMsg, TEXT("%s error: %d"), TEXT(SZSERVICENAME), dwErr);
    lpszStrings[0] = szMsg;
    lpszStrings[1] = lpszMsg;

    if (hEventSource != NULL) {
        ReportEvent(hEventSource, // handle of event source
            EVENTLOG_ERROR_TYPE,  // event type
            0,                    // event category
            1,                    // event ID
            NULL,                 // current user's SID
            2,                    // strings in lpszStrings
            0,                    // no bytes of raw data
            (LPCSTR*)lpszStrings, // array of error strings
            NULL);                // no raw data

        (VOID) DeregisterEventSource(hEventSource);
    }
 }


//
//  FUNCTION: LogEventWarningMessage(LPTSTR lpszMsg)
//
//  PURPOSE: Allows any thread to log an warning message
//
//  PARAMETERS:
//    lpszMsg - text for message
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID LogEventWarningMessage(LPTSTR lpszMsg)
{
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];

    // Use event logging to log the error.
    //
    hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));

    lpszStrings[0] = lpszMsg;
    lpszStrings[1] = '\0';

    if (hEventSource != NULL) {
        ReportEvent(hEventSource, // handle of event source
            EVENTLOG_WARNING_TYPE,// event type
            0,                    // event category
            1,                    // event ID
            NULL,                 // current user's SID
            2,                    // strings in lpszStrings
            0,                    // no bytes of raw data
            (LPCSTR*)lpszStrings, // array of error strings
            NULL);                // no raw data

        (VOID) DeregisterEventSource(hEventSource);
    }
}


//
//  FUNCTION: LogEventInfoMessage(LPTSTR lpszMsg)
//
//  PURPOSE: Allows any thread to log an info message
//
//  PARAMETERS:
//    lpszMsg - text for message
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID LogEventInfoMessage(LPTSTR lpszMsg)
{
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];

    // Use event logging to log the error.
    //
    hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));

    lpszStrings[0] = lpszMsg;
    lpszStrings[1] = '\0';

    if (hEventSource != NULL) {
        ReportEvent(hEventSource, // handle of event source
            EVENTLOG_INFORMATION_TYPE,// event type
            0,                    // event category
            1,                    // event ID
            NULL,                 // current user's SID
            2,                    // strings in lpszStrings
            0,                    // no bytes of raw data
            (LPCSTR*)lpszStrings, // array of error strings
            NULL);                // no raw data

        (VOID) DeregisterEventSource(hEventSource);
    }
}

