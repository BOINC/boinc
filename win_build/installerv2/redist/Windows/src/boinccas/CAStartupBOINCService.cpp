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
#include "CAStartupBOINCService.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CAStartupBOINCService")
#define CUSTOMACTION_PROGRESSTITLE      _T("Startup the BOINC System Service")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAStartupBOINCService::CAStartupBOINCService(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAStartupBOINCService::~CAStartupBOINCService()
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
UINT CAStartupBOINCService::OnExecution()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    UINT      uiReturnValue = ERROR_INSTALL_FAILURE;

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
            if (StartService(schService, 0, NULL))
            {
                uiReturnValue = ERROR_SUCCESS;
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("Setup was able to startup the BOINC System Service.")
                );
            }
            else
            {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    GetLastError(),
                    _T("StartService failed.")
                );
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL, 
                    NULL,
                    NULL,
                    GetLastError(),
                    _T("Setup was unable to shutdown the BOINC System Service.")
                );
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    StartupBOINCService
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall StartupBOINCService(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAStartupBOINCService* pCA = new CAStartupBOINCService(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_8dcb889ada="$Id: CAStartupBOINCService.cpp 11773 2007-01-05 08:49:02Z rwalton $";
