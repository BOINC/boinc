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

// Service Control Manager Routines
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
BOOL		ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
VOID		LogEventErrorMessage(LPTSTR lpszMsg);
VOID		LogEventWarningMessage(LPTSTR lpszMsg);
VOID		LogEventInfoMessage(LPTSTR lpszMsg);

// Service Maintenance Routines
VOID		CmdInstallService();
VOID		CmdUninstallService();


#ifdef __cplusplus
}
#endif

#endif

#endif
