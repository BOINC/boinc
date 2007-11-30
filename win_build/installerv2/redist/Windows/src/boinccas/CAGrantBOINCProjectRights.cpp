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
#include "CAGrantBOINCProjectRights.h"
#include "lsaprivs.h"


#define CUSTOMACTION_NAME               _T("CAGrantBOINCProjectRights")
#define CUSTOMACTION_PROGRESSTITLE      _T("Granting the BOINC Project user account required privileges")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCProjectRights::CAGrantBOINCProjectRights(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCProjectRights::~CAGrantBOINCProjectRights()
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
UINT CAGrantBOINCProjectRights::OnExecution()
{
    LSA_HANDLE  PolicyHandle = NULL;
    PSID        pSid;
    NTSTATUS    Status;
    tstring     strBOINCProjectAccountUsername;
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("BOINC_PROJECT_USERNAME"), strBOINCProjectAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;


    //
    // Open the policy on the local host.
    //
    Status = OpenPolicy(
                _T(""),
                POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
                &PolicyHandle
                );


    if(Status != STATUS_SUCCESS) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            Status,
            _T("OpenPolicy failed.")
        );
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("Failed to be able to open a policy to the localhost")
        );
        return ERROR_INSTALL_FAILURE;
    }


    //
    // Obtain the SID of the user/group.
    // Note that we could target a specific machine, but we don't.
    // Specifying NULL for target machine searches for the SID in the
    // following order: well-known, Built-in and local, primary domain,
    // trusted domains.
    //
    if(
        GetAccountSid(
            NULL,                                     // default lookup logic
            strBOINCProjectAccountUsername.c_str(),   // account to obtain SID
            &pSid                                     // buffer to allocate to contain resultant SID
            )
    ) 
    {

        //
        // We only grant the privilege if we succeeded in obtaining the
        // SID. We can actually add SIDs which cannot be looked up, but
        // looking up the SID is a good sanity check which is suitable for
        // most cases.

        //
        // Grant the SeDenyInteractiveLogonRight to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                   // policy handle
                    pSid,                           // SID to grant privilege
                    L"SeDenyInteractiveLogonRight", // Unicode privilege
                    TRUE                            // enable the privilege
                    );

        if(Status != STATUS_SUCCESS)
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                Status,
                _T("SetPrivilegeOnAccount failed.")
            );
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed call to SetPrivilegeOnAccount - SeDenyInteractiveLogonRight")
            );
            return ERROR_INSTALL_FAILURE;
        }

        //
        // Grant the SeDenyNetworkLogonRight to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                   // policy handle
                    pSid,                           // SID to grant privilege
                    L"SeDenyNetworkLogonRight",     // Unicode privilege
                    TRUE                            // enable the privilege
                    );

        if(Status != STATUS_SUCCESS)
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                Status,
                _T("SetPrivilegeOnAccount failed.")
            );
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed call to SetPrivilegeOnAccount - SeDenyNetworkLogonRight")
            );
            return ERROR_INSTALL_FAILURE;
        }

        //
        // Grant the SeChangeNotifyPrivilege to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                   // policy handle
                    pSid,                           // SID to grant privilege
                    L"SeChangeNotifyPrivilege",     // Unicode privilege
                    TRUE                            // enable the privilege
                    );

        if(Status != STATUS_SUCCESS)
        {
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                Status,
                _T("SetPrivilegeOnAccount failed.")
            );
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed call to SetPrivilegeOnAccount - SeChangeNotifyPrivilege")
            );
            return ERROR_INSTALL_FAILURE;
        }
    }
    else
    {
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("Failed to be able to obtain the SID for the selected user on the localhost")
        );
        return ERROR_INSTALL_FAILURE;
    }


    //
    // Cleanup any handles and memory allocated during the custom action
    //
    LsaClose(PolicyHandle);
    if(pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    GrantBOINCProjectRights
//
// Description: This custom action reads the BOINC_PROJECT_USERNAME
//              public property and grants that user the
//              required rights.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall GrantBOINCProjectRights(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAGrantBOINCProjectRights* pCA = new CAGrantBOINCProjectRights(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

const char *BOINC_RCSID_00ed9696df="$Id: CAGrantBOINCProjectRights.cpp 13804 2007-10-09 11:35:47Z fthomas $";
