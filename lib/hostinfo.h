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

#ifndef _HOSTINFO_
#define _HOSTINFO_

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "miofile.h"

// Other host-specific info is kept in
// TIME_STATS (on/connected/active fractions)
// NET_STATS (average network bandwidths)

struct HOST_INFO {
    int timezone;    // local STANDARD time - UTC time
    char domain_name[256];
    char serialnum[256];
    char ip_addr[256];
    char host_cpid[64];

    int p_ncpus;
    char p_vendor[256];
    char p_model[256];
    double p_fpops;
    double p_iops;
    double p_membw;
    int p_fpop_err;
    int p_iop_err;
    int p_membw_err;
    double p_calculated; //needs to be initialized to zero

    char os_name[256];
    char os_version[256];

    double m_nbytes;
    double m_cache;
    double m_swap;

    double d_total;
    double d_free;

    int parse(MIOFILE&);
    int write(MIOFILE&);
    int parse_cpu_benchmarks(FILE*);
    int write_cpu_benchmarks(FILE*);

    bool host_is_running_on_batteries();
    bool users_idle(bool check_all_logins, double idle_time_to_run);
    int get_host_info();
    void clear_host_info();
    void generate_host_cpid();
};

#ifdef _WIN32
    extern HINSTANCE g_hIdleDetectionDll;       // handle to DLL for user idle
#endif

#ifdef __APPLE__
#ifdef __cplusplus
extern "C" {
#endif
#include <mach/port.h>
typedef mach_port_t NXEventHandle;
NXEventHandle NXOpenEventStatus(void);
extern double NXIdleTime(NXEventHandle handle);
#ifdef __cplusplus
}	// extern "C"
#endif

extern NXEventHandle gEventHandle;
#endif

#endif
