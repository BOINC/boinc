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
//

#include "stdafx.h"
#include "boinccas.h"
#include "win_util.h"
#include "base64.h"
#include "project_init.h"
#include "CACreateProjectInitFile.h"


#define CUSTOMACTION_NAME               _T("CACreateProjectInitFile")
#define CUSTOMACTION_PROGRESSTITLE      _T("Store project initialization data")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateProjectInitFile::CACreateProjectInitFile(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateProjectInitFile::~CACreateProjectInitFile()
{
    BOINCCABase::~BOINCCABase();
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
tstring CACreateProjectInitFile::ParseParameter(tstring& strSetupExeName, tstring& strParameter)
{
    tstring strParameterName;
    tstring strEncodedValue;
    tstring strParameterValue;
    tstring strError;
    size_t iParameterStart = 0;
    size_t iParameterEnd = 0;

    strParameterName = strParameter + _T("_");

    strError  = _T("Searching for parameter '");
    strError += strParameterName;
    strError += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strError.c_str()
    );

    iParameterStart = strSetupExeName.rfind(strParameterName);
    if (iParameterStart != tstring::npos) {
        iParameterStart += strParameterName.size();
        iParameterEnd = strSetupExeName.find(_T("_"), iParameterStart);
        if (iParameterEnd == tstring::npos) {
            iParameterEnd = strSetupExeName.find(_T("."), iParameterStart);
            if (iParameterEnd == tstring::npos) {
                return tstring(_T(""));
            }
        }
        strEncodedValue = strSetupExeName.substr(iParameterStart, iParameterEnd - iParameterStart);

        strError  = _T("Found encoded value '");
        strError += strEncodedValue;
        strError += _T("'");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            strError.c_str()
        );

        // WCG didn't want to have to encode their setup cookie value.  So all parameters but the setup cookie
        // are base64 encoded.
        //
        if (strParameterName == _T("asc_")) {

            strParameterValue = strEncodedValue;

        } else {

            CW2A pszASCIIEncodedValue( strEncodedValue.c_str() );
            CA2W pszUnicodeDecodedValue( r_base64_decode(pszASCIIEncodedValue, strlen(pszASCIIEncodedValue)).c_str() );

            strError  = _T("Decoded value '");
            strError += pszUnicodeDecodedValue;
            strError += _T("'");

            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                strError.c_str()
            );

            strParameterValue = pszUnicodeDecodedValue;

        }
    }

    strError  = _T("Returning value '");
    strError += strParameterValue;
    strError += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strError.c_str()
    );

    return strParameterValue;
}

/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CACreateProjectInitFile::OnExecution()
{
    tstring          strSetupExeName;
    tstring          strDataDirectory;
    tstring          strProjectInitUrl;
    tstring          strProjectInitName;
    tstring          strProjectInitTeamName;
    tstring          strProjectInitAuthenticator;
    tstring          strProjectInitSetupCookie;
    PROJECT_INIT     pi;
    UINT             uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_URL"), strProjectInitUrl );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_AUTH"), strProjectInitAuthenticator );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_TEAMNAME"), strProjectInitTeamName );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("SETUPEXENAME"), strSetupExeName );
    if ( uiReturnValue ) return uiReturnValue;

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("Changing to the data directory")
    );

    _tchdir(strDataDirectory.c_str());

    if (!strProjectInitUrl.empty()) {

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("Detected command line parameters")
        );

        pi.init();

        strncpy(pi.url, CW2A(strProjectInitUrl.c_str()), sizeof(pi.url)-1);
        strncpy(pi.name, CW2A(strProjectInitUrl.c_str()), sizeof(pi.name)-1);
        strncpy(pi.account_key, CW2A(strProjectInitAuthenticator.c_str()), sizeof(pi.account_key)-1);
        strncpy(pi.team_name, CW2A(strProjectInitTeamName.c_str()), sizeof(pi.team_name)-1);

        pi.embedded = false;

        pi.write();

    } else {

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("Checking for file name parameters")
        );

        strProjectInitUrl = ParseParameter(strSetupExeName, tstring(_T("amu")));
        strProjectInitName = ParseParameter(strSetupExeName, tstring(_T("an")));
        strProjectInitAuthenticator = ParseParameter(strSetupExeName, tstring(_T("aa")));
        strProjectInitSetupCookie = ParseParameter(strSetupExeName, tstring(_T("asc")));

        if (!strProjectInitUrl.empty() || !strProjectInitName.empty() || !strProjectInitAuthenticator.empty() || !strProjectInitSetupCookie.empty()) {

            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Detected file name parameters")
            );

            pi.init();

            if (!strProjectInitUrl.empty()) {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("Detected project url")
                );
                strncpy(pi.url, CW2A(strProjectInitUrl.c_str()), sizeof(pi.url)-1);
                pi.embedded = false;
            }
            if (!strProjectInitName.empty()) {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("Detected project name")
                );
                strncpy(pi.name, CW2A(strProjectInitName.c_str()), sizeof(pi.name)-1);
            }
            if (!strProjectInitAuthenticator.empty()) {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("Detected project authenticator")
                );
                strncpy(pi.account_key, CW2A(strProjectInitAuthenticator.c_str()), sizeof(pi.account_key)-1);
            }
            if (!strProjectInitSetupCookie.empty()) {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("Detected setup cookie")
                );
                strncpy(pi.setup_cookie, CW2A(strProjectInitSetupCookie.c_str()), sizeof(pi.setup_cookie)-1);
            }

            pi.write();

        }
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    CreateProjectInitFile
//
// Description: This custom action stores the project init data 
//                specified on the commandline in a file in the data
//                directory.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CreateProjectInitFile(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACreateProjectInitFile* pCA = new CACreateProjectInitFile(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
