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
#include "CAHideBOINCProjectProfile.h"
#include "lsaprivs.h"


#define CUSTOMACTION_NAME               _T("CAHideBOINCProjectProfile")
#define CUSTOMACTION_PROGRESSTITLE      _T("Hide the BOINC Profile user profile.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAHideBOINCProjectProfile::CAHideBOINCProjectProfile(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAHideBOINCProjectProfile::~CAHideBOINCProjectProfile()
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
UINT CAHideBOINCProjectProfile::OnExecution()
{
    tstring     strBOINCProjectAccountUsername;
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;


    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    HideBOINCProjectProfile
//
// Description: This custom action reads the BOINC_PROJECT_USERNAME
//              public property and grants that user the
//              required rights.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall HideBOINCProjectProfile(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAHideBOINCProjectProfile* pCA = new CAHideBOINCProjectProfile(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

