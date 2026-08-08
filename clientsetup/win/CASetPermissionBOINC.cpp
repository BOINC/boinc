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

class CASetPermissionBOINC : public BOINCCABase {
public:
    virtual ~CASetPermissionBOINC() = default;
    explicit CASetPermissionBOINC(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CASetPermissionBOINC"),
            _T("Setting permissions on the BOINC Executable directory.")) {
    }
private:
    UINT OnExecution() override final {
        tstring strEnableProtectedApplicationExecution;
        auto uiReturnValue =
            GetProperty(_T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"),
            strEnableProtectedApplicationExecution);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strEnableProtectedApplicationExecution != _T("1")) {
            return ERROR_SUCCESS;
        }

        tstring strBOINCInstallDirectory;
        uiReturnValue = GetProperty(_T("INSTALLDIR"),
            strBOINCInstallDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (!std::filesystem::exists(strBOINCInstallDirectory)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The install directory doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strBOINCAdminsGroupAlias;
        uiReturnValue = GetProperty(_T("BOINC_ADMINS_GROUPNAME"),
            strBOINCAdminsGroupAlias);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strBOINCAdminsGroupAlias.empty()) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The BOINC Admins group alias is empty."));
            return ERROR_INSTALL_FAILURE;
        }
        if(!localGroupExists(strBOINCAdminsGroupAlias)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
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
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The BOINC Users group alias is empty."));
            return ERROR_INSTALL_FAILURE;
        }
        if (!localGroupExists(strBOINCUsersGroupAlias)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0,
                _T("The BOINC Users group doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        tstring strEnableUseByAllUsers;
        uiReturnValue = GetProperty(_T("ENABLEUSEBYALLUSERS"),
            strEnableUseByAllUsers);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }

        const auto enableUseByAllUsers = strEnableUseByAllUsers == _T("1");
        const ULONG ulEntries = enableUseByAllUsers ? 3 : 2;
        std::vector<EXPLICIT_ACCESS> ea;
        ea.resize(ulEntries);

        ea[0].grfAccessPermissions = GENERIC_ALL;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[0].Trustee.ptstrName =
            reinterpret_cast<LPTSTR>(strBOINCAdminsGroupAlias.data());

        ea[1].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[1].Trustee.ptstrName =
            reinterpret_cast<LPTSTR>(strBOINCUsersGroupAlias.data());

        if (enableUseByAllUsers) {
            DWORD sizeAllUsers = SECURITY_MAX_SID_SIZE;
            std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferAllUsers;
            if (!CreateWellKnownSid(WinBuiltinUsersSid, nullptr,
                &bufferAllUsers, &sizeAllUsers)) {
                LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                    _T("CreateWellKnownSid Error for BUILTIN\\Users"));
                return ERROR_INSTALL_FAILURE;
            }

            ea[2].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
            ea[2].grfAccessMode = SET_ACCESS;
            ea[2].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
            ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ea[2].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
            ea[2].Trustee.ptstrName =
                reinterpret_cast<LPTSTR>(&bufferAllUsers);
        }


        PACL pOldACL = nullptr;
        PSECURITY_DESCRIPTOR pSD = nullptr;

        auto dwRes = GetNamedSecurityInfo(
            reinterpret_cast<LPWSTR>(strBOINCInstallDirectory.data()),
            SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr,
            &pOldACL, nullptr, &pSD);
        wil::unique_any<PSECURITY_DESCRIPTOR, decltype(&::LocalFree),
            ::LocalFree> pSDGuard(pSD);
        if (dwRes != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, GetLastError(),
                _T("GetNamedSecurityInfo Error"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("GetNamedSecurityInfo Error"));
            return ERROR_INSTALL_FAILURE;
        }

        PACL pACL = nullptr;
        dwRes = SetEntriesInAcl(ulEntries, ea.data(), pOldACL, &pACL);
        wil::unique_any<PACL, decltype(&::LocalFree), ::LocalFree>
            pACLGuard(pACL);
        if (dwRes != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, GetLastError(),
                _T("SetEntriesInAcl Error"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("SetEntriesInAcl Error"));
            return ERROR_INSTALL_FAILURE;
        }

        dwRes = SetNamedSecurityInfo(
            strBOINCInstallDirectory.data(), SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION, nullptr, nullptr, pACL,
            nullptr);
        if (dwRes != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, GetLastError(),
                _T("SetNamedSecurityInfo Error"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, GetLastError(),
                _T("SetNamedSecurityInfo Error"));
            return ERROR_INSTALL_FAILURE;
        }

        RecursiveSetPermissions(strBOINCInstallDirectory, pACL);

        return ERROR_SUCCESS;
    }
};

UINT __stdcall SetPermissionBOINC(MSIHANDLE hInstall) {
    return CASetPermissionBOINC(hInstall).Execute();
}
