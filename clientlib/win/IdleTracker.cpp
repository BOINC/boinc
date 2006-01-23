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

#include "stdafx.h"
#include "Identification.h"


/**
 * The following global data is only shared in this instance of the DLL
 **/ 
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
EXTERN_C __declspec(dllexport) DWORD BOINCGetIdleTickCount()
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
BOOL IdleTrackerStartup()
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
            g_hHkKeyboard = SetWindowsHookEx( 
                WH_KEYBOARD, 
                KeyboardTracker, 
                _AtlBaseModule.GetModuleInstance(),
                0
            );
	    }
	    if ( NULL == g_hHkMouse )
        {
		    g_hHkMouse = SetWindowsHookEx( 
                WH_MOUSE, 
                MouseTracker, 
                _AtlBaseModule.GetModuleInstance(),
                0
            );
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
void IdleTrackerShutdown()
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

const char *BOINC_RCSID_14d432d5b3 = "$Id$";
