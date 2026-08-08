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
// Implementation based on: privs.c
// Read the license below
//
/*++

Copyright 1996 - 1997 Microsoft Corporation

Module Name:

    privs.c

Abstract:

    This module illustrates how to use the Windows NT LSA security API
    to manage account privileges on the local or a remote machine.

    When targetting a domain controller for privilege update operations,
    be sure to target the primary domain controller for the domain.
    The privilege settings are replicated by the primary domain controller
    to each backup domain controller as appropriate.  The NetGetDCName()
    Lan Manager API call can be used to get the primary domain controller
    computer name from a domain name.

    For a list of privileges, consult winnt.h, and search for
    SE_ASSIGNPRIMARYTOKEN_NAME.

    For a list of logon rights, which can also be assigned using this
    sample code, consult ntsecapi.h, and search for SE_BATCH_LOGON_NAME

    You can use domain\account as argv[1]. For instance, mydomain\scott will
    grant the privilege to the mydomain domain account scott.

    The optional target machine is specified as argv[2], otherwise, the
    account database is updated on the local machine.

    The LSA APIs used by this sample are Unicode only.

    Use LsaRemoveAccountRights() to remove account rights.

Author:

    Scott Field (sfield)    17-Apr-96
        Minor cleanup

    Scott Field (sfield)    12-Jul-95

--*/

#include "lsaprivs.h"

static void InitLsaString(LSA_UNICODE_STRING& LsaString,
    std::wstring_view String) {
    const auto StringLength = static_cast<USHORT>(String.size());
    LsaString.Buffer = const_cast<wchar_t*>(String.data());
    LsaString.Length = StringLength * sizeof(WCHAR);
    LsaString.MaximumLength = StringLength * sizeof(WCHAR);
}

static NTSTATUS OpenPolicy(std::wstring_view ServerName, DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle) {
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    LSA_UNICODE_STRING ServerString;
    InitLsaString(ServerString, ServerName);

    return LsaOpenPolicy(&ServerString, &ObjectAttributes, DesiredAccess,
        PolicyHandle);
}

static NTSTATUS SetPrivilegeOnAccount(LSA_HANDLE PolicyHandle, PSID AccountSid,
    std::wstring_view PrivilegeName, bool bEnable) {
    LSA_UNICODE_STRING PrivilegeString;

    InitLsaString(PrivilegeString, PrivilegeName);

    if (bEnable) {
        return LsaAddAccountRights(PolicyHandle, AccountSid,
            &PrivilegeString, 1);
    }
    else {
        return LsaRemoveAccountRights(PolicyHandle, AccountSid, FALSE,
            &PrivilegeString, 1);
    }
}

bool LsaPrivs::GetAccountSid(std::wstring_view AccountName, PSID* Sid) {
    DWORD cbSid = 128;
    *Sid = reinterpret_cast<PSID>(HeapAlloc(GetProcessHeap(), 0, cbSid));

    if (*Sid == nullptr) {
        return false;
    }
    wil::unique_process_heap pSidDeleter(*Sid);

    DWORD cchReferencedDomain = 16;
    auto ReferencedDomain = reinterpret_cast<LPTSTR>(
        HeapAlloc(GetProcessHeap(), 0,
            cchReferencedDomain * sizeof(TCHAR)));

    if (ReferencedDomain == nullptr) {
        return false;
    }
    wil::unique_process_heap_string pReferencedDomainDeleter(ReferencedDomain);

    SID_NAME_USE peUse;
    while (!LookupAccountName(nullptr, AccountName.data(), *Sid, &cbSid,
        ReferencedDomain, &cchReferencedDomain, &peUse)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            pSidDeleter.release();
            *Sid = reinterpret_cast<PSID>(HeapReAlloc(GetProcessHeap(), 0,
                *Sid, cbSid));
            if (*Sid == nullptr) {
                return false;
            }
            pSidDeleter.reset(*Sid);

            pReferencedDomainDeleter.release();
            ReferencedDomain = reinterpret_cast<LPTSTR>(HeapReAlloc(
                GetProcessHeap(), 0, ReferencedDomain,
                cchReferencedDomain * sizeof(TCHAR)));
            if (ReferencedDomain == nullptr) {
                return false;
            }
            pReferencedDomainDeleter.reset(ReferencedDomain);
        }
        else {
            return false;
        }
    }

    pSidDeleter.release();
    return true;
}

bool LsaPrivs::GrantUserRight(PSID psidAccountSid, std::wstring_view pszUserRight,
    bool bEnable) {
    LSA_HANDLE  PolicyHandle = nullptr;
    auto Status = OpenPolicy(_T(""), POLICY_ALL_ACCESS, &PolicyHandle);
    if (Status != STATUS_SUCCESS) {
        return false;
    }
    wil::unique_any<LSA_HANDLE, decltype(&::LsaClose), ::LsaClose>
        policyHandleDeleter(PolicyHandle);

    Status = SetPrivilegeOnAccount(PolicyHandle, psidAccountSid, pszUserRight,
        bEnable);

    return Status == STATUS_SUCCESS;
}
