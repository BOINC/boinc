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
#include "CAGrantServiceExecutionRight.h"
#include "lsaprivs.h"


#define CUSTOMACTION_NAME               _T("CAGrantServiceExecutionRight")
#define CUSTOMACTION_PROGRESSTITLE      _T("Granting the service account 'Logon As A Service' privileges")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantServiceExecutionRight::CAGrantServiceExecutionRight(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantServiceExecutionRight::~CAGrantServiceExecutionRight()
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
UINT CAGrantServiceExecutionRight::OnExecution()
{
    LSA_HANDLE  PolicyHandle = NULL;
    PSID        pSid;
    NTSTATUS    Status;
    tstring     strServiceDomainUsername;
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("SERVICE_DOMAINUSERNAME"), strServiceDomainUsername );
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
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            Status,
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
            NULL,                               // default lookup logic
            strServiceDomainUsername.c_str(),   // account to obtain SID
            &pSid                               // buffer to allocate to contain resultant SID
            )
    ) 
    {

        //
        // We only grant the privilege if we succeeded in obtaining the
        // SID. We can actually add SIDs which cannot be looked up, but
        // looking up the SID is a good sanity check which is suitable for
        // most cases.

        //
        // Grant the SeServiceLogonRight to users represented by pSid.
        //
        Status = SetPrivilegeOnAccount(
                    PolicyHandle,           // policy handle
                    pSid,                   // SID to grant privilege
                    L"SeServiceLogonRight", // Unicode privilege
                    TRUE                    // enable the privilege
                    );

        if(Status != STATUS_SUCCESS)
        {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                Status,
                _T("Failed call to SetPrivilegeOnAccount")
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
            _T("Failed to be able to optain the SID for the selected user on the localhost")
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
// Function:    GrantServiceExecutionRight
//
// Description: This custom action reads the SERVICE_DOMAINUSERNAME
//              public property and grants that user the
//              SeLogonServiceRight right.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall GrantServiceExecutionRight(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAGrantServiceExecutionRight* pCA = new CAGrantServiceExecutionRight(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

const char *BOINC_RCSID_00ed9586df="$Id$";
