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
#include "CAVerifyServicePassword.h"

#define CUSTOMACTION_NAME               _T("CAVerifyServicePassword")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyServicePassword::CAVerifyServicePassword(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyServicePassword::~CAVerifyServicePassword()
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
UINT CAVerifyServicePassword::OnExecution()
{
    tstring strServicePassword;
    tstring strServicePasswordConfirmation;
    UINT    uiReturnValue = 0;


    uiReturnValue = GetProperty( _T("SERVICE_PASSWORD"), strServicePassword, false );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("SERVICE_CONFIRMPASSWORD"), strServicePasswordConfirmation, false );
    if ( uiReturnValue ) return uiReturnValue;


    if ( strServicePassword != strServicePasswordConfirmation )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("The password and confirm password editboxes must match.")
            );

        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else
    {
        if ( strServicePassword.empty() )
        {
            DisplayMessage(
                MB_OK, 
                MB_ICONERROR,
                _T("The selected password cannot be null, please choose a password to continue.")
                );

            uiReturnValue = ERROR_INSTALL_USEREXIT;
        }
        else
        {
            SetProperty(_T("RETURN_VERIFYSERVICEPASSWORD"), _T("1"));
            uiReturnValue = ERROR_SUCCESS;
        }
    }

    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    VerifyServicePassword
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall VerifyServicePassword(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAVerifyServicePassword* pCA = new CAVerifyServicePassword(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

const char *BOINC_RCSID_8fa10257fc="$Id$";
