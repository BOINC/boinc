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

class CARestorePermissionBOINCData : public BOINCCABase
{
public:
    virtual ~CARestorePermissionBOINCData() = default;
    explicit CARestorePermissionBOINCData(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CARestorePermissionBOINCData"),
            _T("Restoring permissions on the BOINC Data directory.")) {
    }

    UINT OnExecution() override final {
        tstring strBOINCDataDirectory;
        const auto uiReturnValue = GetProperty(_T("DATADIR"),
            strBOINCDataDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (!std::filesystem::exists(strBOINCDataDirectory)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, 0,
                _T("The data directory doesn't exist."));
            return ERROR_INSTALL_FAILURE;
        }

        constexpr ULONG ulEntries = 2;
        std::array<EXPLICIT_ACCESS, ulEntries> ea;

        DWORD sizeSystem = SECURITY_MAX_SID_SIZE;
        std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferSystem;
        if (!CreateWellKnownSid(WinLocalSystemSid, nullptr, &bufferSystem,
            &sizeSystem)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("CreateWellKnownSid Error for SYSTEM"));
            return ERROR_INSTALL_FAILURE;
        }

        DWORD sizeAdmin = SECURITY_MAX_SID_SIZE;
        std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferAdmin;
        if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr,
            &bufferAdmin, &sizeAdmin)) {
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("CreateWellKnownSid Error for BUILTIN\\Administrators"));
            return ERROR_INSTALL_FAILURE;
        }

        ea[0].grfAccessPermissions = GENERIC_ALL;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName = reinterpret_cast<LPTSTR>(&bufferSystem);

        ea[1].grfAccessPermissions = GENERIC_ALL;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[1].Trustee.ptstrName = reinterpret_cast<LPTSTR>(&bufferAdmin);

        PACL pACL = nullptr;
        wil::unique_any<PACL, decltype(&::LocalFree), ::LocalFree> pACLGuard;
        auto dwRes = SetEntriesInAcl(ulEntries, ea.data(), nullptr, &pACL);
        if (dwRes != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, GetLastError(),
                _T("SetEntriesInAcl Error"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("SetEntriesInAcl Error"));
            return ERROR_INSTALL_FAILURE;
        }

        dwRes = SetNamedSecurityInfo(strBOINCDataDirectory.data(),
            SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
            nullptr, nullptr, pACL, nullptr);
        if (dwRes != ERROR_SUCCESS) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, GetLastError(),
                _T("SetNamedSecurityInfo Error"));
            LogMessage(INSTALLMESSAGE_ERROR, 0, 0, 0, GetLastError(),
                _T("SetNamedSecurityInfo Error"));
            return ERROR_INSTALL_FAILURE;
        }

        RecursiveSetPermissions(strBOINCDataDirectory, pACL);

        return ERROR_SUCCESS;
    }
};

UINT __stdcall RestorePermissionBOINCData(MSIHANDLE hInstall) {
    return CARestorePermissionBOINCData(hInstall).Execute();
}
