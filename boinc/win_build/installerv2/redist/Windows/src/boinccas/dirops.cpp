// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
//
// Source Code Originally from:
// http://support.microsoft.com/kb/814463
//

#include "stdafx.h"
#include "dirops.h"

/////////////////////////////////////////////////////////////////////////////////////
//    FUNCTION:    RemoveReadOnly
//    DESCRIPTION:  Removes the write protection of a specific file (including full path), 
//                if case the file exists.
//
//    RETURN:          TRUE for success, FALSE for failure
//
/////////////////////////////////////////////////////////////////////////////////////
BOOL RemoveReadOnly(tstring& csFileName)
{
        DWORD dwFileAttributes =  GetFileAttributes(csFileName.c_str());
        
        // In case the file does not exist
        if (dwFileAttributes == -1)
            return FALSE;

        dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
        if (SetFileAttributes(csFileName.c_str(), dwFileAttributes) != 0)
            return TRUE;
        return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
//    FUNCTION:    CopyFolder
//    DESCRIPTION: Copies a directory to a new location
//
//    RETURN:         TRUE for success, FALSE for failure
//
/////////////////////////////////////////////////////////////////////////////////////
BOOL RecursiveCopyFolder(tstring& csPath, tstring& csNewPath)
{
    BOOL bRet = TRUE;

    if( !CreateDirectory(csNewPath.c_str(), NULL))
        bRet = FALSE;
    
    tstring csPathMask;
    tstring csFullPath;
    tstring csNewFullPath;
    
    csPath     += _T("\\");
    csNewPath  += _T("\\");
    
    csPathMask = csPath + _T("*.*");
        
    WIN32_FIND_DATA ffData;
    HANDLE hFind;
    hFind = FindFirstFile(csPathMask.c_str(), &ffData);

    if (hFind == INVALID_HANDLE_VALUE){
        return FALSE;
    }
    

    // Copying all the files
    while (hFind && FindNextFile(hFind, &ffData)) 
    {
        csFullPath    = csPath    + ffData.cFileName;
        csNewFullPath = csNewPath + ffData.cFileName;

        RemoveReadOnly(csNewFullPath);

        if( !(ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
        {
            if( !CopyFile(csFullPath.c_str(), csNewFullPath.c_str(), FALSE) ) 
            {
                // Not stopping here, trying to copy the rest of the files
                bRet = FALSE;
            }
        }
        else // it is a directory -> Copying recursivly
        { 
            if( (_tcscmp(ffData.cFileName, _T(".")) != 0) &&
                (_tcscmp(ffData.cFileName, _T("..")) != 0) ) 
            {
                if( !RecursiveCopyFolder(csFullPath, csNewFullPath) )
                {
                    // Not stopping here, trying to copy the rest of the files
                    bRet = FALSE;
                }
            }
        }
    }

    FindClose(hFind);
    return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////
//	FUNCTION:    RecursiveDeleteFolder
//	DESCRIPTION: Deletes a directory, even if not empty
//
//	RETURN:		 TRUE for success, FALSE for failure
//
/////////////////////////////////////////////////////////////////////////////////////
BOOL RecursiveDeleteFolder(tstring& csPath)
{
	BOOL bRet = TRUE;

	tstring csPathMask;
	tstring csFullPath;
	
	csPath     += _T("\\");
	csPathMask = csPath + _T("*.*");
		
	WIN32_FIND_DATA ffData;
	HANDLE hFind;
	hFind = FindFirstFile(csPathMask.c_str(), &ffData);

	if (hFind == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	

	// Delete all the files
	while (hFind && FindNextFile(hFind, &ffData)) 
	{
		csFullPath    = csPath    + ffData.cFileName;

		RemoveReadOnly(csFullPath);

		if( !(ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
		{
			if( !DeleteFile(csFullPath.c_str()) ) 
			{
				// Not stopping here, trying to copy the rest of the files
				bRet = FALSE;
			}
		}
		else // it is a directory -> Copying recursivly
		{ 
			if( (_tcscmp(ffData.cFileName, _T(".")) != 0) &&
				(_tcscmp(ffData.cFileName, _T("..")) != 0) ) 
			{
				if( !RecursiveDeleteFolder(csFullPath) )
				{
					// Not stopping here, trying to copy the rest of the files
					bRet = FALSE;
				}
			}
		}
	}

    FindClose(hFind);
	
	RemoveReadOnly(csPath);
	if( !RemoveDirectory(csPath.c_str()))
		bRet = FALSE;

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////
//	FUNCTION:    MoveFolder
//	DESCRIPTION: Moves a directory to a new location
//
//	RETURN:		 TRUE for success, FALSE for failure
//
/////////////////////////////////////////////////////////////////////////////////////
BOOL MoveFolder(tstring& csPath, tstring& csNewPath)
{
	BOOL bRet = TRUE;
	
	if(!RecursiveCopyFolder(csPath, csNewPath))
		bRet = FALSE;

	if(!RecursiveDeleteFolder(csPath))
		bRet = FALSE;

	return bRet;
}


/////////////////////////////////////////////////////////////////////////////////////
//    FUNCTION:    RecursiveSetPermissions
//    DESCRIPTION: Copies a directory to a new location
//
//    RETURN:         TRUE for success, FALSE for failure
//
/////////////////////////////////////////////////////////////////////////////////////
BOOL RecursiveSetPermissions(tstring& csPath, PACL pACL)
{
    BOOL bRet = TRUE;

    tstring csPathMask;
    tstring csFullPath;
    
    csPath     += _T("\\");
    csPathMask = csPath + _T("*.*");
        
    WIN32_FIND_DATA ffData;
    HANDLE hFind;
    hFind = FindFirstFile(csPathMask.c_str(), &ffData);

    if (hFind == INVALID_HANDLE_VALUE){
        return FALSE;
    }
    

    // Copying all the files
    while (hFind && FindNextFile(hFind, &ffData)) 
    {
        if( (_tcscmp(ffData.cFileName, _T(".")) != 0) &&
            (_tcscmp(ffData.cFileName, _T("..")) != 0) ) 
        {
            csFullPath = csPath + ffData.cFileName;

            // Set the ACL on the file.
            SetNamedSecurityInfo( 
#ifdef _UNICODE
                (LPWSTR)csFullPath.c_str(),
#else
                (LPSTR)csFullPath.c_str(),
#endif
                SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION,
                NULL,
                NULL,
                pACL,
                NULL
            );

            if( ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
            {
                    RecursiveSetPermissions(csFullPath, pACL);
            }
        }
    }

    FindClose(hFind);
    return bRet;
}

