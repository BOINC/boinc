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
#include "CAGrantBOINCProjectsVirtualBoxRights.h"
#include "dcomperm.h"


#define CUSTOMACTION_NAME               _T("CAGrantBOINCProjectsVirtualBoxRights")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating the BOINC Projects VirtualBox access rights")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCProjectsVirtualBoxRights::CAGrantBOINCProjectsVirtualBoxRights(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCProjectsVirtualBoxRights::~CAGrantBOINCProjectsVirtualBoxRights()
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
UINT CAGrantBOINCProjectsVirtualBoxRights::OnExecution()
{
    ChangeAppIDAccessACL(
        _T("{819B4D85-9CEE-493C-B6FC-64FFE759B3C9}"),
        _T("boinc_projects"),
        TRUE,
        TRUE
    );

    ChangeAppIDLaunchACL(
        _T("{819B4D85-9CEE-493C-B6FC-64FFE759B3C9}"),
        _T("boinc_projects"),
        TRUE,
        TRUE
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    GrantBOINCProjectsVirtualBoxRights
//
// Description: This custom action grants the 'boinc_projects' group the
//              required rights to launch and access VirtualBox.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall GrantBOINCProjectsVirtualBoxRights(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAGrantBOINCProjectsVirtualBoxRights* pCA = new CAGrantBOINCProjectsVirtualBoxRights(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
