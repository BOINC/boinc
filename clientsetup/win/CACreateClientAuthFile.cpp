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

class CACreateClientAuthFile : public BOINCCABase {
public:
    virtual ~CACreateClientAuthFile() = default;

    explicit CACreateClientAuthFile(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CACreateClientAuthFile"),
            _T("Store client authorization data")) {
    }

    UINT OnExecution() override final {
        tstring strDataDirectory;
        auto uiReturnValue = GetProperty(_T("DATADIR"), strDataDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strDataDirectory.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The data directory is empty."));
            return ERROR_INSTALL_FAILURE;
        }
        if (!std::filesystem::exists(strDataDirectory)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The data directory doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strEnableProtectedApplicationExecution;
        uiReturnValue = GetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
            strEnableProtectedApplicationExecution);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        const auto bProtectedAppExecEnabled =
            (strEnableProtectedApplicationExecution == _T("1"));

        tstring strBOINCProjectAccountUsername;
        uiReturnValue = GetProperty(_T("BOINC_PROJECT_ISUSERNAME"),
            strBOINCProjectAccountUsername);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (bProtectedAppExecEnabled &&
            strBOINCProjectAccountUsername.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The 'boinc_project' account username is empty."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strBOINCProjectAccountPassword;
        uiReturnValue = GetProperty(_T("BOINC_PROJECT_PASSWORD"),
            strBOINCProjectAccountPassword);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (bProtectedAppExecEnabled &&
            strBOINCProjectAccountPassword.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The 'boinc_project' account password is empty."));
            return ERROR_INSTALL_FAILURE;
        }

        const auto strClientAuthFile =
            strDataDirectory + _T("\\client_auth.xml");

        // If we are not installing in protected mode, there may not
        //   be a valid 'boinc_project' account, so delete the
        //   client_auth.xml file if it exists.
        //
        if (!bProtectedAppExecEnabled)
        {
            if (std::filesystem::exists(strClientAuthFile)) {
                try {
                    if (std::filesystem::remove(strClientAuthFile)) {
                        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                            _T("The client_auth.xml file was "
                                "successfully deleted.")
                        );
                    }
                }
                catch (const std::filesystem::filesystem_error&) {
                    LogMessage(INSTALLMESSAGE_FATALEXIT, 0, 0, 0, 0,
                        _T("The client_auth.xml could not be deleted "
                            "from the data directory. Please delete the file "
                            "and rerun setup. ")
                    );
                    return ERROR_INSTALL_FAILURE;
                }
            }
            return ERROR_SUCCESS;
        }

        // We are installing in protected mode, which means the 'boinc_project'
        // account password has been changed, so we need to write out the new
        // username and password to the client_auth.xml file.
        auto dwSize = Base64EncodeGetRequiredLength(
            static_cast<int>(strBOINCProjectAccountPassword.size()));
        std::string szBuffer;
        szBuffer.resize(dwSize * sizeof(TCHAR), '\0');

        // Base 64 encode the 'boinc_project' account password
        //
        const auto lpszASCIIDecodedPassword =
            boinc_wide_to_ascii(strBOINCProjectAccountPassword.c_str());
        if (!Base64Encode(
            reinterpret_cast<const BYTE*>(lpszASCIIDecodedPassword.c_str()),
            static_cast<int>(lpszASCIIDecodedPassword.size()),
            szBuffer.data(), &dwSize, 0)) {
            LogMessage(INSTALLMESSAGE_FATALEXIT, 0, 0, 0, 0,
                _T("The 'boinc_project' account password failed to be "
                    "encoded."));
            return ERROR_INSTALL_FAILURE;
        }
        auto pszUnicodeEncodedPassword =
            boinc_ascii_to_wide(szBuffer.c_str());

        // trim line endings
        const auto trim = [](auto& str) {
            str.erase(std::remove(str.begin(), str.end(), '\n'), str.cend());
            };
        trim(strBOINCProjectAccountUsername);
        trim(pszUnicodeEncodedPassword);

        std::wofstream fClientAuthFile(strClientAuthFile);
        fClientAuthFile <<
            _T("<client_authorization>\n") <<
            _T("    <boinc_project>\n") <<
            _T("        <username>") << strBOINCProjectAccountUsername <<
            _T("</username>\n") <<
            _T("        <password>") << pszUnicodeEncodedPassword <<
            _T("</password>\n") <<
            _T("    </boinc_project>\n") <<
            _T("</client_authorization>\n");
        fClientAuthFile.close();
        return ERROR_SUCCESS;
    }
};

UINT __stdcall CreateClientAuthFile(MSIHANDLE hInstall) {
    return CACreateClientAuthFile(hInstall).Execute();
}
