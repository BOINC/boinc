// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#include "stdafx.h"
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
int HOST_INFO::get_host_info() {

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
        // Test for the Windows NT product family.
        case VER_PLATFORM_WIN32_NT:

            // Test for the specific product family.
            if ( osvi.dwMajorVersion >= 6 )
                strcpy(os_name, "Microsoft Windows Longhorn " );

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
                strcpy(os_name, "Microsoft Windows 2003 " );

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
                strcpy(os_name, "Microsoft Windows XP " );

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                strcpy(os_name, "Microsoft Windows 2000 " );

            if ( osvi.dwMajorVersion <= 4 )
                strcpy(os_name, "Microsoft Windows NT " );

            // Test for specific product on Windows NT 4.0 SP6 and later.
            if( bOsVersionInfoEx )
            {
                // Test for the workstation type.
                if ( osvi.wProductType == VER_NT_WORKSTATION )
                {
                    if( osvi.dwMajorVersion == 4 )
                        strcat(os_name, "Workstation 4.0 " );
                    else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                        strcat(os_name, "Home Edition " );
                    else
                        strcat(os_name, "Professional Edition " );
                }
            
                // Test for the server type.
                else if ( (osvi.wProductType == VER_NT_SERVER) || (osvi.wProductType == VER_NT_DOMAIN_CONTROLLER) )
                {
                    if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
                    {
                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            strcat(os_name, "Datacenter Server Edition " );
                        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            strcat(os_name, "Enterprise Server Edition " );
                        else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
                            strcat(os_name, "Web Server Edition " );
                        else
                            strcat(os_name, "Standard Server Edition " );
                    }
                    else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                    {
                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            strcat(os_name, "Datacenter Server Edition " );
                        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            strcat(os_name, "Advanced Server Edition " );
                        else
                            strcat(os_name, "Standard Server Edition " );
                    }
                    else  // Windows NT 4.0 
                    {
                        if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            strcat(os_name, "Server 4.0, Enterprise Edition " );
                        else
                            strcat(os_name, "Server 4.0 " );
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
                    strcat(os_name, "Workstation " );
                if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
                    strcat(os_name, "Server " );
                if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
                    strcat(os_name, "Advanced Server " );

                sprintf(os_name, "%s%d.%d ",
                    os_name,
                    osvi.dwMajorVersion,
                    osvi.dwMinorVersion );
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
                    sprintf(os_name, "%sService Pack 6a (Build %d)",
                        os_name,
                        osvi.dwBuildNumber & 0xFFFF );
                }
                else // Windows NT 4.0 prior to SP6a
                {
                    if ( strlen(osvi.szCSDVersion) > 0 )
                    {
                        sprintf(os_name, "%s%s (Build %d)",
                            os_name,
                            osvi.szCSDVersion,
                            osvi.dwBuildNumber & 0xFFFF);
                    }
                    else
                    {
                        sprintf(os_name, "%s(Build %d)",
                            os_name,
                            osvi.dwBuildNumber & 0xFFFF);
                    }
                }

                RegCloseKey( hKey );
            }
            else // Windows NT 3.51 and earlier or Windows 2000 and later
            {
                if ( strlen(osvi.szCSDVersion) > 0 )
                {
                    sprintf(os_name, "%s%s (Build %d)",
                        os_name,
                        osvi.szCSDVersion,
                        osvi.dwBuildNumber & 0xFFFF);
                }
                else
                {
                    sprintf(os_name, "%s(Build %d)",
                        os_name,
                        osvi.dwBuildNumber & 0xFFFF);
                }
            }

            break;

        // Test for the Windows 95 product family.
        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
            {
                strcpy(os_name, "Microsoft Windows 95 " );
                if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                   strcat( os_name, "OSR2 " );
            } 

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
            {
                strcpy( os_name, "Microsoft Windows 98 " );
                if ( osvi.szCSDVersion[1] == 'A' )
                    strcat( os_name, "SE " );
            } 

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
            {
                strcpy( os_name, "Microsoft Windows Millennium Edition" );
            } 
            break;

        case VER_PLATFORM_WIN32s:

            strcpy( os_name, "Microsoft Win32s" );
            break;
    }


    char Version[ 25 ];
    Version[ 0 ] = NULL;
    sprintf( Version, "%lu.%lu",
        osvi.dwMajorVersion,
        osvi.dwMinorVersion);

	safe_strncpy( os_version, Version, sizeof(os_version) );

    SYSTEM_INFO SystemInfo;
    memset( &SystemInfo, NULL, sizeof( SystemInfo ) );
    ::GetSystemInfo( &SystemInfo );

    p_ncpus = SystemInfo.dwNumberOfProcessors;

    switch ( SystemInfo.wProcessorArchitecture ) {
        case PROCESSOR_ARCHITECTURE_INTEL:
        switch ( SystemInfo.dwProcessorType ) {
            case PROCESSOR_INTEL_386:
                strcpy( p_model, "80386" );
                break;
            case PROCESSOR_INTEL_486:
                strcpy( p_model, "80486" );
                break;
            case PROCESSOR_INTEL_PENTIUM:
                strcpy( p_model, "Pentium" );
                break;
            default:
                strcpy( p_model, "x86" );
                break;
            }
        break;

        case PROCESSOR_ARCHITECTURE_MIPS:
            strcpy( p_model, "MIPS" );
            break;

        case PROCESSOR_ARCHITECTURE_ALPHA:
            strcpy( p_model, "Alpha" );
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            strcpy( p_model, "Power PC" );
            break;

        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            strcpy( p_model, "Unknown" );
            break;
    }
        
	get_filesystem_info(d_total, d_free);
    
	// Open the WinSock dll so we can get host info
    WORD    wVersionRequested;
	WSADATA wsdata;
	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsdata);

	// Get host name/ip info
    get_local_domain_name(domain_name, sizeof(domain_name));
    get_local_ip_addr_str(ip_addr, sizeof(ip_addr));

	// Close the WinSock dll
	WSACleanup();

	timezone = get_timezone();

	MEMORYSTATUS mStatus;
	ZeroMemory(&mStatus, sizeof(MEMORYSTATUS));
	mStatus.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&mStatus);
	m_nbytes = (double)mStatus.dwTotalPhys;
	m_swap = (double)mStatus.dwTotalPageFile;
	
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
	if (gotProcName) safe_strncpy( p_vendor, procNameString, sizeof(p_vendor) );
	else if (gotMHz) sprintf( p_vendor, "%s %dMHz", vendorName, procSpeed );
	else safe_strncpy( p_vendor, vendorName, sizeof(p_vendor) );

	RegCloseKey(hKey);
    return 0;
}

bool HOST_INFO::host_is_running_on_batteries() {
	SYSTEM_POWER_STATUS pStatus;
	ZeroMemory(&pStatus, sizeof(SYSTEM_POWER_STATUS));
	if (!GetSystemPowerStatus(&pStatus)) {
		return false;
	}

	return (pStatus.ACLineStatus != 1);
}
