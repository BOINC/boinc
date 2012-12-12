
#include "stdafx.h"
#include "boinccas.h"
#include "launcher.h"


#ifndef SECURITY_MANDATORY_UNTRUSTED_RID

#define SECURITY_MANDATORY_UNTRUSTED_RID            (0x00000000L)
#define SECURITY_MANDATORY_LOW_RID                  (0x00001000L)
#define SECURITY_MANDATORY_MEDIUM_RID               (0x00002000L)
#define SECURITY_MANDATORY_HIGH_RID                 (0x00003000L)
#define SECURITY_MANDATORY_SYSTEM_RID               (0x00004000L)
#define SECURITY_MANDATORY_PROTECTED_PROCESS_RID    (0x00005000L)

typedef struct _TOKEN_MANDATORY_LABEL {
    SID_AND_ATTRIBUTES Label;
} TOKEN_MANDATORY_LABEL, *PTOKEN_MANDATORY_LABEL;

typedef enum _MANDATORY_LEVEL {
    MandatoryLevelUntrusted = 0,
    MandatoryLevelLow,
    MandatoryLevelMedium,
    MandatoryLevelHigh,
    MandatoryLevelSystem,
    MandatoryLevelSecureProcess,
    MandatoryLevelCount
} MANDATORY_LEVEL, *PMANDATORY_LEVEL;

#define TokenVirtualizationEnabled ((TOKEN_INFORMATION_CLASS)24)
#define TokenIntegrityLevel ((TOKEN_INFORMATION_CLASS)25)

#endif //!SECURITY_MANDATORY_UNTRUSTED_RID

/*!
@brief Function enables/disables/removes a privelege associated with the given token
@detailed Calls LookupPrivilegeValue() and AdjustTokenPrivileges()
@param[in] hToken - access token handle
@param[in] lpszPrivilege - name of privilege to enable/disable
@param[in] dwAttributes - (SE_PRIVILEGE_ENABLED) to enable or (0) disable or (SE_PRIVILEGE_REMOVED) to remove privilege
@return HRESULT code
@todo Removing was checked. To check enabling and disabling.
*/
inline HRESULT SetPrivilege(
		  HANDLE hToken,          
		  LPCTSTR lpszPrivilege, 
		  DWORD dwAttributes=SE_PRIVILEGE_ENABLED   
		  ) 
{
	HRESULT hr=S_OK;
	LUID luid;

	if ( LookupPrivilegeValue( 
			NULL,            // lookup privilege on local system
			lpszPrivilege,   // privilege to lookup 
			&luid ) )        // receives LUID of privilege
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = dwAttributes;

		// Enable the privilege or disable all privileges.

		if ( !AdjustTokenPrivileges(
				hToken, 
				FALSE, 
				&tp, 
				sizeof(TOKEN_PRIVILEGES), 
				(PTOKEN_PRIVILEGES) NULL, 
				(PDWORD) NULL) )
			hr=HRESULT_FROM_WIN32(GetLastError());
	}//if(LookupPrivilegeValue(...))
	else
		hr=HRESULT_FROM_WIN32(GetLastError());

	return hr;
}

/*!
Function removes the priveleges which are not associated by default with explorer.exe at Medium Integration Level in Vista
@returns HRESULT of the operation on SE_CREATE_GLOBAL_NAME (="SeCreateGlobalPrivilege")
*/
inline HRESULT ReducePrivilegesForMediumIL(HANDLE hToken) 
{
	HRESULT hr=S_OK;
	hr=SetPrivilege(hToken, SE_CREATE_GLOBAL_NAME, SE_PRIVILEGE_REMOVED);

	SetPrivilege(hToken, SE_BACKUP_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_CREATE_PAGEFILE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, TEXT("SeCreateSymbolicLinkPrivilege"), SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_DEBUG_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_IMPERSONATE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_INC_BASE_PRIORITY_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_INCREASE_QUOTA_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_LOAD_DRIVER_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_MANAGE_VOLUME_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_PROF_SINGLE_PROCESS_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_REMOTE_SHUTDOWN_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_RESTORE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SECURITY_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SYSTEM_ENVIRONMENT_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SYSTEM_PROFILE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SYSTEMTIME_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, SE_PRIVILEGE_REMOVED);

	return hr;
}

/*!
@brief Gets Integration level of the given process in Vista. 
In the older OS assumes the integration level is equal to SECURITY_MANDATORY_HIGH_RID

The function opens the process for all access and opens its token for all access. 
Then it extracts token information and closes the handles.
@param[in] dwProcessId ID of the process to operate
@param[out] pdwProcessIL pointer to write the value
@return HRESULT
@retval <return value> { description }
@remarks Function check for OS version by querying the presence of Kernel32.GetProductInfo function. 
This way is used due to the function is called from InstallShield12 script, so GetVersionEx returns incorrect value.
@todo restrict access rights when quering for tokens
*/
inline HRESULT GetProcessIL(DWORD dwProcessId, LPDWORD pdwProcessIL)
{
	HRESULT hr=S_OK;
	if(!pdwProcessIL)
		hr=E_INVALIDARG;
	if(SUCCEEDED(hr))
	{
		bool bVista=false;
		{
			// When the function is called from IS12, GetVersionEx returns dwMajorVersion=5 on Vista!
			HMODULE hmodKernel32=LoadLibrary(L"Kernel32");
			if(hmodKernel32 && GetProcAddress(hmodKernel32, "GetProductInfo"))
				bVista=true;
			if(hmodKernel32) FreeLibrary(hmodKernel32);
		}

		DWORD dwIL=SECURITY_MANDATORY_HIGH_RID;
		if(bVista)
		{//Vista
			HANDLE hToken=NULL;
			HANDLE hProcess=OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if(hProcess)
			{
				if(OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
				{
					PTOKEN_MANDATORY_LABEL pTIL=NULL;
					DWORD dwSize=0;
					if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwSize) 
						&& ERROR_INSUFFICIENT_BUFFER==GetLastError() && dwSize)
						pTIL=(PTOKEN_MANDATORY_LABEL)HeapAlloc(GetProcessHeap(), 0, dwSize);

					if(pTIL && GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize, &dwSize))
					{
						LPBYTE lpb=GetSidSubAuthorityCount(pTIL->Label.Sid);
						if(lpb)
							dwIL = *GetSidSubAuthority(pTIL->Label.Sid, *lpb-1);
						else
							hr=E_UNEXPECTED;
					}
					if(pTIL)
						HeapFree(GetProcessHeap(), 0, pTIL);
					CloseHandle(hToken);
				}//if(OpenProcessToken(...))
				CloseHandle(hProcess);
			}//if(hProcess)
		}//if(bVista)
		if(SUCCEEDED(hr))
			*pdwProcessIL=dwIL;
	}//if(SUCCEEDED(hr))
	return hr;
}

/*!
@brief Function launches process with the integration level of Explorer on Vista. On the previous OS, simply creates the process.

Function gets the integration level of the current process and Explorer, then launches the new process.
If the integration levels are equal, CreateProcess is called. 
If Explorer has Medium IL, and the current process has High IL, new token is created, its rights are adjusted 
and CreateProcessWithTokenW is called. 
If Explorer has Medium IL, and the current process has High IL, error is returned.
@param[in] szProcessName - the name of exe file (see CreatePorcess()) 
@param[in] szCmdLine - the name of exe file (see CreatePorcess())
@return HRESULT code
@note The function cannot be used in services, due to if uses USER32.FindWindow() to get the proper instance of Explorer. 
The parent of new process in taskmgr.exe, but not the current process.
@sa ReducePrivilegesForMediumIL()
*/
HRESULT CreateProcessWithExplorerIL(LPWSTR szProcessName, LPWSTR szCmdLine)
{
    HRESULT hr = S_OK;
    BOOL bRet;
    PROCESS_INFORMATION ProcInfo = {0};
    STARTUPINFO StartupInfo = {0};
    bool bVista = false;

	// When the function is called from IS12, GetVersionEx returns dwMajorVersion=5 on Vista!
	HMODULE hmodKernel32 = LoadLibrary(_T("Kernel32"));
    if (hmodKernel32) {
        if (GetProcAddress(hmodKernel32, "GetProductInfo")) {
            bVista = true;
        }
        FreeLibrary(hmodKernel32);
    }
	
	if(bVista)
	{
	    HANDLE hToken;
	    HANDLE hNewToken;
		DWORD dwCurIL = SECURITY_MANDATORY_HIGH_RID, dwExplorerIL = SECURITY_MANDATORY_HIGH_RID; 
		DWORD dwExplorerID = 0;
        DWORD dwEnableVirtualization = 0;

		HWND hwndShell = ::FindWindow( _T("Progman"), NULL);
		if(hwndShell) GetWindowThreadProcessId(hwndShell, &dwExplorerID);
		
		GetProcessIL(dwExplorerID, &dwExplorerIL);
		GetProcessIL(GetCurrentProcessId(), &dwCurIL);

        if( (dwCurIL == SECURITY_MANDATORY_HIGH_RID) && (dwExplorerIL == SECURITY_MANDATORY_MEDIUM_RID) )
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwExplorerID);
			if(hProcess)
			{
				if(OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
				{
                    if(!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hNewToken)) {
						hr = HRESULT_FROM_WIN32(GetLastError());
                    }

					if(SUCCEEDED(hr)) {

						hr = ReducePrivilegesForMediumIL(hNewToken);

                        SetTokenInformation(
                            hNewToken,
                            TokenVirtualizationEnabled,
                            &dwEnableVirtualization,
                            sizeof(DWORD)
                        );

                        if(SUCCEEDED(hr)) {
							bRet = CreateProcessAsUser(
                                hNewToken,
                                szProcessName,
                                szCmdLine,
                                NULL,
                                NULL,
                                FALSE,
                                NORMAL_PRIORITY_CLASS,
                                NULL,
                                NULL,
                                &StartupInfo,
                                &ProcInfo
                            );
                            if(!bRet) {
								hr = HRESULT_FROM_WIN32(GetLastError());
                            }
						}
                        CloseHandle(hNewToken);
                    } else {
						hr = HRESULT_FROM_WIN32(GetLastError());
                    }
					CloseHandle(hToken);
                } else {
					hr = HRESULT_FROM_WIN32(GetLastError());
                }
				CloseHandle(hProcess);
			}
		} else if ((dwCurIL == SECURITY_MANDATORY_MEDIUM_RID) && (dwExplorerIL == SECURITY_MANDATORY_HIGH_RID)) {
			hr = E_ACCESSDENIED;
		}
	}

	if(!ProcInfo.dwProcessId) {
		bRet = CreateProcess(
            szProcessName,
            szCmdLine, 
			NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &StartupInfo,
            &ProcInfo
        );
        if(!bRet) {
			hr = HRESULT_FROM_WIN32(GetLastError());
        }
	}

	return hr;
}


