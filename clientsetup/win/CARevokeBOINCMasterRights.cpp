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
#include "lsaprivs.h"

class CARevokeBOINCMasterRights : public BOINCCABase {
public:
    virtual ~CARevokeBOINCMasterRights() = default;
    explicit CARevokeBOINCMasterRights(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CARevokeBOINCMasterRights"),
            _T("Validating BOINC Master's privileges")) {
    }

    UINT OnExecution() override final {
        tstring strBOINCMasterAccountUsername;
        const auto uiReturnValue =
            GetProperty(_T("BOINC_MASTER_USERNAME"),
                strBOINCMasterAccountUsername);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strBOINCMasterAccountUsername.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The BOINC_MASTER_USERNAME property is empty"));
            return ERROR_INSTALL_FAILURE;
        }

        PSID pSid;
        if (!GetAccountSid(nullptr, strBOINCMasterAccountUsername.c_str(),
            &pSid)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("Failed to be able to obtain the SID "
                    "for the selected user on the localhost"));
            return ERROR_INSTALL_FAILURE;
        }
        wil::unique_process_heap pSidDeleter(pSid);

        constexpr std::array rightsToRemove = {
            L"SeNetworkLogonRight",
            L"SeRemoteInteractiveLogonRight",
            L"SeBatchLogonRight",
            L"SeInteractiveLogonRight",
            L"SeServiceLogonRight",
            L"SeDenyNetworkLogonRight",
            L"SeDenyInteractiveLogonRight",
            L"SeDenyBatchLogonRight",
            L"SeDenyServiceLogonRight",
            L"SeDenyRemoteInteractiveLogonRight",
            L"SeTcbPrivilege",
            L"SeMachineAccountPrivilege",
            L"SeIncreaseQuotaPrivilege",
            L"SeBackupPrivilege",
            L"SeChangeNotifyPrivilege",
            L"SeSystemTimePrivilege",
            L"SeCreateTokenPrivilege",
            L"SeCreatePagefilePrivilege",
            L"SeCreateGlobalPrivilege",
            L"SeDebugPrivilege",
            L"SeEnableDelegationPrivilege",
            L"SeRemoteShutdownPrivilege",
            L"SeAuditPrivilege",
            L"SeImpersonatePrivilege",
            L"SeIncreaseBasePriorityPrivilege",
            L"SeLoadDriverPrivilege",
            L"SeLockMemoryPrivilege",
            L"SeSecurityPrivilege",
            L"SeSystemEnvironmentPrivilege",
            L"SeManageVolumePrivilege",
            L"SeProfileSingleProcessPrivilege",
            L"SeSystemProfilePrivilege",
            L"SeUndockPrivilege",
            L"SeAssignPrimaryTokenPrivilege",
            L"SeRestorePrivilege",
            L"SeShutdownPrivilege",
            L"SeSynchAgentPrivilege",
            L"SeTakeOwnershipPrivilege"
        };
        for (auto& right : rightsToRemove) {
            GrantUserRight(pSid, const_cast<wchar_t*>(right), FALSE);
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall RevokeBOINCMasterRights(MSIHANDLE hInstall) {
    return CARevokeBOINCMasterRights(hInstall).Execute();
}
