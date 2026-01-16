// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2023 University of California
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


// VBOX_BASE represents the state of a running VM

#ifndef BOINC_VBOX_COMMON_H
#define BOINC_VBOX_COMMON_H

#include "vboxjob.h"

// Known VirtualBox/COM error codes
//
#ifndef CO_E_SERVER_EXEC_FAILURE
#define CO_E_SERVER_EXEC_FAILURE        0x80080005
#endif
#ifndef RPC_S_SERVER_UNAVAILABLE
#define RPC_S_SERVER_UNAVAILABLE        0x800706BA
#endif
#ifndef VBOX_E_OBJECT_NOT_FOUND
#define VBOX_E_OBJECT_NOT_FOUND         0x80BB0001
#endif
#ifndef VBOX_E_INVALID_VM_STATE
#define VBOX_E_INVALID_VM_STATE         0x80BB0002
#endif
#ifndef VBOX_E_VM_ERROR
#define VBOX_E_VM_ERROR                 0x80BB0003
#endif
#ifndef VBOX_E_FILE_ERROR
#define VBOX_E_FILE_ERROR               0x80BB0004
#endif
#ifndef VBOX_E_IPRT_ERROR
#define VBOX_E_IPRT_ERROR               0x80BB0005
#endif
#ifndef VBOX_E_PDM_ERROR
#define VBOX_E_PDM_ERROR                0x80BB0006
#endif
#ifndef VBOX_E_INVALID_OBJECT_STATE
#define VBOX_E_INVALID_OBJECT_STATE     0x80BB0007
#endif
#ifndef VBOX_E_HOST_ERROR
#define VBOX_E_HOST_ERROR               0x80BB0008
#endif
#ifndef VBOX_E_NOT_SUPPORTED
#define VBOX_E_NOT_SUPPORTED            0x80BB0009
#endif
#ifndef VBOX_E_XML_ERROR
#define VBOX_E_XML_ERROR                0x80BB000A
#endif
#ifndef VBOX_E_INVALID_SESSION_STATE
#define VBOX_E_INVALID_SESSION_STATE    0x80BB000B
#endif
#ifndef VBOX_E_OBJECT_IN_USE
#define VBOX_E_OBJECT_IN_USE            0x80BB000B
#endif


// Vboxwrapper errors
//
#define VBOXWRAPPER_ERR_RECOVERABLE     -1000

#define ENV_UNCLEAN     0
#define VM_RUNNING      1
#define NO_HA           2
#define VBOX_SNAPSHOT   3
#define SESSION_LOCK    4
#define HA_OFF          5
#define LOCKED_HA       6
#define NO_MEM          7
#define NOT_ONLINE      8
#define VM_ENV          9
#define FOREIGN_HYPERV  10
#define TEMP_NO_MEM     11
#define NO_ONLINE       12

// Vboxwrapper diagnostics
//
#define SCREENSHOT_FILENAME "vbox_screenshot.png"
#define REPLAYLOG_FILENAME "vbox_replay.txt"
#define TRACELOG_FILENAME "vbox_trace.txt"

extern bool is_boinc_client_version_newer(int maj, int min, int rel);

// represents a VirtualBox Guest Log Timestamp
//
struct VBOX_TIMESTAMP {
    int hours;
    int minutes;
    int seconds;
    int milliseconds;
};

struct VBOX_BASE : VBOX_JOB {
    VBOX_BASE();
    ~VBOX_BASE();

    // Was 'virtualbox_home_directory' in previous releases.
    // The directory where VirtualBox stores
    // global configuration files and
    // global logfiles such as VBoxSVC.log.
    // It is user based and in the documentation sometimes
    // referred to as "home", sometimes a "profile".
    // Renamed since the latter seems to be more precise and a user can switch
    // between different locations (=profiles) using the VBOX_USER_HOME environment variable.
    //
    string virtualbox_profile_directory;

    // Directory where VirtualBox installs it's executables.
    //
    string virtualbox_install_directory;

    // Path where the VirtualBox Guest Additions iso file is located.
    // Never mix "VirtualBox Guest Additions" with "VirtualBox Extension Pack".
    // The first is part of the base package and to be installed in the guest VM,
    // the latter is to be installed on the host OS and published under a different license.
    // See the VirtualBox documentation for further details.
    //
    string virtualbox_guest_additions;

    string virtualbox_version_raw;
    string virtualbox_version_display;

    FloppyIONS::FloppyIO* pFloppy;

    // last polled copy of the log file
    string vm_log;
    // last VM guest log entry detected
    VBOX_TIMESTAMP vm_log_timestamp;
    // unique name for the VM
    string vm_master_name;
    // unique description for the VM
    string vm_master_description;
    // unique name for the VM or UUID of a stale VM if deregistering it
    string vm_name;
    // required CPU core count
    string vm_cpu_count;
    // name of the virtual machine disk image file
    string image_filename;
    // name of the virtual machine iso9660 disk image file
    string iso_image_filename;
    // name of the virtual machine cache disk image file
    string cache_disk_filename;
    // name of the virtual machine floppy disk image file
    string floppy_image_filename;
    // amount of CPU time consumed by the VM (note: use get_vm_cpu_time())
    double current_cpu_time;
    // is the VM suspended?
    bool suspended;
    // is network access temporarily suspended?
    bool network_suspended;
    // is VM even online?
    bool online;
    // Is VM saving from checkpoint?
    bool saving;
    // Is VM restoring from checkpoint?
    bool restoring;
    // Has the VM crashed?
    bool crashed;
    // Has the VM been successfully started?
    bool started_successfully;
    // whether we were instructed to only register the VM.
    // useful for debugging VMs.
    bool register_only;
    // for optional remote desktop; dynamically assigned
    int rd_host_port;
    bool headless;

    std::streamoff log_pointer;

    int vm_pid;
#ifdef _WIN32
    // the handle to the process for the VM
    // NOTE: we get a handle to the pid right after we parse it from the
    //   log files so we can adjust the process priority and retrieve the process
    //   exit code in case it crashed or was terminated.  Without an outstanding
    //   handle to the process, the OS is free to reuse the pid for some other
    //   executable.
    HANDLE vm_pid_handle;
#endif

    void dump_vmguestlog_entries();
    int dump_screenshot();
    string read_vm_log();
    bool is_vm_machine_configuration_available();
    bool is_logged_failure_vm_extensions_disabled();
    bool is_logged_failure_vm_extensions_in_use();
    bool is_logged_failure_vm_extensions_not_supported();
    bool is_logged_failure_vm_powerup();
    bool is_logged_failure_host_out_of_memory();
    bool is_logged_failure_guest_job_out_of_memory();
    bool is_virtualbox_version_newer(int maj, int min, int rel);

    static int get_install_directory(string& dir);
    static int get_version_information(
        string& version_raw, string& version_display
    );
    int get_system_log(
        string& log, bool tail_only = true, unsigned int buffer_size = 8192
    );
    int get_vm_log(
        string& log, bool tail_only = true, unsigned int buffer_size = 8192
    );
    int get_trace_log(
        string& log, bool tail_only = true, unsigned int buffer_size = 8192
    );
    int get_startup_log(
        string& log, bool tail_only = true, unsigned int buffer_size = 8192
    );

    int read_floppy(string& data);
    int write_floppy(string& data);

    static void sanitize_format(string& output);
    static void sanitize_output(string& output);

    int vbm_popen(
        string& command, string& output, const char* item,
        bool log_error = true, bool retry_failures = true,
        unsigned int timeout = 45, bool log_trace = true
    );
    int vbm_popen_raw(
        string& command, string& output , unsigned int timeout
    );
    static void vbm_replay(string& command);
    static void vbm_trace(string& command, string& ouput, int retval);

    string get_error(int choice);
};

#endif
