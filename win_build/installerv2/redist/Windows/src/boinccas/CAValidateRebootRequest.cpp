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
#include "CAValidateRebootRequest.h"

#define CUSTOMACTION_NAME               _T("CAValidateRebootRequest")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating any reboot requests.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateRebootRequest::CAValidateRebootRequest(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateRebootRequest::~CAValidateRebootRequest()
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
UINT CAValidateRebootRequest::OnExecution()
{
    tstring strInstallDirectory;
    tstring strRebootRequested;
    tstring strRebootPendingFilename;
    FILE*   fRebootPending;
    UINT    uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("RETURN_REBOOTREQUESTED"), strRebootRequested );
    if ( uiReturnValue ) return uiReturnValue;


    // Create reboot pending file
    //
    strRebootPendingFilename = strInstallDirectory + _T("\\RebootPending.txt");

    fRebootPending = _tfopen(strRebootPendingFilename.c_str(), _T("wb"));
    if (fRebootPending) fclose(fRebootPending);


    // Create a registry key to delete the RebootInProgress.txt flag
    //   file on a reboot.
    //
    if ( _T("1") == strRebootRequested )
    {
	    LONG lReturnValue;
	    HKEY hkSetupHive;

	    lReturnValue = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE, 
            _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
		    0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL,
            &hkSetupHive,
            NULL
        );
	    if (lReturnValue == ERROR_SUCCESS) 
        {
            tstring strCommand;

            strCommand = _T("cmd.exe /c \"del \"") + strRebootPendingFilename + _T("\"\"");

            RegSetValueEx(
                hkSetupHive, 
                _T("*BOINCRebootPendingCleanup"),
                0,
                REG_SZ,
                (CONST BYTE *)strCommand.c_str(),
                (DWORD)(strCommand.size()*sizeof(TCHAR))
            );

	        RegCloseKey(hkSetupHive);
        }
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    ValidateRebootRequest
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ValidateRebootRequest(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAValidateRebootRequest* pCA = new CAValidateRebootRequest(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

