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
#include "CAValidateSetupType.h"

#define CUSTOMACTION_NAME               _T("CAValidateSetupType")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating setup type parameters")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateSetupType::CAValidateSetupType(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateSetupType::~CAValidateSetupType()
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
UINT CAValidateSetupType::OnExecution()
{
    tstring strSetupType;
    tstring strAllUsers;
    tstring strIsAdminPackage;
    UINT    uiReturnValue = 0;

    uiReturnValue = GetProperty( _T("SETUPTYPE"), strSetupType );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ALLUSERS"), strAllUsers );
    if ( uiReturnValue ) return uiReturnValue;

    // When applications are assign to the computer, they are installed at boot
    //   time.  In this environment ALLUSERS is always set to 1.
    uiReturnValue = GetProperty( _T("IsAdminPackage"), strIsAdminPackage );
    if ( uiReturnValue ) return uiReturnValue;

    if ( !strSetupType.empty() )
    {
        if ( ( _T("Single") == strSetupType ) || ( _T("Service") == strSetupType ) )
        {
            if ( ( !strAllUsers.empty() ) && ( _T("1") != strIsAdminPackage ) )
            {
                LogMessage(
                    INSTALLMESSAGE_FATALEXIT,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("An invalid SETUPTYPE combination has been detected, If SETUPTYPE is 'Single' or 'Service' then ALLUSERS has to be empty.")
                    );
                return ERROR_INSTALL_FAILURE;
            }
        }
        else
        {
            if ( _T("1") != strAllUsers )
            {
                LogMessage(
                    INSTALLMESSAGE_FATALEXIT,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("An invalid SETUPTYPE combination has been detected, If SETUPTYPE is not 'Shared' then ALLUSERS has to equal '1'.")
                    );
                return ERROR_INSTALL_FAILURE;
            }
        }
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    ValidateSetupType
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ValidateSetupType(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAValidateSetupType* pCA = new CAValidateSetupType(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}



