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

    For a list of privilges, consult winnt.h, and search for
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


NTSTATUS
OpenPolicy(
    LPWSTR ServerName,          // machine to open policy on (Unicode)
    DWORD DesiredAccess,        // desired access to policy
    PLSA_HANDLE PolicyHandle    // resultant policy handle
    );

BOOL
GetAccountSid(
    LPCTSTR SystemName,         // where to lookup account
    LPCTSTR AccountName,        // account of interest
    PSID *Sid                   // resultant buffer containing SID
    );

NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,    // open policy handle
    PSID AccountSid,            // SID to grant privilege to
    LPWSTR PrivilegeName,       // privilege to grant (Unicode)
    BOOL bEnable                // enable or disable
    );

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString, // destination
    LPWSTR String                  // source (Unicode)
    );

BOOL
GrantUserRight(
    PSID    psidAccountSid,
    LPWSTR  pszUserRight,
    BOOL    bEnable
    );