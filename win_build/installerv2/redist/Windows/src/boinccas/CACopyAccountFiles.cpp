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
#include "CACopyAccountFiles.h"

#define CUSTOMACTION_NAME               _T("CACopyAccountFiles")
#define CUSTOMACTION_PROGRESSTITLE      _T("Copying account files from specified location")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACopyAccountFiles::CACopyAccountFiles(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACopyAccountFiles::~CACopyAccountFiles()
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
UINT CACopyAccountFiles::OnExecution()
{
    WIN32_FIND_DATA fdFileData;         // Data structure describes the file found
    HANDLE          hfdSearch;          // Search handle returned by FindFirstFile
    BOOL            bFinished = FALSE;
    tstring         strInstallationDirectory;
    tstring         strAccountFilesLocation;
    tstring         strSearchString;
    tstring         strOriginalFilename;
    tstring         strDestinationFilename;
    tstring         strMessage;
    UINT            uiReturnValue = 0;


    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallationDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ACCOUNTS_LOCATION"), strAccountFilesLocation );
    if ( uiReturnValue ) return uiReturnValue;


    // We want to find all the xml account files from the szAccountsLicationBuf so we need
    //   to allocate a new buffer and tact on \*.xml to it before passing it on to
    //   FindFirstFile
    strSearchString = strAccountFilesLocation + _T("\\*.xml");


    // Start searching for .xml files in the network repository.
    hfdSearch = FindFirstFile(strSearchString.c_str(), &fdFileData);
    if ( INVALID_HANDLE_VALUE == hfdSearch )
    {
        LogMessage(
            INSTALLMESSAGE_ERROR,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("No .xml files found.")
        );
        return ERROR_INSTALL_FAILURE;
    }

    // Copy each .xml file to the new directory.
    while (!bFinished)
    {
        LogProgress( fdFileData.cFileName );
        Sleep(0);

        strOriginalFilename     = strAccountFilesLocation +  _T("\\");
        strOriginalFilename    += fdFileData.cFileName;

        strDestinationFilename  = strInstallationDirectory + _T("\\");
        strDestinationFilename += fdFileData.cFileName;

        if (CopyFile (strOriginalFilename.c_str(), strDestinationFilename.c_str(), FALSE))
        {
            strMessage  = _T("Successfully copied '");
            strMessage += fdFileData.cFileName;
            strMessage += _T("' from '");
            strMessage += strAccountFilesLocation;
            strMessage += _T("' to '");
            strMessage += strInstallationDirectory;
            strMessage += _T("'");

            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                strMessage.c_str()
            );
        }
        else
        {
            strMessage  = _T("Failed to copy '");
            strMessage += fdFileData.cFileName;
            strMessage += _T("' from '");
            strMessage += strAccountFilesLocation;
            strMessage += _T("' to '");
            strMessage += strInstallationDirectory;
            strMessage += _T("'");

            LogMessage(
                INSTALLMESSAGE_ERROR,
                NULL, 
                NULL,
                NULL,
                GetLastError(),
                strMessage.c_str()
            );
            return ERROR_INSTALL_FAILURE;
        }

        if (!FindNextFile (hfdSearch, &fdFileData))
            bFinished = TRUE;
    }


    // Close the search handle and clean up.
    FindClose(hfdSearch);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    CopyAccountFiles
//
// Description: This function is called right after all the files
//              have been copied to the destination folder and before
//              the BOINC daemon is started.  We basically copy all
//              the account files from the source path to the
//              destination path.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CopyAccountFiles(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACopyAccountFiles* pCA = new CACopyAccountFiles(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

