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

// OpenSCManager()
typedef SC_HANDLE (WINAPI *tOSCM)(
    LPCWSTR lpMachineName,
    LPCWSTR lpDatabaseName,
    DWORD   dwDesiredAccess
);

// OpenService()
typedef SC_HANDLE (WINAPI *tOS)(
    SC_HANDLE hSCManager,
    LPCWSTR   lpServiceName,
    DWORD     dwDesiredAccess
);

// ControlService()
typedef SC_HANDLE (WINAPI *tCS)(
    SC_HANDLE           hService,
    DWORD               dwControl,
    LPSERVICE_STATUS    lpServiceStatus
);

// QueryServiceStatus()
typedef BOOL (WINAPI *tQSS)(
    SC_HANDLE           hService,
    LPSERVICE_STATUS    lpServiceStatus
);

UINT CAShutdownBOINC::OnExecution()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;

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
            if (QueryServiceStatus(schService, &ssStatus))
            {
                if (!((SERVICE_STOPPED == ssStatus.dwCurrentState) &&
                      (SERVICE_STOP_PENDING == ssStatus.dwCurrentState)))
                {
                    ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus);
                }
            }
        }

        if (schSCManager)
            CloseServiceHandle(schSCManager);

        if (schService)
            CloseServiceHandle(schService);
    }

    TerminateProcessEx( tstring(_T("boinc.exe")) );
    TerminateProcessEx( tstring(_T("boinctray.exe")) );
    return ERROR_SUCCESS;
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
