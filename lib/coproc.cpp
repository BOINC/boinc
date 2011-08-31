// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#include <cstring>
#include <cstdlib>
#endif


#ifdef _WIN32
#include "win_util.h"
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#endif
#include "config.h"
#include <dlfcn.h>
#include <setjmp.h>
#include <signal.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"
#include "str_replace.h"

#include "coproc.h"

#ifndef _USING_FCGI_
using std::perror;
#endif

int COPROC_REQ::parse(XML_PARSER& xp) {
    char buf[1024];
    strcpy(type, "");
    count = 0;
    MIOFILE& in = *(xp.f);
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc>")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<type>", type, sizeof(type))) continue;
        if (parse_double(buf, "<count>", count)) continue;
    }
    return ERR_XML_PARSE;
}

#ifndef _USING_FCGI_

void COPROC::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc>\n"
        "   <type>%s</type>\n"
        "   <count>%d</count>\n"
        "</coproc>\n",
        type, count
    );
}

void COPROC::write_request(MIOFILE& f) {
    f.printf(
        "   <req_secs>%f</req_secs>\n"
        "   <req_instances>%f</req_instances>\n"
        "   <estimated_delay>%f</estimated_delay>\n",
        req_secs,
        req_instances,
        estimated_delay
    );
}

void COPROC::opencl_write_xml(MIOFILE& f) {
    f.printf(
        "   <coproc_opencl>\n"
        "      <name>%s</name>\n"
        "      <vendor>%s</vendor>\n"
        "      <vendor_id>%lu</vendor_id>\n"
        "      <available>%d</available>\n"
        "      <hp_fp_config>%llu</hp_fp_config>\n"
        "      <sp_fp_config>%llu</sp_fp_config>\n"
        "      <dp_fp_config>%llu</dp_fp_config>\n"
        "      <little_endian>%d</little_endian>\n"
        "      <exec_capabilities>%llu</exec_capabilities>\n"
        "      <extensions>%s</extensions>\n"
        "      <global_RAM>%llu</global_RAM>\n"
        "      <local_RAM>%llu</local_RAM>\n"
        "      <max_clock_freq>%lu</max_clock_freq>\n"
        "      <max_cores>%lu</max_cores>\n"
        "      <openCL_platform_version>%s</openCL_platform_version>\n"
        "      <openCL_device_version>%s</openCL_device_version>\n"
        "      <openCL_driver_version>%s</openCL_driver_version>\n"
        "   </coproc_opencl>\n",
        opencl_prop.name,
        opencl_prop.vendor,
        opencl_prop.vendor_id,
        opencl_prop.available ? 1 : 0,
        opencl_prop.hp_fp_config,
        opencl_prop.sp_fp_config,
        opencl_prop.dp_fp_config,
        opencl_prop.little_endian ? 1 : 0,
        opencl_prop.exec_capab,
        opencl_prop.extensions,
        opencl_prop.global_RAM,
        opencl_prop.local_RAM,
        opencl_prop.max_clock_freq,
        opencl_prop.max_cores,
        opencl_prop.openCL_platform_version,
        opencl_prop.openCL_device_version,
        opencl_prop.openCL_driver_version
    );
}

int COPROC::parse(XML_PARSER& xp) {
    char buf[256];
    strcpy(type, "");
    clear();
    for (int i=0; i<MAX_COPROC_INSTANCES; i++) {
        device_nums[i] = i;
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/coproc")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_str("type", type, sizeof(type))) continue;
        if (xp.parse_int("count", count)) continue;
        if (xp.parse_double("peak_flops", peak_flops)) continue;
        if (xp.parse_str("device_nums", buf, sizeof(buf))) {
            int i=0;
            char* p = strtok(buf, " ");
            while (p && i<MAX_COPROC_INSTANCES) {
                device_nums[i++] = atoi(p);
                p = strtok(NULL, " ");
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

#endif

int COPROC::parse_opencl(XML_PARSER& xp) {
    char buf[1024];
    int n;

    MIOFILE& in = *(xp.f);
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc_opencl>")) {
            return 0;
        }
        if (parse_str(buf, "<name>", opencl_prop.name, sizeof(opencl_prop.name))) continue;
        if (parse_str(buf, "<vendor>", opencl_prop.vendor, sizeof(opencl_prop.vendor))) continue;
        if (parse_double(buf, "<peak_flops>", peak_flops)) continue;
        if (parse_int(buf, "<available>", n)) {
            opencl_prop.available = n;
            continue;
        }
        if (parse_cl_ulong(buf, "<hp_fp_config>", opencl_prop.hp_fp_config)) continue; 
        if (parse_cl_ulong(buf, "<sp_fp_config>", opencl_prop.sp_fp_config)) continue; 
        if (parse_cl_ulong(buf, "<dp_fp_config>", opencl_prop.dp_fp_config)) continue; 
        if (parse_int(buf, "<little_endian>", n)) {
            opencl_prop.little_endian = n;
            continue;
        }
        if (parse_cl_ulong(buf, "<exec_capabilities>", opencl_prop.exec_capab)) continue;
        if (parse_str(buf, "<extensions>", 
                    opencl_prop.extensions, 
                    sizeof(opencl_prop.extensions))) {
            continue;
        }
        if (parse_cl_ulong(buf, "<global_RAM>", opencl_prop.global_RAM)) continue;
        if (parse_cl_ulong(buf, "<local_RAM>", opencl_prop.local_RAM)) continue;
        if (parse_int(buf, "<max_clock_freq>", n)) {
            opencl_prop.max_clock_freq = n;
            continue;
        }
        if (parse_int(buf, "<max_cores>", n)) {
            opencl_prop.max_cores = n;
            continue;
        }
        if (parse_str(buf, "<openCL_platform_version>", 
                    opencl_prop.openCL_platform_version, 
                    sizeof(opencl_prop.openCL_platform_version))) {
            continue;
        }
        if (parse_str(buf, "<openCL_device_version>", 
                    opencl_prop.openCL_device_version, 
                    sizeof(opencl_prop.openCL_device_version))) {
            continue;
        }
        if (parse_str(buf, "<openCL_driver_version>", 
                    opencl_prop.openCL_driver_version, 
                    sizeof(opencl_prop.openCL_driver_version))) {
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void COPROCS::summary_string(char* buf, int len) {
    char bigbuf[8192], buf2[1024];

    strcpy(bigbuf, "");
    if (nvidia.count) {
        int mem = (int)(nvidia.prop.dtotalGlobalMem/MEGA);
        sprintf(buf2, "[CUDA|%s|%d|%dMB|%d]",
            nvidia.prop.name, nvidia.count, mem, nvidia.display_driver_version
        );
        strcat(bigbuf, buf2);
    }
    if (ati.count) {
        sprintf(buf2,"[CAL|%s|%d|%dMB|%s]",
            ati.name, ati.count, ati.attribs.localRAM, ati.version
        );
        strcat(bigbuf,buf2);
    }
    bigbuf[len-1] = 0;
    strcpy(buf, bigbuf);
}

int COPROCS::parse(XML_PARSER& xp) {
    char buf[1024];
    int retval;

    clear();
    n_rsc = 1;
    strcpy(coprocs[0].type, "CPU");
    MIOFILE& in = *(xp.f);
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coprocs>")) {
            return 0;
        }
        if (match_tag(buf, "<coproc_cuda>")) {
            retval = nvidia.parse(xp);
            if (retval) {
                nvidia.clear();
            } else {
                coprocs[n_rsc++] = nvidia;
            }
            continue;
        }
        if (match_tag(buf, "<coproc_ati>")) {
            retval = ati.parse(xp);
            if (retval) {
                ati.clear();
            } else {
                coprocs[n_rsc++] = ati;
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void COPROCS::write_xml(MIOFILE& mf, bool include_request) {
#ifndef _USING_FCGI_
//TODO: Write coprocs[0] through coprocs[n_rsc]
    mf.printf("    <coprocs>\n");
    if (nvidia.count) {
        nvidia.write_xml(mf, include_request);
    }
    if (ati.count) {
        ati.write_xml(mf, include_request);
    }
    mf.printf("    </coprocs>\n");
#endif
}

void COPROC_NVIDIA::description(char* buf) {
    char vers[256];
    if (display_driver_version) {
        sprintf(vers, "%d", display_driver_version);
    } else {
        strcpy(vers, "unknown");
    }
    sprintf(buf, "%s (driver version %s, CUDA version %d, compute capability %d.%d, %.0fMB, %.0f GFLOPS peak)",
        prop.name, vers, cuda_version, prop.major, prop.minor,
        prop.totalGlobalMem/(1024.*1024.), peak_flops/1e9
    );
}

#ifndef _USING_FCGI_
void COPROC_NVIDIA::write_xml(MIOFILE& f, bool include_request) {
    f.printf(
        "<coproc_cuda>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <have_cuda>%d</have_cuda>\n"
        "   <have_cal>%d</have_cal>\n"
        "   <have_opencl>%d</have_opencl>\n",
        count,
        prop.name,
        have_cuda ? 1 : 0,
        have_cal ? 1 : 0,
        have_opencl ? 1 : 0
    );
    if (include_request) {
        write_request(f);
    }
    f.printf(
        "   <peak_flops>%f</peak_flops>\n"
        "   <cudaVersion>%d</cudaVersion>\n"
        "   <drvVersion>%d</drvVersion>\n"
        "   <deviceHandle>%p</deviceHandle>\n"
        "   <totalGlobalMem>%u</totalGlobalMem>\n"
        "   <sharedMemPerBlock>%u</sharedMemPerBlock>\n"
        "   <regsPerBlock>%d</regsPerBlock>\n"
        "   <warpSize>%d</warpSize>\n"
        "   <memPitch>%u</memPitch>\n"
        "   <maxThreadsPerBlock>%d</maxThreadsPerBlock>\n"
        "   <maxThreadsDim>%d %d %d</maxThreadsDim>\n"
        "   <maxGridSize>%d %d %d</maxGridSize>\n"
        "   <clockRate>%d</clockRate>\n"
        "   <totalConstMem>%u</totalConstMem>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <textureAlignment>%u</textureAlignment>\n"
        "   <deviceOverlap>%d</deviceOverlap>\n"
        "   <multiProcessorCount>%d</multiProcessorCount>\n",
        peak_flops,
        cuda_version,
        display_driver_version,
        prop.deviceHandle,
        (unsigned int)prop.totalGlobalMem,
        (unsigned int)prop.sharedMemPerBlock,
        prop.regsPerBlock,
        prop.warpSize,
        (unsigned int)prop.memPitch,
        prop.maxThreadsPerBlock,
        prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2],
        prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2],
        prop.clockRate,
        (unsigned int)prop.totalConstMem,
        prop.major,
        prop.minor,
        (unsigned int)prop.textureAlignment,
        prop.deviceOverlap,
        prop.multiProcessorCount
    );


    if (have_opencl) {
        opencl_write_xml(f);
    }
    
    f.printf("</coproc_cuda>\n");
}
#endif

void COPROC_NVIDIA::clear() {
    COPROC::clear();
    strcpy(type, GPU_TYPE_NVIDIA);
    estimated_delay = -1;   // mark as absent
    cuda_version = 0;
    display_driver_version = 0;
    strcpy(prop.name, "");
    prop.deviceHandle = 0;
    prop.totalGlobalMem = 0;
    prop.sharedMemPerBlock = 0;
    prop.regsPerBlock = 0;
    prop.warpSize = 0;
    prop.memPitch = 0;
    prop.maxThreadsPerBlock = 0;
    prop.maxThreadsDim[0] = 0;
    prop.maxThreadsDim[1] = 0;
    prop.maxThreadsDim[2] = 0;
    prop.maxGridSize[0] = 0;
    prop.maxGridSize[1] = 0;
    prop.maxGridSize[2] = 0;
    prop.clockRate = 0;
    prop.totalConstMem = 0;
    prop.major = 0;
    prop.minor = 0;
    prop.textureAlignment = 0;
    prop.deviceOverlap = 0;
    prop.multiProcessorCount = 0;
}

int COPROC_NVIDIA::parse(XML_PARSER& xp) {
    char buf[1024], buf2[256];
    int retval;

    clear();
    MIOFILE& in = *(xp.f);
    while (in.fgets(buf, sizeof(buf))) {
        if (strstr(buf, "</coproc_cuda>")) {
            if (!peak_flops) {
				set_peak_flops();
            }
            return 0;
        }
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<peak_flops>", peak_flops)) continue;
        if (parse_bool(buf, "have_cuda", have_cuda)) continue;
        if (parse_bool(buf, "have_cal", have_cal)) continue;
        if (parse_bool(buf, "have_opencl", have_opencl)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;
        if (parse_int(buf, "<cudaVersion>", cuda_version)) continue;
        if (parse_int(buf, "<drvVersion>", display_driver_version)) continue;
        if (parse_str(buf, "<name>", prop.name, sizeof(prop.name))) continue;
        if (parse_int(buf, "<deviceHandle>", prop.deviceHandle)) continue;
        if (parse_double(buf, "<totalGlobalMem>", prop.dtotalGlobalMem)) {
            prop.totalGlobalMem = (int)prop.dtotalGlobalMem;
            continue;
        }
        if (parse_int(buf, "<sharedMemPerBlock>", (int&)prop.sharedMemPerBlock)) continue;
        if (parse_int(buf, "<regsPerBlock>", prop.regsPerBlock)) continue;
        if (parse_int(buf, "<warpSize>", prop.warpSize)) continue;
        if (parse_int(buf, "<memPitch>", (int&)prop.memPitch)) continue;
        if (parse_int(buf, "<maxThreadsPerBlock>", prop.maxThreadsPerBlock)) continue;
        if (parse_str(buf, "<maxThreadsDim>", buf2, sizeof(buf2))) {
            // can't use sscanf here (FCGI)
            //
            prop.maxThreadsDim[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                p++;
                prop.maxThreadsDim[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    p++;
                    prop.maxThreadsDim[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_str(buf, "<maxGridSize>", buf2, sizeof(buf2))) {
            prop.maxGridSize[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                p++;
                prop.maxGridSize[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    p++;
                    prop.maxGridSize[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_int(buf, "<clockRate>", prop.clockRate)) continue;
        if (parse_int(buf, "<totalConstMem>", (int&)prop.totalConstMem)) continue;
        if (parse_int(buf, "<major>", prop.major)) continue;
        if (parse_int(buf, "<minor>", prop.minor)) continue;
        if (parse_int(buf, "<textureAlignment>", (int&)prop.textureAlignment)) continue;
        if (parse_int(buf, "<deviceOverlap>", prop.deviceOverlap)) continue;
        if (parse_int(buf, "<multiProcessorCount>", prop.multiProcessorCount)) continue;
        if (match_tag(buf, "<coproc_opencl>")) {
            retval = parse_opencl(xp);
            if (retval) return retval;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

////////////////// ATI STARTS HERE /////////////////

#ifndef _USING_FCGI_
void COPROC_ATI::write_xml(MIOFILE& f, bool include_request) {
    f.printf(
        "<coproc_ati>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <have_cuda>%d</have_cuda>\n"
        "   <have_cal>%d</have_cal>\n"
        "   <have_opencl>%d</have_opencl>\n",
        count,
        name,
        have_cuda ? 1 : 0,
        have_cal ? 1 : 0,
        have_opencl ? 1 : 0
    );
    if (include_request) {
        write_request(f);
    }
    f.printf(
        "   <peak_flops>%f</peak_flops>\n"
        "   <CALVersion>%s</CALVersion>\n"
        "   <target>%d</target>\n"
        "   <localRAM>%d</localRAM>\n"
        "   <uncachedRemoteRAM>%d</uncachedRemoteRAM>\n"
        "   <cachedRemoteRAM>%d</cachedRemoteRAM>\n"
        "   <engineClock>%u</engineClock>\n"
        "   <memoryClock>%d</memoryClock>\n"
        "   <wavefrontSize>%d</wavefrontSize>\n"
        "   <numberOfSIMD>%d</numberOfSIMD>\n"
        "   <doublePrecision>%d</doublePrecision>\n"
        "   <pitch_alignment>%d</pitch_alignment>\n"
        "   <surface_alignment>%d</surface_alignment>\n"
        "   <maxResource1DWidth>%d</maxResource1DWidth>\n"
        "   <maxResource2DWidth>%d</maxResource2DWidth>\n"
        "   <maxResource2DHeight>%d</maxResource2DHeight>\n",
        peak_flops,
        version,
        attribs.target,
        attribs.localRAM,
        attribs.uncachedRemoteRAM,
        attribs.cachedRemoteRAM,
        attribs.engineClock,
        attribs.memoryClock,
        attribs.wavefrontSize,
        attribs.numberOfSIMD,
        attribs.doublePrecision,
        attribs.pitch_alignment,
        attribs.surface_alignment,
        info.maxResource1DWidth,
        info.maxResource2DWidth,
        info.maxResource2DHeight
    );

    if (atirt_detected) {
        f.printf("    <atirt_detected/>\n");
    }

    if (amdrt_detected) {
        f.printf("    <amdrt_detected/>\n");
    }

    if (have_opencl) {
        opencl_write_xml(f);
    }
        
    f.printf("</coproc_ati>\n");
};
#endif

void COPROC_ATI::clear() {
    COPROC::clear();
    strcpy(type, GPU_TYPE_ATI);
    estimated_delay = -1;
    strcpy(name, "");
    strcpy(version, "");
    atirt_detected = false;
    amdrt_detected = false;
    memset(&attribs, 0, sizeof(attribs));
    memset(&info, 0, sizeof(info));
}

int COPROC_ATI::parse(XML_PARSER& xp) {
    char buf[1024];
    int n;
    int retval;

    clear();

    MIOFILE& in = *(xp.f);
    while (in.fgets(buf, sizeof(buf))) {
        if (strstr(buf, "</coproc_ati>")) {
            int major, minor, release;
            sscanf(version, "%d.%d.%d", &major, &minor, &release);
            version_num = major*1000000 + minor*1000 + release;

            if (!peak_flops) {
				set_peak_flops();
            }
            return 0;
        }
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<peak_flops>", peak_flops)) continue;
        if (parse_bool(buf, "have_cuda", have_cuda)) continue;
        if (parse_bool(buf, "have_cal", have_cal)) continue;
        if (parse_bool(buf, "have_opencl", have_opencl)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<CALVersion>", version, sizeof(version))) continue;
        if (parse_bool(buf, "amdrt_detected", amdrt_detected)) continue;
        if (parse_bool(buf, "atirt_detected", atirt_detected)) continue;

        if (parse_int(buf, "<target>", n)) {
            attribs.target = (CALtarget)n;
            continue;
        }
        if (parse_int(buf, "<localRAM>", n)) {
            attribs.localRAM = n;
            continue;
        }
        if (parse_int(buf, "<uncachedRemoteRAM>", n)) {
            attribs.uncachedRemoteRAM = n;
            continue;
        }
        if (parse_int(buf, "<cachedRemoteRAM>", n)) {
            attribs.cachedRemoteRAM = n;
            continue;
        }
        if (parse_int(buf, "<engineClock>", n)) {
            attribs.engineClock = n;
            continue;
        }
        if (parse_int(buf, "<memoryClock>", n)) {
            attribs.memoryClock = n;
            continue;
        }
        if (parse_int(buf, "<wavefrontSize>", n)) {
            attribs.wavefrontSize = n;
            continue;
        }
        if (parse_int(buf, "<numberOfSIMD>"  , n)) {
            attribs.numberOfSIMD = n;
            continue;
        }
        if (parse_int(buf, "<doublePrecision>", n)) {
            attribs.doublePrecision = n?CAL_TRUE:CAL_FALSE;
            continue;
        }
        if (parse_int(buf, "<pitch_alignment>", n)) {
            attribs.pitch_alignment = n;
            continue;
        }
        if (parse_int(buf, "<surface_alignment>", n)) {
            attribs.surface_alignment = n;
            continue;
        }
        if (parse_int(buf, "<maxResource1DWidth>", n)) {
            info.maxResource1DWidth = n;
            continue;
        }
        if (parse_int(buf, "<maxResource2DWidth>", n)) {
            info.maxResource2DWidth = n;
            continue;
        }
        if (parse_int(buf, "<maxResource2DHeight>", n)) {
            info.maxResource2DHeight = n;
            continue;
        }
        if (match_tag(buf, "<coproc_opencl>")) {
            retval = parse_opencl(xp);
            if (retval) return retval;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void COPROC_ATI::description(char* buf) {
    sprintf(buf, "%s (CAL version %s, %.0fMB, %.0f GFLOPS peak)",
        name, version, attribs.localRAM/1024.*1024., peak_flops/1.e9
    );
}

#ifndef __WXWINDOWS__  /* Exclude from Manager */

#ifdef _WIN32
static HMODULE opencl_lib = NULL;

typedef cl_int (__stdcall *CL_PLATFORMIDS) (cl_uint, cl_platform_id*, cl_uint*);
typedef cl_int (__stdcall *CL_DEVICEIDS)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
typedef cl_int (__stdcall *CL_INFO) (cl_device_id, cl_device_info, size_t, void*, size_t*);

static CL_PLATFORMIDS  __clGetPlatformIDs = NULL;
static CL_DEVICEIDS    __clGetDeviceIDs = NULL;
static CL_INFO         __clGetDeviceInfo = NULL;

#else

static jmp_buf resume;

static void segv_handler(int) {
    longjmp(resume, 1);
}

static void* opencl_lib = NULL;

static cl_int (*__clGetPlatformIDs)(cl_uint    /* num_entries */,
                 cl_platform_id * /* platforms */,
                 cl_uint *        /* num_platforms */);
static cl_int (*__clGetDeviceIDs)(cl_platform_id   /* platform */,
               cl_device_type   /* device_type */,
               cl_uint          /* num_entries */,
               cl_device_id *   /* devices */,
               cl_uint *        /* num_devices */);
static cl_int (*__clGetDeviceInfo)(cl_device_id    /* device */,
                cl_device_info  /* param_name */,
                size_t          /* param_value_size */,
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */);
#endif

void boinc_getopencl_ids(char *type, int device_num, OPENCL_REFERENCE *ref) {
    cl_int errnum;
    cl_platform_id platforms[MAX_OPENCL_PLATFORMS];
    cl_uint num_platforms, platform_index, num_devices;
    cl_device_id devices[MAX_COPROC_INSTANCES];
    char vendor[256];                   // Device vendor (NVIDIA, ATI, AMD, etc.)

#ifdef _WIN32
    opencl_lib = LoadLibrary("OpenCL.dll");
    if (!opencl_lib) {
        ref->retval = ERR_NOT_IMPLEMENTED;
        return;
    }
    __clGetPlatformIDs = (CL_PLATFORMIDS)GetProcAddress( opencl_lib, "clGetPlatformIDs" );
    __clGetDeviceIDs = (CL_DEVICEIDS)GetProcAddress( opencl_lib, "clGetDeviceIDs" );
    __clGetDeviceInfo = (CL_INFO)GetProcAddress( opencl_lib, "clGetDeviceInfo" );
#else
#ifdef __APPLE__
    opencl_lib = dlopen("/System/Library/Frameworks/OpenCL.framework/Versions/Current/OpenCL", RTLD_NOW);
#else
//TODO: Is this correct?
    opencl_lib = dlopen("libOpenCL.so", RTLD_NOW);
#endif

    if (!opencl_lib) {
        ref->retval = ERR_NOT_IMPLEMENTED;
        return;
    }
    
    __clGetPlatformIDs = (cl_int(*)(cl_uint, cl_platform_id*, cl_uint*)) dlsym( opencl_lib, "clGetPlatformIDs" );
    __clGetDeviceIDs = (cl_int(*)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*)) dlsym( opencl_lib, "clGetDeviceIDs" );
    __clGetDeviceInfo = (cl_int(*)(cl_device_id, cl_device_info, size_t, void*, size_t*)) dlsym( opencl_lib, "clGetDeviceInfo" );
#endif
    if ((!__clGetPlatformIDs) || (!__clGetDeviceIDs) || (!__clGetDeviceInfo)) {
        ref->retval = ERR_NOT_IMPLEMENTED;
        goto bail;
    }

    errnum = (*__clGetPlatformIDs)(MAX_OPENCL_PLATFORMS, platforms, &num_platforms);
    if (num_platforms == 0) errnum = CL_DEVICE_NOT_FOUND;
    if (errnum != CL_SUCCESS) {
        ref->retval = errnum;
        goto bail;
    }
    
    for (platform_index=0; platform_index<num_platforms; ++platform_index) {
        errnum = (*__clGetDeviceIDs)(
            platforms[platform_index], CL_DEVICE_TYPE_GPU, MAX_COPROC_INSTANCES, devices, &num_devices
        );

        if (num_devices > (cl_uint)(device_num + 1)) continue;
    
        cl_device_id device_id = devices[device_num];

        errnum = (*__clGetDeviceInfo)(device_id, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
        if ((errnum != CL_SUCCESS) || (vendor[0] == 0)) continue;
            
        if ((strstr(vendor, "AMD")) ||  
            (strstr(vendor, "Advanced Micro Devices, Inc."))
        ) {
            strcpy(vendor, GPU_TYPE_ATI);
        }
        
        if (!strcmp(vendor, type)) {
            ref->device_id = device_id;
            ref->platform_id = platforms[platform_index];
            break;
        }
    }

bail:
#ifndef _WIN32
    dlclose(opencl_lib);
#endif

    if ((ref->retval == CL_SUCCESS) && (ref->device_id == NULL)) {
        ref->retval = CL_DEVICE_NOT_FOUND;
    }
}

OPENCL_REFERENCE boinc_getopencl_ids(int argc, char** argv) {
    OPENCL_REFERENCE ref;
    char type[256];
    int device_num;
    
    memset(&ref, 0, sizeof(struct OPENCL_REFERENCE));
    type[0] = '\0';
    device_num = -1;
    
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--gpu_type")) {
            strlcpy(type, argv[++i], sizeof(type));
        }
        if (!strcmp(argv[i], "--device")) {
            device_num = atoi(argv[++i]);
        }
    }

    if (type[0] == '\0'){
        ref.retval = CL_INVALID_DEVICE_TYPE;
        return ref;
    }
    
    if (device_num < 0) {
        ref.retval = CL_INVALID_DEVICE;
        return ref;
    }

#ifdef _WIN32
    try {
        boinc_getopencl_ids(type, device_num, &ref);
    }
    catch (...) {
        ref.retval = ERR_SIGNAL_CATCH;
    }
#else
    void (*old_sig)(int) = signal(SIGSEGV, segv_handler);
    if (setjmp(resume)) {
        ref.retval = ERR_SIGNAL_CATCH;
    } else {
        boinc_getopencl_ids(type, device_num, &ref);
    }

    signal(SIGSEGV, old_sig);
#endif
    
    if ((ref.retval == CL_SUCCESS) && (ref.device_id == NULL)) {
        ref.retval = CL_DEVICE_NOT_FOUND;
    }
    
    return ref;
}
#endif // __WXWINDOWS