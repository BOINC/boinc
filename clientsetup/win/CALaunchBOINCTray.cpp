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
#include "launcher.h"

class CALaunchBOINCTray : public BOINCCABase {
public:
    virtual ~CALaunchBOINCTray() = default;
    explicit CALaunchBOINCTray(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CALaunchBOINCTray"),
            _T("Launching BOINC Tray")) {
    }

    UINT OnExecution() override final {
        tstring strInstallDirectory;

        const auto uiReturnValue =
            GetProperty(_T("INSTALLDIR"), strInstallDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strInstallDirectory.empty()) {
            return ERROR_INSTALL_FAILURE;
        }

        auto strBuffer = tstring(_T("\"")) + strInstallDirectory +
            tstring(_T("\\boinctray.exe\""));
        const auto result = CreateProcessWithExplorerIL(nullptr,
            strBuffer.data());

        return result == S_OK ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    }
};

UINT __stdcall LaunchBOINCTray(MSIHANDLE hInstall) {
    return CALaunchBOINCTray(hInstall).Execute();
}
