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
#include <iostream>
#include <fstream>

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


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CACCConfigMd5sum::OnExecution()
{
	std::string strInstallDirectory;
	std::string strLocation;
	std::ifstream ccFile;
	char contents[8196];
    UINT    uiReturnValue = 0;
    TCHAR   szMessage[2048];

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    SetProperty(_T("CCCONFIGMD5SUM"), _T("0"));

    strLocation = strInstallDirectory + _T("\\cc_config.xml");

	ccFile.open(strLocation.c_str()); // open for reading
	if ( ccFile.bad() ) {
		_sntprintf(szMessage,sizeof(szMessage),_T("No file found at: '%s'"),strLocation);
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		SetProperty(_T("CCCONFIGMD5SUM"), _T("1"));
		return ERROR_SUCCESS;
	}

	ccFile.read((char*) &contents,8192);
	if ( ccFile.bad() ) {
		_sntprintf(szMessage,sizeof(szMessage),_T("Error reading file at: '%s'"),strLocation);
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		return ERROR_FILE_INVALID;
	}
	if ( !ccFile.eof() ) {
		_sntprintf(szMessage,sizeof(szMessage),_T("File is more than 8kb: '%s'"),strLocation);
		LogMessage(INSTALLMESSAGE_INFO,NULL, NULL,NULL,NULL,szMessage);
		return ERROR_SUCCESS;
	}

	ccFile.close();  
	// done with file, now to get md5sum

	if ( !ComputeDigest(contents) ) {
		return ERROR_FUNCTION_FAILED;
	}

    return ERROR_SUCCESS;
}

bool CACCConfigMd5sum::ComputeDigest(char *contents)
{
    HCRYPTPROV provider; 
    HCRYPTHASH hash; 
    BYTE bHash[0x7f]; 
    DWORD hashSize= 16;
    TCHAR   szMessage[2048];
	std::string cc510="03f2dde88c69fd548df12c43da18e31e";

    if(CryptAcquireContext(&provider,NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) {
		if(CryptCreateHash(provider,CALG_MD5,0, 0, &hash)) {
			if(CryptHashData(hash, (BYTE *) contents, (DWORD) strlen(contents), 0)) {
				if(CryptGetHashParam(hash, HP_HASHVAL, bHash, &hashSize, 0)) {
					// Make a string version of the numeric digest value
					char tmp;
					for (int i = 0; i<16; i++) {
						sprintf(&tmp,"%02x",bHash[i]);
						if ( tmp != cc510.at(i) ) {
							return true;
						}
					}
				} else {
		 			_sntprintf(szMessage,sizeof(szMessage),_T("Error getting hash param"));
					LogMessage(INSTALLMESSAGE_ERROR,NULL, NULL,NULL,NULL,szMessage);
					return false;
				}
			} else {
				_sntprintf(szMessage,sizeof(szMessage),_T("Error hashing data"));
				LogMessage(INSTALLMESSAGE_ERROR,NULL, NULL,NULL,NULL,szMessage);
				return false;
			}
		} else {
			_sntprintf(szMessage,sizeof(szMessage),_T("Error creating hash"));
			LogMessage(INSTALLMESSAGE_ERROR,NULL, NULL,NULL,NULL,szMessage);
			return false;
		}
	} else {
		_sntprintf(szMessage,sizeof(szMessage),_T("Error acquiring context"));
		LogMessage(INSTALLMESSAGE_ERROR,NULL, NULL,NULL,NULL,szMessage);
		return false;
	}

	SetProperty(_T("CCCONFIGMD5SUM"), _T("1"));
    CryptDestroyHash(hash); 
    CryptReleaseContext(provider, 0); 
    return true; 
}

UINT __stdcall CCConfigMd5sum(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CACCConfigMd5sum* pCA = new CACCConfigMd5sum(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}
