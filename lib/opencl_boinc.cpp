// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "boinc_stdio.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#endif

#ifdef _WIN32
#include "win_util.h"
#endif
#include "miofile.h"
#include "parse.h"
#include "str_replace.h"
#include "util.h"

#include "opencl_boinc.h"

#ifndef _USING_FCGI_
void OPENCL_DEVICE_PROP::write_xml(MIOFILE& f, const char* tag, bool temp_file) {
    f.printf(
        "   <%s>\n"
        "      <name>%s</name>\n"
        "      <vendor>%s</vendor>\n"
        "      <vendor_id>%lu</vendor_id>\n"
        "      <available>%d</available>\n"
        "      <half_fp_config>%llu</half_fp_config>\n"
        "      <single_fp_config>%llu</single_fp_config>\n"
        "      <double_fp_config>%llu</double_fp_config>\n"
        "      <endian_little>%d</endian_little>\n"
        "      <execution_capabilities>%llu</execution_capabilities>\n"
        "      <extensions>%s</extensions>\n"
        "      <global_mem_size>%llu</global_mem_size>\n"
        "      <local_mem_size>%llu</local_mem_size>\n"
        "      <max_clock_frequency>%lu</max_clock_frequency>\n"
        "      <max_compute_units>%lu</max_compute_units>\n"
        "      <nv_compute_capability_major>%lu</nv_compute_capability_major>\n"
        "      <nv_compute_capability_minor>%lu</nv_compute_capability_minor>\n"
        "      <amd_simd_per_compute_unit>%lu</amd_simd_per_compute_unit>\n"
        "      <amd_simd_width>%lu</amd_simd_width>\n"
        "      <amd_simd_instruction_width>%lu</amd_simd_instruction_width>\n"
        "      <opencl_platform_version>%s</opencl_platform_version>\n"
        "      <opencl_device_version>%s</opencl_device_version>\n"
        "      <opencl_driver_version>%s</opencl_driver_version>\n",
        tag,
        name,
        vendor,
        (unsigned long)vendor_id,
        available ? 1 : 0,
        half_fp_config,
        single_fp_config,
        double_fp_config,
        endian_little ? 1 : 0,
        execution_capabilities,
        extensions,
        global_mem_size,
        local_mem_size,
        (unsigned long)max_clock_frequency,
        (unsigned long)max_compute_units,
        (unsigned long)nv_compute_capability_major,
        (unsigned long)nv_compute_capability_minor,
        (unsigned long)amd_simd_per_compute_unit,
        (unsigned long)amd_simd_width,
        (unsigned long)amd_simd_instruction_width,
        opencl_platform_version,
        opencl_device_version,
        opencl_driver_version
    );
    if (temp_file) {
        f.printf(
            "      <device_num>%d</device_num>\n"
            "      <peak_flops>%f</peak_flops>\n"
            "      <opencl_available_ram>%f</opencl_available_ram>\n"
            "      <opencl_device_index>%d</opencl_device_index>\n"
            "      <warn_bad_cuda>%d</warn_bad_cuda>\n",
            device_num,
            peak_flops,
            opencl_available_ram,
            opencl_device_index,
            warn_bad_cuda
        );
    }
    f.printf("   </%s>\n", tag);
}
#endif

int OPENCL_DEVICE_PROP::parse(XML_PARSER& xp, const char* end_tag) {
    int n;
    unsigned long long ull;

    while (!xp.get_tag()) {
        if (xp.match_tag(end_tag)) {
            get_device_version_int();
            get_opencl_driver_revision();
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("vendor", vendor, sizeof(vendor))) continue;
        if (xp.parse_ulonglong("vendor_id", ull)) {
            vendor_id = (unsigned)ull;
            continue;
        }
        if (xp.parse_int("available", n)) {
            available = n;
            continue;
        }
        if (xp.parse_ulonglong("half_fp_config", ull)) {
                half_fp_config = ull;
                continue;
        }
        if (xp.parse_ulonglong("single_fp_config", ull)) {
            single_fp_config = ull;
            continue;
        }
        if (xp.parse_ulonglong("double_fp_config", ull)) {
            double_fp_config = ull;
            continue;
        }
        if (xp.parse_int("endian_little", n)) {
            endian_little = n;
            continue;
        }
        if (xp.parse_ulonglong("execution_capabilities", ull)) {
            execution_capabilities = ull;
            continue;
        }
        if (xp.parse_str("extensions",
            extensions,
            sizeof(extensions)
        )) {
            continue;
        }
        if (xp.parse_ulonglong("global_mem_size", ull)) {
            global_mem_size = ull;
            continue;
        }
        if (xp.parse_ulonglong("local_mem_size", ull)) {
            local_mem_size = ull;
            continue;
        }
        if (xp.parse_int("max_clock_frequency", n)) {
            max_clock_frequency = n;
            continue;
        }
        if (xp.parse_int("max_compute_units", n)) {
            max_compute_units = n;
            continue;
        }
        if (xp.parse_int("nv_compute_capability_major", n)) {
            nv_compute_capability_major = n;
            continue;
        }
        if (xp.parse_int("nv_compute_capability_minor", n)) {
            nv_compute_capability_minor = n;
            continue;
        }
        if (xp.parse_int("amd_simd_per_compute_unit", n)) {
            amd_simd_per_compute_unit = n;
            continue;
        }
        if (xp.parse_int("amd_simd_width", n)) {
            amd_simd_width = n;
            continue;
        }
        if (xp.parse_int("amd_simd_instruction_width", n)) {
            amd_simd_instruction_width = n;
            continue;
        }
        if (xp.parse_str("opencl_platform_version",
            opencl_platform_version,
            sizeof(opencl_platform_version)
        )) {
            continue;
        }
        if (xp.parse_str("opencl_device_version",
            opencl_device_version,
            sizeof(opencl_device_version)
        )) {
            continue;
        }
        if (xp.parse_str("opencl_driver_version",
            opencl_driver_version,
            sizeof(opencl_driver_version)
        )) {
            continue;
        }

        // The following are used only in the
        // COPROC_INFO_FILENAME temporary file
        if (xp.parse_int("device_num", n)) {
            device_num = n;
            continue;
        }
        if (xp.parse_double("peak_flops", peak_flops)) continue;
        if (xp.parse_double("opencl_available_ram", opencl_available_ram)) continue;
        if (xp.parse_int("opencl_device_index", n)) {
            opencl_device_index = n;
            continue;
        }
        if (xp.parse_bool("warn_bad_cuda", warn_bad_cuda)) continue;
    }
    return ERR_XML_PARSE;
}

int OPENCL_DEVICE_PROP::get_device_version_int() {
    int maj, min;
    int n = sscanf(
        opencl_device_version, "OpenCL %d.%d", &maj, &min
    );
    if (n != 2) {
        return ERR_NOT_FOUND;
    }
    opencl_device_version_int = 100*maj + min;
    return 0;
}

int OPENCL_DEVICE_PROP::get_opencl_driver_revision() {
    // gets the OpenCL runtime revision
    // Thus far this is only necessary for ATI/AMD because there are bad
    // driver sets only distinguisable by the runtime library version.
    // Fortunately this info is in the opencl_device_version string.
    float rev=0;
    char *p=opencl_device_version+sizeof(opencl_device_version)-1;
    // find the last opening bracket
    while ((p > opencl_device_version) && (*p!='(')) p--;
    if (p!=opencl_device_version) {
      int n=sscanf(
          p, "(%f", &rev
      );
      // I don't care about errors because for non-ATI GPUs this should
      // be zero.
      if (n!=1) {
        rev=0;
      }
    }
    opencl_driver_revision = (int)floor(rev*100+0.5);
    return 0;
}

void OPENCL_DEVICE_PROP::description(char* buf, int buflen, const char* type) {
    char s1[256], s2[1024];
    int n;
    // openCL_device_version may have a trailing space
    strlcpy(s1, opencl_device_version, sizeof(s1));
    n = (int)strlen(s1) - 1;
    if ((n > 0) && (s1[n] == ' ')) s1[n] = '\0';
    snprintf(s2, sizeof(s2),
        "%.64s (driver version %.64s, device version %.64s, %.2fGB, %.2fGB available, %.0f GFLOPS peak)",
        name, opencl_driver_version,
        s1, (double)global_mem_size/GIGA,
        opencl_available_ram/GIGA, peak_flops/1.e9
    );

    switch(is_used) {
    case COPROC_IGNORED:
        snprintf(buf, buflen,
            "OpenCL: %s %d (ignored by config): %s",
            type, device_num, s2
        );
        break;
    case COPROC_USED:
        snprintf(buf, buflen,
            "OpenCL: %s %d: %s",
            type, device_num, s2
        );
        break;
    case COPROC_UNUSED:
    default:
        snprintf(buf, buflen,
            "OpenCL: %s %d (not used): %s",
            type, device_num, s2
        );
        break;
    }
}

////////////////// OPENCL CPU STARTS HERE /////////////////


// CPU OpenCL does not really describe a coprocessor but
// this is here to take advantage of the other OpenCL code.
void OPENCL_CPU_PROP::clear() {
    platform_vendor[0] = 0;
    opencl_prop.clear();
}

void OPENCL_CPU_PROP::write_xml(MIOFILE& f) {
#ifndef _USING_FCGI_
    f.printf(
        "<opencl_cpu_prop>\n"
        "   <platform_vendor>%s</platform_vendor>\n",
        platform_vendor
    );
    opencl_prop.write_xml(f, "opencl_cpu_info");
    f.printf("</opencl_cpu_prop>\n");
#endif
}

int OPENCL_CPU_PROP::parse(XML_PARSER& xp) {
    int retval;

    clear();

    while (!xp.get_tag()) {
        if (xp.match_tag("/opencl_cpu_prop")) {
            if (!strlen(platform_vendor)) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_str("platform_vendor", platform_vendor, sizeof(platform_vendor))) continue;
        if (xp.match_tag("opencl_cpu_info")) {
            retval = opencl_prop.parse(xp, "/opencl_cpu_info");
            if (retval) return retval;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void OPENCL_CPU_PROP::description(char* buf, int buflen) {
    char s1[256];
    int n;
    // openCL_device_version may have a trailing space
    strlcpy(s1, opencl_prop.opencl_device_version, sizeof(s1));
    n = (int)strlen(s1) - 1;
    if ((n > 0) && (s1[n] == ' ')) s1[n] = '\0';
    snprintf(buf, buflen,
        "OpenCL CPU: %s (OpenCL driver vendor: %s, driver version %s, device version %s)",
        opencl_prop.name, platform_vendor, opencl_prop.opencl_driver_version, s1
    );
}
