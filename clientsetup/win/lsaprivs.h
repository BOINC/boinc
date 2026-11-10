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

#pragma once

class LsaPrivs {
public:
    LsaPrivs() = delete;
    ~LsaPrivs() = delete;
    LsaPrivs operator=(const LsaPrivs&) = delete;
    LsaPrivs(const LsaPrivs&) = delete;

    static bool GetAccountSid(std::wstring_view AccountName, PSID* Sid);
    static bool GrantUserRight(PSID psidAccountSid,
        std::wstring_view pszUserRight, bool bEnable);
};

