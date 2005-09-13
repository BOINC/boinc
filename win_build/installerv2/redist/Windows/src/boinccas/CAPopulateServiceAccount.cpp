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
#include "CAPopulateServiceAccount.h"

#define CUSTOMACTION_NAME               _T("CAPopulateServiceAccount")
#define CUSTOMACTION_PROGRESSTITLE      _T("")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAPopulateServiceAccount::CAPopulateServiceAccount(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAPopulateServiceAccount::~CAPopulateServiceAccount()
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
UINT CAPopulateServiceAccount::OnExecution()
{
    LPWKSTA_USER_INFO_1 pBuf = NULL;
    NET_API_STATUS      nStatus;
	tstring             strInitialServiceUsername;
	tstring             strInitialServiceDomain;
	tstring             strInitialAPIUsername;
	tstring             strInitialAPIDomain;
	tstring             strServiceUsername;
	tstring             strServiceDomain;
    tstring             strServiceCredentials;
    UINT                uiReturnValue = 0;


    GetProperty( _T("SERVICE_USERNAME"), strInitialServiceUsername );
    GetProperty( _T("SERVICE_DOMAIN"), strInitialServiceDomain );


    //
    // Call the NetWkstaUserGetInfo function;
    //  specify level 1.
    //
    nStatus = NetWkstaUserGetInfo(
        NULL,
        1,
        (LPBYTE *)&pBuf
        );

    if ( NERR_Success == nStatus )
    {
        strInitialAPIUsername = pBuf->wkui1_username;
        strInitialAPIDomain   = pBuf->wkui1_logon_domain;
    }
    else
    {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            nStatus,
            _T("Failed call to NetWkstaUserGetInfo")
        );
    }

    if ( strInitialServiceUsername.empty() )
    {
        if ( NERR_Success == nStatus )
        {
            strServiceUsername = strInitialAPIUsername;
        }
        else
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Unable to determine which Username to use, leaving blank")
            );
        }
    }
    else
    {
        strServiceUsername = strInitialServiceUsername;
    }


    if ( strInitialServiceDomain.empty() )
    {
        if ( NERR_Success == nStatus )
        {
            strServiceDomain = strInitialAPIDomain;
        }
        else
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Unable to determine which Domain to use, leaving blank")
            );
        }
    }
    else
    {
        strServiceDomain = strInitialServiceDomain;
    }


    if ( !strServiceUsername.empty() && !strServiceDomain.empty() )
        strServiceCredentials = strServiceDomain + _T("\\") + strServiceUsername;
    else
        strServiceCredentials = _T("<Domain>\\<Username>");


    uiReturnValue = SetProperty(_T("SERVICE_DOMAINUSERNAME"), strServiceCredentials);
    
    
    //
    // Free the allocated memory.
    //
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);


    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    PopulateServiceAccount
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall PopulateServiceAccount(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAPopulateServiceAccount* pCA = new CAPopulateServiceAccount(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

const char *BOINC_RCSID_c01e4ca76f="$Id$";
