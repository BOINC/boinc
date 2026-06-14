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

#include "boinccas.h"

class CAValidateInstall : public BOINCCABase {
public:
    virtual ~CAValidateInstall() = default;
    explicit CAValidateInstall(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAValidateInstall"),
            _T("Validating the install by checking all executables.")) {
    }
private:
    UINT OnExecution() override final {
        tstring strFilename;
        tstring strTemp;

        tstring strInstallDirectory;
        auto uiReturnValue =
            GetProperty(_T("INSTALLDIR"), strInstallDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strInstallDirectory.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The install directory is not set."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strProductVersion;
        uiReturnValue = GetProperty(_T("ProductVersion"), strProductVersion);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strProductVersion.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The product version is not set."));
            return ERROR_INSTALL_FAILURE;
        }

        // Default to success
        auto validateResult = true;

        constexpr auto components = std::array{
            _T("_BOINC"),
            _T("_BOINCManager"),
            _T("_BOINCCMD"),
            _T("_BOINCTray")
        };

        for (const auto& component : components) {
            if (ValidateComponent(
                component, strInstallDirectory, strProductVersion) !=
                ERROR_SUCCESS) {
                LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                    _T("Component validation failed for: ") +
                    tstring(component));
                validateResult = false;
            }
        }

        SetProperty(_T("RETURN_VALIDATEINSTALL"),
            validateResult ? _T("1") : _T("0"));

        return validateResult ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    }

    UINT ValidateComponent(const tstring& strComponent,
        const tstring& strDirectory, const tstring& strDesiredVersion) {
        tstring strExecutable;
        const auto uiReturnValue =
            GetComponentKeyFilename(strComponent, strExecutable);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strExecutable.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, _T("Component '") +
                strComponent + _T("' is missing a key file."));
            return ERROR_INSTALL_FAILURE;
        }

        strExecutable = strDirectory + _T("\\") + strExecutable;
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("Validating Executable: '") + strExecutable +
            _T("' Version: '") + strDesiredVersion + _T("'"));


        DWORD dwHandle;
        const auto dwSize = GetFileVersionInfoSize(strExecutable.c_str(),
            &dwHandle);
        if (dwSize == 0) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("Failed to get version info size for executable: '") +
                strExecutable + _T("'"));
            return ERROR_INSTALL_FAILURE;
        }

        auto lpData = reinterpret_cast<LPVOID>(malloc(dwSize));
        wil::unique_any<LPVOID, decltype(&::free), ::free>
            pDataDeleter(lpData);
        if (!GetFileVersionInfo(strExecutable.c_str(), dwHandle, dwSize,
            lpData)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("Failed to get version info for executable: '") +
                strExecutable + _T("'"));
            return ERROR_INSTALL_FAILURE;
        }
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("Successfully obtained version info for executable: '") +
            strExecutable + _T("'"));

        struct LANGANDCODEPAGE {
            WORD wLanguage;
            WORD wCodePage;
        } *lpTranslate;
        UINT uiVarSize;
        if (!VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"),
            reinterpret_cast<LPVOID*>(&lpTranslate), &uiVarSize)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("Failed to get version info translation "
                    "for executable: '") + strExecutable + _T("'"));
            return ERROR_INSTALL_FAILURE;
        }

        tostringstream ss;
        ss << _T("\\StringFileInfo\\") <<
            std::hex << std::setw(4) << std::setfill(_T('0')) <<
            lpTranslate[0].wLanguage <<
            std::hex << std::setw(4) << std::setfill(_T('0')) <<
            lpTranslate[0].wCodePage <<
            _T("\\ProductVersion");
        LPVOID lpVar;
        if (!VerQueryValue(lpData, ss.str().data(), &lpVar, &uiVarSize)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("Failed to get product version from version info for "
                    "executable: '") + strExecutable + _T("'"));
            return ERROR_INSTALL_FAILURE;
        }

        const auto productVersion = tstring(reinterpret_cast<wchar_t*>(lpVar));
        if (productVersion.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("Product version is empty in version info "
                    "for executable: '") + strExecutable + _T("'"));
            return ERROR_INSTALL_FAILURE;
        }

        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("Product Version: '") + productVersion + _T("'"));

        if (strDesiredVersion != productVersion) {
            return ERROR_INSTALL_FAILURE;
        }
        return ERROR_SUCCESS;
    }
};

UINT __stdcall ValidateInstall(MSIHANDLE hInstall) {
    return CAValidateInstall(hInstall).Execute();
}
