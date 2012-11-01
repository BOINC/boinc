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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#ifndef __CYGWIN__
#include <intrin.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "client_msgs.h"
#include "client_types.h"
#include "hostinfo_network.h"
#include "hostinfo.h"
#include "idlemon.h"


// Newer system metrics values than what is currently defined in
//   Visual Studio 2005
#ifndef SM_TABLETPC
#define SM_TABLETPC 86
#endif
#ifndef SM_MEDIACENTER
#define SM_MEDIACENTER 87
#endif
#ifndef SM_STARTER
#define SM_STARTER 88
#endif
#ifndef SM_SERVERR2
#define SM_SERVERR2 89
#endif


// Newer product types than what is currently defined in
//   Visual Studio 2005
#ifndef PRODUCT_ULTIMATE
#define PRODUCT_ULTIMATE                            0x00000001
#endif
#ifndef PRODUCT_HOME_BASIC
#define PRODUCT_HOME_BASIC                          0x00000002
#endif
#ifndef PRODUCT_HOME_PREMIUM
#define PRODUCT_HOME_PREMIUM                        0x00000003
#endif
#ifndef PRODUCT_ENTERPRISE
#define PRODUCT_ENTERPRISE                          0x00000004
#endif
#ifndef PRODUCT_HOME_BASIC_N
#define PRODUCT_HOME_BASIC_N                        0x00000005
#endif
#ifndef PRODUCT_BUSINESS
#define PRODUCT_BUSINESS                            0x00000006
#endif
#ifndef PRODUCT_STANDARD_SERVER
#define PRODUCT_STANDARD_SERVER                     0x00000007
#endif
#ifndef PRODUCT_DATACENTER_SERVER
#define PRODUCT_DATACENTER_SERVER                   0x00000008
#endif
#ifndef PRODUCT_SMALLBUSINESS_SERVER
#define PRODUCT_SMALLBUSINESS_SERVER                0x00000009
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER
#define PRODUCT_ENTERPRISE_SERVER                   0x0000000A
#endif
#ifndef PRODUCT_STARTER
#define PRODUCT_STARTER                             0x0000000B
#endif
#ifndef PRODUCT_DATACENTER_SERVER_CORE
#define PRODUCT_DATACENTER_SERVER_CORE              0x0000000C
#endif
#ifndef PRODUCT_STANDARD_SERVER_CORE
#define PRODUCT_STANDARD_SERVER_CORE                0x0000000D
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER_CORE
#define PRODUCT_ENTERPRISE_SERVER_CORE              0x0000000E
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER_IA64
#define PRODUCT_ENTERPRISE_SERVER_IA64              0x0000000F
#endif
#ifndef PRODUCT_BUSINESS_N
#define PRODUCT_BUSINESS_N                          0x00000010
#endif
#ifndef PRODUCT_WEB_SERVER
#define PRODUCT_WEB_SERVER                          0x00000011
#endif
#ifndef PRODUCT_CLUSTER_SERVER
#define PRODUCT_CLUSTER_SERVER                      0x00000012
#endif
#ifndef PRODUCT_HOME_SERVER
#define PRODUCT_HOME_SERVER                         0x00000013
#endif
#ifndef PRODUCT_STORAGE_EXPRESS_SERVER
#define PRODUCT_STORAGE_EXPRESS_SERVER              0x00000014
#endif
#ifndef PRODUCT_STORAGE_STANDARD_SERVER
#define PRODUCT_STORAGE_STANDARD_SERVER             0x00000015
#endif
#ifndef PRODUCT_STORAGE_WORKGROUP_SERVER
#define PRODUCT_STORAGE_WORKGROUP_SERVER            0x00000016
#endif
#ifndef PRODUCT_STORAGE_ENTERPRISE_SERVER
#define PRODUCT_STORAGE_ENTERPRISE_SERVER           0x00000017
#endif
#ifndef PRODUCT_SERVER_FOR_SMALLBUSINESS
#define PRODUCT_SERVER_FOR_SMALLBUSINESS            0x00000018
#endif
#ifndef PRODUCT_SMALLBUSINESS_SERVER_PREMIUM
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM        0x00000019
#endif
#ifndef PRODUCT_HOME_PREMIUM_N
#define PRODUCT_HOME_PREMIUM_N                      0x0000001A
#endif
#ifndef PRODUCT_ENTERPRISE_N
#define PRODUCT_ENTERPRISE_N                        0x0000001B
#endif
#ifndef PRODUCT_ULTIMATE_N
#define PRODUCT_ULTIMATE_N                          0x0000001C
#endif
#ifndef PRODUCT_WEB_SERVER_CORE
#define PRODUCT_WEB_SERVER_CORE                     0x0000001D
#endif
#ifndef PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT
#define PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT    0x0000001E
#endif
#ifndef PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY
#define PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY      0x0000001F
#endif
#ifndef PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING
#define PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING     0x00000020
#endif
#ifndef PRODUCT_SMALLBUSINESS_SERVER_PRIME
#define PRODUCT_SMALLBUSINESS_SERVER_PRIME          0x00000021
#endif
#ifndef PRODUCT_HOME_PREMIUM_SERVER
#define PRODUCT_HOME_PREMIUM_SERVER                 0x00000022
#endif
#ifndef PRODUCT_SERVER_FOR_SMALLBUSINESS_V
#define PRODUCT_SERVER_FOR_SMALLBUSINESS_V          0x00000023
#endif
#ifndef PRODUCT_STANDARD_SERVER_V
#define PRODUCT_STANDARD_SERVER_V                   0x00000024
#endif
#ifndef PRODUCT_DATACENTER_SERVER_V
#define PRODUCT_DATACENTER_SERVER_V                 0x00000025
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER_V
#define PRODUCT_ENTERPRISE_SERVER_V                 0x00000026
#endif
#ifndef PRODUCT_DATACENTER_SERVER_CORE_V
#define PRODUCT_DATACENTER_SERVER_CORE_V            0x00000027
#endif
#ifndef PRODUCT_STANDARD_SERVER_CORE_V
#define PRODUCT_STANDARD_SERVER_CORE_V              0x00000028
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER_CORE_V
#define PRODUCT_ENTERPRISE_SERVER_CORE_V            0x00000029
#endif
#ifndef PRODUCT_HYPERV
#define PRODUCT_HYPERV                              0x0000002A
#endif
#ifndef PRODUCT_STORAGE_EXPRESS_SERVER_CORE
#define PRODUCT_STORAGE_EXPRESS_SERVER_CORE         0x0000002B
#endif
#ifndef PRODUCT_STORAGE_STANDARD_SERVER_CORE
#define PRODUCT_STORAGE_STANDARD_SERVER_CORE        0x0000002C
#endif
#ifndef PRODUCT_STORAGE_WORKGROUP_SERVER_CORE
#define PRODUCT_STORAGE_WORKGROUP_SERVER_CORE       0x0000002D
#endif
#ifndef PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE
#define PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE      0x0000002E
#endif
#ifndef PRODUCT_PROFESSIONAL
#define PRODUCT_PROFESSIONAL                        0x00000030
#endif
#ifndef PRODUCT_PROFESSIONAL_N
#define PRODUCT_PROFESSIONAL_N                      0x00000031
#endif
#ifndef PRODUCT_SB_SOLUTION_SERVER
#define PRODUCT_SB_SOLUTION_SERVER                  0x00000032
#endif
#ifndef PRODUCT_SERVER_FOR_SB_SOLUTIONS
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS             0x00000033
#endif
#ifndef PRODUCT_STANDARD_SERVER_SOLUTIONS
#define PRODUCT_STANDARD_SERVER_SOLUTIONS           0x00000034
#endif
#ifndef PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE
#define PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE      0x00000035
#endif
#ifndef PRODUCT_SB_SOLUTION_SERVER_EM
#define PRODUCT_SB_SOLUTION_SERVER_EM               0x00000036
#endif
#ifndef PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM          0x00000037
#endif
#ifndef PRODUCT_SOLUTION_EMBEDDEDSERVER
#define PRODUCT_SOLUTION_EMBEDDEDSERVER             0x00000038
#endif
#ifndef PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE
#define PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE        0x00000039
#endif
#ifndef PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE   0x0000003F
#endif
#ifndef PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT       0x0000003B
#endif
#ifndef PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL       0x0000003C
#endif
#ifndef PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC    0x0000003D
#endif
#ifndef PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC    0x0000003E
#endif
#ifndef PRODUCT_CLUSTER_SERVER_V
#define PRODUCT_CLUSTER_SERVER_V                    0x00000040
#endif
#ifndef PRODUCT_EMBEDDED
#define PRODUCT_EMBEDDED                            0x00000041
#endif
#ifndef PRODUCT_STARTER_E
#define PRODUCT_STARTER_E                           0x00000042
#endif
#ifndef PRODUCT_HOME_BASIC_E
#define PRODUCT_HOME_BASIC_E                        0x00000043
#endif
#ifndef PRODUCT_HOME_PREMIUM_E
#define PRODUCT_HOME_PREMIUM_E                      0x00000044
#endif
#ifndef PRODUCT_PROFESSIONAL_E
#define PRODUCT_PROFESSIONAL_E                      0x00000045
#endif
#ifndef PRODUCT_ENTERPRISE_E
#define PRODUCT_ENTERPRISE_E                        0x00000046
#endif
#ifndef PRODUCT_ULTIMATE_E
#define PRODUCT_ULTIMATE_E                          0x00000047
#endif
#ifndef PRODUCT_ENTERPRISE_EVALUATION
#define PRODUCT_ENTERPRISE_EVALUATION               0x00000048
#endif
#ifndef PRODUCT_PRERELEASE
#define PRODUCT_PRERELEASE                          0x0000004A
#endif
#ifndef PRODUCT_MULTIPOINT_STANDARD_SERVER
#define PRODUCT_MULTIPOINT_STANDARD_SERVER          0x0000004C
#endif
#ifndef PRODUCT_MULTIPOINT_PREMIUM_SERVER
#define PRODUCT_MULTIPOINT_PREMIUM_SERVER           0x0000004D
#endif
#ifndef PRODUCT_STANDARD_EVALUATION_SERVER
#define PRODUCT_STANDARD_EVALUATION_SERVER          0x0000004F
#endif
#ifndef PRODUCT_DATACENTER_EVALUATION_SERVER
#define PRODUCT_DATACENTER_EVALUATION_SERVER        0x00000050
#endif
#ifndef PRODUCT_PRERELEASE_ARM
#define PRODUCT_PRERELEASE_ARM                      0x00000051
#endif
#ifndef PRODUCT_PRERELEASE_N
#define PRODUCT_PRERELEASE_N                        0x00000052
#endif
#ifndef PRODUCT_ENTERPRISE_N_EVALUATION
#define PRODUCT_ENTERPRISE_N_EVALUATION             0x00000054
#endif
#ifndef PRODUCT_EMBEDDED_AUTOMOTIVE
#define PRODUCT_EMBEDDED_AUTOMOTIVE					0x00000055
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_A
#define PRODUCT_EMBEDDED_INDUSTRY_A					0x00000056
#endif
#ifndef PRODUCT_THINPC								
#define PRODUCT_THINPC								0x00000057
#endif
#ifndef PRODUCT_EMBEDDED_A
#define PRODUCT_EMBEDDED_A							0x00000058
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY
#define PRODUCT_EMBEDDED_INDUSTRY					0x00000059
#endif
#ifndef PRODUCT_EMBEDDED_E
#define PRODUCT_EMBEDDED_E                          0x0000005A
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_E
#define PRODUCT_EMBEDDED_INDUSTRY_E                 0x0000005B
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_A_E
#define PRODUCT_EMBEDDED_INDUSTRY_A_E               0x0000005C
#endif
#ifndef PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER
#define PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER  0x00000060
#endif
#ifndef PRODUCT_CORE_ARM                            
#define PRODUCT_CORE_ARM                            0x00000061
#endif
#ifndef PRODUCT_CORE_N                              
#define PRODUCT_CORE_N                              0x00000062
#endif
#ifndef PRODUCT_CORE_COUNTRYSPECIFIC               
#define PRODUCT_CORE_COUNTRYSPECIFIC                0x00000063
#endif
#ifndef PRODUCT_CORE_SINGLELANGUAGE                 
#define PRODUCT_CORE_SINGLELANGUAGE                 0x00000064
#endif
#ifndef PRODUCT_CORE                                
#define PRODUCT_CORE                                0x00000065
#endif
#ifndef PRODUCT_PROFESSIONAL_WMC                    
#define PRODUCT_PROFESSIONAL_WMC                    0x00000067
#endif

// Newer suite types than what is currently defined in
//   Visual Studio 2005
#ifndef VER_SUITE_WH_SERVER
#define VER_SUITE_WH_SERVER                         0x00008000
#endif




// Returns the number of seconds difference from UTC
//
int get_timezone(int& timezone) {
    TIME_ZONE_INFORMATION tzi;
    memset(&tzi, 0, sizeof(TIME_ZONE_INFORMATION));
    DWORD result = GetTimeZoneInformation(&tzi);
    if (result == TIME_ZONE_ID_DAYLIGHT) {
        timezone = -(tzi.Bias + tzi.DaylightBias) * 60;
    } else {
        timezone = -(tzi.Bias + tzi.StandardBias) * 60;
    }
    return 0;
}


// Returns the memory information
//
int get_memory_info(double& bytes, double& swap) {
    MEMORYSTATUSEX mStatusEx;
    ZeroMemory(&mStatusEx, sizeof(MEMORYSTATUSEX));
    mStatusEx.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&mStatusEx);
    bytes = (double)mStatusEx.ullTotalPhys;
    swap = (double)mStatusEx.ullTotalPageFile;
    return 0;
}


// Returns the OS name and version
//

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

int get_os_information(
    char* os_name, int /*os_name_size*/, char* os_version, int os_version_size
) {
    // This code snip-it was copied straight out of the MSDN Platform SDK
    //   Getting the System Version example and modified to dump the output
    //   into os_name.
    char szVersion[128];
    char szSKU[128];
    char szServicePack[128];
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    PGNSI pGNSI;
    PGPI pGPI;
    BOOL bOsVersionInfoEx;
    DWORD dwType = 0;

    ZeroMemory(szVersion, sizeof(szVersion));
    ZeroMemory(szSKU, sizeof(szSKU));
    ZeroMemory(szServicePack, sizeof(szServicePack));
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);


    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.
    bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
    if(!bOsVersionInfoEx) {
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        GetVersionEx ( (OSVERSIONINFO *) &osvi );
    }


    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo");
    if(NULL != pGNSI) {
        pGNSI(&si);
    } else {
        GetSystemInfo(&si);
    }

    pGPI = (PGPI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetProductInfo");


    // Windows is a Microsoft OS
    strcpy(os_name, "Microsoft ");

    switch (osvi.dwPlatformId) {
        case VER_PLATFORM_WIN32_NT:

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 ) {
                if( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strcat(os_name, "Windows 8");
                } else {
                    strcat(os_name, "Windows Server 2012");
                }
                pGPI( 6, 2, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 ) {
                if( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strcat(os_name, "Windows 7");
                } else {
                    strcat(os_name, "Windows Server 2008 \"R2\"");
                }
                pGPI( 6, 1, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 ) {
                if( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strcat(os_name, "Windows Vista");
                } else {
                    strcat(os_name, "Windows Server 2008");
                }
                pGPI( 6, 0, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ) {
                if( osvi.wProductType == VER_NT_WORKSTATION) {
                    strcat(os_name, "Windows XP");
                } else {
                    if( GetSystemMetrics(SM_SERVERR2) ) {
                        strcat(os_name, "Windows Server 2003 \"R2\"");
                    } else {
                        strcat(os_name, "Windows Server 2003");
                    }
                }
            }

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) {
                strcat(os_name, "Windows XP");
            }

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {
                strcat(os_name, "Windows 2000");
            }

            if ( osvi.dwMajorVersion <= 4 ) {
                strcat(os_name, "Windows NT");
            }

            break;

        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
                strcat(os_name, "Windows 95");
            }

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
                strcat( os_name, "Windows 98");
            }

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
                strcat( os_name, "Windows Millennium");
            }

            break;

        case VER_PLATFORM_WIN32s:

            strcat( os_name, "Win32s");
            break;
    }


    snprintf( szVersion, sizeof(szVersion), ", (%.2u.%.2u.%.4u.%.2u)",
        osvi.dwMajorVersion, osvi.dwMinorVersion, (osvi.dwBuildNumber & 0xFFFF), 0
    );


    switch (osvi.dwPlatformId) {
        // Test for the Windows NT product family.
        case VER_PLATFORM_WIN32_NT:

            // Test for specific product on Windows NT 4.0 SP6 and later.
            if( bOsVersionInfoEx ) {

                // Test for the workstation type.
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {

                    if( (osvi.dwMajorVersion == 6) ) {
                        switch(dwType) {
                            case PRODUCT_ULTIMATE:
                               strcat(szSKU, "Ultimate ");
                               break;
                            case PRODUCT_PROFESSIONAL:
                               strcat(szSKU, "Professional ");
                               break;
                            case PRODUCT_HOME_PREMIUM:
                               strcat(szSKU, "Home Premium ");
                               break;
                            case PRODUCT_HOME_BASIC:
                               strcat(szSKU, "Home Basic ");
                               break;
                            case PRODUCT_ENTERPRISE:
                               strcat(szSKU, "Enterprise ");
                               break;
                            case PRODUCT_BUSINESS:
                               strcat(szSKU, "Business ");
                               break;
                            case PRODUCT_STARTER:
                               strcat(szSKU, "Starter ");
                               break;
                            case PRODUCT_PROFESSIONAL_N:
                               strcat(szSKU, "Professional N ");
                               break;
                            case PRODUCT_HOME_PREMIUM_N:
                               strcat(szSKU, "Home Premium N ");
                               break;
                            case PRODUCT_HOME_BASIC_N:
                               strcat(szSKU, "Home Basic N ");
                               break;
                            case PRODUCT_ULTIMATE_N:
                               strcat(szSKU, "Ultimate N ");
                               break;
                            case PRODUCT_ENTERPRISE_N:
                               strcat(szSKU, "Enterprise N ");
                               break;
                            case PRODUCT_BUSINESS_N:
                               strcat(szSKU, "Business N ");
                               break;
                            case PRODUCT_PROFESSIONAL_E:
                               strcat(szSKU, "Professional E ");
                               break;
                            case PRODUCT_HOME_PREMIUM_E:
                               strcat(szSKU, "Home Premium E ");
                               break;
                            case PRODUCT_HOME_BASIC_E:
                               strcat(szSKU, "Home Basic E ");
                               break;
                            case PRODUCT_ULTIMATE_E:
                               strcat(szSKU, "Ultimate E ");
                               break;
                            case PRODUCT_ENTERPRISE_E:
                               strcat(szSKU, "Enterprise E ");
                               break;
                            case PRODUCT_PRERELEASE:
                                strcat(szSKU, "Developer Preview ");
                                break;
                            case PRODUCT_PRERELEASE_N:
                                strcat(szSKU, "Developer Preview N ");
                                break;
							// added Embbedded SKUs:
							case PRODUCT_EMBEDDED:
								strcat(szSKU, "Embedded Standard ");
								break;
							case PRODUCT_THINPC:    
								strcat(szSKU, "ThinPC ");
								break;

                       }
                    } else if( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ) {
                        if( osvi.wSuiteMask & VER_SUITE_PERSONAL ) {
                            strcat(szSKU, "Home ");
                        } else {
                            strcat(szSKU, "Professional ");
                        }
                    } else if( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) ) {
                        if( osvi.wSuiteMask & VER_SUITE_PERSONAL ) {
                            strcat(szSKU, "Home ");
                        } else if( GetSystemMetrics(SM_TABLETPC) ) {
                            strcat(szSKU, "Tablet PC ");
                        } else if( GetSystemMetrics(SM_MEDIACENTER) ) {
                            strcat(szSKU, "Media Center ");
                        } else if( GetSystemMetrics(SM_STARTER) ) {
                            strcat(szSKU, "Starter ");
                        } else {
                            strcat(szSKU, "Professional ");
                        }
                    } else if( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) ) {
                        strcat(szSKU, "Professional ");
                    } else if(  (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) ) {
                        strcat(szSKU, "Workstation ");
                    }
                }

				// Test for the server type.
                else if ( (osvi.wProductType == VER_NT_SERVER) || (osvi.wProductType == VER_NT_DOMAIN_CONTROLLER) ) {
                    if( (osvi.dwMajorVersion == 6) ) {

                        switch(dwType) {
                            case PRODUCT_CLUSTER_SERVER:
                               strcat( szSKU, "Cluster Server ");
                               break;
                            case PRODUCT_DATACENTER_SERVER:
                               strcat( szSKU, "Datacenter ");
                               break;
                            case PRODUCT_DATACENTER_SERVER_CORE:
                               strcat( szSKU, "Datacenter (core installation) ");
                               break;
                            case PRODUCT_ENTERPRISE_SERVER:
                               strcat( szSKU, "Enterprise ");
                               break;
                            case PRODUCT_ENTERPRISE_SERVER_CORE:
                               strcat( szSKU, "Enterprise (core installation) ");
                               break;
                            case PRODUCT_ENTERPRISE_SERVER_IA64:
                               strcat( szSKU, "Enterprise ");
                               break;
                            case PRODUCT_SMALLBUSINESS_SERVER:
                               strcat( szSKU, "Small Business Server");
                               break;
                            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                               strcat( szSKU, "Small Business Server Premium ");
                               break;
                            case PRODUCT_STANDARD_SERVER:
                               strcat( szSKU, "Standard ");
                               break;
                            case PRODUCT_STANDARD_SERVER_CORE:
                               strcat( szSKU, "Standard (core installation) ");
                               break;
                            case PRODUCT_WEB_SERVER:
                               strcat( szSKU, "Web Server ");
                               break;
                            case PRODUCT_WEB_SERVER_CORE:
                               strcat( szSKU, "Web Server (core installation) ");
                               break;
                            case PRODUCT_HOME_SERVER:
                               strcat( szSKU, "Home Server ");
                               break;
                            case PRODUCT_HOME_PREMIUM_SERVER:
                               strcat( szSKU, "Home Premium Server ");
                               break;
                            case PRODUCT_STORAGE_EXPRESS_SERVER:
                               strcat( szSKU, "Storage Server Express ");
                               break;
                            case PRODUCT_STORAGE_STANDARD_SERVER:
                               strcat( szSKU, "Storage Server Standard ");
                               break;
                            case PRODUCT_STORAGE_WORKGROUP_SERVER:
                               strcat( szSKU, "Storage Server Workgroup ");
                               break;
                            case PRODUCT_STORAGE_ENTERPRISE_SERVER:
                               strcat( szSKU, "Storage Server Enterprise ");
                               break;
                            case PRODUCT_SERVER_FOR_SMALLBUSINESS:
                               strcat( szSKU, "Server For Small Business ");
                               break;
                            case PRODUCT_HYPERV:
								strcat( szSKU, "Hyper-V ");
								break;
                        }

                    } else if( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ) {

                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                            strcat( szSKU, "Datacenter Server " );
                        } else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                            strcat( szSKU, "Enterprise Server " );
                        } else if ( osvi.wSuiteMask & VER_SUITE_BLADE ) {
                            strcat( szSKU, "Web Server " );
                        } else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER ) {
                            strcat( szSKU, "Home Server " );
                        } else {
                            strcat( szSKU, "Standard Server " );
                        }

                    } else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {

                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                            strcat( szSKU, "Datacenter Server " );
                        } else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                            strcat( szSKU, "Advanced Server " );
                        } else {
                            strcat( szSKU, "Standard Server " );
                        }

                    } else { // Windows NT 4.0
                        if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                            strcat( szSKU, "Enterprise Server " );
                        } else {
                            strcat( szSKU, "Server " );
                        }
                    }
                }

                switch (si.wProcessorArchitecture) {
                    case PROCESSOR_ARCHITECTURE_INTEL:
                        strcat(szSKU, "x86 ");
                        break;
                    case PROCESSOR_ARCHITECTURE_MIPS:
                        strcat(szSKU, "MIPS ");
                        break;
                    case PROCESSOR_ARCHITECTURE_ALPHA:
                        strcat(szSKU, "Alpha ");
                        break;
                    case PROCESSOR_ARCHITECTURE_PPC:
                        strcat(szSKU, "PowerPC ");
                        break;
                    case PROCESSOR_ARCHITECTURE_IA64:
                        strcat(szSKU, "Itanium ");
                        break;
                    case PROCESSOR_ARCHITECTURE_ALPHA64:
                        strcat(szSKU, "Alpha 64-bit ");
                        break;
                    case PROCESSOR_ARCHITECTURE_AMD64:
                        strcat(szSKU, "x64 ");
                        break;
                    case PROCESSOR_ARCHITECTURE_UNKNOWN:
                        strcat(szSKU, "Unknown ");
                        break;
                }

                strcat(szSKU, "Edition");

            } else { // Test for specific product on Windows NT 4.0 SP5 and earlier

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

                if ( lstrcmpi( "WINNT", szProductType) == 0 ) {
                    strcpy( szSKU, "Workstation Edition" );
                } if ( lstrcmpi( "LANMANNT", szProductType) == 0 ) {
                    strcpy( szSKU, "Server Edition" );
                } if ( lstrcmpi( "SERVERNT", szProductType) == 0 ) {
                    strcpy( szSKU, "Advanced Server Edition" );
                }

            }

            // Display service pack (if any) and build number.
            if( osvi.dwMajorVersion == 4 && lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0
            ) {
                HKEY hKey;
                LONG lRet;

                // Test for SP6 versus SP6a.
                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
                    0, KEY_QUERY_VALUE, &hKey );

                if( lRet == ERROR_SUCCESS ) {
                    strcpy( szServicePack, ", " );
                    strcat( szServicePack, "Service Pack 6a" );
                } else {// Windows NT 4.0 prior to SP6a
                    if ( strlen(osvi.szCSDVersion) > 0 ) {
                        strcpy( szServicePack, ", " );
                        strcat( szServicePack, osvi.szCSDVersion );
                    }
                }

                RegCloseKey( hKey );

            } else { // Windows NT 3.51 and earlier or Windows 2000 and later
                if ( strlen(osvi.szCSDVersion) > 0 ) {
                    strcpy( szServicePack, ", " );
                    strcat( szServicePack, osvi.szCSDVersion );
                }
            }

            break;

        // Test for the Windows 95 product family.
        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
                if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                    strcpy( szServicePack, "OSR2" );
            }

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
                if ( osvi.szCSDVersion[1] == 'A' )
                    strcpy( szServicePack, "SE" );
            }

            break;
    }

    snprintf( os_version, os_version_size, "%s%s%s", szSKU, szServicePack, szVersion );

    return 0;
}


// Handle the cpuid instruction on supported compilers
// NOTE: This only handles structured exceptions with Microsoft compilers.
//
int get_cpuid(unsigned int info_type, unsigned int& a, unsigned int& b, unsigned int& c, unsigned int& d) {

#ifdef _MSC_VER

    // Microsoft compiler - use intrinsic
    int retval = 1;
    int CPUInfo[4] = {0,0,0,0};

    __try {
        __cpuid(CPUInfo, info_type);

        a = CPUInfo[0];
        b = CPUInfo[1];
        c = CPUInfo[2];
        d = CPUInfo[3];

        retval = 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return retval;

#elif defined(__GNUC__)

    // GCC compiler
    __asm__ __volatile__ ("cpuid": "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (info_type));
    return 0;

#else
    return 1;
#endif
}


// Returns the processor vendor.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
int get_processor_vendor(char* name, int name_size) {
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

    if (!name) return ERR_INVALID_PARAM;
    if (name_size < 13) return ERR_WRONG_SIZE;

    memset(name, 0, sizeof(name_size));

    if (!get_cpuid(0x00000000, eax, ebx, ecx, edx)) {
        *((int*)(name + 0)) = ebx;
        *((int*)(name + 4)) = edx;
        *((int*)(name + 8)) = ecx;
        *((int*)(name + 12)) = '\0';
    }
    return 0;
}


// Returns the processor family, model, stepping.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
int get_processor_version(int& family, int& model, int& stepping) {
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

    get_cpuid(0x00000000, eax, ebx, ecx, edx);
    if (eax < 0x00000001) return ERR_NOT_IMPLEMENTED;

    if (!get_cpuid(0x00000001, eax, ebx, ecx, edx)) {
        family = (((eax >> 8) + (eax >> 20)) & 0xff);
        model = (((((eax >> 16) & 0xf) << 4) + ((eax >> 4) & 0xf)) & 0xff);
        stepping = (eax & 0xf);
    }
    return 0;
}


// Returns the processor name.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
int get_processor_name(char* name, int name_size) {
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    size_t i = 0;

    if (!name) return ERR_INVALID_PARAM;
    if (name_size < 48) return ERR_WRONG_SIZE;

    memset(name, 0, sizeof(name_size));

    get_cpuid(0x80000000, eax, ebx, ecx, edx);
    if (!(eax >= 0x80000004)) return ERR_NOT_IMPLEMENTED;

    if (!get_cpuid(0x80000002, eax, ebx, ecx, edx)) {
        *((int*)(name + 0))  = eax;
        *((int*)(name + 4))  = ebx;
        *((int*)(name + 8))  = ecx;
        *((int*)(name + 12)) = edx;
    }
    if (!get_cpuid(0x80000003, eax, ebx, ecx, edx)) {
        *((int*)(name + 16)) = eax;
        *((int*)(name + 20)) = ebx;
        *((int*)(name + 24)) = ecx;
        *((int*)(name + 28)) = edx;
    }
    if (!get_cpuid(0x80000004, eax, ebx, ecx, edx)) {
        *((int*)(name + 32)) = eax;
        *((int*)(name + 36)) = ebx;
        *((int*)(name + 40)) = ecx;
        *((int*)(name + 44)) = edx;
    }

    // Old processors contain junk in the brand string, Intel's documentation
    // doesn't mention this at all.  So before returning, change all non-ascii
    // characters to spaces.
    for (i = 0; i < strlen(name); i++) {
        if (!isprint(*(name + i))) {
            *(name + i) = ' ';
        }
    }

    return 0;
}


// Returns the processor cache.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
int get_processor_cache(int& cache) {
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

    get_cpuid(0x80000000, eax, ebx, ecx, edx);
    if (!(eax >= 0x80000006)) return ERR_NOT_IMPLEMENTED;

    if (!get_cpuid(0x80000006, eax, ebx, ecx, edx)) {
        cache = ((ecx >> 16) & 0xffff) * 1024;
    }
    return 0;
}


// Returns the features supported by the processor, use the
// Linux CPU processor feature mnemonics.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
#define FEATURE_TEST(feature_set_supported, test, feature_name) \
    if (feature_set_supported && test) strncat(features, feature_name, features_size - strlen(features))

int get_processor_features(char* vendor, char* features, int features_size) {
    unsigned int std_eax = 0, std_ebx = 0, std_ecx = 0, std_edx = 0;
    unsigned int ext_eax = 0, ext_ebx = 0, ext_ecx = 0, ext_edx = 0;
    unsigned int std_supported = 0, ext_supported = 0, intel_supported = 0, amd_supported = 0;

    if (!vendor) return ERR_INVALID_PARAM;
    if (!features) return ERR_INVALID_PARAM;
    if (features_size < 250) return ERR_WRONG_SIZE;

    memset(features, 0, sizeof(features_size));

    if (strcmp(vendor, "GenuineIntel") == 0) {
        intel_supported = 1;
    }
    if (strcmp(vendor, "AuthenticAMD") == 0) {
        amd_supported = 1;
    }

    get_cpuid(0x00000000, std_eax, std_ebx, std_ecx, std_edx);
    if (std_eax >= 0x00000001) {
        std_supported = 1;
        get_cpuid(0x00000001, std_eax, std_ebx, std_ecx, std_edx);
    }

    get_cpuid(0x80000000, ext_eax, ext_ebx, ext_ecx, ext_edx);
    if (ext_eax >= 0x80000001) {
        ext_supported = 1;
        get_cpuid(0x80000001, ext_eax, ext_ebx, ext_ecx, ext_edx);
    }

    FEATURE_TEST(std_supported, (std_edx & (1 << 0)), "fpu ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 1)), "vme ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 2)), "de ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 3)), "pse ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 4)), "tsc ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 5)), "msr ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 6)), "pae ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 7)), "mce ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 8)), "cx8 ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 9)), "apic ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 11)), "sep ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 12)), "mtrr ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 13)), "pge ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 14)), "mca ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 15)), "cmov ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 16)), "pat ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 17)), "pse36 ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 18)), "psn ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 19)), "clflush ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 21)), "dts ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 22)), "acpi ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 23)), "mmx ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 24)), "fxsr ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 25)), "sse ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 26)), "sse2 ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 27)), "ss ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 28)), "htt ");
    FEATURE_TEST(std_supported, (std_edx & (1 << 29)), "tm ");

    FEATURE_TEST(std_supported, (std_ecx & (1 << 0)), "pni ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 9)), "ssse3 ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 13)), "cx16 ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 19)), "sse4_1 ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 20)), "sse4_2 ");

    FEATURE_TEST(ext_supported, (ext_edx & (1 << 11)), "syscall ");
    FEATURE_TEST(ext_supported, (ext_edx & (1 << 20)), "nx ");
    FEATURE_TEST(ext_supported, (ext_edx & (1 << 29)), "lm ");

    if (intel_supported) {
        // Intel only features
        FEATURE_TEST(std_supported, (std_ecx & (1 << 5)), "vmx ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 6)), "smx ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 8)), "tm2 ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 12)), "fma ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 18)), "dca ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 22)), "movebe ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 23)), "popcnt ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 25)), "aes ");

        FEATURE_TEST(std_supported, (std_edx & (1 << 31)), "pbe ");
    }

    if (amd_supported) {
        // AMD only features
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 2)), "svm ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 6)), "sse4a ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 9)), "osvw ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 10)), "ibs ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 11)), "xop ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 12)), "skinit ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 13)), "wdt ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 15)), "lwp ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 16)), "fma4 ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 18)), "cvt16 ");

        FEATURE_TEST(ext_supported, (ext_edx & (1 << 26)), "page1gb ");
        FEATURE_TEST(ext_supported, (ext_edx & (1 << 27)), "rdtscp ");
        FEATURE_TEST(ext_supported, (ext_edx & (1 << 30)), "3dnowext ");
        FEATURE_TEST(ext_supported, (ext_edx & (1 << 31)), "3dnow ");
    }

    strip_whitespace(features);

    return 0;
}


// Returns the CPU count
//
typedef DWORD (WINAPI *GAPC)(WORD);
#ifndef ALL_PROCESSOR_GROUPS
#define ALL_PROCESSOR_GROUPS 0xffff
#endif
int get_processor_count(int& processor_count) {
    GAPC gapc = (GAPC) GetProcAddress(
        GetModuleHandle(_T("kernel32.dll")),
        "GetActiveProcessorCount"
    );

    if (gapc) {
        processor_count = gapc(ALL_PROCESSOR_GROUPS);
    } else {
        SYSTEM_INFO SystemInfo;
        memset( &SystemInfo, NULL, sizeof( SystemInfo ) );
        ::GetSystemInfo( &SystemInfo );

        processor_count = SystemInfo.dwNumberOfProcessors;
    }
    return 0;
}


// Returns the processor make, model, and additional cpu flags supported by
//   the processor, use the Linux CPU processor feature descriptions.
//
int get_processor_info(
    char* p_vendor, int p_vendor_size, char* p_model, int p_model_size,
    char* p_features, int p_features_size, double& p_cache, int& p_ncpus
) {
    int family = 0, model = 0, stepping = 0, cache = 0;
    char vendor_name[256], processor_name[256], features[256];

    get_processor_vendor(vendor_name, sizeof(vendor_name));
    get_processor_version(family, model, stepping);
    get_processor_name(processor_name, sizeof(processor_name));
    get_processor_cache(cache);
    get_processor_features(vendor_name, features, sizeof(features));
    get_processor_count(p_ncpus);

    snprintf(p_vendor, p_vendor_size, "%s", vendor_name);

    snprintf(p_model, p_model_size,
        "%s [Family %d Model %d Stepping %d]",
        processor_name, family, model, stepping
    );

    snprintf(p_features, p_features_size,
        "%s",
        features
    );

    p_cache = (double)cache;

    return 0;
}


// detect the network usage totals for the host.
//
int get_network_usage_totals(unsigned int& total_received, unsigned int& total_sent) {
    int i;
    int iRetVal = 0;
    DWORD dwSize = 0;
    MIB_IFTABLE* pIfTable;
    MIB_IFROW* pIfRow;

    // Allocate memory for our pointers.
    pIfTable = (MIB_IFTABLE*)malloc(sizeof(MIB_IFTABLE));
    if (pIfTable == NULL) {
        return ERR_MALLOC;
    }

    // Make an initial call to GetIfTable to get the
    // necessary size into dwSize
    dwSize = sizeof(MIB_IFTABLE);
    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        free(pIfTable);
        pIfTable = (MIB_IFTABLE*)malloc(dwSize);
        if (pIfTable == NULL) {
            return ERR_MALLOC;
        }
    }

    // Make a second call to GetIfTable to get the actual
    // data we want.
    iRetVal = (int)GetIfTable(pIfTable, &dwSize, FALSE);
    if (iRetVal == NO_ERROR) {
        for (i = 0; i < (int)pIfTable->dwNumEntries; i++) {
            pIfRow = (MIB_IFROW *) & pIfTable->table[i];
            if (IF_TYPE_SOFTWARE_LOOPBACK != pIfRow->dwType) {
                total_received += pIfRow->dwInOctets;
                total_sent += pIfRow->dwOutOctets;
            }
        }
    }

    if (pIfTable != NULL) {
        free(pIfTable);
        pIfTable = NULL;
    }

    return iRetVal;
}


// see if Virtualbox is installed
//
int HOST_INFO::get_virtualbox_version() {
    HKEY hKey;
    char szInstallDir[256];
    char szVersion[256];
    DWORD dwInstallDir = sizeof(szInstallDir);
    DWORD dwVersion = sizeof(szVersion);
    LONG lRet;

    strcpy(virtualbox_version, "");

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Oracle\\VirtualBox",
        0, KEY_QUERY_VALUE, &hKey
    );
    if (lRet == ERROR_SUCCESS) {
        lRet = RegQueryValueEx(hKey, "InstallDir", NULL, NULL,
            (LPBYTE) szInstallDir, &dwInstallDir
        );
        if((lRet != ERROR_SUCCESS) || (dwInstallDir > sizeof(szInstallDir))) {
            return 1;
        }

        lRet = RegQueryValueEx(
            hKey, "VersionExt", NULL, NULL, (LPBYTE) szVersion, &dwVersion
        );
        if((lRet != ERROR_SUCCESS) || (dwVersion > sizeof(szVersion))) {
            lRet = RegQueryValueEx(
                hKey, "Version", NULL, NULL, (LPBYTE) szVersion, &dwVersion
            );
            if((lRet != ERROR_SUCCESS) || (dwVersion > sizeof(szVersion))) {
                return 1;
            }
        }

        strncat(szInstallDir, "\\virtualbox.exe", sizeof(szInstallDir) - strlen(szInstallDir));

        if (boinc_file_exists(szInstallDir)) {
            safe_strcpy(virtualbox_version, szVersion);
        }
    }

    RegCloseKey( hKey );
    return 0;
}


// Gets host information; called on startup and before each sched RPC
//
int HOST_INFO::get_host_info() {
    get_timezone(timezone);
    get_filesystem_info(d_total, d_free);
    get_memory_info(m_nbytes, m_swap);
    get_os_information(
        os_name, sizeof(os_name), os_version, sizeof(os_version)
    );
    get_virtualbox_version();
    get_processor_info(
        p_vendor, sizeof(p_vendor),
        p_model, sizeof(p_model),
        p_features, sizeof(p_features),
        m_cache,
        p_ncpus
    );
    get_local_network_info();
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

bool HOST_INFO::users_idle(bool /*check_all_logins*/, double idle_time_to_run) {
    double seconds_idle = get_idle_tick_count() / 1000;
    double seconds_time_to_run = 60 * idle_time_to_run;
    return seconds_idle > seconds_time_to_run;
}

