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
#include "project_init.h"
#include "common_defs.h"
#include "CACreateProjectInitFile.h"


#define CUSTOMACTION_NAME               _T("CACreateProjectInitFile")
#define CUSTOMACTION_PROGRESSTITLE      _T("Store project initialization data")


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CACreateProjectInitFile::CACreateProjectInitFile(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
//
// Function:
//
// Description:
//
/////////////////////////////////////////////////////////////////////
CACreateProjectInitFile::~CACreateProjectInitFile()
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
UINT CACreateProjectInitFile::OnExecution()
{
    tstring          strSetupExeName;
    tstring          strDataDirectory;
    tstring          strProjectInitUrl;
    tstring          strProjectInitAuthenticator;
    tstring          project_name;
    PROJECT_INIT     pi;
    UINT             uiReturnValue = 0;

    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_URL"), strProjectInitUrl );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty(_T("PROJINIT_NAME"), project_name);
    if (uiReturnValue) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_AUTH"), strProjectInitAuthenticator );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("SETUPEXENAME"), strSetupExeName );
    if ( uiReturnValue ) return uiReturnValue;

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL,
        NULL,
        NULL,
        NULL,
        _T("Changing to the data directory")
    );
    _tchdir(strDataDirectory.c_str());

    // write project_init.xml if project info was passed on cmdline
    //
    if (!strProjectInitUrl.empty()) {
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL,
            NULL,
            NULL,
            NULL,
            _T("Detected command line parameters")
        );

        pi.init();

        strncpy(pi.url, CW2A(strProjectInitUrl.c_str()), sizeof(pi.url) - 1);
        if (!project_name.empty()) {
            strncpy(pi.name, CW2A(project_name.c_str()), sizeof(pi.name) - 1);
        } else {
            strncpy(pi.name, CW2A(strProjectInitUrl.c_str()), sizeof(pi.name) - 1);
        }

        strncpy(pi.account_key, CW2A(strProjectInitAuthenticator.c_str()), sizeof(pi.account_key)-1);

        pi.embedded = false;

        pi.write();

    }

    // write installer filename to a file
    //
    char filename[256];
    FILE* f = fopen(ACCOUNT_DATA_FILENAME, "w");
    strncpy(filename, CW2A(strSetupExeName.c_str()), sizeof(filename)-1);
    fputs(filename, f);
    fclose(f);

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
//
// Function:    CreateProjectInitFile
//
// Description: This custom action stores the project init data
//                specified on the commandline in a file in the data
//                directory.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CreateProjectInitFile(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACreateProjectInitFile* pCA = new CACreateProjectInitFile(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
