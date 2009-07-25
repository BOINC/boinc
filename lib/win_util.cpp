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


#ifdef _BOINC_DLL
#include "stdafx.h"
#else
#include "boinc_win.h"
#endif

#include "win_util.h"


/**
 * Find out if we are on a Windows 2000 compatible system
 **/
BOOL IsWindows2000Compatible() {
   OSVERSIONINFO osvi;
   ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
        return FALSE;

    return (osvi.dwMajorVersion >= 5);
}

/**
 * Define these if they aren't defined.  They are normally found in
 * winnt.h, but some compilers don't have them.
 **/

#ifndef VER_AND
#define VER_AND 6
#endif

#ifndef VER_SUITENAME
#define VER_SUITENAME 0x0000040
#endif

#ifndef VER_SUITE_SINGLEUSERTS
#define VER_SUITE_SINGLEUSERTS              0x00000100
#endif

/**
 * This function performs the basic check to see if
 * the platform on which it is running is Terminal
 * services enabled.  Note, this code is compatible on
 * all Win32 platforms.  For the Windows 2000 platform
 * we perform a "lazy" bind to the new product suite
 * APIs that were first introduced on that platform.
 **/
BOOL IsTerminalServicesEnabled() {
    BOOL    bResult = FALSE;    // assume Terminal Services is not enabled

    DWORD   dwVersion;
    OSVERSIONINFOEXA osVersionInfo;
    DWORDLONG dwlConditionMask = 0;

    dwVersion = GetVersion();

    // are we running NT ?
    if (!(dwVersion & 0x80000000))
    {
        // Is it Windows 2000 (NT 5.0) or greater ?
        if (LOBYTE(LOWORD(dwVersion)) > 4)
        {
            // In Windows 2000 or better we need to use the Product Suite APIs
            dwlConditionMask = VerSetConditionMask( dwlConditionMask, VER_SUITENAME, VER_AND );

            ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
            osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
            bResult = VerifyVersionInfoA(
                          &osVersionInfo,
                          VER_SUITENAME,
                          dwlConditionMask);
        }
    }

    return bResult;
}


/**
 * This function terminates a process by process id instead of a handle.
 **/
BOOL TerminateProcessById( DWORD dwProcessID ) {
    HANDLE hProcess;
    BOOL bRetVal = FALSE;

    hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, dwProcessID );

    if (hProcess) {
        bRetVal = TerminateProcess(hProcess, 1);
    }

    CloseHandle( hProcess );

    return bRetVal;
}


/**
 * This function adjusts the specified WindowStation to include the specfied
 *   user.
 *
 * See: http://msdn2.microsoft.com/en-us/library/aa379608(VS.85).aspx
 **/
BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid)
{
   ACCESS_ALLOWED_ACE   *pace = NULL;
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bSuccess = FALSE;
   DWORD                dwNewAclSize;
   DWORD                dwSidSize = 0;
   DWORD                dwSdSizeNeeded;
   PACL                 pacl = NULL;
   PACL                 pNewAcl = NULL;
   PSECURITY_DESCRIPTOR psd = NULL;
   PSECURITY_DESCRIPTOR psdNew = NULL;
   PVOID                pTempAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   try
   {
      // Obtain the DACL for the window station.

      if (!GetUserObjectSecurity(
             hwinsta,
             &si,
             psd,
             dwSidSize,
             &dwSdSizeNeeded)
      )
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
         psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
               GetProcessHeap(),
               HEAP_ZERO_MEMORY,
               dwSdSizeNeeded);

         if (psd == NULL)
            throw;

         psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
               GetProcessHeap(),
               HEAP_ZERO_MEMORY,
               dwSdSizeNeeded);

         if (psdNew == NULL)
            throw;

         dwSidSize = dwSdSizeNeeded;

         if (!GetUserObjectSecurity(
               hwinsta,
               &si,
               psd,
               dwSidSize,
               &dwSdSizeNeeded)
         )
            throw;
      }
      else
         throw;

      // Create a new DACL.

      if (!InitializeSecurityDescriptor(
            psdNew,
            SECURITY_DESCRIPTOR_REVISION)
      )
         throw;

      // Get the DACL from the security descriptor.

      if (!GetSecurityDescriptorDacl(
            psd,
            &bDaclPresent,
            &pacl,
            &bDaclExist)
      )
         throw;

      // Initialize the ACL.

      ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
      aclSizeInfo.AclBytesInUse = sizeof(ACL);

      // Call only if the DACL is not NULL.

      if (pacl != NULL)
      {
         // get the file ACL size info
         if (!GetAclInformation(
               pacl,
               (LPVOID)&aclSizeInfo,
               sizeof(ACL_SIZE_INFORMATION),
               AclSizeInformation)
         )
            throw;
      }

      // Compute the size of the new ACL.

      dwNewAclSize = aclSizeInfo.AclBytesInUse +
            (2*sizeof(ACCESS_ALLOWED_ACE)) + (2*GetLengthSid(psid)) -
            (2*sizeof(DWORD));

      // Allocate memory for the new ACL.

      pNewAcl = (PACL)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwNewAclSize);

      if (pNewAcl == NULL)
         throw;

      // Initialize the new DACL.

      if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
         throw;

      // If DACL is present, copy it to a new DACL.

      if (bDaclPresent)
      {
         // Copy the ACEs to the new ACL.
         if (aclSizeInfo.AceCount)
         {
            for (i=0; i < aclSizeInfo.AceCount; i++)
            {
               // Get an ACE.
               if (!GetAce(pacl, i, &pTempAce))
                  throw;

               // Add the ACE to the new ACL.
               if (!AddAce(
                     pNewAcl,
                     ACL_REVISION,
                     MAXDWORD,
                     pTempAce,
                    ((PACE_HEADER)pTempAce)->AceSize)
               )
                  throw;
            }
         }
      }

      // Add the first ACE to the window station.

      pace = (ACCESS_ALLOWED_ACE *)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid) -
                  sizeof(DWORD));

      if (pace == NULL)
         throw;

      pace->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
      pace->Header.AceFlags = CONTAINER_INHERIT_ACE |
                   INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
      pace->Header.AceSize  = sizeof(ACCESS_ALLOWED_ACE) +
                   GetLengthSid(psid) - sizeof(DWORD);
      pace->Mask            = GENERIC_ALL;

      if (!CopySid(GetLengthSid(psid), &pace->SidStart, psid))
         throw;

      if (!AddAce(
            pNewAcl,
            ACL_REVISION,
            MAXDWORD,
            (LPVOID)pace,
            pace->Header.AceSize)
      )
         throw;

      // Add an ACE to the window station.

      pace->Header.AceFlags = NO_PROPAGATE_INHERIT_ACE;
      pace->Mask            = GENERIC_ALL;

      if (!AddAce(
            pNewAcl,
            ACL_REVISION,
            MAXDWORD,
            (LPVOID)pace,
            pace->Header.AceSize)
      )
         throw;

      // Set a new DACL for the security descriptor.

      if (!SetSecurityDescriptorDacl(
            psdNew,
            TRUE,
            pNewAcl,
            FALSE)
      )
         throw;

      // Set the new security descriptor for the window station.

      if (!SetUserObjectSecurity(hwinsta, &si, psdNew))
         throw;

      // Indicate success.

      bSuccess = TRUE;
   }
   catch(...)
   {
      // Free the allocated buffers.

      if (pace != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pace);

      if (pNewAcl != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

      if (psd != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

      if (psdNew != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
   }

   return bSuccess;

}


/**
 * This function adjusts the specified Desktop to include the specfied
 *   user.
 *
 * See: http://msdn2.microsoft.com/en-us/library/aa379608(VS.85).aspx
 **/
BOOL AddAceToDesktop(HDESK hdesk, PSID psid)
{
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bSuccess = FALSE;
   DWORD                dwNewAclSize;
   DWORD                dwSidSize = 0;
   DWORD                dwSdSizeNeeded;
   PACL                 pacl = NULL;
   PACL                 pNewAcl = NULL;
   PSECURITY_DESCRIPTOR psd = NULL;
   PSECURITY_DESCRIPTOR psdNew = NULL;
   PVOID                pTempAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   try
   {
      // Obtain the security descriptor for the desktop object.

      if (!GetUserObjectSecurity(
            hdesk,
            &si,
            psd,
            dwSidSize,
            &dwSdSizeNeeded))
      {
         if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
         {
            psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
                  GetProcessHeap(),
                  HEAP_ZERO_MEMORY,
                  dwSdSizeNeeded );

            if (psd == NULL)
               throw;

            psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
                  GetProcessHeap(),
                  HEAP_ZERO_MEMORY,
                  dwSdSizeNeeded);

            if (psdNew == NULL)
               throw;

            dwSidSize = dwSdSizeNeeded;

            if (!GetUserObjectSecurity(
                  hdesk,
                  &si,
                  psd,
                  dwSidSize,
                  &dwSdSizeNeeded)
            )
               throw;
         }
         else
            throw;
      }

      // Create a new security descriptor.

      if (!InitializeSecurityDescriptor(
            psdNew,
            SECURITY_DESCRIPTOR_REVISION)
      )
         throw;

      // Obtain the DACL from the security descriptor.

      if (!GetSecurityDescriptorDacl(
            psd,
            &bDaclPresent,
            &pacl,
            &bDaclExist)
      )
         throw;

      // Initialize.

      ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
      aclSizeInfo.AclBytesInUse = sizeof(ACL);

      // Call only if NULL DACL.

      if (pacl != NULL)
      {
         // Determine the size of the ACL information.

         if (!GetAclInformation(
               pacl,
               (LPVOID)&aclSizeInfo,
               sizeof(ACL_SIZE_INFORMATION),
               AclSizeInformation)
         )
            throw;
      }

      // Compute the size of the new ACL.

      dwNewAclSize = aclSizeInfo.AclBytesInUse +
            sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid(psid) - sizeof(DWORD);

      // Allocate buffer for the new ACL.

      pNewAcl = (PACL)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwNewAclSize);

      if (pNewAcl == NULL)
         throw;

      // Initialize the new ACL.

      if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
         throw;

      // If DACL is present, copy it to a new DACL.

      if (bDaclPresent)
      {
         // Copy the ACEs to the new ACL.
         if (aclSizeInfo.AceCount)
         {
            for (i=0; i < aclSizeInfo.AceCount; i++)
            {
               // Get an ACE.
               if (!GetAce(pacl, i, &pTempAce))
                  throw;

               // Add the ACE to the new ACL.
               if (!AddAce(
                  pNewAcl,
                  ACL_REVISION,
                  MAXDWORD,
                  pTempAce,
                  ((PACE_HEADER)pTempAce)->AceSize)
               )
                  throw;
            }
         }
      }

      // Add ACE to the DACL.

      if (!AddAccessAllowedAce(
            pNewAcl,
            ACL_REVISION,
            GENERIC_ALL,
            psid)
      )
         throw;

      // Set new DACL to the new security descriptor.

      if (!SetSecurityDescriptorDacl(
            psdNew,
            TRUE,
            pNewAcl,
            FALSE)
      )
         throw;

      // Set the new security descriptor for the desktop object.

      if (!SetUserObjectSecurity(hdesk, &si, psdNew))
         throw;

      // Indicate success.

      bSuccess = TRUE;
   }
   catch(...)
   {
      // Free buffers.

      if (pNewAcl != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

      if (psd != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

      if (psdNew != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
   }

   return bSuccess;
}


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

        if(*Sid == NULL) throw;

        ReferencedDomain = (LPSTR)HeapAlloc(
                        GetProcessHeap(),
                        0,
                        cchReferencedDomain * sizeof(CHAR)
                        );

        if(ReferencedDomain == NULL) throw;

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
                if(*Sid == NULL) throw;

                ReferencedDomain = (LPSTR)HeapReAlloc(
                            GetProcessHeap(),
                            0,
                            ReferencedDomain,
                            cchReferencedDomain * sizeof(CHAR)
                            );
                if(ReferencedDomain == NULL) throw;
            }
            else throw;
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


// Suspend or resume the threads in a given process.
// The only way to do this on Windows is to enumerate
// all the threads in the entire system,
// and find those belonging to the process (ugh!!)
//
int suspend_or_resume_threads(DWORD pid, bool resume) { 
    HANDLE threads, thread;
    THREADENTRY32 te = {0}; 
 
    threads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if (threads == INVALID_HANDLE_VALUE) return -1;
 
    te.dwSize = sizeof(THREADENTRY32); 
    if (!Thread32First(threads, &te)) { 
        CloseHandle(threads); 
        return -1;
    }

    do { 
        if (te.th32OwnerProcessID == pid) {
            thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            resume ?  ResumeThread(thread) : SuspendThread(thread);
            CloseHandle(thread);
        } 
    } while (Thread32Next(threads, &te)); 

    CloseHandle (threads); 

    return 0;
} 


// Change the current working directory to the data directory
//
void chdir_to_data_dir() {
	LONG    lReturnValue;
	HKEY    hkSetupHive;
    LPTSTR  lpszRegistryValue = NULL;
	DWORD   dwSize = 0;

    // change the current directory to the boinc data directory if it exists
	lReturnValue = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, 
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),  
		0, 
        KEY_READ,
        &hkSetupHive
    );
    if (lReturnValue == ERROR_SUCCESS) {
        // How large does our buffer need to be?
        lReturnValue = RegQueryValueEx(
            hkSetupHive,
            _T("DATADIR"),
            NULL,
            NULL,
            NULL,
            &dwSize
        );
        if (lReturnValue != ERROR_FILE_NOT_FOUND) {
            // Allocate the buffer space.
            lpszRegistryValue = (LPTSTR) malloc(dwSize);
            (*lpszRegistryValue) = NULL;

            // Now get the data
            lReturnValue = RegQueryValueEx( 
                hkSetupHive,
                _T("DATADIR"),
                NULL,
                NULL,
                (LPBYTE)lpszRegistryValue,
                &dwSize
            );

            SetCurrentDirectory(lpszRegistryValue);
        }
    }

	if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszRegistryValue) free(lpszRegistryValue);
}


#if 0

// return true if running under remote desktop
// (in which case CUDA apps don't work)
//
bool is_remote_desktop() {
    LPTSTR *buf;
    DWORD len;

    if (WTSQuerySessionInformation(
        WTS_CURRENT_SERVER_HANDLE,
        WTS_CURRENT_SESSION,
        WTSClientProtocolType,
        &buf,
        &len
    )) {

        USHORT prot = *(USHORT*) buf;
        WTSFreeMemory(buf);

        if (prot == 2) return true;
    }

    return false;
}

#endif

