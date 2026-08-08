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

class CARevokeBOINCUsersRights : public BOINCCABase {
public:
    virtual ~CARevokeBOINCUsersRights() = default;
    explicit CARevokeBOINCUsersRights(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CARevokeBOINCUsersRights"),
            _T("Validating the BOINC Users group privilege levels")) {
    }
private:
    UINT CARevokeBOINCUsersRights::OnExecution() override final {
        PSID pSid;
        if (!GetAccountSid(_T("boinc_users"), &pSid)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("Failed to be able to obtain the SID "
                    "for the selected user on the localhost"));
            return ERROR_INSTALL_FAILURE;
        }
        wil::unique_process_heap pSidDeleter(pSid);

        constexpr std::array rightsToRemove = {
            _T("SeNetworkLogonRight"),
            _T("SeRemoteInteractiveLogonRight"),
            _T("SeBatchLogonRight"),
            _T("SeInteractiveLogonRight"),
            _T("SeServiceLogonRight"),
            _T("SeDenyNetworkLogonRight"),
            _T("SeDenyInteractiveLogonRight"),
            _T("SeDenyBatchLogonRight"),
            _T("SeDenyServiceLogonRight"),
            _T("SeDenyRemoteInteractiveLogonRight"),
            _T("SeTcbPrivilege"),
            _T("SeMachineAccountPrivilege"),
            _T("SeIncreaseQuotaPrivilege"),
            _T("SeBackupPrivilege"),
            _T("SeChangeNotifyPrivilege"),
            _T("SeSystemTimePrivilege"),
            _T("SeCreateTokenPrivilege"),
            _T("SeCreatePagefilePrivilege"),
            _T("SeCreateGlobalPrivilege"),
            _T("SeDebugPrivilege"),
            _T("SeEnableDelegationPrivilege"),
            _T("SeRemoteShutdownPrivilege"),
            _T("SeAuditPrivilege"),
            _T("SeImpersonatePrivilege"),
            _T("SeIncreaseBasePriorityPrivilege"),
            _T("SeLoadDriverPrivilege"),
            _T("SeLockMemoryPrivilege"),
            _T("SeSecurityPrivilege"),
            _T("SeSystemEnvironmentPrivilege"),
            _T("SeManageVolumePrivilege"),
            _T("SeProfileSingleProcessPrivilege"),
            _T("SeSystemProfilePrivilege"),
            _T("SeUndockPrivilege"),
            _T("SeAssignPrimaryTokenPrivilege"),
            _T("SeRestorePrivilege"),
            _T("SeShutdownPrivilege"),
            _T("SeSynchAgentPrivilege"),
            _T("SeTakeOwnershipPrivilege")
        };
        for (auto right : rightsToRemove) {
            GrantUserRight(pSid, right, false);
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall RevokeBOINCUsersRights(MSIHANDLE hInstall) {
    return CARevokeBOINCUsersRights(hInstall).Execute();
}
