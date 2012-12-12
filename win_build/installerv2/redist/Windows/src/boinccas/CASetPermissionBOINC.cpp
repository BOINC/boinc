// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "stdafx.h"
#include "boinccas.h"
#include "CASetPermissionBOINC.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CASetPermissionBOINC")
#define CUSTOMACTION_PROGRESSTITLE      _T("Setting permissions on the BOINC Executable directory.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CASetPermissionBOINC::CASetPermissionBOINC(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CASetPermissionBOINC::~CASetPermissionBOINC()
{
    BOINCCABase::~BOINCCABase();
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CASetPermissionBOINC::OnExecution()
{
    DWORD                   dwRes = 0;
    PACL                    pACL = NULL;
    PACL                    pOldACL = NULL;
    PSID                    psidEveryone = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    EXPLICIT_ACCESS         ea[3];
    ULONG                   ulEntries = 2;
    tstring                 strBOINCAdminsGroupAlias;
    tstring                 strBOINCUsersGroupAlias;
    tstring                 strBOINCInstallDirectory;
    tstring                 strEnableUseByAllUsers;
    UINT                    uiReturnValue = -1;

    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

    uiReturnValue = GetProperty( _T("BOINC_ADMINS_GROUPNAME"), strBOINCAdminsGroupAlias );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_USERS_GROUPNAME"), strBOINCUsersGroupAlias );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strBOINCInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLEUSEBYALLUSERS"), strEnableUseByAllUsers );
    if ( uiReturnValue ) return uiReturnValue;


    // Initialize an EXPLICIT_ACCESS structure for all ACEs.
    ZeroMemory(&ea, 3 * sizeof(EXPLICIT_ACCESS));

    // boinc_admins
    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR)strBOINCAdminsGroupAlias.c_str();

    // boinc_users
    ea[1].grfAccessPermissions = GENERIC_READ|GENERIC_EXECUTE;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPTSTR)strBOINCUsersGroupAlias.c_str();

    // Everyone
    if (_T("1") == strEnableUseByAllUsers) {

        // Create a well-known SID for the Everyone group.
        if(!AllocateAndInitializeSid(
                         &SIDAuthWorld,
                         1,
                         SECURITY_WORLD_RID,
                         0, 0, 0, 0, 0, 0, 0,
                         &psidEveryone
          ))
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                GetLastError(),
                _T("AllocateAndInitializeSid Error for Everyone group")
            );
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                GetLastError(),
                _T("AllocateAndInitializeSid Error for Everyone group")
            );
        }

        ulEntries = 3;

        ea[2].grfAccessPermissions = GENERIC_READ|GENERIC_EXECUTE;
        ea[2].grfAccessMode = SET_ACCESS;
        ea[2].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[2].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[2].Trustee.ptstrName  = (LPTSTR)psidEveryone;
    }

    // Get the old ACL for the directory
    dwRes = GetNamedSecurityInfo(
        (LPWSTR)strBOINCInstallDirectory.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        NULL,
        NULL,
        &pOldACL,
        NULL,
        &pSD
    );
    if (ERROR_SUCCESS != dwRes) 
    {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("GetNamedSecurityInfo Error")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("GetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Create a new ACL that contains the new ACEs and merges the old.
    dwRes = SetEntriesInAcl(ulEntries, &ea[0], pOldACL, &pACL);
    if (ERROR_SUCCESS != dwRes) 
    {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetEntriesInAcl Error")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetEntriesInAcl Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Set the ACL on the Data Directory itself.
    dwRes = SetNamedSecurityInfo( 
        (LPWSTR)strBOINCInstallDirectory.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        NULL,
        NULL,
        pACL,
        NULL
    );
    if (ERROR_SUCCESS != dwRes) 
    {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetNamedSecurityInfo Error")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Set ACLs on all files and sub folders.
    RecursiveSetPermissions(strBOINCInstallDirectory, pACL);


    if (pACL) 
        LocalFree(pACL);
    if (pSD)
        LocalFree(pSD);
    if (psidEveryone)
        FreeSid(psidEveryone);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    SetPermissionBOINC
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall SetPermissionBOINC(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CASetPermissionBOINC* pCA = new CASetPermissionBOINC(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

