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
#include "CALaunchBOINCManager.h"

#define CUSTOMACTION_NAME               _T("CALaunchBOINCManager")
#define CUSTOMACTION_PROGRESSTITLE      _T("Launching BOINC Manager")

#ifndef SECURITY_MANDATORY_LABEL_AUTHORITY

#define SECURITY_MANDATORY_LABEL_AUTHORITY          {0,0,0,0,0,16}
#define SECURITY_MANDATORY_MEDIUM_RID               (0x00002000L)
#define SE_GROUP_INTEGRITY                          (0x00000020L)

typedef struct _TOKEN_MANDATORY_LABEL {
    SID_AND_ATTRIBUTES Label;
} TOKEN_MANDATORY_LABEL, *PTOKEN_MANDATORY_LABEL;

typedef enum MY_TOKEN_INFORMATION_CLASS {
    TokenVirtualizationEnabled = 24,
    TokenIntegrityLevel = 25
} MY_TOKEN_INFORMATION_CLASS, *PMY_TOKEN_INFORMATION_CLASS;

#endif

/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CALaunchBOINCManager::CALaunchBOINCManager(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CALaunchBOINCManager::~CALaunchBOINCManager()
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

typedef BOOL (__stdcall *tSCREATEL)( IN DWORD, IN DWORD, IN DWORD, OUT SAFER_LEVEL_HANDLE*, OUT LPVOID );
typedef BOOL (__stdcall *tSCTFL)( IN SAFER_LEVEL_HANDLE, IN HANDLE, OUT HANDLE*, IN DWORD, OUT LPVOID );
typedef BOOL (__stdcall *tSCLOSEL)( IN SAFER_LEVEL_HANDLE );

UINT CALaunchBOINCManager::OnExecution()
{
    static HMODULE advapi32lib = NULL;
    static tSCREATEL pSCREATEL = NULL;
    static tSCTFL pSCTFL = NULL;
    static tSCLOSEL pSCLOSEL = NULL;
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    SAFER_LEVEL_HANDLE hSaferHandle;
    HANDLE hRestrictedToken;
    SID_IDENTIFIER_AUTHORITY siaMLA = SECURITY_MANDATORY_LABEL_AUTHORITY;
    PSID  pSidMedium = NULL;
    TOKEN_MANDATORY_LABEL TIL = {0};
    DWORD dwEnableVirtualization = 1;
    tstring strInstallDirectory;
    tstring strBuffer;
    UINT uiReturnValue = -1;
    FILE* f;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_SHOW;

    f = fopen("LaunchManager.txt", "w");

    if (!advapi32lib) {
        advapi32lib = LoadLibraryA("advapi32.dll");
        if (advapi32lib) {
            pSCREATEL = (tSCREATEL)GetProcAddress(advapi32lib, "SaferCreateLevel");
            pSCTFL = (tSCTFL)GetProcAddress(advapi32lib, "SaferComputeTokenFromLevel");
            pSCLOSEL = (tSCLOSEL)GetProcAddress(advapi32lib, "SaferCloseLevel");
        }
    }

    if (!pSCREATEL || !pSCTFL || !pSCLOSEL) {
        return ERROR_FILE_NOT_FOUND;
    }

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;


    // Calculate a restricted token from the current token.
    if (!pSCREATEL( SAFER_SCOPEID_USER, SAFER_LEVELID_NORMALUSER, SAFER_LEVEL_OPEN, &hSaferHandle, NULL ))
    {
        fwprintf(f, _T("SaferCreateLevel retval: '%d'\n"), GetLastError());
    }

    if (!pSCTFL( hSaferHandle, NULL, &hRestrictedToken, NULL, NULL ))
    {
        fwprintf(f, _T("SaferComputeTokenFromLevel retval: '%d'\n"), GetLastError());
    }

    AllocateAndInitializeSid(&siaMLA, 1, SECURITY_MANDATORY_MEDIUM_RID, 0, 0, 0, 0, 0, 0, 0, &pSidMedium);

    TIL.Label.Attributes = SE_GROUP_INTEGRITY;
    TIL.Label.Sid        = pSidMedium;

    if (!SetTokenInformation(hRestrictedToken, (TOKEN_INFORMATION_CLASS)TokenIntegrityLevel, &TIL, sizeof(TOKEN_MANDATORY_LABEL)))
    {
        fwprintf(f, _T("SaferComputeTokenFromLevel (TokenIntegrityLevel) retval: '%d'\n"), GetLastError());
    }

    if (!SetTokenInformation(hRestrictedToken, (TOKEN_INFORMATION_CLASS)TokenVirtualizationEnabled, &dwEnableVirtualization, sizeof(DWORD)))
    {
        fwprintf(f, _T("SaferComputeTokenFromLevel (TokenVirtualizationEnabled) retval: '%d'\n"), GetLastError());
    }


    strBuffer = tstring(_T("\"")) + strInstallDirectory + tstring(_T("boincmgr.exe\""));
    fwprintf(f, _T("Attempting to launch boincmgr.exe\n"));
    fwprintf(f, _T("Launching: '%s'\n"), strBuffer.c_str());
    if (CreateProcessAsUser( hRestrictedToken, NULL, (LPWSTR)strBuffer.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &process_info ))
    {
        fwprintf(f, _T("Success!!!\n"));
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }
    else
    {
        fwprintf(f, _T("CreateProcessAsUser retval: '%d'\n"), GetLastError());
    }

    strBuffer = tstring(_T("\"")) + strInstallDirectory + tstring(_T("gridrepublic.exe\""));
    fwprintf(f, _T("Attempting to launch gridrepublic.exe\n"));
    fwprintf(f, _T("Launching: '%s'\n"), strBuffer.c_str());
    if (CreateProcessAsUser( hRestrictedToken, NULL, (LPWSTR)strBuffer.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &process_info ))
    {
        fwprintf(f, _T("Success!!!\n"));
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }
    else
    {
        fwprintf(f, _T("CreateProcessAsUser retval: '%d'\n"), GetLastError());
    }

    strBuffer = tstring(_T("\"")) + strInstallDirectory + tstring(_T("charityengine.exe\""));
    fwprintf(f, _T("Attempting to launch charityengine.exe\n"));
    fwprintf(f, _T("Launching: '%s'\n"), strBuffer.c_str());
    if (CreateProcessAsUser( hRestrictedToken, NULL, (LPWSTR)strBuffer.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &process_info ))
    {
        fwprintf(f, _T("Success!!!\n"));
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }
    else
    {
        fwprintf(f, _T("CreateProcessAsUser retval: '%d'\n"), GetLastError());
    }

    strBuffer = tstring(_T("\"")) + strInstallDirectory + tstring(_T("progressthruprocessors.exe\""));
    fwprintf(f, _T("Attempting to launch progressthruprocessors.exe\n"));
    fwprintf(f, _T("Launching: '%s'\n"), strBuffer.c_str());
    if (CreateProcessAsUser( hRestrictedToken, NULL, (LPWSTR)strBuffer.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &process_info ))
    {
        fwprintf(f, _T("Success!!!\n"));
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }
    else
    {
        fwprintf(f, _T("CreateProcessAsUser retval: '%d'\n"), GetLastError());
    }

    fclose(f);
    CloseHandle( hRestrictedToken );
    pSCLOSEL( hSaferHandle );
 
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    LaunchBOINCManager
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall LaunchBOINCManager(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CALaunchBOINCManager* pCA = new CALaunchBOINCManager(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_7bca979acf="$Id: CAShutdownBOINC.cpp 14659 2008-02-01 22:08:48Z romw $";
