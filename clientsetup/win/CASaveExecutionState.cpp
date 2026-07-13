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

class CASaveExecutionState : public BOINCCABase {
public:
    virtual ~CASaveExecutionState() = default;
    explicit CASaveExecutionState(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CASaveExecutionState"),
            _T("Save current execution parameters")) {
    }
private:
    UINT OnExecution() override final {
        tstring strLaunchProgram;
        GetProperty(_T("LAUNCHPROGRAM"), strLaunchProgram);
        SetRegistryValue(_T("LAUNCHPROGRAM"), strLaunchProgram);

        tstring strReturnRebootRequested;
        GetProperty(_T("RETURN_REBOOTREQUESTED"),
            strReturnRebootRequested);
        SetRegistryValue(_T("RETURN_REBOOTREQUESTED"),
            strReturnRebootRequested);

        tstring strReturnValidateInstall;
        GetProperty(_T("RETURN_VALIDATEINSTALL"),
            strReturnValidateInstall);
        SetRegistryValue(_T("RETURN_VALIDATEINSTALL"),
            strReturnValidateInstall);

        tstring strBOINCMasterAccountUsername;
        GetProperty(_T("BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);
        SetRegistryValue(_T("RETURN_BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);

        tstring strBOINCProjectAccountUsername;
        GetProperty(_T("BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);
        SetRegistryValue(_T("RETURN_BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);

        // Disable the ability to launch BOINC Manager if either a reboot is
        //   requested or validation of the installation executables fails.
        if ((strReturnRebootRequested == _T("1")) ||
            (strReturnValidateInstall == _T("0"))) {
            SetRegistryValue(_T("LAUNCHPROGRAM"), _T(""));
        }

        tstring strLaunchBOINCWslImageInstaller;
        GetProperty(_T("LAUNCHWSLIMAGEINSTALLER"),
            strLaunchBOINCWslImageInstaller);
        SetRegistryValue(_T("LAUNCHWSLIMAGEINSTALLER"),
            strLaunchBOINCWslImageInstaller);

        return ERROR_SUCCESS;
    }
};

UINT __stdcall SaveExecutionState(MSIHANDLE hInstall) {
    return CASaveExecutionState(hInstall).Execute();
}
