// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// Functions to get host info (CPU, network, disk, mem) on unix-based systems.
// Lots of this is system-dependent so lots of #ifdefs.
// Try to keep this well-organized and not nested.

#include "version.h"         // version numbers from autoconf
#include "cpp.h"
#include "config.h"

#if !defined(_WIN32) || defined(__CYGWIN32__)

// Access to binary files in /proc filesystem doesn't work in the 64bit
// files environment on some systems.
// None of the functions here need 64bit file functions,
// so undefine _FILE_OFFSET_BITS and _LARGE_FILES.
//
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#endif

#ifdef __GLIBC__
#include <gnu/libc-version.h>
#endif

#if HAVE_XSS
#include <X11/extensions/scrnsaver.h> //X-based idle detection
// prevents naming collision between X.h define of Always and boinc's
// lib/prefs.h definition in an enum.
#undef Always
#include <dirent.h> //for opening /tmp/.X11-unix/
  // (There is a DirScanner class in BOINC, but it doesn't do what we want)
#include "log_flags.h" // idle_detection_debug flag for verbose output
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
#if HAVE_SYS_SENSORS_H
#include <sys/sensors.h>
#endif

#ifdef __EMX__
#define INCL_DOSMISC
#include <os2.h>
#include "win/opt_x86.h"
#endif

#ifdef ARMV6
#include <sys/wait.h>
#endif

#include "error_numbers.h"
#include "common_defs.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "client_state.h"
#include "client_types.h"
#include "client_msgs.h"
#include "hostinfo.h"

using std::string;
using std::min;

#ifdef __APPLE__
#include "sandbox.h"
#include <IOKit/IOKitLib.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach/machine.h>
#include <libkern/OSByteOrder.h>

extern int compareOSVersionTo(int toMajor, int toMinor);

#ifdef __cplusplus
extern "C" {
#endif
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#ifdef __cplusplus
}    // extern "C"

#include <dlfcn.h>
#endif
int podman_init_pid = 0;
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

// The following is intended to be true both on Linux
// and Debian GNU/kFreeBSD (see trac #521)
//
#if (defined(__linux__) || defined(__GNU__) || defined(__GLIBC__))  && !defined(__HAIKU__)
#define LINUX_LIKE_SYSTEM 1
#endif

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
    return !(gstate.device_status.on_ac_power || gstate.device_status.on_usb_power);

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
            char *p = fgets(buf, sizeof(buf), fsys);
            fclose(fsys);
            if (p == NULL) {
                break;
            }
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
            int n = fscanf(fapm, "%10s %d.%d %x %x",
                apm_driver_version,
                &apm_major_version,
                &apm_minor_version,
                &apm_flags,
                &apm_ac_line_status
            );

            fclose(fapm);
            if (n != 5) return false;

            return (apm_ac_line_status == 0);
        }
    case ProcACPI:
        {
            // use /proc/acpi/ac_adapter/*/stat{e,us}
            FILE *facpi = fopen(path, "r");
            if (!facpi) return false;

            char buf[128];
            char *p = fgets(buf, sizeof(buf), facpi);

            fclose(facpi);
            if (p == NULL) return false;

            if ((strstr(buf, "state:") != NULL) || (strstr(buf, "Status:") != NULL)) {
                // on batteries if ac adapter is "off-line" (or maybe "offline")
                return (strstr(buf, "off") != NULL);
            }

            return false;
        }
    case SysClass:
        {
            // use /sys/class/power_supply/*/online
            FILE *fsys = fopen(path, "r");
            if (!fsys) return false;

            int online;
            int n = fscanf(fsys, "%d", &online);
            fclose(fsys);
            if (n != 1) return false;

            // online is 1 if on AC power, 0 if on battery
            return (0 == online);
        }
    case NoBattery:
    default:
         // we have no way to determine if we're on batteries,
         // so we say we aren't
        return false;
    }
#elif defined(__OpenBSD__)
    static int mib[] = {CTL_HW, HW_SENSORS, 0, 0, 0};
    static int devn = -1;
    struct sensor s;
    size_t slen = sizeof(struct sensor);

    if (devn == -1) {
        struct sensordev snsrdev;
        size_t sdlen = sizeof(struct sensordev);
        for (devn = 0;; devn++) {
            mib[2] = devn;
            if (sysctl(mib, 3, &snsrdev, &sdlen, NULL, 0) == -1) {
                if (errno == ENXIO) {
                    continue;
                }
                if (errno == ENOENT) {
                    break;
                }
            }
            if (!strcmp("acpiac0", snsrdev.xname)) {
                break;
            }
        }
        mib[3] = 9;
        mib[4] = 0;
    }

    if (sysctl(mib, 5, &s, &slen, NULL, 0) != -1) {
        if (s.value) {
            // AC present
            return false;
        } else {
            return true;
        }
    }

    return false;
#elif defined(__FreeBSD__)
    int ac;
    size_t len = sizeof(ac);

    if (sysctlbyname("hw.acpi.acline", &ac, &len, NULL, 0) != -1) {
        if (ac) {
            // AC present
            return false;
        } else {
            return true;
        }
    }

    return false;
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
    char buf[P_FEATURES_SIZE], features[P_FEATURES_SIZE], model_buf[1024];
    bool vendor_found=false, model_found=false;
    bool cache_found=false, features_found=false;
    bool model_hack=false, vendor_hack=false;
    int n;
#if !defined(__aarch64__) && !defined(__arm__)
    int family=-1, model=-1, stepping=-1;
#else
    char implementer[32] = {0}, architecture[32] = {0}, variant[32] = {0}, cpu_part[32] = {0}, revision[32] = {0};
    bool model_info_found=false;
#endif
    char buf2[256];

    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) {
        msg_printf(NULL, MSG_INFO,
            "Can't open /proc/cpuinfo to get CPU info"
        );
        safe_strcpy(host.p_model, "unknown");
        safe_strcpy(host.p_vendor, "unknown");
        return;
    }

#ifdef __mips__
    safe_strcpy(host.p_model, "MIPS ");
    model_hack = true;
#elif __alpha__
    safe_strcpy(host.p_vendor, "HP (DEC) ");
    vendor_hack = true;
#elif __hppa__
    safe_strcpy(host.p_vendor, "HP ");
    vendor_hack = true;
#elif __ia64__
    safe_strcpy(host.p_model, "IA-64 ");
    model_hack = true;
#elif defined(__arm__) || defined(__aarch64__)
    safe_strcpy(host.p_vendor, "ARM");
    vendor_hack = vendor_found = true;
#endif

    host.m_cache=-1;
    safe_strcpy(features, "");
    while (fgets(buf, sizeof(buf), f)) {
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

#if defined(__aarch64__) || defined(__arm__)
        if (
            // Hardware is specifying the board this CPU is on, store it in product_name while we parse /proc/cpuinfo
            strstr(buf, "Hardware\t: ")
        ) {
            // this makes sure we only ever copy as much bytes as we can still store in host.product_name
            int t = sizeof(host.product_name) - strlen(host.product_name) - 2;
            strlcpy(buf2, strchr(buf, ':') + 2, ((t<sizeof(buf2))?t:sizeof(buf2)));
            strip_whitespace(buf2);
            if (strlen(host.product_name)) {
                safe_strcat(host.product_name, " ");
            }
            safe_strcat(host.product_name, buf2);
        }
#endif

        if (
#ifdef __ia64__
            strstr(buf, "family     : ") || strstr(buf, "model name : ")
#elif __powerpc__ || __sparc__
            strstr(buf, "cpu\t\t: ")
#elif defined(__aarch64__) || defined(__arm__)
            // Hardware is a fallback specifying the board this CPU is on (not ideal but better than nothing)
            strstr(buf, "model name") || strstr(buf, "Processor") || strstr(buf, "Hardware")
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
                    safe_strcpy(features, "altivec");
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
                strlcpy(buf2, strchr(buf, ':') + 1, sizeof(host.p_model) - strlen(host.p_model) - 1);
                strip_whitespace(buf2);
                safe_strcat(host.p_model, buf2);
            }
        }
#if  !defined(__hppa__) && !defined(__aarch64__) && !defined(__arm__)
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
#if !defined(__aarch64__) && !defined(__arm__)
        if (strstr(buf, "stepping\t: ") && stepping<0) {
            stepping = atoi(buf+strlen("stepping\t: "));
        }
#else
        if (strstr(buf, "CPU implementer") && strlen(implementer) == 0) {
            strlcpy(implementer, strchr(buf, ':') + 2, sizeof(implementer));
            model_info_found = true;
        }
        if (strstr(buf, "CPU architecture") && strlen(architecture) == 0) {
            strlcpy(architecture, strchr(buf, ':') + 2, sizeof(architecture));
            model_info_found = true;
        }
        if (strstr(buf, "CPU variant") && strlen(variant) == 0) {
            strlcpy(variant, strchr(buf, ':') + 2, sizeof(variant));
            model_info_found = true;
        }
        if (strstr(buf, "CPU part") && strlen(cpu_part) == 0) {
            strlcpy(cpu_part, strchr(buf, ':') + 2, sizeof(cpu_part));
            model_info_found = true;
        }
        if (strstr(buf, "CPU revision") && strlen(revision) == 0) {
            strlcpy(revision, strchr(buf, ':') + 2, sizeof(revision));
            model_info_found = true;
        }
#endif
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
            } else if ((strstr(buf, "Features\t: ") == buf)) { /* arm */
               strlcpy(features, strchr(buf, ':') + 2, sizeof(features));
            }
            if (strlen(features)) {
                features_found = true;
            }
        }
    }
    safe_strcpy(model_buf, host.p_model);
#if !defined(__aarch64__) && !defined(__arm__)
    if (family>=0 || model>=0 || stepping>0) {
        safe_strcat(model_buf, " [");
        if (family>=0) {
            snprintf(buf, sizeof(buf), "Family %d ", family);
            safe_strcat(model_buf, buf);
        }
        if (model>=0) {
            snprintf(buf, sizeof(buf), "Model %d ", model);
            safe_strcat(model_buf, buf);
        }
        if (stepping>=0) {
            snprintf(buf, sizeof(buf), "Stepping %d", stepping);
            safe_strcat(model_buf, buf);
        }
        safe_strcat(model_buf, "]");
    }
#else
    if (model_info_found) {
        safe_strcat(model_buf, " [");
        if (strlen(implementer)>0) {
            snprintf(buf, sizeof(buf), "Impl %s ", implementer);
            safe_strcat(model_buf, buf);
        }
        if (strlen(architecture)>0) {
            snprintf(buf, sizeof(buf), "Arch %s ", architecture);
            safe_strcat(model_buf, buf);
        }
        if (strlen(variant)>0) {
            snprintf(buf, sizeof(buf), "Variant %s ", variant);
            safe_strcat(model_buf, buf);
        }
        if (strlen(cpu_part)>0) {
            snprintf(buf, sizeof(buf), "Part %s ", cpu_part);
            safe_strcat(model_buf, buf);
        }
        if (strlen(revision)>0) {
            snprintf(buf, sizeof(buf), "Rev %s", revision);
            safe_strcat(model_buf, buf);
        }
        safe_strcat(model_buf, "]");
    }
#endif
    if (strlen(features)) {
        safe_strcpy(host.p_features, features);
    }

    safe_strcpy(host.p_model, model_buf);
    fclose(f);
}
#endif  // LINUX_LIKE_SYSTEM
#ifdef __FreeBSD__
#if defined(__i386__) || defined(__amd64__)
#include <sys/types.h>
#include <sys/cdefs.h>
#include <machine/cpufunc.h>
#include <machine/specialreg.h>

void use_cpuid(HOST_INFO& host) {
    u_int p[4];
    u_int cpu_id;
    char vendor[13];
    int hasMMX, hasSSE, hasSSE2, hasSSE3, has3DNow, has3DNowExt, hasAVX;
    char capabilities[P_FEATURES_SIZE];

    hasMMX = hasSSE = hasSSE2 = hasSSE3 = has3DNow = has3DNowExt = hasAVX = 0;
    do_cpuid(0x0, p);

    if (p[0] >= 0x1) {

        do_cpuid(0x1, p);

        cpu_id = p[0];
        memcpy(vendor, &p[1], 4);   // copy EBX
        memcpy(vendor+4, &p[3], 4); // copy EDX
        memcpy(vendor+8, &p[2], 4); // copy ECX
        vendor[12] = '\0';
        hasMMX  = (p[3] & (1 << 23 )) >> 23; // 0x0800000
        hasSSE  = (p[3] & (1 << 25 )) >> 25; // 0x2000000
        hasSSE2 = (p[3] & (1 << 26 )) >> 26; // 0x4000000
        hasAVX  = (p[2] & (1 << 28 )) >> 28;
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
    if (hasSSE) safe_strcat(capabilities, "sse ");
    if (hasSSE2) safe_strcat(capabilities, "sse2 ");
    if (hasSSE3) safe_strcat(capabilities, "pni ");
    if (has3DNow) safe_strcat(capabilities, "3dnow ");
    if (has3DNowExt) safe_strcat(capabilities, "3dnowext ");
    if (hasMMX) safe_strcat(capabilities, "mmx ");
    if (hasAVX) safe_strcat(capabilities, "avx ");
    strip_whitespace(capabilities);
    char buf[1024];
    snprintf(buf, sizeof(buf), " [Family %u Model %u Stepping %u]",
        CPUID_TO_FAMILY(cpu_id), CPUID_TO_MODEL(cpu_id), cpu_id & CPUID_STEPPING
    );
    strlcat(host.p_model, buf, sizeof(host.p_model));
    safe_strcpy(host.p_features, capabilities);
    safe_strcpy(host.p_vendor, vendor);
}
#endif
#endif

#ifdef __APPLE__
static void get_cpu_info_mac(HOST_INFO& host) {
    int p_model_size = sizeof(host.p_model);
    size_t len;
    char brand_string[256];
    char features[P_FEATURES_SIZE];
    char *p;
    char *sep=" ";

    len = sizeof(brand_string);
    sysctlbyname("machdep.cpu.brand_string", brand_string, &len, NULL, 0);

#if defined(__i386__) || defined(__x86_64__)

    int family, stepping, model;

    // on an Apple M1 chip the cpu.vendor is broken, family, model and stepping don't exist
    if (!strncmp(brand_string, "Apple M", strlen("Apple M"))) {

        strcpy(host.p_vendor, "Apple");
        strncpy(host.p_model, brand_string, sizeof(host.p_model));

    } else {

        len = sizeof(host.p_vendor);
        sysctlbyname("machdep.cpu.vendor", host.p_vendor, &len, NULL, 0);

        len = sizeof(family);
        sysctlbyname("machdep.cpu.family", &family, &len, NULL, 0);

        len = sizeof(model);
        sysctlbyname("machdep.cpu.model", &model, &len, NULL, 0);

        len = sizeof(stepping);
        sysctlbyname("machdep.cpu.stepping", &stepping, &len, NULL, 0);

        snprintf(
            host.p_model, sizeof(host.p_model),
            "%s [x86 Family %d Model %d Stepping %d]",
            brand_string, family, model, stepping
        );
    }

    len = sizeof(features);
    sysctlbyname("machdep.cpu.features", features, &len, NULL, 0);

#else // defined(__i386__) || defined(__x86_64__)

    int feature;
    string feature_string;

    strcpy(host.p_vendor, "Apple");
    strncpy(host.p_model, brand_string, sizeof(host.p_model));

    features[0] = '\0';
    len = sizeof(feature);
    feature_string="";

    sysctlbyname("hw.optional.amx_version", &feature, &len, NULL, 0);
    snprintf(features, sizeof(features), "amx_version_%d", feature);
    feature_string += features;

    sysctlbyname("hw.optional.arm64", &feature, &len, NULL, 0);
    if (feature) feature_string += " arm64";

    sysctlbyname("hw.optional.armv8_1_atomics", &feature, &len, NULL, 0);
    if (feature) feature_string += " armv8_1_atomics";

    sysctlbyname("hw.optional.armv8_2_fhm", &feature, &len, NULL, 0);
    if (feature) feature_string += " armv8_2_fhm";

    sysctlbyname("hw.optional.armv8_2_sha3", &feature, &len, NULL, 0);
    if (feature) feature_string += " armv8_2_sha3";

    sysctlbyname("hw.optional.armv8_2_sha512", &feature, &len, NULL, 0);
    if (feature) feature_string += " armv8_2_sha512";

    sysctlbyname("hw.optional.armv8_crc32", &feature, &len, NULL, 0);
    if (feature) feature_string += " armv8_crc32";

    sysctlbyname("hw.optional.floatingpoint", &feature, &len, NULL, 0);
    if (feature) feature_string += " floatingpoint";

    sysctlbyname("hw.optional.neon", &feature, &len, NULL, 0);
    if (feature) feature_string += " neon";

    sysctlbyname("hw.optional.neon_fp16", &feature, &len, NULL, 0);
    if (feature) feature_string += " neon_fp16";

    sysctlbyname("hw.optional.neon_hpfp", &feature, &len, NULL, 0);
    if (feature) feature_string += " neon_hpfp";

    sysctlbyname("hw.optional.ucnormal_mem", &feature, &len, NULL, 0);
    if (feature) feature_string += " ucnormal_mem";

    // read features of the emulated CPU if there is a file containing these
    char fpath[MAXPATHLEN];
    boinc_getcwd(fpath);
    strcat(fpath,"/");
    strcat(fpath,EMULATED_CPU_INFO_FILENAME);
    if (boinc_file_exists(fpath)) {
        FILE* fp = boinc_fopen(fpath, "r");
        if (fp) {
            fgets(features, sizeof(features), fp);
	    feature_string += features;
            fclose(fp);
        } else if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO, "[x86_64-M1] couldn't open file %s", fpath);
        }
    } else if (log_flags.coproc_debug) {
        msg_printf(0, MSG_INFO, "[x86_64-M1] didn't find file %s", fpath);
    }

    strncpy(features,feature_string.c_str(),sizeof(features));

#endif // defined(__i386__) || defined(__x86_64__)

    // Convert Mac CPU features string to match that returned by Linux
    for(p=features; *p; p++) {
        *p = tolower(*p);
    }

    host.p_features[0] = 0;
    for (p = strtok(features, sep); p; p = strtok(NULL, sep)) {
    if (p != features) safe_strcat(host.p_features, sep);
        if (!strcmp(p, "avx1.0")) {
            safe_strcat(host.p_features, "avx");
        } else if (!strcmp(p, "sse3")) {
            safe_strcat(host.p_features, "pni");
        } else if (!strcmp(p, "sse4.1")) {
            safe_strcat(host.p_features, "sse4_1");
        } else if (!strcmp(p, "sse4.2")) {
            safe_strcat(host.p_features, "sse4_2");
        } else {
            safe_strcat(host.p_features, p);
        }
    }

    host.p_model[p_model_size-1] = 0;

    // This returns an Apple hardware model designation such as "MacPro3,1".
    // One source for converting this to a common model name is:
    // <http://www.everymac.com/systems/by_capability/mac-specs-by-machine-model-machine-id.html>
    len = sizeof(host.product_name);
    sysctlbyname("hw.model", host.product_name, &len, NULL, 0);
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
        cpuInfo.eax_0.vendor_id
    );

    maxStandardFunction = cpuInfo.eax_0.max_eax;
    if (maxStandardFunction >= 500) {
        // old Pentium sample chips has cpu signature here
        maxStandardFunction = 0;
    }

    // Extended cpuid
    get_cpuid(&cpuInfo, 0x80000000, cpu);

    // extended cpuid is only supported if max_eax is greater
    // than the service id
    //
    if (cpuInfo.eax_0.max_eax > 0x80000000) {
        maxExtendedFunction = cpuInfo.eax_0.max_eax & 0xff;
    }

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
        while (name[0] == ' ') {
            name++;
        }

        // the BIOS may not have set the processor name
        if (name[0]) {
            strlcpy(brand_string, name, sizeof(brand_string));
        } else {
            // Intel CPUs don't seem to have the genuine vendor field
            snprintf(brand_string, sizeof(brand_string), "%.12s",
                cpuInfo.eax_0.vendor_id
            );
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
        stepping
    );

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
    char buf[256];

    for (i = 0; i < 32; i++) {
        if ((cpuInfo.eax_1.features & (1UL << i)) && kFeatures[i] != NULL) {
            snprintf(buf, sizeof(buf), "%s%s", found == 0 ? "" : " ",
                kFeatures[i]
            );
            strlcat(host.p_features, buf, sizeof(host.p_features));
            found++;
        }
    }

    if (maxStandardFunction >= 1) {
        /* Extended features */
        static const char *kFeatures2[32] = {
            "pni", NULL, "dtes64", "monitor", "ds-cpl", "vmx", "smx" "est",
            "tm2", "ssse3", "cnxt-id", NULL, NULL, "cx16", "xtpr", "pdcm",
            NULL, NULL, "dca", "sse4_1", "sse4_2", "x2apic", "movbe", "popcnt",
            NULL, NULL, "xsave", "osxsave", NULL, NULL, NULL, NULL
        };

        for (i = 0; i < 32; i++) {
            if ((cpuInfo.eax_1.extended_features & (1UL << i)) &&
                kFeatures2[i] != NULL) {
                snprintf(buf, sizeof(buf), "%s%s", found == 0 ? "" : " ",
                    kFeatures2[i]
                );
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

// Is this a dual GPU MacBook with automatic GPU switching?
bool isDualGPUMacBook() {
    io_service_t service = IO_OBJECT_NULL;

    service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleGraphicsControl"));
    return (service != IO_OBJECT_NULL);
}

#endif  // __APPLE__

// see if Virtualbox is installed
//
const char* vbox_locations[] = {
    "/usr/bin/VBoxManage",
    "/usr/local/bin/VBoxManage",
    // add other ifdefs here as necessary.
    NULL
};

int HOST_INFO::get_virtualbox_version() {
    const char** paths;
    char cmd [MAXPATHLEN+35];
    char buf[256];
    FILE* fd;

    for (paths = vbox_locations; *paths != NULL; ++paths) {
        const char* path = *paths;
        if (access(path, X_OK)) {
            continue;
        }
        safe_strcpy(cmd, path);
        safe_strcat(cmd, " --version");
        fd = popen(cmd, "r");
        if (fd) {
            if (fgets(buf, sizeof(buf), fd)) {
                strip_whitespace(buf);
                int n, a,b,c;
                n = sscanf(buf, "%d.%d.%d", &a, &b, &c);
                if (n == 3) {
                    safe_strcpy(virtualbox_version, buf);
                    pclose(fd);
                    break;
                }
            }
            pclose(fd);
        }
    }
    return 0;
}

#ifdef __APPLE__
bool HOST_INFO::is_podman_VM_running() {
    char cmd[1024], buf[256];

    snprintf(cmd, sizeof(cmd), "%s machine list",
        docker_cli_prog(PODMAN)
    );
    FILE* f = popen(cmd, "r");
    if (!f) return false;
    bool isrunning = false;
    while (fgets(buf, sizeof(buf), f)) {
        if (strcasestr(buf, "running")) {
            isrunning = true;
            break;
        }
    }
    return isrunning;
}
#endif

// check if docker/podman is installed and functional on this host.
// if so, populate docker_version and return true
//
bool HOST_INFO::get_docker_version_aux(DOCKER_TYPE type){
    char cmd[1024], buf[256];

#ifdef __APPLE__
    if (docker_cli_prog(type)[0] == '\0') return false;
#endif
    snprintf(cmd, sizeof(cmd), "%s --version 2>/dev/null",
        docker_cli_prog(type)
    );
    FILE* f = popen(cmd, "r");
    if (!f) return false;
    // normally the version is on the first line,
    // but it's on the 2nd line if using podman-docker
    //
    bool found = false;
    while (fgets(buf, sizeof(buf), f)) {
        string version;
        if (get_docker_version_string(type, buf, version)) {
            safe_strcpy(docker_version, version.c_str());
            found = true;
            break;
        }
    }
    pclose(f);
    if (!found) return false;

#ifdef __APPLE__
    // download (if not there) and start QEMU VM
    if (type == PODMAN) {
        snprintf(cmd, sizeof(cmd),
            "%s machine init -v /Library:/Library; %s machine start",
            docker_cli_prog(type),
            docker_cli_prog(type)
        );
        char * argv[4];
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = cmd;
        argv[3] = NULL;
        run_program(NULL, "/bin/sh", 0, argv, podman_init_pid);

#if 0   // For debugging
        snprintf(cmd, sizeof(cmd),
                 "%s machine list",
            docker_cli_prog(type)
        );
        // fprintf(stderr, "\ncmd = %s\n\n", cmd);   // For debugging
        f = popen(cmd, "r");
        if (f) {
            char buf[256];
            while (fgets(buf, sizeof(buf), f)) {
                fprintf(stderr, "podman machine list returned: %s\n", buf);
            }
        }
        pclose(f);
#endif
    }
#endif  // __APPLE__

#ifdef __linux__
    // if we're running as an unprivileged user, Docker/podman may not work.
    // Check by running the Hello World image.
    //
    // Since we do this every time on startup: delete the created container
    // but don't delete the image.
    //
    snprintf(cmd, sizeof(cmd),
        "%s run --rm hello-world 2>/dev/null",
        docker_cli_prog(type)
    );
    found = false;
    f = popen(cmd, "r");
    if (f) {
        while (fgets(buf, sizeof(buf), f)) {
            if (strstr(buf, "Hello")) {
                found = true;
                break;
            }
        }
        pclose(f);
    }
    if (!found) {
        msg_printf(NULL, MSG_INFO,
            "%s found but 'hello-world' test failed",
            docker_type_str(type)
        );
        docker_version[0] = 0;
        return false;
    }
#endif
    return true;
}

bool HOST_INFO::get_docker_version(){
    bool check_podman = true;
#ifdef __linux__
    // podman doesn't work on Linux with remote FS
    bool remote;
    int retval = is_filesystem_remote(".", remote);
    if (!retval && remote) {
        check_podman = false;
        msg_printf(NULL, MSG_INFO, "Data dir is remote: not checking podman");
    }
#endif

    if (check_podman) {
        if (get_docker_version_aux(PODMAN)) {
            docker_type = PODMAN;
            return true;
        }
    }

    if (get_docker_version_aux(DOCKER)) {
        docker_type = DOCKER;
        return true;
    }
    return false;
}

// check if docker compose is installed on this host
// populate docker_compose_version on success
//
bool HOST_INFO::get_docker_compose_version_aux(DOCKER_TYPE type){
    bool ret = false;
    char cmd[1024];

    snprintf(cmd, sizeof(cmd),
        "%s compose version 2>/dev/null",
        docker_cli_prog(type)
    );
    FILE* f = popen(cmd, "r");
    if (f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), f)) {
            string version;
            if (get_docker_compose_version_string(type, buf, version)) {
                safe_strcpy(docker_compose_version, version.c_str());
                docker_compose_type = type;
                ret = true;
                break;
            }
        }
        pclose(f);
    }
    return ret;
}

bool HOST_INFO::get_docker_compose_version(){
#if defined(__APPLE__) || defined(_WIN32)
    if (get_docker_compose_version_aux(PODMAN)) {
        return true;
    }
#endif
    if (get_docker_compose_version_aux(DOCKER)) {
        return true;
    }
    return false;
}

// get p_vendor, p_model, p_features
//
int HOST_INFO::get_cpu_info() {
#if LINUX_LIKE_SYSTEM
    parse_cpuinfo_linux(*this);
#elif defined( __APPLE__)
    get_cpu_info_mac(*this);
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

    safe_strcpy(p_vendor, "HP (DEC)");

    getsysinfo( GSI_PROC_TYPE, (caddr_t) &cpu_type, sizeof( cpu_type));
    CPU_TYPE_TO_TEXT( (cpu_type& 0xffffffff), cpu_type_name);
    strlcpy(p_model, "Alpha ", sizeof(p_model));
    strlcat(p_model, cpu_type_name, sizeof(p_model));
    p_model[sizeof(p_model)-1]=0;
#elif HAVE_SYS_SYSTEMINFO_H
    sysinfo(SI_PLATFORM, p_vendor, sizeof(p_vendor));
    sysinfo(SI_ISALIST, p_model, sizeof(p_model));
#else
#error Need to specify a method to get p_vendor, p_model
#endif

#if defined(__FreeBSD__)
#if defined(__i386__) || defined(__amd64__)
    use_cpuid(*this);
#endif
#endif
    return 0;
}

// get p_ncpus
//
int HOST_INFO::get_cpu_count() {

#if defined(ANDROID)
    // this should work on most devices
    p_ncpus = sysconf(_SC_NPROCESSORS_CONF);

    // work around for bug in Android's bionic
    // format of /sys/devices/system/cpu/present:
    // 0 : single core
    // 0-j: j+1 cores (e.g. 0-3 quadcore)
    FILE* fp;
    int res, i=-1, j=-1, cpus_sys_path=0;
    fp = fopen("/sys/devices/system/cpu/present", "r");
    if(fp) {
        res = fscanf(fp, "%d-%d", &i, &j);
        fclose(fp);
        if(res == 1 && i == 0) {
            cpus_sys_path = 1;
        }
        if(res == 2 && i == 0) {
            cpus_sys_path = j + 1;
        }
    }

    // return whatever number is greater
    if(cpus_sys_path > p_ncpus){
        p_ncpus = cpus_sys_path;
    }
#elif __GNU_LIBRARY__ /* glibc */
    cpu_set_t set;

    if (sched_getaffinity (0, sizeof (set), &set) == 0) {
        p_ncpus = CPU_COUNT (&set);
    }
#elif defined(_SC_NPROCESSORS_ONLN) && !defined(__EMX__) && !defined(__APPLE__)
    // sysconf not working on OS2
    p_ncpus = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(HAVE_SYS_SYSCTL_H) && defined(CTL_HW) && defined(HW_NCPU)
    // Get number of CPUs
    int mib[2];
    size_t len;

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

    return 0;
}

// get m_nbytes, m_swap
//
int HOST_INFO::get_memory_info() {
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
#elif defined(__APPLE__)
    // On Mac OS X, sysctl with selectors CTL_HW, HW_PHYSMEM returns only a
    // 4-byte value, even if passed an 8-byte buffer, and limits the returned
    // value to 2GB when the actual RAM size is > 2GB.
    // But HW_MEMSIZE returns a uint64_t value.
    //
    // _SC_PHYS_PAGES is defined as of Apple SDK 10.11 but sysconf(_SC_PHYS_PAGES)
    // fails in older OS X versions, so we must test __APPLE__ before _SC_PHYS_PAGES
    uint64_t mem_size;
    size_t len = sizeof(mem_size);
    sysctlbyname("hw.memsize", &mem_size, &len, NULL, 0);
    m_nbytes = mem_size;
#elif defined(_SC_PHYS_PAGES)
    m_nbytes = (double)sysconf(_SC_PAGESIZE) * (double)sysconf(_SC_PHYS_PAGES);
    if (m_nbytes < 0) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "RAM size not measured correctly: page size %ld, #pages %ld",
            sysconf(_SC_PAGESIZE), sysconf(_SC_PHYS_PAGES)
        );
    }
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
    // for OpenBSD & NetBSD & FreeBSD
    int mem_size;
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
    //    of the available space on the machine's boot partition.
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

    return 0;
}

#if LINUX_LIKE_SYSTEM
// gather libc version from the system installed ldd binary
// a usual first line looks like: ldd (Debian GLIBC 2.24-11+deb9u1) 2.24
// where the last part is copied to version and the string between parenthesis
// is copied to extra_info
//
// return BOINC_SUCCESS if at least version could be found (extra_info may remain empty)
// return ERR_NOT_FOUND if ldd couldn't be opened or no version information was found
//
#ifdef __GLIBC__
int get_libc_version(string& version, string&) {
    version = string(gnu_get_libc_version());
    return BOINC_SUCCESS;
}
#else
int get_libc_version(string& version, string& extra_info) {
    char buf[1024] = "";
    string strbuf;
    FILE* f = popen("PATH=/usr/bin:/bin:/usr/local/bin ldd --version 2>&1", "r");
    if (f) {
        char* retval = fgets(buf, sizeof(buf), f);
        strbuf = (string)buf;
        while (fgets(buf, sizeof(buf), f)) {
            // consume output to allow command to exit gracefully
        }
        int status = pclose(f);
        if (!retval || status == -1 || !WIFEXITED(status) || WEXITSTATUS(status)) {
            return ERR_NOT_FOUND;
        }
        strip_whitespace(strbuf);
        string::size_type parens1 = strbuf.find('(');
        string::size_type parens2 = strbuf.rfind(')');
        string::size_type blank = strbuf.rfind(' ');

        if (blank != string::npos) {
            // extract version number
            version = strbuf.substr(blank+1);
        } else {
            return ERR_NOT_FOUND;
        }
        if (parens1 != string::npos && parens2 != string::npos && parens1 < parens2) {
            // extract extra information without parenthesis
            extra_info = strbuf.substr(parens1+1, parens2-parens1-1);
        }
    } else {
        return ERR_NOT_FOUND;
    }
    return BOINC_SUCCESS;
}
#endif
#endif

// get os_name, os_version
//
int HOST_INFO::get_os_info() {

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

#if LINUX_LIKE_SYSTEM
    bool found_something = false;
    char dist_name[256], dist_version[256];
    string os_version_extra("");
    safe_strcpy(dist_name, "");
    safe_strcpy(dist_version, "");

    // see: http://refspecs.linuxbase.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/lsbrelease.html
    // although the output is not clearly specified it seems to be constant
    FILE* f = popen(command_lsbrelease, "r");
    if (f) {
        found_something = parse_linux_os_info(f, lsbrelease, dist_name, sizeof(dist_name),
            dist_version, sizeof(dist_version));
        pclose(f);
    }
    if (!found_something) {
        // see: https://www.freedesktop.org/software/systemd/man/os-release.html
        f = fopen(file_osrelease, "r");
        if (f) {
            found_something = parse_linux_os_info(f, osrelease, dist_name, sizeof(dist_name),
                dist_version, sizeof(dist_version));
            fclose(f);
        }
    }

    if (!found_something) {
        // last ditch effort for older redhat releases
        f = fopen(file_redhatrelease, "r");
        if (f) {
            found_something = parse_linux_os_info(f, redhatrelease, dist_name, sizeof(dist_name),
                dist_version, sizeof(dist_version));
            fclose(f);
        }
    }

    if (found_something) {
        os_version_extra = (string)os_version;
        safe_strcpy(os_version, dist_version);
        if (strlen(dist_name)) {
            safe_strcat(os_name, " ");
            safe_strcat(os_name, dist_name);
        }
    }

    string libc_version(""), libc_extra_info("");
    if (!get_libc_version(libc_version, libc_extra_info)) {
        msg_printf(NULL, MSG_INFO,
            "libc: %s version %s",
            libc_extra_info.c_str(), libc_version.c_str()
        );
        // add info to os_version_extra
        //
        if (!os_version_extra.empty()) {
            os_version_extra += "|";
        }
        os_version_extra += "libc " + libc_version;
        if (!libc_extra_info.empty()) {
            os_version_extra += " (" + libc_extra_info + ")";
        }
    }

    if (!os_version_extra.empty()) {
        safe_strcat(os_version, " [");
        safe_strcat(os_version, os_version_extra.c_str());
        safe_strcat(os_version, "]");
    }
#endif //LINUX_LIKE_SYSTEM
    return 0;
}

// This is called at startup with init=true
// and before scheduler RPCs, with init=false.
// In the latter case only get items that could change,
// like disk usage and network info
//
int HOST_INFO::get_host_info(bool init) {
    int retval = get_filesystem_info(d_total, d_free);
    if (retval) {
        msg_printf(0, MSG_INTERNAL_ERROR,
            "get_filesystem_info() failed: %s", boincerror(retval)
        );
    }
    get_local_network_info();

    if (!init) return 0;

    // everything after here is assumed not to change during
    // a run of the client
    //

#ifndef ANDROID
    if (!cc_config.dont_use_vbox) {
        get_virtualbox_version();
    }

    get_docker_version();
    get_docker_compose_version();
#endif

    get_cpu_info();
    get_cpu_count();
    get_memory_info();
    timezone = get_timezone();
    get_os_info();
    collapse_whitespace(p_model);
    collapse_whitespace(p_vendor);
    if (!strlen(host_cpid)) {
        generate_host_cpid();
    }
    return 0;
}

inline long device_idle_time(const char *device) {
    struct stat sbuf;
    if (stat(device, &sbuf)) {
        return USER_IDLE_TIME_INF;
    }
    return gstate.now - sbuf.st_atime;
}

// list of directories and prefixes of TTY devices
//
static const struct dir_tty_dev {
    const char *dir;
    const char *dev;
    const vector<string> ignore_list;

    bool should_ignore(const string &devname) const {
        for (const string &ignore : ignore_list) {
            if (devname.rfind(ignore, 0) == 0) return true;
        }
        return false;
    }
} tty_patterns[] = {
#if defined(LINUX_LIKE_SYSTEM) and !defined(ANDROID)
    { "/dev", "tty",
      {"ttyS", "ttyACM"},
    },
    { "/dev", "pty", {}},
    { "/dev/pts", NULL, {}},
#endif
};
#define N_TTY_PATTERNS sizeof(tty_patterns)/sizeof(dir_tty_dev)

// Make a list of all TTY devices on the system.
//
vector<string> get_tty_list() {
    char devname[1024];
    char fullname[1024];
    vector<string> tty_list;

    for (unsigned int i=0; i<N_TTY_PATTERNS; i++) {
        DIRREF dev = dir_open(tty_patterns[i].dir);
        if (!dev) continue;
        while (1) {
            if (dir_scan(devname, dev, 1024)) break;
            if (devname[0] == '.') continue;

            // check name prefix
            //
            if (tty_patterns[i].dev) {
                if ((strstr(devname, tty_patterns[i].dev) != devname)) continue;

                // Ignore some devices. This could be, for example,
                // ttyS* (serial port) or devACM* (serial USB) devices
                // which may be used even without a user being active.
                //
                if (tty_patterns[i].should_ignore(devname)) continue;
            }

            snprintf(fullname, sizeof(fullname), "%s/%s", tty_patterns[i].dir, devname);

            // check for ignored paths
            //
            bool ignore = false;
            for (unsigned int j=0; j<cc_config.ignore_tty.size(); j++) {
                if (strstr(fullname, cc_config.ignore_tty[j].c_str()) == fullname) {
                    ignore = true;
                    break;
                }
            }
            if (ignore) continue;
            tty_list.push_back(fullname);
        }
        dir_close(dev);
    }
    return tty_list;
}

inline long all_tty_idle_time() {
    static vector<string> tty_list;
    static bool first = true;
    struct stat sbuf;
    long idle_time = USER_IDLE_TIME_INF;

    if (first) {
        first = false;
        tty_list = get_tty_list();
    }
    for (unsigned int i=0; i<tty_list.size(); i++) {
        // ignore errors
        if (!stat(tty_list[i].c_str(), &sbuf)) {
            // printf("tty: %s %d %d\n",tty_list[i].c_str(), sbuf.st_atime, t);
            idle_time = min(idle_time, (long)(gstate.now-sbuf.st_atime));
        }
    }
    return idle_time;
}

#ifdef __APPLE__

// We can't link the client with the AppKit framework because the client
// must be setuid boinc_master. So the client uses this to get the system
// up time instead of our getTimeSinceBoot() function in lib/mac_util.mm.
//
int get_system_uptime() {
    struct timeval tv;
    size_t len = sizeof(tv);
    gettimeofday(&tv, 0);
    time_t now = tv.tv_sec;
    sysctlbyname("kern.boottime", &tv, &len, NULL, 0);
    return ((int)now - (int)tv.tv_sec);
}

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
// IOHIDGetParameter() is deprecated in OS 10.12, but IORegistryEntryFromPath()
// and IORegistryEntryCreateCFProperty() do the same thing and have been
// available since OS 10.0.
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
//TODO: I believe that the IOHIDSystemEntry returned by IORegistryEntryFromPath()
// will remain valid as long as the client application runs if we don't call
// IOObjectRelease() on it, so we could probably make this more efficient by
// calling IORegistryEntryFromPath() only once. But I have not been able to
// confirm that, so I call it each time through this function out of caution.
// Even with calling IORegistryEntryFromPath() each time, this code is much
// faster than the previous method, which called IOHIDGetParameter().
//
long HOST_INFO::user_idle_time(bool /*check_all_logins*/) {
    static bool     error_posted = false;
    int64_t         idleNanoSeconds;
    double          idleTime = 0;
    double          idleTimeFromCG = 0;

    CFTypeRef idleTimeProperty;
    io_registry_entry_t IOHIDSystemEntry;

    if (error_posted) return USER_IDLE_TIME_INF;

    IOHIDSystemEntry = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/IOResources/IOHIDSystem");
    if (IOHIDSystemEntry != MACH_PORT_NULL) {
        idleTimeProperty = IORegistryEntryCreateCFProperty(IOHIDSystemEntry, CFSTR(EVSIOIDLE), kCFAllocatorDefault, kNilOptions);
        CFNumberGetValue((CFNumberRef)idleTimeProperty, kCFNumberSInt64Type, &idleNanoSeconds);
        idleTime = ((double)idleNanoSeconds) / 1000.0 / 1000.0 / 1000.0;
        IOObjectRelease(IOHIDSystemEntry);  // Prevent a memory leak (see comment above)
        CFRelease(idleTimeProperty);
    } else {
        // When the system first starts up, allow time for HIDSystem to be available if needed
        if (get_system_uptime() > (120)) {   // If system has been up for more than 2 minutes
            msg_printf(NULL, MSG_INFO,
                "Could not connect to HIDSystem: user idle detection is disabled."
            );
            error_posted = true;
            return USER_IDLE_TIME_INF;
        }
    }

    if (!gstate.executing_as_daemon) {
        idleTimeFromCG =  CGEventSourceSecondsSinceLastEventType (
            kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType
        );

        if (idleTimeFromCG < idleTime) {
            idleTime = idleTimeFromCG;
        }
    }

    return (long)idleTime;
}

#else  // ! __APPLE__

#if HAVE_UTMP_H
inline long user_idle_time(struct utmp* u) {
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
    return device_idle_time(tty);
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
          if ((ufp = fopen(UTMP_LOCATION, "r")) == NULL)
#elif defined(UTMP_FILE)
          if ((ufp = fopen(UTMP_FILE, "r")) == NULL)
#elif defined(_PATH_UTMP)
          if ((ufp = fopen(_PATH_UTMP, "r")) == NULL)
#else
          if ((ufp = fopen("/etc/utmp", "r")) == NULL)
#endif
          { // Please keep all braces balanced in source files; repeated
            // open braces in conditional compiles confuse Xcode's editor.
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
  inline long all_logins_idle() {
      struct utmp* u;
      setutent();
      long idle_time = USER_IDLE_TIME_INF;

      while ((u = getutent()) != NULL) {
          idle_time = min(idle_time, user_idle_time(u));
      }
      return idle_time;
  }
#endif  // HAVE_UTMP_H

#if LINUX_LIKE_SYSTEM

#if HAVE_XSS

// return vector of X server names
//
const vector<string> X_display_values_initialize() {
    // According to "man Xserver", each local Xserver will have a socket file
    // at /tmp/.X11-unix/Xn, where "n" is the display number (0, 1, 2, etc).
    // We will parse this directory for currently open Xservers and attempt
    // to ultimately query them for their idle time.
    //
    // If we are unable to open _any_ Xserver,
    // idle detection is up to other methods.
    //
    static const string dir = "/tmp/.X11-unix/";
    vector<string> display_values;
    vector<string>::iterator it;

    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        if (log_flags.idle_detection_debug ) {
            msg_printf(NULL, MSG_INFO,
                "[idle_detection] Error (%d) opening %s.", errno, dir.c_str()
            );
        }
    } else {
        if (log_flags.idle_detection_debug ) {
            msg_printf(NULL, MSG_INFO,
                "[idle_detection] scanning %s",  dir.c_str()
            );
        }
        while ((dirp = readdir(dp)) != NULL) {
            if (!strcmp(dirp->d_name, ".")) continue;
            if (!strcmp(dirp->d_name, "..")) continue;
            if (log_flags.idle_detection_debug ) {
                msg_printf(NULL, MSG_INFO,
                    "[idle_detection] found X server %s", dirp->d_name
                );
            }
            display_values.push_back(string(dirp->d_name));
        }
        closedir(dp);
    }

    // Get rid of non-matching elements and format the matching ones.
    //
    for (it = display_values.begin(); it != display_values.end(); ) {
        if (it->c_str()[0] != 'X') {
            it = display_values.erase(it);
        } else {
            replace(it->begin(), it->end(), 'X', ':');
            it++;
        }
    }

    return display_values;
}

// Ask X servers for user idle time (using XScreenSaver API)
// Return min of idle times.
// This function assumes that the boinc user has been
// granted access to the Xservers a la "xhost +SI:localuser:boinc".
// If access isn't available for an Xserver, that Xserver is skipped.
// One may drop a file in /etc/X11/Xsession.d/ that runs the xhost command
// for all Xservers on a machine when the Xservers start up.
//
// TODO: call X_display_values_initialize() once, not once per second
//
long xss_idle() {
    long idle_time = USER_IDLE_TIME_INF;
    const vector<string> display_values = X_display_values_initialize();
    vector<string>::const_iterator it;

    // If we can connect to at least one DISPLAY, this is set to false.
    //
    bool no_available_x_display = true;

    static XScreenSaverInfo* xssInfo = XScreenSaverAllocInfo();
    // This shouldn't fail. XScreenSaverAllocInfo just returns a small
    // struct (see "man 3 xss"). If we can't allocate this, then we've
    // got bigger problems to worry about.
    //
    if (xssInfo == NULL) {
        if (log_flags.idle_detection_debug) {
            msg_printf(NULL, MSG_INFO,
                "[idle_detection] XScreenSaverAllocInfo failed. Out of memory? Skipping XScreenSaver idle detection."
            );
        }
        return true;
    }

    for (it = display_values.begin(); it != display_values.end() ; it++) {

        Display* disp = NULL;
        long display_idle_time = 0;

        disp = XOpenDisplay(it->c_str());
        // XOpenDisplay may return NULL if there is no running X
        // or DISPLAY points to wrong/invalid display
        //
        if (disp == NULL) {
            if (log_flags.idle_detection_debug) {
	            msg_printf(NULL, MSG_INFO,
	                "[idle_detection] DISPLAY '%s' not found or insufficient access.",
	                it->c_str()
                );
            }
            continue;
        }

        // Determine if the DISPLAY we have accessed has the XScreenSaver
        // extension or not.
        //
        int event_base_return, error_base_return;
        if (!XScreenSaverQueryExtension(
            disp, &event_base_return, &error_base_return
        )){
            if (log_flags.idle_detection_debug) {
	            msg_printf(NULL, MSG_INFO,
	                "[idle_detection] XScreenSaver extension not available for DISPLAY '%s'.",
	                it->c_str()
                );
            }
            XCloseDisplay(disp);
            continue;
        }

        // All checks passed. Get the idle information.
        //
        no_available_x_display = false;
        XScreenSaverQueryInfo(disp, DefaultRootWindow(disp), xssInfo);
        display_idle_time = xssInfo->idle;

        // Close the connection to the XServer
        //
        XCloseDisplay(disp);

        // convert from milliseconds to seconds
        //
        display_idle_time /= 1000;

        if (log_flags.idle_detection_debug) {
            msg_printf(NULL, MSG_INFO,
                "[idle_detection] XSS idle time on display '%s': %ld",
                it->c_str(), display_idle_time
            );
        }

        idle_time = min(idle_time, display_idle_time);
    }

    // If none of the Xservers were queryable, report it
    //
    if (log_flags.idle_detection_debug && no_available_x_display) {
        msg_printf(NULL, MSG_INFO,
            "[idle_detection] Could not connect to any DISPLAYs. XSS idle determination impossible."
        );
    }
    return idle_time;

}
#endif // HAVE_XSS

#endif // LINUX_LIKE_SYSTEM

long HOST_INFO::user_idle_time(bool check_all_logins) {
    long idle_time = USER_IDLE_TIME_INF;

#if HAVE_UTMP_H
    if (check_all_logins) {
        idle_time = min(idle_time, all_logins_idle());
    }
#endif

    idle_time = min(idle_time, all_tty_idle_time());

#if LINUX_LIKE_SYSTEM

#if HAVE_XSS
    idle_time = min(idle_time, xss_idle());
#endif // HAVE_XSS

#else
    // We should find out which of the following are actually relevant
    // on which systems (if any)
    //
    idle_time = min(idle_time, (long)device_idle_time("/dev/mouse"));
        // solaris, linux
    idle_time = min(idle_time, (long)device_idle_time("/dev/input/mice"));
    idle_time = min(idle_time, (long)device_idle_time("/dev/kbd"));
        // solaris
#endif // LINUX_LIKE_SYSTEM
    return idle_time;
}

#endif  // ! __APPLE__

#ifdef __APPLE__

union headeru {
    fat_header fat;
    mach_header mach;
};

// Get the architecture of this computer's CPU: x86_64 or arm64.
// Read the executable file's mach-o headers to determine the
// architecture(s) of its code.
// Returns 1 if application can run natively on this computer,
// else returns 0.
//
// TODO: determine whether x86_64 graphics apps emulated on arm64 Macs
// properly run under Rosetta 2. Note: years ago, PowerPC apps emulated
// by Rosetta on i386 Macs crashed when running graphics.
//
bool can_run_on_this_CPU(char* exec_path) {
    FILE *f;
    int retval = false;

    headeru myHeader;
    fat_arch fatHeader;

    static bool x86_64_CPU = false;
    static bool arm64_cpu = false;
    static bool need_CPU_architecture = true;
    uint32_t n, i, len;
    uint32_t theMagic;
    integer_t file_architecture;

    if (need_CPU_architecture) {
        // Determine the architecture of the CPU we are running on
        // TODO: adjust this code accordingly.
        uint32_t cputype = 0;
        size_t size = sizeof (cputype);
        int res = sysctlbyname ("hw.cputype", &cputype, &size, NULL, 0);
        if (res) return false;  // Should never happen
        // Since we require MacOS >= 10.7, the CPU must be x86_64 or arm64
        x86_64_CPU = ((cputype &0xff) == CPU_TYPE_X86);
        arm64_cpu = ((cputype &0xff) == CPU_TYPE_ARM);

        need_CPU_architecture = false;
    }

    f = boinc_fopen(exec_path, "rb");
    if (!f) {
        return retval;          // Should never happen
    }

    myHeader.fat.magic = 0;
    myHeader.fat.nfat_arch = 0;

    fread(&myHeader, 1, sizeof(fat_header), f);
    theMagic = myHeader.mach.magic;
    switch (theMagic) {
    case MH_CIGAM:
    case MH_MAGIC:
    case MH_MAGIC_64:
    case MH_CIGAM_64:
       file_architecture = myHeader.mach.cputype;
        if ((theMagic == MH_CIGAM) || (theMagic == MH_CIGAM_64)) {
            file_architecture = OSSwapInt32(file_architecture);
        }
        if (x86_64_CPU && (file_architecture == CPU_TYPE_I386)) {
            // Single-architecture i386 file on x86_64 CPU
            if (compareOSVersionTo(10, 15) < 0) {
                // OS >= 10.15 are 64-bit only
                retval = true;
            }
        } else if (x86_64_CPU && (file_architecture == CPU_TYPE_X86_64)) {
            retval = true; // Single-architecture x86_64 file on x86_64 CPU
        } else if (arm64_cpu && (file_architecture == CPU_TYPE_ARM64)) {
            retval = true; // Single-architecture arm64 file on arm64 CPU
        } else if (arm64_cpu && (file_architecture == CPU_TYPE_X86_64)) {
            retval = true; // Single-architecture x86_64 file emulated on arm64 CPU
            // TODO: determine whether emulated graphics apps work properly
        }
        break;
    case FAT_MAGIC:
    case FAT_CIGAM:
        n = myHeader.fat.nfat_arch;
        if (theMagic == FAT_CIGAM) {
            n = OSSwapInt32(myHeader.fat.nfat_arch);
        }

        // Multiple architecture (fat) file
        //
        for (i=0; i<n; i++) {
            len = fread(&fatHeader, 1, sizeof(fat_arch), f);
            if (len < sizeof(fat_arch)) {
                break;          // Should never happen
            }
            file_architecture = fatHeader.cputype;
            if (theMagic == FAT_CIGAM) {
                file_architecture = OSSwapInt32(file_architecture);
            }

            if (x86_64_CPU && (file_architecture == CPU_TYPE_X86_64)) {
                retval = true; // file with x86_64 architecture on x86_64 CPU
                break;
            } else if (arm64_cpu && (file_architecture == CPU_TYPE_ARM64)) {
                retval = true; // file with arm64 architecture on arm64 CPU
                break;
            } else if (arm64_cpu && (file_architecture == CPU_TYPE_X86_64)) {
                retval = true; // file with x86_64 architecture emulated on arm64 CPU
                // TODO: determine whether emulated graphics apps work properly
            }
        }
        break;
    default:
        break;
    }

    fclose (f);
    return retval;
}
#endif
