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

class CASaveSetupState : public BOINCCABase {
public:
    virtual ~CASaveSetupState() = default;
    explicit CASaveSetupState(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CASaveSetupState"),
            _T("Save current setup parameters")) {
    }

    UINT CASaveSetupState::OnExecution() override final {
        tstring strInstallDirectory;
        GetProperty(_T("INSTALLDIR"), strInstallDirectory);
        SetRegistryValue(_T("INSTALLDIR"), strInstallDirectory);

        tstring strDataDirectory;
        GetProperty(_T("DATADIR"), strDataDirectory);
        SetRegistryValue(_T("DATADIR"), strDataDirectory);

        tstring strLaunchProgram;
        GetProperty(_T("LAUNCHPROGRAM"), strLaunchProgram);
        if (_T("1") == strLaunchProgram) {
            SetRegistryValue(_T("LAUNCHPROGRAM"), _T("1"));
        }
        else {
            SetRegistryValue(_T("LAUNCHPROGRAM"), _T("0"));
        }

        tstring strBOINCMasterAccountUsername;
        GetProperty(_T("BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);
        SetRegistryValue(_T("BOINC_MASTER_USERNAME"),
            strBOINCMasterAccountUsername);

        tstring strBOINCProjectAccountUsername;
        GetProperty(_T("BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);
        SetRegistryValue(_T("BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);

        tstring strEnableLaunchAtLogon;
        GetProperty(_T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon);
        if (_T("1") == strEnableLaunchAtLogon) {
            SetRegistryValue(_T("ENABLELAUNCHATLOGON"), _T("1"));
        }
        else {
            SetRegistryValue(_T("ENABLELAUNCHATLOGON"), _T("0"));
        }

        tstring strEnableScreensaver;
        GetProperty(_T("ENABLESCREENSAVER"), strEnableScreensaver);
        if (_T("1") == strEnableScreensaver) {
            SetRegistryValue(_T("ENABLESCREENSAVER"), _T("1"));
        }
        else {
            SetRegistryValue(_T("ENABLESCREENSAVER"), _T("0"));
        }

        tstring strEnableProtectedApplicationExecution;
        GetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
            strEnableProtectedApplicationExecution);
        if (_T("1") == strEnableProtectedApplicationExecution) {
            SetRegistryValue(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
                _T("1"));
        }
        else {
            SetRegistryValue(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
                _T("0"));
        }

        tstring strEnableUseByAllUsers;
        GetProperty(_T("ENABLEUSEBYALLUSERS"), strEnableUseByAllUsers);
        if (_T("1") == strEnableUseByAllUsers) {
            SetRegistryValue(_T("ENABLEUSEBYALLUSERS"), _T("1"));
        }
        else {
            SetRegistryValue(_T("ENABLEUSEBYALLUSERS"), _T("0"));
        }

        SetRegistryValue(_T("SETUPSTATESTORED"), _T("TRUE"));

        return ERROR_SUCCESS;
    }
};

UINT __stdcall SaveSetupState(MSIHANDLE hInstall) {
    return CASaveSetupState(hInstall).Execute();
}
