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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#include <sys/stat.h>
#if HAVE_SYS_SWAP_H
#include <sys/swap.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#if HAVE_SYS_SYSTEMINFO_H
#include <sys/systeminfo.h>
#endif
#include <sys/utsname.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include "client_types.h"

// functions to get name/addr of local host

int get_local_domain_name(char* p) {
    char buf[256];

    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
    strcpy(p, he->h_name);
    return 0;
}

int get_local_ip_addr(int& p) {
    char buf[256];
    struct in_addr addr;

    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    p = addr.s_addr;
    return 0;
}

int get_local_ip_addr_str(char* p) {
    char buf[256];
    struct in_addr addr;

    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    strcpy(p, inet_ntoa(addr));
    return 0;
}

char* ip_addr_string(int ip_addr) {
    in_addr ia;

    ia.s_addr = ip_addr;
    return inet_ntoa(ia);
}


void get_timezone(int& tz) {
    tzset();

    // TODO: take daylight savings time into account
#ifdef linux
    tz = __timezone;
#endif
#ifdef solaris
    tz = timezone;
#endif
}

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
    return (i2 == 0);
}

#ifdef linux

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
//
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

void get_osinfo(HOST_INFO& host) {
    struct utsname u;
    uname(&u);
    strcpy(host.os_name, u.sysname);
    strcpy(host.os_version, u.release);
}

#endif

int get_host_info(HOST_INFO& host) {
    host.m_nbytes = 0;
    host.m_cache = 0;
#ifdef solaris
    struct statvfs foo;

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
      
    get_local_domain_name(host.domain_name);
    get_local_ip_addr_str(host.ip_addr);
			    
    parse_cpuinfo(host);
    parse_meminfo(host);
    get_osinfo(host);
    get_timezone(host.timezone);
#endif

    return 0;
}


