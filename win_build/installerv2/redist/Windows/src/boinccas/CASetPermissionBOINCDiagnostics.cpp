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
#include "CASetPermissionBOINCDiagnostics.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CASetPermissionBOINCDiagnostics")
#define CUSTOMACTION_PROGRESSTITLE      _T("Setting permissions on the BOINC Diagnostics Framework.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CASetPermissionBOINCDiagnostics::CASetPermissionBOINCDiagnostics(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CASetPermissionBOINCDiagnostics::~CASetPermissionBOINCDiagnostics()
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
UINT CASetPermissionBOINCDiagnostics::OnExecution()
{
    DWORD                   dwRes = 0;
    PACL                    pACL = NULL;
    PACL                    pOldACL = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    EXPLICIT_ACCESS         ea;
    tstring                 strBOINCProjectsGroupAlias;
    tstring                 strBOINCInstallDirectory;
    tstring                 strTemp;
    UINT                    uiReturnValue = -1;

    uiReturnValue = GetProperty( _T("BOINC_PROJECTS_GROUPNAME"), strBOINCProjectsGroupAlias );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strBOINCInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

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
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("GetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Initialize an EXPLICIT_ACCESS structure for all ACEs.
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));

    // boinc_projects
    ea.grfAccessPermissions = GENERIC_READ|GENERIC_EXECUTE;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea.Trustee.ptstrName  = (LPTSTR)strBOINCProjectsGroupAlias.c_str();


    // Create a new ACL that contains the new ACEs.
    dwRes = SetEntriesInAcl(1, &ea, pOldACL, &pACL);
    if (ERROR_SUCCESS != dwRes) 
    {
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

    // Set the ACL on the DBGHELP.DLL file.
    strTemp = strBOINCInstallDirectory + _T("\\dbghelp.dll");
    dwRes = SetNamedSecurityInfo( 
        (LPWSTR)strTemp.c_str(),
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
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Set the ACL on the DBGHELP95.DLL file.
    strTemp = strBOINCInstallDirectory + _T("\\dbghelp95.dll");
    dwRes = SetNamedSecurityInfo( 
        (LPWSTR)strTemp.c_str(),
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
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Set the ACL on the SRCSRV.DLL file.
    strTemp = strBOINCInstallDirectory + _T("\\srcsrv.dll");
    dwRes = SetNamedSecurityInfo( 
        (LPWSTR)strTemp.c_str(),
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
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Set the ACL on the SYMSRV.DLL file.
    strTemp = strBOINCInstallDirectory + _T("\\symsrv.dll");
    dwRes = SetNamedSecurityInfo( 
        (LPWSTR)strTemp.c_str(),
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
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Set the ACL on the SYMSRV.YES file.
    strTemp = strBOINCInstallDirectory + _T("\\symsrv.yes");
    dwRes = SetNamedSecurityInfo( 
        (LPWSTR)strTemp.c_str(),
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
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            GetLastError(),
            _T("SetNamedSecurityInfo Error")
        );
        return ERROR_INSTALL_FAILURE;
    }

    if (pACL) 
        LocalFree(pACL);
    if (pSD)
        LocalFree(pSD);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    SetPermissionBOINCDiagnostics
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall SetPermissionBOINCDiagnostics(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CASetPermissionBOINCDiagnostics* pCA = new CASetPermissionBOINCDiagnostics(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

