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

#ifdef __cplusplus
extern "C" {
#endif

extern int initialize_system_monitor(int argc, char** argv);
extern int initialize_service_dispatcher(int argc, char** argv);
extern int cleanup_system_monitor();

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
extern VOID WINAPI BOINCServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
extern VOID WINAPI BOINCServiceCtrl(DWORD dwCtrlCode);
extern BOOL		ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
extern VOID		LogEventErrorMessage(LPTSTR lpszMsg);
extern VOID		LogEventWarningMessage(LPTSTR lpszMsg);
extern VOID		LogEventInfoMessage(LPTSTR lpszMsg);


// Originally from WinHttp.h
//   Which is not included in the VS 2005 platform SDK

typedef LPVOID HINTERNET;

typedef struct {
    DWORD  dwAccessType;      // see WINHTTP_ACCESS_* types below
    LPWSTR lpszProxy;         // proxy server list
    LPWSTR lpszProxyBypass;   // proxy bypass list
} WINHTTP_PROXY_INFO, * LPWINHTTP_PROXY_INFO;

typedef struct {
    DWORD   dwFlags;
    DWORD   dwAutoDetectFlags;
    LPCWSTR lpszAutoConfigUrl;
    LPVOID  lpvReserved;
    DWORD   dwReserved;
    BOOL    fAutoLogonIfChallenged;
} WINHTTP_AUTOPROXY_OPTIONS;

typedef struct {
    BOOL    fAutoDetect;
    LPWSTR  lpszAutoConfigUrl;
    LPWSTR  lpszProxy;
    LPWSTR  lpszProxyBypass;
} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;

typedef HINTERNET (WINAPI * pfnWinHttpOpen)
(
    IN LPCWSTR pwszUserAgent,
    IN DWORD   dwAccessType,
    IN LPCWSTR pwszProxyName   OPTIONAL,
    IN LPCWSTR pwszProxyBypass OPTIONAL,
    IN DWORD   dwFlags
);
typedef BOOL (STDAPICALLTYPE * pfnWinHttpCloseHandle)
(
    IN HINTERNET hInternet
);
typedef BOOL (STDAPICALLTYPE * pfnWinHttpGetProxyForUrl)
(
    IN  HINTERNET                   hSession,
    IN  LPCWSTR                     lpcwszUrl,
    IN  WINHTTP_AUTOPROXY_OPTIONS * pAutoProxyOptions,
    OUT WINHTTP_PROXY_INFO *        pProxyInfo  
);
typedef BOOL (STDAPICALLTYPE * pfnWinHttpGetIEProxyConfig)
(
    IN OUT WINHTTP_CURRENT_USER_IE_PROXY_CONFIG * pProxyConfig
);

#define WINHTTP_AUTOPROXY_AUTO_DETECT           0x00000001
#define WINHTTP_AUTOPROXY_CONFIG_URL            0x00000002
#define WINHTTP_AUTOPROXY_RUN_INPROCESS         0x00010000
#define WINHTTP_AUTOPROXY_RUN_OUTPROCESS_ONLY   0x00020000
#define WINHTTP_AUTO_DETECT_TYPE_DHCP           0x00000001
#define WINHTTP_AUTO_DETECT_TYPE_DNS_A          0x00000002
#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY                0
#define WINHTTP_ACCESS_TYPE_NO_PROXY                     1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY                  3
#define WINHTTP_NO_PROXY_NAME     NULL
#define WINHTTP_NO_PROXY_BYPASS   NULL

#ifdef __cplusplus
}
#endif

#endif
