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
#include "CACCConfigMd5sum.h"
#include <stdio.h>
#include <ctype.h>

#define CUSTOMACTION_NAME               _T("CACCConfigMd5sum")
#define CUSTOMACTION_PROGRESSTITLE      _T("Obtain the md5sum of the current cc_config.xml file.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACCConfigMd5sum::CACCConfigMd5sum(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CACCConfigMd5sum::~CACCConfigMd5sum()
{
    BOINCCABase::~BOINCCABase();
}

void remove_white(char *contents) {
	int i=0,j=0;
	while ( contents[j] != '\0' ) {
		if ( isspace(contents[j]) ) {
			j++;
			continue;
		}
		contents[i++]=contents[j++];
	}
	contents[i]='\0';
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CACCConfigMd5sum::OnExecution()
{
	std::string strDataDirectory;
	std::string strInstallDirectory;
	std::string strLocation;
	char contents[8196];
    UINT    uiReturnValue = 0;
    TCHAR   szMessage[16392];
	FILE *file;
	memset(&contents,'\0',sizeof(contents));

	SetProperty(_T("KEEPEXISTINGCONFIG"), _T("0"));
    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;
	if (strInstallDirectory.rfind('\\') != strInstallDirectory.size() - 1 ) {
		strInstallDirectory = strInstallDirectory + "\\";
		_sntprintf(szMessage,sizeof(szMessage),_T("Added trailing slash to install dir"),strLocation.c_str());
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
	}

    uiReturnValue = GetProperty( _T("DATADIR"), strDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;
	if (strDataDirectory.rfind('\\') != strDataDirectory.size() - 1 ) {
		strDataDirectory = strDataDirectory + "\\";
		_sntprintf(szMessage,sizeof(szMessage),_T("Added trailing slash to data dir"),strLocation.c_str());
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
	}

	// Figure out of the file is in the data directory or the install directory
	// it should be in the data directory, but if it isn't we will check the install directory
    strLocation = strDataDirectory + _T("cc_config.xml");
	file = fopen(strLocation.c_str(),"r");
	if ( file == NULL ) {
		_sntprintf(szMessage,sizeof(szMessage),_T("fopen: Error Message '%s': '%s'"),strerror(errno),strLocation.c_str());
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		strLocation = strInstallDirectory + _T("cc_config.xml");
		file = fopen(strLocation.c_str(),"r");
		if ( file == NULL ) {
			_sntprintf(szMessage,sizeof(szMessage),_T("fopen: Error Message '%s': '%s'"),strerror(errno),strLocation.c_str());
			LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
			return ERROR_SUCCESS;
		} else {
			_sntprintf(szMessage,sizeof(szMessage),_T("fopen: File found at: '%s'"),strLocation.c_str());
			LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		}
	} else {
		_sntprintf(szMessage,sizeof(szMessage),_T("fopen: File found at: '%s'"),strLocation.c_str());
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
	}

	int bytesread = (int) fread(&contents, sizeof(char), sizeof(contents), file);
	if ( bytesread > 0 && bytesread < sizeof(char) ) {
		contents[bytesread]='\0';
	}

	if ( !feof(file) ) {
		_sntprintf(szMessage,sizeof(szMessage),_T("File is more than 8kb: '%s'"),strLocation);
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		fclose(file);
		return ERROR_SUCCESS;
	}

	if ( ferror(file) ) {
		_sntprintf(szMessage,sizeof(szMessage),_T("Error reading file at: '%s'"),strLocation);
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		return ERROR_FILE_INVALID;
	}

	fclose(file);

	remove_white(contents);

	_sntprintf(szMessage,sizeof(szMessage),_T("File contents: '%s'"),contents);
	LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);

	if ( CheckFile(contents) ) {
		if ( remove(strLocation.c_str()) ) {
			_sntprintf(szMessage,sizeof(szMessage),_T("Failed to delete CCConfig version from previous install.  Error Message '%s'"),strerror(errno));
			LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
			return ERROR_FAIL_I24;
		}
		_sntprintf(szMessage,sizeof(szMessage),_T("CCConfig was a version from previous install.  Deleted"));
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		return ERROR_SUCCESS;
	}

	_sntprintf(szMessage,sizeof(szMessage),_T("CCConfig was not a version from previous install"));
	SetProperty(_T("KEEPEXISTINGCONFIG"), _T("1"));
	LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
    return ERROR_SUCCESS;
}

bool CACCConfigMd5sum::CheckFile(char *contents)
{
	bool match = false;
	char *cc510="<cc_config><log_flags></log_flags><options><dont_contact_ref_site>1</dont_contact_ref_site></options></cc_config>";
	if ( strstr(contents, cc510) ) match = true;
	char *cc602="<cc_config><log_flags></log_flags><options><version_check_server>www.worldcommunitygrid.org</version_check_server><proxy_test_server>www.ibm.com</proxy_test_server><start_delay>120</start_delay></options></cc_config>";
	if ( strstr(contents, cc602) ) match = true;
    return match; 
}

UINT __stdcall CCConfigMd5sum(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACCConfigMd5sum* pCA = new CACCConfigMd5sum(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
