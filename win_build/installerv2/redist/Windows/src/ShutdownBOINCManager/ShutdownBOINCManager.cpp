// ShutdownBOINCManager.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"


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
// Function:    ShutdownBOINCManager
//
// Description: This custom action shuts down the BOINC Manager.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall ShutdownBOINCManager(MSIHANDLE hInstall)
{
    HWND        hWndBOINCManagerSystray = NULL;
    LRESULT     lrReturnValue = NULL;
    UINT        uiLoopCounter = 0;
    UINT        uiReturn = -1;

    const UINT WM_TASKBARSHUTDOWN = ::RegisterWindowMessage(_T("TaskbarShutdown"));

    do
    {
        hWndBOINCManagerSystray = FindWindow( _T("wxTaskBarExWindowClass"), _T("BOINCManagerSystray") );
        if ( NULL != hWndBOINCManagerSystray )
        {
            lrReturnValue = SendMessage( hWndBOINCManagerSystray, WM_TASKBARSHUTDOWN, NULL, NULL );
            if ( 0 != lrReturnValue )
            {
                LogError(
                    hInstall,
                    INSTALLMESSAGE_ERROR,
                    NULL, 
                    NULL,
                    NULL,
                    _T("ShutdownBOINCManager"),
                    _T("Setup was unable to shutdown the BOINC Manager Systray window."),
                    (int)lrReturnValue
                );
                return ERROR_INSTALL_FAILURE;
            }
            Sleep(1000);
        }
        uiLoopCounter++;
    }
    while ( (NULL != hWndBOINCManagerSystray) && ( 20 >= uiLoopCounter ) );

    if ( NULL != hWndBOINCManagerSystray )
    {
        LogError(
            hInstall,
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            _T("ShutdownBOINCManager"),
            _T("One or more BOINC Manager applications could not be closed, please close them and then rerun setup."),
            uiLoopCounter
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Give the manager a few seconds to shutdown.
    Sleep(5000);

    return ERROR_SUCCESS;
}


const char *BOINC_RCSID_eeaf7c4c79 = "$Id$";
