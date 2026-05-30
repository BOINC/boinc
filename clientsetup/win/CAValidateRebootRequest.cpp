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

class CAValidateRebootRequest : public BOINCCABase {
public:
    virtual ~CAValidateRebootRequest() = default;
    explicit CAValidateRebootRequest(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAValidateRebootRequest"),
            _T("Validating any reboot requests.")) {
    }
private:
    UINT OnExecution() override final {
        tstring strInstallDirectory;
        const auto uiReturnValue = GetProperty(_T("INSTALLDIR"),
            strInstallDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strInstallDirectory.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The install directory is not set."));
            return ERROR_INSTALL_FAILURE;
        }
        if (!std::filesystem::exists(strInstallDirectory)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The install directory does not exist: ") +
                strInstallDirectory);
            return ERROR_INSTALL_FAILURE;
        }

        const auto strRebootPendingFilename =
            strInstallDirectory + _T("\\RebootPending.txt");

        std::wofstream rebootPendingFile(strRebootPendingFilename);
        rebootPendingFile.close();
        // Schedule the file for deletion after a reboot.
        //
        if (!MoveFileEx(strRebootPendingFilename.c_str(), NULL,
            MOVEFILE_DELAY_UNTIL_REBOOT)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("Failed to schedule file for removal after reboot"));
            return ERROR_INSTALL_FAILURE;
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall ValidateRebootRequest(MSIHANDLE hInstall) {
    return CAValidateRebootRequest(hInstall).Execute();
}
