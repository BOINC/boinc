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
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"

#include "coproc.h"

#ifndef _USING_FCGI_
using std::perror;
#endif

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
#endif

int COPROC_REQ::parse(MIOFILE& fin) {
    char buf[1024];
    strcpy(type, "");
    count = 0;
    while (fin.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc>")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<type>", type, sizeof(type))) continue;
        if (parse_double(buf, "<count>", count)) continue;
    }
    return ERR_XML_PARSE;
}

int COPROC::parse(MIOFILE& fin) {
    char buf[1024];
    strcpy(type, "");
    count = 0;
    used = 0;
    req_secs = 0;
    estimated_delay = 0;
    req_instances = 0;
    while (fin.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc>")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<type>", type, sizeof(type))) continue;
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;
    }
    return ERR_XML_PARSE;
}

void COPROCS::summary_string(char* buf, int len) {
    char bigbuf[8192], buf2[1024];

    strcpy(bigbuf, "");
    for (unsigned int i=0; i<coprocs.size(); i++) {
        COPROC* cp = coprocs[i];
        if (!strcmp(cp->type, "CUDA")) {
            COPROC_CUDA* cp2 = (COPROC_CUDA*) cp;
            int mem = (int)(cp2->prop.dtotalGlobalMem/MEGA);
            sprintf(buf2, "[CUDA|%s|%d|%dMB|%d]",
                cp2->prop.name, cp2->count, mem, cp2->display_driver_version
            );
            strcat(bigbuf, buf2);
        } else if (!strcmp(cp->type, "ATI")){
            COPROC_ATI* cp2 =(COPROC_ATI*) cp;
            sprintf(buf2,"[CAL|%s|%d|%dMB|%s]",
                cp2->name, cp2->count, cp2->attribs.localRAM, cp2->version
            );
            strcat(bigbuf,buf2);
        }
    }
    bigbuf[len-1] = 0;
    strcpy(buf, bigbuf);
}

int COPROCS::parse(MIOFILE& fin) {
    char buf[1024];

    while (fin.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coprocs>")) {
            return 0;
        }
        if (strstr(buf, "<coproc_cuda>")) {
            COPROC_CUDA* cc = new COPROC_CUDA;
            int retval = cc->parse(fin);
            if (!retval) {
                coprocs.push_back(cc);
            }
        }
        if (strstr(buf, "<coproc_ati>")) {
            COPROC_ATI* cc = new COPROC_ATI;
            int retval = cc->parse(fin);
            if (!retval) {
                coprocs.push_back(cc);
            }
        }
    }
    return ERR_XML_PARSE;
}

void COPROCS::write_xml(MIOFILE& mf) {
#ifndef _USING_FCGI_
    mf.printf("    <coprocs>\n");
    for (unsigned i=0; i<coprocs.size(); i++) {
        COPROC* c = coprocs[i];
        c->write_xml(mf);
    }
    mf.printf("    </coprocs>\n");
#endif
}

COPROC* COPROCS::lookup(const char* type) {
    for (unsigned int i=0; i<coprocs.size(); i++) {
        COPROC* cp = coprocs[i];
        if (!strcmp(type, cp->type)) return cp;
    }
    return NULL;
}

#ifdef _WIN32

#endif

void COPROC_CUDA::description(char* buf) {
    char vers[256];
    if (display_driver_version) {
        sprintf(vers, "%d", display_driver_version);
    } else {
        strcpy(vers, "unknown");
    }
    sprintf(buf, "%s (driver version %s, CUDA version %d, compute capability %d.%d, %.0fMB, %.0f GFLOPS peak)",
        prop.name, vers, cuda_version, prop.major, prop.minor,
        prop.totalGlobalMem/(1024.*1024.), peak_flops()/1e9
    );
}

#ifndef _USING_FCGI_
void COPROC_CUDA::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc_cuda>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <req_secs>%f</req_secs>\n"
        "   <req_instances>%f</req_instances>\n"
        "   <estimated_delay>%f</estimated_delay>\n"
        "   <drvVersion>%d</drvVersion>\n"
        "   <cudaVersion>%d</cudaVersion>\n"
        "   <totalGlobalMem>%u</totalGlobalMem>\n"
        "   <sharedMemPerBlock>%u</sharedMemPerBlock>\n"
        "   <regsPerBlock>%d</regsPerBlock>\n"
        "   <warpSize>%d</warpSize>\n"
        "   <memPitch>%u</memPitch>\n"
        "   <maxThreadsPerBlock>%d</maxThreadsPerBlock>\n"
        "   <maxThreadsDim>%d %d %d</maxThreadsDim>\n"
        "   <maxGridSize>%d %d %d</maxGridSize>\n"
        "   <totalConstMem>%u</totalConstMem>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <clockRate>%d</clockRate>\n"
        "   <textureAlignment>%u</textureAlignment>\n"
        "   <deviceOverlap>%d</deviceOverlap>\n"
        "   <multiProcessorCount>%d</multiProcessorCount>\n"
        "</coproc_cuda>\n",
        count,
        prop.name,
        req_secs,
        req_instances,
        estimated_delay,
        display_driver_version,
        cuda_version,
        (unsigned int)prop.totalGlobalMem,
        (unsigned int)prop.sharedMemPerBlock,
        prop.regsPerBlock,
        prop.warpSize,
        (unsigned int)prop.memPitch,
        prop.maxThreadsPerBlock,
        prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2],
        prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2],
        (unsigned int)prop.totalConstMem,
        prop.major,
        prop.minor,
        prop.clockRate,
        (unsigned int)prop.textureAlignment,
        prop.deviceOverlap,
        prop.multiProcessorCount
    );
}
#endif

void COPROC_CUDA::clear() {
    count = 0;
    used = 0;
    req_secs = 0;
    req_instances = 0;
    estimated_delay = -1;   // mark as absent
    cuda_version = 0;
    display_driver_version = 0;
    strcpy(prop.name, "");
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

int COPROC_CUDA::parse(MIOFILE& fin) {
    char buf[1024], buf2[256];

    clear();
    while (fin.fgets(buf, sizeof(buf))) {
        if (strstr(buf, "</coproc_cuda>")) {
            return 0;
        }
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;
        if (parse_str(buf, "<name>", prop.name, sizeof(prop.name))) continue;
        if (parse_int(buf, "<drvVersion>", display_driver_version)) continue;
        if (parse_int(buf, "<cudaVersion>", cuda_version)) continue;
        if (parse_double(buf, "<totalGlobalMem>", prop.dtotalGlobalMem)) continue;
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
    }
    return ERR_XML_PARSE;
}

////////////////// ATI STARTS HERE /////////////////

#ifndef _USING_FCGI_
void COPROC_ATI::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc_ati>\n"
    );

    f.printf(
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <req_secs>%f</req_secs>\n"
        "   <req_instances>%f</req_instances>\n"
        "   <estimated_delay>%f</estimated_delay>\n"
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
        "   <maxResource2DHeight>%d</maxResource2DHeight>\n"
        "   <CALVersion>%s</CALVersion>\n",
        count,
        name,
        req_secs,
        req_instances,
        estimated_delay,
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
        info.maxResource2DHeight,
        version
    );

    if (atirt_detected) {
        f.printf("    <atirt_detected/>\n");
    }

    if (amdrt_detected) {
        f.printf("    <amdrt_detected/>\n");
    }

    f.printf("</coproc_ati>\n");
};
#endif

void COPROC_ATI::clear() {
    count = 0;
    used = 0;
    req_secs = 0;
    req_instances = 0;
    estimated_delay = -1;
    strcpy(name, "");
    strcpy(version, "");
    atirt_detected = false;
    amdrt_detected = false;
    memset(&attribs, 0, sizeof(attribs));
    memset(&info, 0, sizeof(info));
}

int COPROC_ATI::parse(MIOFILE& fin) {
    char buf[1024];
    int n;

    clear();

    while (fin.fgets(buf, sizeof(buf))) {
        if (strstr(buf, "</coproc_ati>")) return 0;
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_double(buf, "<req_instances>", req_instances)) continue;
        if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;

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
        if (parse_bool(buf, "amdrt_detected", amdrt_detected)) continue;
        if (parse_bool(buf, "atirt_detected", atirt_detected)) continue;
        if (parse_str(buf, "<CALVersion>", version, sizeof(version))) continue;
    }
    return ERR_XML_PARSE;
}

void COPROC_ATI::description(char* buf) {
    sprintf(buf, "%s (CAL version %s, %.0fMB, %.0f GFLOPS peak)",
        name, version, attribs.localRAM/1024.*1024., peak_flops()/1.e9
    );
}
