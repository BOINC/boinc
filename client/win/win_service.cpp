// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//		Craig Link, Microsoft Corp., Service Sample Template
//

#if defined(_WIN32) && defined(_CONSOLE)

#include "stdafx.h"

#include "diagnostics.h"
#include "win_service.h"
#include "util.h"


// internal variables
SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
TCHAR                   szErr[1024];


// define the execution engine start point
extern int boinc_main_loop(int argc, char** argv);
extern void quit_client(int a);
extern void susp_client(int a);
extern void resume_client(int a);


// Define API's that are going to be used through LoadLibrary calls.
typedef WINADVAPI BOOL (WINAPI *PROCCHANGESERVICECONFIG2)(SC_HANDLE, DWORD, LPCVOID);


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
void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
    TCHAR   szPath[MAX_PATH-1];


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


    // change the current directory to the boinc install directory
    if (!GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR))))
		goto cleanup;
		
    TCHAR *pszProg = strrchr(szPath, '\\');
    if (pszProg) {
        szPath[pszProg - szPath + 1] = 0;
        SetCurrentDirectory(szPath);
    }

    dwErr = boinc_main_loop(dwArgc, lpszArgv);


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
            ReportStatus(SERVICE_STOP_PENDING, ERROR_SUCCESS, 10000);
			quit_client(NULL);
            ReportStatus(SERVICE_STOPPED, ERROR_SUCCESS, 10000);
            return;

        // Pause the service.
        //
		case SERVICE_CONTROL_PAUSE:
            ReportStatus(SERVICE_PAUSE_PENDING, ERROR_SUCCESS, 10000);
			susp_client(NULL);
            ReportStatus(SERVICE_PAUSED, ERROR_SUCCESS, 10000);
            return;

        // Continue the service.
        //
		case SERVICE_CONTROL_CONTINUE:
            ReportStatus(SERVICE_CONTINUE_PENDING, ERROR_SUCCESS, 10000);
			resume_client(NULL);
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
    if (!(fResult = SetServiceStatus( sshStatusHandle, &ssStatus))) {
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

//
//  FUNCTION: CmdInstallService()
//
//  PURPOSE: Installs the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CmdInstallService()
{
    SC_HANDLE                       schService;
    SC_HANDLE                       schSCManager;
    HINSTANCE                       hinstAdvAPI32; 
    PROCCHANGESERVICECONFIG2        ProcChangeServiceConfig2; 

    TCHAR szPath[512];

    if ( GetModuleFileName( NULL, szPath, 512 ) == 0 )
    {
        _tprintf(TEXT("Unable to install %s - %s\n"), TEXT(SZSERVICEDISPLAYNAME), windows_error_string(szErr, (sizeof(szErr)/sizeof(TCHAR))));
        return;
    }

	// Append the -win_service option to the commandline.
	strcat(szPath, TEXT(" -win_service"));

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                        );

	if ( schSCManager )
    {
		// Attempt to install the service with the Network Service account.
        schService = CreateService(
            schSCManager,               // SCManager database
            TEXT(SZSERVICENAME),        // name of service
            TEXT(SZSERVICEDISPLAYNAME), // name to display
            SERVICE_ALL_ACCESS,         // desired access
            SERVICE_WIN32_OWN_PROCESS,  // service type
            SERVICE_AUTO_START,         // start type
            SERVICE_ERROR_NORMAL,       // error control type
            szPath,                     // service's binary
            NULL,                       // no load ordering group
            NULL,                       // no tag identifier
            NULL,                       // dependencies
            TEXT("NT AUTHORITY\\NetworkService"), // NetworkService account
            NULL);                      // no password

		// If we couldn't install the service with the Network Service account, switch
		//	to LocalSystem.
		if ( (schService == NULL) && (GetLastError() == ERROR_INVALID_SERVICE_ACCOUNT) )
		{
			schService = CreateService(
				schSCManager,               // SCManager database
				TEXT(SZSERVICENAME),        // name of service
				TEXT(SZSERVICEDISPLAYNAME), // name to display
				SERVICE_ALL_ACCESS,         // desired access
				SERVICE_WIN32_OWN_PROCESS,  // service type
				SERVICE_AUTO_START,         // start type
				SERVICE_ERROR_NORMAL,       // error control type
				szPath,                     // service's binary
				NULL,                       // no load ordering group
				NULL,                       // no tag identifier
				NULL,                       // dependencies
				NULL,                       // LocalSystem account
				NULL);                      // no password
		}

		if ( schService )
        {
            _tprintf(TEXT("%s installed.\n"), TEXT(SZSERVICEDISPLAYNAME) );

			SERVICE_DESCRIPTION sdDescription;
			sdDescription.lpDescription = TEXT(SZSERVICEDESCRIPTION);

            hinstAdvAPI32 = LoadLibrary("ADVAPI32"); 
         
            // If the handle is valid, try to get the function address.
            if ( NULL != hinstAdvAPI32 ) 
            { 
#ifdef _UNICODE
                ProcChangeServiceConfig2 = (PROCCHANGESERVICECONFIG2)GetProcAddress(
                    hinstAdvAPI32,
                    TEXT("ChangeServiceConfig2W")
                    ); 
#else
                ProcChangeServiceConfig2 = (PROCCHANGESERVICECONFIG2)GetProcAddress(
                    hinstAdvAPI32,
                    TEXT("ChangeServiceConfig2A")
                    ); 
#endif

                // If the function address is valid, call the function.
                if ( NULL != ProcChangeServiceConfig2 ) 
                {
			        if ( ProcChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sdDescription) ){
	                    _tprintf(TEXT("%s service description installed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
			        } else {
	                    _tprintf(TEXT("ChangeServiceConfig2 failed - %s\n"), windows_error_string(szErr, (sizeof(szErr)/sizeof(TCHAR))));
			        }
                }

                // Free the DLL module
                FreeLibrary( hinstAdvAPI32 ); 
            } 

            CloseServiceHandle(schService);
        }
        else
        {
            _tprintf(TEXT("CreateService failed - %s\n"), windows_error_string(szErr, (sizeof(szErr)/sizeof(TCHAR))));
        }

        CloseServiceHandle(schSCManager);
    }
    else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), windows_error_string(szErr, (sizeof(szErr)/sizeof(TCHAR))));
}



//
//  FUNCTION: CmdUninstallService()
//
//  PURPOSE: Stops and removes the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CmdUninstallService()
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                        );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);

        if (schService)
        {
            // try to stop the service
            if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                _tprintf(TEXT("Stopping %s."), TEXT(SZSERVICEDISPLAYNAME));
                Sleep( 1000 );

                while( QueryServiceStatus( schService, &ssStatus ) )
                {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                    {
                        _tprintf(TEXT("."));
                        Sleep( 1000 );
                    }
                    else
                        break;
                }

                if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                    _tprintf(TEXT("\n%s stopped.\n"), TEXT(SZSERVICEDISPLAYNAME) );
                else
                    _tprintf(TEXT("\n%s failed to stop.\n"), TEXT(SZSERVICEDISPLAYNAME) );

            }

            // now remove the service
            if( DeleteService(schService) )
                _tprintf(TEXT("%s removed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
            else
                _tprintf(TEXT("DeleteService failed - %s\n"), windows_error_string(szErr, (sizeof(szErr)/sizeof(TCHAR))));


            CloseServiceHandle(schService);
        }
        else
            _tprintf(TEXT("OpenService failed - %s\n"), windows_error_string(szErr, (sizeof(szErr)/sizeof(TCHAR))));

        CloseServiceHandle(schSCManager);
    }
    else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), windows_error_string(szErr, (sizeof(szErr)/sizeof(TCHAR))));
}


#endif