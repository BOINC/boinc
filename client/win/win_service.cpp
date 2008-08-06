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

#if defined(_WIN32) && defined(_CONSOLE)

#include "diagnostics.h"
#include "util.h"
#include "main.h"
#include "win_service.h"


// internal variables
SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
TCHAR                   szErr[1024];


//
//  FUNCTION: service_main
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
void WINAPI service_main(DWORD /*dwArgc*/, LPTSTR * /*lpszArgv*/)
{
    // SERVICE_STATUS members that don't change in example
    //
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwControlsAccepted = SERVICE_ACCEPTED_ACTIONS;
    ssStatus.dwServiceSpecificExitCode = 0;


    // register our service control handler:
    //
    sshStatusHandle = RegisterServiceCtrlHandler( TEXT(SZSERVICENAME), service_ctrl);
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
//  FUNCTION: service_ctrl
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
VOID WINAPI service_ctrl(DWORD dwCtrlCode)
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
			suspend_client();
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


#endif


const char *BOINC_RCSID_ad2dd5eef4 = "$Id$";
