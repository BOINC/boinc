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
#include "CAVerifyServiceExecutionRight.h"

#define CUSTOMACTION_NAME               _T("CAVerifyServiceExecutionRight")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyServiceExecutionRight::CAVerifyServiceExecutionRight(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAVerifyServiceExecutionRight::~CAVerifyServiceExecutionRight()
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
UINT CAVerifyServiceExecutionRight::OnExecution()
{
    tstring strNTVersion;
    tstring strNTProductType;
    ULONG   ulNTVersion = 0;
    ULONG   ulNTProductType = 0;
    UINT    uiReturnValue = 0;


    uiReturnValue = GetProperty( _T("VersionNT"), strNTVersion );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("MsiNTProductType"), strNTProductType );
    if ( uiReturnValue ) return uiReturnValue;


    ulNTVersion = _tstol(strNTVersion.c_str());
    ulNTProductType = _tstol(strNTProductType.c_str());


    if ( (400 == ulNTVersion) && (2 == ulNTProductType) )
    {
        // NT 4.0 Domain Controller Detected
        LogMessage(
            INSTALLMESSAGE_USER,
            MB_OK, 
            MB_ICONWARNING,
            NULL,
            NULL,
            _T("Setup has detected you are attempting to install on a Windows NT 4.0 domain controller, you'll need to manually grant the selected user the permission to 'Logon As a Service' through the User Manager for Domains.")
            );
        SetProperty(_T("SERVICE_GRANTEXECUTIONRIGHT"), _T("0"));
    }
    else
    {
        uiReturnValue = LogMessage(
            INSTALLMESSAGE_USER,
            MB_YESNO, 
            MB_ICONQUESTION,
            25000,
            NULL,
            _T("")
            );

        if ( IDYES == uiReturnValue )
        {
            SetProperty(_T("SERVICE_GRANTEXECUTIONRIGHT"), _T("1"));
        }
        else
        {
            SetProperty(_T("SERVICE_GRANTEXECUTIONRIGHT"), _T("0"));
        }
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    VerifyServiceExecutionRight
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall VerifyServiceExecutionRight(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAVerifyServiceExecutionRight* pCA = new CAVerifyServiceExecutionRight(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

