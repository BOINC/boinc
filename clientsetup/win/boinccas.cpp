// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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


/////////////////////////////////////////////////////////////////////
//
// Function:    BOINCCABase::BOINCCABase
//
// Description: Initialize the Custom Action infrastructure.
//
/////////////////////////////////////////////////////////////////////

BOINCCABase::BOINCCABase(
    MSIHANDLE hMSIHandle,
    const tstring strActionName,
    const tstring strProgressTitle
    )
{

    // Store the parameters for later use
    m_hMSIHandle = hMSIHandle;
    m_strActionName = strActionName;
    m_strProgressTitle = strProgressTitle;

    // Initialize all other values to zero or null
	m_phActionStartRec = NULL;
	m_phActionDataRec = NULL;
    m_phProgressRec = NULL;
    m_phLogInfoRec = NULL;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    BOINCCABase::~BOINCCABase
//
// Description: Cleanup up allocation resources.
//
/////////////////////////////////////////////////////////////////////
BOINCCABase::~BOINCCABase()
{

    if (m_phActionStartRec)
    {
        MsiCloseHandle(m_phActionStartRec);
        m_phActionStartRec = NULL;
    }

    if (m_phActionDataRec)
    {
        MsiCloseHandle(m_phActionDataRec);
        m_phActionDataRec = NULL;
    }

    if (m_phProgressRec)
    {
        MsiCloseHandle(m_phProgressRec);
        m_phProgressRec = NULL;
    }

    if (m_phLogInfoRec)
    {
        MsiCloseHandle(m_phLogInfoRec);
        m_phLogInfoRec = NULL;
    }

    m_strActionName.clear();
    m_strProgressTitle.clear();

}


/////////////////////////////////////////////////////////////////////
//
// Function:    Execute
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::Execute()
{
    UINT    uiReturnValue = 0;

    OnInitialize();

    if      ( TRUE == MsiGetMode( m_hMSIHandle, MSIRUNMODE_SCHEDULED ) )
    {
        uiReturnValue = OnInstall();
    }
    else if ( TRUE == MsiGetMode( m_hMSIHandle, MSIRUNMODE_COMMIT ) )
    {
        uiReturnValue = OnCommit();
    }
    else if ( TRUE == MsiGetMode( m_hMSIHandle, MSIRUNMODE_ROLLBACK ) )
    {
        uiReturnValue = OnRollback();
    }
    else
    {
        uiReturnValue = OnExecution();
    }

    OnCleanup();

    return uiReturnValue;
}


static BOOL IsVersionNewer(const tstring v1, const tstring v2) {
    int v1_maj=0, v1_min=0, v1_rel=0;
    int v2_maj=0, v2_min=0, v2_rel=0;

    _stscanf(v1.c_str(), _T("%d.%d.%d"), &v1_maj, &v1_min, &v1_rel);
    _stscanf(v2.c_str(), _T("%d.%d.%d"), &v2_maj, &v2_min, &v2_rel);

    if (v1_maj > v2_maj) return TRUE;
    if (v1_maj < v2_maj) return FALSE;
    if (v1_min > v2_min) return TRUE;
    if (v1_min < v2_min) return FALSE;
    if (v1_rel > v2_rel) return TRUE;
    return FALSE;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    SetUpgradeParameters
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::SetUpgradeParameters()
{
    tstring strCurrentProductVersion;
    UINT    uiReturnValue = 0;

    uiReturnValue = GetProperty( _T("ProductVersion"), strCurrentProductVersion );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = SetRegistryValue( _T("UpgradingTo"), strCurrentProductVersion );
    if ( uiReturnValue ) return uiReturnValue;

    return ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
//
// Function:    IsUpgrading
//
// Description:
//
/////////////////////////////////////////////////////////////////////
BOOL BOINCCABase::IsUpgrading()
{
    tstring strCurrentProductVersion;
    tstring strRegistryProductVersion;
    UINT    uiReturnValue = 0;

    uiReturnValue = GetProperty( _T("ProductVersion"), strCurrentProductVersion );
    if ( uiReturnValue ) return FALSE;

    uiReturnValue = GetRegistryValue( _T("UpgradingTo"), strRegistryProductVersion );
    if ( uiReturnValue ) return FALSE;

    return IsVersionNewer(strRegistryProductVersion, strCurrentProductVersion);
}


/////////////////////////////////////////////////////////////////////
//
// Function:    OnInitialize
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnInitialize()
{
    UINT uiReturnValue = 0;

	m_phActionStartRec = MsiCreateRecord(3);
    assert(NULL != m_phActionStartRec);

    MsiRecordSetString(m_phActionStartRec, 1, m_strActionName.c_str());
    MsiRecordSetString(m_phActionStartRec, 2, m_strProgressTitle.c_str());
    MsiRecordSetString(m_phActionStartRec, 3, _T("[1]"));
    uiReturnValue = MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_ACTIONSTART, m_phActionStartRec);
	if ((uiReturnValue == IDCANCEL))
		return ERROR_INSTALL_USEREXIT;

    // Give the UI a chance to refresh.
    Sleep(0);

	m_phActionDataRec = MsiCreateRecord(3);
    assert(NULL != m_phActionDataRec);


    m_phProgressRec = MsiCreateRecord(3);
    assert(NULL != m_phProgressRec);

    MsiRecordSetInteger(m_phProgressRec, 1, 1);
    MsiRecordSetInteger(m_phProgressRec, 2, 1);
    MsiRecordSetInteger(m_phProgressRec, 3, 0);
    uiReturnValue = MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_PROGRESS, m_phProgressRec);
	if ((uiReturnValue == IDCANCEL))
		return ERROR_INSTALL_USEREXIT;


    m_phLogInfoRec = MsiCreateRecord(3);
    assert(NULL != m_phLogInfoRec);

    MsiRecordSetString (m_phLogInfoRec, 0, _T("Custom Message : Action Name: [1] Description: [2] Error Code: [3] "));
    MsiRecordSetString (m_phLogInfoRec, 1, m_strActionName.c_str());


    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL,
        NULL,
        NULL,
        NULL,
        _T("Starting Custom Action")
        );


    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    OnCleanup
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnCleanup()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    OnInstall
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnInstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    OnRollback
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnRollback()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    OnCommit
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnCommit()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    OnExecution
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnExecution()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    GetRegistryValue
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::GetRegistryValue(
    const tstring strName,
    tstring&      strValue,
    bool          bDisplayValue
    )
{
	LONG lReturnValue;
	HKEY hkSetupHive;
	DWORD dwSize = 0;
    LPTSTR lpszRegistryValue = NULL;
    tstring strMessage;

	lReturnValue = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),
		0,
        KEY_READ,
        &hkSetupHive
    );
	if (lReturnValue != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;

    // How large does our buffer need to be?
    RegQueryValueEx(
        hkSetupHive,
        strName.c_str(),
        NULL,
        NULL,
        NULL,
        &dwSize
    );

    // Allocate the buffer space.
    lpszRegistryValue = (LPTSTR) malloc(dwSize);
    (*lpszRegistryValue) = NULL;

    // Now get the data
    lReturnValue = RegQueryValueEx(
        hkSetupHive,
        strName.c_str(),
        NULL,
        NULL,
        (LPBYTE)lpszRegistryValue,
        &dwSize
    );

    // Send up the returned value.
    if (lReturnValue == ERROR_SUCCESS) strValue = lpszRegistryValue;

    // Cleanup
	RegCloseKey(hkSetupHive);
    free(lpszRegistryValue);

    // One last check to make sure everything is on the up and up.
    if (lReturnValue != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;


    strMessage  = _T("Successfully retrieved registry value '") + strName;
    strMessage += _T("' with a value of '");
    if (bDisplayValue)
        strMessage += strValue;
    else
        strMessage += _T("<Value Hidden>");
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL,
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    SetRegistryValue
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::SetRegistryValue(
    const tstring strName,
    const tstring strValue,
    bool          bDisplayValue
    )
{
	LONG lReturnValue;
	HKEY hkSetupHive;
    tstring strMessage;

	lReturnValue = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),
		0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        NULL,
        &hkSetupHive,
        NULL
    );
	if (lReturnValue != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;

    lReturnValue = RegSetValueEx(
        hkSetupHive,
        strName.c_str(),
        0,
        REG_SZ,
        (CONST BYTE *)strValue.c_str(),
        (DWORD)(strValue.size()*sizeof(TCHAR))
    );

	RegCloseKey(hkSetupHive);
	if (lReturnValue != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;

    strMessage  = _T("Successfully set registry value '") + strName;
    strMessage += _T("' to a value of '");
    if (bDisplayValue)
        strMessage += strValue;
    else
        strMessage += _T("<Value Hidden>");
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL,
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    GetProperty
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::GetProperty(
    const tstring strPropertyName,
    tstring&      strPropertyValue,
    bool          bDisplayValue
    )
{
    LPTSTR  lpszBuffer = NULL;
    DWORD   dwCharacterCount = 0;
    tstring strMessage;
    UINT    uiReturnValue = 0;

    uiReturnValue = MsiGetProperty(m_hMSIHandle, strPropertyName.c_str(), _T(""), &dwCharacterCount);
    switch(uiReturnValue)
    {
    case ERROR_INVALID_HANDLE:
    case ERROR_INVALID_PARAMETER:
        strMessage  = _T("Failed to get '") + strPropertyName;
        strMessage += _T("'");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            strMessage.c_str()
        );
        return ERROR_INSTALL_FAILURE;
        break;
    }

    // Allocate memory for the property value return buffer
    lpszBuffer = (LPTSTR) malloc( ((++dwCharacterCount)*sizeof(LPTSTR)) );
    uiReturnValue = MsiGetProperty(m_hMSIHandle, strPropertyName.c_str(), lpszBuffer, &dwCharacterCount);
    switch(uiReturnValue)
    {
    case ERROR_INVALID_HANDLE:
    case ERROR_INVALID_PARAMETER:
        strMessage  = _T("Failed to get '") + strPropertyName;
        strMessage += _T("' after allocating the buffer");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            strMessage.c_str()
        );
        if ( lpszBuffer ) free( lpszBuffer );
        return ERROR_INSTALL_FAILURE;
        break;
    }

    strPropertyValue = lpszBuffer;
    free( lpszBuffer );

    strMessage  = _T("Successfully retrieved '") + strPropertyName;
    strMessage += _T("' with a value of '");
    if (bDisplayValue)
        strMessage += strPropertyValue;
    else
        strMessage += _T("<Value Hidden>");
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL,
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    SetProperty
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::SetProperty(
    const tstring strPropertyName,
    const tstring strPropertyValue,
    bool          bDisplayValue
    )
{
    tstring strMessage;
    UINT uiReturnValue = 0;

    uiReturnValue = MsiSetProperty(
        m_hMSIHandle,
        strPropertyName.c_str(),
        strPropertyValue.c_str()
        );
    switch(uiReturnValue)
    {
    case ERROR_FUNCTION_FAILED:
    case ERROR_INVALID_HANDLE:
    case ERROR_INVALID_PARAMETER:
        strMessage  = _T("Failed to set '") + strPropertyName;
        strMessage += _T("' to a value of '");
        if (bDisplayValue)
            strMessage += strPropertyValue;
        else
            strMessage += _T("<Value Hidden>");
        strMessage += _T("'");

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            strMessage.c_str()
        );
        return ERROR_INSTALL_FAILURE;
        break;
    }

    strMessage  = _T("Successfully set '") + strPropertyName;
    strMessage += _T("' to a value of '");
    if (bDisplayValue)
        strMessage += strPropertyValue;
    else
        strMessage += _T("<Value Hidden>");
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL,
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    GetComponentKeyFilename
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::GetComponentKeyFilename(
    const tstring strComponentName,
    tstring&      strComponentKeyFilename
    )
{
    UINT        uiReturnValue = 0;
    tstring     strMessage;
    tstring     strQuery;
    TCHAR       szBuffer[256];
    DWORD       dwBufferSize = sizeof(szBuffer);
    MSIHANDLE   hDatabase;
    MSIHANDLE   hView;
    MSIHANDLE   hRecComponentName;
    MSIHANDLE   hRec;


	// Get the handle to the MSI package we are executing for.
	hDatabase = MsiGetActiveDatabase(m_hMSIHandle);
	if (!hDatabase) return ERROR_INSTALL_FAILURE;

	// Construct the query that is going to give us the result we need.
    strQuery  = _T("SELECT `KeyPath` FROM `Component` WHERE `Component`= ?");

	// Create the view
    uiReturnValue = MsiDatabaseOpenView(hDatabase, strQuery.c_str(), &hView);
    switch(uiReturnValue)
    {
    case ERROR_BAD_QUERY_SYNTAX:
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiDatabaseOpenView reports an invalid query was issued")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    case ERROR_INVALID_HANDLE:
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiDatabaseOpenView reports an invalid handle was used")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    }

    // Create query parameter
    hRecComponentName = MsiCreateRecord(1);
    uiReturnValue = MsiRecordSetString(hRecComponentName, 1, strComponentName.c_str());
    switch(uiReturnValue)
    {
    case ERROR_INVALID_HANDLE:
        MsiCloseHandle(hRecComponentName);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiRecordSetString reports an invalid handle was used")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    case ERROR_INVALID_PARAMETER:
        MsiCloseHandle(hRecComponentName);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiRecordSetString reports an invalid parameter was used")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    }


    // Execute the query
    uiReturnValue = MsiViewExecute(hView, hRecComponentName);
    switch(uiReturnValue)
    {
    case ERROR_FUNCTION_FAILED:
        MsiViewClose(hView);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiViewExecute failed to execute the view")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    case ERROR_INVALID_HANDLE:
        MsiViewClose(hView);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiViewExecute reports an invalid handle was used")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    }

    // Only one row should be returned by the resultset, so there is no need
    //   to execute MsiViewFetch more than once.
    uiReturnValue = MsiViewFetch(hView, &hRec);
    switch(uiReturnValue)
    {
    case ERROR_FUNCTION_FAILED:
        MsiViewClose(hView);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiViewFetch: An error occurred during fetching")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    case ERROR_INVALID_HANDLE:
        MsiViewClose(hView);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiViewFetch reports an invalid handle was used")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    case ERROR_INVALID_HANDLE_STATE:
        MsiViewClose(hView);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiViewFetch reports the handle was in an invalid state")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    }

    // Okay, now it is time to parse the string that was returned.
    uiReturnValue = MsiRecordGetString(hRec, 1, (LPTSTR)&szBuffer, &dwBufferSize);
    switch(uiReturnValue)
    {
    case ERROR_INVALID_HANDLE:
        MsiCloseHandle(hRec);
        MsiViewClose(hView);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiRecordGetString reports an invalid handle was used")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    case ERROR_INVALID_PARAMETER:
        MsiCloseHandle(hRec);
        MsiViewClose(hView);
        MsiCloseHandle(hDatabase);

        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("MsiRecordGetString reports an invalid parameter was used")
        );

        return ERROR_INSTALL_FAILURE;
        break;
    }

    // Save the string
    strComponentKeyFilename = szBuffer;

    // strComponentKeyFilename format is [strComponentName]_[filename]
    // Remove the component name from the string
    strComponentKeyFilename =
        strComponentKeyFilename.substr(strComponentName.size() + 1);

    strMessage  = _T("The key filename for component '");
    strMessage += strComponentName;
    strMessage += _T("' is '");
    strMessage += strComponentKeyFilename;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL,
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    DisplayMessage
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::DisplayMessage(
    const UINT         uiPushButtonStyle,       // push button sstyle to use in message box
    const UINT         uiIconStyle,             // icon style to use in message box
    const tstring      strMessage               // message
    )
{
    UINT        uiReturnValue = 0;

    uiReturnValue = ::MessageBox(
        NULL,
        strMessage.c_str(),
        _T("Installer Message"),
        uiPushButtonStyle | uiIconStyle | MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION
        );

    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    LogProgress
//
// Description:
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::LogProgress(
    const tstring strProgress
    )
{
    UINT uiReturnValue = 0;

    MsiRecordSetString(m_phActionDataRec, 2, strProgress.c_str());

    // returns IDOK if successful
    uiReturnValue = MsiProcessMessage( m_hMSIHandle, INSTALLMESSAGE_ACTIONDATA, m_phActionDataRec );
	if ((uiReturnValue == IDCANCEL))
		return ERROR_INSTALL_USEREXIT;

    // Give the UI a chance to refresh.
    Sleep(0);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    LogMessage
//
// Description: This function writes to the MSI log file and displays
//              the SetupError dialog box as appropriate.
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::LogMessage(
    const UINT    uiInstallMessageType,    // message type to send to Windows Installer
    const UINT    uiPushButtonStyle,       // push button sstyle to use in message box
    const UINT    uiIconStyle,             // icon style to use in message box
    const UINT    uiErrorNumber,           // number of error in Error table
    const UINT    uiErrorCode,             // the return value from an api
    const tstring strMessage               // message
    )
{
    UINT        uiReturnValue = 0;

    switch(uiInstallMessageType)
    {

    // Send informational message to the log file
    case INSTALLMESSAGE_INFO:

        MsiRecordSetString (m_phLogInfoRec, 2, strMessage.c_str());
        MsiRecordSetInteger(m_phLogInfoRec, 3, uiErrorCode);

        // returns IDOK if successful
        uiReturnValue = MsiProcessMessage(
            m_hMSIHandle,
            INSTALLMESSAGE(uiInstallMessageType),
            m_phLogInfoRec
            );
        break;

    // Display a dialog and send error message to log file
    case INSTALLMESSAGE_ERROR:
    case INSTALLMESSAGE_WARNING:
    case INSTALLMESSAGE_USER:

        PMSIHANDLE phLogErrorRec = MsiCreateRecord(2);

        MsiRecordSetString (phLogErrorRec, 0, _T("[1]"));
        MsiRecordSetString (phLogErrorRec, 1, strMessage.c_str());

        // Return value to indicate which button is
        // pushed on message box
        uiReturnValue = MsiProcessMessage(
            m_hMSIHandle,
            INSTALLMESSAGE(uiInstallMessageType|uiPushButtonStyle|uiIconStyle),
            phLogErrorRec
            );
        break;

    }
    return uiReturnValue;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    RebootWhenFinished
//
// Description: Reboot computer when setup completes installation.
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::RebootWhenFinished()
{
    SetProperty(_T("RETURN_REBOOTREQUESTED"), _T("1"));
    return MsiSetMode(m_hMSIHandle, MSIRUNMODE_REBOOTATEND, TRUE);
}

