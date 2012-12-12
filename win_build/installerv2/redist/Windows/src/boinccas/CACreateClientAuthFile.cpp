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
#include "CACreateClientAuthFile.h"


#define CUSTOMACTION_NAME               _T("CACreateClientAuthFile")
#define CUSTOMACTION_PROGRESSTITLE      _T("Store client authorization data")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateClientAuthFile::CACreateClientAuthFile(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateClientAuthFile::~CACreateClientAuthFile()
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
UINT CACreateClientAuthFile::OnExecution()
{
    tstring          strDataDirectory;
    tstring          strEnableProtectedApplicationExecution;
    tstring          strBOINCProjectAccountUsername;
    tstring          strBOINCProjectAccountPassword;
    tstring          strClientAuthFile;
    tstring          strVersionNT;
    struct _stat     buf;
    TCHAR                   szMessage[2048];
    UINT             uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ENABLEPROTECTEDAPPLICATIONEXECUTION2"), strEnableProtectedApplicationExecution );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECT_ISUSERNAME"), strBOINCProjectAccountUsername );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("BOINC_PROJECT_PASSWORD"), strBOINCProjectAccountPassword );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("VersionNT"), strVersionNT );
    if ( uiReturnValue ) return uiReturnValue;


    // The client_auth.xml file is stored in the data directory.
    //
    strClientAuthFile = strDataDirectory + _T("\\client_auth.xml");

    // If we are not installing in protected mode, there may not
    //   be a valid 'boinc_project' account, so delete the
    //   client_auth.xml file if it exists.
    //
    // NOTE: Windows 2000 or older requires the SeTcbPrivilege
    //   user right, which makes the account the equiv of an
    //   administrator on the system. Disable the use of
    //   'boinc_project' on Windows 2000 or older
    //
    if ((_T("1") != strEnableProtectedApplicationExecution) || _T("500") >= strVersionNT)
    {
        if (0 == _tstat(strClientAuthFile.c_str(), &buf))
        {
            if (DeleteFile(strClientAuthFile.c_str()))
            {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("The client_auth.xml file was successfully deleted.")
                );
            }
            else
            {
                LogMessage(
                    INSTALLMESSAGE_FATALEXIT,
                    NULL, 
                    NULL,
                    NULL,
                    NULL,
                    _T("The client_auth.xml could not be deleted from the data direvtory. ")
                    _T("Please delete the file and rerun setup.")
                );
                return ERROR_INSTALL_FAILURE;
            }
        }
    }
    else
    {
        // We are installing in protected mode, which means the 'boinc_project'
        //   account password has been changed, so we need to write out the new
        //   username and password to the client_auth.xml file.
        DWORD dwSize = Base64EncodeGetRequiredLength((int)strBOINCProjectAccountPassword.length());
        LPSTR szBuffer = (LPSTR)malloc(dwSize*sizeof(TCHAR));
        if (!szBuffer)
        {
            LogMessage(
                INSTALLMESSAGE_FATALEXIT,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Not enough memory could be allocated to complete the requested action. ")
                _T("Please shutdown any running applications or reboot the computer and rerun ")
                _T("setup to complete installation.")
            );
            return ERROR_INSTALL_FAILURE;
        }
        memset(szBuffer, '\0', (dwSize*sizeof(TCHAR)));

        // Base 64 encode the 'boinc_project' account password
        //
        CW2A pszASCIIDecodedPassword( strBOINCProjectAccountPassword.c_str() );
        if (!Base64Encode(
                (const BYTE*)((LPSTR)pszASCIIDecodedPassword),
                (int)strlen(pszASCIIDecodedPassword),
                szBuffer,
                (int*)&dwSize,
                0)
            )
		{
            LogMessage(
                INSTALLMESSAGE_FATALEXIT,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("The 'boinc_project' account password failed to be encoded.")
            );
            return ERROR_INSTALL_FAILURE;
		}
        CA2W pszUnicodeEncodedPassword( szBuffer );

        _sntprintf(
            szMessage, 
            sizeof(szMessage),
            _T("(Unicode) Base64 Encoded String: '%s'"),
            pszUnicodeEncodedPassword.m_psz
        );
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            szMessage
        );

        FILE* fClientAuthFile = NULL;
        fClientAuthFile = _tfopen(strClientAuthFile.c_str(), _T("w"));
        
        _ftprintf(
            fClientAuthFile,
            _T("<client_authorization>\n")
            _T("    <boinc_project>\n")
            _T("        <username>%s</username>\n")
            _T("        <password>%s</password>\n")
            _T("    </boinc_project>\n")
            _T("</client_authorization>\n"),
            strBOINCProjectAccountUsername.c_str(),
            pszUnicodeEncodedPassword.m_psz
        );

        fclose(fClientAuthFile);
        free(szBuffer);
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    CreateClientAuthFile
//
// Description: This custom action stores the 'boinc_project' account
//                information in the client_auth.xml file.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CreateClientAuthFile(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACreateClientAuthFile* pCA = new CACreateClientAuthFile(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}

const char *BOINC_RCSID_01ed9786df="$Id: CACreateClientAuthFile.cpp 11804 2007-01-08 18:42:48Z rwalton $";
