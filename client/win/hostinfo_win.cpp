// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#define _WIN32_WINNT 0x0400

#include <afxwin.h>
#include <winsock.h>
#include "client_types.h"
#include "hostinfo.h"
#include "filesys.h"
#include "util.h"

// Returns the number of seconds difference from UTC
//
int get_timezone(void) {
	TIME_ZONE_INFORMATION tzi;
	ZeroMemory(&tzi, sizeof(TIME_ZONE_INFORMATION));
	GetTimeZoneInformation(&tzi);
	return (tzi.Bias * 60);
}

// Gets windows specific host information (not complete)
//
int get_host_info(HOST_INFO& host) {
    OSVERSIONINFO OSVersionInfo;
    memset( &OSVersionInfo, NULL, sizeof( OSVersionInfo ) );
    OSVersionInfo.dwOSVersionInfoSize = sizeof( OSVersionInfo );
    ::GetVersionEx( &OSVersionInfo );
    switch ( OSVersionInfo.dwPlatformId ) {
    case VER_PLATFORM_WIN32s:
        strcpy( host.os_name, "Windows 3.1/Win32s" ); // does ANYBODY use this anymore?
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        if ( OSVersionInfo.dwMajorVersion > 4
	        || ( OSVersionInfo.dwMajorVersion == 4
	        && OSVersionInfo.dwMinorVersion >= 90 ) )
        {
            strcpy( host.os_name, "Windows Me" );
        }
		else if ( OSVersionInfo.dwMajorVersion == 4
            && OSVersionInfo.dwMinorVersion >= 10 )
		{
            strcpy( host.os_name, "Windows 98" );
        }
		else
		{
            strcpy( host.os_name, "Windows 95" );
        }
        break;

    case VER_PLATFORM_WIN32_NT:
		if ( OSVersionInfo.dwMajorVersion > 5
            || (OSVersionInfo.dwMajorVersion == 5
            && OSVersionInfo.dwMinorVersion >= 2) )
		{
		    strcpy( host.os_name, "Windows .NET Server" );
		}
		else if (OSVersionInfo.dwMajorVersion == 5
			&& OSVersionInfo.dwMinorVersion == 1)
		{
		    strcpy( host.os_name, "Windows XP" );
		}
		else if (OSVersionInfo.dwMajorVersion == 5
			&& OSVersionInfo.dwMinorVersion == 0)
		{
		    strcpy( host.os_name, "Windows 2000" );
		}
		else if (OSVersionInfo.dwMajorVersion == 4
			&& OSVersionInfo.dwMinorVersion == 0)
		{
		    strcpy( host.os_name, "Windows NT 4.0" );
		}
		else if (OSVersionInfo.dwMajorVersion == 3
			&& OSVersionInfo.dwMinorVersion == 51)
		{
		    strcpy( host.os_name, "Windows NT 3.51" );
		}
		else
		{
            strcpy( host.os_name, "Windows NT" );
        }
        break;

    default:
        sprintf( host.os_name, "Unknown Win32 (%ld)", OSVersionInfo.dwPlatformId );
        break;
    }

    char Version[ 25 ];
    Version[ 0 ] = NULL;
    sprintf(
	Version, "%lu.%lu", OSVersionInfo.dwMajorVersion,
        OSVersionInfo.dwMinorVersion
    );
	safe_strncpy( host.os_version, Version, sizeof(host.os_version) );

    SYSTEM_INFO SystemInfo;
    memset( &SystemInfo, NULL, sizeof( SystemInfo ) );
    ::GetSystemInfo( &SystemInfo );

    host.p_ncpus = SystemInfo.dwNumberOfProcessors;

    switch ( SystemInfo.wProcessorArchitecture ) {
        case PROCESSOR_ARCHITECTURE_INTEL:
        switch ( SystemInfo.dwProcessorType ) {
            case PROCESSOR_INTEL_386:
                strcpy( host.p_model, "80386" );
                break;
            case PROCESSOR_INTEL_486:
                strcpy( host.p_model, "80486" );
                break;
            case PROCESSOR_INTEL_PENTIUM:
                strcpy( host.p_model, "Pentium" );
                break;
            default:
                strcpy( host.p_model, "x86" );
                break;
            }
        break;

        case PROCESSOR_ARCHITECTURE_MIPS:
            strcpy( host.p_model, "MIPS" );
            break;

        case PROCESSOR_ARCHITECTURE_ALPHA:
            strcpy( host.p_model, "Alpha" );
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            strcpy( host.p_model, "Power PC" );
            break;

        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            strcpy( host.p_model, "Unknown" );
            break;
    }
        
	get_filesystem_info(host.d_total, host.d_free);
    
	// Open the WinSock dll so we can get host info
    WORD    wVersionRequested;
	WSADATA wsdata;
	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsdata);

	// Get host name/ip info
    get_local_domain_name(host.domain_name, sizeof(host.domain_name));
    get_local_ip_addr_str(host.ip_addr, sizeof(host.ip_addr));

	// Close the WinSock dll
	WSACleanup();

	host.timezone = get_timezone();

	MEMORYSTATUS mStatus;
	ZeroMemory(&mStatus, sizeof(MEMORYSTATUS));
	mStatus.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&mStatus);
	host.m_nbytes = (double)mStatus.dwTotalPhys;
	host.m_swap = (double)mStatus.dwTotalPageFile;
	
	// gets processor vendor name from registry, works for intel
	char vendorName[256], procNameString[256];
	HKEY hKey;
	LONG retval;
	DWORD nameSize, procSpeed;
	bool gotProcName = false, gotMHz = false, gotVendIdent = false;

	retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
	if(retval == ERROR_SUCCESS) {
		// Look in various places for processor information, add'l
		// entries suggested by mark mcclure
		nameSize = sizeof(procNameString);
		retval = RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)procNameString, &nameSize);
		if (retval == ERROR_SUCCESS) gotProcName = true;

		nameSize = sizeof(vendorName);
		retval = RegQueryValueEx(hKey, "VendorIdentifier", NULL, NULL, (LPBYTE)vendorName, &nameSize);
		if (retval == ERROR_SUCCESS) gotVendIdent = true;
		else strcpy( "Unknown", vendorName );

		nameSize = sizeof(DWORD);
		retval = RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&procSpeed, &nameSize);
		if (retval == ERROR_SUCCESS) gotMHz = true;
	}
	if (gotProcName) safe_strncpy( host.p_vendor, procNameString, sizeof(host.p_vendor) );
	else if (gotMHz) sprintf( host.p_vendor, "%s %dMHz", vendorName, procSpeed );
	else safe_strncpy( host.p_vendor, vendorName, sizeof(host.p_vendor) );

	RegCloseKey(hKey);
    return 0;
}

bool host_is_running_on_batteries() {
	SYSTEM_POWER_STATUS pStatus;
	ZeroMemory(&pStatus, sizeof(SYSTEM_POWER_STATUS));
	if (!GetSystemPowerStatus(&pStatus)) {
		printf( "Got error: %d\n", GetLastError());
		return false;
	}

	return (pStatus.ACLineStatus != 1);
}
