// GrantServiceExecutionRight.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "lsaprivs.h"


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    LogError
//
// Description: This function writes to the MSI log file and displays
//              the SetupError dialog box as appropriate.
//
/////////////////////////////////////////////////////////////////////
int LogError(
    MSIHANDLE hInstall, // handle to the installation session
    UINT msgType,       // message type to send to Windows Installer
    UINT pbStyle,       // push button sstyle to use in message box
    UINT iconStyle,     // icon style to use in message box
    int errNum,         // number of error in Error table
    TCHAR* ActionName,  // name of the custom action causing error
    TCHAR* ErrorDesc,   // description of the error
    int statuscode      // the return value from an api
    )
{
    PMSIHANDLE  hRec;               // handle to a record object
    TCHAR       frmtString[4096];   // string to use in field 0

    switch(msgType)
    {
    // Send informational message to the log file
    case INSTALLMESSAGE_INFO:
        // generate the format string for field 0
        _tcscpy(frmtString, _T("Custom Message : "));
        _tcscat(frmtString, _T("Action Name: [1] "));
        _tcscat(frmtString, _T("Description: [2] "));
        _tcscat(frmtString, _T("Status Code: [3] "));

        hRec = MsiCreateRecord(3);

        MsiRecordSetString(hRec, 0,  frmtString);
        MsiRecordSetString(hRec, 1,  ActionName);
        MsiRecordSetString(hRec, 2,  ErrorDesc);
        MsiRecordSetInteger(hRec, 3, statuscode);

        // returns IDOK if successful
        return (MsiProcessMessage(hInstall, INSTALLMESSAGE_INFO, 
                                                            hRec));
        break;

    // Display a dialog and send error message to log file
    case INSTALLMESSAGE_ERROR:
    case INSTALLMESSAGE_WARNING:
    case INSTALLMESSAGE_USER:
        hRec = MsiCreateRecord(4);

        MsiRecordSetInteger(hRec, 1, errNum);
        MsiRecordSetString(hRec, 2, ActionName);
        MsiRecordSetString(hRec, 3, ErrorDesc);
        MsiRecordSetInteger(hRec, 4, statuscode);

        // Return value to indicate which button is 
        // pushed on message box
        return (MsiProcessMessage(hInstall, 
                  INSTALLMESSAGE(msgType|pbStyle|iconStyle), hRec));
        break;
    }
    return 0;
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
    LSA_HANDLE  PolicyHandle = NULL;
    LPCTSTR     PropertyName = TEXT("SERVICE_DOMAINUSERNAME");
    TCHAR*      szPropertyBuf = NULL;
    DWORD       ncharCount = 0;
    UINT        uiReturn = -1;
    PSID        pSid;
    NTSTATUS    Status;


    // You first send an empty string in order to get the
    // correct size of the buffer needed to hold the property value
    uiReturn = MsiGetProperty(hInstall, PropertyName, TEXT(""), &ncharCount);

    switch(uiReturn)
    {
    case ERROR_INVALID_HANDLE:
    case ERROR_INVALID_PARAMETER:
        return ERROR_INSTALL_FAILURE;
    }

    // Allocate memory for the property value return buffer
    szPropertyBuf = (LPTSTR) malloc( ((++ncharCount)*sizeof(LPTSTR)) );
    uiReturn = MsiGetProperty(hInstall, PropertyName, szPropertyBuf, &ncharCount);

    switch(uiReturn)
    {
    case ERROR_INVALID_HANDLE:
    case ERROR_INVALID_PARAMETER:
        return ERROR_INSTALL_FAILURE;
    }


    //
    // Open the policy on the local host.
    //
    Status = OpenPolicy(
                _T(""),
                POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
                &PolicyHandle
                );


    if(Status != STATUS_SUCCESS) {
        LogError(
            hInstall,
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            _T("GrantServiceExecutionRight"),
            _T("Failed to be able to open a policy to the localhost"),
            NULL
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
            NULL,         // default lookup logic
            szPropertyBuf,// account to obtain SID
            &pSid         // buffer to allocate to contain resultant SID
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
            LogError(
                hInstall,
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                _T("GrantServiceExecutionRight"),
                _T("Failed call to SetPrivilegeOnAccount"),
                Status
            );
            return ERROR_INSTALL_FAILURE;
        }
    }
    else
    {
        LogError(
            hInstall,
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            _T("GrantServiceExecutionRight"),
            _T("Failed to be able to optain the SID for the selected user on the localhost"),
            NULL
        );
        return ERROR_INSTALL_FAILURE;
    }


    //
    // Cleanup any handles and memory allocated during the custom action
    //
    LsaClose(PolicyHandle);
    if(pSid != NULL) HeapFree(GetProcessHeap(), 0, pSid);
    free(szPropertyBuf);

    return ERROR_SUCCESS;
}


const char *BOINC_RCSID_eeaf7c4c79 = "$Id$";
