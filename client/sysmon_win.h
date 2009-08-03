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

#ifndef _SYSMON_WIN_H
#define _SYSMON_WIN_H

#if defined(_WIN32) && defined(_CONSOLE)

#ifdef __cplusplus
extern "C" {
#endif

int initialize_system_monitor(int argc, char** argv);
int initialize_service_dispatcher(int argc, char** argv);

int cleanup_system_monitor();


// Internal name of the service
#define SZSERVICENAME        "BOINC"

// Displayed name of the service
#define SZSERVICEDISPLAYNAME "BOINC"

// Displayed description of the service
#define SZSERVICEDESCRIPTION "Berkeley Open Infrastructure for Network Computing"

// Service Accepted Actions
#define SERVICE_ACCEPTED_ACTIONS  ( \
    SERVICE_ACCEPT_STOP | \
    SERVICE_ACCEPT_PAUSE_CONTINUE | \
    SERVICE_ACCEPT_SHUTDOWN ) 

// Service Control Manager Routines
VOID WINAPI BOINCServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI BOINCServiceCtrl(DWORD dwCtrlCode);
BOOL		ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
VOID		LogEventErrorMessage(LPTSTR lpszMsg);
VOID		LogEventWarningMessage(LPTSTR lpszMsg);
VOID		LogEventInfoMessage(LPTSTR lpszMsg);

#ifdef __cplusplus
}
#endif

#endif

#endif
