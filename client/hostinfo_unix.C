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

#include <sys/param.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_VMMETER_H
#include <sys/vmmeter.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
#define STATFS statvfs
#else
#define STATFS statfs
#endif

#include <sys/stat.h>
#if HAVE_SYS_SWAP_H
#include <sys/swap.h>
#endif

#ifdef _WIN32
#include <afxwin.h>
#include <winsock.h>
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
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
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "client_types.h"
#include "error_numbers.h"
#include "util.h"

// functions to get name/addr of local host

// Converts a int ip address to a string representation (i.e. "66.218.71.198")
//
#if 0
char* ip_addr_string(int ip_addr) {
    in_addr ia;

    ia.s_addr = ip_addr;
    return inet_ntoa(ia);
}
#endif

// Returns the number of seconds difference from UTC
//
int get_timezone( void ) {
    tzset();
    // TODO: take daylight savings time into account
#ifdef HAVE_GMTOFF
    time_t cur_time;
    struct tm *time_data;

    cur_time = time(NULL);
    time_data = localtime( &cur_time );
    return time_data->tm_gmtoff;
#else
#ifdef __timezone
    return __timezone;
#else
    return timezone;
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

// Determine the memory sizes for this host,
// including RAM and swap space
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
    safe_strcpy(host.os_name, u.sysname);
    safe_strcpy(host.os_version, u.release);
}
#endif

// Returns total and free space on current disk (in bytes)
//
void get_host_disk_info( double &total_space, double &free_space ) {
#ifdef STATFS
    struct STATFS fs_info;
    
    STATFS(".", &fs_info);
    total_space = (double)fs_info.f_bsize * (double)fs_info.f_blocks;
    free_space = (double)fs_info.f_bsize * (double)fs_info.f_bavail;
#endif
}

// General function to get all relevant host information
//
int get_host_info(HOST_INFO& host) {
    get_host_disk_info( host.d_total, host.d_free );

#ifdef linux
    parse_cpuinfo(host);
    parse_meminfo(host);
#endif

#if HAVE_SYS_SYSTEMINFO_H
    char buf[256];

    int i, n;
    sysinfo(SI_SYSNAME, host.os_name, sizeof(host.os_name));
    sysinfo(SI_RELEASE, host.os_version, sizeof(host.os_version));
    sysinfo(SI_HW_SERIAL, host.serialnum, sizeof(host.serialnum));
    host.p_ncpus = sysconf(_SC_NPROCESSORS_ONLN);

/*	There can be a variety of methods to obtain amount of
 *		usable memory.  You will have to check your sysconf()
 *		defines, probably in unistd.h
 *		- 2002-11-03 hiram@users.sourceforge.net
 */
#if defined(_SC_USEABLE_MEMORY)
    host.m_nbytes = (double)sysconf(_SC_PAGESIZE)
	* (double)sysconf(_SC_USEABLE_MEMORY);  /*      UnixWare        */
#elif defined(_SC_PHYS_PAGES)
    host.m_nbytes = (double)sysconf(_SC_PAGESIZE)
	* (double)sysconf(_SC_PHYS_PAGES);      /*      Linux   */
#else
#error Need to specify a sysconf() define to obtain memory size
#endif

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

#if HAVE_SYS_SYSCTL_H
    int mib[2], mem_size;
    size_t len;
    vmtotal vm_info;
    
    // Get number of CPUs
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(host.p_ncpus);
    sysctl(mib, 2, &host.p_ncpus, &len, NULL, 0);
    
    // Get machine
    mib[0] = CTL_HW;
    mib[1] = HW_MACHINE;
    len = sizeof(host.p_vendor);
    sysctl(mib, 2, &host.p_vendor, &len, NULL, 0);
    
    // Get model
    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    len = sizeof(host.p_model);
    sysctl(mib, 2, &host.p_model, &len, NULL, 0);
    
    // Get physical memory size
    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;
    len = sizeof(mem_size);
    sysctl(mib, 2, &mem_size, &len, NULL, 0);
    host.m_nbytes = mem_size;
    
    // Get operating system name
    mib[0] = CTL_KERN;
    mib[1] = KERN_OSTYPE;
    len = sizeof(host.os_name);
    sysctl(mib, 2, &host.os_name, &len, NULL, 0);
    
    // Get operating system version
    mib[0] = CTL_KERN;
    mib[1] = KERN_OSRELEASE;
    len = sizeof(host.os_version);
    sysctl(mib, 2, &host.os_version, &len, NULL, 0);
    
    // TODO: Get virtual memory info
    /*mib[0] = CTL_VM;
    mib[1] = VM_METER;
    len = sizeof(vm_info);
    sysctl(mib, 2, &vm_info, &len, NULL, 0);
    host.m_swap = vm_info.t_vm;*/
#endif

    get_local_domain_name(host.domain_name, sizeof(host.domain_name));
    get_local_ip_addr_str(host.ip_addr, sizeof(host.ip_addr));
    host.timezone = get_timezone();
#ifdef HAVE_SYS_UTSNAME_H
    get_osinfo(host);
#endif

    return 0;
}


