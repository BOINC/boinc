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
#include "CASetPermissionBOINCDataProjects.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CASetPermissionBOINCDataProjects")
#define CUSTOMACTION_PROGRESSTITLE      _T("Setting permissions on the BOINC Projects directory.")


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CASetPermissionBOINCDataProjects::CASetPermissionBOINCDataProjects(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CASetPermissionBOINCDataProjects::~CASetPermissionBOINCDataProjects()
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
UINT CASetPermissionBOINCDataProjects::OnExecution()
{
    DWORD               dwRes = 0;
    PACL                pACL = NULL;
    ULONGLONG           rgSidSY[(SECURITY_MAX_SID_SIZE+sizeof(ULONGLONG)-1)/sizeof(ULONGLONG)]={0};
    ULONGLONG           rgSidBA[(SECURITY_MAX_SID_SIZE+sizeof(ULONGLONG)-1)/sizeof(ULONGLONG)]={0};
    ULONGLONG           rgSidBU[(SECURITY_MAX_SID_SIZE+sizeof(ULONGLONG)-1)/sizeof(ULONGLONG)]={0};
    DWORD               dwSidSize;
    EXPLICIT_ACCESS     ea[6];
    ULONG               ulEntries = 0;
    tstring             strBOINCAdminsGroupAlias;
    tstring             strBOINCUsersGroupAlias;
    tstring             strBOINCProjectsGroupAlias;
    tstring             strBOINCDataDirectory;
    tstring             strBOINCDataProjectsDirectory;
    tstring             strEnableProtectedApplicationExecution;
    tstring             strEnableUseByAllUsers;
    UINT                uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_ADMINS_GROUPNAME"), strBOINCAdminsGroupAlias );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_USERS_GROUPNAME"), strBOINCUsersGroupAlias );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECTS_GROUPNAME"), strBOINCProjectsGroupAlias );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("DATADIR"), strBOINCDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"), strEnableProtectedApplicationExecution );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLEUSEBYALLUSERS"), strEnableUseByAllUsers );
    if ( uiReturnValue ) return uiReturnValue;


    // If we are not installing as a service, we do not need to modify the
    // projects directory permissions
    if (_T("1") != strEnableProtectedApplicationExecution) {
        return ERROR_SUCCESS;
    }


    strBOINCDataProjectsDirectory = strBOINCDataDirectory + _T("\\projects");


    // Initialize an EXPLICIT_ACCESS structure for all ACEs.
    ZeroMemory(&ea, 6 * sizeof(EXPLICIT_ACCESS));

    // Create a SID for the SYSTEM.
    dwSidSize = sizeof( rgSidSY );
    if(!CreateWellKnownSid(WinLocalSystemSid, NULL, rgSidSY, &dwSidSize))
    {
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            GetLastError(),
            _T("CreateWellKnownSid Error for SYSTEM")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Create a SID for the Administrators group.
    dwSidSize = sizeof( rgSidBA );
    if(!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, rgSidBA, &dwSidSize))
    {
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            GetLastError(),
            _T("CreateWellKnownSid Error for BUILTIN\\Administrators")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Create a SID for the Users group.
    dwSidSize = sizeof( rgSidBU );
    if(!CreateWellKnownSid(WinBuiltinUsersSid, NULL, rgSidBU, &dwSidSize))
    {
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            GetLastError(),
            _T("CreateWellKnownSid Error for BUILTIN\\Users")
        );
        return ERROR_INSTALL_FAILURE;
    }

    ulEntries = 5;

    // SYSTEM
    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR)rgSidSY;

    // Administrators
    ea[1].grfAccessPermissions = GENERIC_ALL;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[1].Trustee.ptstrName  = (LPTSTR)rgSidBA;

    // boinc_admins
    ea[2].grfAccessPermissions = GENERIC_ALL;
    ea[2].grfAccessMode = SET_ACCESS;
    ea[2].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[2].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[2].Trustee.ptstrName  = (LPTSTR)strBOINCAdminsGroupAlias.c_str();

    // boinc_users
    ea[3].grfAccessPermissions = GENERIC_READ|GENERIC_EXECUTE;
    ea[3].grfAccessMode = SET_ACCESS;
    ea[3].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[3].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[3].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[3].Trustee.ptstrName  = (LPTSTR)strBOINCUsersGroupAlias.c_str();

    // boinc_projects
    ea[4].grfAccessPermissions = GENERIC_ALL;
    ea[4].grfAccessMode = SET_ACCESS;
    ea[4].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[4].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[4].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[4].Trustee.ptstrName  = (LPTSTR)strBOINCProjectsGroupAlias.c_str();

    // Users
    if (_T("1") == strEnableUseByAllUsers) {
        ulEntries = 6;

        ea[5].grfAccessPermissions = GENERIC_READ|GENERIC_EXECUTE;
        ea[5].grfAccessMode = SET_ACCESS;
        ea[5].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[5].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[5].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[5].Trustee.ptstrName  = (LPTSTR)rgSidBU;
    }

    // Create a new ACL that contains the new ACEs.
    dwRes = SetEntriesInAcl(ulEntries, &ea[0], NULL, &pACL);
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
        (LPWSTR)strBOINCDataProjectsDirectory.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
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
    RecursiveSetPermissions(strBOINCDataProjectsDirectory, pACL);


    if (pACL)
        LocalFree(pACL);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    SetPermissionBOINCDataProjects
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall SetPermissionBOINCDataProjects(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CASetPermissionBOINCDataProjects* pCA = new CASetPermissionBOINCDataProjects(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

