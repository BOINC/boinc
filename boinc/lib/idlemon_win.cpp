// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif

#include "win_util.h"

/**
 * The following global data is only shared in this process
 **/ 
HANDLE    g_hMemoryMappedData = NULL;

/**
 * The following global data is SHARED among all (processes); i.e., these are system-wide globals.
 **/
struct SystemWideIdleData {
	DWORD	dwLastTick;         // tick time of last input event
};

struct SystemWideIdleData* g_pSystemWideIdleData = NULL;

/**
 * Get tick count of last keyboard or mouse event
 **/
long get_idle_tick_count() {
    DWORD dwCurrentTickCount = GetTickCount();
    DWORD dwLastTickCount = 0;

    if ( g_pSystemWideIdleData )
    {
        LASTINPUTINFO lii;
        ZeroMemory( &lii, sizeof(lii) );
        lii.cbSize = sizeof(lii);
        GetLastInputInfo( &lii );

        /**
         * If both values are greater than the system tick count then
         *   the system must have looped back to the beginning.
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

	return (dwCurrentTickCount - dwLastTickCount);
}


bool startup_idle_monitor() {
 	BOOL                bExists = FALSE;
	bool                bResult = false;
 	SECURITY_ATTRIBUTES	sec_attr;
 	SECURITY_DESCRIPTOR sd;


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
    g_hMemoryMappedData = 
        CreateFileMapping(
            INVALID_HANDLE_VALUE,
		    &sec_attr,
		    PAGE_READWRITE,
		    0,
		    4096,
		    _T("Global\\BoincIdleTracker")
        );
    if( NULL == g_hMemoryMappedData )
    {
        g_hMemoryMappedData = 
            CreateFileMapping(
                INVALID_HANDLE_VALUE,
		        &sec_attr,
		        PAGE_READWRITE,
		        0,
		        4096,
		        _T("BoincIdleTracker")
            );
    }

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
    }

    if( !bExists && g_pSystemWideIdleData )
    {
	    g_pSystemWideIdleData->dwLastTick = GetTickCount();
    }


    if (!g_hMemoryMappedData || !g_pSystemWideIdleData )
	    bResult = false;
    else
	    bResult = true;


    return bResult;
}


bool attach_idle_monitor() {
 	BOOL                bExists = FALSE;
	bool                bResult = false;
 	SECURITY_ATTRIBUTES	sec_attr;
 	SECURITY_DESCRIPTOR sd;


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
    g_hMemoryMappedData = 
        OpenFileMapping(
            FILE_MAP_READ | FILE_MAP_WRITE,
		    FALSE,
		    _T("Global\\BoincIdleTracker")
        );
    if( NULL == g_hMemoryMappedData )
    {
        g_hMemoryMappedData = 
            OpenFileMapping(
                FILE_MAP_READ | FILE_MAP_WRITE,
		        FALSE,
		        _T("BoincIdleTracker")
            );
    }

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
    }

    if( !bExists && g_pSystemWideIdleData )
    {
	    g_pSystemWideIdleData->dwLastTick = GetTickCount();
    }


    if (!g_hMemoryMappedData || !g_pSystemWideIdleData )
	    bResult = false;
    else
	    bResult = true;

    return bResult;
}


void shutdown_idle_monitor()
{
    if( NULL != g_pSystemWideIdleData )
    {
	    UnmapViewOfFile(g_pSystemWideIdleData);
	    CloseHandle(g_hMemoryMappedData);
    }
}


void detach_idle_monitor()
{
	shutdown_idle_monitor();
}
