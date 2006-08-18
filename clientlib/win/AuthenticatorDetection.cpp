// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#include "stdafx.h"
#include "Identification.h"


/**
 * Detect what authenticator to use from the current users cookie cache.
 *
 * A project will assign an authenticator from some web based signup system as part
 * of their HTTP cookie, from there we can query Internet Explorer and get the
 * authenticator and use it during the attach to project wizard execution.
 *
 * Internet Explorer is the only browser supported at present.
 **/
EXTERN_C __declspec(dllexport) BOOL DetectSetupAuthenticator(LPCTSTR szProjectURL, LPTSTR szAuthenticator, LPDWORD lpdwSize)
{
    BOOL  bReturnValue = FALSE;
    TCHAR  szCookie[2048];
    DWORD dwSize = sizeof(szCookie)/sizeof(TCHAR);

    bReturnValue = InternetGetCookieEx(szProjectURL, _T("Setup"), szCookie, &dwSize, INTERNET_COOKIE_THIRD_PARTY, NULL);
    if (bReturnValue)
    {
        _tcsncpy(szAuthenticator, _tcsstr(szCookie, TEXT("=")) + 1, *lpdwSize);
        *lpdwSize = (DWORD)_tcslen(szAuthenticator);
    }
    else
    {
        fprintf(stderr, TEXT("DetectSetupAuthenticator() - InternetGetCookieEx Failed. GetLastError = '%d'"), GetLastError());
    }

    return bReturnValue;
}

