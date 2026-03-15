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

class CADeleteBOINCAccounts : public BOINCCABase {
public:
    virtual ~CADeleteBOINCAccounts() = default;
    explicit CADeleteBOINCAccounts(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CADeleteBOINCAccounts"),
            _T("Validating user accounts used by BOINC "
                "for secure sandboxes")) {
    }

    UINT OnExecution() override final {
        if (IsUpgrading()) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, 0,
                _T("Upgrade detected, no need to delete accounts"));
            return ERROR_SUCCESS;
        }


        tstring strBOINCMasterAccountUsername;
        auto uiReturnValue =
            GetProperty(_T("BOINC_MASTER_USERNAME"),
                strBOINCMasterAccountUsername);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (!strBOINCMasterAccountUsername.empty()) {
            const auto nasReturnValue = NetUserDel(nullptr,
                    strBOINCMasterAccountUsername.c_str());
            if (nasReturnValue != NERR_Success) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                    _T("NetUserDel retval"));
                LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                    _T("Failed to delete the 'boinc_master' account."));
                return ERROR_INSTALL_FAILURE;
            }
        }

        tstring strBOINCProjectAccountUsername;
        uiReturnValue = GetProperty(_T("BOINC_PROJECT_USERNAME"),
            strBOINCProjectAccountUsername);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (!strBOINCProjectAccountUsername.empty()) {
            const auto nasReturnValue = NetUserDel(nullptr,
                    strBOINCProjectAccountUsername.c_str());
            if (nasReturnValue != NERR_Success) {
                LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                    _T("NetUserDel retval"));
                LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                    _T("Failed to delete the 'boinc_project' account."));
                return ERROR_INSTALL_FAILURE;
            }
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall DeleteBOINCAccounts(MSIHANDLE hInstall) {
    return CADeleteBOINCAccounts(hInstall).Execute();
}
