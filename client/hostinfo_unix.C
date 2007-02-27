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

#include "cpp.h"

#include "config.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

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
#if HAVE_UTMP_H
#include <utmp.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef __EMX__
#define INCL_DOSMISC
#include <os2.h>
#include "win/opt_x86.h"
#endif

#include "client_types.h"
#include "filesys.h"
#include "error_numbers.h"
#include "str_util.h"
#include "client_state.h"
#include "hostinfo_network.h"

using std::string;

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#ifdef __cplusplus
}	// extern "C"
#endif
#endif  // __APPLE__

#ifdef _HPUX_SOURCE
#include <sys/pstat.h>
#endif

// Tru64 UNIX.
// 2005-12-26 SMS.
#ifdef __osf__
#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>
#include <machine/cpuconf.h>
#endif

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

// Returns the offset between LOCAL STANDARD TIME and UTC.
// LOCAL_STANDARD_TIME = UTC_TIME + get_timezone().
//
int get_timezone() {
    tzset();
    // TODO: take daylight savings time into account
#ifdef HAVE_STRUCT_TM_TM_ZONE
    time_t cur_time;
    struct tm *time_data;

    cur_time = time(NULL);
    time_data = localtime( &cur_time );
    if (time_data->tm_isdst>0) {
        // daylight savings in effect
        return (time_data->tm_gmtoff)-3600;
    } else {
        // no daylight savings in effect
        return time_data->tm_gmtoff;
    }
#elif defined(linux)
    return -1*(__timezone);
#elif defined(__CYGWIN32__)
    return -1*(_timezone);
#elif defined(unix)
    return -1*timezone;
#elif defined(HAVE_TZNAME)
    return -1*timezone;
#else
#error timezone
#endif
    return 0;
}

// Returns true if the host is currently running off battery power
// If you can't figure out, return false
//
// TODO: port this to other platforms (Windows, others?)
//
bool HOST_INFO::host_is_running_on_batteries() {
#if defined(__APPLE__)
  CFDictionaryRef pSource = NULL;
  CFStringRef psState;
  int i;
  bool retval = false;
  
  CFTypeRef blob = IOPSCopyPowerSourcesInfo();
  CFArrayRef list = IOPSCopyPowerSourcesList(blob);

  for(i = 0; i < CFArrayGetCount(list); i++) {
    pSource = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(list, i));
    if(!pSource) break;
    psState = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSPowerSourceStateKey));
    if(!CFStringCompare(psState,CFSTR(kIOPSBatteryPowerValue),0))
      retval = true;
  }

  CFRelease(blob);
  CFRelease(list);
  return(retval);

#elif defined(linux)
    bool    retval = false;

    FILE* fapm = fopen("/proc/apm", "r");
    if (fapm) {         // Then we're using APM!  Yay.

        char    apm_driver_version[10];
        int     apm_major_version;
        int     apm_minor_version;
        int     apm_flags;
        int     apm_ac_line_status=1;

        // Supposedly we're on batteries if the 5th entry is zero.
        fscanf(fapm, "%10s %d.%d %x %x",
            apm_driver_version,
            &apm_major_version,
            &apm_minor_version,
            &apm_flags,
            &apm_ac_line_status
        );
        retval = (apm_ac_line_status == 0);
        fclose(fapm);
    } else {

        // we try ACPI
        char buf[128];
        char ac_state[64];
        std::string ac_name;
        FILE* facpi;

        // we need to find the right ac adapter first
        DirScanner dir("/proc/acpi/ac_adapter/");
        while (dir.scan(ac_name)) {
            if ((ac_name.c_str()==".")||(ac_name.c_str()=="..")) continue;

            // newer ACPI versions use "state" as filename
            sprintf(ac_state, "/proc/acpi/ac_adapter/%s/state", ac_name.c_str());
            facpi = fopen(ac_state, "r");
            if (!facpi) {
                // older ACPI versions use "status" instead
                sprintf(ac_state, "/proc/acpi/ac_adapter/%s/status", ac_name.c_str());
                facpi = fopen(ac_state, "r");
            }

            if (facpi) {
                fgets(buf, 128, facpi);
                fclose(facpi);

                // only valid state if it contains "state" (newer) or "Status" (older)
                if ((strstr(buf, "state:") != NULL) || (strstr(buf, "Status:") != NULL)) {
                    // on batteries if ac adapter is "off-line" (or maybe "offline")
                    retval = (strstr(buf, "off") != NULL);
                    break;
                }
            }
        }
    }

    return retval;
#else
    return false;
#endif
}

#ifdef linux

// Unfortunately the format of /proc/cpuinfo is not standardized.
// See http://people.nl.linux.org/~hch/cpuinfo/ for some examples.
//
void parse_cpuinfo(HOST_INFO& host) {
    char buf[256], features[1024], model_buf[1024];
    bool vendor_found=false, model_found=false;
    bool cache_found=false, features_found=false;
    int n;
    int family=-1, model=-1, stepping=-1;

    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return;

#ifdef __mips__
    strcpy(host.p_model, "MIPS ");
    model_found = true;
#elif __alpha__
    strcpy(host.p_vendor, "HP (DEC) ");
    vendor_found = true;
#endif

    strcpy(features, "");
    while (fgets(buf, 256, f)) {
        strip_whitespace(buf);
        if (strstr(buf, "vendor_id\t: ") || strstr(buf, "system type\t\t: ")) {
            if (!vendor_found) {
                vendor_found = true;
                strlcpy(host.p_vendor, strchr(buf, ':') + 2, sizeof(host.p_vendor));
            }
        }
        if (strstr(buf, "model name\t: ") || strstr(buf, "cpu model\t\t: ")) {
            if (!model_found) {
                model_found = true;
                strlcpy(host.p_model, strchr(buf, ':') + 2, sizeof(host.p_model));
            }
        }
        if (strstr(buf, "cpu family\t: ") && family<0) {
            family = atoi(buf+strlen("cpu family\t: "));
        }
        if (strstr(buf, "model\t\t: ") && model<0) {
            model = atoi(buf+strlen("model\t\t: "));
        }
        if (strstr(buf, "stepping\t: ") && stepping<0) {
            stepping = atoi(buf+strlen("stepping\t: "));
        }
        if (!cache_found && (strstr(buf, "cache size\t: ") == buf)) {
            cache_found = true;
            sscanf(buf, "cache size\t: %d", &n);
            host.m_cache = n*1024;
        }

        if (!features_found) {
            // Some versions of the linux kernel call them flags,
            // others call them features, so look for both.
            //
            if ((strstr(buf, "flags\t\t: ") == buf)) {
                strlcpy(features, strchr(buf, ':') + 2, sizeof(features));
            } else if ((strstr(buf, "features\t\t: ") == buf)) {
                strlcpy(features, strchr(buf, ':') + 2, sizeof(features));
            }
            if (strlen(features)) {
                features_found = true;
            }
        }
    }
    strcpy(model_buf, host.p_model);
    if (family>=0 || model>=0 || stepping>0) {
        strcat(model_buf, " [");
        if (family>=0) {
            sprintf(buf, "Family %d ", family);
            strcat(model_buf, buf);
        }
        if (model>=0) {
            sprintf(buf, "Model %d ", model);
            strcat(model_buf, buf);
        }
        if (stepping>=0) {
            sprintf(buf, "Stepping %d", stepping);
            strcat(model_buf, buf);
        }
        strcat(model_buf, "]");
    }
    if (strlen(features)) {
        strcat(model_buf, "[");
        strcat(model_buf, features);
        strcat(model_buf, "]");
    }

    strlcpy(host.p_model, model_buf, sizeof(host.p_model));
    fclose(f);
}

#endif  // linux

// get all relevant host information
//
int HOST_INFO::get_host_info() {
    get_filesystem_info(d_total, d_free);

#ifdef linux
    parse_cpuinfo(*this);
#elif defined(__EMX__)
    int mib[2];
    unsigned int mem_size;
    size_t len;
    CPU_INFO_t	cpuInfo;
    strcpy( p_vendor, cpuInfo.vendor.company);
    strcpy( p_model, cpuInfo.name.fromID);
#else
#if HAVE_SYS_SYSCTL_H
    int mib[2];
    unsigned int mem_size;
    size_t len;
#ifndef __APPLE__

    // Get machine
    mib[0] = CTL_HW;
    mib[1] = HW_MACHINE;
    len = sizeof(p_vendor);
    sysctl(mib, 2, &p_vendor, &len, NULL, 0);

    // Get model
    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    len = sizeof(p_model);
    sysctl(mib, 2, &p_model, &len, NULL, 0);
#endif      // ! __APPLE__
#else
// Tru64 UNIX.
// 2005-12-26 SMS.
#ifdef __osf__
    int mem_size;
    long cpu_type;
    char *cpu_type_name;

    strcpy(p_vendor, "HP (DEC)");

    getsysinfo( GSI_PROC_TYPE, (caddr_t) &cpu_type, sizeof( cpu_type));
    CPU_TYPE_TO_TEXT( (cpu_type& 0xffffffff), cpu_type_name);
    strncpy( p_model, "Alpha ", sizeof( p_model));
    strncat( p_model, cpu_type_name, (sizeof( p_model)- strlen( p_model)- 1));
#endif
#endif
#endif

// sysconf not working on OS2
#if defined(_SC_NPROCESSORS_ONLN) && !defined(__EMX__)
    p_ncpus = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(HAVE_SYS_SYSCTL_H) && defined(CTL_HW) && defined(HW_NCPU)
    // Get number of CPUs
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(p_ncpus);
    sysctl(mib, 2, &p_ncpus, &len, NULL, 0);
#elif defined(_HPUX_SOURCE)
    struct pst_dynamic psd; 
    pstat_getdynamic ( &psd, sizeof ( psd ), (size_t)1, 0 );
    p_ncpus = psd.psd_proc_cnt;
#else
#error Need to specify a sysconf() define to obtain number of processors
#endif

// There can be a variety of methods to obtain amount of usable memory.
// You will have to check your sysconf() defines, probably in unistd.h
// - 2002-11-03 hiram@users.sourceforge.net
//
#ifdef __EMX__
    {
        ULONG ulMem;
        CPU_INFO_t	cpuInfo;
        DosQuerySysInfo( QSV_TOTPHYSMEM, QSV_TOTPHYSMEM, &ulMem, sizeof(ulMem));
        m_nbytes = ulMem;
        // YD this is not the swap free space, but should be enough
        DosQuerySysInfo( QSV_TOTAVAILMEM, QSV_TOTAVAILMEM, &ulMem, sizeof(ulMem));
        m_swap = ulMem;
    }
#elif defined(_SC_USEABLE_MEMORY)
    m_nbytes = (double)sysconf(_SC_PAGESIZE)
        * (double)sysconf(_SC_USEABLE_MEMORY);  // UnixWare
#elif defined(_SC_PHYS_PAGES)
    m_nbytes = (double)sysconf(_SC_PAGESIZE) * (double)sysconf(_SC_PHYS_PAGES);
    // Linux
#elif defined(HAVE_SYS_SYSCTL_H) && defined(CTL_HW) && defined(HW_PHYSMEM)
    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;
    len = sizeof(mem_size);
    sysctl(mib, 2, &mem_size, &len, NULL, 0);    // Mac OS X
    m_nbytes = mem_size;
#elif defined(_HPUX_SOURCE)
    struct pst_static pst; 
    pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0);
    m_nbytes = (long double)pst.physical_memory * (long double)pst.page_size;
#elif defined(__osf__)
    // Tru64 UNIX.
    // 2005-12-26 SMS.
    getsysinfo( GSI_PHYSMEM, (caddr_t) &mem_size, sizeof( mem_size));
    m_nbytes = 1024.* (double)mem_size;
#else
#error Need to specify a sysconf() define to obtain memory size
#endif

#if defined(HAVE_SYS_SWAP_H) && defined(SC_GETNSWP)
    // Solaris, ...
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
    m_swap = 0.0;
    for (i=0; i<n; i++) {
        m_swap += 512.*(double)s->swt_ent[i].ste_length;
    }
#elif defined(HAVE_SYS_SWAP_H) && defined(SWAP_NSWAP)
    // NetBSD (the above line should probably be more comprehensive
    struct swapent * s;
    int i, n;
    n = swapctl(SWAP_NSWAP, NULL, 0);
    s = (struct swapent*)malloc(n * sizeof(struct swapent));
    swapctl(SWAP_STATS, s, n);
    m_swap = 0.0;
    for (i = 0; i < n; i ++) {
      if (s[i].se_flags & SWF_ENABLE)
        m_swap += 512. * (double)s[i].se_nblks;
    }
#elif defined(HAVE__PROC_MEMINFO)
    // Linux
    FILE *fp;
    if ((fp = fopen("/proc/meminfo", "r")) != 0) {
        char minfo_buf[1024];
        int n;
        if ((n = fread(minfo_buf, sizeof(char), sizeof(minfo_buf)-1, fp))) {
            char *p;
            minfo_buf[n] = '\0';
            if ((p = strstr(minfo_buf, "SwapTotal:"))) {
                p += 10; // move past "SwapTotal:"
                m_swap = 1024.*(double) strtoul(p, NULL, 10);
            }
        }
        fclose(fp);
    }
#elif defined(__APPLE__)
    // The sysctl(vm.vmmeter) function doesn't work on OS X.  However, swap  
    // space is limited only by free disk space, so we get that info instead. 
    // This is larger than free disk space reported by get_filesystem_info() 
    // because it includes space available only to the kernel / super-user.
    //
    // http://developer.apple.com/documentation/Performance/Conceptual/ManagingMemory/Articles/AboutMemory.html says:
    //    Unlike most UNIX-based operating systems, Mac OS X does not use a 
    //    preallocated swap partition for virtual memory. Instead, it uses all
    //    of the available space on the machineÕs boot partition.
    struct statfs fs_info;

    statfs(".", &fs_info);
    m_swap = (double)fs_info.f_bsize * (double)fs_info.f_bfree;

#elif defined(HAVE_VMMETER_H) && defined(HAVE_SYS_SYSCTL_H) && defined(CTL_VM) && defined(VM_METER)
    // MacOSX, I think...
    // <http://www.osxfaq.com/man/3/sysctl.ws>
    // The sysctl(vm.vmmeter) function doesn't work on OS X, so the following 
    // code fails to get the total swap space.  See note above for APPLE case.
    // I've left this code here in case it is used by a different platform, 
    // though I believe the first argument should be CTL_VM instead of CTL_USER.
    struct vmtotal vm_info;

    mib[0] = CTL_USER;  // Should this be CTL_VM ?
    mib[1] = VM_METER;
    len = sizeof(vm_info);
    if (!sysctl(mib, 2, &vm_info, &len, NULL, 0)) {
        m_swap = 1024. * getpagesize() * (double) vm_info.t_vm;
    }

#elif defined(_HPUX_SOURCE)
    struct pst_vminfo vminfo;
    pstat_getvminfo(&vminfo, sizeof(vminfo), (size_t)1, 0);
    m_swap = (vminfo.psv_swapspc_max * pst.page_size);
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

#ifdef __APPLE__
#ifdef __i386__
    char brand_string[256], capabilities[256];
    int family, stepping, model;
    int p_model_size = sizeof(p_model);
    
    len = sizeof(p_vendor);
    sysctlbyname("machdep.cpu.vendor", p_vendor, &len, NULL, 0);

    len = sizeof(brand_string);
    sysctlbyname("machdep.cpu.brand_string", brand_string, &len, NULL, 0);

    len = sizeof(family);
    sysctlbyname("machdep.cpu.family", &family, &len, NULL, 0);

    len = sizeof(model);
    sysctlbyname("machdep.cpu.model", &model, &len, NULL, 0);

    len = sizeof(stepping);
    sysctlbyname("machdep.cpu.stepping", &stepping, &len, NULL, 0);

    len = sizeof(capabilities);
    sysctlbyname("machdep.cpu.features", capabilities, &len, NULL, 0);

    snprintf(p_model, p_model_size, "%s [x86 Family %d Model %d Stepping %d] [%s]", 
                brand_string, family, model, stepping, capabilities);
                
#else       // PowerPC
    char capabilities[256], model[256];
    int p_model_size = sizeof(p_model);
    int response = 0;
    int retval;
    len = sizeof(response);
    safe_strcpy(p_vendor, "Power Macintosh");
    retval = sysctlbyname("hw.optional.altivec", &response, &len, NULL, 0);
    if (response && (!retval)) 
        safe_strcpy(capabilities, "AltiVec");
        
    len = sizeof(model);
    sysctlbyname("hw.model", model, &len, NULL, 0);

    snprintf(p_model, p_model_size, "%s [%s Model %s] [%s]", p_vendor, p_vendor, model, capabilities);

#endif  // i386 or PowerPC

    p_model[p_model_size-1] = 0;
    char *in = p_model + 1;
    char *out = in;
    // Strip out runs of multiple spaces
    do {
        if ((!isspace(*(in-1))) || (!isspace(*in)))
            *out++ = *in;
        } while (*in++);
    
#endif  // __APPLE__

    get_local_network_info();

    timezone = get_timezone();
#ifdef HAVE_SYS_UTSNAME_H
    struct utsname u;
    uname(&u);
    safe_strcpy(os_name, u.sysname);
#ifdef __EMX__ // OS2: version is in u.version
    safe_strcpy(os_version, u.version);
#else
    safe_strcpy(os_version, u.release);
#endif
#ifdef _HPUX_SOURCE
    safe_strcpy(p_model, u.machine);
    safe_strcpy(p_vendor, "Hewlett-Packard");
#endif
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

    if (!strlen(host_cpid)) {
        generate_host_cpid();
    }
    return 0;
}

// returns true iff device was last accessed before t
// or if an error occurred looking at the device.
inline bool device_idle(time_t t, const char *device) {
    struct stat sbuf;
    return stat(device, &sbuf) || (sbuf.st_atime < t);
}

inline bool all_tty_idle(time_t t, char *device, char first_char, int num_tty) {
    struct stat sbuf;
    char *tty_index = device + strlen(device) - 1;
    *tty_index = first_char;
    for (int i = 0; i < num_tty; i++, (*tty_index)++) {
        if (stat(device, &sbuf)) {
            // error looking at device; don't try any more
            return true;
        } else if (sbuf.st_atime >= t) {
            return false;
        }
    }
    return true;
}

#ifdef HAVE_UTMP_H
inline bool user_idle(time_t t, struct utmp* u) {
    char tty[5 + sizeof u->ut_line + 1] = "/dev/";
    unsigned int i;

    for (i=0; i < sizeof(u->ut_line); i++) {
        // clean up tty if garbled
        if (isalnum((int) u->ut_line[i]) || (u->ut_line[i]=='/')) {
            tty[i+5] = u->ut_line[i];
        } else {
            tty[i+5] = '\0';
        }
    }
    return device_idle(t, tty);
}

#if !defined(HAVE_SETUTENT) || !defined(HAVE_GETUTENT)
static FILE *ufp = NULL;
static struct utmp ut;

// get next user login record
// (this is defined on everything except BSD)
//
struct utmp *getutent() {
    if (ufp == NULL) {
#if defined(UTMP_LOCATION)
        if ((ufp = fopen(UTMP_LOCATION, "r")) == NULL) {
#elif defined(UTMP_FILE)
        if ((ufp = fopen(UTMP_FILE, "r")) == NULL) {
#elif defined(_PATH_UTMP)
        if ((ufp = fopen(_PATH_UTMP, "r")) == NULL) {
#else
        if ((ufp = fopen("/etc/utmp", "r")) == NULL) {
#endif
            return((struct utmp *)NULL);
        }
    }
    do {
        if (fread((char *)&ut, sizeof(ut), 1, ufp) != 1) {
            return((struct utmp *)NULL);
        }
    } while (ut.ut_name[0] == 0);
    return(&ut);
}

void setutent() {
    if (ufp != NULL) rewind(ufp);
}
#endif

// scan list of logged-in users, and see if they're all idle
//
inline bool all_logins_idle(time_t t) {
    struct utmp* u;
    setutent();

    while ((u = getutent()) != NULL) {
        if (!user_idle(t, u)) {
            return false;
        }
    }
    return true;
}
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>

#ifdef __i386__

#include <ApplicationServices/ApplicationServices.h>

// Returns the system idle time in seconds
static double GetOSXIdleTime(void) {
    return (double)CGEventSourceSecondsSinceLastEventType (kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);
}

#else

// CGEventSourceSecondsSinceLastEventType() is available only in OS 10.4 and later.  
// Since the OS10.3.9 SDK doesn't have this function, the PowerPC build would fail 
// with a link error even with weak linking.  So we have to do this the hard way.

extern "C" {
    extern double CGSSecondsSinceLastInputEvent(long evType); //  private API for pre-10.4 systems
}

enum {
    kCGEventSourceStatePrivate = -1,
    kCGEventSourceStateCombinedSessionState = 0,
    kCGEventSourceStateHIDSystemState = 1
};

#define kCGAnyInputEventType ((CGEventType)(~0))
typedef uint32_t CGEventSourceStateID;
typedef uint32_t CGEventType;

CG_EXTERN CFTimeInterval CGEventSourceSecondsSinceLastEventType( CGEventSourceStateID source, CGEventType eventType );

typedef CFTimeInterval (*GetIdleTimeProc)( CGEventSourceStateID source, CGEventType eventType );

// Returns the system idle time in seconds
static double GetOSXIdleTime(void) {
    static CFBundleRef bundleRef = NULL;
    static GetIdleTimeProc GetSysIdleTime = NULL;
    CFURLRef frameworkURL = NULL;
    double idleTime = 0;
    static bool tryNewAPI = true;

    if (tryNewAPI) {
        if (bundleRef == NULL) {
            frameworkURL = CFURLCreateWithFileSystemPath (kCFAllocatorSystemDefault, 
                                CFSTR("/System/Library/Frameworks/ApplicationServices.framework"), kCFURLPOSIXPathStyle, true);
            if (frameworkURL) {
                bundleRef = CFBundleCreate(kCFAllocatorSystemDefault, frameworkURL);
                CFRelease( frameworkURL );
            }
        }
        
        if (bundleRef) {
            if ( (GetSysIdleTime == NULL) || 
                    ( ! CFBundleIsExecutableLoaded( bundleRef ) ) )     // Is this test necessary ?
                GetSysIdleTime = (GetIdleTimeProc) 
                            CFBundleGetFunctionPointerForName( bundleRef, CFSTR("CGEventSourceSecondsSinceLastEventType") );
        }
        
        if (GetSysIdleTime)
            idleTime = (double)GetSysIdleTime (kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);
        else {
            CFRelease( bundleRef );
            bundleRef = NULL;
            tryNewAPI = false;  // CGEventSourceSecondsSinceLastEventType() API is not available on this system
        }
           
        if (GetSysIdleTime) 
            return idleTime;
    }   // if (tryNewAPI)
    
    // On 10.3 use this SPI
    // From Adium:
    // On MDD Powermacs, the above function will return a large value when the machine 
    // is active (-1?).  18446744073.0 is the lowest I've seen on my MDD -ai
    // Here we check for that value and correctly return a 0 idle time.
    idleTime = CGSSecondsSinceLastInputEvent (-1);
    if (idleTime >= 18446744000.0) idleTime = 0.0;
    return idleTime;
//    return (double)NXIdleTime(gEventHandle);      // Very old and very slow API
}
#endif  // ! __i386__

bool HOST_INFO::users_idle(bool check_all_logins, double idle_time_to_run, double *actual_idle_time) {
    double idleTime = GetOSXIdleTime();
    
    if (actual_idle_time)
        *actual_idle_time = idleTime;
    return (idleTime > (60 * idle_time_to_run));
}

#else  // ! __APPLE__

bool HOST_INFO::users_idle(bool check_all_logins, double idle_time_to_run) {
#ifdef HAVE__DEV_TTY1
    char device_tty[] = "/dev/tty1";
#endif
    time_t idle_time = time(NULL) - (long) (60 * idle_time_to_run);
    return true
#ifdef HAVE_UTMP_H
        && (!check_all_logins || all_logins_idle(idle_time))
#endif
#ifdef HAVE__DEV_MOUSE
        && device_idle(idle_time, "/dev/mouse") // solaris, linux
#endif
#ifdef HAVE__DEV_KBD
        && device_idle(idle_time, "/dev/kbd") // solaris
#endif
#ifdef HAVE__DEV_TTY1
        && (check_all_logins || all_tty_idle(idle_time, device_tty, '1', 7))
#endif

        ;
}

#endif  // ! __APPLE__

const char *BOINC_RCSID_2cf92d205b = "$Id$";
