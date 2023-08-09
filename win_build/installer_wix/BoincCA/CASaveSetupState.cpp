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
    tstring     strDataDirectory;
    tstring     strLaunchProgram;
    tstring     strBOINCMasterAccountUsername;
    tstring     strBOINCProjectAccountUsername;
    tstring     strEnableLaunchAtLogon;
    tstring     strEnableScreensaver;
    tstring     strEnableProtectedApplicationExecution;
    tstring     strEnableUseByAllUsers;
    tstring     strSetupStateStored;

    strSetupStateStored = _T("TRUE");

    GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    GetProperty( _T("DATADIR"), strDataDirectory );
    GetProperty( _T("LAUNCHPROGRAM"), strLaunchProgram );
    GetProperty( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    GetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );
    GetProperty( _T("ENABLELAUNCHATLOGON"), strEnableLaunchAtLogon );
    GetProperty( _T("ENABLESCREENSAVER"), strEnableScreensaver );
    GetProperty( _T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"), strEnableProtectedApplicationExecution );
    GetProperty( _T("ENABLEUSEBYALLUSERS"), strEnableUseByAllUsers );

    SetRegistryValue( _T("INSTALLDIR"), strInstallDirectory );
    SetRegistryValue( _T("DATADIR"), strDataDirectory );

    if (_T("1") == strLaunchProgram) {
        SetRegistryValue( _T("LAUNCHPROGRAM"), _T("1") );
    } else {
        SetRegistryValue( _T("LAUNCHPROGRAM"), _T("0") );
    }

    SetRegistryValue( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    SetRegistryValue( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );

    if (_T("1") == strEnableLaunchAtLogon) {
        SetRegistryValue( _T("ENABLELAUNCHATLOGON"), _T("1") );
    } else {
        SetRegistryValue( _T("ENABLELAUNCHATLOGON"), _T("0") );
    }

    if (_T("1") == strEnableScreensaver) {
        SetRegistryValue( _T("ENABLESCREENSAVER"), _T("1") );
    } else {
        SetRegistryValue( _T("ENABLESCREENSAVER"), _T("0") );
    }

    if (_T("1") == strEnableProtectedApplicationExecution) {
        SetRegistryValue( _T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"), _T("1") );
    } else {
        SetRegistryValue( _T("ENABLEPROTECTEDAPPLICATIONEXECUTION3"), _T("0") );
    }

    if (_T("1") == strEnableUseByAllUsers) {
        SetRegistryValue( _T("ENABLEUSEBYALLUSERS"), _T("1") );
    } else {
        SetRegistryValue( _T("ENABLEUSEBYALLUSERS"), _T("0") );
    }

    SetRegistryValue( _T("SETUPSTATESTORED"), strSetupStateStored );

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
