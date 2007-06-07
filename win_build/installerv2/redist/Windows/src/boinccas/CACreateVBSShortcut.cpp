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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "stdafx.h"
#include "boinccas.h"
#include "CACreateVBSShortcut.h"

#define CUSTOMACTION_NAME               _T("CACreateVBSShortcut")
#define CUSTOMACTION_PROGRESSTITLE      _T("Create a start menu shortcut to the BOINC Manager.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateVBSShortcut::CACreateVBSShortcut(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateVBSShortcut::~CACreateVBSShortcut()
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
UINT CACreateVBSShortcut::OnExecution()
{
    tstring     strInstallDirectory;
    tstring     strAllUsers;
    tstring     strVBSScript;
    tstring     strEnableLaunchAtLogon;
    tstring     strRemove;
	TCHAR		szBuffer[MAX_PATH];
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ALLUSERS"), strAllUsers );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("REMOVE"), strRemove);
    if ( uiReturnValue ) return uiReturnValue;

    // Fix for Win9x systems
    if (_T("1") != strEnableLaunchAtLogon) {
        return ERROR_SUCCESS;
    }

    // Find the correct startup directory.
    if (_T("1") == strAllUsers) {
        SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    } else {
        SHGetFolderPath(NULL, CSIDL_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    }

    // Open the file for writing
    strVBSScript =  szBuffer;
    strVBSScript += _T("\\BOINC Manager.vbs");

    if (_T("ALL") == strRemove) {
        // Uninstall operation
        DeleteFile(strVBSScript.c_str());
    } else {
        // Install operation
        FILE* pScript = fopen(strVBSScript.c_str(), "w");

        // Write Script
        fprintf(
            pScript,
            _T("On Error Resume Next\n"
               "set objShell = CreateObject(\"Wscript.Shell\")\n"
               "objShell.Run \"\"\"%s\\boincmgr.exe\"\" /s\""),
            strInstallDirectory.c_str()
        );

        // Close the file
        fclose(pScript);
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CACreateVBSShortcut::OnUninstall()
{
    tstring     strAllUsers;
    tstring     strVBSScript;
    tstring     strEnableLaunchAtLogon;
	TCHAR		szBuffer[MAX_PATH];
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("ALLUSERS"), strAllUsers );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon );
    if ( uiReturnValue ) return uiReturnValue;

    // Fix for Win9x systems
    if (_T("1") != strEnableLaunchAtLogon) {
        return ERROR_SUCCESS;
    }

    // Find the correct startup directory.
    if (_T("1") == strAllUsers) {
        SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    } else {
        SHGetFolderPath(NULL, CSIDL_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    }

    // Open the file for writing
    strVBSScript =  szBuffer;
    strVBSScript += _T("\\BOINC Manager.vbs");

    // Uninstall operation
    DeleteFile(strVBSScript.c_str());

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CACreateVBSShortcut::OnRollbackUninstall()
{
    tstring     strAllUsers;
    tstring     strVBSScript;
    tstring     strEnableLaunchAtLogon;
	TCHAR		szBuffer[MAX_PATH];
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("ALLUSERS"), strAllUsers );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon );
    if ( uiReturnValue ) return uiReturnValue;

    // Fix for Win9x systems
    if (_T("1") != strEnableLaunchAtLogon) {
        return ERROR_SUCCESS;
    }

    // Find the correct startup directory.
    if (_T("1") == strAllUsers) {
        SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    } else {
        SHGetFolderPath(NULL, CSIDL_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    }

    // Open the file for writing
    strVBSScript =  szBuffer;
    strVBSScript += _T("\\BOINC Manager.vbs");

    // Uninstall operation
    DeleteFile(strVBSScript.c_str());

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CACreateVBSShortcut::OnCommitUninstall()
{
    tstring     strAllUsers;
    tstring     strVBSScript;
    tstring     strEnableLaunchAtLogon;
	TCHAR		szBuffer[MAX_PATH];
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("ALLUSERS"), strAllUsers );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon );
    if ( uiReturnValue ) return uiReturnValue;

    // Fix for Win9x systems
    if (_T("1") != strEnableLaunchAtLogon) {
        return ERROR_SUCCESS;
    }

    // Find the correct startup directory.
    if (_T("1") == strAllUsers) {
        SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    } else {
        SHGetFolderPath(NULL, CSIDL_STARTUP|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer);
    }

    // Open the file for writing
    strVBSScript =  szBuffer;
    strVBSScript += _T("\\BOINC Manager.vbs");

    // Uninstall operation
    DeleteFile(strVBSScript.c_str());

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    CreateVBSShortcut
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CreateVBSShortcut(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACreateVBSShortcut* pCA = new CACreateVBSShortcut(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

