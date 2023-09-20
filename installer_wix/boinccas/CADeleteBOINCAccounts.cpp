// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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
#include "CADeleteBOINCAccounts.h"
#include "lsaprivs.h"
#include "password.h"


#define CUSTOMACTION_NAME               _T("CADeleteBOINCAccounts")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating user accounts used by BOINC for secure sandboxes")


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CADeleteBOINCAccounts::CADeleteBOINCAccounts(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CADeleteBOINCAccounts::~CADeleteBOINCAccounts()
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
UINT CADeleteBOINCAccounts::OnExecution()
{
    tstring          strBOINCMasterAccountUsername;
    tstring          strBOINCProjectAccountUsername;
    NET_API_STATUS   nasReturnValue;
    UINT             uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;


    if (IsUpgrading())
    {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("Upgrade detected, no need to delete accounts")
        );
        return ERROR_SUCCESS;
    }


    // Delete 'boinc_master' account
    //
    nasReturnValue = NetUserDel(
        NULL,
        (LPWSTR)strBOINCMasterAccountUsername.c_str()
    );

    if (NERR_Success != nasReturnValue) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("NetUserDel retval")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("Failed to delete the 'boinc_master' account.")
        );
        return ERROR_INSTALL_FAILURE;
    }


    // Delete 'boinc_project' account
    //
    nasReturnValue = NetUserDel(
        NULL,
        (LPWSTR)strBOINCProjectAccountUsername.c_str()
    );

    if (NERR_Success != nasReturnValue) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("NetUserDel retval")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("Failed to delete the 'boinc_project' account.")
        );
        return ERROR_INSTALL_FAILURE;
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    DeleteBOINCAccounts
//
// Description: This custom action delete the two user accounts that
//              are used to enforce the account based sandboxing scheme
//              on Windows.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall DeleteBOINCAccounts(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CADeleteBOINCAccounts* pCA = new CADeleteBOINCAccounts(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
