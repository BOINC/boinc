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
#include "launcher.h"
#include "CALaunchBOINCTray.h"

#define CUSTOMACTION_NAME               _T("CALaunchBOINCTray")
#define CUSTOMACTION_PROGRESSTITLE      _T("Launching BOINC Tray")

/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CALaunchBOINCTray::CALaunchBOINCTray(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CALaunchBOINCTray::~CALaunchBOINCTray()
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

UINT CALaunchBOINCTray::OnExecution()
{
    tstring strInstallDirectory;
    tstring strBuffer;
    UINT uiReturnValue;

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;


    strBuffer = tstring(_T("\"")) + strInstallDirectory + tstring(_T("boinctray.exe\""));
    CreateProcessWithExplorerIL( NULL, (LPWSTR)strBuffer.c_str() );


    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    LaunchBOINCTray
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall LaunchBOINCTray(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CALaunchBOINCTray* pCA = new CALaunchBOINCTray(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

