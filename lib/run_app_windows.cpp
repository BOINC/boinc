// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// Code to run a BOINC application (main or graphics) under Windows
// Don't include this in applications

#include "boinc_win.h"

#include "win_util.h"
#include "filesys.h"
#include "error_numbers.h"
#include "str_replace.h"
#include "common_defs.h"
#include "util.h"
#include "parse.h"
#include "base64.h"

HANDLE sandbox_account_interactive_token = NULL;
HANDLE sandbox_account_service_token = NULL;

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
    LPCSTR SystemName,
    LPCSTR AccountName,
    PSID *Sid
    )
{
    LPSTR ReferencedDomain=NULL;
    DWORD cbSid=128;    // initial allocation attempt
    DWORD cchReferencedDomain=16; // initial allocation size
    SID_NAME_USE peUse;
    BOOL bSuccess=FALSE; // assume this function will fail

    try
    {
        //
        // initial memory allocations
        //
        *Sid = (PSID)HeapAlloc(GetProcessHeap(), 0, cbSid);

        if(*Sid == NULL) throw 0;

        ReferencedDomain = (LPSTR)HeapAlloc(
                        GetProcessHeap(),
                        0,
                        cchReferencedDomain * sizeof(CHAR)
                        );

        if(ReferencedDomain == NULL) throw 0;

        //
        // Obtain the SID of the specified account on the specified system.
        //
        while(!LookupAccountNameA(
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
                if(*Sid == NULL) throw 0;

                ReferencedDomain = (LPSTR)HeapReAlloc(
                            GetProcessHeap(),
                            0,
                            ReferencedDomain,
                            cchReferencedDomain * sizeof(CHAR)
                            );
                if(ReferencedDomain == NULL) throw 0;
            }
            else throw 0;
        }

        //
        // Indicate success.
        //
        bSuccess = TRUE;

    } // try
    catch(...)
    {
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


void get_sandbox_account_interactive_token() {
    FILE*       f;
    char        buf[256];
    std::string encoded_username_str;
    std::string encoded_password_str;
    std::string username_str;
    std::string domainname_str;
    std::string password_str;
    int         retval = 0;
    static bool first = true;

    if (!first) return;
    first = false;

    f = fopen(CLIENT_AUTH_FILENAME, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<username>", encoded_username_str)) continue;
        if (parse_str(buf, "<password>", encoded_password_str)) continue;
    }
    fclose(f);

    password_str = r_base64_decode(encoded_password_str);

    if (std::string::npos != encoded_username_str.find('\\')) {
        domainname_str = encoded_username_str.substr(
            0, encoded_username_str.find('\\')
        );
        username_str = encoded_username_str.substr(
            encoded_username_str.rfind(_T('\\')) + 1,
            encoded_username_str.length() - encoded_username_str.rfind(_T('\\')) - 1
        );
        retval = LogonUserA(
            username_str.c_str(),
            domainname_str.c_str(),
            password_str.c_str(),
            LOGON32_LOGON_INTERACTIVE,
            LOGON32_PROVIDER_DEFAULT,
            &sandbox_account_interactive_token
        );
    } else {
        username_str = encoded_username_str;
        retval = LogonUserA(
            username_str.c_str(),
            NULL,
            password_str.c_str(),
            LOGON32_LOGON_INTERACTIVE,
            LOGON32_PROVIDER_DEFAULT,
            &sandbox_account_interactive_token
        );
    }

    if (!retval) {
        sandbox_account_interactive_token = NULL;
    } else {


    }
}

void get_sandbox_account_service_token() {
    ACCESS_ALLOWED_ACE   *pace1 = NULL;
    ACCESS_ALLOWED_ACE   *pace2 = NULL;
    ACL_SIZE_INFORMATION aclSizeInfo;
    DWORD                dwNewAclSize;
    PACL                 pOldAcl = NULL;
    PACL                 pNewAcl = NULL;
    PSID                 pBOINCProjectSID = NULL;
    PSID                 pBOINCMasterSID = NULL;
    PTOKEN_DEFAULT_DACL  pTokenDefaultDACL = NULL;
    PVOID                pTempAce;
    DWORD                dwSize = 0;
    DWORD                dwSizeNeeded = 0;
    LPTSTR               pszUserName = NULL;
    FILE* f;
    char buf[256];
    std::string encoded_username_str;
    std::string encoded_password_str;
    std::string username_str;
    std::string domainname_str;
    std::string password_str;
    int retval = 0;
    static bool first=true;

    if (!first) return;
    first = false;

    f = fopen(CLIENT_AUTH_FILENAME, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<username>", encoded_username_str)) continue;
        if (parse_str(buf, "<password>", encoded_password_str)) continue;
    }
    fclose(f);

    password_str = r_base64_decode(encoded_password_str);

    if (std::string::npos != encoded_username_str.find('\\')) {
        domainname_str = encoded_username_str.substr(
            0, encoded_username_str.find('\\')
        );
        username_str = encoded_username_str.substr(
            encoded_username_str.rfind(_T('\\')) + 1,
            encoded_username_str.length() - encoded_username_str.rfind(_T('\\')) - 1
        );
        retval = LogonUserA(
            username_str.c_str(),
            domainname_str.c_str(),
            password_str.c_str(),
            LOGON32_LOGON_SERVICE,
            LOGON32_PROVIDER_DEFAULT,
            &sandbox_account_service_token
        );
        if (retval) {
            GetAccountSid(domainname_str.c_str(), username_str.c_str(), &pBOINCProjectSID);
        }
    } else {
        username_str = encoded_username_str;
        retval = LogonUserA(
            username_str.c_str(),
            NULL,
            password_str.c_str(),
            LOGON32_LOGON_SERVICE,
            LOGON32_PROVIDER_DEFAULT,
            &sandbox_account_service_token
        );
        if (retval) {
            GetAccountSid(NULL, username_str.c_str(), &pBOINCProjectSID);
        }
    }

    if (!retval) {
        sandbox_account_service_token = NULL;
    } else {

        try
        {
            // Obtain the current user name.

            dwSize = 0;
            dwSizeNeeded = 0;
            if (!GetUserNameEx(
                    NameSamCompatible,
                    pszUserName,
                    &dwSize)
            )
            if (GetLastError() == ERROR_MORE_DATA)
            {
                pszUserName = (LPTSTR)HeapAlloc(
                    GetProcessHeap(),
                    HEAP_ZERO_MEMORY,
                    dwSize + 1);

                if (pszUserName == NULL)
                    throw 0;

                if (!GetUserNameEx(
                        NameSamCompatible,
                        pszUserName,
                        &dwSize)
                )
                    throw 0;
            }
            else
                throw 0;

            // Obtain the SID for the current user name.

            if (!GetAccountSid(
                    NULL,
                    pszUserName,
                    &pBOINCMasterSID)
            )
                throw 0;

            // Obtain the DACL for the service token.

            dwSize = 0;
            dwSizeNeeded = 0;
            if (!GetTokenInformation(
                    sandbox_account_service_token,
                    TokenDefaultDacl,
                    NULL,
                    dwSize,
                    &dwSizeNeeded)
            )
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                pTokenDefaultDACL = (PTOKEN_DEFAULT_DACL)HeapAlloc(
                    GetProcessHeap(),
                    HEAP_ZERO_MEMORY,
                    dwSizeNeeded);

                if (pTokenDefaultDACL == NULL)
                    throw 0;

                dwSize = dwSizeNeeded;

                if (!GetTokenInformation(
                        sandbox_account_service_token,
                        TokenDefaultDacl,
                        pTokenDefaultDACL,
                        dwSize,
                        &dwSizeNeeded)
                )
                    throw 0;
            }
            else
                throw 0;

            //
            pOldAcl = pTokenDefaultDACL->DefaultDacl;

            // Initialize the ACL.

            ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
            aclSizeInfo.AclBytesInUse = sizeof(ACL);

            // Call only if the DACL is not NULL.

            if (pOldAcl != NULL)
            {
                // get the file ACL size info
                if (!GetAclInformation(
                    pOldAcl,
                    (LPVOID)&aclSizeInfo,
                    sizeof(ACL_SIZE_INFORMATION),
                    AclSizeInformation)
                )
                   throw 0;
            }

            // Compute the size of the new ACL.

            dwNewAclSize = aclSizeInfo.AclBytesInUse +
                (2*sizeof(ACCESS_ALLOWED_ACE)) +
                (2*GetLengthSid(pBOINCProjectSID)) +
                (2*GetLengthSid(pBOINCMasterSID));

            // Allocate memory for the new ACL.

            pNewAcl = (PACL)HeapAlloc(
                GetProcessHeap(),
                HEAP_ZERO_MEMORY,
                dwNewAclSize);

            if (pNewAcl == NULL)
                throw 0;

            // Initialize the new DACL.

            if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
                throw 0;

            // If DACL is present, copy it to a new DACL.

            if (pOldAcl)
            {
                // Copy the ACEs to the new ACL.
                if (aclSizeInfo.AceCount)
                {
                    for (unsigned int i=0; i < aclSizeInfo.AceCount; i++)
                    {
                        // Get an ACE.
                        if (!GetAce(pOldAcl, i, &pTempAce))
                            throw 0;

                        // Add the ACE to the new ACL.
                        if (!AddAce(
                                pNewAcl,
                                ACL_REVISION,
                                MAXDWORD,
                                pTempAce,
                            ((PACE_HEADER)pTempAce)->AceSize)
                        )
                            throw 0;
                    }
                }
            }

            // Add the first ACE to the process.

            pace1 = (ACCESS_ALLOWED_ACE *)HeapAlloc(
                GetProcessHeap(),
                HEAP_ZERO_MEMORY,
                sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pBOINCProjectSID) - sizeof(DWORD)
            );

            if (pace1 == NULL)
                throw 0;

            pace1->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
            pace1->Header.AceFlags = CONTAINER_INHERIT_ACE |
                                    INHERIT_ONLY_ACE |
                                    OBJECT_INHERIT_ACE;
            pace1->Header.AceSize  = (WORD)sizeof(ACCESS_ALLOWED_ACE) +
                                    (WORD)GetLengthSid(pBOINCProjectSID) -
                                    (WORD)sizeof(DWORD);
            pace1->Mask            = PROCESS_ALL_ACCESS;

            if (!CopySid(GetLengthSid(pBOINCProjectSID), &pace1->SidStart, pBOINCProjectSID))
                throw 0;

            // Add an ACE to the process.

            if (!AddAce(
                pNewAcl,
                ACL_REVISION,
                MAXDWORD,
                (LPVOID)pace1,
                pace1->Header.AceSize)
            )
                throw 0;

            // Add the second ACE to the process.

            pace2 = (ACCESS_ALLOWED_ACE *)HeapAlloc(
                GetProcessHeap(),
                HEAP_ZERO_MEMORY,
                sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pBOINCMasterSID) - sizeof(DWORD)
            );

            if (pace2 == NULL)
                throw 0;

            pace2->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
            pace2->Header.AceFlags = CONTAINER_INHERIT_ACE |
                                    INHERIT_ONLY_ACE |
                                    OBJECT_INHERIT_ACE;
            pace2->Header.AceSize  = (WORD)sizeof(ACCESS_ALLOWED_ACE) +
                                    (WORD)GetLengthSid(pBOINCMasterSID) -
                                    (WORD)sizeof(DWORD);
            pace2->Mask            = PROCESS_ALL_ACCESS;

            if (!CopySid(GetLengthSid(pBOINCMasterSID), &pace2->SidStart, pBOINCMasterSID))
                throw 0;

            // Add an ACE to the process.

            if (!AddAce(
                pNewAcl,
                ACL_REVISION,
                MAXDWORD,
                (LPVOID)pace2,
                pace2->Header.AceSize)
            )
                throw 0;

            // Set a new Default DACL for the token.
            pTokenDefaultDACL->DefaultDacl = pNewAcl;

            if (!SetTokenInformation(
                sandbox_account_service_token,
                TokenDefaultDacl,
                pTokenDefaultDACL,
                dwNewAclSize)
            )
                throw 0;

            // Indicate success.
            fprintf(stderr, "New Token ACL Success!!!\n");
        }
        catch(...)
        {
            // Free the allocated buffers.

            if (pace1 != NULL)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pace1);

            if (pace2 != NULL)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pace2);

            if (pOldAcl != NULL)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pOldAcl);

            if (pNewAcl != NULL)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

            if (pBOINCProjectSID != NULL)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pBOINCProjectSID);

            if (pBOINCMasterSID != NULL)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pBOINCMasterSID);

        }

        if (pszUserName != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pszUserName);

    }
}

// Run application, Windows.
// chdir into the given directory, and run a program there.
// argv is set up Unix-style, i.e. argv[0] is the program name
//

int run_app_windows(
    const char* dir, const char* file, int argc, char *const argv[], HANDLE& id
) {
    int retval;
    PROCESS_INFORMATION process_info;
    STARTUPINFOA startup_info;
    LPVOID environment_block = NULL;
    char cmdline[1024];
    char error_msg[1024];

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    safe_strcpy(cmdline, "");
    for (int i=0; i<argc; i++) {
        safe_strcat(cmdline, argv[i]);
        if (i<argc-1) {
            safe_strcat(cmdline, " ");
        }
    }

    get_sandbox_account_interactive_token();
    if (sandbox_account_interactive_token != NULL) {

        // Construct an environment block that contains environment variables that don't
        //   describe the current user.
        if (!CreateEnvironmentBlock(&environment_block, sandbox_account_interactive_token, FALSE)) {
            windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg));
            fprintf(stderr, "CreateEnvironmentBlock failed: %s\n", error_msg);
        }

        retval = CreateProcessAsUserA(
            sandbox_account_interactive_token,
            file,
            cmdline,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP|CREATE_UNICODE_ENVIRONMENT,
            environment_block,
            dir,
            &startup_info,
            &process_info
        );

        if (!DestroyEnvironmentBlock(environment_block)) {
            windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg));
            fprintf(stderr, "DestroyEnvironmentBlock failed: %s\n", error_msg);
        }

    } else {
        retval = CreateProcessA(
            file,
            cmdline,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            dir,
            &startup_info,
            &process_info
        );
    }
    if (!retval) {
        windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg));
        fprintf(stderr, "CreateProcess failed: '%s'\n", error_msg);
        return -1; // CreateProcess returns 1 if successful, false if it failed.
    }

    id = process_info.hProcess;
    return 0;
}
