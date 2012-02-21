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
#include "CAHideBOINCMasterProfile.h"
#include "lsaprivs.h"


#define CUSTOMACTION_NAME               _T("CAHideBOINCMasterProfile")
#define CUSTOMACTION_PROGRESSTITLE      _T("Hide the BOINC Master user profile.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAHideBOINCMasterProfile::CAHideBOINCMasterProfile(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAHideBOINCMasterProfile::~CAHideBOINCMasterProfile()
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
UINT CAHideBOINCMasterProfile::OnExecution()
{
    tstring     strBOINCMasterAccountUsername;
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;


    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    HideBOINCMasterProfile
//
// Description: This custom action reads the BOINC_PROJECT_USERNAME
//              public property and grants that user the
//              required rights.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall HideBOINCMasterProfile(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAHideBOINCMasterProfile* pCA = new CAHideBOINCMasterProfile(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
