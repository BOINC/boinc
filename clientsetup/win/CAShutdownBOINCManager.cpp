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
#include "terminate.h"

class CAShutdownBOINCManager : public BOINCCABase {
public:
    virtual ~CAShutdownBOINCManager() = default;
    explicit CAShutdownBOINCManager(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAShutdownBOINCManager"),
            _T("Shutting down running instances of BOINC Manager")) {
    }

    UINT OnExecution() override final {
        const auto WM_TASKBARSHUTDOWN =
            ::RegisterWindowMessage(_T("TaskbarShutdown"));

        TerminateProcessEx(_T("boincmgr.exe"), false);
        auto terminateResult = false;
        for (auto attempt = 0u; attempt <= 5u; ++attempt) {
            auto hWndBOINCManagerSystray =
                FindWindow(_T("wxTaskBarExWindowClass"),
                    _T("BOINCManagerSystray"));

            if (hWndBOINCManagerSystray == nullptr) {
                terminateResult = true;
                break;
            }

            if (hWndBOINCManagerSystray != nullptr) {
                constexpr auto szWindowTitleSize = 256;
                TCHAR szWindowTitle[szWindowTitleSize];

                if (GetWindowText(hWndBOINCManagerSystray, szWindowTitle,
                    szWindowTitleSize) == 0) {
                    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, GetLastError(),
                        _T("Setup was unable to get the window title of the "
                            "BOINC Manager Systray window."));
                    break;
                }
                LogProgress(szWindowTitle);

                const auto result = SendMessage(hWndBOINCManagerSystray,
                    WM_TASKBARSHUTDOWN, NULL, NULL);
                if (result != 0) {
                    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
                        static_cast<int>(result), _T("Setup was unable to "
                            "shutdown the BOINC Manager Systray window."));
                    return ERROR_INSTALL_FAILURE;
                }
                Sleep(1000);
            }
        }

        if (!terminateResult) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                _T("One or more BOINC Manager applications "
                    "could not be closed, terminating process(s)."));
            return ERROR_INSTALL_FAILURE;
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall ShutdownBOINCManager(MSIHANDLE hInstall) {
    return CAShutdownBOINCManager(hInstall).Execute();
}
