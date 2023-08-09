// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
//
// Source Code Originally from:
// http://support.microsoft.com/kb/814463
//

#include "stdafx.h"
#include "password.h"

//Generates a Random string of length nLen - 1.  Buffer ppwd must allocate an extra character for null terminator.
//Returns TRUE if successful, FALSE if fails.
//Extended error information can be obtained from GetLastError().
BOOL GenPwd(TCHAR* ppwd, int nLen)
{
	BOOL bResult = FALSE;	//assume failure
	HCRYPTPROV hProv = NULL;
	HCRYPTHASH hHash = NULL;

	//Storage for random string 4 times longer than the resulting password.
	DWORD dwBufSize = nLen*4;
	DWORD dwSize = Base64EncodeGetRequiredLength((int)dwBufSize);
	LPSTR pEncodedString = NULL;
	LPBYTE pRandomBuf = NULL;
	TCHAR* pTRandomPwd = NULL;

	try
	{
		pEncodedString = new char[dwSize];
		pRandomBuf = new BYTE[dwBufSize];

		// Try to acquire context to Crypto provider.
		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_SILENT))
		{
			if (GetLastError() == NTE_BAD_KEYSET) //Test for non-existent keyset
			{
				if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_SILENT | CRYPT_NEWKEYSET))
					throw(GetLastError());
			}
			else
				throw(GetLastError());
		}

		//Generate a random sequence.
		if (!CryptGenRandom(hProv, dwBufSize, pRandomBuf))
		{
			throw(GetLastError());
		}

		//Get a handle to a hash, then hash the random stream.
		if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
		{
			throw(GetLastError());
		}

		if (!CryptHashData(hHash, pRandomBuf, dwBufSize, NULL))
		{
			throw(GetLastError());
		}

		//Destroy the hash object.
		CryptDestroyHash(hHash);
		//Release Provider context
		CryptReleaseContext(hProv, 0);

		//Encode the hash value to base64.
		if (!Base64Encode(pRandomBuf, dwBufSize, pEncodedString, (int*) &dwSize, 0))
		{
			throw(GetLastError());
		}

		//Determine how many tchars you need to convert string to base64.
		int nTchars = (int) strlen(pEncodedString);

		pTRandomPwd = new TCHAR[nTchars];

#ifdef UNICODE
		if (MultiByteToWideChar(CP_UTF8, 0, pEncodedString, nTchars, pTRandomPwd, nTchars) == 0)
		{
			throw(GetLastError());
		}
#else
		_tcsncpy( pTRandomPwd, pEncodedString, nLen);
#endif

		//Copy the first x characters of random string to output buffer.
		_tcsncpy(ppwd, pTRandomPwd, nLen);
		//Add null terminator to ppwd string.
		ppwd[nLen] = _T('\0');

		bResult = TRUE;

	}
	catch (DWORD)
	{
		//Set return value to false.
		bResult = FALSE;
	}
	catch (...)
	{
		//Unknown error, throw.
		throw;
	}

	//Clean up memory.
	if (pRandomBuf)
	{
		delete[] pRandomBuf;
		pRandomBuf = NULL;
	}

	if (pEncodedString)
	{
		delete[] pEncodedString;
		pEncodedString = NULL;
	}

	if (pTRandomPwd)
	{
		delete[] pTRandomPwd;
		pTRandomPwd = NULL;
	}

	return bResult;
}


BOOL GenerateRandomPassword( tstring& strPassword, DWORD dwDesiredLength )
{
    TCHAR szBuffer[512];
    BOOL  bReturnValue = FALSE;

    bReturnValue = GenPwd(szBuffer, dwDesiredLength);

    strPassword = szBuffer;

    return bReturnValue;
}

