/*++

DCOM Permission Configuration Sample
Copyright (c) 1996, Microsoft Corporation. All rights reserved.

Module Name:

    utils.cpp

Abstract:

    Miscellaneous utility functions

Author:

    Michael Nelson

Environment:

    Windows NT

--*/
// the following fixes mysterious/sporadic errors in Win include files
//#define _NTDEF_
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "ntsecapi.h"
#include "dcomperm.h"

DWORD
GetCurrentUserSID (
    PSID *Sid
    )
{
    TOKEN_USER  *tokenUser=NULL;
    HANDLE      tokenHandle;
    DWORD       tokenSize;
    DWORD       sidLength;

    if (OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
    {
        GetTokenInformation (tokenHandle,
                             TokenUser,
                             tokenUser,
                             0,
                             &tokenSize);

        tokenUser = (TOKEN_USER *) malloc (tokenSize);

        if (GetTokenInformation (tokenHandle,
                                 TokenUser,
                                 tokenUser,
                                 tokenSize,
                                 &tokenSize))
        {
            sidLength = GetLengthSid (tokenUser->User.Sid);
            *Sid = (PSID) malloc (sidLength);

            memcpy (*Sid, tokenUser->User.Sid, sidLength);
            CloseHandle (tokenHandle);
        } else
        {
            free (tokenUser);
            return GetLastError();
        }
    } else
    {
        free (tokenUser);
        return GetLastError();
    }

    free (tokenUser);
    return ERROR_SUCCESS;
}
