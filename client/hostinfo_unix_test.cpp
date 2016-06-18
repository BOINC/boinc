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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// normally configure would find this out
// only activate one of the three to test with
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_SYS_SYSCTL_H 0
#define HAVE_SYS_SYSTEMINFO_H 0

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#if HAVE_SYS_SYSTEMINFO_H
#include <sys/systeminfo.h>
#endif
#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#define false 0
#define true 1
#define bool int
#define strlcpy strncpy
#define safe_strcpy(x, y) strlcpy(x, y, sizeof(x))

int main(void) {
    char buf[256], features[1024], model_buf[1024];
    bool vendor_found=false, model_found=false;
    bool cache_found=false, features_found=false;
    bool icache_found=false,dcache_found=false;
    bool model_hack=false, vendor_hack=false;
    int n;
    int family=-1, model=-1, stepping=-1;
    char  p_vendor[256], p_model[256], product_name[256];
    char  os_name[256], os_version[256];
    char buf2[256];
    int m_cache=-1;


    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return (EXIT_FAILURE);

    strcpy(p_model, "");

#ifdef __mips__
    strcpy(p_model, "MIPS ");
    model_hack = true;
#elif __alpha__
    strcpy(p_vendor, "HP (DEC) ");
    vendor_hack = true;
#elif __hppa__
    strcpy(p_vendor, "HP ");
    vendor_hack = true;
#elif __ia64__
    strcpy(p_model, "IA-64 ");
    model_hack = true;
#elif __arm__
    strcpy(p_vendor, "ARM ");
    vendor_hack = vendor_found = true;
#elif __aarch64__
    strcpy(p_vendor, "ARM ");
    vendor_hack = vendor_found = true;
    model_hack = true;
#endif

    strcpy(features, "");
    strcpy(product_name, "");
    while (fgets(buf, 256, f)) {
        //strip_whitespace(buf);
        if (
                /* there might be conflicts if we dont #ifdef */
#ifdef __ia64__
            strstr(buf, "vendor     : ")
#elif __hppa__        
            strstr(buf, "cpu\t\t: ")
#elif __powerpc__
            strstr(buf, "machine\t\t: ")
#elif __sparc__
            strstr(buf, "type\t\t: ")
#elif __alpha__
            strstr(buf, "cpu\t\t\t: ")
#elif __arm__
            strstr(buf, "CPU architecture: ")
#else
            strstr(buf, "vendor_id\t: ") || strstr(buf, "system type\t\t: ")
#endif
        ) {
            if (!vendor_hack && !vendor_found) {
                vendor_found = true;
                strlcpy(p_vendor, strchr(buf, ':') + 2, sizeof(p_vendor));
            } else if (!vendor_found) {
                vendor_found = true;
                strlcpy(buf2, strchr(buf, ':') + 2, sizeof(p_vendor) - strlen(p_vendor) - 1);
                strcat(p_vendor, buf2);
            }
        }

#ifdef __aarch64__
        if (
            // Hardware is specifying the board this CPU is on, store it in product_name while we parse /proc/cpuinfo
            strstr(buf, "Hardware\t: ")
        ) {
            strlcpy(buf2, strchr(buf, ':') + 2, sizeof(product_name) - strlen(product_name) - 1);
            //strip_whitespace(buf2);
            if (strlen(product_name)) {
                strcat(product_name, " ");
            }
            strcat(product_name, buf2);
        }
#endif
        if (
#ifdef __ia64__
            strstr(buf, "family     : ") || strstr(buf, "model name : ")
#elif __powerpc__ || __sparc__
            strstr(buf, "cpu\t\t: ")
#elif __arm__
            strstr(buf, "Processor\t: ") || strstr(buf, "model name")
#elif __aarch64__
            // Hardware is a fallback specifying the board this CPU is on (not ideal but better than nothing)
            strstr(buf, "Processor\t: ") || strstr(buf, "CPU architecture: ") || strstr(buf, "Hardware\t: ")
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
                strlcpy(p_model, strchr(buf, ':') + 2, sizeof(p_model));
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
#ifdef __aarch64__
                /* depending on kernel version, CPU architecture can either be
                 * a number or a string. If a string, we have a model name, else we don't
                 */
                char *testc = NULL;
                testc = strrchr(buf, ':')+2;
                if (isdigit(*testc)) {
                    continue;    /* skip this line */
                }
#endif
                model_found = true;
                strlcpy(buf2, strchr(buf, ':') + 2, sizeof(p_model) - strlen(p_model) - 1);
                //strip_whitespace(buf2);
                strcat(p_model, buf2);
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
            m_cache += n*1024;
        }
        if (!dcache_found && strstr(buf, "D-cache\t\t: ")) {
            dcache_found = true;
            sscanf(buf, "D-cache\t\t: %d", &n);
            m_cache += n*1024;
        }
#elif __powerpc__
        if (!cache_found && strstr(buf, "L2 cache\t: ")) {
            cache_found = true;
            sscanf(buf, "L2 cache\t: %d", &n);
            m_cache = n*1024;
        }
#else
        if (!cache_found && (strstr(buf, "cache size\t: ") == buf)) {
            cache_found = true;
            sscanf(buf, "cache size\t: %d", &n);
            m_cache = n*1024;
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
            } else if ((strstr(buf, "features   : ") == buf)) { /* ia64 */
                strlcpy(features, strchr(buf, ':') + 2, sizeof(features));
            } else if ((strstr(buf, "Features\t: ") == buf)) { /* arm */
                strlcpy(features, strchr(buf, ':') + 2, sizeof(features));
            }
            if (strlen(features)) {
                features_found = true;
            }
        }
    }
    strlcpy(model_buf, p_model, sizeof(model_buf));
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


    printf("p_vendor: %s\nm_cache: %d\nmodel_buf: %s\n",
        p_vendor, m_cache, model_buf
    );
    fclose(f);

    // detect OS name as in HOST_INFO::get_os_info()
    strcpy(os_name, "");
    strcpy(os_version, "");

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

    printf("os_name: %s\nos_version: %s\nproduct_name: %s\n",
        os_name, os_version, product_name
    );

}
