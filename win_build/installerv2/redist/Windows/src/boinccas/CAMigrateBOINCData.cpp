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
#include "CAMigrateBOINCData.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CAMigrateBOINCData")
#define CUSTOMACTION_PROGRESSTITLE      _T("Move application data from the old software location to the new location.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCData::CAMigrateBOINCData(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCData::~CAMigrateBOINCData()
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
UINT CAMigrateBOINCData::OnExecution()
{
    tstring     strInstallDirectory;
    tstring     strDataDirectory;
    struct stat buf;
    UINT        uiReturnValue = -1;

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    if ( stat(strDataDirectory.c_str(), &buf) )
    {
        if ( MoveFolder(strInstallDirectory, strDataDirectory ) )
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("BOINC files have been migrated to the data directory.")
            );
        }
        else
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                GetLastError(),
                _T("MoveFolder failed.")
            );
        }
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    MigrateBOINCData
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall MigrateBOINCData(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAMigrateBOINCData* pCA = new CAMigrateBOINCData(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_8dca879ada="$Id: CAMigrateBOINCData.cpp 11773 2007-01-05 08:49:02Z rwalton $";
