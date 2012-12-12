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
#include "CAGrantBOINCAdminsVirtualBoxRights.h"
#include "dcomperm.h"


#define CUSTOMACTION_NAME               _T("CAGrantBOINCAdminsVirtualBoxRights")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating the BOINC Administrators VirtualBox access rights")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCAdminsVirtualBoxRights::CAGrantBOINCAdminsVirtualBoxRights(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCAdminsVirtualBoxRights::~CAGrantBOINCAdminsVirtualBoxRights()
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
UINT CAGrantBOINCAdminsVirtualBoxRights::OnExecution()
{
    ChangeAppIDAccessACL(
        _T("{819B4D85-9CEE-493C-B6FC-64FFE759B3C9}"),
        _T("boinc_admins"),
        TRUE,
        TRUE
    );

    ChangeAppIDLaunchACL(
        _T("{819B4D85-9CEE-493C-B6FC-64FFE759B3C9}"),
        _T("boinc_admins"),
        TRUE,
        TRUE
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    GrantBOINCAdminsVirtualBoxRights
//
// Description: This custom action grants the 'boinc_admins' group the
//              required rights to launch and access VirtualBox.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall GrantBOINCAdminsVirtualBoxRights(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAGrantBOINCAdminsVirtualBoxRights* pCA = new CAGrantBOINCAdminsVirtualBoxRights(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
