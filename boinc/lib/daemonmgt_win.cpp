// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif

/**
 * Find out if BOINC has been installed as a service.
 **/
bool is_daemon_installed()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    bool bRetVal = false;

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
            bRetVal = true;
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
bool is_daemon_starting()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    bool bRetVal = false;

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
                    bRetVal = true;
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
bool is_daemon_running()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    bool bRetVal = false;

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
                    bRetVal = true;
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
bool is_daemon_stopping()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    bool bRetVal = false;

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
                    bRetVal = true;
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
bool is_daemon_stopped()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    bool bRetVal = false;

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
                    bRetVal = true;
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
 * Start the BOINC Service via the BOINC Service Control utility.
 **/
bool start_daemon_via_daemonctrl()
{
    bool                bRetVal = false;
    BOOL                bProcessStarted;
    SHELLEXECUTEINFO    shex;
    TCHAR               szPath[MAX_PATH+1];
    TCHAR               szExe[MAX_PATH+1];
    unsigned long       ulExitCode;

    // Determine the path to the BOINC Service Control utility
    //   by finding out the path to the executable that requested
    //   that we start the service.
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));
		
    TCHAR *pszProg = _tcsrchr(szPath, '\\');
    if (pszProg) {
        szPath[pszProg - szPath] = 0;
    }

    // The executable needs to contain the path.
    _sntprintf(
        szExe, (sizeof(szPath)/sizeof(TCHAR)),
        _T("\"%s\\boincsvcctrl.exe\""),
        szPath
    );

    memset( &shex, 0, sizeof( shex) );

    shex.cbSize        = sizeof( SHELLEXECUTEINFO );
    shex.fMask         = SEE_MASK_NOCLOSEPROCESS;
    shex.hwnd          = NULL;
    if ((LOBYTE(LOWORD(GetVersion()))) >= 6) {
        shex.lpVerb        = _T("runas");
    }
    shex.lpFile        = (LPCTSTR)&szExe;
    shex.lpParameters  = _T("--start");
    shex.lpDirectory   = (LPCTSTR)&szPath;
    shex.nShow         = SW_HIDE;

    bProcessStarted = ShellExecuteEx( &shex );

    if (bProcessStarted) {
        WaitForSingleObject(shex.hProcess, INFINITE);
        if (GetExitCodeProcess(shex.hProcess, &ulExitCode)) {
            if (ulExitCode == 0) {
                bRetVal = true;
            }
        }
    }

    return bRetVal;
}


/**
 * Start the BOINC Service.
 **/
bool start_daemon()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    bool bRetVal = false;

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
                bRetVal = true;
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
 * Stop the BOINC Service via the BOINC Service Control utility.
 **/
bool stop_daemon_via_daemonctrl()
{
    bool                bRetVal = false;
    BOOL                bProcessStarted;
    SHELLEXECUTEINFO    shex;
    TCHAR               szPath[MAX_PATH+1];
    TCHAR               szExe[MAX_PATH+1];
    unsigned long       ulExitCode;

    // Determine the path to the BOINC Service Control utility
    //   by finding out the path to the executable that requested
    //   that we start the service.
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));
		
    TCHAR *pszProg = _tcsrchr(szPath, '\\');
    if (pszProg) {
        szPath[pszProg - szPath] = 0;
    }

    // The executable needs to contain the path.
    _sntprintf(
        szExe, (sizeof(szPath)/sizeof(TCHAR)),
        _T("\"%s\\boincsvcctrl.exe\""),
        szPath
    );

    memset( &shex, 0, sizeof( shex) );

    shex.cbSize        = sizeof( SHELLEXECUTEINFO );
    shex.fMask         = SEE_MASK_NOCLOSEPROCESS;
    shex.hwnd          = NULL;
    if ((LOBYTE(LOWORD(GetVersion()))) >= 6) {
        shex.lpVerb        = _T("runas");
    }
    shex.lpFile        = (LPCTSTR)&szExe;
    shex.lpParameters  = _T("--stop");
    shex.lpDirectory   = (LPCTSTR)&szPath;
    shex.nShow         = SW_HIDE;

    bProcessStarted = ShellExecuteEx( &shex );

    if (bProcessStarted) {
        WaitForSingleObject(shex.hProcess, INFINITE);
        if (GetExitCodeProcess(shex.hProcess, &ulExitCode)) {
            if (ulExitCode == 0) {
                bRetVal = true;
            }
        }
    }

    return bRetVal;
}


/**
 * Stop the BOINC Service.
 **/
bool stop_daemon()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    bool bRetVal = false;

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
                bRetVal = true;
            }
        }
    }

    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (schService)
        CloseServiceHandle(schService);

    return bRetVal;
}

