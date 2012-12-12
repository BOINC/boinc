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
#include "CAMigrateBOINCDataVersion.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CAMigrateBOINCDataVersion")
#define CUSTOMACTION_PROGRESSTITLE      _T("Store current installer's version.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCDataVersion::CAMigrateBOINCDataVersion(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCDataVersion::~CAMigrateBOINCDataVersion()
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
UINT CAMigrateBOINCDataVersion::OnExecution()
{
    tstring      strMigrationVersion;
    UINT         uiReturnValue = -1;

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCDataVersion::OnExecution -- Function Begin")
    );


    uiReturnValue = GetProperty( _T("ProductVersion"), strMigrationVersion );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = SetRegistryValue( _T("MIGRATIONVERSION"), strMigrationVersion );
    if ( uiReturnValue ) return uiReturnValue;


    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCDataVersion::OnExecution -- Function End")
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    MigrateBOINCDataVersion
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall MigrateBOINCDataVersion(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAMigrateBOINCDataVersion* pCA = new CAMigrateBOINCDataVersion(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

