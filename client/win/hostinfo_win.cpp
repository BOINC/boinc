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


// Newer processor features than what is currently defined in
//   Visual Studio 2003
#ifndef PF_SSE_DAZ_MODE_AVAILABLE
#define PF_SSE_DAZ_MODE_AVAILABLE          11   
#endif
#ifndef PF_NX_ENABLED
#define PF_NX_ENABLED                      12   
#endif
#ifndef PF_SSE3_INSTRUCTIONS_AVAILABLE
#define PF_SSE3_INSTRUCTIONS_AVAILABLE     13   
#endif
#ifndef PF_COMPARE_EXCHANGE128
#define PF_COMPARE_EXCHANGE128             14   
#endif
#ifndef PF_COMPARE64_EXCHANGE128
#define PF_COMPARE64_EXCHANGE128           15   
#endif


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
int get_timezone(int& timezone) {

    TIME_ZONE_INFORMATION tzi;

	memset(&tzi, '\0', sizeof(TIME_ZONE_INFORMATION));

    GetTimeZoneInformation(&tzi);

    timezone = -(tzi.Bias * 60);

    return 0;
}


// Returns the memory information
//
int get_memory_info(double& bytes, double& swap) {
    HMODULE hKernel32Lib;
    MYGLOBALMEMORYSTATUSEX myGlobalMemoryStatusEx;
    hKernel32Lib = GetModuleHandle("kernel32.dll");
    if (hKernel32Lib) {
        myGlobalMemoryStatusEx = (MYGLOBALMEMORYSTATUSEX) GetProcAddress(hKernel32Lib, "GlobalMemoryStatusEx");
    }

    if (hKernel32Lib && myGlobalMemoryStatusEx) {
	    MYMEMORYSTATUSEX mStatusEx;
	    ZeroMemory(&mStatusEx, sizeof(MYMEMORYSTATUSEX));
	    mStatusEx.dwLength = sizeof(MYMEMORYSTATUSEX);
	    (*myGlobalMemoryStatusEx)(&mStatusEx);
        bytes = (double)mStatusEx.ullTotalPhys;
        swap = (double)mStatusEx.ullTotalPageFile;
    } else {
	    MEMORYSTATUS mStatus;
	    ZeroMemory(&mStatus, sizeof(MEMORYSTATUS));
	    mStatus.dwLength = sizeof(MEMORYSTATUS);
	    GlobalMemoryStatus(&mStatus);
	    bytes = (double)mStatus.dwTotalPhys;
	    swap = (double)mStatus.dwTotalPageFile;
    }

    return 0;
}


// Returns the OS name and version
//
int get_os_information(
    char* os_name, int os_name_size, char* os_version, int os_version_size
)
{
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

    snprintf( os_version, os_version_size, "%s%s%s", szSKU, szServicePack, szVersion );

    return 0;
}


// Returns the processor make and model
//
int get_processor_info(
    char* p_vendor, int p_vendor_size, char* p_model, int p_model_size
)
{
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

    if (gotVendIdent)
        strlcpy( p_vendor, vendorName, p_vendor_size );
    else
        strlcpy( p_vendor, "Unknown", p_vendor_size );

    if (gotProcName) {
        strlcpy( p_model, processorName, p_model_size );
    } else if (gotIdent && gotMHz) {
        sprintf( p_model, "%s %dMHz", identifierName, procSpeed );
    } else if (gotVendIdent && gotMHz) {
        sprintf( p_model, "%s %dMHz", vendorName, procSpeed );
    } else if (gotIdent) {
        strlcpy( p_model, identifierName, p_model_size );
    } else if (gotVendIdent) {
        strlcpy( p_model, vendorName, p_model_size );
    } else {
        strlcpy( p_model, "Unknown", p_model_size );
    }

	RegCloseKey(hKey);

    return 0;
}


// Returns the CPU count
//
int get_processor_count(int& processor_count) {
    SYSTEM_INFO SystemInfo;
    memset( &SystemInfo, NULL, sizeof( SystemInfo ) );
    ::GetSystemInfo( &SystemInfo );

    processor_count = SystemInfo.dwNumberOfProcessors;
    return 0;
}


// Check to see if a processor feature is available for use
BOOL test_processor_feature(DWORD feature) {
    __try {
        switch (feature) {
            case PF_XMMI_INSTRUCTIONS_AVAILABLE:
                __asm {
                    xorps xmm0, xmm0        // executing SSE instruction
                }
                break;
            case PF_XMMI64_INSTRUCTIONS_AVAILABLE:
                __asm {
                    xorpd xmm0, xmm0        // executing SSE2 instruction
                }
                break;
            case PF_3DNOW_INSTRUCTIONS_AVAILABLE:
                __asm {
                    pfrcp mm0, mm0          // executing 3DNow! instruction
                    emms
                }
                break;
            case PF_MMX_INSTRUCTIONS_AVAILABLE:
                __asm {
                    pxor mm0, mm0           // executing MMX instruction
                    emms
                }
                break;
            default:
                return 0;
                break;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
    return 1;
}


// Detect to see if a processor feature is available for use

// IsProcessorFeaturePresent()
typedef BOOL (__stdcall *tIPFP)( IN DWORD dwFeature );

BOOL is_processor_feature_supported(DWORD feature) {
    // Detect platform information
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId) {
        // Win9x does have the IsProcessorFeaturePresent function, so just
        //   run a quick test.
        return test_processor_feature(feature);
    } else {
        HMODULE hKernel32Lib = GetModuleHandle("kernel32.dll");
        tIPFP pIPFP = (tIPFP)GetProcAddress(hKernel32Lib, "IsProcessorFeaturePresent");
        if (pIPFP) {
            // IsProcessorFeaturePresent is available, use it.
            return pIPFP(feature);
        } else {
            // Ooooppppssss, whichever version of Windows we are running on
            //   doesn't support IsProcessorFeaturePresent, so just test things
            //   out.
            return test_processor_feature(feature);
        }
    }
    return 0;
}


// Returns the list of capabilities supported by both the processor
// and operating system.  The feature list should use the same
// identifiers as defined in Linux.
//
int get_processor_capabilities( char* capabilities, int capabilities_size ) {
    if (is_processor_feature_supported(PF_RDTSC_INSTRUCTION_AVAILABLE)) {
        strncat(capabilities, "tsc ", capabilities_size - strlen(capabilities));
    }
    if (is_processor_feature_supported(PF_PAE_ENABLED)) {
        strncat(capabilities, "pae ", capabilities_size - strlen(capabilities));
    }
    if (is_processor_feature_supported(PF_NX_ENABLED)) {
        strncat(capabilities, "nx ", capabilities_size - strlen(capabilities));
    }
    if (is_processor_feature_supported(PF_XMMI_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "sse ", capabilities_size - strlen(capabilities));
    }
    if (is_processor_feature_supported(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "sse2 ", capabilities_size - strlen(capabilities));
    }
    if (is_processor_feature_supported(PF_SSE3_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "sse3 ", capabilities_size - strlen(capabilities));
    }
    if (is_processor_feature_supported(PF_3DNOW_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "3dnow ", capabilities_size - strlen(capabilities));
    }
    if (is_processor_feature_supported(PF_MMX_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "mmx ", capabilities_size - strlen(capabilities));
    }
    strip_whitespace(capabilities);
    return 0;
}


// Returns the accelerator list.
//
int get_accelerators(
    char* accelerators, int accelerators_size
)
{
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
                    strncat(accelerators, dispdev.DeviceString, accelerators_size - strlen(accelerators));
                } else {
                    strncat(accelerators, "/", accelerators_size - strlen(accelerators));
                    strncat(accelerators, dispdev.DeviceString, accelerators_size - strlen(accelerators));
                }
            }
        }
        iDevice++;
    }

    // TODO: Detect the ClearSpeed accelerator card(s)
    // TODO: Detect any other types of accelerators that might be useful
    //   for the scheduler to know about.

    strip_whitespace(accelerators);
    return 0;
}


// Gets windows specific host information (not complete)
//
int HOST_INFO::get_host_info() {

    // Get timezone
	get_timezone(timezone);

    // Detect the filesystem information
	get_filesystem_info(d_total, d_free);
    
    // Detect the amount of memory the system has
    get_memory_info(m_nbytes, m_swap);

    // Detect OS Information
    get_os_information(
        os_name, sizeof(os_name), os_version, sizeof(os_version)
    );

    // Detect proccessor make and model.
    get_processor_info(
        p_vendor, sizeof(p_vendor), p_model, sizeof(p_model)
    );

    // Detect the number of CPUs
    get_processor_count(p_ncpus);

    // Detect processor capabilities
    get_processor_capabilities(
        p_capabilities, sizeof(p_capabilities)
    );

    // Detect host name/ip info
    get_local_network_info(
        domain_name, sizeof(domain_name), ip_addr, sizeof(ip_addr)
    );

    // Detect which accelerators are installed on the system
    get_accelerators(
        accelerators, sizeof(accelerators)
    );

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
