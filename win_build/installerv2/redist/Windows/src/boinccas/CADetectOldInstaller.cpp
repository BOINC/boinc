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
#include "CADetectOldInstaller.h"

#define CUSTOMACTION_NAME               _T("CADetectOldInstaller")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CADetectOldInstall::CADetectOldInstall(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CADetectOldInstall::~CADetectOldInstall()
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
UINT CADetectOldInstall::OnExecution()
{
    UINT    uiReturnValue = 0;
    HKEY    hKey;
    LONG    lRet;

    lRet = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\BOINC"),
        0,
        KEY_QUERY_VALUE,
        &hKey
        );

    if ( lRet == ERROR_SUCCESS )
    {
        DisplayMessage(
            MB_OK,
            MB_ICONERROR,
            _T("Setup has detected an older version of BOINC which must be uninstalled before this version of BOINC can be installed.")
            );

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("Setup has detected an older version of BOINC which must be uninstalled before this version of BOINC can be installed.")
            );

        uiReturnValue = ERROR_INSTALL_FAILURE;
    }
    else // Windows NT 4.0 prior to SP6a
    {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("Setup did NOT detect a previous version of the BOINC installer on the system.")
            );
        uiReturnValue = ERROR_SUCCESS;
    }

    RegCloseKey( hKey );

    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    DetectOldInstaller
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall DetectOldInstaller(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CADetectOldInstall* pCA = new CADetectOldInstall(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

