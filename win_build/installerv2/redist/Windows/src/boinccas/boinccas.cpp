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


/////////////////////////////////////////////////////////////////////
// 
// Function:    BOINCCABase::BOINCCABase
//
// Description: Initialize the Custom Action infrastructure.
//
/////////////////////////////////////////////////////////////////////

BOINCCABase::BOINCCABase(
    MSIHANDLE     hMSIHandle, 
    const tstring strActionName,
    const tstring strProgressTtle
    )
{

    // Store the parameters for later use
    m_hMSIHandle = hMSIHandle;
    m_strActionName = strActionName;
    m_strProgressTtle = strProgressTtle;


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
    m_strProgressTtle.clear();

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
    tstring strMaintenanceMode;

    OnInitialize();

    uiReturnValue = GetProperty( _T("_IsMaintenance"), strMaintenanceMode );
    if ( uiReturnValue ) return uiReturnValue;

    if      ( TRUE == MsiGetMode( m_hMSIHandle, MSIRUNMODE_SCHEDULED ) )
    {
        if      ( _T("Install") == strMaintenanceMode )
        {
            uiReturnValue = OnInstall();;
        }
        else if ( _T("Reinstall") == strMaintenanceMode )
        {
            uiReturnValue = OnReinstall();
        }
        else if ( _T("Remove") == strMaintenanceMode )
        {
            uiReturnValue = OnUninstall();
        }
    }
    else if ( TRUE == MsiGetMode( m_hMSIHandle, MSIRUNMODE_ROLLBACK ) )
    {
        if      ( _T("Install") == strMaintenanceMode )
        {
            uiReturnValue = OnRollbackInstall();;
        }
        else if ( _T("Reinstall") == strMaintenanceMode )
        {
            uiReturnValue = OnRollbackReinstall();
        }
        else if ( _T("Remove") == strMaintenanceMode )
        {
            uiReturnValue = OnRollbackUninstall();
        }
    }
    else if ( TRUE == MsiGetMode( m_hMSIHandle, MSIRUNMODE_COMMIT ) )
    {
        if      ( _T("Install") == strMaintenanceMode )
        {
            uiReturnValue = OnCommitInstall();;
        }
        else if ( _T("Reinstall") == strMaintenanceMode )
        {
            uiReturnValue = OnCommitReinstall();
        }
        else if ( _T("Remove") == strMaintenanceMode )
        {
            uiReturnValue = OnCommitUninstall();
        }
    }
    else
    {
        uiReturnValue = OnExecution();
    }

    OnCleanup();

    return uiReturnValue;
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
    MsiRecordSetString(m_phActionStartRec, 2, m_strProgressTtle.c_str());
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
// Function:    OnReinstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnReinstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    OnUninstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnUninstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    OnRollbackInstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnRollbackInstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    OnRollbackReinstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnRollbackReinstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    OnRollbackUninstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnRollbackUninstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    OnCommitInstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnCommitInstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    OnCommitReinstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnCommitReinstall()
{
    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    OnCommitUninstall
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::OnCommitUninstall()
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
// Function:    GetProperty
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT BOINCCABase::GetProperty( 
    const tstring strPropertyName, 
    tstring&      strPropertyValue
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
        return ERROR_INSTALL_FAILURE;
    }

    // Allocate memory for the property value return buffer
    lpszBuffer = (LPTSTR) malloc( ((++dwCharacterCount)*sizeof(LPTSTR)) );
    uiReturnValue = MsiGetProperty(m_hMSIHandle, strPropertyName.c_str(), lpszBuffer, &dwCharacterCount);
    switch(uiReturnValue)
    {
    case ERROR_INVALID_HANDLE:
    case ERROR_INVALID_PARAMETER:
        if ( lpszBuffer ) free( lpszBuffer );
        return ERROR_INSTALL_FAILURE;
    }

    strPropertyValue = lpszBuffer;
    free( lpszBuffer );

    strMessage  = _T("Successfully retrieved '") + strPropertyName;
    strMessage += _T("' with a value of '") + strPropertyValue;
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
    const tstring strPropertyValue
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
        return ERROR_INSTALL_FAILURE;
    }

    strMessage  = _T("Successfully set '") + strPropertyName;
    strMessage += _T("' to a value of '") + strPropertyValue;
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
        assert(NULL != phLogErrorRec);

        MsiRecordSetInteger(phLogErrorRec, 1, uiErrorNumber);

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

