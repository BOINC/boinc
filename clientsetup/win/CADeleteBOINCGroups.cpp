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

class CADeleteBOINCGroups : public BOINCCABase {
public:
    virtual ~CADeleteBOINCGroups() = default;
    explicit CADeleteBOINCGroups(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CADeleteBOINCGroups"),
            _T("Validating user groups used by BOINC for secure sandboxes")) {
    }

    UINT OnExecution() override final {
        if (IsUpgrading()) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                _T("Upgrade detected, no need to delete groups"));
            return ERROR_SUCCESS;
        }

        auto nasReturnValue = NetLocalGroupDel(nullptr, _T("boinc_admins"));
        if ((nasReturnValue != NERR_Success) &&
            (nasReturnValue != ERROR_ALIAS_EXISTS) &&
            nasReturnValue != NERR_GroupNotFound) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                _T("NetLocalGroupDel retval"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                _T("Failed to delete the 'boinc_admins' group."));
            return ERROR_INSTALL_FAILURE;
        }

        nasReturnValue = NetLocalGroupDel(nullptr, _T("boinc_users"));
        if ((nasReturnValue != NERR_Success) &&
            (nasReturnValue != ERROR_ALIAS_EXISTS) &&
            nasReturnValue != NERR_GroupNotFound) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                _T("NetLocalGroupDel retval"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                _T("Failed to Delete the 'boinc_users' group."));
            return ERROR_INSTALL_FAILURE;
        }

        nasReturnValue = NetLocalGroupDel(nullptr, _T("boinc_projects"));
        if ((nasReturnValue != NERR_Success) &&
            (nasReturnValue != ERROR_ALIAS_EXISTS) &&
            nasReturnValue != NERR_GroupNotFound) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                _T("NetLocalGroupDel retval"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                _T("Failed to remove the 'boinc_projects' group."));
            return ERROR_INSTALL_FAILURE;
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall DeleteBOINCGroups(MSIHANDLE hInstall) {
    return CADeleteBOINCGroups(hInstall).Execute();
}
