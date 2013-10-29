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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "stdafx.h"
#include "boinccas.h"
#include "CAValidateInstall.h"

#define CUSTOMACTION_NAME               _T("CAValidateInstall")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating the install by checking all executables.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateInstall::CAValidateInstall(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateInstall::~CAValidateInstall()
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
UINT CAValidateInstall::OnExecution()
{
    tstring strInstallDirectory;
    tstring strProductVersion;
    tstring strFilename;
    tstring strTemp;
    UINT    uiReturnValue = 0;

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ProductVersion"), strProductVersion );
    if ( uiReturnValue ) return uiReturnValue;

    // Default to success
    SetProperty(_T("RETURN_VALIDATEINSTALL"), _T("1"));

    uiReturnValue = GetComponentKeyFilename( _T("_BOINC"), strFilename );
    if ( uiReturnValue ) return uiReturnValue;

    strTemp = strInstallDirectory + _T("\\") + strFilename;
    if (!ValidateExecutable( strTemp, strProductVersion ))
    {
        SetProperty(_T("RETURN_VALIDATEINSTALL"), _T("0"));
    }

    uiReturnValue = GetComponentKeyFilename( _T("_BOINCManager"), strFilename );
    if ( uiReturnValue ) return uiReturnValue;

    strTemp = strInstallDirectory + _T("\\") + strFilename;
    if (!ValidateExecutable( strTemp, strProductVersion ))
    {
        SetProperty(_T("RETURN_VALIDATEINSTALL"), _T("0"));
    }
    
    uiReturnValue = GetComponentKeyFilename( _T("_BOINCCMD"), strFilename );
    if ( uiReturnValue ) return uiReturnValue;

    strTemp = strInstallDirectory + _T("\\") + strFilename;
    if (!ValidateExecutable( strTemp, strProductVersion ))
    {
        SetProperty(_T("RETURN_VALIDATEINSTALL"), _T("0"));
    }
    
    uiReturnValue = GetComponentKeyFilename( _T("_BOINCTray"), strFilename );
    if ( uiReturnValue ) return uiReturnValue;

    strTemp = strInstallDirectory + _T("\\") + strFilename;
    if (!ValidateExecutable( strTemp, strProductVersion ))
    {
        SetProperty(_T("RETURN_VALIDATEINSTALL"), _T("0"));
    }


    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
BOOL CAValidateInstall::ValidateExecutable( tstring strExecutable, tstring strDesiredVersion )
{
    DWORD               dwHandle;
    LPVOID              lpData;
    DWORD               dwSize;
    TCHAR               szQuery[256];
    LPVOID              lpVar;
    UINT                uiVarSize;
    VS_FIXEDFILEINFO*   pFileInfo;
    TCHAR               szVersionInfo[24];
    TCHAR               szProductVersion[256];
    TCHAR               szMessage[2048];

    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;


    _sntprintf(
        szMessage, 
        sizeof(szMessage),
        _T("Validating Executable: '%s' Version: '%s'"),
        strExecutable.c_str(),
        strDesiredVersion.c_str()
    );
    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        szMessage
    );


    // Get File Version Information
    //
    dwSize = GetFileVersionInfoSize(strExecutable.c_str(), &dwHandle);
    if (dwSize) {
        lpData = (LPVOID)malloc(dwSize);
        if(GetFileVersionInfo(strExecutable.c_str(), dwHandle, dwSize, lpData)) {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Executable Found")
            );

            // Which language should be used to lookup the structure?
            strcpy(szQuery, _T("\\VarFileInfo\\Translation"));
            VerQueryValue(lpData, szQuery, (LPVOID*)&lpTranslate, &uiVarSize);

            // Version specified as part of the root record.
            if (VerQueryValue(lpData, _T("\\"), (LPVOID*)&pFileInfo, &uiVarSize)) {
                _sntprintf(szVersionInfo, sizeof(szVersionInfo), _T("%d.%d.%d.%d"), 
                    HIWORD(pFileInfo->dwFileVersionMS),
                    LOWORD(pFileInfo->dwFileVersionMS),
                    HIWORD(pFileInfo->dwFileVersionLS),
                    LOWORD(pFileInfo->dwFileVersionLS)
                );
            }

            // Product Version.
            _stprintf(szQuery, _T("\\StringFileInfo\\%04x%04x\\ProductVersion"),
                lpTranslate[0].wLanguage,
                lpTranslate[0].wCodePage
            );
            if (VerQueryValue(lpData, szQuery, &lpVar, &uiVarSize)) {
                uiVarSize = _sntprintf(szProductVersion, sizeof(szProductVersion), _T("%s"), lpVar);
                if ((sizeof(szProductVersion) == uiVarSize) || (-1 == uiVarSize)) {
                    szProductVersion[255] = '\0';
                }
            }

            _sntprintf(
                szMessage, 
                sizeof(szMessage),
                _T("Product Version: '%s'"),
                szProductVersion
            );
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                szMessage
            );
            free(lpData);
        }
    }

    if (strDesiredVersion != szProductVersion) {
        return FALSE;
    }
    return TRUE;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    ValidateInstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ValidateInstall(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAValidateInstall* pCA = new CAValidateInstall(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

