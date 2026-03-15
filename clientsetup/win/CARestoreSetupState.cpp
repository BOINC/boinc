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

class CARestoreSetupState : public BOINCCABase {
public:
    virtual ~CARestoreSetupState() = default;
    explicit CARestoreSetupState(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CARestoreSetupState"),
            _T("Restore the previous setups saved parameters.")) {
    }

    UINT OnExecution() override final {
        tstring strSetupStateStored;
        GetRegistryValue(_T("SETUPSTATESTORED"), strSetupStateStored);
        if (strSetupStateStored == _T("TRUE")) {
            tstring strOverrideInstallDirectory;
            GetProperty(_T("OVERRIDE_INSTALLDIR"),
                strOverrideInstallDirectory);
            if (!strOverrideInstallDirectory.empty()) {
                SetProperty(_T("INSTALLDIR"), strOverrideInstallDirectory);
            }
            else {
                tstring strInstallDirectory;
                GetRegistryValue(_T("INSTALLDIR"), strInstallDirectory);
                SetProperty(_T("INSTALLDIR"), strInstallDirectory);
            }

            tstring strOverrideDataDirectory;
            GetProperty(_T("OVERRIDE_DATADIR"), strOverrideDataDirectory);
            if (!strOverrideDataDirectory.empty()) {
                SetProperty(_T("DATADIR"), strOverrideDataDirectory);
            }
            else {
                tstring strDataDirectory;
                GetRegistryValue(_T("DATADIR"), strDataDirectory);
                SetProperty(_T("DATADIR"), strDataDirectory);
            }

            tstring strOverrideLaunchProgram;
            GetProperty(_T("OVERRIDE_LAUNCHPROGRAM"),
                strOverrideLaunchProgram);
            if (!strOverrideLaunchProgram.empty()) {
                SetProperty(_T("LAUNCHPROGRAM"), strOverrideLaunchProgram);
            }
            else {
                tstring strLaunchProgram;
                GetRegistryValue(_T("LAUNCHPROGRAM"), strLaunchProgram);
                if (strLaunchProgram == _T("1") ||
                    strLaunchProgram.empty()) {
                    SetProperty(_T("LAUNCHPROGRAM"), _T("1"));
                }
                else {
                    SetProperty(_T("LAUNCHPROGRAM"), _T(""));
                }
            }

            tstring strOverrideBOINCMasterAccountUsername;
            GetProperty(_T("OVERRIDE_BOINC_MASTER_USERNAME"),
                strOverrideBOINCMasterAccountUsername);
            if (!strOverrideBOINCMasterAccountUsername.empty()) {
                SetProperty(_T("BOINC_MASTER_USERNAME"),
                    strOverrideBOINCMasterAccountUsername);
            }
            else {
                tstring strBOINCMasterAccountUsername;
                GetRegistryValue(_T("BOINC_MASTER_USERNAME"),
                    strBOINCMasterAccountUsername);
                SetProperty(_T("BOINC_MASTER_USERNAME"),
                    strBOINCMasterAccountUsername);
            }

            tstring strOverrideBOINCProjectAccountUsername;
            GetProperty(_T("OVERRIDE_BOINC_PROJECT_USERNAME"),
                strOverrideBOINCProjectAccountUsername);
            if (!strOverrideBOINCProjectAccountUsername.empty()) {
                SetProperty(_T("BOINC_PROJECT_USERNAME"),
                    strOverrideBOINCProjectAccountUsername);
            }
            else {
                tstring strBOINCProjectAccountUsername;
                GetRegistryValue(_T("BOINC_PROJECT_USERNAME"),
                    strBOINCProjectAccountUsername);
                SetProperty(_T("BOINC_PROJECT_USERNAME"),
                    strBOINCProjectAccountUsername);
            }

            tstring strOverrideEnableLaunchAtLogon;
            GetProperty(_T("OVERRIDE_ENABLELAUNCHATLOGON"),
                strOverrideEnableLaunchAtLogon);
            if (!strOverrideEnableLaunchAtLogon.empty()) {
                SetProperty(_T("ENABLELAUNCHATLOGON"),
                    strOverrideEnableLaunchAtLogon);
            }
            else {
                tstring strEnableLaunchAtLogon;
                GetRegistryValue(_T("ENABLELAUNCHATLOGON"),
                    strEnableLaunchAtLogon);
                if (strEnableLaunchAtLogon == _T("1")) {
                    SetProperty(_T("ENABLELAUNCHATLOGON"), _T("1"));
                }
                else {
                    SetProperty(_T("ENABLELAUNCHATLOGON"), _T(""));
                }
            }

            tstring strOverrideEnableScreensaver;
            GetProperty(_T("OVERRIDE_ENABLESCREENSAVER"),
                strOverrideEnableScreensaver);
            if (!strOverrideEnableScreensaver.empty()) {
                SetProperty(_T("ENABLESCREENSAVER"),
                    strOverrideEnableScreensaver);
            }
            else {
                tstring strEnableScreensaver;
                GetRegistryValue(_T("ENABLESCREENSAVER"),
                    strEnableScreensaver);
                if (strEnableScreensaver == _T("1")) {
                    SetProperty(_T("ENABLESCREENSAVER"), _T("1"));
                }
                else {
                    SetProperty(_T("ENABLESCREENSAVER"), _T(""));
                }
            }

            tstring strOverrideEnableProtectedApplicationExecution;
            GetProperty(_T("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
                strOverrideEnableProtectedApplicationExecution);
            if (!strOverrideEnableProtectedApplicationExecution.empty()) {
                SetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
                    strOverrideEnableProtectedApplicationExecution);
            }
            else {
                tstring strEnableProtectedApplicationExecution;
                GetRegistryValue(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
                    strEnableProtectedApplicationExecution);
                if (strEnableProtectedApplicationExecution == _T("1")) {
                    SetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
                        _T("1"));
                }
                else {
                    SetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
                        _T(""));
                }
            }

            tstring strOverrideEnableUseByAllUsers;
            GetProperty(_T("OVERRIDE_ENABLEUSEBYALLUSERS"),
                strOverrideEnableUseByAllUsers);
            if (!strOverrideEnableUseByAllUsers.empty()) {
                SetProperty(_T("ENABLEUSEBYALLUSERS"),
                    strOverrideEnableUseByAllUsers);
                SetProperty(_T("ALLUSERS"), strOverrideEnableUseByAllUsers);
            }
            else {
                tstring strEnableUseByAllUsers;
                GetRegistryValue(_T("ENABLEUSEBYALLUSERS"),
                    strEnableUseByAllUsers);
                if (_T("1") == strEnableUseByAllUsers) {
                    SetProperty(_T("ENABLEUSEBYALLUSERS"), _T("1"));
                    SetProperty(_T("ALLUSERS"), _T("1"));
                }
                else {
                    SetProperty(_T("ENABLEUSEBYALLUSERS"), _T(""));
                    SetProperty(_T("ALLUSERS"), _T(""));
                }
            }
        }

        tstring strDataDirectory;
        GetProperty(_T("DATADIR"), strDataDirectory);
        if (strDataDirectory.empty()) {
            tstring strCommonApplicationDataFolder;
            GetProperty(_T("CommonAppDataFolder"),
                strCommonApplicationDataFolder);
            strDataDirectory = strCommonApplicationDataFolder + _T("BOINC\\");
            SetProperty(_T("DATADIR"), strDataDirectory);
        }

        if (IsUpgrading()) {
            SetProperty(_T("IS_MAJOR_UPGRADE"), _T("Yes"));
        }
        return ERROR_SUCCESS;
    }
};

UINT __stdcall RestoreSetupState(MSIHANDLE hInstall) {
    return CARestoreSetupState(hInstall).Execute();
}
