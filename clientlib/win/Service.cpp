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


#include "stdafx.h"

/**
 * Find out if BOINC has been installed as a service.
 **/
EXTERN_C __declspec(dllexport) BOOL IsBOINCServiceInstalled()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    BOOL bRetVal = FALSE;

    schSCManager = OpenSCManager( 
        NULL,                    // local machine 
        NULL,                    // ServicesActive database 
        GENERIC_READ);           // full access rights 

    if (schSCManager)
    {
        schService = OpenService( 
            schSCManager,            // SCM database 
            _T("BOINC"),             // service name
            GENERIC_READ); 
     
        if (schService) 
        {
            bRetVal = TRUE;
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}


/**
 * Find out if BOINC has been told to start.
 **/
EXTERN_C __declspec(dllexport) BOOL IsBOINCServiceStarting()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    BOOL bRetVal = FALSE;

    schSCManager = OpenSCManager( 
        NULL,                    // local machine 
        NULL,                    // ServicesActive database 
        GENERIC_READ);           // full access rights 

    if (schSCManager)
    {
        schService = OpenService( 
            schSCManager,            // SCM database 
            _T("BOINC"),             // service name
            GENERIC_READ); 
     
        if (schService) 
        {
            if (QueryServiceStatus(schService, &ssStatus))
            {
                if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
                    bRetVal = TRUE;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}


/**
 * Find out if BOINC is executing as a service.
 **/
EXTERN_C __declspec(dllexport) BOOL IsBOINCServiceRunning()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    BOOL bRetVal = FALSE;

    schSCManager = OpenSCManager( 
        NULL,                    // local machine 
        NULL,                    // ServicesActive database 
        GENERIC_READ);           // full access rights 

    if (schSCManager)
    {
        schService = OpenService( 
            schSCManager,            // SCM database 
            _T("BOINC"),             // service name
            GENERIC_READ); 
     
        if (schService) 
        {
            if (QueryServiceStatus(schService, &ssStatus))
            {
                if (ssStatus.dwCurrentState == SERVICE_RUNNING)
                    bRetVal = TRUE;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}


/**
 * Find out if BOINC has been told to stop.
 **/
EXTERN_C __declspec(dllexport) BOOL IsBOINCServiceStopping()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    BOOL bRetVal = FALSE;

    schSCManager = OpenSCManager( 
        NULL,                    // local machine 
        NULL,                    // ServicesActive database 
        GENERIC_READ);           // full access rights 

    if (schSCManager)
    {
        schService = OpenService( 
            schSCManager,            // SCM database 
            _T("BOINC"),             // service name
            GENERIC_READ); 
     
        if (schService) 
        {
            if (QueryServiceStatus(schService, &ssStatus))
            {
                if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
                    bRetVal = TRUE;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}


/**
 * Find out if BOINC has stopped executing as a service.
 **/
EXTERN_C __declspec(dllexport) BOOL IsBOINCServiceStopped()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    BOOL bRetVal = FALSE;

    schSCManager = OpenSCManager( 
        NULL,                    // local machine 
        NULL,                    // ServicesActive database 
        GENERIC_READ);           // full access rights 

    if (schSCManager)
    {
        schService = OpenService( 
            schSCManager,            // SCM database 
            _T("BOINC"),             // service name
            GENERIC_READ); 
     
        if (schService) 
        {
            if (QueryServiceStatus(schService, &ssStatus))
            {
                if (ssStatus.dwCurrentState == SERVICE_STOPPED)
                    bRetVal = TRUE;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}


/**
 * Start the BOINC Service.
 **/
EXTERN_C __declspec(dllexport) BOOL StartBOINCService()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    BOOL bRetVal = FALSE;

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
                bRetVal = TRUE;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}


/**
 * Stop the BOINC Service.
 **/
EXTERN_C __declspec(dllexport) BOOL StopBOINCService()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    BOOL bRetVal = FALSE;

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
            if (ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus))
            {
                bRetVal = TRUE;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}
