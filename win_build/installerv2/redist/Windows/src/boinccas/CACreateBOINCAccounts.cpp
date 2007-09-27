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
#include "CACreateBOINCAccounts.h"
#include "lsaprivs.h"


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
    tstring          strBOINCAccountUsername;
    tstring          strBOINCAccountPassword;
    tstring          strBOINCProjectAccountUsername;
    tstring          strBOINCProjectAccountPassword;
    tstring          strComputerName;
    tstring          strProductType;
    PSID             pSid;
    NET_API_STATUS   nasReturnValue;
    UINT             uiReturnValue = -1;

    uiReturnValue = GetProperty( _T("BOINC_USERNAME"), strBOINCAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PASSWORD"), strBOINCAccountPassword );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECT_PASSWORD"), strBOINCProjectAccountPassword );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ComputerName"), strComputerName );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("MsiNTProductType"), strProductType );
    if ( uiReturnValue ) return uiReturnValue;


    // Determine what the real values of the usernames should be based off
    //   of the inputs
    //
    if (strProductType == tstring(_T("2"))) {                    // Domain Controller
        if (strBOINCAccountUsername.empty()) {
            strBOINCAccountUsername = _T("boinc_") + strComputerName;
        }
        if (strBOINCProjectAccountUsername.empty()) {
            strBOINCProjectAccountUsername = _T("boinc_project_") + strComputerName;
        }
    } else {
        if (strBOINCAccountUsername.empty()) {
            strBOINCAccountUsername = _T("boinc");
        }
        if (strBOINCProjectAccountUsername.empty()) {
            strBOINCProjectAccountUsername = _T("boinc_project");
        }
    }


    // Generate random passwords if needed
    //
    if (strBOINCAccountPassword.empty()) {
        strBOINCAccountPassword = _T("boinc");
    }
    if (strBOINCProjectAccountPassword.empty()) {
        strBOINCProjectAccountPassword = _T("boinc_project");
    }


    // Create the 'boinc' account if needed, otherwise just update the password.
    //
    if(GetAccountSid(NULL, strBOINCAccountUsername.c_str(), &pSid)) {   // Check if user exists
        // Account already exists, just change the password
        //
        USER_INFO_1003 ui1003;
        DWORD          dwParameterError;

        ui1003.usri1003_password = (LPWSTR)strBOINCAccountPassword.c_str();

        nasReturnValue = NetUserSetInfo(
            NULL,
            strBOINCAccountUsername.c_str(),
            1003,
            (LPBYTE)&ui1003,
            &dwParameterError
        );

        if (NERR_Success != nasReturnValue) {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                nasReturnValue,
                _T("Failed to reset password on the 'boinc' account.")
            );
            return ERROR_INSTALL_FAILURE;
        }
    } else {
        // Account does not exist, create it
        //
        USER_INFO_1 ui1;
        DWORD       dwParameterError;

        ui1.usri1_name = (LPWSTR)strBOINCAccountUsername.c_str();
        ui1.usri1_password = (LPWSTR)strBOINCAccountPassword.c_str();
        ui1.usri1_comment = _T("Account used to execute BOINC as a system service");
        ui1.usri1_priv = USER_PRIV_USER;
        ui1.usri1_flags = UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;

        nasReturnValue = NetUserAdd(
            NULL,
            1,
            (LPBYTE)&ui1,
            &dwParameterError
        );

        if (NERR_Success != nasReturnValue) {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                nasReturnValue,
                _T("Failed to create the 'boinc' account.")
            );
            return ERROR_INSTALL_FAILURE;
        }

    }
    if(pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);


    // Create the 'boinc_project' account if needed, otherwise just update the password.
    //
    if(GetAccountSid(NULL, strBOINCProjectAccountUsername.c_str(), &pSid)) {   // Check if user exists
        // Account already exists, just change the password
        //
        USER_INFO_1003 ui1003;
        DWORD          dwParameterError;

        ui1003.usri1003_password = (LPWSTR)strBOINCProjectAccountPassword.c_str();

        nasReturnValue = NetUserSetInfo(
            NULL,
            strBOINCProjectAccountUsername.c_str(),
            1003,
            (LPBYTE)&ui1003,
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
        USER_INFO_1 ui1;
        DWORD       dwParameterError;

        ui1.usri1_name = (LPWSTR)strBOINCProjectAccountUsername.c_str();
        ui1.usri1_password = (LPWSTR)strBOINCProjectAccountPassword.c_str();
        ui1.usri1_comment = _T("Account used to execute BOINC applications");
        ui1.usri1_priv = USER_PRIV_USER;
        ui1.usri1_flags = UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;

        nasReturnValue = NetUserAdd(
            NULL,
            1,
            (LPBYTE)&ui1,
            &dwParameterError
        );

        if (NERR_Success != nasReturnValue) {
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
