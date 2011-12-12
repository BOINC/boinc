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

UINT CALaunchBOINCManager::OnExecution()
{
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    HANDLE hProcessToken;
    HANDLE hRestrictedToken;
    tstring strInstallDirectory;
    tstring strBuffer;
    UINT uiReturnValue = -1;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    OpenProcessToken( GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &hProcessToken );
    CreateRestrictedToken( hProcessToken, DISABLE_MAX_PRIVILEGE, 0, 0, 0, 0, 0, 0, &hRestrictedToken );

    strBuffer = strInstallDirectory + _T("\\boincmgr.exe");
    if (CreateProcessAsUser( hRestrictedToken, strBuffer.c_str(), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup_info, &process_info ))
    {
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }

    strBuffer = strInstallDirectory + _T("\\gridrepublic.exe");
    if (CreateProcessAsUser( hRestrictedToken, strBuffer.c_str(), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup_info, &process_info ))
    {
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }

    strBuffer = strInstallDirectory + _T("\\charityengine.exe");
    if (CreateProcessAsUser( hRestrictedToken, strBuffer.c_str(), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup_info, &process_info ))
    {
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }

    strBuffer = strInstallDirectory + _T("\\progressthruprocessors.exe");
    if (CreateProcessAsUser( hRestrictedToken, strBuffer.c_str(), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup_info, &process_info ))
    {
        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
    }

    CloseHandle( hRestrictedToken );
    CloseHandle( hProcessToken );
 
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
