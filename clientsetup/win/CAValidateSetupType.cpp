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

class CAValidateSetupType : public BOINCCABase {
public:
    virtual ~CAValidateSetupType() = default;
    explicit CAValidateSetupType(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAValidateSetupType"),
            _T("Validating setup type parameters")) {
    }
private:
    UINT OnExecution() override final {
        tstring strInstallDirectory;
        GetProperty(_T("INSTALLDIR"), strInstallDirectory);
        if (strInstallDirectory.empty()) {
            tstring strProgramFiles64Folder;
            GetProperty(_T("ProgramFiles64Folder"), strProgramFiles64Folder);
            strInstallDirectory = strProgramFiles64Folder + _T("BOINC\\");
            SetProperty(_T("INSTALLDIR"), strInstallDirectory);
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

        return ERROR_SUCCESS;
    }
};

UINT __stdcall ValidateSetupType(MSIHANDLE hInstall) {
    return CAValidateSetupType(hInstall).Execute();
}
