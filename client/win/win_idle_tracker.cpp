
/**
 * IdleTracker - a DLL that tracks the user's idle input time
 *               system-wide.
 *
 * Usage
 * =====
 * - call IdleTrackerInit() when you want to start monitoring.
 * - call IdleTrackerTerm() when you want to stop monitoring.
 * - to get the time past since last user input, do the following:
 *    GetTickCount() - IdleTrackerGetLastTickCount()
 *
 * Author: Sidney Chong
 * Date: 25/5/2000
 * Version: 1.0
 **/

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400           // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0400   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 5.01 or later.
#define _WIN32_IE 0x0501        // Change this to the appropriate value to target IE 6.0 or later.
#endif

#include <windows.h>
#include <crtdbg.h>

/**
 * The following global data is only shared in this instance of the DLL
 **/ 
HINSTANCE g_hInstance = NULL;   // global instance handle
HMODULE   g_hUser32 = NULL;
HANDLE    g_hMemoryMappedData = NULL;
BOOL      g_bIsWindows2000Compatible = FALSE;
BOOL      g_bIsTerminalServicesEnabled = FALSE;

/**
 * The following global data is SHARED among all instances of the DLL
 * (processes) within a terminal services session.
 **/ 
#pragma data_seg(".IdleTrac")	// you must define as SHARED in .def
HHOOK 	g_hHkKeyboard = NULL;	// handle to the keyboard hook
HHOOK 	g_hHkMouse = NULL;	    // handle to the mouse hook
LONG	g_mouseLocX = -1;	    // x-location of mouse position
LONG	g_mouseLocY = -1;	    // y-location of mouse position
DWORD	g_dwLastTick = 0;       // tick time of last input event
#pragma data_seg()
#pragma comment(linker, "/section:.IdleTrac,rws")

/**
 * The following global data is SHARED among all instances of the DLL
 * (processes); i.e., these are system-wide globals.
 **/
struct SystemWideIdleData
{
	DWORD	dwLastTick;         // tick time of last input event
};

struct SystemWideIdleData* g_pSystemWideIdleData = NULL;

/**
 * Define stuff that only exists on Windows 2000 compatible machines
 **/
typedef struct tagLASTINPUTINFO {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO, *PLASTINPUTINFO;

typedef BOOL (WINAPI *GETLASTINPUTINFO)(PLASTINPUTINFO);

GETLASTINPUTINFO g_fnGetLastInputInfo = NULL;

/**
 * Find out if we are on a Windows 2000 compatible system
 **/
BOOL IsWindows2000Compatible()
{
   OSVERSIONINFO osvi;
   ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
        return FALSE;

    return (osvi.dwMajorVersion >= 5);
}

/**
 * This function compares the passed in "suite name" string
 * to the product suite information stored in the registry.
 * This only works on the Terminal Server 4.0 platform.
 **/
BOOL ValidateProductSuite (LPSTR SuiteName)
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPSTR ProductSuite = NULL;
    LPSTR p;

    Rslt = RegOpenKeyA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\ProductOptions",
        &hKey
        );

    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type, NULL, &Size );
    if (Rslt != ERROR_SUCCESS || !Size)
        goto exit;

    ProductSuite = (LPSTR) LocalAlloc( LPTR, Size );
    if (!ProductSuite)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type,
        (LPBYTE) ProductSuite, &Size );
     if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    p = ProductSuite;
    while (*p)
    {
        if (lstrcmpA( p, SuiteName ) == 0)
        {
            rVal = TRUE;
            break;
        }
        p += (lstrlenA( p ) + 1);
    }

exit:
    if (ProductSuite)
        LocalFree( ProductSuite );

    if (hKey)
        RegCloseKey( hKey );

    return rVal;
}

/**
 * This function performs the basic check to see if
 * the platform on which it is running is Terminal
 * services enabled.  Note, this code is compatible on
 * all Win32 platforms.  For the Windows 2000 platform
 * we perform a "lazy" bind to the new product suite
 * APIs that were first introduced on that platform.
 **/
BOOL IsTerminalServicesEnabled()
{
    BOOL    bResult = FALSE;    // assume Terminal Services is not enabled

    DWORD   dwVersion;
    OSVERSIONINFOEXA osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    HMODULE hmodK32 = NULL;
    HMODULE hmodNtDll = NULL;
    typedef ULONGLONG (WINAPI *PFnVerSetConditionMask)(ULONGLONG,ULONG,UCHAR);
    typedef BOOL (WINAPI *PFnVerifyVersionInfoA)(POSVERSIONINFOEXA, DWORD, DWORDLONG);
    PFnVerSetConditionMask pfnVerSetConditionMask;
    PFnVerifyVersionInfoA pfnVerifyVersionInfoA;

    dwVersion = GetVersion();

    // are we running NT ?
    if (!(dwVersion & 0x80000000))
    {
        // Is it Windows 2000 (NT 5.0) or greater ?
        if (LOBYTE(LOWORD(dwVersion)) > 4)
        {
            // In Windows 2000 we need to use the Product Suite APIs
            // Don't static link because it won't load on non-Win2000 systems
            hmodNtDll = GetModuleHandle( "NTDLL.DLL" );
            if (hmodNtDll != NULL)
            {
                pfnVerSetConditionMask = (PFnVerSetConditionMask )GetProcAddress( hmodNtDll, "VerSetConditionMask");
                if (pfnVerSetConditionMask != NULL)
                {
                    dwlConditionMask = (*pfnVerSetConditionMask)( dwlConditionMask, VER_SUITENAME, VER_AND );
                    hmodK32 = GetModuleHandle( "KERNEL32.DLL" );
                    if (hmodK32 != NULL)
                    {
                        pfnVerifyVersionInfoA = (PFnVerifyVersionInfoA)GetProcAddress( hmodK32, "VerifyVersionInfoA") ;
                        if (pfnVerifyVersionInfoA != NULL)
                        {
                            ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
                            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
                            osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
                            bResult = (*pfnVerifyVersionInfoA)(
                                              &osVersionInfo,
                                              VER_SUITENAME,
                                              dwlConditionMask);
                        }
                    }
                }
            }
        }
        else
        {
            // This is NT 4.0 or older
            bResult = ValidateProductSuite( "Terminal Server" );
        }
    }

    return bResult;
}

/**
 * Keyboard hook: record tick count
 **/
LRESULT CALLBACK KeyboardTracker(int code, WPARAM wParam, LPARAM lParam)
{
	if (code==HC_ACTION)
    {
		g_dwLastTick = GetTickCount();
	}
	return ::CallNextHookEx(g_hHkKeyboard, code, wParam, lParam);
}

/**
 * Mouse hook: record tick count
 **/
LRESULT CALLBACK MouseTracker(int code, WPARAM wParam, LPARAM lParam)
{
	if (code==HC_ACTION) 
    {
		MOUSEHOOKSTRUCT* pStruct = (MOUSEHOOKSTRUCT*)lParam;
		//we will assume that any mouse msg with the same locations as spurious
		if (pStruct->pt.x != g_mouseLocX || pStruct->pt.y != g_mouseLocY)
		{
			g_mouseLocX = pStruct->pt.x;
			g_mouseLocY = pStruct->pt.y;
			g_dwLastTick = GetTickCount();
		}
	}
	return ::CallNextHookEx(g_hHkMouse, code, wParam, lParam);
}

/**
 * Get tick count of last keyboard or mouse event
 **/
__declspec(dllexport) DWORD IdleTrackerGetIdleTickCount()
{
    DWORD dwCurrentTickCount = GetTickCount();
    DWORD dwLastTickCount = 0;

    if ( g_bIsWindows2000Compatible )
    {
        LASTINPUTINFO lii;
        ZeroMemory( &lii, sizeof(lii) );
        lii.cbSize = sizeof(lii);
        g_fnGetLastInputInfo( &lii );

        /**
         * If both values are greater than the system tick count then
         *   the system must have looped back to the begining.
         **/
        if ( ( dwCurrentTickCount < lii.dwTime ) &&
             ( dwCurrentTickCount < g_pSystemWideIdleData->dwLastTick ) )
        {
            lii.dwTime = dwCurrentTickCount;
            g_pSystemWideIdleData->dwLastTick = dwCurrentTickCount;
        }

        if ( lii.dwTime > g_pSystemWideIdleData->dwLastTick )
            g_pSystemWideIdleData->dwLastTick = lii.dwTime;

        dwLastTickCount = g_pSystemWideIdleData->dwLastTick;
    }
    else
    {
        dwLastTickCount = g_dwLastTick;
    }

	return (dwCurrentTickCount - dwLastTickCount);
}

/**
 * Initialize DLL: install kbd/mouse hooks.
 **/
__declspec(dllexport) BOOL IdleTrackerInit()
{
 	BOOL                bExists = FALSE;
	BOOL                bResult = FALSE;
 	SECURITY_ATTRIBUTES	sec_attr;
 	SECURITY_DESCRIPTOR sd;


    g_bIsWindows2000Compatible = IsWindows2000Compatible();
    g_bIsTerminalServicesEnabled = IsTerminalServicesEnabled();

        
    if ( !g_bIsWindows2000Compatible )
    {
        if ( NULL == g_hHkKeyboard )
        {
		    g_hHkKeyboard = SetWindowsHookEx( WH_KEYBOARD, KeyboardTracker, g_hInstance, 0 );
	    }
	    if ( NULL == g_hHkMouse )
        {
		    g_hHkMouse = SetWindowsHookEx( WH_MOUSE, MouseTracker, g_hInstance, 0 );
	    }

	    _ASSERT( g_hHkKeyboard );
	    _ASSERT( g_hHkMouse );
    }
    else
    {
        g_hUser32 = LoadLibrary("user32.dll");            
        if (g_hUser32)
            g_fnGetLastInputInfo = (GETLASTINPUTINFO)GetProcAddress(g_hUser32, "GetLastInputInfo");


 	    /*
 	    * Create a security descriptor that will allow
 	    * everyone full access.
 	    */
 	    InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
 	    SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );

 	    sec_attr.nLength = sizeof(sec_attr);
 	    sec_attr.bInheritHandle = TRUE;
 	    sec_attr.lpSecurityDescriptor = &sd;

 	    /*
	    * Create a filemap object that is global for everyone,
 	    * including users logged in via terminal services.
 	    */
 	    if( g_bIsTerminalServicesEnabled )
 	    {
 		    g_hMemoryMappedData = 
                CreateFileMapping(
                    INVALID_HANDLE_VALUE,
 				    &sec_attr,
 				    PAGE_READWRITE,
 				    0,
 				    4096,
 				    "Global\\BoincIdleTracker"
                );
 	    }
 	    else
 	    {
 		    g_hMemoryMappedData = 
                CreateFileMapping(
                    INVALID_HANDLE_VALUE,
 				    &sec_attr,
 				    PAGE_READWRITE,
 				    0,
 				    4096,
 				    "BoincIdleTracker"
                );
 	    }
	    _ASSERT( g_hMemoryMappedData );

 	    if( NULL != g_hMemoryMappedData )
 	    {
 		    if( ERROR_ALREADY_EXISTS == GetLastError() )
 			    bExists = TRUE;

            g_pSystemWideIdleData = (struct SystemWideIdleData*) 
                MapViewOfFile(
                    g_hMemoryMappedData, 
                    FILE_MAP_ALL_ACCESS,
 				    0,
                    0,
                    0
                );

            _ASSERT( g_pSystemWideIdleData );
        }

 	    if( !bExists )
 	    {
 		    g_pSystemWideIdleData->dwLastTick = GetTickCount();
 	    }
    }


    if ( !g_bIsWindows2000Compatible )
    {
	    if ( !g_hHkKeyboard || !g_hHkMouse )
		    bResult = FALSE;
	    else
		    bResult = TRUE;
    }
    else
    {
        if ( !g_hUser32 || !g_fnGetLastInputInfo || !g_hMemoryMappedData || !g_pSystemWideIdleData )
		    bResult = FALSE;
	    else
		    bResult = TRUE;
    }

    return bResult;
}

/**
 * Terminate DLL: remove hooks.
 **/
__declspec(dllexport) void IdleTrackerTerm()
{
    if ( !g_bIsWindows2000Compatible )
    {
	    BOOL bResult;
	    if ( g_hHkKeyboard )
	    {
		    bResult = UnhookWindowsHookEx( g_hHkKeyboard );
		    _ASSERT( bResult );
		    g_hHkKeyboard = NULL;
	    }
	    if ( g_hHkMouse )
	    {
		    bResult = UnhookWindowsHookEx(g_hHkMouse);
		    _ASSERT( bResult );
		    g_hHkMouse = NULL;
	    }
    }
    else
    {
        if( NULL != g_pSystemWideIdleData )
 	    {
 		    UnmapViewOfFile(g_pSystemWideIdleData);
 		    CloseHandle(g_hMemoryMappedData);
 	    }

        if ( NULL != g_hUser32 )
            FreeLibrary(g_hUser32);
    }
}

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason)
    {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hInstance);
			g_hInstance = hInstance;
            break;
	}
	return TRUE;
}

const char *BOINC_RCSID_14d432d5b3 = "$Id$";
