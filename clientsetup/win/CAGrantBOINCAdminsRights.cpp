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

class CAGrantBOINCAdminsRights : public BOINCCABase
{
public:
    virtual ~CAGrantBOINCAdminsRights() = default;
    explicit CAGrantBOINCAdminsRights(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAGrantBOINCAdminsRights"),
            _T("Validating the BOINC Administrators group privilege levels")) {
    }

    UINT OnExecution() override final {
        PSID pSid;
        if (!GetAccountSid(nullptr, L"boinc_admins", &pSid))
        {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("Failed to be able to obtain the SID "
                    "for the selected user on the localhost"));
            return ERROR_INSTALL_FAILURE;
        }
        wil::unique_process_heap pSidDeleter(pSid);

        constexpr std::array rightsToSet = {
            L"SeIncreaseQuotaPrivilege",
            L"SeChangeNotifyPrivilege",
            L"SeCreateGlobalPrivilege",
            L"SeAssignPrimaryTokenPrivilege"
        };
        for (auto& right : rightsToSet) {
            if (!GrantUserRight(pSid, const_cast<wchar_t*>(right), TRUE)) {
                LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                    _T("Failed call to GrantUserRight - ") + tstring(right));
                return ERROR_INSTALL_FAILURE;
            }
        }

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
            L"SeBackupPrivilege",
            L"SeSystemTimePrivilege",
            L"SeCreateTokenPrivilege",
            L"SeCreatePagefilePrivilege",
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

UINT __stdcall GrantBOINCAdminsRights(MSIHANDLE hInstall) {
    return CAGrantBOINCAdminsRights(hInstall).Execute();
}
