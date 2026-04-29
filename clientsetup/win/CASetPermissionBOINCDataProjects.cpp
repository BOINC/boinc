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
#include "dirops.h"

class CASetPermissionBOINCDataProjects : public BOINCCABase {
public:
    virtual ~CASetPermissionBOINCDataProjects() = default;
    explicit CASetPermissionBOINCDataProjects(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CASetPermissionBOINCDataProjects"),
            _T("Setting permissions on the BOINC Projects directory.")) {
    }

    UINT CASetPermissionBOINCDataProjects::OnExecution() override final {
        tstring strEnableProtectedApplicationExecution;
        auto uiReturnValue =
            GetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
            strEnableProtectedApplicationExecution);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (_T("1") != strEnableProtectedApplicationExecution) {
            return ERROR_SUCCESS;
        }

        tstring strBOINCDataDirectory;
        uiReturnValue = GetProperty(_T("DATADIR"), strBOINCDataDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (!std::filesystem::exists(strBOINCDataDirectory)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The data directory doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        auto strBOINCDataProjectsDirectory =
            strBOINCDataDirectory + _T("\\projects");
        if (!std::filesystem::exists(strBOINCDataProjectsDirectory)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The projects directory doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strEnableUseByAllUsers;
        uiReturnValue = GetProperty(_T("ENABLEUSEBYALLUSERS"),
            strEnableUseByAllUsers);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        const auto enableUseByAllUsers = strEnableUseByAllUsers == _T("1");

        tstring strBOINCAdminsGroupAlias;
        uiReturnValue = GetProperty(_T("BOINC_ADMINS_GROUPNAME"),
            strBOINCAdminsGroupAlias);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strBOINCAdminsGroupAlias.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The BOINC Admins group alias is empty."));
            return ERROR_INSTALL_FAILURE;
        }
        if (!localGroupExists(strBOINCAdminsGroupAlias)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The BOINC Admins group doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strBOINCUsersGroupAlias;
        uiReturnValue = GetProperty(_T("BOINC_USERS_GROUPNAME"),
            strBOINCUsersGroupAlias);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strBOINCUsersGroupAlias.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The BOINC Users group alias is empty."));
            return ERROR_INSTALL_FAILURE;
        }
        if (!localGroupExists(strBOINCUsersGroupAlias)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The BOINC Users group doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strBOINCProjectsGroupAlias;
        uiReturnValue = GetProperty(_T("BOINC_PROJECTS_GROUPNAME"),
            strBOINCProjectsGroupAlias);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strBOINCProjectsGroupAlias.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The BOINC Projects group alias is empty."));
            return ERROR_INSTALL_FAILURE;
        }
        if (!localGroupExists(strBOINCProjectsGroupAlias)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The BOINC Projects group doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        DWORD sizeSystem = SECURITY_MAX_SID_SIZE;
        std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferSystem;
        if (!CreateWellKnownSid(WinLocalSystemSid, nullptr, &bufferSystem,
            &sizeSystem)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("CreateWellKnownSid Error for SYSTEM"));
            return ERROR_INSTALL_FAILURE;
        }

        DWORD sizeAdmins = SECURITY_MAX_SID_SIZE;
        std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferAdmins;
        if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr,
            &bufferAdmins, &sizeAdmins)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("CreateWellKnownSid Error for BUILTIN\\Administrators"));
            return ERROR_INSTALL_FAILURE;
        }

        DWORD sizeUsers = SECURITY_MAX_SID_SIZE;
        std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferUsers;
        if (enableUseByAllUsers) {
            if (!CreateWellKnownSid(WinBuiltinUsersSid, nullptr, &bufferUsers,
                &sizeUsers)) {
                LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                    _T("CreateWellKnownSid Error for BUILTIN\\Users"));
                return ERROR_INSTALL_FAILURE;
            }
        }

        std::vector<EXPLICIT_ACCESS> ea;
        ULONG ulEntries = enableUseByAllUsers ? 6 : 5;
        ea.resize(ulEntries);

        // SYSTEM
        ea[0].grfAccessPermissions = GENERIC_ALL;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName = reinterpret_cast<LPTSTR>(&bufferSystem);

        // Administrators
        ea[1].grfAccessPermissions = GENERIC_ALL;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[1].Trustee.ptstrName = reinterpret_cast<LPTSTR>(&bufferAdmins);

        // boinc_admins
        ea[2].grfAccessPermissions = GENERIC_ALL;
        ea[2].grfAccessMode = SET_ACCESS;
        ea[2].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[2].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[2].Trustee.ptstrName = strBOINCAdminsGroupAlias.data();

        // boinc_users
        ea[3].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
        ea[3].grfAccessMode = SET_ACCESS;
        ea[3].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[3].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[3].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[3].Trustee.ptstrName = strBOINCUsersGroupAlias.data();

        // boinc_projects
        ea[4].grfAccessPermissions = GENERIC_ALL;
        ea[4].grfAccessMode = SET_ACCESS;
        ea[4].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[4].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[4].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[4].Trustee.ptstrName = strBOINCProjectsGroupAlias.data();

        // Users
        if (enableUseByAllUsers) {
            ea[5].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
            ea[5].grfAccessMode = SET_ACCESS;
            ea[5].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
            ea[5].Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ea[5].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
            ea[5].Trustee.ptstrName = reinterpret_cast<LPTSTR>(&bufferUsers);
        }

        PACL pACL = nullptr;
        auto dwRes = SetEntriesInAcl(ulEntries, ea.data(), nullptr, &pACL);
        wil::unique_any<PACL, decltype(&::LocalFree), ::LocalFree>
            pACLGuard(pACL);
        if (dwRes != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, GetLastError(),
                _T("SetEntriesInAcl Error"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("SetEntriesInAcl Error"));
            return ERROR_INSTALL_FAILURE;
        }

        dwRes = SetNamedSecurityInfo(strBOINCDataProjectsDirectory.data(),
            SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
            NULL, NULL, pACL, NULL);
        if (dwRes != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, GetLastError(),
                _T("SetNamedSecurityInfo Error"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("SetNamedSecurityInfo Error"));
            return ERROR_INSTALL_FAILURE;
        }

        RecursiveSetPermissions(strBOINCDataProjectsDirectory, pACL);

        return ERROR_SUCCESS;
    }
};

UINT __stdcall SetPermissionBOINCDataProjects(MSIHANDLE hInstall) {
    return CASetPermissionBOINCDataProjects(hInstall).Execute();
}
