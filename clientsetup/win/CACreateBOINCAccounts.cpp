// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "boinccas.h"
#include "win_util.h"

class CACreateBOINCAccounts : public BOINCCABase {
public:
    virtual ~CACreateBOINCAccounts() = default;

    explicit CACreateBOINCAccounts(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CACreateBOINCAccounts"),
            _T("Validating user accounts used by BOINC "
                "for secure sandboxes")) {
    }

    UINT CACreateBOINCAccounts::OnExecution() override final {
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

        // Only create a new account or change the password on an existing
        // account if the user hasn't explicitly defined an account
        auto bCreateBOINCMasterAccount = false;
        if (strBOINCMasterAccountUsername.empty() &&
            strBOINCMasterAccountPassword.empty()) {
            bCreateBOINCMasterAccount = true;
        }
        if (strBOINCMasterAccountUsername == _T("boinc_master")) {
            bCreateBOINCMasterAccount = true;
        }

        if (strProductType == _T("2") && (strBOINCMasterAccountUsername ==
            (_T("boinc_master_") + strComputerName))) {
            bCreateBOINCMasterAccount = true;
        }

        auto bBOINCMasterAccountCreated = false;
        auto bBOINCMasterAccountModified = false;
        if (bCreateBOINCMasterAccount) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                _T("Using automatic account creation and management of "
                    "'boinc_master' account"));

            // Determine what the real values of the usernames should be
            // based off of the inputs
            if (strBOINCMasterAccountUsername.empty()) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Generating 'boinc_master' account name"));
                strBOINCMasterAccountUsername = _T("boinc_master");
                // Domain Controller
                if (strProductType == _T("2")) {
                    strBOINCMasterAccountUsername += _T("_") + strComputerName;
                }
            }

            // Generate random passwords if needed
            //
            if (strBOINCMasterAccountPassword.empty()) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Generating 'boinc_master' password"));
                auto errorcode = 0;
                std::tie(errorcode, strBOINCMasterAccountPassword)
                    = GenerateRandomPassword(32);
                if (strBOINCMasterAccountPassword.empty()) {
                    LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, errorcode,
                        _T("Failed to generate 'boinc_master' password"));
                    return ERROR_INSTALL_FAILURE + errorcode;
                }
                strBOINCMasterAccountPassword =
                    _T("!") + strBOINCMasterAccountPassword;
            }

            // Create the 'boinc_master' account if needed,
            // otherwise just update the password.
            //
            // Check if user exists
            LPUSER_INFO_0 pBuf = nullptr;
            const auto result = NetUserGetInfo(nullptr,
                strBOINCMasterAccountUsername.c_str(), 0,
                reinterpret_cast<LPBYTE*>(&pBuf));
            if (pBuf != nullptr) {
                NetApiBufferFree(pBuf);
            }
            if (result == NERR_Success) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Resetting 'boinc_master' password"));

                // Account already exists, just change the password
                //
                USER_INFO_1003 ui;
                ui.usri1003_password = strBOINCMasterAccountPassword.data();

                DWORD dwParameterError;
                const auto nasReturnValue = NetUserSetInfo(nullptr,
                    strBOINCMasterAccountUsername.c_str(), 1003,
                    reinterpret_cast<LPBYTE>(&ui), &dwParameterError);

                if (nasReturnValue != NERR_Success) {
                    LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                        nasReturnValue, _T("Failed to reset password on the "
                            "'boinc_master' account."));
                    return ERROR_INSTALL_FAILURE;
                }
            }
            else {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Creating 'boinc_master' account"));

                // Account does not exist, create it
                //
                USER_INFO_1 ui;
                ui.usri1_name = strBOINCMasterAccountUsername.data();
                ui.usri1_password = strBOINCMasterAccountPassword.data();
                ui.usri1_comment =
                    _T("Account used to execute BOINC as a system service");
                ui.usri1_priv = USER_PRIV_USER;
                ui.usri1_home_dir = nullptr;
                ui.usri1_comment = nullptr;
                ui.usri1_flags =
                    UF_SCRIPT | UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;
                ui.usri1_script_path = nullptr;

                DWORD dwParameterError;
                const auto nasReturnValue = NetUserAdd(nullptr, 1,
                    reinterpret_cast<LPBYTE>(&ui), &dwParameterError);

                if (nasReturnValue != NERR_Success) {
                    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                        _T("NetUserAdd retval"));
                    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, dwParameterError,
                        _T("NetUserAdd dwParameterError")
                    );
                    LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                        _T("Failed to create the 'boinc_master' account.")
                    );
                    return ERROR_INSTALL_FAILURE;
                }

                bBOINCMasterAccountCreated = true;
            }
            bBOINCMasterAccountModified = true;
        }

        // Only create a new account or change the password on an existing
        // account if the user hasn't explicitly defined an account
        auto bCreateBOINCProjectAccount = false;
        if (strBOINCProjectAccountUsername.empty() &&
            strBOINCProjectAccountPassword.empty()) {
            bCreateBOINCProjectAccount = true;
        }
        if (strBOINCProjectAccountUsername == _T("boinc_project")) {
            bCreateBOINCProjectAccount = true;
        }
        if (strProductType == _T("2") && (strBOINCProjectAccountUsername ==
            (_T("boinc_project_") + strComputerName))) {
            bCreateBOINCProjectAccount = true;
        }

        auto bBOINCProjectAccountCreated = false;
        auto bBOINCProjectAccountModified = false;
        if (bCreateBOINCProjectAccount) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                _T("Using automatic account creation and management of "
                    "'boinc_project' account"));

            // Determine what the real values of the usernames should be based
            // off of the inputs
            if (strBOINCProjectAccountUsername.empty()) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Generating 'boinc_project' account name"));
                strBOINCProjectAccountUsername = _T("boinc_project");
                // Domain Controller
                if (strProductType == _T("2")) {
                    strBOINCProjectAccountUsername +=
                        _T("_") + strComputerName;
                }
            }

            // Generate random passwords if needed
            //
            if (strBOINCProjectAccountPassword.empty()) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Generating 'boinc_project' password"));
                auto errorcode = 0;
                std::tie(errorcode, strBOINCProjectAccountPassword) =
                    GenerateRandomPassword(32);
                if (strBOINCProjectAccountPassword.empty()) {
                    LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, errorcode,
                        _T("Failed to generate 'boinc_project' password"));
                    return ERROR_INSTALL_FAILURE;
                }
                strBOINCProjectAccountPassword =
                    _T("!") + strBOINCProjectAccountPassword;
            }

            // Create the 'boinc_project' account if needed,
            // otherwise just update the password.
            //
            LPUSER_INFO_0 pBuf = nullptr;
            const auto result = NetUserGetInfo(nullptr,
                strBOINCProjectAccountUsername.c_str(), 0,
                reinterpret_cast<LPBYTE*>(&pBuf));
            if (pBuf != nullptr) {
                NetApiBufferFree(pBuf);
            }
            // Check if user exists
            if (result == NERR_Success) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Resetting 'boinc_project' password"));

                // Account already exists, just change the password
                //
                USER_INFO_1003 ui;
                ui.usri1003_password = strBOINCProjectAccountPassword.data();

                DWORD dwParameterError;
                const auto nasReturnValue = NetUserSetInfo(nullptr,
                    strBOINCProjectAccountUsername.c_str(), 1003,
                    reinterpret_cast<LPBYTE>(&ui), &dwParameterError);

                if (nasReturnValue != NERR_Success) {
                    LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                        _T("Failed to reset password on the "
                            "'boinc_project' account."));
                    return ERROR_INSTALL_FAILURE;
                }
            }
            else {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                    _T("Creating 'boinc_project' account"));

                // Account does not exist, create it
                //
                USER_INFO_1 ui;
                ui.usri1_name = strBOINCProjectAccountUsername.data();
                ui.usri1_password = strBOINCProjectAccountPassword.data();
                ui.usri1_comment =
                    _T("Account used to execute BOINC applications");
                ui.usri1_priv = USER_PRIV_USER;
                ui.usri1_home_dir = nullptr;
                ui.usri1_comment = nullptr;
                ui.usri1_flags =
                    UF_SCRIPT | UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;
                ui.usri1_script_path = nullptr;

                DWORD dwParameterError;
                const auto nasReturnValue = NetUserAdd(nullptr, 1,
                    reinterpret_cast<LPBYTE>(&ui), &dwParameterError);

                if (nasReturnValue != NERR_Success) {
                    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                        _T("NetUserAdd retval"));
                    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, dwParameterError,
                        _T("NetUserAdd dwParameterError"));
                    LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                        _T("Failed to create the 'boinc_project' account."));
                    return ERROR_INSTALL_FAILURE;
                }

                bBOINCProjectAccountCreated = true;
            }
            bBOINCProjectAccountModified = true;
        }


        SetProperty(_T("BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);
        if (bBOINCMasterAccountModified) {
            SetProperty(_T("BOINC_MASTER_ISUSERNAME"), _T(".\\") +
                strBOINCMasterAccountUsername);
        }
        else {
            SetProperty(_T("BOINC_MASTER_ISUSERNAME"),
                strBOINCMasterAccountUsername);
        }
        SetProperty(_T("BOINC_MASTER_PASSWORD"),
            strBOINCMasterAccountPassword, false);

        SetProperty(_T("BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);
        if (bBOINCProjectAccountModified) {
            SetProperty(_T("BOINC_PROJECT_ISUSERNAME"),
                _T(".\\") + strBOINCProjectAccountUsername);
        }
        else {
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

private:
    // Source Code Originally from:
    // http://support.microsoft.com/kb/814463
    std::pair<int, tstring> CACreateBOINCAccounts::GenerateRandomPassword(
        size_t desiredLength)
    {
        HCRYPTPROV prov = NULL;
        if (!CryptAcquireContext(&prov, nullptr, nullptr, PROV_RSA_FULL,
            CRYPT_SILENT)) {
            return { 1, {} };
        }
        wil::unique_hcryptprov hProv(prov);

        const auto dwBufSize = desiredLength * 4;
        std::vector<BYTE> randomBuffer(dwBufSize);
        if (!CryptGenRandom(hProv.get(), static_cast<DWORD>(dwBufSize),
            randomBuffer.data())) {
            return { 2, {} };
        }

        HCRYPTHASH hash = NULL;
        if (!CryptCreateHash(hProv.get(), CALG_SHA1, 0, 0, &hash)) {
            return { 3, {} };
        }
        wil::unique_hcrypthash hHash(hash);

        if (!CryptHashData(hHash.get(), randomBuffer.data(),
            static_cast<DWORD>(dwBufSize), 0)) {
            return { 4, {} };
        }

        auto dwSize = Base64EncodeGetRequiredLength(
            static_cast<int>(dwBufSize));
        std::string encodedString(dwSize, '\0');
        if (!Base64Encode(randomBuffer.data(), static_cast<int>(dwBufSize),
            encodedString.data(), &dwSize, 0)) {
            return { 5, {} };
        }

        auto randomPwd = boinc_ascii_to_wide(encodedString);
        tstring resultPwd(desiredLength, _T('\0'));
        randomPwd.copy(resultPwd.data(), desiredLength);
        return { 0, resultPwd };
    }
};

UINT __stdcall CreateBOINCAccounts(MSIHANDLE hInstall) {
    return CACreateBOINCAccounts(hInstall).Execute();
}
