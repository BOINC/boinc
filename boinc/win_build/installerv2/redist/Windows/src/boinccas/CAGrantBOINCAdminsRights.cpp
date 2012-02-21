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
#include "CAGrantBOINCAdminsRights.h"
#include "lsaprivs.h"


#define CUSTOMACTION_NAME               _T("CAGrantBOINCAdminsRights")
#define CUSTOMACTION_PROGRESSTITLE      _T("Validating the BOINC Administrators group privilege levels")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCAdminsRights::CAGrantBOINCAdminsRights(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAGrantBOINCAdminsRights::~CAGrantBOINCAdminsRights()
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
UINT CAGrantBOINCAdminsRights::OnExecution()
{
    PSID        pSid;
    tstring     strOSVersion;
    UINT        uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("VersionNT"), strOSVersion );
    if ( uiReturnValue ) return uiReturnValue;


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
            tstring(L"boinc_admins").c_str(),        // account to obtain SID
            &pSid                                    // buffer to allocate to contain resultant SID
            )
    ) 
    {

        //
        // We only grant the privilege if we succeeded in obtaining the
        // SID. We can actually add SIDs which cannot be looked up, but
        // looking up the SID is a good sanity check which is suitable for
        // most cases.

        // User Rights
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeNetworkLogonRight' right."));
        GrantUserRight(pSid, L"SeNetworkLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeRemoteInteractiveLogonRight' right."));
        GrantUserRight(pSid, L"SeRemoteInteractiveLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeBatchLogonRight' right."));
        GrantUserRight(pSid, L"SeBatchLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeInteractiveLogonRight' right."));
        GrantUserRight(pSid, L"SeInteractiveLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeServiceLogonRight' right."));
        GrantUserRight(pSid, L"SeServiceLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeDenyNetworkLogonRight' right."));
        GrantUserRight(pSid, L"SeDenyNetworkLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeDenyInteractiveLogonRight' right."));
        GrantUserRight(pSid, L"SeDenyInteractiveLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeDenyBatchLogonRight' right."));
        GrantUserRight(pSid, L"SeDenyBatchLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeDenyServiceLogonRight' right."));
        GrantUserRight(pSid, L"SeDenyServiceLogonRight", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        // Windows 2000 and older does not have the SeDenyRemoteInteractiveLogonRight user right
        //
        if (strOSVersion > _T("500")) {
            LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeDenyRemoteInteractiveLogonRight' right."));
            GrantUserRight(pSid, L"SeDenyRemoteInteractiveLogonRight", FALSE);
            LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));
        }

        // Privileges
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeTcbPrivilege' right."));
        GrantUserRight(pSid, L"SeTcbPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeMachineAccountPrivilege' right."));
        GrantUserRight(pSid, L"SeMachineAccountPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeIncreaseQuotaPrivilege' right."));
        if (!GrantUserRight(pSid, L"SeIncreaseQuotaPrivilege", TRUE))
        {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed call to GrantUserRight - SeIncreaseQuotaPrivilege")
            );
        }
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeBackupPrivilege' right."));
        GrantUserRight(pSid, L"SeBackupPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeChangeNotifyPrivilege' right."));
        if (!GrantUserRight(pSid, L"SeChangeNotifyPrivilege", TRUE))
        {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed call to GrantUserRight - SeChangeNotifyPrivilege")
            );
        }
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeSystemTimePrivilege' right."));
        GrantUserRight(pSid, L"SeSystemTimePrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeCreateTokenPrivilege' right."));
        GrantUserRight(pSid, L"SeCreateTokenPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeCreatePagefilePrivilege' right."));
        GrantUserRight(pSid, L"SeCreatePagefilePrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeCreateGlobalPrivilege' right."));
        if (!GrantUserRight(pSid, L"SeCreateGlobalPrivilege", TRUE))
        {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed call to GrantUserRight - SeCreateGlobalPrivilege")
            );
        }
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeDebugPrivilege' right."));
        GrantUserRight(pSid, L"SeDebugPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeEnableDelegationPrivilege' right."));
        GrantUserRight(pSid, L"SeEnableDelegationPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeRemoteShutdownPrivilege' right."));
        GrantUserRight(pSid, L"SeRemoteShutdownPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeAuditPrivilege' right."));
        GrantUserRight(pSid, L"SeAuditPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeImpersonatePrivilege' right."));
        GrantUserRight(pSid, L"SeImpersonatePrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeIncreaseBasePriorityPrivilege' right."));
        GrantUserRight(pSid, L"SeIncreaseBasePriorityPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeLoadDriverPrivilege' right."));
        GrantUserRight(pSid, L"SeLoadDriverPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeLockMemoryPrivilege' right."));
        GrantUserRight(pSid, L"SeLockMemoryPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeSecurityPrivilege' right."));
        GrantUserRight(pSid, L"SeSecurityPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeSystemEnvironmentPrivilege' right."));
        GrantUserRight(pSid, L"SeSystemEnvironmentPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeManageVolumePrivilege' right."));
        GrantUserRight(pSid, L"SeManageVolumePrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeProfileSingleProcessPrivilege' right."));
        GrantUserRight(pSid, L"SeProfileSingleProcessPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeSystemProfilePrivilege' right."));
        GrantUserRight(pSid, L"SeSystemProfilePrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeUndockPrivilege' right."));
        GrantUserRight(pSid, L"SeUndockPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeAssignPrimaryTokenPrivilege' right."));
        if (!GrantUserRight(pSid, L"SeAssignPrimaryTokenPrivilege", TRUE))
        {
            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed call to GrantUserRight - SeAssignPrimaryTokenPrivilege")
            );
        }
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeRestorePrivilege' right."));
        GrantUserRight(pSid, L"SeRestorePrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeShutdownPrivilege' right."));
        GrantUserRight(pSid, L"SeShutdownPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeSynchAgentPrivilege' right."));
        GrantUserRight(pSid, L"SeSynchAgentPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Checking the 'SeTakeOwnershipPrivilege' right."));
        GrantUserRight(pSid, L"SeTakeOwnershipPrivilege", FALSE);
        LogMessage(INSTALLMESSAGE_INFO, NULL, NULL, NULL, NULL, _T("Check completed."));

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
    if(pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    GrantBOINCAdminsRights
//
// Description: This custom action grants the 'boinc_admins' group the
//              required rights.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall GrantBOINCAdminsRights(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAGrantBOINCAdminsRights* pCA = new CAGrantBOINCAdminsRights(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
