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
#include "CAValidateServiceAccount.h"

#define CUSTOMACTION_NAME               _T("CAValidateServiceAccount")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating service account parameters")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateServiceAccount::CAValidateServiceAccount(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAValidateServiceAccount::~CAValidateServiceAccount()
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
UINT CAValidateServiceAccount::OnExecution()
{
	tstring strInitialServiceUsername;
	tstring strInitialServiceDomain;
	tstring strInitialServicePassword;
	tstring strInitialServiceDomainUsername;
    tstring strSetupType;
    UINT    uiReturnValue = 0;


    static const std::basic_string <char>::size_type npos = -1;


    uiReturnValue = GetProperty( _T("SERVICE_USERNAME"), strInitialServiceUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("SERVICE_DOMAIN"), strInitialServiceDomain );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("SERVICE_PASSWORD"), strInitialServicePassword, false );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("SERVICE_DOMAINUSERNAME"), strInitialServiceDomainUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("SETUPTYPE"), strSetupType );
    if ( uiReturnValue ) return uiReturnValue;


    if ( _T("ServiceGUI") != strSetupType )
    {
        if ( strInitialServicePassword.empty() )
        {
            LogMessage(
                INSTALLMESSAGE_FATALEXIT,
                MB_OK, 
                MB_ICONERROR,
                NULL,
                NULL,
                _T("Setup has detected an invalid condition, SERVICE_PASSWORD cannot be empty or null.")
            );
            return ERROR_INSTALL_FAILURE;
        }
    }
    else
    {
        if ( strInitialServicePassword.empty() )
        {
            if ( ( npos == strInitialServiceDomainUsername.find( _T("LOCALSYSTEM") ) ) &&
                 ( npos == strInitialServiceDomainUsername.find( _T("NetworkService") ) ) )
            {
                LogMessage(
                    INSTALLMESSAGE_FATALEXIT,
                    MB_OK, 
                    MB_ICONERROR,
                    NULL,
                    NULL,
                    _T("Setup has detected an invalid condition, SERVICE_PASSWORD cannot be empty or null.")
                );
                return ERROR_INSTALL_FAILURE;
            }
        }
    }


    if ( strInitialServiceDomainUsername.empty() )
    {
        if ( strInitialServiceUsername.empty() || strInitialServiceDomain.empty() )
        {
            LogMessage(
                INSTALLMESSAGE_FATALEXIT,
                MB_OK, 
                MB_ICONERROR,
                NULL,
                NULL,
                _T("Setup has detected an invalid condition, SERVICE_USERNAME or SERVICE_DOMAIN was null or empty.")
            );
            return ERROR_INSTALL_FAILURE;
        }
    }
    else
    {

        uiReturnValue = SetProperty(
            _T("SERVICE_USERNAME"),
            strInitialServiceDomainUsername.substr( (strInitialServiceDomainUsername.rfind( _T("\\") ) + 1 ), strInitialServiceDomainUsername.size() )
           );
        if ( uiReturnValue ) return uiReturnValue;

        uiReturnValue = SetProperty( 
            _T("SERVICE_DOMAIN"),
            strInitialServiceDomainUsername.substr( 0, strInitialServiceDomainUsername.find( _T("\\") ) )
            );
        if ( uiReturnValue ) return uiReturnValue;

    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    ValidateServiceAccount
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ValidateServiceAccount(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAValidateServiceAccount* pCA = new CAValidateServiceAccount(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}



