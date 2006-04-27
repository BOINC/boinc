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

// OpenSCManager()
typedef SC_HANDLE (WINAPI *tOSCM)(
    LPCSTR lpMachineName,
    LPCSTR lpDatabaseName,
    DWORD  dwDesiredAccess
);

// OpenService()
typedef SC_HANDLE (WINAPI *tOS)(
    SC_HANDLE hSCManager,
    LPCSTR    lpServiceName,
    DWORD     dwDesiredAccess
);

// ControlService()
typedef SC_HANDLE (WINAPI *tCS)(
    SC_HANDLE           hService,
    DWORD               dwControl,
    LPSERVICE_STATUS    lpServiceStatus
);

UINT CAShutdownBOINC::OnExecution()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    UINT uiReturn = ERROR_SUCCESS;
    tOSCM pOSCM = NULL;
    tOS pOS = NULL;
    tCS pCS = NULL;


    HMODULE hAdvapi32 = LoadLibrary(_T("advapi32.dll"));
    if (hAdvapi32) {
        pOSCM = (tOSCM)GetProcAddress(hAdvapi32, _T("OpenSCManagerA"));
        pOS = (tOS)GetProcAddress(hAdvapi32, _T("OpenServiceA"));
        pCS = (tCS)GetProcAddress(hAdvapi32, _T("ControlService"));
        if (!pOSCM && !pOS && !pCS) {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            pOSCM = NULL;
            pOS = NULL;
            pCS = NULL;
        }
    }

    if (pOSCM && pOS && pCS) {
        schSCManager = pOSCM( 
            NULL,                    // local machine 
            NULL,                    // ServicesActive database 
            GENERIC_READ);           // full access rights 

        if (schSCManager)
        {
            schService = pOS( 
                schSCManager,            // SCM database 
                _T("BOINC"),             // service name
                GENERIC_READ | GENERIC_EXECUTE); 
         
            if (schService) 
            {
                if (!pCS(schService, SERVICE_CONTROL_STOP, &ssStatus))
                {
                    uiReturn = ERROR_INSTALL_FAILURE;
                }
            }
        }

        if (schSCManager)
            CloseServiceHandle(schSCManager);

        if (schService)
            CloseServiceHandle(schService);
    }

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
