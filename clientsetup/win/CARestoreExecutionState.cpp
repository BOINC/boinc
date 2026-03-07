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

class CARestoreExecutionState : public BOINCCABase {
public:
    virtual ~CARestoreExecutionState() = default;
    explicit CARestoreExecutionState(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CARestoreExecutionState"),
            _T("Restore previous execution state")) {
    }

    UINT OnExecution() override final {
        tstring strLaunchProgram;
        GetRegistryValue(_T("LAUNCHPROGRAM"),
            strLaunchProgram);
        SetProperty(_T("LAUNCHPROGRAM"),
            strLaunchProgram);

        tstring strReturnRebootRequested;
        GetRegistryValue(_T("RETURN_REBOOTREQUESTED"),
            strReturnRebootRequested);
        SetProperty(_T("RETURN_REBOOTREQUESTED"),
            strReturnRebootRequested);

        tstring strReturnValidateInstall;
        GetRegistryValue(_T("RETURN_VALIDATEINSTALL"),
            strReturnValidateInstall);
        SetProperty(_T("RETURN_VALIDATEINSTALL"),
            strReturnValidateInstall);

        tstring strBOINCMasterAccountUsername;
        GetRegistryValue(_T("RETURN_BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);
        SetProperty(_T("BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);

        tstring strBOINCProjectAccountUsername;
        GetRegistryValue(_T("RETURN_BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);
        SetProperty(_T("BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);

        return ERROR_SUCCESS;
    }
};

UINT __stdcall RestoreExecutionState(MSIHANDLE hInstall) {
    return CARestoreExecutionState(hInstall).Execute();
}
