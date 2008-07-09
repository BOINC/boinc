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
#include "CAVerifyInstallDirectories.h"

#define CUSTOMACTION_NAME               _T("CAVerifyInstallDirectories")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyInstallDirectories::CAVerifyInstallDirectories(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyInstallDirectories::~CAVerifyInstallDirectories()
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
UINT CAVerifyInstallDirectories::OnExecution()
{
    tstring strInstallDirectory;
    tstring strDataDirectory;
    tstring strWindowsDirectory;
    tstring strWindowsSystemDirectory;
    tstring strProgramFilesDirectory;
    tstring strSystemDrive;
    tstring strMessage;
    TCHAR   szBuffer[2048];
    UINT    uiReturnValue = 0;


    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;


    // Determine the Windows directory
    if (GetWindowsDirectory(szBuffer, sizeof(szBuffer)/sizeof(TCHAR)))
    {
        strWindowsDirectory = tstring(szBuffer) + _T("\\");

        strMessage = _T("strWindowsDirectory: '");
        strMessage += strWindowsDirectory;
        strMessage += _T("'");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            strMessage.c_str()
        );
    }

    // Determine the system drive
    if (!strWindowsDirectory.empty())
    {
        TCHAR drive[_MAX_DRIVE];
        TCHAR dir[_MAX_DIR];
        TCHAR fname[_MAX_FNAME];
        TCHAR ext[_MAX_EXT];

        _tsplitpath(strWindowsDirectory.c_str(), drive, dir, fname, ext);

        strSystemDrive = tstring(drive) + _T("\\");       

        strMessage = _T("strSystemDrive: '");
        strMessage += strSystemDrive;
        strMessage += _T("'");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            strMessage.c_str()
        );
    }

    // Determine the Windows System directory.
    if (GetSystemDirectory(szBuffer, sizeof(szBuffer)/sizeof(TCHAR)))
    {
        strWindowsSystemDirectory = tstring(szBuffer) + _T("\\");

        strMessage = _T("strWindowsSystemDirectory: '");
        strMessage += strWindowsSystemDirectory;
        strMessage += _T("'");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            strMessage.c_str()
        );
    }

    // Determine the Program Files directory.
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, szBuffer)))
    {
        strProgramFilesDirectory = tstring(szBuffer) + _T("\\");

        strMessage = _T("strProgramFilesDirectory: '");
        strMessage += strProgramFilesDirectory;
        strMessage += _T("'");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            strMessage.c_str()
        );
    }


    // Perform some basic sanity tests to see if we need to migrate
    //   anything.
    BOOL bInstallDataSameDirectory = 
        (BOOL)(strInstallDirectory == strDataDirectory);
    BOOL bInstallDirWindowsDirSame = 
        (BOOL)(strInstallDirectory == strWindowsDirectory);
    BOOL bDataDirWindowsDirSame = 
        (BOOL)(strDataDirectory == strWindowsDirectory);
    BOOL bInstallDirSystemDriveSame = 
        (BOOL)(strInstallDirectory == strSystemDrive);
    BOOL bDataDirSystemDriveSame = 
        (BOOL)(strDataDirectory == strSystemDrive);
    BOOL bInstallDirWindowsSystemDirSame = 
        (BOOL)(strInstallDirectory == strWindowsSystemDirectory);
    BOOL bDataDirWindowsSystemDirSame = 
        (BOOL)(strDataDirectory == strWindowsSystemDirectory);
    BOOL bInstallDirProgramFilesDirSame = 
        (BOOL)(strInstallDirectory == strProgramFilesDirectory);
    BOOL bDataDirProgramFilesDirSame = 
        (BOOL)(strDataDirectory == strProgramFilesDirectory);


    if ( bInstallDataSameDirectory )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("The installation directory and data directory must be different directories. Please select a different data directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bInstallDirWindowsDirSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Install directory is the same as the Windows directory. Please select a different install directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bDataDirWindowsDirSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Data directory is the same as the Windows directory. Please select a different data directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bInstallDirSystemDriveSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Install directory is the same as the system drive. Please select a different install directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bDataDirSystemDriveSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Data directory is the same as the system drive. Please select a different data directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bInstallDirWindowsSystemDirSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Install directory is the same as the Windows system directory. Please select a different install directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bDataDirWindowsSystemDirSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Data directory is the same as the Windows system directory. Please select a different data directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bInstallDirProgramFilesDirSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Install directory is the same as the program files directory. Please select a different install directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else if ( bDataDirProgramFilesDirSame )
    {
        DisplayMessage(
            MB_OK, 
            MB_ICONERROR,
            _T("Data directory is the same as the program files directory. Please select a different data directory.")
        );

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
        uiReturnValue = ERROR_INSTALL_USEREXIT;
    }
    else
    {
        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("1"));
        uiReturnValue = ERROR_SUCCESS;
    }

    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    VerifyInstallDirectories
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall VerifyInstallDirectories(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAVerifyInstallDirectories* pCA = new CAVerifyInstallDirectories(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

