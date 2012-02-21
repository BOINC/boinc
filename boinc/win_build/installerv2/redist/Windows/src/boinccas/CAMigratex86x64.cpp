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
#include "CAMigratex86x64.h"

#define CUSTOMACTION_NAME               _T("CAMigratex86x64")
#define CUSTOMACTION_PROGRESSTITLE      _T("Move application data from the 32-bit software location to the 64-bit software location.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateX86X64::CAMigrateX86X64(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateX86X64::~CAMigrateX86X64()
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
UINT CAMigrateX86X64::OnExecution()
{
    typedef BOOL (WINAPI *tMFE)( LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags );

    tstring     strInstallDirectory;
    UINT        uiReturnValue = -1;
    BOOL        bReturnValue = FALSE;
    HMODULE     hKernel32Lib = NULL;
    tMFE        pMFE = NULL;


    // Dynamically link to the proper function pointers.
    hKernel32Lib = GetModuleHandle("kernel32.dll");
#ifdef _UNICODE
    pMFE = (tMFE) GetProcAddress( hKernel32Lib, "MoveFileExW" );
#else
    pMFE = (tMFE) GetProcAddress( hKernel32Lib, "MoveFileExA" );
#endif

    if (pMFE) {
        uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
        if ( uiReturnValue ) return uiReturnValue;

        bReturnValue = pMFE(
            _T("C:\\Program Files (x86)\\BOINC"),
            strInstallDirectory.c_str(),
            MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH 
        );
        if ( bReturnValue )
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("BOINC files have been migrated to the 64-bit installation directory.")
            );
        }
    }
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    MigrateX86X64
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall MigrateX86X64(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAMigrateX86X64* pCA = new CAMigrateX86X64(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_7dca879ada="$Id: CAMigrateX86X64.cpp 11773 2007-01-05 08:49:02Z rwalton $";
