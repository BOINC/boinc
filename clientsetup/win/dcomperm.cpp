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
//
// Implementation based on: DCOM Permission Configuration Sample
// Read the license below
//
/*++

DCOM Permission Configuration Sample
Copyright (c) 1996, Microsoft Corporation. All rights reserved.

Module Name:

    dcomperm.h

Abstract:

    Include file for DCOM Permission Configuration sample

Author:

    Michael Nelson

Environment:

    Windows NT

--*/

#include "lsaprivs.h"
#include "ntsecapi.h"
#include "dcomperm.h"

static DWORD CopyACL(PACL OldACL, PACL NewACL) {
    ACL_SIZE_INFORMATION  aclSizeInfo;

    if (!GetAclInformation(OldACL, reinterpret_cast<LPVOID>(&aclSizeInfo),
        sizeof(aclSizeInfo), AclSizeInformation)) {
        return GetLastError();
    }

    for (decltype(aclSizeInfo.AceCount) i = 0; i < aclSizeInfo.AceCount; ++i) {
        LPVOID ace = nullptr;
        if (!GetAce(OldACL, i, &ace)) {
            return GetLastError();
        }

        auto aceHeader = reinterpret_cast<ACE_HEADER*>(ace);

        if (!AddAce(
            NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize)) {
            return GetLastError();
        }
    }

    return ERROR_SUCCESS;
}

static DWORD AddAccessAllowedACEToACL(PACL* Acl, DWORD PermissionMask,
    std::wstring_view Principal) {

    PSID principalSID = nullptr;
    if (!LsaPrivs::GetAccountSid(Principal.data(), &principalSID)) {
        return GetLastError();
    }
    wil::unique_process_heap pPrincipalSIDDeleter(principalSID);

    auto oldACL = *Acl;
    ACL_SIZE_INFORMATION aclSizeInfo;
    if (!GetAclInformation(oldACL, reinterpret_cast<LPVOID>(&aclSizeInfo),
        sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
        return GetLastError();
    }

    const DWORD aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) +
        sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(principalSID) -
        sizeof(DWORD);

    auto newACL = reinterpret_cast<PACL>(new BYTE[aclSize]);

    if (!InitializeAcl(newACL, aclSize, ACL_REVISION)) {
        return GetLastError();
    }

    const auto returnValue = CopyACL(oldACL, newACL);
    if (returnValue != ERROR_SUCCESS) {
        return returnValue;
    }

    if (!AddAccessAllowedAce(
        newACL, ACL_REVISION2, PermissionMask, principalSID)) {
        return GetLastError();
    }

    *Acl = newACL;

    return ERROR_SUCCESS;
}

static DWORD RemovePrincipalFromACL(PACL Acl, std::wstring_view Principal) {
    PSID principalSID = nullptr;
    if (!LsaPrivs::GetAccountSid(Principal.data(), &principalSID)) {
        return GetLastError();
    }
    wil::unique_process_heap pPrincipalSIDDeleter(principalSID);

    ACL_SIZE_INFORMATION aclSizeInfo;
    if (!GetAclInformation(Acl, reinterpret_cast<LPVOID>(&aclSizeInfo),
        sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
        return GetLastError();
    }

    for (decltype(aclSizeInfo.AceCount) i = 0; i < aclSizeInfo.AceCount; ++i) {
        LPVOID ace = nullptr;
        if (!GetAce(Acl, i, &ace)) {
            return GetLastError();
        }

        auto aceHeader = reinterpret_cast<ACE_HEADER*>(ace);
        switch (aceHeader->AceType) {
        case ACCESS_ALLOWED_ACE_TYPE: {
            auto accessAllowedAce = reinterpret_cast<ACCESS_ALLOWED_ACE*>(ace);

            if (EqualSid(principalSID,
                reinterpret_cast<PSID>(&accessAllowedAce->SidStart))) {
                if (!DeleteAce(Acl, i)) {
                    return GetLastError();
                }
                return ERROR_SUCCESS;
            }
        }
                                    break;
        case ACCESS_DENIED_ACE_TYPE: {
            auto accessDeniedAce = reinterpret_cast<ACCESS_DENIED_ACE*>(ace);

            if (EqualSid(principalSID,
                reinterpret_cast<PSID>(&accessDeniedAce->SidStart))) {
                if (!DeleteAce(Acl, i)) {
                    return GetLastError();
                }
                return ERROR_SUCCESS;
            }
        }
                                   break;
        case SYSTEM_AUDIT_ACE_TYPE: {
            auto systemAuditAce = reinterpret_cast<SYSTEM_AUDIT_ACE*>(ace);

            if (EqualSid(principalSID,
                reinterpret_cast<PSID>(&systemAuditAce->SidStart))) {
                if (!DeleteAce(Acl, i)) {
                    return GetLastError();
                }
                return ERROR_SUCCESS;
            }
        }
                                  break;
        default:
            // Ignore other ACE types
            break;
        }
    }

    return ERROR_SUCCESS;
}

static DWORD GetCurrentUserSID(PSID* Sid) {
    HANDLE tokenHandle;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
        return GetLastError();
    }
    wil::unique_handle tokenHandleDeleter(tokenHandle);

    TOKEN_USER* tokenUser = nullptr;
    DWORD tokenSize;
    GetTokenInformation(tokenHandle, TokenUser, tokenUser, 0, &tokenSize);

    tokenUser = reinterpret_cast<TOKEN_USER*>(malloc(tokenSize));
    wil::unique_any<TOKEN_USER*, decltype(&::free), ::free>
        tokenUserDeleter(tokenUser);

    if (!GetTokenInformation(tokenHandle, TokenUser, tokenUser, tokenSize,
        &tokenSize)) {
        return GetLastError();
    }

    const auto sidLength = GetLengthSid(tokenUser->User.Sid);
    *Sid = reinterpret_cast<PSID>(malloc(sidLength));
    memcpy(*Sid, tokenUser->User.Sid, sidLength);

    return ERROR_SUCCESS;
}

static DWORD CreateNewSD(SECURITY_DESCRIPTOR** SD) {
    PSID sid;
    const auto returnValue = GetCurrentUserSID(&sid);
    if (returnValue != ERROR_SUCCESS) {
        return returnValue;
    }
    wil::unique_any<PSID, decltype(&::free), ::free>
        pSidDeleter(sid);

    *SD = nullptr;
    const auto sidLength = GetLengthSid(sid);
    *SD = reinterpret_cast<SECURITY_DESCRIPTOR*>(malloc(
        (sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + sidLength) +
        (2 * sidLength) + sizeof(SECURITY_DESCRIPTOR)));

    wil::unique_any<SECURITY_DESCRIPTOR*, decltype(&::free), ::free>
        pSDDeleter(*SD);

    auto groupSID = reinterpret_cast<SID*>(*SD + 1);
    auto ownerSID = reinterpret_cast<SID*>(
        reinterpret_cast<BYTE*>(groupSID) + sidLength);
    auto dacl = reinterpret_cast<ACL*>(
        reinterpret_cast<BYTE*>(ownerSID) + sidLength);

    if (!InitializeSecurityDescriptor(*SD, SECURITY_DESCRIPTOR_REVISION)) {
        return GetLastError();
    }

    if (!InitializeAcl(dacl,
        sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + sidLength, ACL_REVISION2)) {
        return GetLastError();
    }

    if (!AddAccessAllowedAce(dacl, ACL_REVISION2, COM_RIGHTS_EXECUTE, sid)) {
        return GetLastError();
    }

    if (!SetSecurityDescriptorDacl(*SD, TRUE, dacl, FALSE)) {
        return GetLastError();
    }

    memcpy(groupSID, sid, sidLength);
    if (!SetSecurityDescriptorGroup(*SD, groupSID, FALSE)) {
        return GetLastError();
    }

    memcpy(ownerSID, sid, sidLength);
    if (!SetSecurityDescriptorOwner(*SD, ownerSID, FALSE)) {
        return GetLastError();
    }
    pSDDeleter.release();
    return ERROR_SUCCESS;
}

static DWORD MakeSDAbsolute(PSECURITY_DESCRIPTOR OldSD,
    PSECURITY_DESCRIPTOR* NewSD) {
    BOOL present;
    PACL sacl;
    BOOL systemDefault;

    if (!GetSecurityDescriptorSacl(OldSD, &present, &sacl, &systemDefault)) {
        return GetLastError();
    }

    DWORD saclSize = (sacl && present) ? sacl->AclSize : 0;

    PACL dacl;
    if (!GetSecurityDescriptorDacl(OldSD, &present, &dacl, &systemDefault)) {
        return GetLastError();
    }

    DWORD daclSize = (dacl && present) ? dacl->AclSize : 0;

    PSID ownerSID;
    if (!GetSecurityDescriptorOwner(OldSD, &ownerSID, &systemDefault)) {
        return GetLastError();
    }

    auto ownerSIDSize = GetLengthSid(ownerSID);

    PSID groupSID;
    if (!GetSecurityDescriptorGroup(OldSD, &groupSID, &systemDefault)) {
        return GetLastError();
    }

    auto groupSIDSize = GetLengthSid(groupSID);

    DWORD descriptorSize = 0;
    PSECURITY_DESCRIPTOR  sd = nullptr;

    MakeAbsoluteSD(OldSD, sd, &descriptorSize, dacl, &daclSize, sacl,
        &saclSize, ownerSID, &ownerSIDSize, groupSID, &groupSIDSize);

    sd = reinterpret_cast<PSECURITY_DESCRIPTOR>(
        new BYTE[SECURITY_DESCRIPTOR_MIN_LENGTH]);
    if (!InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION)) {
        delete[] sd;
        return GetLastError();
    }

    if (!MakeAbsoluteSD(OldSD, sd, &descriptorSize, dacl, &daclSize, sacl,
        &saclSize, ownerSID, &ownerSIDSize, groupSID, &groupSIDSize)) {
        delete[] sd;
        return GetLastError();
    }

    *NewSD = sd;
    return ERROR_SUCCESS;
}

static DWORD SetNamedValueSD(HKEY RootKey, std::wstring_view KeyName,
    std::wstring_view ValueName, SECURITY_DESCRIPTOR* SD) {
    DWORD disposition;
    HKEY registryKey;
    auto returnValue = RegCreateKeyEx(RootKey, KeyName.data(), 0, _T(""), 0,
        KEY_ALL_ACCESS, NULL, &registryKey, &disposition);
    if (returnValue != ERROR_SUCCESS) {
        return returnValue;
    }
    wil::unique_hkey registryKeyDeleter(registryKey);

    returnValue = RegSetValueEx(registryKey, ValueName.data(), 0, REG_BINARY,
        reinterpret_cast<LPBYTE>(SD), GetSecurityDescriptorLength(SD));
    if (returnValue != ERROR_SUCCESS) {
        return returnValue;
    }

    return ERROR_SUCCESS;
}

//
// Get the security descriptor from the named value. If it doesn't
// exist, create a fresh one.
//
static DWORD GetNamedValueSD(HKEY RootKey, std::wstring_view KeyName,
    std::wstring_view ValueName, SECURITY_DESCRIPTOR** SD, bool& NewSD) {
    NewSD = false;

    HKEY registryKey;
    auto returnValue = RegOpenKeyEx(RootKey, KeyName.data(), 0,
        KEY_ALL_ACCESS, &registryKey);

    if (returnValue != ERROR_SUCCESS) {
        if (returnValue != ERROR_FILE_NOT_FOUND) {
            return returnValue;
        }
        *SD = nullptr;
        returnValue = CreateNewSD(SD);
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }

        NewSD = true;
        return ERROR_SUCCESS;
    }

    wil::unique_hkey registryKeyDeleter(registryKey);

    DWORD valueType;
    DWORD valueSize;
    returnValue = RegQueryValueEx(registryKey, ValueName.data(), nullptr,
        &valueType, nullptr, &valueSize);

    if (returnValue && returnValue != ERROR_INSUFFICIENT_BUFFER) {
        *SD = nullptr;
        returnValue = CreateNewSD(SD);
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }
        NewSD = true;
    }
    else {
        *SD = reinterpret_cast<SECURITY_DESCRIPTOR*>(malloc(valueSize));
        returnValue = RegQueryValueEx(registryKey, ValueName.data(), nullptr,
            &valueType, reinterpret_cast<LPBYTE>(*SD), &valueSize);
        if (returnValue) {
            free(*SD);

            *SD = nullptr;
            returnValue = CreateNewSD(SD);
            if (returnValue != ERROR_SUCCESS) {
                return returnValue;
            }
            NewSD = true;
        }
    }

    return ERROR_SUCCESS;
}

static DWORD AddPrincipalToNamedValueSD(HKEY RootKey,
    std::wstring_view KeyName, std::wstring_view ValueName,
    std::wstring_view Principal) {
    auto newSD = false;
    SECURITY_DESCRIPTOR* sd = nullptr;
    auto returnValue = GetNamedValueSD(RootKey, KeyName, ValueName, &sd,
        newSD);

    if (returnValue != ERROR_SUCCESS) {
        return returnValue;
    }

    typedef wil::unique_any<SECURITY_DESCRIPTOR*, decltype(&::free), ::free>
        unique_sd;
    unique_sd pSDDeleter(sd);

    BOOL present;
    PACL dacl;
    BOOL defaultDACL;
    if (!GetSecurityDescriptorDacl(sd, &present, &dacl, &defaultDACL)) {
        return GetLastError();
    }

    if (newSD) {
        returnValue = AddAccessAllowedACEToACL(&dacl, COM_RIGHTS_EXECUTE,
            _T("SYSTEM"));
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }
        returnValue = AddAccessAllowedACEToACL(&dacl, COM_RIGHTS_EXECUTE,
            _T("INTERACTIVE"));
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }
    }

    returnValue = AddAccessAllowedACEToACL(&dacl, COM_RIGHTS_EXECUTE,
        Principal);

    if (returnValue != ERROR_SUCCESS) {
        return returnValue;
    }

    SECURITY_DESCRIPTOR* sdAbsolute = nullptr;
    unique_sd pSDAbsoluteDeleter(nullptr);

    if (!newSD) {
        returnValue = MakeSDAbsolute(sd,
            reinterpret_cast<PSECURITY_DESCRIPTOR*>(&sdAbsolute));
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }
        pSDAbsoluteDeleter.reset(sdAbsolute);
    }
    else {
        sdAbsolute = sd;
    }

    if (!SetSecurityDescriptorDacl(sdAbsolute, TRUE, dacl, FALSE)) {
        return GetLastError();
    }

    DWORD secDescSize = 0;
    MakeSelfRelativeSD(sdAbsolute, nullptr, &secDescSize);

    auto sdSelfRelative =
        reinterpret_cast<SECURITY_DESCRIPTOR*>(malloc(secDescSize));
    unique_sd pSDSelfRelativeDeleter(sdSelfRelative);

    if (!MakeSelfRelativeSD(sdAbsolute, sdSelfRelative, &secDescSize)) {
        return GetLastError();
    }

    return SetNamedValueSD(RootKey, KeyName, ValueName, sdSelfRelative);
}

static DWORD RemovePrincipalFromNamedValueSD(HKEY RootKey,
    std::wstring_view KeyName, std::wstring_view ValueName,
    std::wstring_view Principal) {
    SECURITY_DESCRIPTOR* sd = NULL;
    auto newSD = false;
    auto returnValue =
        GetNamedValueSD(RootKey, KeyName, ValueName, &sd, newSD);

    if (returnValue != ERROR_SUCCESS)
        return returnValue;

    typedef wil::unique_any<SECURITY_DESCRIPTOR*, decltype(&::free), ::free>
        unique_sd;
    unique_sd pSDDeleter(sd);

    BOOL present;
    PACL dacl;
    BOOL defaultDACL;
    if (!GetSecurityDescriptorDacl(sd, &present, &dacl, &defaultDACL)) {
        return GetLastError();
    }

    if (newSD) {
        returnValue = AddAccessAllowedACEToACL(&dacl, COM_RIGHTS_EXECUTE,
            _T("SYSTEM"));
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }
        returnValue = AddAccessAllowedACEToACL(&dacl, COM_RIGHTS_EXECUTE,
            _T("INTERACTIVE"));
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }
    }

    returnValue = RemovePrincipalFromACL(dacl, Principal);
    if (returnValue != ERROR_SUCCESS) {
        return returnValue;
    }

    SECURITY_DESCRIPTOR* sdAbsolute = nullptr;
    unique_sd pSDAbsoluteDeleter(nullptr);
    if (!newSD) {
        returnValue = MakeSDAbsolute(sd,
            reinterpret_cast<PSECURITY_DESCRIPTOR*>(&sdAbsolute));
        if (returnValue != ERROR_SUCCESS) {
            return returnValue;
        }
        pSDAbsoluteDeleter.reset(sdAbsolute);
    }
    else {
        sdAbsolute = sd;
    }

    if (!SetSecurityDescriptorDacl(sdAbsolute, TRUE, dacl, FALSE)) {
        return GetLastError();
    }

    DWORD secDescSize = 0;
    MakeSelfRelativeSD(sdAbsolute, nullptr, &secDescSize);
    auto sdSelfRelative =
        reinterpret_cast<SECURITY_DESCRIPTOR*>(malloc(secDescSize));
    if (!MakeSelfRelativeSD(sdAbsolute, sdSelfRelative, &secDescSize)) {
        return GetLastError();
    }

    return SetNamedValueSD(RootKey, KeyName, ValueName, sdSelfRelative);
}

DWORD DCOMPermissionConfig::ChangeAppIDAccessACL(std::wstring_view AppID,
    std::wstring_view Principal) {
    tstring keyName = _T("APPID\\");
    keyName += AppID;

    const auto result = RemovePrincipalFromNamedValueSD(HKEY_CLASSES_ROOT,
        keyName.data(), _T("AccessPermission"), Principal);
    if (result != ERROR_SUCCESS) {
        return result;
    }
    return AddPrincipalToNamedValueSD(HKEY_CLASSES_ROOT, keyName.data(),
        _T("AccessPermission"), Principal);
}

DWORD DCOMPermissionConfig::ChangeAppIDLaunchACL(std::wstring_view AppID,
    std::wstring_view Principal) {
    tstring keyName = _T("APPID\\");
    keyName += AppID;

    const auto result = RemovePrincipalFromNamedValueSD(HKEY_CLASSES_ROOT,
        keyName.data(), TEXT("LaunchPermission"), Principal);
    if (result != ERROR_SUCCESS) {
        return result;
    }
    return AddPrincipalToNamedValueSD(HKEY_CLASSES_ROOT, keyName.data(),
        TEXT("LaunchPermission"), Principal);
}
