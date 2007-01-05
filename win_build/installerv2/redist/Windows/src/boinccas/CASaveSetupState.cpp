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
#include "CASaveSetupState.h"

#define CUSTOMACTION_NAME               _T("CASaveSetupState")
#define CUSTOMACTION_PROGRESSTITLE      _T("Attempt to rename CPDNBBC installation directory to the default BOINC directory.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CASaveSetupState::CASaveSetupState(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CASaveSetupState::~CASaveSetupState()
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
UINT CASaveSetupState::OnExecution()
{
    tstring     strInstallDirectory;
    tstring     strSetupType;
    tstring     strLaunchProgram;
    tstring     strEnableLaunchAtLogon;
    tstring     strEnableScreensaver;
    tstring     strServiceDomain;
    tstring     strServiceUsername;

    GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    GetProperty( _T("SETUPTYPE"), strSetupType );
    GetProperty( _T("LAUNCHPROGRAM"), strLaunchProgram );
    GetProperty( _T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon );
    GetProperty( _T("ENABLESCREENSAVER"), strEnableScreensaver );
    GetProperty( _T("SERVICE_DOMAIN"), strServiceDomain );
    GetProperty( _T("SERVICE_USERNAME"), strServiceUsername );

    SetRegistryValue( _T("INSTALLDIR"), strInstallDirectory );
    SetRegistryValue( _T("SETUPTYPE"), strSetupType );
    SetRegistryValue( _T("LAUNCHPROGRAM"), strLaunchProgram );
    SetRegistryValue( _T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon );
    SetRegistryValue( _T("ENABLESCREENSAVER"), strEnableScreensaver );
    SetRegistryValue( _T("SERVICE_DOMAIN"), strServiceDomain );
    SetRegistryValue( _T("SERVICE_USERNAME"), strServiceUsername );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    SaveSetupState
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall SaveSetupState(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CASaveSetupState* pCA = new CASaveSetupState(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_7bca879adc="$Id$";
