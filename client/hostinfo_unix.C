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

#include "cpp.h"

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
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_VMMETER_H
#include <sys/vmmeter.h>
#endif

#include <sys/stat.h>
#if HAVE_SYS_SWAP_H
#include <sys/swap.h>
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
#include "filesys.h"
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
int get_timezone() {
    tzset();
    // TODO: take daylight savings time into account
#ifdef HAVE_STRUCT_TM_TM_ZONE
    time_t cur_time;
    struct tm *time_data;

    cur_time = time(NULL);
    time_data = localtime( &cur_time );
    return time_data->tm_gmtoff;
#elif defined(linux)
    return __timezone;
#elif defined(__CYGWIN32__)
    return _timezone;
#elif defined(unix)
    return timezone;
#else
#error timezone
#endif
    return 0;
}

// Returns true if the host is currently running off battery power
// If you can't figure out, return false
//
// TODO: port this to other platforms (Windows, Mac OS X, others?)
//
bool HOST_INFO::host_is_running_on_batteries() {
    bool    retval = false;
	char    apm_driver_version[10];
    int     apm_major_version;
    int     apm_minor_version;
    int     apm_flags;
    int     apm_ac_line_status=1;

    if (!strncasecmp(os_name, "Linux", 5)) {
        // the following only works on Linux APM systems.
        //
        FILE* f = fopen("/proc/apm", "r");
        if (f) {
            // Supposedly we're on batteries if the 5th entry is zero.
            //
            fscanf(f, "%10s %d.%d %x %x",
                &apm_driver_version,
                &apm_major_version,
                &apm_minor_version,
                &apm_flags,
                &apm_ac_line_status
            );
            fclose(f);
            retval = (apm_ac_line_status == 0);
        } else {
            // Need Linux ACPI check here...
        }
    }

    return retval;
}

#ifdef linux

#ifdef __mips__

void parse_cpuinfo(HOST_INFO& host) {
    char buf[256];
    char buf2[256];
    int system_found=0,model_found=0;

    strcpy(host.p_model, "MIPS ");

    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return;

    while (fgets(buf, 256, f)) {
        if ( (strstr(buf, "system type\t\t: ") == buf) &&
             (system_found == 0) ) {
            system_found = 1;
            strncpy(host.p_vendor, strchr(buf, ':') + 2, sizeof(host.p_vendor)-1);
            char * p = strchr(host.p_vendor, '\n');
            if (p) {
                *p = '\0';
            }
        }
        if ( (strstr(buf, "cpu model\t\t: ") == buf) &&
             (model_found == 0) ) {
            model_found = 1;
            strncpy(buf2, strchr(buf, ':') + 2, sizeof(host.p_model) - strlen(host.p_model) - 1);
            strcat(host.p_model, buf2);
            char * p = strchr(host.p_model, '\n');
            if (p) {
                *p = '\0';
            }
        }
    }

    fclose(f);
}

#else

// Unfortunately the format of /proc/cpuinfo is not standardized.
// See http://people.nl.linux.org/~hch/cpuinfo/ for some examples.
// The following is for Redhat Linux 2.2.14.
// TODO: get this to work on all platforms
//
void parse_cpuinfo(HOST_INFO& host) {
    char buf[256];
    int system_found=0,model_found=0,cache_found=0;
    int n;

    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return;

    while (fgets(buf, 256, f)) {
        if ( (strstr(buf, "vendor_id\t: ") == buf) &&
             (system_found == 0) ) {
            system_found = 1;
            strncpy(host.p_vendor, strchr(buf, ':') + 2, sizeof(host.p_vendor) - 1);
            char * p = strchr(host.p_vendor, '\n');
            if (p) {
                *p = '\0';
            }
        }
        if ( (strstr(buf, "model name\t: ") == buf) &&
             (model_found == 0) ) {
            model_found = 1;
            strncpy(host.p_model, strchr(buf, ':') + 2, sizeof(host.p_model) - 1);
            char * p = strchr(host.p_model, '\n');
            if (p) {
                *p = '\0';
            }
        }
        if ( (strstr(buf, "cache size\t: ") == buf) &&
             (cache_found == 0) ) {
            cache_found = 1;
            sscanf(buf, "cache size\t: %d", &n);
            host.m_cache = n*1024;
        }
    }

    fclose(f);
}

#endif // MIPS

#endif

// get all relevant host information
//
int HOST_INFO::get_host_info() {
    get_filesystem_info(d_total, d_free);

#ifdef linux
    parse_cpuinfo(*this);
#else
#if HAVE_SYS_SYSCTL_H
    int mib[2];
    unsigned int mem_size;
    size_t len;

    // Get machine
    mib[0] = CTL_HW;
    mib[1] = HW_MACHINE;
    len = sizeof(p_vendor);
    sysctl(mib, 2, &p_vendor, &len, NULL, 0);
    
    // Get model
    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    len = sizeof(host.p_model);
    sysctl(mib, 2, &p_model, &len, NULL, 0);
#endif
#endif
 

#if defined(_SC_NPROCESSORS_ONLN)
    p_ncpus = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(HAVE_SYS_SYSCTL_H) && defined(CTL_HW) && defined(HW_NCPU)
    // Get number of CPUs
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(p_ncpus);
    sysctl(mib, 2, &p_ncpus, &len, NULL, 0);
#else
#error Need to specify a sysconf() define to obtain number of processors
#endif

/*    There can be a variety of methods to obtain amount of
 *        usable memory.  You will have to check your sysconf()
 *        defines, probably in unistd.h
 *        - 2002-11-03 hiram@users.sourceforge.net
 */
#if defined(_SC_USEABLE_MEMORY)
    m_nbytes = (double)sysconf(_SC_PAGESIZE)
        * (double)sysconf(_SC_USEABLE_MEMORY);  /*      UnixWare        */
#elif defined(_SC_PHYS_PAGES)
    m_nbytes = (double)sysconf(_SC_PAGESIZE)
        * (double)sysconf(_SC_PHYS_PAGES);      /*      Linux   */
#elif defined(HAVE_SYS_SYSCTL_H) && defined(CTL_HW) && defined(HW_PHYSMEM)
    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;
    len = sizeof(mem_size);
    sysctl(mib, 2, &mem_size, &len, NULL, 0);    // Mac OS X
    m_nbytes = mem_size;
#else
#error Need to specify a sysconf() define to obtain memory size
#endif

#if defined(HAVE_SYS_SWAP_H) && defined(SC_GETNSWP)
    char buf[256];
    swaptbl_t* s;
    int i, n;
    n = swapctl(SC_GETNSWP, 0);
    s = (swaptbl_t*)malloc(n*sizeof(swapent_t) + sizeof(struct swaptable));
    for (i=0; i<n; i++) {
        s->swt_ent[i].ste_path = buf;
    }
    s->swt_n = n;
    n = swapctl(SC_LIST, s);
    for (i=0; i<n; i++) {
        m_swap += 512.*(double)s->swt_ent[i].ste_length;
    }
#elif defined(HAVE_SYS_SYSCTL_H) && defined(CTL_VM) && defined(VM_METER)
    // TODO: figure this out
    /*vmtotal vm_info;
   
    mib[0] = CTL_VM;
    mib[1] = VM_METER;
    len = sizeof(vm_info);
    sysctl(mib, 2, &vm_info, &len, NULL, 0);
    m_swap = vm_info.t_vm;
    */
#else
#endif

#if defined(HAVE_SYS_SYSTEMINFO_H) 
#if defined(SI_HW_SERIAL)
    sysinfo(SI_HW_SERIAL, serialnum, sizeof(serialnum));
#else
//#error Need to specify a method to obtain serial num
#endif
#ifdef SI_PLATFORM
    sysinfo(SI_PLATFORM, p_vendor, sizeof(p_vendor));
#endif
#ifdef SI_ISALIST
    sysinfo(SI_ISALIST, p_model, sizeof(p_model));
    for (unsigned int i=0; i<sizeof(p_model); i++) {
        if (p_model[i]==' ') {
            p_model[i]=0;
        }
        if (p_model[i]==0) {
            i=sizeof(p_model);
        }
    }
#endif
#endif
   
    get_local_domain_name(domain_name, sizeof(domain_name));
    get_local_ip_addr_str(ip_addr, sizeof(ip_addr));
    timezone = get_timezone();
#ifdef HAVE_SYS_UTSNAME_H
    struct utsname u;
    uname(&u);
    safe_strcpy(os_name, u.sysname);
    safe_strcpy(os_version, u.release);
#elif defined(HAVE_SYS_SYSCTL_H) && defined(CTL_KERN) && defined(KERN_OSTYPE) && defined(KERN_OSRELEASE)
    mib[0] = CTL_KERN;
    mib[1] = KERN_OSTYPE;
    len = sizeof(os_name);
    sysctl(mib, 2, &os_name, &len, NULL, 0);
    
    mib[0] = CTL_KERN;
    mib[1] = KERN_OSRELEASE;
    len = sizeof(os_version);
    sysctl(mib, 2, &os_version, &len, NULL, 0);
#elif HAVE_SYS_SYSTEMINFO_H
    sysinfo(SI_SYSNAME, os_name, sizeof(os_name));
    sysinfo(SI_RELEASE, os_version, sizeof(os_version));
#else
#error Need to specify a method to obtain OS name/version
#endif

    return 0;
}


