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
#include "win_util.h"


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
    BOOL        bReturnValue = FALSE;
    BOOL        bValidates = TRUE;
    TCHAR       szCookieBuffer[2048];
    LPTSTR      pszCookieFragment = NULL;
    DWORD       dwSize = sizeof(szCookieBuffer)/sizeof(TCHAR);
    tstring     strCookieFragment;
    tstring     strCookieName;
    tstring     strCookieValue;
    size_t      uiDelimeterLocation;

    bReturnValue = InternetGetCookie(szProjectURL, NULL, szCookieBuffer, &dwSize);
    if (bReturnValue)
    {
        // Format of cookie buffer:
        // 'cookie1=value1; cookie2=value2; cookie3=value3;
        //
        pszCookieFragment = _tcstok(szCookieBuffer, _T("; "));
        while(pszCookieFragment)
        {
            // Convert to a std::string so we can go to town
            strCookieFragment = pszCookieFragment;

            // Extract the name & value
            uiDelimeterLocation = strCookieFragment.find(_T("="), 0);
            strCookieName = strCookieFragment.substr(0, uiDelimeterLocation);
            strCookieValue = strCookieFragment.substr(uiDelimeterLocation + 1);

            if (tstring(_T("Setup")) == strCookieName)
            {
                // Perform some basic validation of the suspect authenticator
                //

                // If the string is null then it is invalid.
                if (0 == strCookieValue.length()) {
                    bValidates = FALSE;
                }

                // If the string contains non alpha numeric characters it is invalid.
                tstring::iterator it = strCookieValue.begin();
                while (it != strCookieValue.end()) {
                    if (!_istalpha(*it) && !_istdigit(*it)) {
                        bValidates = FALSE;
                    }
                    it++;
                }

                // If validation failed, null out the Authenticator field just in case
                //   somebody tries to use it, otherwise copy in the real deal.
                if (!bValidates) {
                    _tcsncpy(szAuthenticator, _T(""), *lpdwSize);
                    *lpdwSize = 0;
                } else {
                    _tcsncpy(szAuthenticator, strCookieValue.c_str(), *lpdwSize);
                    *lpdwSize = (DWORD)_tcslen(szAuthenticator);
                }
            }

            pszCookieFragment = _tcstok(NULL, _T("; "));
        }
    }
    else
    {
        fprintf(stderr, _T("DetectSetupAuthenticator() - InternetGetCookieEx Failed. GetLastError = '%d'"), GetLastError());
    }


    return bReturnValue;
}

