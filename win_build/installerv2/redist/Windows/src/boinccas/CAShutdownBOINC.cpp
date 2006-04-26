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
#include "CAShutdownBOINC.h"

#define CUSTOMACTION_NAME               _T("CAShutdownBOINC")
#define CUSTOMACTION_PROGRESSTITLE      _T("Shutting down running instances of BOINC")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAShutdownBOINC::CAShutdownBOINC(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAShutdownBOINC::~CAShutdownBOINC()
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
UINT CAShutdownBOINC::OnExecution()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    UINT uiReturn = ERROR_SUCCESS;

    schSCManager = OpenSCManager( 
        NULL,                    // local machine 
        NULL,                    // ServicesActive database 
        GENERIC_READ);           // full access rights 

    if (schSCManager)
    {
        schService = OpenService( 
            schSCManager,            // SCM database 
            _T("BOINC"),             // service name
            GENERIC_READ | GENERIC_EXECUTE); 
     
        if (schService) 
        {
            if (!ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus))
            {
                uiReturn = ERROR_INSTALL_FAILURE;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return uiReturn;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    ShutdownBOINCManager
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ShutdownBOINC(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAShutdownBOINC* pCA = new CAShutdownBOINC(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_7bca879acf="$Id$";
