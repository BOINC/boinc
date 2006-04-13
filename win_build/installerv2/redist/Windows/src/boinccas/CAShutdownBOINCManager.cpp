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
#include "CAShutdownBOINCManager.h"

#define CUSTOMACTION_NAME               _T("CAShutdownBOINCManager")
#define CUSTOMACTION_PROGRESSTITLE      _T("Shutting down running instances of BOINC Manager")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAShutdownBOINCManager::CAShutdownBOINCManager(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAShutdownBOINCManager::~CAShutdownBOINCManager()
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
UINT CAShutdownBOINCManager::OnExecution()
{
    HWND        hWndBOINCManagerSystray = NULL;
    TCHAR       szWindowTitle[256];
    LRESULT     lrReturnValue = NULL;
    UINT        uiLoopCounter = 0;
    UINT        uiReturn = -1;

    const UINT WM_TASKBARSHUTDOWN = ::RegisterWindowMessage(_T("TaskbarShutdown"));

    do
    {
        hWndBOINCManagerSystray = FindWindow( _T("wxTaskBarExWindowClass"), _T("BOINCManagerSystray") );
        if ( NULL != hWndBOINCManagerSystray )
        {
            GetWindowText( hWndBOINCManagerSystray, szWindowTitle, (sizeof(szWindowTitle) * sizeof(TCHAR)));
            LogProgress( szWindowTitle );

            lrReturnValue = SendMessage( hWndBOINCManagerSystray, WM_TASKBARSHUTDOWN, NULL, NULL );
            if ( 0 != lrReturnValue )
            {
                LogMessage(
                    INSTALLMESSAGE_ERROR,
                    NULL, 
                    NULL,
                    NULL,
                    (int)lrReturnValue,
                    _T("Setup was unable to shutdown the BOINC Manager Systray window.")
                );
                return ERROR_INSTALL_FAILURE;
            }
            Sleep(1000);
        }
        uiLoopCounter++;
    }
    while ( (NULL != hWndBOINCManagerSystray) && ( 30 >= uiLoopCounter ) );

    if ( NULL != hWndBOINCManagerSystray )
    {
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            uiLoopCounter,
            _T("One or more BOINC Manager applications could not be closed, please close them and then rerun setup.")
        );
        return ERROR_INSTALL_FAILURE;
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    ShutdownBOINCManager
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ShutdownBOINCManager(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAShutdownBOINCManager* pCA = new CAShutdownBOINCManager(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}



const char *BOINC_RCSID_7bca879ace="$Id$";
