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

#define false 0
#define true 1
#define bool int
#define strlcpy strncpy

int main(void) {
    char buf[256], features[1024], model_buf[1024];
    bool vendor_found=false, model_found=false;
    bool cache_found=false, features_found=false;
    bool icache_found=false,dcache_found=false;
    bool model_hack=false, vendor_hack=false;
    int n;
    int family=-1, model=-1, stepping=-1;
    char  p_vendor[256], p_model[256];
    char buf2[256];
    int m_cache=-1;


    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) return (EXIT_FAILURE);

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
#endif

    strcpy(features, "");
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

        if (
#ifdef __ia64__
            strstr(buf, "family     : ") || strstr(buf, "model name : ")
#elif __powerpc__ || __sparc__
            strstr(buf, "cpu\t\t: ")
#elif __arm__
            strstr(buf, "Processor\t: ")
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
        model_found = true;
        strlcpy(buf2, strchr(buf, ':') + 2, sizeof(p_model) - strlen(p_model) - 1);
        strcat(p_model, buf2);
        }        
        }

#ifndef __hppa__
    /* XXX: hppa: "cpu family    : PA-RISC 2.0" */
        if (strstr(buf, "cpu family\t: ") && family<0) {
        family = atoi(buf+strlen("cpu family\t: "));
        }
        /* XXX: hppa: "model            : 9000/785/J6000" */
        if (strstr(buf, "model\t\t: ") && model<0) {
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
    strcpy(model_buf, p_model);
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
}
