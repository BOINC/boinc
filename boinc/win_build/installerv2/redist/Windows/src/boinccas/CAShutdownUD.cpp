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
#include "terminate.h"
#include "CAShutdownUD.h"

#define CUSTOMACTION_NAME               _T("CAShutdownUD")
#define CUSTOMACTION_PROGRESSTITLE      _T("Shutting down running instances of United Devices agent")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAShutdownUD::CAShutdownUD(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAShutdownUD::~CAShutdownUD()
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

UINT CAShutdownUD::OnExecution()
{
    TerminateProcessEx( tstring(_T("ud.exe")) );
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    ShutdownUD
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ShutdownUD(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAShutdownUD* pCA = new CAShutdownUD(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_7bcb879acf="$Id: CAShutdownUD.cpp 12091 2007-02-15 07:41:13Z rwalton $";
