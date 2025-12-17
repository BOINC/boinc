// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#include "wil/resource.h"
#include "boinccas.h"
#include "CACreateBOINCGroups.h"
#include "lsaprivs.h"

CACreateBOINCGroups::CACreateBOINCGroups(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, _T("CACreateBOINCGroups"),
        _T("Validating user groups used by BOINC for secure sandboxes")) {
}

UINT CACreateBOINCGroups::OnExecution()
{
    DWORD            dwParameterError;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

    tstring strUserSID;
    auto uiReturnValue = GetProperty(_T("UserSID"), strUserSID);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }
    if (strUserSID.empty()) {
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
            _T("The installing user's SID is empty."));
        return ERROR_INSTALL_FAILURE;
    }

    tstring strEnableProtectedApplicationExecution;
    uiReturnValue = GetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
        strEnableProtectedApplicationExecution);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }
    const auto bProtectedAppExecEnabled =
        (strEnableProtectedApplicationExecution == _T("1"));

    tstring strUsersGroupName;
    uiReturnValue = GetProperty(_T("GROUPALIAS_USERS"), strUsersGroupName);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }
    if (bProtectedAppExecEnabled && strUsersGroupName.empty()) {
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
            _T("The 'Users' group alias is empty."));
        return ERROR_INSTALL_FAILURE;
    }

    tstring strBOINCMasterAccountUsername;
    uiReturnValue = GetProperty(_T("BOINC_MASTER_USERNAME"),
        strBOINCMasterAccountUsername);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }
    if (bProtectedAppExecEnabled && strBOINCMasterAccountUsername.empty()) {
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
            _T("The 'boinc_master' account username is empty."));
        return ERROR_INSTALL_FAILURE;
    }

    tstring strBOINCProjectAccountUsername;
    uiReturnValue = GetProperty(_T("BOINC_PROJECT_USERNAME"),
        strBOINCProjectAccountUsername);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }
    if (bProtectedAppExecEnabled && strBOINCProjectAccountUsername.empty()) {
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
            _T("The 'boinc_project' account username is empty."));
        return ERROR_INSTALL_FAILURE;
    }

    std::array<BYTE, SECURITY_MAX_SID_SIZE> sidBuf{};
    DWORD sidSize = SECURITY_MAX_SID_SIZE;
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, sidBuf.data(), &sidSize)) {
        return {};
    }
    wil::unique_sid pAdminSID(static_cast<PSID>(LocalAlloc(LMEM_FIXED, sidSize)));
    CopySid(sidSize, pAdminSID.get(), reinterpret_cast<PSID>(sidBuf.data()));

    if (!pAdminSID.is_valid()) {
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
            _T("AllocateAndInitializeSid Error for BUILTIN\\Administrators"));
        return ERROR_INSTALL_FAILURE;
    }

    PSID pInstallingUserSID = nullptr;
    if (!ConvertStringSidToSid(strUserSID.c_str(), &pInstallingUserSID)) {
        if (pInstallingUserSID) {
            LocalFree(pInstallingUserSID);
            pInstallingUserSID = nullptr;
        }
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
            _T("ConvertStringSidToSid Error for installing user"));
        return ERROR_INSTALL_FAILURE;
    }
    wil::unique_sid pInstallingUserSIDDeleter(pInstallingUserSID);

    LOCALGROUP_INFO_1 lgrpiUsers;
    lgrpiUsers.lgrpi1_name = _T("boinc_users");
    lgrpiUsers.lgrpi1_comment =
        _T("Accounts in this group can monitor the BOINC client.");

    auto nasReturnValue = NetLocalGroupAdd(nullptr, 1,
        reinterpret_cast<LPBYTE>(&lgrpiUsers), &dwParameterError);

    if ((nasReturnValue != NERR_Success) &&
        (nasReturnValue != ERROR_ALIAS_EXISTS)) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
            _T("NetLocalGroupAdd retval"));
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
            _T("Failed to create the 'boinc_users' group."));
        return ERROR_INSTALL_FAILURE;
    }

    auto bBOINCUsersCreated = false;
    if (nasReturnValue == NERR_Success) {
        bBOINCUsersCreated = true;
    }
    SetProperty(_T("BOINC_USERS_GROUPNAME"), _T("boinc_users"));

    LOCALGROUP_INFO_1 lgrpiProjects;
    lgrpiProjects.lgrpi1_name = _T("boinc_projects");
    lgrpiProjects.lgrpi1_comment =
        _T("Accounts in this group are used to execute boinc applications.");

    nasReturnValue = NetLocalGroupAdd(nullptr, 1,
        reinterpret_cast<LPBYTE>(&lgrpiProjects), &dwParameterError);

    if ((nasReturnValue != NERR_Success) &&
        (nasReturnValue != ERROR_ALIAS_EXISTS)) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
            _T("NetLocalGroupAdd retval"));
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
            _T("Failed to create the 'boinc_projects' group."));
        return ERROR_INSTALL_FAILURE;
    }

    auto bBOINCProjectsCreated = false;
    if (nasReturnValue == NERR_Success) {
        bBOINCProjectsCreated = true;
    }
    SetProperty(_T("BOINC_PROJECTS_GROUPNAME"), _T("boinc_projects"));

    LOCALGROUP_INFO_1 lgrpiAdmins;
    lgrpiAdmins.lgrpi1_name = _T("boinc_admins");
    lgrpiAdmins.lgrpi1_comment =
        _T("Accounts in this group can control the BOINC client.");

    nasReturnValue = NetLocalGroupAdd(nullptr, 1,
        reinterpret_cast<LPBYTE>(&lgrpiAdmins), &dwParameterError);

    if ((nasReturnValue != NERR_Success) &&
        (nasReturnValue != ERROR_ALIAS_EXISTS)) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
            _T("NetLocalGroupAdd retval"));
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
            _T("Failed to create the 'boinc_admins' group."));
        return ERROR_INSTALL_FAILURE;
    }

    auto bBOINCAdminsCreated = false;
    if (nasReturnValue == NERR_Success) {
        bBOINCAdminsCreated = true;
    }
    SetProperty(_T("BOINC_ADMINS_GROUPNAME"), _T("boinc_admins"));

    LOCALGROUP_MEMBERS_INFO_0 lgrmiAdmins;
    lgrmiAdmins.lgrmi0_sid = pAdminSID.get();

    nasReturnValue = NetLocalGroupAddMembers(nullptr, _T("boinc_admins"), 0,
        reinterpret_cast<LPBYTE>(&lgrmiAdmins), 1);

    if ((nasReturnValue != NERR_Success) &&
        (nasReturnValue != ERROR_MEMBER_IN_ALIAS)) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
            _T("NetLocalGroupAddMembers retval"));
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
            _T("Failed to add user to the 'boinc_admins' group "
                "(Administrator)."));
        return ERROR_INSTALL_FAILURE;
    }

    lgrmiAdmins.lgrmi0_sid = pInstallingUserSID;
    nasReturnValue = NetLocalGroupAddMembers(nullptr, _T("boinc_admins"), 0,
        reinterpret_cast<LPBYTE>(&lgrmiAdmins), 1);

    if ((nasReturnValue != NERR_Success) &&
        (nasReturnValue != ERROR_MEMBER_IN_ALIAS)) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
            _T("NetLocalGroupAddMembers retval"));
        LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
            _T("Failed to add user to the 'boinc_admins' group "
                "(Installing User)."));
        return ERROR_INSTALL_FAILURE;
    }

    if (bProtectedAppExecEnabled) {
        PSID pBOINCMasterSID = nullptr;
        if (!GetAccountSid(NULL, strBOINCMasterAccountUsername.c_str(),
            &pBOINCMasterSID))
        {
            if (pBOINCMasterSID) {
                FreeSid(pBOINCMasterSID);
                pBOINCMasterSID = nullptr;
            }
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("GetAccountSid Error for 'boinc_master' user account"));
            return ERROR_INSTALL_FAILURE;
        }
        wil::unique_sid pBOINCMasterSIDDeleter(pBOINCMasterSID);

        lgrmiAdmins.lgrmi0_sid = pBOINCMasterSID;
        nasReturnValue = NetLocalGroupAddMembers(nullptr, _T("boinc_admins"),
            0, reinterpret_cast<LPBYTE>(&lgrmiAdmins), 1);

        if ((nasReturnValue != NERR_Success) &&
            (nasReturnValue != ERROR_MEMBER_IN_ALIAS)) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                _T("NetLocalGroupAddMembers retval"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                _T("Failed to add user to the 'boinc_admins' group "
                    "(BOINC Master)."));
            return ERROR_INSTALL_FAILURE;
        }

        PSID pBOINCProjectSID = nullptr;
        if (!GetAccountSid(NULL, strBOINCProjectAccountUsername.c_str(),
            &pBOINCProjectSID)) {
            if (pBOINCProjectSID) {
                FreeSid(pBOINCProjectSID);
                pBOINCProjectSID = nullptr;
            }
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("GetAccountSid Error for 'boinc_master' user account"));
            return ERROR_INSTALL_FAILURE;
        }
        wil::unique_sid pBOINCProjectSIDDeleter(pBOINCProjectSID);

        LOCALGROUP_MEMBERS_INFO_0 lgrmiMembers;
        lgrmiMembers.lgrmi0_sid = pBOINCProjectSID;

        nasReturnValue = NetLocalGroupAddMembers(nullptr, _T("boinc_projects"),
            0, reinterpret_cast<LPBYTE>(&lgrmiMembers), 1);

        if ((nasReturnValue != NERR_Success) &&
            (nasReturnValue != ERROR_MEMBER_IN_ALIAS)) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                _T("NetLocalGroupAddMembers retval"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                _T("Failed to add user to the 'boinc_projects' group "
                    "(boinc_project)."));
            return ERROR_INSTALL_FAILURE;
        }

        nasReturnValue = NetLocalGroupAddMembers(nullptr,
            strUsersGroupName.c_str(), 0,
            reinterpret_cast<LPBYTE>(&lgrmiMembers), 1);

        if ((nasReturnValue != NERR_Success) &&
            (nasReturnValue != ERROR_MEMBER_IN_ALIAS)) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                _T("NetLocalGroupAddMembers retval"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                _T("Failed to add user to the 'Users' group "
                    "(boinc_project)."));
            return ERROR_INSTALL_FAILURE;
        }

        lgrmiMembers.lgrmi0_sid = pBOINCMasterSID;
        nasReturnValue = NetLocalGroupAddMembers(nullptr,
            strUsersGroupName.c_str(), 0,
            reinterpret_cast<LPBYTE>(&lgrmiMembers), 1);

        if ((nasReturnValue != NERR_Success) &&
            (nasReturnValue != ERROR_MEMBER_IN_ALIAS)) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, nasReturnValue,
                _T("NetLocalGroupAddMembers retval"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, nasReturnValue,
                _T("Failed to add user to the 'Users' group (boinc_master)."));
            return ERROR_INSTALL_FAILURE;
        }
    }

    if (bBOINCAdminsCreated || bBOINCUsersCreated || bBOINCProjectsCreated) {
        RebootWhenFinished();
    }

    return ERROR_SUCCESS;
}

UINT __stdcall CreateBOINCGroups(MSIHANDLE hInstall) {
    return CACreateBOINCGroups(hInstall).Execute();
}
