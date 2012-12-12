/*++

DCOM Permission Configuration Sample
Copyright (c) 1996, Microsoft Corporation. All rights reserved.

Module Name:

    listacl.cpp

Abstract:

    Code to list ACL information

Author:

    Michael Nelson

Environment:

    Windows NT

--*/

#include "stdafx.h"
#include "ntsecapi.h"
#include "dcomperm.h"

void
ListACL (
    PACL Acl
    )
{
    ACL_SIZE_INFORMATION     aclSizeInfo;
    ACL_REVISION_INFORMATION aclRevInfo;
    ULONG                    i = 0;
    LPVOID                   ace = NULL;
    ACE_HEADER               *aceHeader = NULL;
    ACCESS_ALLOWED_ACE       *paaace = NULL;
    ACCESS_DENIED_ACE        *padace = NULL;
    TCHAR                    domainName [256];
    TCHAR                    userName [256];
    DWORD                    nameLength = 0;
    SID_NAME_USE             snu;

    if (!GetAclInformation (Acl,
                            &aclSizeInfo,
                            sizeof (ACL_SIZE_INFORMATION),
                            AclSizeInformation))
    {
        _tprintf (TEXT("Could not get AclSizeInformation"));
        return;
    }

    if (!GetAclInformation (Acl,
                            &aclRevInfo,
                            sizeof (ACL_REVISION_INFORMATION),
                            AclRevisionInformation))
    {
        _tprintf (TEXT("Could not get AclRevisionInformation"));
        return;
    }

    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
        if (!GetAce (Acl, i, &ace))
            return;

        aceHeader = (ACE_HEADER *) ace;

        if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            paaace = (ACCESS_ALLOWED_ACE *) ace;
            nameLength = 255;
            LookupAccountSid (NULL,
                              &paaace->SidStart,
                              userName,
                              &nameLength,
                              domainName,
                              &nameLength,
                              &snu);

            _tprintf (TEXT("Access permitted to %s\\%s.\n"), domainName, userName);
        } else
        if (aceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
        {
            padace = (ACCESS_DENIED_ACE *) ace;
            nameLength = 255;
            LookupAccountSid (NULL,
                              &padace->SidStart,
                              userName,
                              &nameLength,
                              domainName,
                              &nameLength,
                              &snu);

            _tprintf (TEXT("Access denied to %s\\%s.\n"), domainName, userName);

        }
   }
}

