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
#include "CAMigrateCPDNBBC.h"

#define CUSTOMACTION_NAME               _T("CAMigrateCPDNBBC")
#define CUSTOMACTION_PROGRESSTITLE      _T("Attempt to rename CPDNBBC installation directory to the default BOINC directory.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateCPDNBBC::CAMigrateCPDNBBC(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateCPDNBBC::~CAMigrateCPDNBBC()
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


UINT CAMigrateCPDNBBC::OnExecution()
{
    tstring     strInstallDirectory;
    UINT        uiReturnValue = -1;
    BOOL        bReturnValue = FALSE;

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    bReturnValue = MoveFileEx(
        _T("C:\\Program Files\\Climate Change Experiment"),
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
            _T("Climate Change Experiment files have been migrated to the installation directory.")
        );
    }
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    MigrateCPDNBBC
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall MigrateCPDNBBC(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAMigrateCPDNBBC* pCA = new CAMigrateCPDNBBC(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_7bca879ada="$Id$";
