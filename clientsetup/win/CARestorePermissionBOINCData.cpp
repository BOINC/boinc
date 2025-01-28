// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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
#include "CARestorePermissionBOINCData.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CARestorePermissionBOINCData")
#define CUSTOMACTION_PROGRESSTITLE      _T("Restoring permissions on the BOINC Data directory.")


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CARestorePermissionBOINCData::CARestorePermissionBOINCData(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CARestorePermissionBOINCData::~CARestorePermissionBOINCData()
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
UINT CARestorePermissionBOINCData::OnExecution()
{
    DWORD               dwRes = 0;
    PACL                pACL = NULL;
    ULONGLONG           rgSidSY[(SECURITY_MAX_SID_SIZE+sizeof(ULONGLONG)-1)/sizeof(ULONGLONG)]={0};
    ULONGLONG           rgSidBA[(SECURITY_MAX_SID_SIZE+sizeof(ULONGLONG)-1)/sizeof(ULONGLONG)]={0};
    DWORD               dwSidSize;
    EXPLICIT_ACCESS     ea[2];
    ULONG               ulEntries = 0;
    tstring             strBOINCDataDirectory;
    UINT                uiReturnValue;

    uiReturnValue = GetProperty( _T("DATADIR"), strBOINCDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;


    // Initialize an EXPLICIT_ACCESS structure for all ACEs.
    ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

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

    ulEntries = 2;

    // SYSTEM
    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = (LPTSTR)rgSidSY;

    // Administrators
    ea[1].grfAccessPermissions = GENERIC_ALL;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[1].Trustee.ptstrName = (LPTSTR)rgSidBA;

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
        (LPWSTR)strBOINCDataDirectory.c_str(),
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
    RecursiveSetPermissions(strBOINCDataDirectory, pACL);


    if (pACL)
        LocalFree(pACL);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    RestorePermissionBOINCData
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall RestorePermissionBOINCData(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CARestorePermissionBOINCData* pCA = new CARestorePermissionBOINCData(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

