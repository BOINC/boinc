// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifdef HAVE_INTRIN_H
#include <intrin.h>
#endif

#include "error_numbers.h"
#include "common_defs.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "client_msgs.h"
#include "client_types.h"
#include "hostinfo.h"
#include "idlemon.h"


// Newer product types than what is currently defined in
//   Visual Studio 2010
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
#define PRODUCT_EMBEDDED_AUTOMOTIVE                 0x00000055
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_A
#define PRODUCT_EMBEDDED_INDUSTRY_A                 0x00000056
#endif
#ifndef PRODUCT_THINPC
#define PRODUCT_THINPC                              0x00000057
#endif
#ifndef PRODUCT_EMBEDDED_A
#define PRODUCT_EMBEDDED_A                          0x00000058
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY
#define PRODUCT_EMBEDDED_INDUSTRY                   0x00000059
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
#define PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER 0x0000005F
#endif
#ifndef PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER
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
#ifndef PRODUCT_MOBILE_CORE
#define PRODUCT_MOBILE_CORE                         0x00000068
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_EVAL
#define PRODUCT_EMBEDDED_INDUSTRY_EVAL              0x00000069
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_E_EVAL
#define PRODUCT_EMBEDDED_INDUSTRY_E_EVAL            0x0000006A
#endif
#ifndef PRODUCT_EMBEDDED_EVAL
#define PRODUCT_EMBEDDED_EVAL                       0x0000006B
#endif
#ifndef PRODUCT_EMBEDDED_E_EVAL
#define PRODUCT_EMBEDDED_E_EVAL                     0x0000006C
#endif
#ifndef PRODUCT_NANO_SERVER
#define PRODUCT_NANO_SERVER                         0x0000006D
#endif
#ifndef PRODUCT_CLOUD_STORAGE_SERVER
#define PRODUCT_CLOUD_STORAGE_SERVER                0x0000006E
#endif
#ifndef PRODUCT_CORE_CONNECTED
#define PRODUCT_CORE_CONNECTED                      0x0000006F
#endif
#ifndef PRODUCT_PROFESSIONAL_STUDENT
#define PRODUCT_PROFESSIONAL_STUDENT                0x00000070
#endif
#ifndef PRODUCT_CORE_CONNECTED_N
#define PRODUCT_CORE_CONNECTED_N                    0x00000071
#endif
#ifndef PRODUCT_PROFESSIONAL_STUDENT_N
#define PRODUCT_PROFESSIONAL_STUDENT_N              0x00000072
#endif
#ifndef PRODUCT_CORE_CONNECTED_SINGLELANGUAGE
#define PRODUCT_CORE_CONNECTED_SINGLELANGUAGE       0x00000073
#endif
#ifndef PRODUCT_CORE_CONNECTED_COUNTRYSPECIFIC
#define PRODUCT_CORE_CONNECTED_COUNTRYSPECIFIC      0x00000074
#endif
#ifndef PRODUCT_CONNECTED_CAR
#define PRODUCT_CONNECTED_CAR                       0x00000075
#endif
#ifndef PRODUCT_INDUSTRY_HANDHELD
#define PRODUCT_INDUSTRY_HANDHELD                   0x00000076
#endif
#ifndef PRODUCT_PPI_PRO
#define PRODUCT_PPI_PRO                             0x00000077
#endif
#ifndef PRODUCT_ARM64_SERVER
#define PRODUCT_ARM64_SERVER                        0x00000078
#endif
#ifndef PRODUCT_EDUCATION
#define PRODUCT_EDUCATION                           0x00000079
#endif
#ifndef PRODUCT_EDUCATION_N
#define PRODUCT_EDUCATION_N                         0x0000007A
#endif
#ifndef PRODUCT_IOTUAP
#define PRODUCT_IOTUAP                              0x0000007B
#endif
#ifndef PRODUCT_CLOUD_HOST_INFRASTRUCTURE_SERVER
#define PRODUCT_CLOUD_HOST_INFRASTRUCTURE_SERVER    0x0000007C
#endif
#ifndef PRODUCT_ENTERPRISE_S
#define PRODUCT_ENTERPRISE_S                        0x0000007D
#endif
#ifndef PRODUCT_ENTERPRISE_S_N
#define PRODUCT_ENTERPRISE_S_N                      0x0000007E
#endif
#ifndef PRODUCT_PROFESSIONAL_S
#define PRODUCT_PROFESSIONAL_S                      0x0000007F
#endif
#ifndef PRODUCT_PROFESSIONAL_S_N
#define PRODUCT_PROFESSIONAL_S_N                    0x00000080
#endif
#ifndef PRODUCT_ENTERPRISE_S_EVALUATION
#define PRODUCT_ENTERPRISE_S_EVALUATION             0x00000081
#endif
#ifndef PRODUCT_ENTERPRISE_S_N_EVALUATION
#define PRODUCT_ENTERPRISE_S_N_EVALUATION           0x00000082
#endif
#ifndef PRODUCT_IOTENTERPRISE
#define PRODUCT_IOTENTERPRISE                       0x000000BC
#endif
#ifndef PRODUCT_IOTENTERPRISES
#define PRODUCT_IOTENTERPRISES                      0x000000BF
#endif
#ifndef PRODUCT_IOTENTERPRISESK
#define PRODUCT_IOTENTERPRISESK                     0x000000CD
#endif
#ifndef PRODUCT_AZURESTACKHCI_SERVER_CORE
#define PRODUCT_AZURESTACKHCI_SERVER_CORE           0x00000196
#endif
#ifndef PRODUCT_DATACENTER_SERVER_AZURE_EDITION
#define PRODUCT_DATACENTER_SERVER_AZURE_EDITION     0x00000197
#endif
#ifndef PRODUCT_DATACENTER_SERVER_CORE_AZURE_EDITION
#define PRODUCT_DATACENTER_SERVER_CORE_AZURE_EDITION 0x00000198
#endif


// new Architecture(s)
#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64            12
#endif


/* HAVE_DECL__XGETBV should be set by autoconf or in boinc_win.h */
#if !defined(HAVE_DECL__XGETBV) || !HAVE_DECL__XGETBV
#if HAVE_DECL_XGETBV
#define _xgetbv(x) xgetbv(x)
#elif HAVE_DECL___XGETBV
#define _xgetbv(x) __xgetbv(x)
#else
static unsigned long long _xgetbv(unsigned int index){
      unsigned int A=0, D=0;

#ifdef __GNUC__
  #ifdef ASM_SUPPORTS_XGETBV
      __asm__ __volatile__("xgetbv" : "=a"(A), "=d"(D) : "c"(index));
  #else
      __asm__ __volatile__(".byte 0x0f, 0x01, 0xd0": "=a"(A), "=d"(D) : "c"(index));
  #endif
#elif defined(_MSC_VER)
  #ifdef _M_IX86
      __asm {
                       mov ecx,index
                       __emit 00fh
                       __emit 001h
                       __emit 0d0h
                       mov D,edx
                       mov A,eax
       }
  #elif defined(_M_AMD64)
      // damn Microsoft for not having inline assembler in 64-bit code
      // so this is in an NASM compiled library
      return asm_xgetbv(index);
  #endif
#endif
      return ((unsigned long long)D << 32) | A;
}
#endif
#endif

/* HAVE_DECL___CPUID should be set by autoconf or in boinc_win.h */
#if !defined(HAVE_DECL___CPUID) || !HAVE_DECL___CPUID
#if HAVE_DECL_CPUID
#define __cpuid(x,y) cpuid(x,y)
#elif HAVE_DECL__CPUID
#define __cpuid(x,y) _cpuid(x,y)
#else
static void __cpuid(unsigned int cpuinfo[4], unsigned int type)  {
#ifdef __GNUC__
  #ifdef ASM_SUPPORTS_CPUID
      __asm__ __volatile__("cpuid"
                            : "=a" (cpuinfo[0]), "=b" (cpuinfo[1]),
                              "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
                            : "a" (type));
  #else
      __asm__ __volatile__(".byte 0x0f, 0xa2"
                            : "=a" (cpuinfo[0]), "=b" (cpuinfo[1]),
                              "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
                            : "a" (type));
  #endif
#elif defined(_MSC_VER)
  #ifdef _M_IX86
      __asm {
                       mov eax,type
                       __emit 00fh
                       __emit 0a2h
                       mov cpuinfo[0],eax
                       mov cpuinfo[1],ebx
                       mov cpuinfo[2],ecx
                       mov cpuinfo[3],edx
       }
  #elif defined(_M_AMD64)
      // damn Microsoft for not having inline assembler in 64-bit code
      // so this is in an NASM compiled library
      asm_cpuid(cpuinfo, type);
  #endif
#endif
}
#endif
#endif

// Returns the number of seconds difference from UTC
//
int get_timezone(int& tz) {
    TIME_ZONE_INFORMATION tzi;
    memset(&tzi, 0, sizeof(TIME_ZONE_INFORMATION));
    DWORD result = GetTimeZoneInformation(&tzi);
    if (result == TIME_ZONE_ID_DAYLIGHT) {
        tz = -(tzi.Bias + tzi.DaylightBias) * 60;
    } else {
        tz = -(tzi.Bias + tzi.StandardBias) * 60;
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

typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

BOOL get_OSVERSIONINFO(OSVERSIONINFOEX& osvi) {
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.
    BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);
    if (!bOsVersionInfoEx) {
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);
    }
    return bOsVersionInfoEx;
}

int get_os_information(
    char* os_name, const int os_name_size, char* os_version, const int os_version_size
) {
    // This code snip-it was copied straight out of the MSDN Platform SDK
    //   Getting the System Version example and modified to dump the output
    //   into os_name.
    char szVersion[128];
    char szSKU[128];
    char szServicePack[128];
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    PGPI pGPI;
    DWORD dwType = 0;

    ZeroMemory(szVersion, sizeof(szVersion));
    ZeroMemory(szSKU, sizeof(szSKU));
    ZeroMemory(szServicePack, sizeof(szServicePack));
    ZeroMemory(&si, sizeof(SYSTEM_INFO));


    // GetProductInfo is a Vista+ API
    pGPI = (PGPI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetProductInfo");

    BOOL bOsVersionInfoEx = get_OSVERSIONINFO(osvi);

    GetNativeSystemInfo(&si);


    // Windows is a Microsoft OS
    strlcpy(os_name, "Microsoft ", os_name_size);

    switch (osvi.dwPlatformId) {
        case VER_PLATFORM_WIN32_NT:

            if ( osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0) {
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {
                    if ( osvi.dwBuildNumber >= 22000 ) {
                        strlcat(os_name, "Windows 11", os_name_size);
                    } else {
                        strlcat(os_name, "Windows 10", os_name_size);
                    }
                } else {
                    if (osvi.dwBuildNumber >= 26100) {
			            strlcat(os_name, "Windows Server 2025", os_name_size);
		            } else if (osvi.dwBuildNumber >= 25398) {
                        strlcat(os_name, "Windows Server 23H2", os_name_size);
                    } else if (osvi.dwBuildNumber >= 20348) {
                        strlcat(os_name, "Windows Server 2022", os_name_size);
                    } else if ( osvi.dwBuildNumber >= 17623) {
                        strlcat(os_name, "Windows Server 2019", os_name_size);
                    } else {
                        strlcat(os_name, "Windows Server 2016", os_name_size);
                    }
                }
                pGPI( 10, 0, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 4 ) {
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strlcat(os_name, "Windows 10 Beta", os_name_size);
                } else {
                    strlcat(os_name, "Windows 10 Server Beta", os_name_size);
                }
                pGPI( 6, 4, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3 ) {
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strlcat(os_name, "Windows 8.1", os_name_size);
                } else {
                    strlcat(os_name, "Windows Server 2012 R2", os_name_size);
                }
                pGPI( 6, 3, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 ) {
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strlcat(os_name, "Windows 8", os_name_size);
                } else {
                    strlcat(os_name, "Windows Server 2012", os_name_size);
                }
                pGPI( 6, 2, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 ) {
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strlcat(os_name, "Windows 7", os_name_size);
                } else {
                    strlcat(os_name, "Windows Server 2008 \"R2\"", os_name_size);
                }
                pGPI( 6, 1, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 ) {
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {
                    strlcat(os_name, "Windows Vista", os_name_size);
                } else {
                    strlcat(os_name, "Windows Server 2008", os_name_size);
                }
                pGPI( 6, 0, 0, 0, &dwType);
            }

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ) {
                if ( osvi.wProductType == VER_NT_WORKSTATION) {
                    strlcat(os_name, "Windows XP", os_name_size);
                } else {
                    if ( GetSystemMetrics(SM_SERVERR2) ) {
                        strlcat(os_name, "Windows Server 2003 \"R2\"", os_name_size);
                    } else {
                        strlcat(os_name, "Windows Server 2003", os_name_size);
                    }
                }
            }

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) {
                strlcat(os_name, "Windows XP", os_name_size);
            }

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {
                strlcat(os_name, "Windows 2000", os_name_size);
            }

            if ( osvi.dwMajorVersion <= 4 ) {
                strlcat(os_name, "Windows NT", os_name_size);
            }

            break;

        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
                strlcat(os_name, "Windows 95", os_name_size);
            }

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
                strlcat( os_name, "Windows 98", os_name_size);
            }

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
                strlcat( os_name, "Windows Millennium", os_name_size);
            }

            break;

        case VER_PLATFORM_WIN32s:

            strlcat( os_name, "Win32s", os_name_size);
            break;
    }


    snprintf( szVersion, sizeof(szVersion), ", (%.2u.%.2u.%.4u.%.2u)",
        osvi.dwMajorVersion, osvi.dwMinorVersion, (osvi.dwBuildNumber & 0xFFFF), 0
    );


    switch (osvi.dwPlatformId) {
        // Test for the Windows NT product family.
        case VER_PLATFORM_WIN32_NT:

            // Test for specific product on Windows NT 4.0 SP6 and later.
            if ( bOsVersionInfoEx ) {

                // Test for the workstation type.
                if ( osvi.wProductType == VER_NT_WORKSTATION ) {

                    // all NT6 or higher have dwType (Vista,7,8,81,10...)
                    if ( (osvi.dwMajorVersion >= 6) ) {
                        switch(dwType) {
                            case PRODUCT_BUSINESS:
                                safe_strcat(szSKU, "Business ");
                                break;
                            case PRODUCT_BUSINESS_N:
                                safe_strcat(szSKU, "Business N ");
                                break;
                            case PRODUCT_CORE:
                                safe_strcat(szSKU, "Core ");
                                break;
                            case PRODUCT_CORE_ARM:
                                safe_strcat(szSKU, "Core ");
                                break;
                            case PRODUCT_CORE_COUNTRYSPECIFIC:
                                safe_strcat(szSKU, "Core ");  // specific W8 for China
                                break;
                            case PRODUCT_CORE_N:
                                safe_strcat(szSKU, "Core N ");
                                break;
                            case PRODUCT_EDUCATION:
                                safe_strcat(szSKU, "Education ");
                                break;
                            case PRODUCT_EDUCATION_N:
                                safe_strcat(szSKU, "Education N ");
                                break;
                            case PRODUCT_EMBEDDED:
                                safe_strcat(szSKU, "Embedded Standard ");
                                break;
                            case PRODUCT_ENTERPRISE:
                                safe_strcat(szSKU, "Enterprise ");
                                break;
                            case PRODUCT_ENTERPRISE_E:
                                safe_strcat(szSKU, "Enterprise E ");
                                break;
                            case PRODUCT_ENTERPRISE_N:
                                safe_strcat(szSKU, "Enterprise N ");
                                break;
                            case PRODUCT_ENTERPRISE_N_EVALUATION:
                                safe_strcat(szSKU, "Enterprise N (Evaluation) ");
                                break;
                            case PRODUCT_ENTERPRISE_S:
                                safe_strcat(szSKU, "Enterprise LTSB ");
                                break;
                            case PRODUCT_ENTERPRISE_S_EVALUATION:
                                safe_strcat(szSKU, "Enterprise LTSB Evaluation ");
                                break;
                            case PRODUCT_ENTERPRISE_S_N:
                                safe_strcat(szSKU, "Enterprise LTSB N ");
                                break;
                            case PRODUCT_ENTERPRISE_S_N_EVALUATION:
                                safe_strcat(szSKU, "Enterprise LTSB N Evaluation ");
                                break;
                            case PRODUCT_HOME_BASIC:
                                safe_strcat(szSKU, "Home Basic ");
                                break;
                            case PRODUCT_HOME_BASIC_E:
                                safe_strcat(szSKU, "Home Basic E ");
                                break;
                            case PRODUCT_HOME_BASIC_N:
                                safe_strcat(szSKU, "Home Basic N ");
                                break;
                            case PRODUCT_HOME_PREMIUM:
                                safe_strcat(szSKU, "Home Premium ");
                                break;
                            case PRODUCT_HOME_PREMIUM_E:
                                safe_strcat(szSKU, "Home Premium E ");
                                break;
                            case PRODUCT_HOME_PREMIUM_N:
                                safe_strcat(szSKU, "Home Premium N ");
                                break;
                            case PRODUCT_IOTENTERPRISE:
                                safe_strcat(szSKU, "IoT Enterprise ");
                                break;
                            case PRODUCT_IOTENTERPRISES:
                                safe_strcat(szSKU, "IoT Enterprise LTSC ");
                                break;
                            case PRODUCT_IOTENTERPRISESK:
                                safe_strcat(szSKU, "IoT Enterprise Subscription LTSC ");
                                break;
                            case PRODUCT_IOTUAP:
                                safe_strcat(szSKU, "Internet of Things ");
                                break;
                            case PRODUCT_PRERELEASE:
                                safe_strcat(szSKU, "Developer Preview ");
                                break;
                            case PRODUCT_PRERELEASE_N:
                                safe_strcat(szSKU, "Developer Preview N ");
                                break;
                            case PRODUCT_PROFESSIONAL:
                                safe_strcat(szSKU, "Professional ");
                                break;
                            case PRODUCT_PROFESSIONAL_E:
                                safe_strcat(szSKU, "Professional E ");
                                break;
                            case PRODUCT_PROFESSIONAL_N:
                                safe_strcat(szSKU, "Professional N ");
                                break;
                            case PRODUCT_PROFESSIONAL_S:
                                safe_strcat(szSKU, "Professional S ");
                                break;
                            case PRODUCT_PROFESSIONAL_S_N:
                                safe_strcat(szSKU, "Professional SN "); //??
                                break;
                            case PRODUCT_PROFESSIONAL_WMC:
                                safe_strcat(szSKU, "Professional with Media Center ");
                                break;
                            case PRODUCT_STARTER:
                                safe_strcat(szSKU, "Starter ");
                                break;
                            case PRODUCT_STARTER_E:
                                safe_strcat(szSKU, "Starter E ");
                                break;
                            case PRODUCT_STARTER_N:
                                safe_strcat(szSKU, "Starter N ");
                                break;
                            case PRODUCT_THINPC:
                                safe_strcat(szSKU, "ThinPC ");
                                break;
                            case PRODUCT_ULTIMATE:
                                safe_strcat(szSKU, "Ultimate ");
                                break;
                            case PRODUCT_ULTIMATE_E:
                                safe_strcat(szSKU, "Ultimate E ");
                                break;
                            case PRODUCT_ULTIMATE_N:
                                safe_strcat(szSKU, "Ultimate N ");
                                break;
                            case PRODUCT_PRO_WORKSTATION:
                                safe_strcat(szSKU, "Pro for Workstations ");
                                break;
                            case PRODUCT_PRO_WORKSTATION_N:
                                safe_strcat(szSKU, "Pro for Workstations N ");
                                break;
                        }

                    } else if ( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ) {

                        if ( osvi.wSuiteMask & VER_SUITE_PERSONAL ) {
                            safe_strcat(szSKU, "Home ");
                        } else {
                            safe_strcat(szSKU, "Professional ");
                        }

                    } else if ( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) ) {

                        if ( osvi.wSuiteMask & VER_SUITE_PERSONAL ) {
                            safe_strcat(szSKU, "Home ");
                        } else if ( GetSystemMetrics(SM_TABLETPC) ) {
                            safe_strcat(szSKU, "Tablet PC ");
                        } else if ( GetSystemMetrics(SM_MEDIACENTER) ) {
                            safe_strcat(szSKU, "Media Center ");
                        } else if ( GetSystemMetrics(SM_STARTER) ) {
                            safe_strcat(szSKU, "Starter ");
                        } else {
                            safe_strcat(szSKU, "Professional ");
                        }

                    } else if ( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) ) {

                        safe_strcat(szSKU, "Professional ");

                    } else if (  (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) ) {

                        safe_strcat(szSKU, "Workstation ");

                    }
                }

				// Test for the server type.
                else if ( (osvi.wProductType == VER_NT_SERVER) || (osvi.wProductType == VER_NT_DOMAIN_CONTROLLER) ) {

                    // all NT6 or higher (Server 2008,2008r2,2012,2012r2,2015...)
                    if ( (osvi.dwMajorVersion >= 6) ) {
                        switch(dwType) {
                            case PRODUCT_ARM64_SERVER:
                                safe_strcat(szSKU, "ARM64 Server ");
                                break;
                            case PRODUCT_AZURESTACKHCI_SERVER_CORE:
                                safe_strcat(szSKU, "Azure Stack HCI ");
                                break;
                            case PRODUCT_CLOUD_HOST_INFRASTRUCTURE_SERVER:
                                safe_strcat(szSKU, "Cloud Host Infrastructure Server ");
                                break;
                            case PRODUCT_CLOUD_STORAGE_SERVER:
                                safe_strcat(szSKU, "Cloud Storage Server ");
                                break;
                            case PRODUCT_CLUSTER_SERVER:
                                safe_strcat(szSKU, "Cluster Server ");
                                break;
                            case PRODUCT_CLUSTER_SERVER_V:
                                safe_strcat(szSKU, "Cluster Server (without Hyper-V) ");
                                break;
                            case PRODUCT_DATACENTER_EVALUATION_SERVER:
                                safe_strcat(szSKU, "Datacenter (Evaluation) ");
                                break;
                            case PRODUCT_DATACENTER_A_SERVER_CORE:
                                safe_strcat(szSKU, "Datacenter, Semi-Annual Channel (core installation) ");
                                break;
                            case PRODUCT_STANDARD_A_SERVER_CORE:
                                safe_strcat(szSKU, "Standard, Semi-Annual Channel (core installation) ");
                                break;
                            case PRODUCT_DATACENTER_SERVER:
                                safe_strcat(szSKU, "Datacenter ");
                                break;
                            case PRODUCT_DATACENTER_SERVER_AZURE_EDITION:
                                safe_strcat(szSKU, "Datacenter Azure ");
                                break;
                            case PRODUCT_DATACENTER_SERVER_CORE:
                                safe_strcat(szSKU, "Datacenter (core installation) ");
                                break;
                            case PRODUCT_DATACENTER_SERVER_CORE_AZURE_EDITION:
                                safe_strcat(szSKU, "Datacenter Azure (core installation) ");
                                break;
                            case PRODUCT_DATACENTER_SERVER_CORE_V:
                                safe_strcat(szSKU, "Datacenter (core installation without Hyper-V) ");
                                break;
                            case PRODUCT_DATACENTER_SERVER_V:
                                safe_strcat(szSKU, "Datacenter (without Hyper-V) ");
                                break;
                            case PRODUCT_ENTERPRISE_EVALUATION:
                                safe_strcat(szSKU, "Enterprise (Evaluation) ");
                                break;
                            case PRODUCT_ENTERPRISE_SERVER:
                                safe_strcat(szSKU, "Enterprise ");
                                break;
                            case PRODUCT_ENTERPRISE_SERVER_CORE:
                                safe_strcat(szSKU, "Enterprise (core installation) ");
                                break;
                            case PRODUCT_ENTERPRISE_SERVER_CORE_V:
                                safe_strcat(szSKU, "Enterprise (core installation without Hyper-V) ");
                                break;
                            case PRODUCT_ENTERPRISE_SERVER_IA64:
                                safe_strcat(szSKU, "Enterprise ");
                                break;
                            case PRODUCT_ENTERPRISE_SERVER_V:
                                safe_strcat(szSKU, "Enterprise (without Hyper-V) ");
                                break;
                            case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
                                safe_strcat(szSKU, "Essential Server Solution Additional ");
                                break;
                            case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
                                safe_strcat(szSKU, "Essential Server Solution Additional SVC ");
                                break;
                            case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
                                safe_strcat(szSKU, "Essential Server Solution Management ");
                                break;
                            case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
                                safe_strcat(szSKU, "Essential Server Solution Management SVC ");
                                break;
                            case PRODUCT_HOME_PREMIUM_SERVER:
                                safe_strcat(szSKU, "Home Server 2011");
                                break;
                            case PRODUCT_HOME_SERVER:
                                safe_strcat(szSKU, "Storage Server Essentials ");
                                break;
                            case PRODUCT_HYPERV:
                                safe_strcat(szSKU, "Hyper-V ");
                                break;
                            case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
                                safe_strcat(szSKU, "Essential Business Server Management Server ");
                                break;
                            case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
                                safe_strcat(szSKU, "Essential Business Server Messaging Server ");
                                break;
                            case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
                                safe_strcat(szSKU, "Essential Business Server Security Server ");
                                break;
                            case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
                                safe_strcat( szSKU, "MultiPoint Server Premium ");
                                break;
                            case PRODUCT_MULTIPOINT_STANDARD_SERVER:
                                safe_strcat( szSKU, "MultiPoint Server Standard ");
                                break;
                            case PRODUCT_NANO_SERVER:
                                safe_strcat(szSKU, "Nano Server ");
                                break;
                            case PRODUCT_SERVER_FOR_SMALLBUSINESS:
                                safe_strcat( szSKU, "Essential Server Solutions ");
                                break;
                            case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
                                safe_strcat( szSKU, "Essential Server Solutions (without Hyper-V) ");
                                break;
                            case PRODUCT_SERVER_FOUNDATION:
                                safe_strcat( szSKU, "Foundation ");
                                break;
                            case PRODUCT_SMALLBUSINESS_SERVER:
                                safe_strcat( szSKU, "Small Business Server");
                                break;
                            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                                safe_strcat( szSKU, "Small Business Server Premium ");
                                break;
                            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
                                safe_strcat( szSKU, "Small Business Server Premium (core installation) ");
                                break;
                            case PRODUCT_SOLUTION_EMBEDDEDSERVER:
                                safe_strcat( szSKU, "MultiPoint Server ");
                                break;
                            case PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE:
                                safe_strcat( szSKU, "MultiPoint Server (core installation) ");
                                break;
                            case PRODUCT_STANDARD_EVALUATION_SERVER:
                                safe_strcat(szSKU, "Standard (Evaluation) ");
                                break;
                            case PRODUCT_STANDARD_SERVER:
                                safe_strcat(szSKU, "Standard ");
                                break;
                            case PRODUCT_STANDARD_SERVER_CORE:
                                safe_strcat(szSKU, "Standard (core installation) ");
                                break;
                            case PRODUCT_STANDARD_SERVER_CORE_V:
                                safe_strcat(szSKU, "Standard (core installation without Hyper-V) ");
                                break;
                            case PRODUCT_STANDARD_SERVER_V:
                                safe_strcat(szSKU, "Standard (without Hyper-V) ");
                                break;
                            case PRODUCT_STORAGE_ENTERPRISE_SERVER:
                                safe_strcat( szSKU, "Storage Server Enterprise ");
                                break;
                            case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
                                safe_strcat( szSKU, "Storage Server Enterprise (core installation) ");
                                break;
                            case PRODUCT_STORAGE_EXPRESS_SERVER:
                                safe_strcat( szSKU, "Storage Server Express ");
                                break;
                            case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
                                safe_strcat( szSKU, "Storage Server Express (core installation) ");
                                break;
                            case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
                                safe_strcat( szSKU, "Storage Server Standard (Evaluation) ");
                                break;
                            case PRODUCT_STORAGE_STANDARD_SERVER:
                                safe_strcat( szSKU, "Storage Server Standard ");
                                break;
                            case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
                                safe_strcat( szSKU, "Storage Server Standard (core installation) ");
                                break;
                            case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
                                safe_strcat( szSKU, "Storage Server Workgroup (Evaluation) ");
                                break;
                            case PRODUCT_STORAGE_WORKGROUP_SERVER:
                                safe_strcat( szSKU, "Storage Server Workgroup ");
                                break;
                            case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
                                safe_strcat( szSKU, "Storage Server Workgroup (core installation) ");
                                break;
                            case PRODUCT_WEB_SERVER:
                                safe_strcat(szSKU, "Web Server ");
                                break;
                            case PRODUCT_WEB_SERVER_CORE:
                                safe_strcat(szSKU, "Web Server (core installation) ");
                                break;
                            case PRODUCT_SB_SOLUTION_SERVER:
                                safe_strcat(szSKU, "Small Business Server Essentials ");
                                break;
                            case PRODUCT_SB_SOLUTION_SERVER_EM:
                                safe_strcat(szSKU, "Server For SB Solutions EM ");
                                break;
                            case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
                                safe_strcat(szSKU, "Server For SB Solutions ");
                                break;
                            case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
                                safe_strcat(szSKU, "Server For SB Solutions EM ");
                                break;
                            case PRODUCT_STANDARD_SERVER_SOLUTIONS:
                                safe_strcat(szSKU, "Server Solutions Premium ");
                                break;
                            case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
                                safe_strcat(szSKU, "Server Solutions Premium (core installation) ");
                                break;
                        }

                    } else if ( (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ) {

                        if ( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                            safe_strcat( szSKU, "Datacenter Server " );
                        } else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                            safe_strcat( szSKU, "Enterprise Server " );
                        } else if ( osvi.wSuiteMask & VER_SUITE_BLADE ) {
                            safe_strcat( szSKU, "Web Server " );
                        } else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER ) {
                            safe_strcat( szSKU, "Home Server " );
                        } else {
                            safe_strcat( szSKU, "Standard Server " );
                        }

                    } else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {

                        if ( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                            safe_strcat( szSKU, "Datacenter Server " );
                        } else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                            safe_strcat( szSKU, "Advanced Server " );
                        } else {
                            safe_strcat( szSKU, "Standard Server " );
                        }

                    } else { // Windows NT 4.0
                        if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                            safe_strcat( szSKU, "Enterprise Server " );
                        } else {
                            safe_strcat( szSKU, "Server " );
                        }
                    }
                }

                switch (si.wProcessorArchitecture) {
                    case PROCESSOR_ARCHITECTURE_INTEL:
                        safe_strcat(szSKU, "x86 ");
                        break;
                    case PROCESSOR_ARCHITECTURE_MIPS:
                        safe_strcat(szSKU, "MIPS ");
                        break;
                    case PROCESSOR_ARCHITECTURE_ALPHA:
                        safe_strcat(szSKU, "Alpha ");
                        break;
                    case PROCESSOR_ARCHITECTURE_PPC:
                        safe_strcat(szSKU, "PowerPC ");
                        break;
                    case PROCESSOR_ARCHITECTURE_IA64:
                        safe_strcat(szSKU, "Itanium ");
                        break;
                    case PROCESSOR_ARCHITECTURE_ALPHA64:
                        safe_strcat(szSKU, "Alpha 64-bit ");
                        break;
                    case PROCESSOR_ARCHITECTURE_AMD64:
                        safe_strcat(szSKU, "x64 ");
                        break;
                    // could be needed for Windows RT Boinc ?
                    case PROCESSOR_ARCHITECTURE_ARM:
                        safe_strcat(szSKU, "ARM ");
                        break;
                    case PROCESSOR_ARCHITECTURE_ARM64:
                        safe_strcat(szSKU, "ARM64 ");
                        break;
                    case PROCESSOR_ARCHITECTURE_UNKNOWN:
                        safe_strcat(szSKU, "Unknown ");
                        break;
                }

                safe_strcat(szSKU, "Edition");

            } else { // Test for specific product on Windows NT 4.0 SP5 and earlier

                HKEY hKey;
                char szProductType[80];
                DWORD dwBufLen=sizeof(szProductType);
                LONG lRet;

                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                    0, KEY_QUERY_VALUE, &hKey );
                if ( lRet != ERROR_SUCCESS )
                    return FALSE;

                lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL,
                    (LPBYTE) szProductType, &dwBufLen);
                if ( (lRet != ERROR_SUCCESS) || (dwBufLen > 80) )
                    return FALSE;

                RegCloseKey( hKey );

                if ( lstrcmpi( "WINNT", szProductType) == 0 ) {
                    safe_strcpy( szSKU, "Workstation Edition" );
                } else if ( lstrcmpi( "LANMANNT", szProductType) == 0 ) {
                    safe_strcpy( szSKU, "Server Edition" );
                } else if ( lstrcmpi( "SERVERNT", szProductType) == 0 ) {
                    safe_strcpy( szSKU, "Advanced Server Edition" );
                }

            }

            // Display service pack (if any) and build number.
            if ( osvi.dwMajorVersion == 4 && lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0
            ) {
                HKEY hKey;
                LONG lRet;

                // Test for SP6 versus SP6a.
                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
                    0, KEY_QUERY_VALUE, &hKey );

                if ( lRet == ERROR_SUCCESS ) {
                    safe_strcpy( szServicePack, ", " );
                    safe_strcat( szServicePack, "Service Pack 6a" );
                } else {// Windows NT 4.0 prior to SP6a
                    if ( strlen(osvi.szCSDVersion) > 0 ) {
                        safe_strcpy( szServicePack, ", " );
                        safe_strcat( szServicePack, osvi.szCSDVersion );
                    }
                }

                RegCloseKey( hKey );

            } else { // Windows NT 3.51 and earlier or Windows 2000 and later
                if ( strlen(osvi.szCSDVersion) > 0 ) {
                    safe_strcpy( szServicePack, ", " );
                    safe_strcat( szServicePack, osvi.szCSDVersion );
                }
            }

            break;

        // Test for the Windows 95 product family.
        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
                if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                    safe_strcpy( szServicePack, "OSR2" );
            }

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
                if ( osvi.szCSDVersion[1] == 'A' )
                    safe_strcpy( szServicePack, "SE" );
            }

            break;
    }

    snprintf( os_version, os_version_size, "%s%s%s", szSKU, szServicePack, szVersion );

    return 0;
}


// Handle the cpuid instruction on supported compilers
// NOTE: This only handles structured exceptions with Microsoft compilers.
//
#if !defined(_M_ARM) && !defined(_M_ARM64)
int get_cpuid(unsigned int info_type, unsigned int& a, unsigned int& b, unsigned int& c, unsigned int& d) {


    int retval = 1;
    int CPUInfo[4] = {0, 0, 0, 0};
#ifdef _MSC_VER
    __try {
#endif
        __cpuid(CPUInfo, info_type);

        a = CPUInfo[0];
        b = CPUInfo[1];
        c = CPUInfo[2];
        d = CPUInfo[3];

        retval = 0;
#ifdef _MSC_VER
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
#endif
    return retval;
}
#endif

void get_processor_info_from_registry(const char* name, char* info, int info_size) {
    char inBuffer[BUFSIZ] = "";
    HKEY hKey;
    DWORD gotType, gotSize = BUFSIZ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (!RegQueryValueExA(hKey, name, 0, &gotType, (PBYTE)(inBuffer), &gotSize)) {
            if ((gotType == REG_SZ) && strlen(inBuffer)) {
                strlcpy(info, inBuffer, info_size);
            }
        }
        RegCloseKey(hKey);
    }
}

// Returns the processor vendor.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
int get_processor_vendor(char* name, int name_size) {
    if (!name) return ERR_INVALID_PARAM;
    if (name_size < 13) return ERR_WRONG_SIZE;

    memset(name, 0, sizeof(name_size));
#if defined(_M_ARM) || defined(_M_ARM64)
    get_processor_info_from_registry("VendorIdentifier", name, name_size);
#else
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

    if (!get_cpuid(0x00000000, eax, ebx, ecx, edx)) {
        *((int*)(name + 0)) = ebx;
        *((int*)(name + 4)) = edx;
        *((int*)(name + 8)) = ecx;
        *((int*)(name + 12)) = '\0';
    }
#endif
    return 0;
}

#if defined(_M_ARM) || defined(_M_ARM64)
int get_processor_version(char* version, int version_size)
{
    if (!version) return ERR_INVALID_PARAM;
    if (version_size < 128) return ERR_WRONG_SIZE;

    memset(version, 0, sizeof(version_size));

    get_processor_info_from_registry("Identifier", version, version_size);

    return 0;
}
#else
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
#endif


// Returns the processor name.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
int get_processor_name(char* name, int name_size) {
    if (!name) return ERR_INVALID_PARAM;
    if (name_size < 48) return ERR_WRONG_SIZE;

    memset(name, 0, sizeof(name_size));

#if defined(_M_ARM) || defined(_M_ARM64)
    get_processor_info_from_registry("ProcessorNameString", name, name_size);
#else
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    size_t i = 0;

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
#endif
    return 0;
}


// Returns the processor cache.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
//
int get_processor_cache(int& cache) {
#if defined(_M_ARM) || defined(_M_ARM64)
    DWORD buffer_size = 0;
    GetLogicalProcessorInformation(0, &buffer_size);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);

    for (DWORD i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            cache = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);
#else
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

    get_cpuid(0x80000000, eax, ebx, ecx, edx);
    if (!(eax >= 0x80000006)) return ERR_NOT_IMPLEMENTED;

    if (!get_cpuid(0x80000006, eax, ebx, ecx, edx)) {
        cache = ((ecx >> 16) & 0xffff) * 1024;
    }
#endif
    return 0;
}

#ifndef _XCR_XFEATURE_ENABLED_MASK
#define _XCR_XFEATURE_ENABLED_MASK 0
#endif
// Returns true if the AVX instruction set is supported with the current
// combination of OS and CPU.
// see: http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/
//
bool is_avx_supported() {

    bool supported = false;

#if !defined(_M_ARM) && !defined(_M_ARM64)
    // Checking for AVX on Windows requires 3 things:
    // 1) CPUID indicates that the OS uses XSAVE and XRSTORE
    //     instructions (allowing saving YMM registers on context
    //     switch)
    // 2) CPUID indicates support for AVX
    // 3) XGETBV indicates the AVX registers will be saved and
    //     restored on context switch
    //
    // Note that XGETBV is only available on 686 or later CPUs, so
    // the instruction needs to be conditionally run.
    unsigned int a, b, c, d;
    get_cpuid(1, a, b, c, d);

    bool osUsesXSAVE_XRSTORE = c & (1 << 27) || false;
    bool cpuAVXSuport = c & (1 << 28) || false;

    if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
    {
        // Check if the OS will save the YMM registers
        unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        supported = (xcrFeatureMask & 0x6) || false;
    }
#endif

    return supported;
}

// Returns the features supported by the processor, use the
// Linux CPU processor feature mnemonics.
// see: http://msdn.microsoft.com/en-us/library/hskdteyh.aspx
// see: http://www.intel.com/Assets/PDF/appnote/241618.pdf
// see: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
#define FEATURE_TEST(feature_set_supported, test, feature_name) \
    if (feature_set_supported && test) strlcat(features, feature_name, features_size)

int get_processor_features(char* vendor, char* features, int features_size) {
    if (!vendor) return ERR_INVALID_PARAM;
    if (!features) return ERR_INVALID_PARAM;
    if (features_size < 250) return ERR_WRONG_SIZE;

    memset(features, 0, sizeof(features_size));

#if defined(_M_ARM) || defined(_M_ARM64)
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_VFP_32_REGISTERS_AVAILABLE), "vfp32 ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE), "neon ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_DIVIDE_INSTRUCTION_AVAILABLE), "divide ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_64BIT_LOADSTORE_ATOMIC), "64lsatomic ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_EXTERNAL_CACHE_AVAILABLE), "excache ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE), "fmac ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_V8_INSTRUCTIONS_AVAILABLE), "v8 ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE), "crypto ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE), "crc32 ");
    FEATURE_TEST(1, IsProcessorFeaturePresent(PF_ARM_V81_ATOMIC_INSTRUCTIONS_AVAILABLE), "atomic ");
#else
    unsigned int std_eax = 0, std_ebx = 0, std_ecx = 0, std_edx = 0;
    unsigned int ext_eax = 0, ext_ebx = 0, ext_ecx = 0, ext_edx = 0;
	unsigned int struc_eax = 0, struc_ebx = 0, struc_ecx = 0, struc_edx = 0;
    unsigned int std_supported = 0, ext_supported = 0, struc_ext_supported = 0, intel_supported = 0, amd_supported = 0, hygon_supported = 0;

    if (strcmp(vendor, "GenuineIntel") == 0) {
        intel_supported = 1;
    }
    if (strcmp(vendor, "AuthenticAMD") == 0) {
        amd_supported = 1;
    }
    if (strcmp(vendor, "HygonGenuine") == 0) {
        hygon_supported = 1;
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

	get_cpuid(0x00000000, struc_eax, struc_ebx, struc_ecx, struc_edx);
	if (struc_eax >= 0x00000007) {
		struc_ext_supported = 1;
		get_cpuid(0x00000007, struc_eax, struc_ebx, struc_ecx, struc_edx);
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
    FEATURE_TEST(std_supported, (std_ecx & (1 << 1)), "pclmulqdq ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 9)), "ssse3 ");
	FEATURE_TEST(std_supported, (std_ecx & (1 << 12)), "fma ");
	FEATURE_TEST(std_supported, (std_ecx & (1 << 13)), "cx16 ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 19)), "sse4_1 ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 20)), "sse4_2 ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 22)), "movebe ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 23)), "popcnt ");
    FEATURE_TEST(std_supported, (std_ecx & (1 << 25)), "aes ");
	FEATURE_TEST(std_supported, (std_ecx & (1 << 29)), "f16c ");
	FEATURE_TEST(std_supported, (std_ecx & (1 << 30)), "rdrand");

    FEATURE_TEST(ext_supported, (ext_edx & (1 << 11)), "syscall ");
    FEATURE_TEST(ext_supported, (ext_edx & (1 << 20)), "nx ");
    FEATURE_TEST(ext_supported, (ext_edx & (1 << 29)), "lm ");

    if (is_avx_supported()) {
        FEATURE_TEST(std_supported, (std_ecx & (1 << 28)), "avx ");
    }

    if (is_avx_supported() && struc_ext_supported) {
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 5)), "avx2 ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 16)), "avx512f ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 17)), "avx512dq ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 19)), "adx ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 21)), "avx512ifma ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 26)), "avx512pf ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 27)), "avx512er ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 28)), "avx512cd ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 30)), "avx512bw ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 31)), "avx512vl ");

		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 1)), "avx512vbmi ");
		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 6)), "avx512_vbmi2 ");
		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 8)), "gfni ");
		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 9)), "vaes ");
		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 10)), "vpclmulqdq ");
		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 11)), "avx512_vnni ");
		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 12)), "avx512_bitalg ");
		FEATURE_TEST(struc_ext_supported, (struc_ecx & (1 << 14)), "avx512_vpopcntdq ");
    }

    if (intel_supported) {
        // Intel only features
        FEATURE_TEST(std_supported, (std_ecx & (1 << 5)), "vmx ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 6)), "smx ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 8)), "tm2 ");
        FEATURE_TEST(std_supported, (std_ecx & (1 << 18)), "dca ");

        FEATURE_TEST(std_supported, (std_edx & (1 << 31)), "pbe ");
    }

    if (amd_supported || hygon_supported) {
        // AMD or Hygon features
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 2)), "svm ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 6)), "sse4a ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 9)), "osvw ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 10)), "ibs ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 11)), "xop ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 12)), "skinit ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 13)), "wdt ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 15)), "lwp ");
        FEATURE_TEST(ext_supported, (ext_ecx & (1 << 16)), "fma4 ");
		FEATURE_TEST(ext_supported, (ext_ecx & (1 << 17)), "tce ");
		FEATURE_TEST(ext_supported, (ext_ecx & (1 << 18)), "cvt16 ");
		FEATURE_TEST(ext_supported, (ext_ecx & (1 << 21)), "tbm ");
		FEATURE_TEST(ext_supported, (ext_ecx & (1 << 22)), "topx ");

        FEATURE_TEST(ext_supported, (ext_edx & (1 << 26)), "page1gb ");
        FEATURE_TEST(ext_supported, (ext_edx & (1 << 27)), "rdtscp ");
        FEATURE_TEST(ext_supported, (ext_edx & (1 << 30)), "3dnowext ");
        FEATURE_TEST(ext_supported, (ext_edx & (1 << 31)), "3dnow ");
    }

	if (struc_ext_supported) {
		// Structured Ext. Feature Flags
		// used by newer Intel and newer AMD CPUs
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 0)), "fsgsbase ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 3)), "bmi1 ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 4)), "hle ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 7)), "smep ");
		FEATURE_TEST(struc_ext_supported, (struc_ebx & (1 << 8)), "bmi2 ");
	}
#endif

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
    char vendor_name[256];
    get_processor_vendor(vendor_name, sizeof(vendor_name));
    snprintf(p_vendor, p_vendor_size, "%s", vendor_name);

    char processor_name[256];
    get_processor_name(processor_name, sizeof(processor_name));
#if defined(_M_ARM) || defined(_M_ARM64)
    char processor_version[256];
    get_processor_version(processor_version, sizeof(processor_version));
    snprintf(p_model, p_model_size,
        "%s [%s]",
        processor_name, processor_version
    );
#else
    int family = 0, model = 0, stepping = 0;
    get_processor_version(family, model, stepping);
    snprintf(p_model, p_model_size,
        "%s [Family %d Model %d Stepping %d]",
        processor_name, family, model, stepping
    );
#endif

    int cache = 0;
    get_processor_cache(cache);
    p_cache = (double)cache;

    char features[P_FEATURES_SIZE];
    get_processor_features(vendor_name, features, sizeof(features));
    snprintf(p_features, p_features_size, "%s", features);

    get_processor_count(p_ncpus);

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

    free(pIfTable);
    pIfTable = NULL;

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

    safe_strcpy(virtualbox_version, "");

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Oracle\\VirtualBox",
        0, KEY_QUERY_VALUE, &hKey
    );
    if (lRet == ERROR_SUCCESS) {
        lRet = RegQueryValueEx(hKey, "InstallDir", NULL, NULL,
            (LPBYTE) szInstallDir, &dwInstallDir
        );
        if ((lRet != ERROR_SUCCESS) || (dwInstallDir > sizeof(szInstallDir))) {
            return 1;
        }

        lRet = RegQueryValueEx(
            hKey, "VersionExt", NULL, NULL, (LPBYTE) szVersion, &dwVersion
        );
        if ((lRet != ERROR_SUCCESS) || (dwVersion > sizeof(szVersion))) {
            lRet = RegQueryValueEx(
                hKey, "Version", NULL, NULL, (LPBYTE) szVersion, &dwVersion
            );
            if ((lRet != ERROR_SUCCESS) || (dwVersion > sizeof(szVersion))) {
                return 1;
            }
        }

        safe_strcat(szInstallDir, "\\virtualbox.exe");

        if (boinc_file_exists(szInstallDir)) {
            safe_strcpy(virtualbox_version, szVersion);
        }
    }

    RegCloseKey( hKey );
    return 0;
}

// get info about processor groups
//
static void show_proc_info(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX &pi) {
    for (int i=0; i<pi.Group.ActiveGroupCount; i++) {
        PROCESSOR_GROUP_INFO &pgi = pi.Group.GroupInfo[i];
        msg_printf(NULL, MSG_INFO, "Windows processor group %d: %d processors",
            i, pgi.ActiveProcessorCount
        );
    }
}

typedef BOOL (WINAPI *GLPI)(
    LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD
);
void HOST_INFO::win_get_processor_info() {
    n_processor_groups = 0;
    GLPI glpi = (GLPI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetLogicalProcessorInformationEx");
    if (!glpi) return;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buf[64];
    DWORD size = sizeof(buf);
    glpi(
        RelationGroup,
        buf,
        &size
    );
    char *p = (char*)buf;
    while (size > 0) {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX pi = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)p;
        show_proc_info(*pi);
        p += pi->Size;
        size -= pi->Size;
        n_processor_groups++;
    }
}

typedef BOOL (WINAPI *GPGA)(HANDLE, PUSHORT, PUSHORT);
int get_processor_group(HANDLE process_handle) {
    USHORT groups[1], count;
    static GPGA gpga = 0;
    if (!gpga) {
        gpga = (GPGA) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetProcessGroupAffinity");
    }
    if (!gpga) return 0;
    count = 1;
    BOOL ret = gpga(process_handle, &count, groups);
    if (ret && count>0) {
        return groups[0];
    }
    return -1;
}

// Gets host information; called on startup and before each sched RPC
//
int HOST_INFO::get_host_info(bool init) {
    get_timezone(timezone);
    int retval = get_filesystem_info(d_total, d_free);
    if (retval) {
        msg_printf(0, MSG_INTERNAL_ERROR,
            "get_filesystem_info(): %s", boincerror(retval)
        );
    }
    get_local_network_info();

    if (!init) return 0;
    ::get_memory_info(m_nbytes, m_swap);
    get_os_information(
        os_name, sizeof(os_name), os_version, sizeof(os_version)
    );
#ifdef _WIN64
    OSVERSIONINFOEX osvi;
    if (get_OSVERSIONINFO(osvi) && osvi.dwMajorVersion >= 10) {
        retval = get_wsl_information(wsl_distros);
        if (retval) {
            msg_printf(0, MSG_INTERNAL_ERROR,
                "get_wsl_information(): %s", boincerror(retval)
            );
        }
    }
#endif

    get_virtualbox_version();

    get_processor_info(
        p_vendor, sizeof(p_vendor),
        p_model, sizeof(p_model),
        p_features, sizeof(p_features),
        m_cache,
        p_ncpus
    );
    collapse_whitespace(p_model);
    collapse_whitespace(p_vendor);
    if (!strlen(host_cpid)) {
        generate_host_cpid();
    }
    win_get_processor_info();
    return 0;
}

bool HOST_INFO::host_is_running_on_batteries() {
    SYSTEM_POWER_STATUS Status;
    ZeroMemory(&Status, sizeof(SYSTEM_POWER_STATUS));
    if (!GetSystemPowerStatus(&Status)) {
        return false;
    }

    // Sometimes the system reports the ACLineStatus as an
    //   undocumented value, so lets check to see if the
    //   battery is charging or missing and make that part
    //   of the decision.
    bool bIsOnBatteryPower  = (Status.ACLineStatus != 1);
    bool bIsBatteryCharging = ((Status.BatteryFlag & 8) == 8);
    bool bIsBatteryMissing = ((Status.BatteryFlag & 128) == 128);

    return (bIsOnBatteryPower && !bIsBatteryCharging && !bIsBatteryMissing);
}

int HOST_INFO::get_host_battery_charge() {
    SYSTEM_POWER_STATUS Status;
    ZeroMemory(&Status, sizeof(SYSTEM_POWER_STATUS));
    if (!GetSystemPowerStatus(&Status)) {
        return false;
    }

    if (((int)Status.BatteryLifePercent) == 255) return 0;
    return ((int)Status.BatteryLifePercent);
}

int HOST_INFO::get_host_battery_state() {
    SYSTEM_POWER_STATUS Status;
    ZeroMemory(&Status, sizeof(SYSTEM_POWER_STATUS));
    if (!GetSystemPowerStatus(&Status)) {
        return BATTERY_STATE_UNKNOWN;
    }

    // Sometimes the system reports the ACLineStatus as an
    //   undocumented value, so lets check to see if the
    //   battery is charging or missing and make that part
    //   of the decision.
    bool bIsOnBatteryPower  = (Status.ACLineStatus != 1);
    bool bIsBatteryCharging = ((Status.BatteryFlag & 8) == 8);

    if        (bIsOnBatteryPower && !bIsBatteryCharging) {
        return BATTERY_STATE_DISCHARGING;
    } else if (((int)Status.BatteryLifePercent) == 100) {
        return BATTERY_STATE_FULL;
    } else if (bIsBatteryCharging) {
        return BATTERY_STATE_CHARGING;
    }
    return BATTERY_STATE_UNKNOWN;
}

long HOST_INFO::user_idle_time(bool /*check_all_logins*/) {
    return get_idle_tick_count() / 1000;
}
