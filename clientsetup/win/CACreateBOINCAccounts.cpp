// Berkeley Open Infrastructure for Network Computing
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

CACreateBOINCAccounts::CACreateBOINCAccounts(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, _T("CACreateBOINCAccounts"),
        _T("Validating user accounts used by BOINC for secure sandboxes")) {}


UINT CACreateBOINCAccounts::OnExecution() {
    tstring strBOINCMasterAccountUsername;
    auto uiReturnValue =
        GetProperty(_T("BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    tstring strBOINCMasterAccountPassword;
    uiReturnValue =
        GetProperty(_T("BOINC_MASTER_PASSWORD"),
            strBOINCMasterAccountPassword);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    tstring strBOINCProjectAccountUsername;
    uiReturnValue =
        GetProperty(_T("BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    tstring strBOINCProjectAccountPassword;
    uiReturnValue =
        GetProperty(_T("BOINC_PROJECT_PASSWORD"),
            strBOINCProjectAccountPassword);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    tstring strComputerName;
    uiReturnValue = GetProperty(_T("ComputerName"), strComputerName);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    tstring strProductType;
    uiReturnValue = GetProperty(_T("MsiNTProductType"), strProductType);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    tstring strEnableProtectedApplicationExecution;
    uiReturnValue =
        GetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
            strEnableProtectedApplicationExecution);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    // Only create a new account or change the password on an existing account
    //   if the user hasn't explicitly defined an account
    auto bCreateBOINCMasterAccount = false;
    if (strBOINCMasterAccountUsername.empty() &&
        strBOINCMasterAccountPassword.empty()) {
        bCreateBOINCMasterAccount = true;
    }
    if (strBOINCMasterAccountUsername == _T("boinc_master")) {
        bCreateBOINCMasterAccount = true;
    }

    if (strProductType == tstring(_T("2")) &&
        (strBOINCMasterAccountUsername ==
            (tstring(_T("boinc_master_")) + strComputerName))) {
        bCreateBOINCMasterAccount = true;
    }

    auto bBOINCMasterAccountCreated = false;
    auto bBOINCMasterAccountModified = false;
    if (bCreateBOINCMasterAccount) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("Using automatic account creation and management of "
                "'boinc_master' account")
        );

        // Determine what the real values of the usernames should be based off
        //   of the inputs
        //
        if (strBOINCMasterAccountUsername.empty()) {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Generating 'boinc_master' account name")
            );
            strBOINCMasterAccountUsername = _T("boinc_master");
            // Domain Controller
            if (strProductType == tstring(_T("2"))) {                    
                strBOINCMasterAccountUsername += _T("_") + strComputerName;
            }
        }

        // Generate random passwords if needed
        //
        if (strBOINCMasterAccountPassword.empty()) {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Generating 'boinc_master' password")
            );
            GenerateRandomPassword(strBOINCMasterAccountPassword, 32);
            strBOINCMasterAccountPassword =
                _T("!") + strBOINCMasterAccountPassword;
        }

        // Create the 'boinc_master' account if needed, otherwise just update the password.
        //
        // Check if user exists
        PSID pSid;
        if(GetAccountSid(NULL, strBOINCMasterAccountUsername.c_str(), &pSid)) {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Resetting 'boinc_master' password")
            );

            // Account already exists, just change the password
            //
            USER_INFO_1003 ui;
            ui.usri1003_password =
                const_cast<LPWSTR>(
                    strBOINCMasterAccountPassword.c_str());

            DWORD dwParameterError;
            const auto nasReturnValue = NetUserSetInfo(
                NULL,
                strBOINCMasterAccountUsername.c_str(),
                1003,
                reinterpret_cast<LPBYTE>(&ui),
                &dwParameterError
            );

            if (nasReturnValue != NERR_Success) {
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL,
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("Failed to reset password on the 'boinc_master' "
                        "account.")
                );
                return ERROR_INSTALL_FAILURE;
            }
        } else {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Creating 'boinc_master' account")
            );

            // Account does not exist, create it
            //
            USER_INFO_1 ui;
            ui.usri1_name =
                const_cast<LPWSTR>(strBOINCMasterAccountUsername.c_str());
            ui.usri1_password =
                const_cast<LPWSTR>(strBOINCMasterAccountPassword.c_str());
            ui.usri1_comment =
                _T("Account used to execute BOINC as a system service");
            ui.usri1_priv = USER_PRIV_USER;
            ui.usri1_home_dir = NULL;
            ui.usri1_comment = NULL;
            ui.usri1_flags =
                UF_SCRIPT | UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;
            ui.usri1_script_path = NULL;

            DWORD dwParameterError;
            const auto nasReturnValue = NetUserAdd(
                NULL,
                1,
                reinterpret_cast<LPBYTE>(&ui),
                &dwParameterError
            );

            if (nasReturnValue != NERR_Success) {
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

            bBOINCMasterAccountCreated = true;
        }
        if(pSid != NULL) {
            HeapFree(GetProcessHeap(), 0, pSid);
            pSid = NULL;
        }

        bBOINCMasterAccountModified = false;
    }

    // Only create a new account or change the password on an existing account
    //   if the user hasn't explicitly defined an account
    auto bCreateBOINCProjectAccount = false;
    if (strBOINCProjectAccountUsername.empty() &&
        strBOINCProjectAccountPassword.empty()) {
        bCreateBOINCProjectAccount = true;
    }
    if (strBOINCProjectAccountUsername == _T("boinc_project")) {
        bCreateBOINCProjectAccount = true;
    }
    if (strProductType == tstring(_T("2")) &&
        (strBOINCProjectAccountUsername ==
            (tstring(_T("boinc_project_")) + strComputerName))) {
        bCreateBOINCProjectAccount = true;
    }

    auto bBOINCProjectAccountCreated = false;
    auto bBOINCProjectAccountModified = false;
    if (bCreateBOINCProjectAccount) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("Using automatic account creation and management of "
                "'boinc_project' account")
        );

        // Determine what the real values of the usernames should be based off
        //   of the inputs
        //
        if (strBOINCProjectAccountUsername.empty()) {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Generating 'boinc_project' account name")
            );
            strBOINCProjectAccountUsername = _T("boinc_project");
            // Domain Controller
            if (strProductType == tstring(_T("2"))) {
                strBOINCProjectAccountUsername += _T("_") + strComputerName;
            }
        }

        // Generate random passwords if needed
        //
        if (strBOINCProjectAccountPassword.empty()) {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Generating 'boinc_project' password")
            );
            GenerateRandomPassword(strBOINCProjectAccountPassword, 32);
            strBOINCProjectAccountPassword =
                _T("!") + strBOINCProjectAccountPassword;
        }

        // Create the 'boinc_project' account if needed,
        // otherwise just update the password.
        //
        PSID pSid;
        // Check if user exists
        if(GetAccountSid(NULL, strBOINCProjectAccountUsername.c_str(),
            &pSid)) {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Resetting 'boinc_project' password")
            );

            // Account already exists, just change the password
            //
            USER_INFO_1003 ui;
            ui.usri1003_password =
                const_cast<LPWSTR>(strBOINCProjectAccountPassword.c_str());

            DWORD dwParameterError;
            const auto nasReturnValue = NetUserSetInfo(
                NULL,
                strBOINCProjectAccountUsername.c_str(),
                1003,
                reinterpret_cast<LPBYTE>(&ui),
                &dwParameterError
            );

            if (nasReturnValue != NERR_Success) {
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL,
                    NULL,
                    NULL,
                    nasReturnValue,
                    _T("Failed to reset password on the 'boinc_project' "
                        "account.")
                );
                return ERROR_INSTALL_FAILURE;
            }
        } else {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL,
                NULL,
                NULL,
                NULL,
                _T("Creating 'boinc_project' account")
            );

            // Account does not exist, create it
            //
            USER_INFO_1 ui;
            ui.usri1_name =
                const_cast<LPWSTR>(strBOINCProjectAccountUsername.c_str());
            ui.usri1_password =
                const_cast<LPWSTR>(strBOINCProjectAccountPassword.c_str());
            ui.usri1_comment = _T("Account used to execute BOINC applications");
            ui.usri1_priv = USER_PRIV_USER;
            ui.usri1_home_dir = NULL;
            ui.usri1_comment = NULL;
            ui.usri1_flags =
                UF_SCRIPT | UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;
            ui.usri1_script_path = NULL;

            DWORD dwParameterError;
            const auto nasReturnValue = NetUserAdd(
                NULL,
                1,
                reinterpret_cast<LPBYTE>(&ui),
                &dwParameterError
            );

            if (nasReturnValue != NERR_Success) {
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

            bBOINCProjectAccountCreated = true;
        }
        if(pSid != NULL) {
            HeapFree(GetProcessHeap(), 0, pSid);
            pSid = NULL;
        }

        bBOINCProjectAccountModified = true;
    }


    SetProperty(_T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername);
    if (bBOINCMasterAccountModified) {
        SetProperty(_T("BOINC_MASTER_ISUSERNAME"), tstring(_T(".\\") +
            strBOINCMasterAccountUsername));
    } else {
        SetProperty(_T("BOINC_MASTER_ISUSERNAME"),
            strBOINCMasterAccountUsername);
    }
    SetProperty(_T("BOINC_MASTER_PASSWORD"),
        strBOINCMasterAccountPassword, false);

    SetProperty(_T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername);
    if (bBOINCProjectAccountModified) {
        SetProperty(_T("BOINC_PROJECT_ISUSERNAME"),
            tstring(_T(".\\") + strBOINCProjectAccountUsername));
    } else {
        SetProperty(_T("BOINC_PROJECT_ISUSERNAME"),
            strBOINCProjectAccountUsername);
    }
    SetProperty(_T("BOINC_PROJECT_PASSWORD"),
        strBOINCProjectAccountPassword, false);

    if (bBOINCMasterAccountCreated || bBOINCProjectAccountCreated) {
        RebootWhenFinished();
    }

    return ERROR_SUCCESS;
}

UINT __stdcall CreateBOINCAccounts(MSIHANDLE hInstall) {
    return CACreateBOINCAccounts(hInstall).Execute();
}
