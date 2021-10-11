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

#include "stdafx.h"
#include "lsaprivs.h"


/*++
This function attempts to obtain a SID representing the supplied
account on the supplied system.

If the function succeeds, the return value is TRUE. A buffer is
allocated which contains the SID representing the supplied account.
This buffer should be freed when it is no longer needed by calling
HeapFree(GetProcessHeap(), 0, buffer)

If the function fails, the return value is FALSE. Call GetLastError()
to obtain extended error information.

Scott Field (sfield)    12-Jul-95
--*/

BOOL
GetAccountSid(
    LPCTSTR SystemName,
    LPCTSTR AccountName,
    PSID *Sid
    )
{
    LPTSTR ReferencedDomain=NULL;
    DWORD cbSid=128;    // initial allocation attempt
    DWORD cchReferencedDomain=16; // initial allocation size
    SID_NAME_USE peUse;
    BOOL bSuccess=FALSE; // assume this function will fail

    __try {

    //
    // initial memory allocations
    //
    *Sid = (PSID)HeapAlloc(GetProcessHeap(), 0, cbSid);

    if(*Sid == NULL) __leave;

    ReferencedDomain = (LPTSTR)HeapAlloc(
                    GetProcessHeap(),
                    0,
                    cchReferencedDomain * sizeof(TCHAR)
                    );

    if(ReferencedDomain == NULL) __leave;

    //
    // Obtain the SID of the specified account on the specified system.
    //
    while(!LookupAccountName(
                    SystemName,         // machine to lookup account on
                    AccountName,        // account to lookup
                    *Sid,               // SID of interest
                    &cbSid,             // size of SID
                    ReferencedDomain,   // domain account was found on
                    &cchReferencedDomain,
                    &peUse
                    )) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            //
            // reallocate memory
            //
            *Sid = (PSID)HeapReAlloc(
                        GetProcessHeap(),
                        0,
                        *Sid,
                        cbSid
                        );
            if(*Sid == NULL) __leave;

            ReferencedDomain = (LPTSTR)HeapReAlloc(
                        GetProcessHeap(),
                        0,
                        ReferencedDomain,
                        cchReferencedDomain * sizeof(TCHAR)
                        );
            if(ReferencedDomain == NULL) __leave;
        }
        else __leave;
    }

    //
    // Indicate success.
    //
    bSuccess = TRUE;

    } // try
    __finally {

    //
    // Cleanup and indicate failure, if appropriate.
    //

    HeapFree(GetProcessHeap(), 0, ReferencedDomain);

    if(!bSuccess) {
        if(*Sid != NULL) {
            HeapFree(GetProcessHeap(), 0, *Sid);
            *Sid = NULL;
        }
    }

    } // finally

    return bSuccess;
}

NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,    // open policy handle
    PSID AccountSid,            // SID to grant privilege to
    LPWSTR PrivilegeName,       // privilege to grant (Unicode)
    BOOL bEnable                // enable or disable
    )
{
    LSA_UNICODE_STRING PrivilegeString;

    //
    // Create a LSA_UNICODE_STRING for the privilege name.
    //
    InitLsaString(&PrivilegeString, PrivilegeName);

    //
    // grant or revoke the privilege, accordingly
    //
    if(bEnable) {
        return LsaAddAccountRights(
                PolicyHandle,       // open policy handle
                AccountSid,         // target SID
                &PrivilegeString,   // privileges
                1                   // privilege count
                );
    }
    else {
        return LsaRemoveAccountRights(
                PolicyHandle,       // open policy handle
                AccountSid,         // target SID
                FALSE,              // do not disable all rights
                &PrivilegeString,   // privileges
                1                   // privilege count
                );
    }
}

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String
    )
{
    DWORD StringLength;

    if(String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = lstrlenW(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
}

NTSTATUS
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

    //
    // Always initialize the object attributes to all zeroes.
    //
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    if (ServerName != NULL) {
        //
        // Make a LSA_UNICODE_STRING out of the LPWSTR passed in
        //
        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    } else {
        Server = NULL;
    }

    //
    // Attempt to open the policy.
    //
    return LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle
                );
}


BOOL
GrantUserRight(
    PSID    psidAccountSid,
    LPWSTR  pszUserRight,
    BOOL    bEnable
    )
{
    LSA_HANDLE  PolicyHandle = NULL;
    NTSTATUS    Status;

    //
    // Open the policy on the local host.
    //
    Status = OpenPolicy(
                _T(""),
                POLICY_ALL_ACCESS,
                &PolicyHandle
                );


    if(Status != STATUS_SUCCESS) {
        return FALSE;
    }


    //
    // Grant the requested user right represented by psidAccountSid.
    //
    Status = SetPrivilegeOnAccount(
                PolicyHandle,                   // policy handle
                psidAccountSid,                 // SID to grant privilege
                pszUserRight,                   // Unicode privilege
                bEnable                         // enable the privilege
                );

    if(Status != STATUS_SUCCESS)
    {
        LsaClose(PolicyHandle);
        return FALSE;
    }

    //
    // Cleanup any handles and memory allocated during the custom action
    //
    LsaClose(PolicyHandle);
    return TRUE;
}
