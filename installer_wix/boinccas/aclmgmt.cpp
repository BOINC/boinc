/*++

DCOM Permission Configuration Sample
Copyright (c) 1996, Microsoft Corporation. All rights reserved.

Module Name:

    aclmgmt.cpp

Abstract:

    Routines to manage access control lists

Author:

    Michael Nelson

Environment:

    Windows NT

--*/

#include "stdafx.h"
#include "lsaprivs.h"
#include "ntsecapi.h"
#include "dcomperm.h"

DWORD
CopyACL (
    PACL OldACL,
    PACL NewACL
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo;
    LPVOID                ace = NULL;
    ACE_HEADER            *aceHeader = NULL;
    ULONG                 i = 0;

    GetAclInformation (OldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (aclSizeInfo), AclSizeInformation);

    //
    // Copy all of the ACEs to the new ACL
    //

    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
        //
        // Get the ACE and header info
        //

        if (!GetAce (OldACL, i, &ace))
            return GetLastError();

        aceHeader = (ACE_HEADER *) ace;

        //
        // Add the ACE to the new list
        //

        if (!AddAce (NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
            return GetLastError();
    }

    return ERROR_SUCCESS;
}

DWORD
AddAccessDeniedACEToACL (
    PACL *Acl,
    DWORD PermissionMask,
    LPTSTR Principal
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo;
    int                   aclSize = 0;
    DWORD                 returnValue = 0;
    PSID                  principalSID = 0;
    PACL                  oldACL = NULL, newACL = NULL;

    oldACL = *Acl;

    if (!GetAccountSid(NULL, Principal, &principalSID))
    {
        return GetLastError();
    }

    GetAclInformation (oldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (ACL_SIZE_INFORMATION), AclSizeInformation);

    aclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_DENIED_ACE) +
              GetLengthSid (principalSID) - sizeof (DWORD);

    newACL = (PACL) new BYTE [aclSize];

    if (!InitializeAcl (newACL, aclSize, ACL_REVISION))
    {
        HeapFree(GetProcessHeap(), 0, principalSID);
        return GetLastError();
    }

    if (!AddAccessDeniedAce (newACL, ACL_REVISION2, PermissionMask, principalSID))
    {
        HeapFree(GetProcessHeap(), 0, principalSID);
        return GetLastError();
    }

    returnValue = CopyACL (oldACL, newACL);
    if (returnValue != ERROR_SUCCESS)
    {
        HeapFree(GetProcessHeap(), 0, principalSID);
        return returnValue;
    }

    *Acl = newACL;

    if(principalSID != NULL) HeapFree(GetProcessHeap(), 0, principalSID);
    return ERROR_SUCCESS;
}

DWORD
AddAccessAllowedACEToACL (
    PACL *Acl,
    DWORD PermissionMask,
    LPTSTR Principal
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo;
    int                   aclSize = 0;
    DWORD                 returnValue = 0;
    PSID                  principalSID = NULL;
    PACL                  oldACL = NULL, newACL = NULL;

    oldACL = *Acl;

    if (!GetAccountSid(NULL, Principal, &principalSID))
    {
        return GetLastError();
    }

    GetAclInformation (oldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (ACL_SIZE_INFORMATION), AclSizeInformation);

    aclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) +
              GetLengthSid (principalSID) - sizeof (DWORD);

    newACL = (PACL) new BYTE [aclSize];

    if (!InitializeAcl (newACL, aclSize, ACL_REVISION))
    {
        HeapFree(GetProcessHeap(), 0, principalSID);
        return GetLastError();
    }

    returnValue = CopyACL (oldACL, newACL);
    if (returnValue != ERROR_SUCCESS)
    {
        HeapFree(GetProcessHeap(), 0, principalSID);
        return returnValue;
    }

    if (!AddAccessAllowedAce (newACL, ACL_REVISION2, PermissionMask, principalSID))
    {
        HeapFree(GetProcessHeap(), 0, principalSID);
        return GetLastError();
    }

    *Acl = newACL;

    if(principalSID != NULL) HeapFree(GetProcessHeap(), 0, principalSID);
    return ERROR_SUCCESS;
}

DWORD
RemovePrincipalFromACL (
    PACL Acl,
    LPTSTR Principal
    )
{
    ACL_SIZE_INFORMATION    aclSizeInfo;
    ULONG                   i = 0;
    LPVOID                  ace = NULL;
    ACCESS_ALLOWED_ACE      *accessAllowedAce = NULL;
    ACCESS_DENIED_ACE       *accessDeniedAce = NULL;
    SYSTEM_AUDIT_ACE        *systemAuditAce = NULL;
    PSID                    principalSID = NULL;
    ACE_HEADER              *aceHeader = NULL;

    if (!GetAccountSid(NULL, Principal, &principalSID))
    {
        return GetLastError();
    }

    GetAclInformation (Acl, (LPVOID) &aclSizeInfo, (DWORD) sizeof (ACL_SIZE_INFORMATION), AclSizeInformation);

    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
        if (!GetAce (Acl, i, &ace))
        {
            HeapFree(GetProcessHeap(), 0, principalSID);
            return GetLastError();
        }

        aceHeader = (ACE_HEADER *) ace;

        if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            accessAllowedAce = (ACCESS_ALLOWED_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &accessAllowedAce->SidStart))
            {
                DeleteAce (Acl, i);
                HeapFree(GetProcessHeap(), 0, principalSID);
                return ERROR_SUCCESS;
            }
        } else

        if (aceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
        {
            accessDeniedAce = (ACCESS_DENIED_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &accessDeniedAce->SidStart))
            {
                DeleteAce (Acl, i);
                HeapFree(GetProcessHeap(), 0, principalSID);
                return ERROR_SUCCESS;
            }
        } else

        if (aceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
        {
            systemAuditAce = (SYSTEM_AUDIT_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &systemAuditAce->SidStart))
            {
                DeleteAce (Acl, i);
                HeapFree(GetProcessHeap(), 0, principalSID);
                return ERROR_SUCCESS;
            }
        }
    }

    if(principalSID != NULL) HeapFree(GetProcessHeap(), 0, principalSID);
    return ERROR_SUCCESS;
}

