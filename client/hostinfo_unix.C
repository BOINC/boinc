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

#include "windows_cpp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#include <sys/stat.h>
#if HAVE_SYS_SWAP_H
#include <sys/swap.h>
#endif

#ifdef _WIN32
#include <afxwin.h>
#include <winsock.h>
#endif

#if HAVE_SYS_SYSTEMINFO_H
#include <sys/systeminfo.h>
#endif
#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "client_types.h"
#include "error_numbers.h"

// functions to get name/addr of local host

// Returns the domain of the local host
// TODO: Should the 256 be MAXHOSTNAMELEN instead?
//
int get_local_domain_name(char* p) {
    char buf[256];

    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
	if (!he) return -1;
    strcpy(p, he->h_name);
    return 0;
}

// Gets the ip address of the local host
//
int get_local_ip_addr(int& p) {
    p = 0;

#if HAVE_NETDB_H
    char buf[256];
    struct in_addr addr;
    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    p = addr.s_addr;
#endif
    return 0;
}

// Returns the name of the local host
// TODO: Should the 256 be MAXHOSTNAMELEN instead?
//
int get_local_ip_addr_str(char* p) {
    strcpy( p,"" );
#if HAVE_NETDB_H
    char buf[256];
    struct in_addr addr;
    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    strcpy(p, inet_ntoa(addr));
#endif
    return 0;
}

// Converts a int ip address to a string representation (i.e. "66.218.71.198")
//
char* ip_addr_string(int ip_addr) {
    in_addr ia;

    ia.s_addr = ip_addr;
    return inet_ntoa(ia);
}

// Returns the number of seconds difference from UTC
//
int get_timezone( void ) {
    tzset();
    // TODO: get this to work on all platforms
    // TODO: take daylight savings time into account
#ifdef HAVE_GMTOFF
    //time_t cur_time;
    //struct tm *time_data;

    //cur_time = time(NULL);
    //time_data = localtime( &cur_time );
    //return time_data->tm_gmtoff;
#else
#ifdef __timezone
    //return __timezone;
#else
    //return timezone;
#endif
#endif
    return 0;
}

// Returns true if the host is currently running off battery power
// TODO: port this to other platforms (Windows, Mac OS X, others?)
//
bool host_is_running_on_batteries() {
    float x1, x2;
    int i1, i2;

// the following only works on Linux.
// Need to find something else for other systems
//
    FILE* f = fopen("/proc/apm", "r");

    if (!f) return false;

    // Supposedly we're on batteries if the 4th entry is zero.
    //
    fscanf(f, "%f %f %x %x", &x1, &x2, &i1, &i2);
    fclose(f);
    return (i2 == 0);
}

#ifdef linux

// Determine the memory specifications for this host, including RAM and swap space
//
void parse_meminfo(HOST_INFO& host) {
    char buf[256];
    FILE* f = fopen("/proc/meminfo", "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (strstr(buf, "Mem:") == buf) {
            sscanf(buf, "Mem: %lf", &host.m_nbytes);
        }
        if (strstr(buf, "Swap:") == buf) {
            sscanf(buf, "Swap: %lf", &host.m_swap);
        }
    }
    fclose(f);
}

// Unfortunately the format of /proc/cpuinfo is not standardized.
// See http://people.nl.linux.org/~hch/cpuinfo/ for some examples.
// The following is for Redhat Linux 2.2.14.
// TODO: get this to work on all platforms
void parse_cpuinfo(HOST_INFO& host) {
    char buf[256];
    int n;

    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return;
    host.p_ncpus = 1;
    while (fgets(buf, 256, f)) {
        if (strstr(buf, "vendor_id\t: ") == buf) {
            sscanf(buf, "vendor_id\t: %s", host.p_vendor);
        }
        if (strstr(buf, "model name\t: ") == buf) {
            sscanf(buf, "model name\t: %s", host.p_model);
        }
        if (strstr(buf, "cache size\t: ") == buf) {
            sscanf(buf, "cache size\t: %d", &n);
            host.m_cache = n*1024;
        }
    }
    fclose(f);
}
#endif

#if HAVE_SYS_UTSNAME_H
// Puts the operating system name and version into the HOST_INFO structure
//
void get_osinfo(HOST_INFO& host) {
    struct utsname u;
    uname(&u);
    strcpy(host.os_name, u.sysname);
    strcpy(host.os_version, u.release);
}
#endif

int get_host_info2(HOST_INFO &host);

// General function to get all relevant host information
//
int get_host_info(HOST_INFO& host) {
    host.timezone = 0;		// seconds added to local time to get UTC
    strcpy(host.domain_name,"");
    strcpy(host.serialnum,"");
    strcpy(host.ip_addr,"");

    host.on_frac = 0;
    host.conn_frac = 0;
    host.active_frac = 0;

    host.p_ncpus = 0;
    strcpy(host.p_vendor,"");
    strcpy(host.p_model,"");
    host.p_fpops = 0;
    host.p_iops = 0;
    host.p_membw = 0;
    host.p_calculated = 0;
    
    strcpy(host.os_name,"");
    strcpy(host.os_version,"");

    host.m_nbytes = 0;
    host.m_cache = 0;
    host.m_swap = 0;

    host.d_total = 0;
    host.d_free = 0;

#ifdef _WIN32
    return get_host_info2( host );
#endif

#if HAVE_SYS_SYSTEMINFO_H
    struct statvfs foo;
    char buf[256];

    memset(&host, 0, sizeof(host));
    
    get_local_domain_name(host.domain_name);
    get_local_ip_addr_str(host.ip_addr);

    statvfs(".", &foo);
    host.d_total = (double)foo.f_bsize * (double)foo.f_blocks;
    host.d_free = (double)foo.f_bsize * (double)foo.f_bavail;

    int i, n;
    sysinfo(SI_SYSNAME, host.os_name, sizeof(host.os_name));
    sysinfo(SI_RELEASE, host.os_version, sizeof(host.os_version));
    sysinfo(SI_HW_SERIAL, host.serialnum, sizeof(host.serialnum));
    host.p_ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    host.m_nbytes = (double)sysconf(_SC_PAGESIZE)
        * (double)sysconf(_SC_PHYS_PAGES);

    swaptbl_t* s;
    n = swapctl(SC_GETNSWP, 0);
    s = (swaptbl_t*)malloc(n*sizeof(swapent_t) + sizeof(struct swaptable));
    for (i=0; i<n; i++) {
        s->swt_ent[i].ste_path = buf;
    }
    s->swt_n = n;
    n = swapctl(SC_LIST, s);
    for (i=0; i<n; i++) {
        host.m_swap += 512.*(double)s->swt_ent[i].ste_length;
    }
#endif

#ifdef linux
    memset(&host, 0, sizeof(host)); 
    parse_cpuinfo(host);
    parse_meminfo(host);
#endif
    get_local_domain_name(host.domain_name);
    get_local_ip_addr_str(host.ip_addr);
    host.timezone = get_timezone();
#ifdef HAVE_SYS_UTSNAME_H
    get_osinfo(host);
#endif

    return 0;
}


