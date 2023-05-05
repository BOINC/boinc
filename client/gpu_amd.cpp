// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// Detection of AMD/ATI GPUs
//
// Docs:
// http://developer.amd.com/gpu/ATIStreamSDK/assets/ATI_Stream_SDK_CAL_Programming_Guide_v2.0%5B1%5D.pdf
// ?? why don't they have HTML docs??

#ifdef _WIN32
#include "boinc_win.h"
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#endif
#include "config.h"
#include <dlfcn.h>
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;

#include "coproc.h"
#include "str_replace.h"
#include "util.h"

#include "client_msgs.h"
#include "gpu_detect.h"

static void get_available_ati_ram(COPROC_ATI &cc, vector<string>& warnings);

// criteria:
//
// - double precision support
// - local RAM
// - speed
//
int ati_compare(COPROC_ATI& c1, COPROC_ATI& c2, bool loose) {
    if (c1.attribs.doublePrecision && !c2.attribs.doublePrecision) return 1;
    if (!c1.attribs.doublePrecision && c2.attribs.doublePrecision) return -1;
    if (loose) {
        if (c1.attribs.localRAM> 1.4*c2.attribs.localRAM) return 1;
        if (c1.attribs.localRAM< .7* c2.attribs.localRAM) return -1;
        return 0;
    }
    if (c1.attribs.localRAM > c2.attribs.localRAM) return 1;
    if (c1.attribs.localRAM < c2.attribs.localRAM) return -1;
    double s1 = c1.peak_flops;
    double s2 = c2.peak_flops;
    if (s1 > s2) return 1;
    if (s1 < s2) return -1;
    return 0;
}

#ifdef _WIN32
typedef int (__stdcall *ATI_ATTRIBS) (CALdeviceattribs *attribs, CALuint ordinal);
typedef int (__stdcall *ATI_CLOSE)(void);
typedef int (__stdcall *ATI_GDC)(CALuint *numDevices);
typedef int (__stdcall *ATI_GDI)(void);
typedef int (__stdcall *ATI_INFO) (CALdeviceinfo *info, CALuint ordinal);
typedef int (__stdcall *ATI_VER) (CALuint *cal_major, CALuint *cal_minor, CALuint *cal_imp);
typedef int (__stdcall *ATI_STATUS) (CALdevicestatus*, CALdevice);
typedef int (__stdcall *ATI_DEVICEOPEN) (CALdevice*, CALuint);
typedef int (__stdcall *ATI_DEVICECLOSE) (CALdevice);

ATI_ATTRIBS p_calDeviceGetAttribs = NULL;
ATI_CLOSE   p_calShutdown = NULL;
ATI_GDC     p_calDeviceGetCount = NULL;
ATI_GDI     p_calInit = NULL;
ATI_INFO    p_calDeviceGetInfo = NULL;
ATI_VER     p_calGetVersion = NULL;
ATI_STATUS  p_calDeviceGetStatus = NULL;
ATI_DEVICEOPEN  p_calDeviceOpen = NULL;
ATI_DEVICECLOSE  p_calDeviceClose = NULL;

#else

int (*p_calInit)();
int (*p_calGetVersion)(CALuint*, CALuint*, CALuint*);
int (*p_calDeviceGetCount)(CALuint*);
int (*p_calDeviceGetAttribs)(CALdeviceattribs*, CALuint);
int (*p_calShutdown)();
int (*p_calDeviceGetInfo)(CALdeviceinfo*, CALuint);
int (*p_calDeviceGetStatus)(CALdevicestatus*, CALdevice);
int (*p_calDeviceOpen)(CALdevice*, CALuint);
int (*p_calDeviceClose)(CALdevice);

#endif

void COPROC_ATI::get(
    vector<string>& warnings
) {
    CALuint numDevices, cal_major, cal_minor, cal_imp;
    char buf[256];
    int retval;
    COPROC_ATI cc, cc2;
    string s, gpu_name;

    attribs.struct_size = sizeof(CALdeviceattribs);
    numDevices =0;

#ifdef _WIN32

#if defined _M_X64
    const char* atilib_name = "aticalrt64.dll";
    const char* amdlib_name = "amdcalrt64.dll";
#else
    const char* atilib_name = "aticalrt.dll";
    const char* amdlib_name = "amdcalrt.dll";
#endif

    HINSTANCE callib = LoadLibrary(atilib_name);
    if (callib) {
        atirt_detected = true;
    } else {
        callib = LoadLibrary(amdlib_name);
        if (callib) {
            amdrt_detected = true;
        }
    }

    if (!callib) {
        gpu_warning(warnings, "No ATI library found.");
        return;
    }

    p_calInit = (ATI_GDI)GetProcAddress(callib, "calInit" );
    p_calGetVersion = (ATI_VER)GetProcAddress(callib, "calGetVersion" );
    p_calDeviceGetCount = (ATI_GDC)GetProcAddress(callib, "calDeviceGetCount" );
    p_calDeviceGetAttribs =(ATI_ATTRIBS)GetProcAddress(callib, "calDeviceGetAttribs" );
    p_calShutdown = (ATI_CLOSE)GetProcAddress(callib, "calShutdown" );
    p_calDeviceGetInfo = (ATI_INFO)GetProcAddress(callib, "calDeviceGetInfo" );
    p_calDeviceGetStatus = (ATI_STATUS)GetProcAddress(callib, "calDeviceGetStatus" );
    p_calDeviceOpen = (ATI_DEVICEOPEN)GetProcAddress(callib, "calDeviceOpen" );
    p_calDeviceClose = (ATI_DEVICECLOSE)GetProcAddress(callib, "calDeviceClose" );

#else

    void* callib = dlopen("libaticalrt.so", RTLD_NOW);
    if (!callib) {
        snprintf(buf, sizeof(buf), "ATI: %s", dlerror());
        gpu_warning(warnings, buf);
        return;
    }

    atirt_detected = true;

    p_calInit = (int(*)()) dlsym(callib, "calInit");
    p_calGetVersion = (int(*)(CALuint*, CALuint*, CALuint*)) dlsym(callib, "calGetVersion");
    p_calDeviceGetCount = (int(*)(CALuint*)) dlsym(callib, "calDeviceGetCount");
    p_calDeviceGetAttribs = (int(*)(CALdeviceattribs*, CALuint)) dlsym(callib, "calDeviceGetAttribs");
    p_calShutdown = (int(*)()) dlsym(callib, "calShutdown");
    p_calDeviceGetInfo = (int(*)(CALdeviceinfo*, CALuint)) dlsym(callib, "calDeviceGetInfo");
    p_calDeviceGetStatus = (int(*)(CALdevicestatus*, CALdevice)) dlsym(callib, "calDeviceGetStatus");
    p_calDeviceOpen = (int(*)(CALdevice*, CALuint)) dlsym(callib, "calDeviceOpen");
    p_calDeviceClose = (int(*)(CALdevice)) dlsym(callib, "calDeviceClose");

#endif

    if (!p_calInit) {
        gpu_warning(warnings, "calInit() missing from CAL library");
        goto leave;
    }
    if (!p_calGetVersion) {
        gpu_warning(warnings, "calGetVersion() missing from CAL library");
        goto leave;
    }
    if (!p_calDeviceGetCount) {
        gpu_warning(warnings, "calDeviceGetCount() missing from CAL library");
        goto leave;
    }
    if (!p_calDeviceGetAttribs) {
        gpu_warning(warnings, "calDeviceGetAttribs() missing from CAL library");
        goto leave;
    }
    if (!p_calDeviceGetInfo) {
        gpu_warning(warnings, "calDeviceGetInfo() missing from CAL library");
        goto leave;
    }

    retval = (*p_calInit)();
    if (retval != CAL_RESULT_OK) {
        snprintf(buf, sizeof(buf), "calInit() returned %d", retval);
        gpu_warning(warnings, buf);
        goto leave;
    }
    retval = (*p_calDeviceGetCount)(&numDevices);
    if (retval != CAL_RESULT_OK) {
        snprintf(buf, sizeof(buf), "calDeviceGetCount() returned %d", retval);
        gpu_warning(warnings, buf);
        goto leave;
    }
    retval = (*p_calGetVersion)(&cal_major, &cal_minor, &cal_imp);
    if (retval != CAL_RESULT_OK) {
        snprintf(buf, sizeof(buf), "calGetVersion() returned %d", retval);
        gpu_warning(warnings, buf);
        goto leave;
    }

    if (!numDevices) {
        gpu_warning(warnings, "No usable CAL devices found");
        goto leave;
    }

    for (CALuint i=0; i<numDevices; i++) {
        retval = (*p_calDeviceGetInfo)(&info, i);
        if (retval != CAL_RESULT_OK) {
            snprintf(buf, sizeof(buf), "calDeviceGetInfo() returned %d", retval);
            gpu_warning(warnings, buf);
            goto leave;
        }
        retval = (*p_calDeviceGetAttribs)(&attribs, i);
        if (retval != CAL_RESULT_OK) {
            snprintf(buf, sizeof(buf), "calDeviceGetAttribs() returned %d", retval);
            gpu_warning(warnings, buf);
            goto leave;
        }
        switch ((int)attribs.target) {
        case CAL_TARGET_600:
            gpu_name="ATI Radeon HD 2900 (RV600)";
            break;
        case CAL_TARGET_610:
            gpu_name="ATI Radeon HD 2300/2400/3200/4200 (RV610)";
            attribs.numberOfSIMD=1;        // set correct values (reported wrong by driver)
            attribs.wavefrontSize=32;
            break;
        case CAL_TARGET_630:
            gpu_name="ATI Radeon HD 2600/3650 (RV630/RV635)";
            // set correct values (reported wrong by driver)
            attribs.numberOfSIMD=3;
            attribs.wavefrontSize=32;
            break;
        case CAL_TARGET_670:
            gpu_name="ATI Radeon HD 3800 (RV670)";
            break;
        case CAL_TARGET_710:
            gpu_name="ATI Radeon HD 4350/4550 (R710)";
            break;
        case CAL_TARGET_730:
            gpu_name="ATI Radeon HD 4600 series (R730)";
            break;
        case CAL_TARGET_7XX:
            gpu_name="ATI Radeon (RV700 class)";
            break;
        case CAL_TARGET_770:
            gpu_name="ATI Radeon HD 4700/4800 (RV740/RV770)";
            break;
        case 8:
            gpu_name="ATI Radeon HD 5800/5900 series (Cypress/Hemlock)";
            break;
        case 9:
            gpu_name="ATI Radeon HD 5700/6750/6770 series (Juniper)";
            break;
        case 10:
            gpu_name="ATI Radeon HD 5500/5600 series (Redwood)";
            break;
        case 11:
            gpu_name="ATI Radeon HD 5400/R5 210 series (Cedar)";
            break;
        case 12:
            gpu_name="AMD Radeon HD 6370D/6380G/6410D/6480G (Sumo)";
            break;
        case 13:
            gpu_name="AMD Radeon HD 6520G/6530D/6550D/6620G (SuperSumo)";
            break;
        case 14:
            gpu_name="AMD Radeon HD 6200/6300/7200/7300 series (Wrestler)";
            break;
        case 15:
            gpu_name="AMD Radeon HD 6900 series (Cayman)";
            break;
        case 16:
            gpu_name="AMD Radeon HD (Kauai)";
            break;
        case 17:
            gpu_name="AMD Radeon HD 6790/6850/6870 series (Barts)";
            break;
        case 18:
            gpu_name="AMD Radeon HD 6570/6670/7570/7670 series (Turks)";
            break;
        case 19:
            gpu_name="AMD Radeon HD 6350/6450/7450/7470/R5 230 series (Caicos)";
            break;
        case 20:
            gpu_name="AMD Radeon HD 7870/7950/7970/R9 280/R9 280X series (Tahiti)";
            break;
        case 21:
            gpu_name="AMD Radeon HD 7850/7870 series (Pitcairn)";
            break;
        case 22:
            gpu_name="AMD Radeon HD 7700/R7 250X/R9 255 series (Capeverde)";
            break;
        case 23:
            gpu_name="AMD Radeon HD 7500/7600/8500/8600 series (Devastator)";
            break;
        case 24:
            gpu_name="AMD Radeon HD 7400/7500/8300/8400 series (Scrapper)";
            break;
        case 25:
            gpu_name="AMD Radeon HD 8600/8790M/R5 330/R5 340/R7 240/R7 250/R7 340/R7 350 (Oland)";
            break;
        case 26:
            gpu_name="AMD Radeon HD 7790/R7 260/R7 260X/R9 360 (Bonaire)";
            break;
        case 27:
            gpu_name="AMD Radeon HD (Spectre)"; // Kaveri
            break;
        case 28:
            gpu_name="AMD Radeon HD (Spooky)";  // Kaveri
            break;
        case 29:
            gpu_name="AMD Radeon HD 8200/8300/8400 series (Kalindi)"; // Kabini
            break;
        case 30:
            gpu_name="AMD Radeon HD 8600M (Hainan)";
            break;
        case 31:
            gpu_name="AMD Radeon R7 265/R9 270/R9 270X/R9 370 (Curacao)";
            break;
        case 32:
            gpu_name="AMD Radeon R9 290 (Hawaii)";
            break;
        case 33:
            gpu_name="AMD Radeon R2/R3 (Skunk)"; // Mullins/new FT3 APU
            break;
        case 34:
            gpu_name="AMD Radeon R9 285/R9 380 (Tonga)";
            break;
        case 35:
            gpu_name="AMD Radeon R9 295X2 (Vesuvius)";
            break;
        case 36:
            gpu_name="AMD Radeon R7 360 (Tobago)";
            break;
        case 37:
            gpu_name="AMD Radeon R7 370/R9 370X (Trinidad)";
            break;
        case 38:
            gpu_name="AMD Radeon R9 390/R9 390X (Grenada)";
            break;
        case 39:
            gpu_name="AMD Radeon R9 Fury/R9 Nano/R9 Fury X/R9 Fury X2 (Fiji)";
            break;
        default:
            gpu_name="AMD Radeon HD (unknown)";
            break;
        }
        have_cal = true;
        cc.have_cal = true;
        cc.attribs = attribs;
        cc.info = info;
        safe_strcpy(cc.name, gpu_name.c_str());
        snprintf(cc.version, sizeof(cc.version), "%u.%u.%u", cal_major, cal_minor, cal_imp);
        cc.amdrt_detected = amdrt_detected;
        cc.atirt_detected = atirt_detected;
        cc.device_num = i;
        cc.set_peak_flops();
        if (cc.bad_gpu_peak_flops("CAL", s)) {
            gpu_warning(warnings, s.c_str());
        }
        get_available_ati_ram(cc, warnings);
        ati_gpus.push_back(cc);
    }

    // shut down CAL, otherwise Lenovo won't be able to switch to low-power GPU
    //
    retval = (*p_calShutdown)();

    if (!ati_gpus.size()) {
        gpu_warning(warnings, "No ATI GPUs found");
    }
leave:
#ifdef _WIN32
    if (callib) FreeLibrary(callib);
#else
    if (callib) dlclose(callib);
#endif
}

void COPROC_ATI::correlate(
    bool use_all,
    vector<int>& ignore_devs
) {
    char buf[256];

    if (!ati_gpus.size()) return;

    // find the most capable non-ignored instance
    //
    bool first = true;
    unsigned int i;
    for (i=0; i<ati_gpus.size(); i++) {
        if (in_vector(ati_gpus[i].device_num, ignore_devs)) continue;
        if (first) {
            *this = ati_gpus[i];
            first = false;
        } else if (ati_compare(ati_gpus[i], *this, false) > 0) {
            *this = ati_gpus[i];
        }
    }

    // see which other instances are equivalent,
    // and set the "count" and "device_nums" fields
    //
    count = 0;
    for (i=0; i<ati_gpus.size(); i++) {
        ati_gpus[i].description(buf, sizeof(buf));
        if (in_vector(ati_gpus[i].device_num, ignore_devs)) {
            ati_gpus[i].is_used = COPROC_IGNORED;
        } else if (this->have_opencl && !ati_gpus[i].have_opencl) {
            ati_gpus[i].is_used = COPROC_UNUSED;
        } else if (this->have_cal && !ati_gpus[i].have_cal) {
            ati_gpus[i].is_used = COPROC_UNUSED;
        } else if (use_all || !ati_compare(ati_gpus[i], *this, true)) {
            device_nums[count] = ati_gpus[i].device_num;
            count++;
            ati_gpus[i].is_used = COPROC_USED;
        } else {
            ati_gpus[i].is_used = COPROC_UNUSED;
        }
    }
}

// get available RAM of ATI GPU
//
// CAUTION: as currently written, this method should be
// called only from COPROC_ATI::get().  If in the future
// you wish to call it from additional places:
// * It must be called from a separate child process on
//   dual-GPU laptops (e.g., Macbook Pros) with the results
//   communicated to the main client process via IPC or a
//   temp file.  See the comments about dual-GPU laptops
//   in gpu_detect.cpp and main.cpp for more details.
// * The CAL library must be loaded and calInit() called
//   first.
// * See client/coproc_detect.cpp and cpu_sched.cpp in
//   BOINC 6.12.36 for an earlier attempt to call this
//   from the scheduler.  Note that it was abandoned
//   due to repeated calls crashing the driver.
//
static void get_available_ati_ram(COPROC_ATI &cc, vector<string>& warnings) {
    CALdevicestatus st;
    CALdevice dev;
    char buf[256];
    int retval;

    cc.available_ram = cc.attribs.localRAM*MEGA;

    st.struct_size = sizeof(CALdevicestatus);

    if (!p_calDeviceOpen) {
        gpu_warning(warnings, "calDeviceOpen() missing from CAL library");
        return;
    }
    if (!p_calDeviceGetStatus) {
        gpu_warning(warnings, "calDeviceGetStatus() missing from CAL library");
        return;
    }
    if (!p_calDeviceClose) {
        gpu_warning(warnings, "calDeviceClose() missing from CAL library");
        return;
    }

    retval = (*p_calDeviceOpen)(&dev, cc.device_num);
    if (retval) {
        snprintf(buf, sizeof(buf),
            "[coproc] calDeviceOpen(%d) returned %d", cc.device_num, retval
        );
        gpu_warning(warnings, buf);
        return;
    }
    retval = (*p_calDeviceGetStatus)(&st, dev);
    if (retval) {
        snprintf(buf, sizeof(buf),
            "[coproc] calDeviceGetStatus(%d) returned %d",
            cc.device_num, retval
        );
        gpu_warning(warnings, buf);
        (*p_calDeviceClose)(dev);
        return;
    }
    cc.available_ram = st.availLocalRAM*MEGA;
    (*p_calDeviceClose)(dev);
}
