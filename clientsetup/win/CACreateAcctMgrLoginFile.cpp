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
#include "CACreateAcctMgrLoginFile.h"


#define CUSTOMACTION_NAME               _T("CACreateAcctMgrLoginFile")
#define CUSTOMACTION_PROGRESSTITLE      _T("Store account manager initialization data")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateAcctMgrLoginFile::CACreateAcctMgrLoginFile(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACreateAcctMgrLoginFile::~CACreateAcctMgrLoginFile()
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
UINT CACreateAcctMgrLoginFile::OnExecution()
{
    tstring          strDataDirectory;
    tstring          strAcctMgrLogin;
    tstring          strAcctMgrPasswordHash;
    tstring          strAcctMgrLoginFile;
    UINT             uiReturnValue = -1;


    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ACCTMGR_LOGIN"), strAcctMgrLogin );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ACCTMGR_PASSWORDHASH"), strAcctMgrPasswordHash );
    if ( uiReturnValue ) return uiReturnValue;


    if (!strAcctMgrLogin.empty()) {

        // The project_init.xml file is stored in the data directory.
        //
        strAcctMgrLoginFile = strDataDirectory + _T("\\acct_mgr_login.xml");

        FILE* fAcctMgrLoginFile = _tfopen(strAcctMgrLoginFile.c_str(), _T("w"));
        
        _ftprintf(
            fAcctMgrLoginFile,
            _T("<acct_mgr_login>\n")
            _T("    <login>%s</login>\n")
            _T("    <password_hash>%s</password_hash>\n")
            _T("</acct_mgr_login>\n"),
            strAcctMgrLogin.c_str(),
            !strAcctMgrPasswordHash.empty() ? strAcctMgrPasswordHash.c_str() : _T("")
        );

        fclose(fAcctMgrLoginFile);
    }

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    CreateAcctMgrLoginFile
//
// Description: This custom action stores the account manager login data 
//                specified on the commandline in a file in the data
//                directory.
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall CreateAcctMgrLoginFile(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACreateAcctMgrLoginFile* pCA = new CACreateAcctMgrLoginFile(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
