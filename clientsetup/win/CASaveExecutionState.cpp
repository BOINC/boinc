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
#include "CASaveExecutionState.h"

#define CUSTOMACTION_NAME               _T("CASaveExecutionState")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CASaveExecutionState::CASaveExecutionState(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CASaveExecutionState::~CASaveExecutionState()
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
UINT CASaveExecutionState::OnExecution()
{
    tstring     strLaunchProgram;
    tstring     strReturnRebootRequested;
    tstring     strReturnValidateInstall;
    tstring     strRebootPrompt;
    tstring     strBOINCMasterAccountUsername;
    tstring     strBOINCProjectAccountUsername;

    GetProperty( _T("LAUNCHPROGRAM"), strLaunchProgram );
    GetProperty( _T("REBOOTPROMPT"), strRebootPrompt );
    GetProperty( _T("RETURN_REBOOTREQUESTED"), strReturnRebootRequested );
    GetProperty( _T("RETURN_VALIDATEINSTALL"), strReturnValidateInstall );
    GetProperty( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    GetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );

    SetRegistryValue( _T("LAUNCHPROGRAM"), strLaunchProgram );
    SetRegistryValue( _T("REBOOTPROMPT"), strRebootPrompt );
    SetRegistryValue( _T("RETURN_REBOOTREQUESTED"), strReturnRebootRequested );
    SetRegistryValue( _T("RETURN_VALIDATEINSTALL"), strReturnValidateInstall );
    SetRegistryValue( _T("RETURN_BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
    SetRegistryValue( _T("RETURN_BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );

    // Disable the ability to launch BOINC Manager if either a reboot is
    //   requested or validation of the installation executables fails.
    if ((strReturnRebootRequested == _T("1")) || (strReturnValidateInstall == _T("0")))
    {
        SetRegistryValue( _T("LAUNCHPROGRAM"), _T("") );
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    SaveExecutionState
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall SaveExecutionState(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CASaveExecutionState* pCA = new CASaveExecutionState(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

