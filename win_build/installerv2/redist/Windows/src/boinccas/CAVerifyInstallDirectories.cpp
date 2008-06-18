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
#include "CAVerifyInstallDirectories.h"

#define CUSTOMACTION_NAME               _T("CAVerifyInstallDirectories")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyInstallDirectories::CAVerifyInstallDirectories(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyInstallDirectories::~CAVerifyInstallDirectories()
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
UINT CAVerifyInstallDirectories::OnExecution()
{
    tstring strInstallDirectory;
    tstring strDataDirectory;
    UINT    uiReturnValue = 0;


    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;


    if ( strInstallDirectory == strDataDirectory )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("The installation directory and data directory must be different directories. Please select a different data directory.")
            );

        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else
    {
        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("1"));
        uiReturnValue = ERROR_SUCCESS;
    }

    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    VerifyInstallDirectories
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall VerifyInstallDirectories(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAVerifyInstallDirectories* pCA = new CAVerifyInstallDirectories(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

