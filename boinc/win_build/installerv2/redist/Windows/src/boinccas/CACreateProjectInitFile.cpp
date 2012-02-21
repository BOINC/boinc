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
    tstring          strDataDirectory;
    tstring          strProjectInitUrl;
    tstring          strProjectInitAuthenticator;
    tstring          strProjectInitTeamName;
    tstring          strProjectInitFile;
    UINT             uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_URL"), strProjectInitUrl );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_AUTH"), strProjectInitAuthenticator );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("PROJINIT_TEAMNAME"), strProjectInitTeamName );
    if ( uiReturnValue ) return uiReturnValue;


    if (!strProjectInitUrl.empty()) {

        // The project_init.xml file is stored in the data directory.
        //
        strProjectInitFile = strDataDirectory + _T("\\project_init.xml");

        FILE* fProjectInitFile = NULL;
        fProjectInitFile = _tfopen(strProjectInitFile.c_str(), _T("w"));
        
        _ftprintf(
            fProjectInitFile,
            _T("<project_init>\n")
            _T("    <name>%s</name>\n")
            _T("    <url>%s</url>\n")
            _T("    <account_key>%s</account_key>\n")
            _T("    <team_name>%s</team_name>\n")
            _T("</project_init>\n"),
            strProjectInitUrl.c_str(),
            strProjectInitUrl.c_str(),
            !strProjectInitAuthenticator.empty() ? strProjectInitAuthenticator.c_str() : _T(""),
            !strProjectInitTeamName.empty() ? strProjectInitTeamName.c_str() : _T("")
        );

        fclose(fProjectInitFile);

    }

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

const char *BOINC_RCSID_01ed9786df="$Id: CACreateProjectInitFile.cpp 11804 2007-01-08 18:42:48Z rwalton $";
