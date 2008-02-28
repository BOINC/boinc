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
#include "CAGrantBOINCMasterRights.h"
#include "lsaprivs.h"


#define CUSTOMACTION_NAME               _T("CAGrantBOINCMasterRights")
#define CUSTOMACTION_PROGRESSTITLE      _T("Granting the BOINC service user account required privileges")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCMasterRights::CAGrantBOINCMasterRights(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCMasterRights::~CAGrantBOINCMasterRights()
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
UINT CAGrantBOINCMasterRights::OnExecution()
{
    LSA_HANDLE  PolicyHandle = NULL;
    PSID        pSid;
    NTSTATUS    Status;
    tstring     strBOINCMasterAccountUsername;
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("BOINC_MASTER_USERNAME"), strBOINCMasterAccountUsername );
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
            NULL,                                    // default lookup logic
            strBOINCMasterAccountUsername.c_str(),   // account to obtain SID
            &pSid                                    // buffer to allocate to contain resultant SID
            )
    ) 
    {

        //
        // We only grant the privilege if we succeeded in obtaining the
        // SID. We can actually add SIDs which cannot be looked up, but
        // looking up the SID is a good sanity check which is suitable for
        // most cases.

        //
        // Grant the SeCreateGlobalPrivilege to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                   // policy handle
                    pSid,                           // SID to grant privilege
                    L"SeCreateGlobalPrivilege",     // Unicode privilege
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
                _T("Failed call to SetPrivilegeOnAccount - SeCreateGlobalPrivilege")
            );
            return ERROR_INSTALL_FAILURE;
        }

        //
        // Grant the SeAssignPrimaryTokenPrivilege to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                         // policy handle
                    pSid,                                 // SID to grant privilege
                    L"SeAssignPrimaryTokenPrivilege",     // Unicode privilege
                    TRUE                                  // enable the privilege
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
                _T("Failed call to SetPrivilegeOnAccount - SeAssignPrimaryTokenPrivilege")
            );
            return ERROR_INSTALL_FAILURE;
        }

        //
        // Grant the SeIncreaseQuotaPrivilege to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                   // policy handle
                    pSid,                           // SID to grant privilege
                    L"SeIncreaseQuotaPrivilege",    // Unicode privilege
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
                _T("Failed call to SetPrivilegeOnAccount - SeIncreaseQuotaPrivilege")
            );
            return ERROR_INSTALL_FAILURE;
        }

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
        // Grant the SeDebugPrivilege to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                   // policy handle
                    pSid,                           // SID to grant privilege
                    L"SeDebugPrivilege",            // Unicode privilege
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
                _T("Failed call to SetPrivilegeOnAccount - SeDebugPrivilege")
            );
            return ERROR_INSTALL_FAILURE;
        }

        //
        // Grant the SeServiceLogonRight to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,                   // policy handle
                    pSid,                           // SID to grant privilege
                    L"SeServiceLogonRight",         // Unicode privilege
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
                _T("Failed call to SetPrivilegeOnAccount - SeServiceLogonRight")
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
// Function:    GrantBOINCMasterRights
//
// Description: This custom action reads the BOINC_MASTER_USERNAME
//              public property and grants that user the
//              required rights.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall GrantBOINCMasterRights(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAGrantBOINCMasterRights* pCA = new CAGrantBOINCMasterRights(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

const char *BOINC_RCSID_00ed9586df="$Id: CAGrantBOINCMasterRights.cpp 13804 2007-10-09 11:35:47Z fthomas $";
