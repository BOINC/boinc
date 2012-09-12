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

#ifndef _HOSTINFO_
#define _HOSTINFO_

// Description of a host's hardware and software.
// This is used a few places:
// - it's part of the client's state file, client_state.xml
// - it's passed in the reply to the get_host_info GUI RPC
// - it's included in scheduler RPC requests
//
// Other host-specific info is kept in
// TIME_STATS (on/connected/active fractions)
// NET_STATS (average network bandwidths)

#include "miofile.h"
#include "coproc.h"

// if you add fields, update clear_host_info()

class HOST_INFO {
public:
    int timezone;                 // local STANDARD time - UTC time (in seconds)
    char domain_name[256];
    char serialnum[256];
    char ip_addr[256];
    char host_cpid[64];

    int p_ncpus;
    char p_vendor[256];
    char p_model[256];
    char p_features[1024];
    double p_fpops;
    double p_iops;
    double p_membw;
    double p_calculated;          // when benchmarks were last run, or zero
    bool p_vm_extensions_disabled;

    double m_nbytes;              // Total amount of memory in bytes
    double m_cache;
    double m_swap;                // Total amount of swap space in bytes

    double d_total;               // Total amount of disk in bytes
    double d_free;                // Total amount of free disk in bytes

    char os_name[256];
    char os_version[256];

    // the following is non-empty if VBox is installed
    //
    char virtualbox_version[256];

    COPROCS _coprocs;

    HOST_INFO();
    int parse(XML_PARSER&, bool benchmarks_only = false);
    int write(MIOFILE&, bool include_net_info, bool include_coprocs);
    int parse_cpu_benchmarks(FILE*);
    int write_cpu_benchmarks(FILE*);
    void print();

    bool host_is_running_on_batteries();
#ifdef __APPLE__
    bool users_idle(bool check_all_logins, double idle_time_to_run, double *actual_idle_time=NULL);
#else
    bool users_idle(bool check_all_logins, double idle_time_to_run);
#endif
#ifdef ANDROID
    bool host_wifi_online();
#endif
    int get_host_info();
    int get_local_network_info();
    int get_virtualbox_version();
    void clear_host_info();
    void make_random_string(const char* salt, char* out);
    void generate_host_cpid();
};

#ifdef __APPLE__
#ifdef __cplusplus
extern "C" {
#endif

#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/hidsystem/event_status_driver.h>

bool isDualGPUMacBook();

// Apple has removed NxIdleTime() beginning with OS 10.6, so we must try
// loading it at run time to avoid a link error.  For details, please see
// the comments in the __APPLE__ version of HOST_INFO::users_idle() in
// client/hostinfo_unix.cpp.
typedef double (*nxIdleTimeProc)(NXEventHandle handle);
#ifdef __cplusplus
}	// extern "C"
#endif

extern NXEventHandle gEventHandle;
#endif

#endif
