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
#include "project_init.h"
#include "common_defs.h"
#include "win_util.h"

class CACreateProjectInitFile : public BOINCCABase {
public:
    virtual ~CACreateProjectInitFile() = default;
    explicit CACreateProjectInitFile(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CACreateProjectInitFile"),
            _T("Store project initialization data")) {
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

        tstring strProjectInitUrl;
        uiReturnValue = GetProperty(_T("PROJINIT_URL"), strProjectInitUrl);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }

        tstring project_name;
        uiReturnValue = GetProperty(_T("PROJINIT_NAME"), project_name);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }

        tstring strProjectInitAuthenticator;
        uiReturnValue = GetProperty(_T("PROJINIT_AUTH"), strProjectInitAuthenticator);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }

        tstring strSetupExeName;
        uiReturnValue = GetProperty(_T("SETUPEXENAME"), strSetupExeName);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }

        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
            _T("Changing to the data directory"));
        const auto prev_dir = std::filesystem::current_path();
        std::filesystem::current_path(strDataDirectory);

        // write project_init.xml if project info was passed on cmdline
        //
        if (!strProjectInitUrl.empty() &&
            !strProjectInitAuthenticator.empty()) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                _T("Detected command line parameters"));

            PROJECT_INIT pi;
            strncpy(pi.url, boinc_wide_to_ascii(strProjectInitUrl).c_str(),
                sizeof(pi.url) - 1);
            if (!project_name.empty()) {
                strncpy(pi.name, boinc_wide_to_ascii(project_name).c_str(),
                    sizeof(pi.name) - 1);
            }
            else {
                strncpy(pi.name,
                    boinc_wide_to_ascii(strProjectInitUrl).c_str(),
                    sizeof(pi.name) - 1);
            }
            strncpy(pi.account_key,
                boinc_wide_to_ascii(strProjectInitAuthenticator).c_str(),
                sizeof(pi.account_key) - 1);
            pi.embedded = false;
            pi.write();
        }

        if (!strSetupExeName.empty()) {
            // write installer filename to a file
            //
            std::ofstream f(ACCOUNT_DATA_FILENAME);
            f << boinc_wide_to_ascii(strSetupExeName);
            f.close();
        }

        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
            _T("Restoring previous directory"));
        std::filesystem::current_path(prev_dir);

        return ERROR_SUCCESS;
    }
};

UINT __stdcall CreateProjectInitFile(MSIHANDLE hInstall) {
    return CACreateProjectInitFile(hInstall).Execute();
}
