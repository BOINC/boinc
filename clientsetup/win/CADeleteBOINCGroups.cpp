// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

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
