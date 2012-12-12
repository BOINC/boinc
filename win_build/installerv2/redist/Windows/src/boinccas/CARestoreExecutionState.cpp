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
#include "CARestoreExecutionState.h"

#define CUSTOMACTION_NAME               _T("CARestoreExecutionState")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CARestoreExecutionState::CARestoreExecutionState(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CARestoreExecutionState::~CARestoreExecutionState()
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
UINT CARestoreExecutionState::OnExecution()
{
    tstring     strLaunchProgram;
    tstring     strReturnRebootRequested;
    tstring     strReturnValidateInstall;
    tstring     strRebootPrompt;

    GetRegistryValue( _T("LAUNCHPROGRAM"), strLaunchProgram );
    GetRegistryValue( _T("RETURN_REBOOTREQUESTED"), strReturnRebootRequested );
    GetRegistryValue( _T("REBOOTPROMPT"), strRebootPrompt );
    GetRegistryValue( _T("RETURN_VALIDATEINSTALL"), strReturnValidateInstall );

    SetProperty( _T("LAUNCHPROGRAM"), strLaunchProgram );
    SetProperty( _T("RETURN_REBOOTREQUESTED"), strReturnRebootRequested );
    SetProperty( _T("REBOOTPROMPT"), strRebootPrompt );
    SetProperty( _T("RETURN_VALIDATEINSTALL"), strReturnValidateInstall );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    RestoreExecutionState
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall RestoreExecutionState(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CARestoreExecutionState* pCA = new CARestoreExecutionState(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

