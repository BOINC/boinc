// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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
#include "CADeleteBOINCGroups.h"
#include "lsaprivs.h"


#define CUSTOMACTION_NAME               _T("CADeleteBOINCGroups")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating user groups used by BOINC for secure sandboxes")


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CADeleteBOINCGroups::CADeleteBOINCGroups(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CADeleteBOINCGroups::~CADeleteBOINCGroups()
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
UINT CADeleteBOINCGroups::OnExecution()
{
    NET_API_STATUS   nasReturnValue;


    if (IsUpgrading())
    {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("Upgrade detected, no need to delete groups")
        );
        return ERROR_SUCCESS;
    }


    // Delete the 'boinc_admins'
    //
    nasReturnValue = NetLocalGroupDel(
        NULL,
        _T("boinc_admins")
    );

    if ((NERR_Success != nasReturnValue) && (ERROR_ALIAS_EXISTS != nasReturnValue)) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("NetLocalGroupDel retval")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("Failed to delete the 'boinc_admins' group.")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Delete the 'boinc_users'
    //
    nasReturnValue = NetLocalGroupDel(
        NULL,
        _T("boinc_users")
    );

    if ((NERR_Success != nasReturnValue) && (ERROR_ALIAS_EXISTS != nasReturnValue)) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("NetLocalGroupDel retval")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("Failed to Delete the 'boinc_users' group.")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Delete the 'boinc_projects'
    //
    nasReturnValue = NetLocalGroupDel(
        NULL,
        _T("boinc_projects")
    );

    if ((NERR_Success != nasReturnValue) && (ERROR_ALIAS_EXISTS != nasReturnValue)) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("NetLocalGroupDel retval")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL,
            NULL,
            NULL,
            nasReturnValue,
            _T("Failed to remove the 'boinc_projects' group.")
        );
        return ERROR_INSTALL_FAILURE;
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    DeleteBOINCGroups
//
// Description: This custom action deletes the three user groups that
//              are used to enforce the account based sandboxing scheme
//              on Windows.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall DeleteBOINCGroups(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CADeleteBOINCGroups* pCA = new CADeleteBOINCGroups(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
