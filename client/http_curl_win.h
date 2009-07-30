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

#ifndef _HTTP_CURL_WIN_
#define _HTTP_CURL_WIN_

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

extern "C" {

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

} // extern "C"

#define WINHTTP_AUTOPROXY_AUTO_DETECT           0x00000001
#define WINHTTP_AUTOPROXY_CONFIG_URL            0x00000002
#define WINHTTP_AUTOPROXY_RUN_INPROCESS         0x00010000
#define WINHTTP_AUTOPROXY_RUN_OUTPROCESS_ONLY   0x00020000
#define WINHTTP_AUTO_DETECT_TYPE_DHCP           0x00000001
#define WINHTTP_AUTO_DETECT_TYPE_DNS_A          0x00000002
#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY               0
#define WINHTTP_ACCESS_TYPE_NO_PROXY                    1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY                 3
#define WINHTTP_NO_PROXY_NAME     NULL
#define WINHTTP_NO_PROXY_BYPASS   NULL

#endif // _HTTP_CURL_WIN