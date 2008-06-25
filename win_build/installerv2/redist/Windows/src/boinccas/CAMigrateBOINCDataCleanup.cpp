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
#include "CAMigrateBOINCDataCleanup.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CAMigrateBOINCDataCleanup")
#define CUSTOMACTION_PROGRESSTITLE      _T("Cleanup current installer's version.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCDataCleanup::CAMigrateBOINCDataCleanup(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCDataCleanup::~CAMigrateBOINCDataCleanup()
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
UINT CAMigrateBOINCDataCleanup::OnExecution()
{
    tstring      strMigrationVersion;
    UINT         uiReturnValue = -1;

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCDataCleanup::OnExecution -- Function Begin")
    );


    uiReturnValue = SetRegistryValue( _T("MIGRATIONVERSION"), _T("") );
    if ( uiReturnValue ) return uiReturnValue;


    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCDataCleanup::OnExecution -- Function End")
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    MigrateBOINCDataCleanup
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall MigrateBOINCDataCleanup(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAMigrateBOINCDataCleanup* pCA = new CAMigrateBOINCDataCleanup(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

