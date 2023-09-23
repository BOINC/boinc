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
#include "CAGetUsersGroupName.h"
#include "lkuprid.h"


#define CUSTOMACTION_NAME               _T("CAGetUsersGroupName")
#define CUSTOMACTION_PROGRESSTITLE      _T("Retrieving the Users group name")


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CAGetUsersGroupName::CAGetUsersGroupName(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CAGetUsersGroupName::~CAGetUsersGroupName()
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
UINT CAGetUsersGroupName::OnExecution()
{
    tstring     strGroupAlias;
    WCHAR       szName[UNLEN+1];
    DWORD       cchName = UNLEN;
    UINT        uiReturnValue;


    uiReturnValue = GetProperty( _T("GROUPALIAS_USERS"), strGroupAlias );
    if ( uiReturnValue ) return uiReturnValue;


    // If something is already defined then don't override it.
    if( strGroupAlias.empty() )
    {
        if( !LookupAliasFromRid( NULL, DOMAIN_ALIAS_RID_USERS, szName, &cchName) )
        {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL,
                NULL,
                NULL,
                GetLastError(),
                _T("Setup was unable to determine the Users group name.")
            );
            return ERROR_INSTALL_FAILURE;
        }

        uiReturnValue = SetProperty( _T("GROUPALIAS_USERS"), szName);
        if ( uiReturnValue ) return uiReturnValue;
    }


    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    GetUsersGroupName
//
// Description: This custom action looks up the users group
//                name.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall GetUsersGroupName(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAGetUsersGroupName* pCA = new CAGetUsersGroupName(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
