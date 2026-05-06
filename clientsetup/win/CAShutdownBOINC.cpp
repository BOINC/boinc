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

class CAShutdownBOINC : public BOINCCABase {
public:
    virtual ~CAShutdownBOINC() = default;
    explicit CAShutdownBOINC(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAShutdownBOINC"),
            _T("Shutting down running instances of BOINC")) {
    }

    UINT OnExecution() override final {
        auto schSCManager = OpenSCManager(nullptr, nullptr, GENERIC_READ);
        if (schSCManager) {
            wil::unique_schandle scmHandle(schSCManager);
            auto schService = OpenService(schSCManager, _T("BOINC"),
                GENERIC_READ | GENERIC_EXECUTE);

            if (schService) {
                wil::unique_schandle serviceHandle(schService);
                SERVICE_STATUS ssStatus = {};
                if (QueryServiceStatus(schService, &ssStatus)) {
                    if ((ssStatus.dwCurrentState != SERVICE_STOPPED) &&
                        (ssStatus.dwCurrentState != SERVICE_STOP_PENDING)) {
                        ControlService(schService, SERVICE_CONTROL_STOP,
                            &ssStatus);
                    }
                }
            }
        }

        TerminateProcessEx(_T("boinc.exe"));
        TerminateProcessEx(_T("boinctray.exe"));
        return ERROR_SUCCESS;
    }
};

UINT __stdcall ShutdownBOINC(MSIHANDLE hInstall) {
    return CAShutdownBOINC(hInstall).Execute();
}
