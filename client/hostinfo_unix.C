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

#include "client_types.h"
#include "filesys.h"
#include "error_numbers.h"
#include "util.h"
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

NXEventHandle gEventHandle;
#endif  // __APPLE__

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

#elif __alpha__

void parse_cpuinfo(HOST_INFO& host) {
    char buf[256];
    char buf2[256];
    int system_found=0,model_found=0;

    strcpy(host.p_vendor, "HP (DEC) ");

    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return;

    while (fgets(buf, 256, f)) {
        if ((strstr(buf, "cpu\t\t\t: ") == buf) &&
             (system_found == 0)
        ) {
            system_found = 1;
            strncpy(buf2, strchr(buf, ':') + 2, sizeof(host.p_vendor) - strlen(host.p_vendor) - 1);

            strcat(host.p_vendor, buf2);
            char * p = strchr(host.p_vendor, '\n');
            if (p) {
                *p = '\0';
            }
        }
        if ( (strstr(buf, "cpu model\t\t: ") == buf) &&
             (model_found == 0) ) {
            model_found = 1;
            strncpy(host.p_model, strchr(buf, ':') + 2,

sizeof(host.p_model)-1);

            char * p = strchr(host.p_model, '\n');
            if (p) {
                *p = '\0';
            }
        }
    }

    fclose(f);
}

#else   // not mips or alpha

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

#endif  // linux

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
    len = sizeof(p_model);
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
    for (i=0; i<n; i++) {
        m_swap += 512.*(double)s->swt_ent[i].ste_length;
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
#elif defined(HAVE_VMMETER_H) && defined(HAVE_SYS_SYSCTL_H) && defined(CTL_USER) && defined(VM_METER)
    // MacOSX, I think...
    // <http://www.osxfaq.com/man/3/sysctl.ws>
    struct vmtotal vm_info;

    mib[0] = CTL_USER;
    mib[1] = VM_METER;
    len = sizeof(vm_info);
    if (!sysctl(mib, 2, &vm_info, &len, NULL, 0)) {
        m_swap = 1024. * getpagesize() * (double) vm_info.t_vm;
    }
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

    get_local_network_info(
        domain_name, sizeof(domain_name), ip_addr, sizeof(ip_addr)
    );

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
#ifdef __APPLE__
        && (NXIdleTime(gEventHandle) > (60 * idle_time_to_run))
#endif  // __APPLE__

        ;
}


const char *BOINC_RCSID_2cf92d205b = "$Id$";

