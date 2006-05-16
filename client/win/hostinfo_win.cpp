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

#include "boinc_win.h"
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

#include "client_types.h"
#include "filesys.h"
#include "util.h"
#include "client_msgs.h"
#include "hostinfo_network.h"
#include "hostinfo.h"

HINSTANCE g_hClientLibraryDll;


// Memory Status Structure for Win2K and WinXP based systems.
typedef struct _MYMEMORYSTATUSEX {  
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORDLONG ullTotalPhys;
    DWORDLONG ullAvailPhys;
    DWORDLONG ullTotalPageFile;
    DWORDLONG ullAvailPageFile;
    DWORDLONG ullTotalVirtual;
    DWORDLONG ullAvailVirtual;
    DWORDLONG ullAvailExtendedVirtual;
} MYMEMORYSTATUSEX, *LPMYMEMORYSTATUSEX;

typedef BOOL (WINAPI *MYGLOBALMEMORYSTATUSEX)(LPMYMEMORYSTATUSEX lpBuffer);


// Traverse the video adapters and flag them as potiential accelerators.
struct INTERNALMONITORINFO
{
    DWORD  cb;
    TCHAR  DeviceName[32];
    TCHAR  DeviceString[128];
    DWORD  StateFlags;
    TCHAR  DeviceID[128];
    TCHAR  DeviceKey[128];
};


// Returns the number of seconds difference from UTC
//
int get_timezone(void) {
	TIME_ZONE_INFORMATION tzi;
	ZeroMemory(&tzi, sizeof(TIME_ZONE_INFORMATION));
	GetTimeZoneInformation(&tzi);
	return -(tzi.Bias * 60);
}

// Gets windows specific host information (not complete)
//
int HOST_INFO::get_host_info() {

    ZeroMemory(os_name, sizeof(os_name));
    ZeroMemory(os_version, sizeof(os_version));

    // This code snip-it was copied straight out of the MSDN Platform SDK
    //   Getting the System Version example and modified to dump the output
    //   into os_name.

    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;

    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
    {
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        GetVersionEx ( (OSVERSIONINFO *) &osvi );
    }


    switch (osvi.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_NT:

            if ( osvi.dwMajorVersion >= 6 )
                strcpy(os_name, "Microsoft Windows Longhorn" );

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
                strcpy(os_name, "Microsoft Windows 2003" );

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
                strcpy(os_name, "Microsoft Windows XP" );

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                strcpy(os_name, "Microsoft Windows 2000" );

            if ( osvi.dwMajorVersion <= 4 )
                strcpy(os_name, "Microsoft Windows NT" );

            break;

        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
                strcpy(os_name, "Microsoft Windows 95" );

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
                strcpy( os_name, "Microsoft Windows 98" );

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
                strcpy( os_name, "Microsoft Windows Millennium" );

            break;

        case VER_PLATFORM_WIN32s:

            strcpy( os_name, "Microsoft Win32s" );
            break;
    }


    char szVersion[128];
    char szSKU[128];
    char szServicePack[128];


    ZeroMemory( szVersion, sizeof(szVersion) );
    ZeroMemory( szSKU, sizeof(szSKU) );
    ZeroMemory( szServicePack, sizeof(szServicePack) );


    snprintf( szVersion, sizeof(szVersion), ", (%.2u.%.2u.%.4u.%.2u)",
        osvi.dwMajorVersion, osvi.dwMinorVersion, (osvi.dwBuildNumber & 0xFFFF), 0 );


    switch (osvi.dwPlatformId)
    {
        // Test for the Windows NT product family.
        case VER_PLATFORM_WIN32_NT:

            // Test for specific product on Windows NT 4.0 SP6 and later.
            if( bOsVersionInfoEx )
            {
                // Test for the workstation type.
                if ( osvi.wProductType == VER_NT_WORKSTATION )
                {
                    if( osvi.dwMajorVersion == 4 )
                        strcpy( szSKU, "Workstation Edition" );
                    else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                        strcpy( szSKU, "Home Edition" );
                    else
                        strcpy( szSKU, "Professional Edition" );
                }
            
                // Test for the server type.
                else if ( (osvi.wProductType == VER_NT_SERVER) || (osvi.wProductType == VER_NT_DOMAIN_CONTROLLER) )
                {
                    if( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
                    {
                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            strcpy( szSKU, "Datacenter Server Edition" );
                        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            strcpy( szSKU, "Enterprise Server Edition" );
                        else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
                            strcpy( szSKU, "Web Server Edition" );
                        else
                            strcpy( szSKU, "Standard Server Edition" );
                    }
                    else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
                    {
                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            strcpy( szSKU, "Datacenter Server Edition" );
                        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            strcpy( szSKU, "Enterprise Server Edition" );
                        else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
                            strcpy( szSKU, "Web Server Edition" );
                        else
                            strcpy( szSKU, "Standard Server Edition" );
                    }
                    else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                    {
                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            strcpy( szSKU, "Datacenter Server Edition" );
                        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            strcpy( szSKU, "Advanced Server Edition" );
                        else
                            strcpy( szSKU, "Standard Server Edition" );
                    }
                    else  // Windows NT 4.0 
                    {
                        if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            strcpy( szSKU, "Enterprise Server Edition" );
                        else
                            strcpy( szSKU, "Server Edition" );
                    }
                }
            }
            else  // Test for specific product on Windows NT 4.0 SP5 and earlier
            {
                HKEY hKey;
                char szProductType[80];
                DWORD dwBufLen=sizeof(szProductType);
                LONG lRet;

                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                    0, KEY_QUERY_VALUE, &hKey );
                if( lRet != ERROR_SUCCESS )
                    return FALSE;

                lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL,
                    (LPBYTE) szProductType, &dwBufLen);
                if( (lRet != ERROR_SUCCESS) || (dwBufLen > 80) )
                    return FALSE;

                RegCloseKey( hKey );

                if ( lstrcmpi( "WINNT", szProductType) == 0 )
                    strcpy( szSKU, "Workstation Edition" );
                if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
                    strcpy( szSKU, "Server Edition" );
                if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
                    strcpy( szSKU, "Advanced Server Edition" );

            }

            // Display service pack (if any) and build number.

            if( osvi.dwMajorVersion == 4 && lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0 )
            {
                HKEY hKey;
                LONG lRet;

                // Test for SP6 versus SP6a.
                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
                    0, KEY_QUERY_VALUE, &hKey );
                if( lRet == ERROR_SUCCESS )
                {
                    strcpy( szServicePack, ", " );
                    strcat( szServicePack, "Service Pack 6a" );
                }
                else // Windows NT 4.0 prior to SP6a
                {
                    if ( strlen(osvi.szCSDVersion) > 0 )
                    {
                        strcpy( szServicePack, ", " );
                        strcat( szServicePack, osvi.szCSDVersion );
                    }
                }

                RegCloseKey( hKey );
            }
            else // Windows NT 3.51 and earlier or Windows 2000 and later
            {
                if ( strlen(osvi.szCSDVersion) > 0 )
                {
                    strcpy( szServicePack, ", " );
                    strcat( szServicePack, osvi.szCSDVersion );
                }
            }

            break;

        // Test for the Windows 95 product family.
        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
            {
                if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                    strcpy( szServicePack, "OSR2" );
            } 

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
            {
                if ( osvi.szCSDVersion[1] == 'A' )
                    strcpy( szServicePack, "SE" );
            } 

            break;
    }

    snprintf( os_version, sizeof(os_version), "%s%s%s",
        szSKU, szServicePack, szVersion );


	timezone = get_timezone();

	// Open the WinSock dll so we can get host info
    WORD    wVersionRequested;
	WSADATA wsdata;
	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsdata);

	// Get host name/ip info
    get_local_network_info(
        domain_name, sizeof(domain_name), ip_addr, sizeof(ip_addr)
    );

	// Close the WinSock dll
	WSACleanup();

    // Detect the filesystem information
	get_filesystem_info(d_total, d_free);
    
    // Detect the amount of memory the system has with the new API, if it doesn't
    //   exist, then use the older API.
    HMODULE hKernel32;
    MYGLOBALMEMORYSTATUSEX myGlobalMemoryStatusEx;
    hKernel32 = LoadLibrary("kernel32.dll");
    if (hKernel32) {
        myGlobalMemoryStatusEx = (MYGLOBALMEMORYSTATUSEX) GetProcAddress(hKernel32, "GlobalMemoryStatusEx");
    }

    if (hKernel32 && myGlobalMemoryStatusEx) {
	    MYMEMORYSTATUSEX mStatusEx;
	    ZeroMemory(&mStatusEx, sizeof(MYMEMORYSTATUSEX));
	    mStatusEx.dwLength = sizeof(MYMEMORYSTATUSEX);
	    (*myGlobalMemoryStatusEx)(&mStatusEx);
        m_nbytes = (double)mStatusEx.ullTotalPhys;
        m_swap = (double)mStatusEx.ullTotalPageFile;
    } else {
	    MEMORYSTATUS mStatus;
	    ZeroMemory(&mStatus, sizeof(MEMORYSTATUS));
	    mStatus.dwLength = sizeof(MEMORYSTATUS);
	    GlobalMemoryStatus(&mStatus);
	    m_nbytes = (double)mStatus.dwTotalPhys;
	    m_swap = (double)mStatus.dwTotalPageFile;
    }

    if (hKernel32) {
        FreeLibrary(hKernel32);
    }
	
    // Detect the number of CPUs
    SYSTEM_INFO SystemInfo;
    memset( &SystemInfo, NULL, sizeof( SystemInfo ) );
    ::GetSystemInfo( &SystemInfo );

    p_ncpus = SystemInfo.dwNumberOfProcessors;

	// gets processor vendor name and model name from registry, works for intel
	char vendorName[256], processorName[256], identifierName[256];
	HKEY hKey;
	LONG retval;
	DWORD nameSize, procSpeed;
	bool gotIdent = false, gotProcName = false, gotMHz = false, gotVendIdent = false;

	retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
	if(retval == ERROR_SUCCESS) {
        // Win9x and WinNT store different information in these field.
        // NT Examples:
        // ProcessorNameString: Intel(R) Xeon(TM) CPU 3.06GHz
        // Identifier: x86 Family 15 Model 2 Stepping 7
        // VendorIdentifier: GenuineIntel
        // ~MHz: 3056
        // 9X Examples:
        // ProcessorNameString: <Not Defined>
        // Identifier: Pentium(r) Processor
        // ~MHz: <Not Defined>
        // VendorIdentifier: GenuineIntel

        // Look in various places for processor information, add'l
		// entries suggested by mark mcclure
		nameSize = sizeof(vendorName);
		retval = RegQueryValueEx(hKey, "VendorIdentifier", NULL, NULL, (LPBYTE)vendorName, &nameSize);
		if (retval == ERROR_SUCCESS) gotVendIdent = true;

		nameSize = sizeof(identifierName);
		retval = RegQueryValueEx(hKey, "Identifier", NULL, NULL, (LPBYTE)identifierName, &nameSize);
		if (retval == ERROR_SUCCESS) gotIdent = true;

		nameSize = sizeof(processorName);
		retval = RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)processorName, &nameSize);
		if (retval == ERROR_SUCCESS) gotProcName = true;

		nameSize = sizeof(DWORD);
		retval = RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&procSpeed, &nameSize);
		if (retval == ERROR_SUCCESS) gotMHz = true;
	}

    if (gotVendIdent) strlcpy( p_vendor, vendorName, sizeof(p_vendor) );
    else strlcpy( p_vendor, "Unknown", sizeof(p_vendor) );

    if (gotProcName) {
        strlcpy( p_model, processorName, sizeof(p_model) );
    } else if (gotIdent && gotMHz) {
        sprintf( p_model, "%s %dMHz", identifierName, procSpeed );
    } else if (gotVendIdent && gotMHz) {
        sprintf( p_model, "%s %dMHz", vendorName, procSpeed );
    } else if (gotIdent) {
        strlcpy( p_model, identifierName, sizeof(p_model) );
    } else if (gotVendIdent) {
        strlcpy( p_model, vendorName, sizeof(p_model) );
    } else {
        strlcpy( p_model, "Unknown", sizeof(p_model) );
    }

	RegCloseKey(hKey);


    // Detect video accelerators on the system.
    DWORD iDevice = 0;
    INTERNALMONITORINFO dispdev;
    dispdev.cb = sizeof(dispdev);
    while(EnumDisplayDevices(NULL, iDevice, (PDISPLAY_DEVICE)&dispdev, 0)) {
        // Ignore NetMeeting's mirrored displays
        if ((dispdev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0) {
            // Is the entry already listed?
            if (!strstr(accelerators, dispdev.DeviceString)) {
                // Is this the first entry?
                if (!strlen(accelerators)) {
                    strncat(accelerators, dispdev.DeviceString, sizeof(accelerators));
                } else {
                    strncat(accelerators, "/", sizeof(accelerators));
                    strncat(accelerators, dispdev.DeviceString, sizeof(accelerators));
                }
            }
        }
        iDevice++;
    }


    // TODO: Detect the ClearSpeed accelerator card(s)
    // TODO: Detect any other types of accelerators that might be useful
    //   for yhe scheduler to know about.


    if (!strlen(host_cpid)) {
        generate_host_cpid();
    }
    return 0;
}

bool HOST_INFO::host_is_running_on_batteries() {
	SYSTEM_POWER_STATUS pStatus;
	ZeroMemory(&pStatus, sizeof(SYSTEM_POWER_STATUS));
	if (!GetSystemPowerStatus(&pStatus)) {
		return false;
	}

    // Sometimes the system reports the ACLineStatus as an 
    //   undocumented value, so lets check to see if the
    //   battery is charging or missing and make that part
    //   of the decision.
    bool bIsOnBatteryPower  = (pStatus.ACLineStatus != 1);
    bool bIsBatteryCharging = ((pStatus.BatteryFlag & 8) == 8);
    bool bIsBatteryMissing = ((pStatus.BatteryFlag & 128) == 128);

	return (bIsOnBatteryPower && !bIsBatteryCharging && !bIsBatteryMissing);
}

bool HOST_INFO::users_idle(bool check_all_logins, double idle_time_to_run) {
    typedef DWORD (CALLBACK* GetFn)();
    static GetFn fn = (GetFn)GetProcAddress(g_hClientLibraryDll, "BOINCGetIdleTickCount");

    if (g_hClientLibraryDll && fn) {
        double seconds_idle = fn() / 1000;
        double seconds_time_to_run = 60 * idle_time_to_run;
        return seconds_idle > seconds_time_to_run;
    }

    return false;
}

const char *BOINC_RCSID_37fbd07edd = "$Id$";
