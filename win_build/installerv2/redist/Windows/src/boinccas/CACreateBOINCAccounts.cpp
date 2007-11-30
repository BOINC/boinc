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
#include "CACreateBOINCAccounts.h"
#include "lsaprivs.h"
#include "password.h"


#define CUSTOMACTION_NAME               _T("CACreateBOINCAccounts")
#define CUSTOMACTION_PROGRESSTITLE      _T("Create user accounts used by BOINC for secure sandboxes")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateBOINCAccounts::CACreateBOINCAccounts(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateBOINCAccounts::~CACreateBOINCAccounts()
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
UINT CACreateBOINCAccounts::OnExecution()
{
    tstring          strBOINCMasterAccountUsername;
    tstring          strBOINCMasterAccountPassword;
    tstring          strBOINCProjectAccountUsername;
    tstring          strBOINCProjectAccountPassword;
    tstring          strComputerName;
    tstring          strProductType;
    PSID             pSid;
    NET_API_STATUS   nasReturnValue;
    BOOL             bBOINCMasterAccountModified = FALSE;
    BOOL             bBOINCProjectAccountModified = FALSE;
    UINT             uiReturnValue = -1;

    uiReturnValue = GetProperty( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_MASTER_PASSWORD"), strBOINCMasterAccountPassword );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECT_PASSWORD"), strBOINCProjectAccountPassword );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ComputerName"), strComputerName );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("MsiNTProductType"), strProductType );
    if ( uiReturnValue ) return uiReturnValue;


    // Only create a new account or change the password on an existing account
    //   if the user hasn't explicitly defined an account
    if (strBOINCMasterAccountUsername.empty() && strBOINCMasterAccountPassword.empty()) {

        // Determine what the real values of the usernames should be based off
        //   of the inputs
        //
        if (strProductType == tstring(_T("2"))) {                    // Domain Controller
            if (strBOINCMasterAccountUsername.empty()) {
                strBOINCMasterAccountUsername = _T("boinc_master_") + strComputerName;
            }
        } else {
            if (strBOINCMasterAccountUsername.empty()) {
                strBOINCMasterAccountUsername = _T("boinc_master");
            }
        }


        // Generate random passwords if needed
        //
        if (strBOINCMasterAccountPassword.empty()) {
            GenerateRandomPassword(strBOINCMasterAccountPassword, 12);
            strBOINCMasterAccountPassword = _T("!") + strBOINCMasterAccountPassword;
        }


        // Create the 'boinc_master' account if needed, otherwise just update the password.
        //
        if(GetAccountSid(NULL, strBOINCMasterAccountUsername.c_str(), &pSid)) {   // Check if user exists
            // Account already exists, just change the password
            //
            USER_INFO_1003 ui;
            DWORD          dwParameterError;

            ui.usri1003_password = (LPWSTR)strBOINCMasterAccountPassword.c_str();

            nasReturnValue = NetUserSetInfo(
                NULL,
                strBOINCMasterAccountUsername.c_str(),
                1003,
                (LPBYTE)&ui,
                &dwParameterError
            );

            if (NERR_Success != nasReturnValue) {
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL, 
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("Failed to reset password on the 'boinc_master' account.")
                );
                return ERROR_INSTALL_FAILURE;
            }
        } else {
            // Account does not exist, create it
            //
            USER_INFO_1 ui;
            DWORD       dwParameterError;

            ui.usri1_name = (LPWSTR)strBOINCMasterAccountUsername.c_str();
            ui.usri1_password = (LPWSTR)strBOINCMasterAccountPassword.c_str();
            ui.usri1_comment = _T("Account used to execute BOINC as a system service");
            ui.usri1_priv = USER_PRIV_USER;
            ui.usri1_flags = UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;

            nasReturnValue = NetUserAdd(
                NULL,
                1,
                (LPBYTE)&ui,
                &dwParameterError
            );

            if (NERR_Success != nasReturnValue) {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("NetUserAdd retval")
                );
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    dwParameterError,
                    _T("NetUserAdd dwParameterError")
                );
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL, 
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("Failed to create the 'boinc_master' account.")
                );
                return ERROR_INSTALL_FAILURE;
            }
        }
        if(pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);

        bBOINCMasterAccountModified = TRUE;
    }

    // Only create a new account or change the password on an existing account
    //   if the user hasn't explicitly defined an account
    if (strBOINCProjectAccountUsername.empty() && strBOINCProjectAccountPassword.empty()) {

        // Determine what the real values of the usernames should be based off
        //   of the inputs
        //
        if (strProductType == tstring(_T("2"))) {                    // Domain Controller
            if (strBOINCProjectAccountUsername.empty()) {
                strBOINCProjectAccountUsername = _T("boinc_project_") + strComputerName;
            }
        } else {
            if (strBOINCProjectAccountUsername.empty()) {
                strBOINCProjectAccountUsername = _T("boinc_project");
            }
        }


        // Generate random passwords if needed
        //
        if (strBOINCProjectAccountPassword.empty()) {
            GenerateRandomPassword(strBOINCProjectAccountPassword, 12);
            strBOINCProjectAccountPassword = _T("!") + strBOINCProjectAccountPassword;
        }


        // Create the 'boinc_project' account if needed, otherwise just update the password.
        //
        if(GetAccountSid(NULL, strBOINCProjectAccountUsername.c_str(), &pSid)) {   // Check if user exists
            // Account already exists, just change the password
            //
            USER_INFO_1003 ui;
            DWORD          dwParameterError;

            ui.usri1003_password = (LPWSTR)strBOINCProjectAccountPassword.c_str();

            nasReturnValue = NetUserSetInfo(
                NULL,
                strBOINCProjectAccountUsername.c_str(),
                1003,
                (LPBYTE)&ui,
                &dwParameterError
            );

            if (NERR_Success != nasReturnValue) {
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL, 
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("Failed to reset password on the 'boinc_project' account.")
                );
                return ERROR_INSTALL_FAILURE;
            }
        } else {
            // Account does not exist, create it
            //
            USER_INFO_1 ui;
            DWORD       dwParameterError;

            ui.usri1_name = (LPWSTR)strBOINCProjectAccountUsername.c_str();
            ui.usri1_password = (LPWSTR)strBOINCProjectAccountPassword.c_str();
            ui.usri1_comment = _T("Account used to execute BOINC applications");
            ui.usri1_priv = USER_PRIV_USER;
            ui.usri1_flags = UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;

            nasReturnValue = NetUserAdd(
                NULL,
                1,
                (LPBYTE)&ui,
                &dwParameterError
            );

            if (NERR_Success != nasReturnValue) {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("NetUserAdd retval")
                );
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    dwParameterError,
                    _T("NetUserAdd dwParameterError")
                );
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL, 
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("Failed to create the 'boinc_project' account.")
                );
                return ERROR_INSTALL_FAILURE;
            }

        }
        if(pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);

        bBOINCProjectAccountModified = TRUE;
    }


    SetProperty( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    if (bBOINCMasterAccountModified) {
        SetProperty( _T("BOINC_MASTER_ISUSERNAME"), tstring(_T(".\\") + strBOINCMasterAccountUsername) );
    } else {
        SetProperty( _T("BOINC_MASTER_ISUSERNAME"), strBOINCMasterAccountUsername );
    }
    SetProperty( _T("BOINC_MASTER_PASSWORD"), strBOINCMasterAccountPassword );

    SetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );
    if (bBOINCProjectAccountModified) {
        SetProperty( _T("BOINC_PROJECT_ISUSERNAME"), tstring(_T(".\\") + strBOINCProjectAccountUsername) );
    } else {
        SetProperty( _T("BOINC_PROJECT_ISUSERNAME"), strBOINCProjectAccountUsername );
    }
    SetProperty( _T("BOINC_PROJECT_PASSWORD"), strBOINCProjectAccountPassword );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    CreateBOINCAccounts
//
// Description: This custom action creates the two user accounts that'll
//              be used to enfore the account based sandboxing scheme
//              on Windows.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CreateBOINCAccounts(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACreateBOINCAccounts* pCA = new CACreateBOINCAccounts(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

const char *BOINC_RCSID_00ed9686df="$Id: CACreateBOINCAccounts.cpp 11804 2007-01-08 18:42:48Z rwalton $";
