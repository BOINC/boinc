/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    acctname.c

Abstract:

    This module illustrates how to obtain user and group names
    Associated with accounts that have been renamed or localized. This
    sample works by building a Sid value and then looking up the name
    associated with that Sid value.

    For example, in a default English install of Windows NT, the local
    administrators group is called "Administrators." In a default German
    install of Windows NT, the local administrators group is called
    "Administratoren." This can lead to problems when managing users
    and groups if the proper steps are not taken.

    The following relative ID values are an index into an account
    database that represents a specific user or group. In DOMAIN_USER_
    and DOMAIN_GROUP_ cases, the relative ID is appended to the account
    domain Sid from the machine of interest. In the DOMAIN_ALIAS_ case,
    the relative ID is appended to a well-known Sid representing the
    BUILTIN domain that is consistent across machines.

    // Well-known users and groups...

    #define DOMAIN_USER_RID_ADMIN          (0x000001F4L)
    #define DOMAIN_USER_RID_GUEST          (0x000001F5L)

    #define DOMAIN_GROUP_RID_ADMINS        (0x00000200L)
    #define DOMAIN_GROUP_RID_USERS         (0x00000201L)
    #define DOMAIN_GROUP_RID_GUESTS        (0x00000202L)

    // well-known aliases ...

    #define DOMAIN_ALIAS_RID_ADMINS        (0x00000220L)
    #define DOMAIN_ALIAS_RID_USERS         (0x00000221L)
    #define DOMAIN_ALIAS_RID_GUESTS        (0x00000222L)
    #define DOMAIN_ALIAS_RID_POWER_USERS   (0x00000223L)
    #define DOMAIN_ALIAS_RID_ACCOUNT_OPS   (0x00000224L)
    #define DOMAIN_ALIAS_RID_SYSTEM_OPS    (0x00000225L)
    #define DOMAIN_ALIAS_RID_PRINT_OPS     (0x00000226L)
    #define DOMAIN_ALIAS_RID_BACKUP_OPS    (0x00000227L)
    #define DOMAIN_ALIAS_RID_REPLICATOR    (0x00000228L)

    The following section is for informational purposes and is useful
    for visualizing Sid values:

    // Universal well-known SIDs:
    //
    //     Null SID                     S-1-0-0
    //     World                        S-1-1-0
    //     Local                        S-1-2-0
    //     Creator Owner ID             S-1-3-0
    //     Creator Group ID             S-1-3-1
    //     Creator Owner Server ID      S-1-3-2
    //     Creator Group Server ID      S-1-3-3
    //
    //     (Non-unique IDs)             S-1-4

    #define SECURITY_NULL_SID_AUTHORITY       {0,0,0,0,0,0}
    #define SECURITY_WORLD_SID_AUTHORITY      {0,0,0,0,0,1}
    #define SECURITY_LOCAL_SID_AUTHORITY      {0,0,0,0,0,2}
    #define SECURITY_CREATOR_SID_AUTHORITY    {0,0,0,0,0,3}
    #define SECURITY_NON_UNIQUE_AUTHORITY     {0,0,0,0,0,4}

    #define SECURITY_NULL_RID                 (0x00000000L)
    #define SECURITY_WORLD_RID                (0x00000000L)
    #define SECURITY_LOCAL_RID                (0X00000000L)

    #define SECURITY_CREATOR_OWNER_RID        (0x00000000L)
    #define SECURITY_CREATOR_GROUP_RID        (0x00000001L)

    #define SECURITY_CREATOR_OWNER_SERVER_RID (0x00000002L)
    #define SECURITY_CREATOR_GROUP_SERVER_RID (0x00000003L)

    // NT well-known SIDs:
    //
    //     NT Authority          S-1-5
    //     Dialup                S-1-5-1
    //
    //     Network               S-1-5-2
    //     Batch                 S-1-5-3
    //     Interactive           S-1-5-4
    //     Service               S-1-5-6
    //     AnonymousLogon        S-1-5-7       (aka null logon session)
    //     Proxy                 S-1-5-8
    //     ServerLogon           S-1-5-8       (aka domain controller
    //                                            account)
    //
    //     (Logon IDs)           S-1-5-5-X-Y
    //
    //     (NT non-unique IDs)   S-1-5-0x15-...
    //
    //     (Built-in domain)     S-1-5-0x20

    #define SECURITY_NT_AUTHORITY           {0,0,0,0,0,5}   // ntifs

    #define SECURITY_DIALUP_RID             (0x00000001L)
    #define SECURITY_NETWORK_RID            (0x00000002L)
    #define SECURITY_BATCH_RID              (0x00000003L)
    #define SECURITY_INTERACTIVE_RID        (0x00000004L)
    #define SECURITY_SERVICE_RID            (0x00000006L)
    #define SECURITY_ANONYMOUS_LOGON_RID    (0x00000007L)
    #define SECURITY_PROXY_RID              (0x00000008L)
    #define SECURITY_SERVER_LOGON_RID       (0x00000009L)

    #define SECURITY_LOGON_IDS_RID          (0x00000005L)
    #define SECURITY_LOGON_IDS_RID_COUNT    (3L)

    #define SECURITY_LOCAL_SYSTEM_RID       (0x00000012L)

    #define SECURITY_NT_NON_UNIQUE          (0x00000015L)

    #define SECURITY_BUILTIN_DOMAIN_RID     (0x00000020L)

    If no Command line arguments are specified, names are looked up
    on the local machine. If argv[1] is present, the lookup occurs
    on the specified machine.

    For example, acctname.exe \\winbase will look up names from the
    machine named \\winbase. If \\winbase is a default German install
    of Windows NT, names will appear in German locale.

Author:

    Scott Field (sfield)    02-Oct-96

--*/

#pragma once


BOOL
LookupAliasFromRid(
    LPWSTR TargetComputer,
    DWORD Rid,
    LPWSTR Name,
    PDWORD cchName
    );

BOOL
LookupUserGroupFromRid(
    LPWSTR TargetComputer,
    DWORD Rid,
    LPWSTR Name,
    PDWORD cchName
    );

