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
#include "dcomperm.h"

class CAGrantBOINCAdminsVirtualBoxRights : public BOINCCABase {
public:
    virtual ~CAGrantBOINCAdminsVirtualBoxRights() = default;
    explicit CAGrantBOINCAdminsVirtualBoxRights(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAGrantBOINCAdminsVirtualBoxRights"),
            _T("Validating the BOINC Administrators "
                "VirtualBox access rights")) {
    }

    UINT OnExecution() override final {
        auto result =
            ChangeAppIDAccessACL(_T("{819B4D85-9CEE-493C-B6FC-64FFE759B3C9}"),
            _T("boinc_admins"), TRUE, TRUE);
        if (result != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("Failed call to ChangeAppIDAccessACL"));
            return result;
        }

        result =
            ChangeAppIDLaunchACL(_T("{819B4D85-9CEE-493C-B6FC-64FFE759B3C9}"),
            _T("boinc_admins"), TRUE,TRUE);
        if (result != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("Failed call to ChangeAppIDLaunchACL"));
            return result;
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall GrantBOINCAdminsVirtualBoxRights(MSIHANDLE hInstall) {
    return CAGrantBOINCAdminsVirtualBoxRights(hInstall).Execute();
}
