// This file is part of BOINC.
// http://boinc.berkeley.edu
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

#ifndef BOINC_HOSTINFO_H
#define BOINC_HOSTINFO_H

// struct HOST_INFO describes a host's hardware and software.
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
#include "common_defs.h"

#ifdef _WIN32
#include "wslinfo.h"
#endif

#define USER_IDLE_TIME_INF   86400

// we use three ways of getting info about a Linux OS (name/version)
// They all involve parsing some text.
// Variants:
enum LINUX_OS_INFO_PARSER {
    lsbrelease,
    osrelease,
    redhatrelease
};

const char command_lsbrelease[] = "/usr/bin/lsb_release -a 2>&1";
const char file_osrelease[] = "/etc/os-release";
const char file_redhatrelease[] = "/etc/redhat-release";

extern const char* docker_cli_prog(DOCKER_TYPE type);
extern const char* docker_type_str(DOCKER_TYPE type);

// if you add fields, update clear_host_info()

#define P_FEATURES_SIZE 8192

class HOST_INFO {
public:
    int timezone;                 // local STANDARD time - UTC time (in seconds)
    char domain_name[256];
    char ip_addr[256];
    char host_cpid[64];

    int p_ncpus;
    char p_vendor[256];
    char p_model[256];
    char p_features[P_FEATURES_SIZE];
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

#ifdef _WIN32
    // on Windows, Docker info is per WSL_DISTRO, not global
    WSL_DISTROS wsl_distros;
#else
    char docker_version[256]; // null if not present
    DOCKER_TYPE docker_type;
    char docker_compose_version[256];
    DOCKER_TYPE docker_compose_type;
#endif

    char product_name[256];       // manufacturer and/or model of system
    char mac_address[256];      // MAC addr e.g. 00:00:00:00:00:00
                                // currently populated for Android

    // the following is non-empty if VBox is installed
    //
    char virtualbox_version[256];

    COPROCS coprocs;

    int num_opencl_cpu_platforms;
    OPENCL_CPU_PROP opencl_cpu_prop[MAX_OPENCL_CPU_PLATFORMS];

#ifdef _WIN32
    int n_processor_groups;
#endif

    void clear_host_info();
    HOST_INFO();

    int parse(XML_PARSER&, bool static_items_only = false);
    int write(MIOFILE&, bool include_net_info, bool include_coprocs);
    int parse_cpu_benchmarks(FILE*);
    int write_cpu_benchmarks(FILE*);
    void print();

    bool host_is_running_on_batteries();
    long user_idle_time(bool check_all_logins);
        // seconds since last user interaction
    int get_host_info(bool init);
    int get_cpu_info();
    int get_cpu_count();
    int get_memory_info();
    int get_os_info();
    int get_host_battery_charge();
    int get_host_battery_state();
    int get_local_network_info();
    int get_virtualbox_version();
    bool have_docker();
#ifdef __APPLE__
    bool is_podman_VM_running();
#endif
#ifndef _WIN32
    // on Windows, Docker info is per WSL_DISTRO, not global
    bool get_docker_version();
    bool get_docker_version_aux(DOCKER_TYPE);
    bool get_docker_compose_version();
    bool get_docker_compose_version_aux(DOCKER_TYPE);
#endif
    void make_random_string(const char* salt, char* out);
    void generate_host_cpid();
    static bool parse_linux_os_info(
        FILE* file, const LINUX_OS_INFO_PARSER parser,
        char* os_name, const int os_name_size, char* os_version,
        const int os_version_size
    );
    static bool parse_linux_os_info(
        const std::string& line, const LINUX_OS_INFO_PARSER parser,
        char* os_name, const int os_name_size, char* os_version,
        const int os_version_size
    );
    static bool parse_linux_os_info(
        const std::vector<std::string>& lines,
        const LINUX_OS_INFO_PARSER parser,
        char* os_name, const int os_name_size, char* os_version,
        const int os_version_size
    );
    static bool get_docker_version_string(
        DOCKER_TYPE type, const char* raw, std::string& version
    );
    static bool get_docker_compose_version_string(
        DOCKER_TYPE type, const char* raw, std::string& version
    );
#ifdef _WIN32
    void win_get_processor_info();
#endif
};

extern void make_secure_random_string(char*);

#ifdef _WIN32
#ifndef SIM
extern BOOL get_OSVERSIONINFO(OSVERSIONINFOEX& osvi);
#endif
#endif

#ifdef _WIN32
extern int get_wsl_information(WSL_DISTROS &distros);
extern int get_processor_group(HANDLE);
#endif

#ifdef __APPLE__
extern int get_system_uptime();
extern bool can_run_on_this_CPU(char* exec_path);
    // can the app run on this CPU architecture?

#ifdef __cplusplus
extern "C" {
#endif

#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/hidsystem/event_status_driver.h>

extern bool isDualGPUMacBook();

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

// is the filesystem remote? (Linux only)
//
extern int is_filesystem_remote(const char* path, bool&);

#endif
