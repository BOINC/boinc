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

#ifdef _WIN32

#include <windows.h>
#include "client_types.h"

extern int get_local_domain_name(char* p);
extern int get_local_ip_addr_str(char* p);

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
        if ( OSVersionInfo.dwMajorVersion > 4 || ( OSVersionInfo.dwMajorVersion == 4
			&& OSVersionInfo.dwMinorVersion > 10 ) )
        {
            strcpy( host.os_name, "Windows 98" );
        } else {
            strcpy( host.os_name, "Windows 95" );
        }
        break;

    case VER_PLATFORM_WIN32_NT:
        strcpy( host.os_name, "Windows NT" );
        break;

    default:
        sprintf( host.os_name, "Unknown Win32 (%ld)", OSVersionInfo.dwPlatformId );
        break;
    }

    char Version[ 25 ];
    Version[ 0 ] = NULL;
    sprintf( Version, ": %lu.%lu", OSVersionInfo.dwMajorVersion,
        OSVersionInfo.dwMinorVersion );
    strcat( host.os_name, Version );

    SYSTEM_INFO SystemInfo;
    memset( &SystemInfo, NULL, sizeof( SystemInfo ) );
    ::GetSystemInfo( &SystemInfo );
    switch ( SystemInfo.wProcessorArchitecture ) {
    case PROCESSOR_ARCHITECTURE_INTEL:
        switch ( SystemInfo.dwProcessorType ) {
        case PROCESSOR_INTEL_386:
            strcpy( host.p_model, "Intel 80386" );
            break;
        case PROCESSOR_INTEL_486:
            strcpy( host.p_model, "Intel 80486" );
            break;
        case PROCESSOR_INTEL_PENTIUM:
            strcpy( host.p_model, "Intel Pentium" );
            break;
        default:
            strcpy( host.p_model, "Intel x86" );
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
        
    memset(&host, 0, sizeof(host));
      
    get_local_domain_name(host.domain_name);
    get_local_ip_addr_str(host.ip_addr);

    return 0;
}

#endif
