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

// There is a reason that having a file called "cpp.h" that includes config.h
// and some of the C++ header files is bad.  That reason is because there are
// #defines that alter the behiour of the standard C and C++ headers.  In
// this case we need to use the "small files" environment on some unix
// systems.  That can't be done if we include "cpp.h"

#include "version.h"         // version numbers from autoconf

#include "cpp.h"
#include "config.h"

#if !defined(_WIN32) || defined(__CYGWIN32__)

// Access to binary files in /proc filesystem doesn't work in the 64bit
// files environment on some systems.  None of the functions here need
// 64bit file functions, so we'll undefine _FILE_OFFSET_BITS and _LARGE_FILES.
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <sys/param.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#if HAVE_SYS_VMMETER_H
#include <sys/vmmeter.h>
#endif

#include <sys/stat.h>

#if HAVE_SYS_SWAP_H
#ifdef ANDROID
#include <linux/swap.h>
#else
#include <sys/swap.h>
#endif
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

#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "client_state.h"
#include "client_types.h"
#include "client_msgs.h"
#include "hostinfo_network.h"
#include "hostinfo.h"

using std::string;

#ifdef __APPLE__
#include <IOKit/IOKitLib.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#ifdef __cplusplus
}    // extern "C"

#include <dlfcn.h>
#endif

mach_port_t gEventHandle = NULL;
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

#ifdef __HAIKU__
#include <OS.h>
#endif

// Some OS define _SC_PAGE_SIZE instead of _SC_PAGESIZE
#if defined(_SC_PAGE_SIZE) && !defined(_SC_PAGESIZE)
#define _SC_PAGESIZE _SC_PAGE_SIZE
#endif

#if HAVE_XSS
#include <X11/extensions/scrnsaver.h>
#endif

#ifdef ANDROID
#include "android_log.h"
#endif

// The following is intended to be true both on Linux
// and Debian GNU/kFreeBSD (see trac #521)
//
#define LINUX_LIKE_SYSTEM (defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)) && !defined(__HAIKU__)

// Returns the offset between LOCAL STANDARD TIME and UTC.
// LOCAL_STANDARD_TIME = UTC_TIME + get_timezone().
//
int get_timezone() {
    tzset();
    // TODO: take daylight savings time into account
#if HAVE_STRUCT_TM_TM_ZONE
    time_t cur_time;
    struct tm *time_data;

    cur_time = time(NULL);
    time_data = localtime( &cur_time );
    // tm_gmtoff is already adjusted for daylight savings time
    return time_data->tm_gmtoff;
#elif LINUX_LIKE_SYSTEM
    return -1*(__timezone);
#elif defined(__CYGWIN32__)
    return -1*(_timezone);
#elif defined(unix)
    return -1*timezone;
#elif HAVE_TZNAME
    return -1*timezone;
#else
#error timezone
#endif
    return 0;
}

// Returns true if the host is currently running off battery power
// If you can't figure out, return false
//
bool HOST_INFO::host_is_running_on_batteries() {
#if defined(__APPLE__)
    CFDictionaryRef pSource = NULL;
    CFStringRef psState;
    int i;
    bool retval = false;

    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    CFArrayRef list = IOPSCopyPowerSourcesList(blob);

    for (i=0; i<CFArrayGetCount(list); i++) {
        pSource = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(list, i));
        if(!pSource) break;
        psState = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSPowerSourceStateKey));
        if(!CFStringCompare(psState,CFSTR(kIOPSBatteryPowerValue),0))
        retval = true;
    }

    CFRelease(blob);
    CFRelease(list);
    return retval;

#elif ANDROID
    // using /sys/class/power_supply/*/online
    // power supplies are both ac and usb!
    //
    char acpath[1024];
    snprintf(acpath, sizeof(acpath), "/sys/class/power_supply/ac/online");
    char usbpath[1024];
    snprintf(usbpath, sizeof(usbpath), "/sys/class/power_supply/usb/online");

    FILE *fsysac = fopen(acpath, "r");
    FILE *fsysusb = fopen(usbpath, "r");
    int aconline = 0;
    int usbonline = 0;
    bool power_supply_online = false;

    if(fsysac) {
        (void) fscanf(fsysac, "%d", &aconline);
        fclose(fsysac);
    }

    if(fsysusb) {
        (void) fscanf(fsysusb, "%d", &usbonline);
        fclose(fsysusb);
    }

    if ((aconline == 1) || (usbonline == 1)){
        power_supply_online = true;
        char msg[1024];
        snprintf(msg, sizeof(msg),
            "power supply online! status for usb: %d and ac: %d",
            usbonline,
            aconline
        );
        LOGD(msg);
    } else {
        LOGD("running on batteries");
    }

    return !power_supply_online;

#elif LINUX_LIKE_SYSTEM
    static enum {
      Detect,
      ProcAPM,
      ProcACPI,
      SysClass,
      NoBattery
    } method = Detect;
    static char path[64] = "";

    if (Detect == method) {
        // try APM in ProcFS
        FILE *fapm = fopen("/proc/apm", "r");
        if (fapm) {
            method = ProcAPM;
            fclose(fapm);
        }
    }
    if (Detect == method) {
        // try ACPI in ProcFS
        string ac_name;
        FILE* facpi;

        DirScanner dir("/proc/acpi/ac_adapter/");
        while (dir.scan(ac_name)) {
            // newer ACPI versions use "state" as filename
            snprintf(
                path, sizeof(path), "/proc/acpi/ac_adapter/%s/state",
                ac_name.c_str()
            );
            facpi = fopen(path, "r");
            if (!facpi) {
                // older ACPI versions use "status" instead
                snprintf(
                    path, sizeof(path), "/proc/acpi/ac_adapter/%s/status",
                    ac_name.c_str()
                );
                facpi = fopen(path, "r");
            }
            if (facpi) {
                method = ProcACPI;
                fclose(facpi);
                break;
            }
        }
    }
    if (Detect == method) {
        // try SysFS
        char buf[256];
        string ps_name;
        FILE* fsys;

        DirScanner dir("/sys/class/power_supply/");
        while (dir.scan(ps_name)) {
            // check the type of the power supply
            snprintf(
                path, sizeof(path), "/sys/class/power_supply/%s/type",
                ps_name.c_str()
            );
            fsys = fopen(path, "r");
            if (!fsys) continue;
            (void) fgets(buf, sizeof(buf), fsys);
            fclose(fsys);
            // AC adapters have type "Mains"
            if ((strstr(buf, "mains") != NULL) || (strstr(buf, "Mains") != NULL)) {
                method = SysClass;
                // to check if we're on battery we look at "online",
                // located in the same directory
                snprintf(
                    path, sizeof(path), "/sys/class/power_supply/%s/online",
                    ps_name.c_str()
                );
                break;
            }
        }
    }
    switch (method) {
    case Detect:
        // if we haven't found a method so far, give up
        method = NoBattery;
        // fall through
    case ProcAPM:
        {
            // use /proc/apm
            FILE* fapm = fopen("/proc/apm", "r");
            if (!fapm) return false;

            char    apm_driver_version[11];
            int     apm_major_version;
            int     apm_minor_version;
            int     apm_flags;
            int     apm_ac_line_status=1;

            // supposedly we're on batteries if the 5th entry is zero.
            (void) fscanf(fapm, "%10s %d.%d %x %x",
                apm_driver_version,
                &apm_major_version,
                &apm_minor_version,
                &apm_flags,
                &apm_ac_line_status
            );

            fclose(fapm);

            return (apm_ac_line_status == 0);
        }
    case ProcACPI:
        {
            // use /proc/acpi/ac_adapter/*/stat{e,us}
            FILE *facpi = fopen(path, "r");
            if (!facpi) return false;

            char buf[128];
            (void) fgets(buf, sizeof(buf), facpi);

            fclose(facpi);

            if ((strstr(buf, "state:") != NULL) || (strstr(buf, "Status:") != NULL))
                // on batteries if ac adapter is "off-line" (or maybe "offline")
                return (strstr(buf, "off") != NULL);

            return false;
        }
    case SysClass:
        {
            // use /sys/class/power_supply/*/online
            FILE *fsys = fopen(path, "r");
            if (!fsys) return false;

            int online;
            (void) fscanf(fsys, "%d", &online);
            fclose(fsys);

            // online is 1 if on AC power, 0 if on battery
            return (0 == online);
        }
    case NoBattery:
    default:
         // we have no way to determine if we're on batteries,
         // so we say we aren't
        return false;
    }
#else
    return false;
#endif
}

#if LINUX_LIKE_SYSTEM
static void parse_meminfo_linux(HOST_INFO& host) {
    char buf[256];
    double x;
    FILE* f = fopen("/proc/meminfo", "r");
    if (!f) {
        msg_printf(NULL, MSG_INFO,
            "Can't open /proc/meminfo to get memory size - defaulting to 1 GB."
        );
        host.m_nbytes = GIGA;
        host.m_swap = GIGA;
        return;
    }
    while (fgets(buf, 256, f)) {
        if (strstr(buf, "MemTotal:")) {
            sscanf(buf, "MemTotal: %lf", &x);
            host.m_nbytes = x*1024;
        } else if (strstr(buf, "SwapTotal:")) {
            sscanf(buf, "SwapTotal: %lf", &x);
            host.m_swap = x*1024;
        } else if (strstr(buf, "Mem:")) {
            sscanf(buf, "Mem: %lf", &host.m_nbytes);
        } else if (strstr(buf, "Swap:")) {
            sscanf(buf, "Swap: %lf", &host.m_swap);
        }
    }
    fclose(f);
}

// Unfortunately the format of /proc/cpuinfo is not standardized.
// See http://people.nl.linux.org/~hch/cpuinfo/ for some examples.
//
static void parse_cpuinfo_linux(HOST_INFO& host) {
    char buf[1024], features[1024], model_buf[1024];
    bool vendor_found=false, model_found=false;
    bool cache_found=false, features_found=false;
    bool model_hack=false, vendor_hack=false;
    int n;
    int family=-1, model=-1, stepping=-1;
    char buf2[256];

    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) {
        msg_printf(NULL, MSG_INFO,
            "Can't open /proc/cpuinfo to get CPU info"
        );
        strcpy(host.p_model, "unknown");
        strcpy(host.p_vendor, "unknown");
        return;
    }

#ifdef __mips__
    strcpy(host.p_model, "MIPS ");
    model_hack = true;
#elif __alpha__
    strcpy(host.p_vendor, "HP (DEC) ");
    vendor_hack = true;
#elif __hppa__
    strcpy(host.p_vendor, "HP ");
    vendor_hack = true;
#elif __ia64__
    strcpy(host.p_model, "IA-64 ");
    model_hack = true;
#endif

    host.m_cache=-1;
    strcpy(features, "");
    while (fgets(buf, 1024, f)) {
        strip_whitespace(buf);
        if (
                /* there might be conflicts if we dont #ifdef */
#ifdef __ia64__
            strstr(buf, "vendor     : ")
#elif __hppa__
            strstr(buf, "cpu\t\t: ")
#elif __powerpc__
            strstr(buf, "machine\t\t: ") || strstr(buf, "platform\t: ")
#elif __sparc__
            strstr(buf, "type\t\t: ")
#elif __alpha__
            strstr(buf, "cpu\t\t\t: ")
#else
            strstr(buf, "vendor_id\t: ") || strstr(buf, "system type\t\t: ")
#endif
        ) {
            if (!vendor_hack && !vendor_found) {
                vendor_found = true;
                strlcpy(host.p_vendor, strchr(buf, ':') + 2, sizeof(host.p_vendor));
            } else if (!vendor_found) {
                vendor_found = true;
                strlcpy(buf2, strchr(buf, ':') + 2,
                    sizeof(host.p_vendor) - strlen(host.p_vendor) - 1
                );
                strlcat(host.p_vendor, buf2, sizeof(host.p_vendor));
            }
        }
        if (
#ifdef __ia64__
            strstr(buf, "family     : ") || strstr(buf, "model name : ")
#elif __powerpc__ || __sparc__
            strstr(buf, "cpu\t\t: ")
#else
            strstr(buf, "model name\t: ") || strstr(buf, "cpu model\t\t: ")
#endif
        ) {
            if (!model_hack && !model_found) {
                model_found = true;
#ifdef __powerpc__
                char *coma = NULL;
                if ((coma = strrchr(buf, ','))) {   /* we have ", altivec supported" */
                    *coma = '\0';    /* strip the unwanted line */
                    strcpy(features, "altivec");
                    features_found = true;
                }
#endif
                strlcpy(host.p_model, strchr(buf, ':') + 2, sizeof(host.p_model));
            } else if (!model_found) {
#ifdef __ia64__
                /* depending on kernel version, family can be either
                a number or a string. If number, we have a model name,
                else we don't */
                char *testc = NULL;
                testc = strrchr(buf, ':')+2;
                if (isdigit(*testc)) {
                    family = atoi(testc);
                    continue;    /* skip this line */
                }
#endif
                model_found = true;
                strlcpy(buf2, strchr(buf, ':') + 2, sizeof(host.p_model) - strlen(host.p_model) - 1);
                strcat(host.p_model, buf2);
            }
        }
#ifndef __hppa__
    /* XXX hppa: "cpu family\t: PA-RISC 2.0" */
        if (strstr(buf, "cpu family\t: ") && family<0) {
            family = atoi(buf+strlen("cpu family\t: "));
        }
        /* XXX hppa: "model\t\t: 9000/785/J6000" */
    /* XXX alpha: "cpu model\t\t: EV6" -> ==buf necessary */
        if ((strstr(buf, "model\t\t: ") == buf) && model<0) {
            model = atoi(buf+strlen("model\t\t: "));
        }
        /* ia64 */
        if (strstr(buf, "model      : ") && model<0) {
            model = atoi(buf+strlen("model     : "));
        }
#endif
        if (strstr(buf, "stepping\t: ") && stepping<0) {
            stepping = atoi(buf+strlen("stepping\t: "));
        }
#ifdef __hppa__
        bool icache_found=false,dcache_found=false;
        if (!icache_found && strstr(buf, "I-cache\t\t: ")) {
            icache_found = true;
            sscanf(buf, "I-cache\t\t: %d", &n);
            host.m_cache += n*1024;
        }
        if (!dcache_found && strstr(buf, "D-cache\t\t: ")) {
            dcache_found = true;
            sscanf(buf, "D-cache\t\t: %d", &n);
            host.m_cache += n*1024;
        }
#elif __powerpc__
        if (!cache_found && strstr(buf, "L2 cache\t: ")) {
            cache_found = true;
            sscanf(buf, "L2 cache\t: %d", &n);
            host.m_cache = n*1024;
        }
#else
        if (!cache_found && (strstr(buf, "cache size\t: ") == buf)) {
            cache_found = true;
            sscanf(buf, "cache size\t: %d", &n);
            host.m_cache = n*1024;
        }
#endif
        if (!features_found) {
            // Some versions of the linux kernel call them flags,
            // others call them features, so look for both.
            //
            if ((strstr(buf, "flags\t\t: ") == buf)) {
                strlcpy(features, strchr(buf, ':') + 2, sizeof(features));
            } else if ((strstr(buf, "features\t\t: ") == buf)) {
                strlcpy(features, strchr(buf, ':') + 2, sizeof(features));
            } else if ((strstr(buf, "features   : ") == buf)) {    /* ia64 */
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
        strlcpy(host.p_features, features, sizeof(host.p_features));
    }

    strlcpy(host.p_model, model_buf, sizeof(host.p_model));
    fclose(f);
}
#endif  // LINUX_LIKE_SYSTEM
#ifdef __FreeBSD__
#if defined(__i386__) || defined(__amd64__)
#include <sys/types.h>
#include <sys/cdefs.h>
#include <machine/cpufunc.h>

void use_cpuid(HOST_INFO& host) {
    u_int p[4];
    int hasMMX, hasSSE, hasSSE2, hasSSE3, has3DNow, has3DNowExt = 0;
    char capabilities[256];

    do_cpuid(0x0, p);

    if (p[0] >= 0x1) {

        do_cpuid(0x1, p);

        hasMMX  = (p[3] & (1 << 23 )) >> 23; // 0x0800000
        hasSSE  = (p[3] & (1 << 25 )) >> 25; // 0x2000000
        hasSSE2 = (p[3] & (1 << 26 )) >> 26; // 0x4000000
        hasSSE3 = (p[2] & (1 << 0 )) >> 0;
    }

    do_cpuid(0x80000000, p);
    if (p[0]>=0x80000001) {
        do_cpuid(0x80000001, p);
        hasMMX  |= (p[3] & (1 << 23 )) >> 23; // 0x0800000
        has3DNow    = (p[3] & (1 << 31 )) >> 31; //0x80000000
        has3DNowExt = (p[3] & (1 << 30 )) >> 30;
    }

    capabilities[0] = '\0';
    if (hasSSE) strncat(capabilities, "sse ", 4);
    if (hasSSE2) strncat(capabilities, "sse2 ", 5);
    if (hasSSE3) strncat(capabilities, "sse3 ", 5);
    if (has3DNow) strncat(capabilities, "3dnow ", 6);
    if (has3DNowExt) strncat(capabilities, "3dnowext ", 9);
    if (hasMMX) strncat(capabilities, "mmx ", 4);
    strip_whitespace(capabilities);
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s [] [%s]",
        host.p_model, capabilities
    );
    strlcat(host.p_model, buf, sizeof(host.p_model));
}
#endif
#endif

#ifdef __APPLE__
static void get_cpu_info_maxosx(HOST_INFO& host) {
    int p_model_size = sizeof(host.p_model);
    size_t len;
#if defined(__i386__) || defined(__x86_64__)
    char brand_string[256];
    int family, stepping, model;

    len = sizeof(host.p_vendor);
    sysctlbyname("machdep.cpu.vendor", host.p_vendor, &len, NULL, 0);

    len = sizeof(brand_string);
    sysctlbyname("machdep.cpu.brand_string", brand_string, &len, NULL, 0);

    len = sizeof(family);
    sysctlbyname("machdep.cpu.family", &family, &len, NULL, 0);

    len = sizeof(model);
    sysctlbyname("machdep.cpu.model", &model, &len, NULL, 0);

    len = sizeof(stepping);
    sysctlbyname("machdep.cpu.stepping", &stepping, &len, NULL, 0);

    len = sizeof(host.p_features);
    sysctlbyname("machdep.cpu.features", host.p_features, &len, NULL, 0);

    snprintf(
        host.p_model, sizeof(host.p_model),
        "%s [x86 Family %d Model %d Stepping %d]",
        brand_string, family, model, stepping
    );
#else       // PowerPC
    char model[256];
    int response = 0;
    int retval;
    len = sizeof(response);
    retval = sysctlbyname("hw.optional.altivec", &response, &len, NULL, 0);
    if (response && (!retval)) {
        safe_strcpy(host.p_features, "AltiVec");
    }

    len = sizeof(model);
    sysctlbyname("hw.model", model, &len, NULL, 0);

    safe_strcpy(host.p_vendor, "Power Macintosh");
    snprintf(host.p_model, p_model_size, "%s [%s Model %s] [%s]", host.p_vendor, host.p_vendor, model, host.p_features);

#endif

    host.p_model[p_model_size-1] = 0;
    char *in = host.p_model + 1;
    char *out = in;
    // Strip out runs of multiple spaces
    do {
        if ((!isspace(*(in-1))) || (!isspace(*in))) {
            *out++ = *in;
        }
    } while (*in++);
}
#endif

#ifdef __HAIKU__
static void get_cpu_info_haiku(HOST_INFO& host) {
    /* This function has been adapted from Haiku's sysinfo.c
     * which spits out a bunch of formatted CPU info to
     * the terminal, it was easier to copy some of the logic
     * here.
     */
    system_info sys_info;
    int32 cpu = 0; // always use first CPU for now
    cpuid_info cpuInfo;
    int32 maxStandardFunction;
    int32 maxExtendedFunction = 0;

    char brand_string[256];

    if (get_system_info(&sys_info) != B_OK) {
        msg_printf(NULL, MSG_INTERNAL_ERROR, "Error getting Haiku system information!\n");
        return;
    }

    if (get_cpuid(&cpuInfo, 0, cpu) != B_OK) {
        // this CPU doesn't support cpuid
        return;
    }

    snprintf(host.p_vendor, sizeof(host.p_vendor), "%.12s",
        cpuInfo.eax_0.vendor_id);

    maxStandardFunction = cpuInfo.eax_0.max_eax;
    if (maxStandardFunction >= 500)
        maxStandardFunction = 0; /* old Pentium sample chips has
                                    cpu signature here */

    /* Extended cpuid */
    get_cpuid(&cpuInfo, 0x80000000, cpu);

    // extended cpuid is only supported if max_eax is greater
    // than the service id
    if (cpuInfo.eax_0.max_eax > 0x80000000)
        maxExtendedFunction = cpuInfo.eax_0.max_eax & 0xff;

    if (maxExtendedFunction >=4 ) {
        char buffer[49];
        char *name = buffer;
        int32 i;

        memset(buffer, 0, sizeof(buffer));

        for (i = 0; i < 3; i++) {
            cpuid_info nameInfo;
            get_cpuid(&nameInfo, 0x80000002 + i, cpu);

            memcpy(name, &nameInfo.regs.eax, 4);
            memcpy(name + 4, &nameInfo.regs.ebx, 4);
            memcpy(name + 8, &nameInfo.regs.ecx, 4);
            memcpy(name + 12, &nameInfo.regs.edx, 4);
            name += 16;
        }

        // cut off leading spaces (names are right aligned)
        name = buffer;
        while (name[0] == ' ')
            name++;

        // the BIOS may not have set the processor name
        if (name[0]) {
            strlcpy(brand_string, name, sizeof(brand_string));
        } else {
            // Intel CPUs don't seem to have the genuine vendor field
            snprintf(brand_string, sizeof(brand_string), "%.12s",
                cpuInfo.eax_0.vendor_id);
        }
    }

    get_cpuid(&cpuInfo, 1, cpu);

    int family, stepping, model;

    family = cpuInfo.eax_1.family + (cpuInfo.eax_1.family == 0xf ?
        cpuInfo.eax_1.extended_family : 0);

    // model calculation is different for AMD and INTEL
    if ((sys_info.cpu_type & B_CPU_x86_VENDOR_MASK) == B_CPU_AMD_x86) {
        model = cpuInfo.eax_1.model + (cpuInfo.eax_1.model == 0xf ?
            cpuInfo.eax_1.extended_model << 4 : 0);
    } else if ((sys_info.cpu_type & B_CPU_x86_VENDOR_MASK) == B_CPU_INTEL_x86) {
        model = cpuInfo.eax_1.model + ((cpuInfo.eax_1.family == 0xf ||
            cpuInfo.eax_1.family == 0x6) ? cpuInfo.eax_1.extended_model << 4 : 0);
    }

    stepping = cpuInfo.eax_1.stepping;

    snprintf(host.p_model, sizeof(host.p_model),
        "%s [Family %u Model %u Stepping %u]", brand_string, family, model,
        stepping);

    static const char *kFeatures[32] = {
        "fpu", "vme", "de", "pse",
        "tsc", "msr", "pae", "mce",
        "cx8", "apic", NULL, "sep",
        "mtrr", "pge", "mca", "cmov",
        "pat", "pse36", "psnum", "clflush",
        NULL, "ds", "acpi", "mmx",
        "fxsr", "sse", "sse2", "ss",
        "htt", "tm", "ia64", "pbe",
    };

    int32 found = 0;
    int32 i;
    char buf[12];

    for (i = 0; i < 32; i++) {
        if ((cpuInfo.eax_1.features & (1UL << i)) && kFeatures[i] != NULL) {
            snprintf(buf, sizeof(buf), "%s%s", found == 0 ? "" : " ", 
                kFeatures[i]);
            strlcat(host.p_features, buf, sizeof(host.p_features));
            found++;
        }
    }

    if (maxStandardFunction >= 1) {
        /* Extended features */
        static const char *kFeatures2[32] = {
            "sse3", NULL, "dtes64", "monitor", "ds-cpl", "vmx", "smx" "est",
            "tm2", "ssse3", "cnxt-id", NULL, NULL, "cx16", "xtpr", "pdcm",
            NULL, NULL, "dca", "sse4.1", "sse4.2", "x2apic", "movbe", "popcnt",
            NULL, NULL, "xsave", "osxsave", NULL, NULL, NULL, NULL
        };

        for (i = 0; i < 32; i++) {
            if ((cpuInfo.eax_1.extended_features & (1UL << i)) &&
                kFeatures2[i] != NULL) {
                snprintf(buf, sizeof(buf), "%s%s", found == 0 ? "" : " ",
                    kFeatures2[i]);
                strlcat(host.p_features, buf, sizeof(host.p_features));
                found++;
            }
        }
    }

    //TODO: there are additional AMD features that probably need to be queried
}
#endif


// Note: this may also work on other UNIX-like systems in addition to Macintosh
#ifdef __APPLE__

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>

// detect the network usage totals for the host.
//
int get_network_usage_totals(
    unsigned int& total_received, unsigned int& total_sent
) {
    static size_t  sysctlBufferSize = 0;
    static uint8_t *sysctlBuffer = NULL;

    int    mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
    struct if_msghdr *ifmsg;
    size_t currentSize = 0;

    total_received = 0;
    total_sent = 0;

    if (sysctl(mib, 6, NULL, &currentSize, NULL, 0) != 0) return errno;
    if (!sysctlBuffer || (currentSize > sysctlBufferSize)) {
        if (sysctlBuffer) free(sysctlBuffer);
        sysctlBufferSize = 0;
        sysctlBuffer = (uint8_t*)malloc(currentSize);
        if (!sysctlBuffer) return ERR_MALLOC;
        sysctlBufferSize = currentSize;
    }
    
    // Read in new data
    if (sysctl(mib, 6, sysctlBuffer, &currentSize, NULL, 0) != 0) return errno;
    
    // Walk through the reply 
    uint8_t *currentData = sysctlBuffer;
    uint8_t *currentDataEnd = sysctlBuffer + currentSize;

    while (currentData < currentDataEnd) {
        // Expecting interface data
        ifmsg = (struct if_msghdr *)currentData;
        if (ifmsg->ifm_type != RTM_IFINFO) {
            currentData += ifmsg->ifm_msglen;
            continue;
        }
        // Must not be loopback
        if (ifmsg->ifm_flags & IFF_LOOPBACK) {
            currentData += ifmsg->ifm_msglen;
            continue;
        }
        // Only look at link layer items
        struct sockaddr_dl *sdl = (struct sockaddr_dl *)(ifmsg + 1);
        if (sdl->sdl_family != AF_LINK) {
            currentData += ifmsg->ifm_msglen;
            continue;
        }
        
#if 0   // Use this code if we want only Ethernet interface 0
        if (!strcmp(sdl->sdl_data, "en0")) {
            total_received = ifmsg->ifm_data.ifi_ibytes;
            total_sent = ifmsg->ifm_data.ifi_obytes;
            return 0;
        }
#else   // Use this code if we want total of all non-loopback interfaces
        total_received += ifmsg->ifm_data.ifi_ibytes;
        total_sent += ifmsg->ifm_data.ifi_obytes;
#endif
    }
        
    return 0;
}


#if defined(__i386__) || defined(__x86_64__)

// Code to get maximum CPU temperature (Apple Intel only)
// Adapted from Apple System Management Control (SMC) Tool under the GPL

#define KERNEL_INDEX_SMC      2

#define SMC_CMD_READ_BYTES    5
#define SMC_CMD_READ_KEYINFO  9

typedef struct {
    char                  major;
    char                  minor;
    char                  build;
    char                  reserved[1]; 
    UInt16                release;
} SMCKeyData_vers_t;

typedef struct {
    UInt16                version;
    UInt16                length;
    UInt32                cpuPLimit;
    UInt32                gpuPLimit;
    UInt32                memPLimit;
} SMCKeyData_pLimitData_t;

typedef struct {
    UInt32                dataSize;
    UInt32                dataType;
    char                  dataAttributes;
} SMCKeyData_keyInfo_t;

typedef char              SMCBytes_t[32]; 

typedef struct {
  UInt32                  key; 
  SMCKeyData_vers_t       vers; 
  SMCKeyData_pLimitData_t pLimitData;
  SMCKeyData_keyInfo_t    keyInfo;
  char                    result;
  char                    status;
  char                    data8;
  UInt32                  data32;
  SMCBytes_t              bytes;
} SMCKeyData_t;

static io_connect_t conn;

kern_return_t SMCOpen() {
    kern_return_t       result;
    mach_port_t         masterPort;
    io_iterator_t       iterator;
    io_object_t         device;

    result = IOMasterPort(MACH_PORT_NULL, &masterPort);

    CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
    result = IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);
    if (result != kIOReturnSuccess) {
        return result;
    }

    device = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
    if (device == 0) {
        return result;
    }

    result = IOServiceOpen(device, mach_task_self(), 0, &conn);
    IOObjectRelease(device);
    if (result != kIOReturnSuccess) {
        return result;
    }

    return kIOReturnSuccess;
}

kern_return_t SMCClose() {
    if (conn) {
        return IOServiceClose(conn);
    }
    return kIOReturnSuccess;
}

kern_return_t SMCReadKey(UInt32 key, SMCBytes_t val) {
    kern_return_t       result;
    SMCKeyData_t        inputStructure;
    SMCKeyData_t        outputStructure;
    size_t              structureOutputSize = 0;

    memset(&inputStructure, 0, sizeof(inputStructure));
    memset(&outputStructure, 0, sizeof(outputStructure));
    memset(val, 0, sizeof(SMCBytes_t));

    inputStructure.key = key;
    inputStructure.data8 = SMC_CMD_READ_KEYINFO;

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
    result = IOConnectCallStructMethod(conn,
        KERNEL_INDEX_SMC,
        &inputStructure,
        sizeof(inputStructure),
        &outputStructure,
        &structureOutputSize
    );
#else
    result = IOConnectMethodStructureIStructureO(conn,
        KERNEL_INDEX_SMC,
        sizeof(inputStructure),
        &structureOutputSize,
        &inputStructure,
        &outputStructure
    );
#endif
    if (result != kIOReturnSuccess) {
        return result;
    }

    inputStructure.keyInfo.dataSize = outputStructure.keyInfo.dataSize;
    inputStructure.data8 = SMC_CMD_READ_BYTES;

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
    result = IOConnectCallStructMethod(conn,
        KERNEL_INDEX_SMC,
        &inputStructure,
        sizeof(inputStructure),
        &outputStructure,
        &structureOutputSize
    );
#else
    result = IOConnectMethodStructureIStructureO(conn,
        KERNEL_INDEX_SMC,
        sizeof(inputStructure),
        &structureOutputSize,
        &inputStructure,
        &outputStructure
    );
#endif
    if (result != kIOReturnSuccess) {
        return result;
    }

    memcpy(val, outputStructure.bytes, sizeof(outputStructure.bytes));

    return kIOReturnSuccess;
}


// Check up to 10 die temperatures (TC0D, TC1D, etc.) and 
// 10 heatsink temperatures (TCAH, TCBH, etc.)
// Returns the highest current CPU temperature as degrees Celsius.
// Returns zero if it fails (or on a PowerPC Mac).
int get_max_cpu_temperature() {
    kern_return_t       result;
    int                 maxTemp = 0, thisTemp, i;
    union tempKey {
        UInt32          word;
        char            bytes[4];
    };
    tempKey             key;
    SMCBytes_t          val;
    static bool         skip[20];

    // open connection to SMC kext if this is the first time
    if (!conn) {
        result = SMCOpen();
        if (result != kIOReturnSuccess) {
            return 0;
        }
    }

    for (i=0; i<20; ++i) {
        if (skip[i]) continue;
        if (i < 10) {
            key.word = 'TC0D';
            key.bytes[1] += i;          // TC0D, TC1D, TC2D, etc.
        } else {
            key.word = 'TCAH';
            key.bytes[1] += (i - 10);   // TCAH, TCBH, TCCH, etc.
        }
        result = SMCReadKey(key.word, val);
        if (result != kIOReturnSuccess) {
            skip[i] = true;
            continue;
        }
        
        if (val[0] < 1) {
            skip[i] = true;
            continue;
        }
        
        thisTemp = val[0];
        if (val[1] & 0x80) ++thisTemp;
        if (thisTemp > maxTemp) {
            maxTemp = thisTemp;
        }
    }
    
    return maxTemp;
}

#else       // PowerPC

int get_max_cpu_temperature() {
    return 0;
}

#endif
#endif  // __APPLE__

// see if Virtualbox is installed
//
int HOST_INFO::get_virtualbox_version() {
    char path[MAXPATHLEN];
    char cmd [MAXPATHLEN+35];
    char *newlinePtr;
    FILE* fd;

#if LINUX_LIKE_SYSTEM
    strcpy(path, "/usr/lib/virtualbox/VBoxManage");
#elif defined( __APPLE__)
    FSRef theFSRef;
    OSStatus status = noErr;

    // First try to locate the VirtualBox application by Bundle ID and Creator Code
    status = LSFindApplicationForInfo(
        'VBOX', CFSTR("org.virtualbox.app.VirtualBox"), NULL, &theFSRef, NULL
    );
    if (status == noErr) {
        status = FSRefMakePath(&theFSRef, (unsigned char *)path, sizeof(path));
    }
    // If that failed, try its default location
    if (status != noErr) {
        strcpy(path, "/Applications/VirtualBox.app");
    }
#endif

    if (boinc_file_exists(path)) {
#if LINUX_LIKE_SYSTEM
        safe_strcpy(cmd, path);
        safe_strcat(cmd, " --version");
#elif defined( __APPLE__)
        safe_strcpy(cmd, "defaults read ");
        safe_strcat(cmd, path);
        safe_strcat(cmd, "/Contents/Info CFBundleShortVersionString");
#endif
        fd = popen(cmd, "r");
        if (fd) {
            fgets(virtualbox_version, sizeof(virtualbox_version), fd);
            newlinePtr = strchr(virtualbox_version, '\n');
            if (newlinePtr) *newlinePtr = '\0';
            newlinePtr = strchr(virtualbox_version, '\r');
            if (newlinePtr) *newlinePtr = '\0';
            pclose(fd);
        }
    }

    return 0;
}


// Rules:
// - Keep code in the right place
// - only one level of #if
//
int HOST_INFO::get_host_info() {
    get_filesystem_info(d_total, d_free);
    get_virtualbox_version();

///////////// p_vendor, p_model, p_features /////////////////
#if LINUX_LIKE_SYSTEM
    parse_cpuinfo_linux(*this);
#elif defined( __APPLE__)
    int mib[2];
    size_t len;

    get_cpu_info_maxosx(*this);
#elif defined(__EMX__)
    CPU_INFO_t    cpuInfo;
    strlcpy( p_vendor, cpuInfo.vendor.company, sizeof(p_vendor));
    strlcpy( p_model, cpuInfo.name.fromID, sizeof(p_model));
#elif defined(__HAIKU__)
    get_cpu_info_haiku(*this);
#elif HAVE_SYS_SYSCTL_H
    int mib[2];
    size_t len;

    // Get machine
#ifdef IRIX
    mib[0] = 0;
    mib[1] = 1;
#else
    mib[0] = CTL_HW;
    mib[1] = HW_MACHINE;
#endif
    len = sizeof(p_vendor);
    sysctl(mib, 2, &p_vendor, &len, NULL, 0);

    // Get model
#ifdef IRIX
    mib[0] = 0;
    mib[1] = 1;
#else
    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
#endif
    len = sizeof(p_model);
    sysctl(mib, 2, &p_model, &len, NULL, 0);
#elif defined(__osf__)
    // Tru64 UNIX.
    // 2005-12-26 SMS.
    long cpu_type;
    char *cpu_type_name;

    strcpy(p_vendor, "HP (DEC)");

    getsysinfo( GSI_PROC_TYPE, (caddr_t) &cpu_type, sizeof( cpu_type));
    CPU_TYPE_TO_TEXT( (cpu_type& 0xffffffff), cpu_type_name);
    strncpy( p_model, "Alpha ", sizeof( p_model));
    strncat( p_model, cpu_type_name, (sizeof( p_model)- strlen( p_model)- 1));
    p_model[sizeof(p_model)-1]=0;
#elif HAVE_SYS_SYSTEMINFO_H
    sysinfo(SI_PLATFORM, p_vendor, sizeof(p_vendor));
    sysinfo(SI_ISALIST, p_model, sizeof(p_model));
    for (unsigned int i=0; i<sizeof(p_model); i++) {
        if (p_model[i]==' ') {
            p_model[i]=0;
        }
        if (p_model[i]==0) {
            i=sizeof(p_model);
        }
    }
#else
#error Need to specify a method to get p_vendor, p_model
#endif

#if defined(__FreeBSD__)
#if defined(__i386__) || defined(__amd64__)
    use_cpuid(*this);
#endif
#endif

///////////// p_ncpus /////////////////

// sysconf not working on OS2
#if defined(_SC_NPROCESSORS_ONLN) && !defined(__EMX__) && !defined(__APPLE__)
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
#error Need to specify a method to get number of processors
#endif

///////////// m_nbytes, m_swap /////////////////

#ifdef __EMX__
    {
        ULONG ulMem;
        CPU_INFO_t    cpuInfo;
        DosQuerySysInfo( QSV_TOTPHYSMEM, QSV_TOTPHYSMEM, &ulMem, sizeof(ulMem));
        m_nbytes = ulMem;
        // YD this is not the swap free space, but should be enough
        DosQuerySysInfo( QSV_TOTAVAILMEM, QSV_TOTAVAILMEM, &ulMem, sizeof(ulMem));
        m_swap = ulMem;
    }
#elif LINUX_LIKE_SYSTEM
    parse_meminfo_linux(*this);
#elif defined(_SC_USEABLE_MEMORY)
    // UnixWare
    m_nbytes = (double)sysconf(_SC_PAGESIZE) * (double)sysconf(_SC_USEABLE_MEMORY);
#elif defined(_SC_PHYS_PAGES)
    m_nbytes = (double)sysconf(_SC_PAGESIZE) * (double)sysconf(_SC_PHYS_PAGES);
    if (m_nbytes < 0) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "RAM size not measured correctly: page size %d, #pages %d",
            sysconf(_SC_PAGESIZE), sysconf(_SC_PHYS_PAGES)
        );
    }
#elif defined(__APPLE__)
    // On Mac OS X, sysctl with selectors CTL_HW, HW_PHYSMEM returns only a 
    // 4-byte value, even if passed an 8-byte buffer, and limits the returned 
    // value to 2GB when the actual RAM size is > 2GB.  The Gestalt selector 
    // gestaltPhysicalRAMSizeInMegabytes is available starting with OS 10.3.0.
    SInt32 mem_size;
    if (Gestalt(gestaltPhysicalRAMSizeInMegabytes, &mem_size)) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Couldn't determine physical RAM size"
        );
    }
    m_nbytes = (1024. * 1024.) * (double)mem_size;
#elif defined(_HPUX_SOURCE)
    struct pst_static pst; 
    pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0);
    m_nbytes = (long double)pst.physical_memory * (long double)pst.page_size;
#elif defined(__osf__)
    // Tru64 UNIX.
    // 2005-12-26 SMS.
    int mem_size;
    getsysinfo( GSI_PHYSMEM, (caddr_t) &mem_size, sizeof( mem_size));
    m_nbytes = 1024.* (double)mem_size;
#elif defined(HW_PHYSMEM) 
    // for OpenBSD
    mib[0] = CTL_HW; 
    int mem_size; 
    mib[1] = HW_PHYSMEM; 
    len = sizeof(mem_size); 
    sysctl(mib, 2, &mem_size, &len, NULL, 0); 
    m_nbytes = mem_size; 
#elif defined(__FreeBSD__)
    unsigned int mem_size;
    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;
    len = sizeof(mem_size);
    sysctl(mib, 2, &mem_size, &len, NULL, 0);
    m_nbytes = mem_size;
#else
#error Need to specify a method to get memory size
#endif

#if HAVE_SYS_SWAP_H && defined(SC_GETNSWP)
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
#elif HAVE_SYS_SWAP_H && defined(SWAP_NSWAP)
    // NetBSD (the above line should probably be more comprehensive
    struct swapent * s;
    int i, n;
    n = swapctl(SWAP_NSWAP, NULL, 0);
    s = (struct swapent*)malloc(n * sizeof(struct swapent));
    swapctl(SWAP_STATS, s, n);
    m_swap = 0.0;
    for (i = 0; i < n; i ++) {
        if (s[i].se_flags & SWF_ENABLE) {
            m_swap += 512. * (double)s[i].se_nblks;
        }
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

#elif defined(__HAIKU__)
#warning HAIKU: missing swapfile size info
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
//#error Need to specify a method to obtain swap space
#endif

    get_local_network_info();

    timezone = get_timezone();

///////////// os_name, os_version /////////////////

#if HAVE_SYS_UTSNAME_H
    struct utsname u;
    uname(&u);
#ifdef ANDROID
    safe_strcpy(os_name, "Android");
#else
    safe_strcpy(os_name, u.sysname);
#endif //ANDROID
#if defined(__EMX__) // OS2: version is in u.version
    safe_strcpy(os_version, u.version);
#elif defined(__HAIKU__)
    snprintf(os_version, sizeof(os_version), "%s, %s", u.release, u.version);
#else
    safe_strcpy(os_version, u.release);
#endif
#ifdef _HPUX_SOURCE
    safe_strcpy(p_model, u.machine);
    safe_strcpy(p_vendor, "Hewlett-Packard");
#endif
#elif HAVE_SYS_SYSCTL_H && defined(CTL_KERN) && defined(KERN_OSTYPE) && defined(KERN_OSRELEASE)
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
//
inline bool device_idle(time_t t, const char *device) {
    struct stat sbuf;
    return stat(device, &sbuf) || (sbuf.st_atime < t);
}

static const struct dir_dev {
    const char *dir;
    const char *dev;
} tty_patterns[] = {
#ifdef unix
    { "/dev","tty" },
    { "/dev","pty" },
    { "/dev/pts","" },
#endif
    // add other ifdefs here as necessary.
    { NULL, NULL },
};

vector<string> get_tty_list() {
    // Create a list of all terminal devices on the system.
    char devname[1024];
    char fullname[1024];
    int done,i=0;
    vector<string> tty_list;
    
    do {
        DIRREF dev=dir_open(tty_patterns[i].dir);
        if (dev) {
            do {
                // get next file
                done=dir_scan(devname,dev,1024);
                // does it match our tty pattern? If so, add it to the tty list.
                if (!done && (strstr(devname,tty_patterns[i].dev) == devname)) {
                    // don't add anything starting with .
                    if (devname[0] != '.') {
                        sprintf(fullname,"%s/%s",tty_patterns[i].dir,devname);
                        tty_list.push_back(fullname);
                    }
                }
            } while (!done);
            dir_close(dev);
        }
        i++;
    } while (tty_patterns[i].dir != NULL);
    return tty_list;
}
       

inline bool all_tty_idle(time_t t) {
    static vector<string> tty_list;
    struct stat sbuf;
    unsigned int i;

    if (tty_list.size()==0) tty_list=get_tty_list();
    for (i=0; i<tty_list.size(); i++) {
        // ignore errors
        if (!stat(tty_list[i].c_str(), &sbuf)) {
            // printf("%s %d %d\n",tty_list[i].c_str(),sbuf.st_atime,t);
            if (sbuf.st_atime >= t) {
                return false;
            }
        }
    }
    return true;
}

#if HAVE_UTMP_H
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

#if !HAVE_SETUTENT || !HAVE_GETUTENT
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
#endif  // HAVE_UTMP_H

#ifdef __APPLE__

// NXIdleTime() is an undocumented Apple API to return user idle time, which 
// was implemented from before OS 10.0 through OS 10.5.  In OS 10.4, Apple 
// added the CGEventSourceSecondsSinceLastEventType() API as a replacement for 
// NXIdleTime().  However, BOINC could not use this newer API when configured 
// as a pre-login launchd daemon unless that daemon was running as root, 
// because it could not connect to the Window Server.  So BOINC continued to 
// use NXIdleTime().  
//
// In OS 10.6, Apple removed the NXIdleTime() API.  BOINC can instead use the 
// IOHIDGetParameter() API in OS 10.6.  When BOINC is a pre-login launchd 
// daemon running as user boinc_master, this API works properly under OS 10.6 
// but fails under OS 10.5 and earlier.
//
// In OS 10.7, IOHIDGetParameter() fails to recognize activity from remote 
// logins via Apple Remote Desktop or Screen Sharing (VNC), but the 
// CGEventSourceSecondsSinceLastEventType() API does work with ARD and VNC, 
// except when BOINC is a pre-login launchd daemon running as user boinc_master.
//
// Also, CGEventSourceSecondsSinceLastEventType() does not detect user activity 
// when the user who launched the client is switched out by fast user switching.
//
// So we use weak-linking of NxIdleTime() to prevent a run-time crash from the 
// dynamic linker and use it if it exists. 
// If NXIdleTime does not exist, we call both IOHIDGetParameter() and 
// CGEventSourceSecondsSinceLastEventType().  If both return without error, 
// we use the lower of the two returned values.
//
bool HOST_INFO::users_idle(
    bool check_all_logins, double idle_time_to_run, double *actual_idle_time
) {
    static bool     error_posted = false;
    static long     OSVersionInfo = 0;
    OSStatus        err = noErr;
    double          idleTime = 0;
    double          idleTimeFromCG = 0;
    io_service_t    service;
    kern_return_t   kernResult = kIOReturnError; 
    UInt64          params;
    IOByteCount     rcnt = sizeof(UInt64);
    void            *IOKitlib = NULL;
    static bool     triedToLoadNXIdleTime = false;
    static nxIdleTimeProc  myNxIdleTimeProc = NULL;
    
    if (error_posted) goto bail;
    
    if (!triedToLoadNXIdleTime) {
        triedToLoadNXIdleTime = true;
        err = Gestalt(gestaltSystemVersion, &OSVersionInfo);
        if (err) OSVersionInfo = 0;
        
        IOKitlib = dlopen ("/System/Library/Frameworks/IOKit.framework/IOKit", RTLD_NOW );
        if (IOKitlib) {
            myNxIdleTimeProc = (nxIdleTimeProc)dlsym(IOKitlib, "NXIdleTime");
        }
    }

    if (myNxIdleTimeProc) {   // Use NXIdleTime API in OS 10.5 and earlier
        if (gEventHandle) {
            idleTime = myNxIdleTimeProc(gEventHandle);    
        } else {
            // Initialize Mac OS X idle time measurement / idle detection
            // Do this here because NXOpenEventStatus() may not be available 
            // immediately on system startup when running as a deaemon.

            gEventHandle = NXOpenEventStatus();
            if (!gEventHandle) {
                if (TickCount() > (120*60)) {   // If system has been up for more than 2 minutes 
                     msg_printf(NULL, MSG_INFO,
                        "User idle detection is disabled: initialization failed."
                    );
                    error_posted = true;
                    goto bail;
                }
            }
        }
    } else {        // NXIdleTime API does not exist in OS 10.6 and later
        if (gEventHandle) {
            kernResult = IOHIDGetParameter( gEventHandle, CFSTR(EVSIOIDLE), sizeof(UInt64), &params, &rcnt );
            if ( kernResult != kIOReturnSuccess ) {
                msg_printf(NULL, MSG_INFO,
                    "User idle time measurement failed because IOHIDGetParameter failed."
                );
                error_posted = true;
                goto bail;
            }
            idleTime = ((double)params) / 1000.0 / 1000.0 / 1000.0;
            
             if (!gstate.executing_as_daemon) {
                idleTimeFromCG =  CGEventSourceSecondsSinceLastEventType  
                        (kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);
        
                if (idleTimeFromCG < idleTime) {
                    idleTime = idleTimeFromCG;
                }
            }
        } else {
            service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(kIOHIDSystemClass));
            if (service) {
                 kernResult = IOServiceOpen(service, mach_task_self(), kIOHIDParamConnectType, &gEventHandle);
            }
            if ( (!service) || (kernResult != KERN_SUCCESS) ) {
                // When the system first starts up, allow time for HIDSystem to be available if needed
                if (TickCount() > (120*60)) {        // If system has been up for more than 2 minutes 
                     msg_printf(NULL, MSG_INFO,
                        "Could not connect to HIDSystem: user idle detection is disabled."
                    );
                    error_posted = true;
                    goto bail;
                }
            }
        }   // End (gEventHandle == NULL)
    }           // End NXIdleTime API does not exist
    
 bail:   
    if (actual_idle_time) {
        *actual_idle_time = idleTime;
    }
    return (idleTime > (60 * idle_time_to_run));
}

#else  // ! __APPLE__

#if LINUX_LIKE_SYSTEM
bool interrupts_idle(time_t t) {
    // This method doesn't really work reliably on USB keyboards and mice.
    static FILE *ifp = NULL;
    static long irq_count[256];
    static time_t last_irq = time(NULL);

    char line[256];
    int i = 0;
    long ccount = 0;

    if (ifp == NULL) {
        if ((ifp = fopen("/proc/interrupts", "r")) == NULL) {
            return true;
        }
    }
    rewind(ifp);
    while (fgets(line, sizeof(line), ifp)) {
        // Check for mouse, keyboard and PS/2 devices.
        if (strcasestr(line, "mouse") != NULL ||
            strcasestr(line, "keyboard") != NULL ||
            strcasestr(line, "i8042") != NULL) {
            // If any IRQ count changed, update last_irq.
            if (sscanf(line, "%d: %ld", &i, &ccount) == 2
                && irq_count[i] != ccount
            ) {
                last_irq = time(NULL);
                irq_count[i] = ccount;
            }
        }
    }
    return last_irq < t;
}

#if HAVE_XSS
// Ask the X server for user idle time (using XScreenSaver API)
// Returns true if the idle_treshold is smaller than the
// idle time of the user (means: true = user is idle)
bool xss_idle(long idle_treshold) {
    static XScreenSaverInfo* xssInfo = NULL;
    static Display* disp = NULL;
    
    long idle_time = 0;
    
    if(disp != NULL) {
        XScreenSaverQueryInfo(disp, DefaultRootWindow(disp), xssInfo);
        idle_time = xssInfo->idle / 1000; // xssInfo->idle is in ms
    } else {
        disp = XOpenDisplay(NULL);
        // XOpenDisplay may return NULL if there is no running X
        // or DISPLAY points to wrong/invalid display
        if(disp != NULL) {
            xssInfo = XScreenSaverAllocInfo();
        }
    }

    return idle_treshold < idle_time;
}
#endif // HAVE_XSS
#endif // LINUX_LIKE_SYSTEM

bool HOST_INFO::users_idle(bool check_all_logins, double idle_time_to_run) {
    time_t idle_time = time(0) - (long) (60 * idle_time_to_run);

#if HAVE_UTMP_H
    if (check_all_logins) {
        if (!all_logins_idle(idle_time)) {
            return false;
        }
    }
#endif

    if (!all_tty_idle(idle_time)) {
        return false;
    }

#if LINUX_LIKE_SYSTEM
    // Check /proc/interrupts to detect keyboard or mouse activity.
    // this ignores USB keyboards/mice.  They don't use the keyboard
    // and mouse interrupts.
    if (!interrupts_idle(idle_time)) {
        return false;
    }

    // Lets at least check the dev entries which should be correct for
    // USB mice.  The tty check will catch keyboards if they are entering
    // data into a tty.
    if (!device_idle(idle_time, "/dev/input/mice")) return false;

#if HAVE_XSS
    if (!xss_idle((long)(idle_time_to_run * 60))) {
        return false;
    }
#endif

#else
    // We should find out which of the following are actually relevant
    // on which systems (if any)
    //
    if (!device_idle(idle_time, "/dev/mouse")) return false;
        // solaris, linux
    if (!device_idle(idle_time, "/dev/input/mice")) return false;
    if (!device_idle(idle_time, "/dev/kbd")) return false;
        // solaris
#endif
    return true;
}

#endif  // ! __APPLE__

