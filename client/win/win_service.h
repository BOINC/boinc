// Contributor(s):
//		Craig Link, Microsoft Corp., Service Sample Template
//

#ifndef _WIN_SERVICE_H
#define _WIN_SERVICE_H

#if defined(_WIN32) && defined(_CONSOLE)

#ifdef __cplusplus
extern "C" {
#endif


// internal name of the service
#define SZSERVICENAME        "BOINC"

// displayed name of the service
#define SZSERVICEDISPLAYNAME "BOINC"

// displayed description of the service
#define SZSERVICEDESCRIPTION "Berkeley Open Infrastructure for Network Computing"

// Service Accepted Actions
#define SERVICE_ACCEPTED_ACTIONS  ( \
    SERVICE_ACCEPT_STOP | \
    SERVICE_ACCEPT_PAUSE_CONTINUE | \
    SERVICE_ACCEPT_SHUTDOWN ) 

// Service Control Manager Routines
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
BOOL		ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
VOID		LogEventErrorMessage(LPTSTR lpszMsg);
VOID		LogEventWarningMessage(LPTSTR lpszMsg);
VOID		LogEventInfoMessage(LPTSTR lpszMsg);

#ifdef __cplusplus
}
#endif

#endif

#endif
