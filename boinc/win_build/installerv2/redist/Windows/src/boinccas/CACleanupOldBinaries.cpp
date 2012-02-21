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
#include "CACleanupOldBinaries.h"

#define CUSTOMACTION_NAME               _T("CACleanupOldBinaries")
#define CUSTOMACTION_PROGRESSTITLE      _T("Cleanup any old binaries that were left lying around from some other install.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACleanupOldBinaries::CACleanupOldBinaries(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACleanupOldBinaries::~CACleanupOldBinaries()
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
UINT CACleanupOldBinaries::OnExecution()
{
    tstring     strInstallDirectory;
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    DeleteFile(tstring(strInstallDirectory + _T("\\boinc.exe")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\boincmgr.exe")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\boinccmd.exe")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\boinc.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\libcurl.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\libeay32.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\ssleay32.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\zlib1.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\dbghelp.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\dbghelp95.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\srcsrv.dll")).c_str());
    DeleteFile(tstring(strInstallDirectory + _T("\\symsrv.dll")).c_str());

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    CleanupOldBinaries
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CleanupOldBinaries(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACleanupOldBinaries* pCA = new CACleanupOldBinaries(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_7bcd879ada="$Id: CAMigrateX86X64.cpp 11773 2007-01-05 08:49:02Z rwalton $";
