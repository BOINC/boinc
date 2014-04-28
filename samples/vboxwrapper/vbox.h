// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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


// Provide cross-platform interfaces for making changes to VirtualBox

#ifndef _VBOX_H_
#define _VBOX_H_

// Known VirtualBox/COM error codes
//
#ifndef CO_E_SERVER_EXEC_FAILURE
#define CO_E_SERVER_EXEC_FAILURE        0x80080005
#endif
#ifndef RPC_S_SERVER_UNAVAILABLE
#define RPC_S_SERVER_UNAVAILABLE        0x800706ba
#endif
#ifndef VBOX_E_INVALID_OBJECT_STATE
#define VBOX_E_INVALID_OBJECT_STATE     0x80bb0007
#endif

// Vboxwrapper errors
//
#define VBOXWRAPPER_ERR_RECOVERABLE     -1000

// Vboxwrapper diagnostics
//
#define REPLAYLOG_FILENAME "vbox_replay.txt"
#define TRACELOG_FILENAME "vbox_trace.txt"


// raw floppy drive device
class FloppyIO;

// represents a VirtualBox VM
struct VBOX_VM {
    VBOX_VM();
    ~VBOX_VM();

    // Virtualbox Home Directory
    std::string virtualbox_home_directory;
    // Virtualbox Install Directory
    std::string virtualbox_install_directory;
    // Virtualbox Version Information
    std::string virtualbox_version;

    // Floppy IO abstraction
    FloppyIO* pFloppy;

    // last polled copy of the log file
    std::string vm_log;
    // unique name for the VM
    std::string vm_master_name;
    // unique description for the VM
    std::string vm_master_description;
    // unique name for the VM or UUID of a stale VM if deregistering a stale VM
    std::string vm_name;
    // required CPU core count
    std::string vm_cpu_count;
    // the type of disk controller to emulate
    std::string vm_disk_controller_type;
    // the disk controller model to emulate
    std::string vm_disk_controller_model;
    // name of the OS the VM runs
    std::string os_name;
    // size of the memory allocation for the VM, in megabytes
    std::string memory_size_mb;
    // name of the virtual machine disk image file
    std::string image_filename;
    // name of the virtual machine floppy disk image file
    std::string floppy_image_filename;
    // maximum amount of wall-clock time this VM is allowed to run before
    // considering itself done.
    double job_duration;
    // name of file where app will write its fraction done
    std::string fraction_done_filename;
    // is the VM suspended?
    bool suspended;
    // is network access temporarily suspended?
    bool network_suspended;
    // is VM even online?
    bool online;
    // Is VM saving/restoring from checkpoint?
    bool saving;
    bool restoring;
    // Has the VM crashed?
    bool crashed;
    // whether to use CERN specific data structures
    bool enable_cern_dataformat;
    // whether to use shared directory infrastructure at all
    bool enable_shared_directory;
    // whether to use floppy io infrastructure at all
    bool enable_floppyio;
    // whether to enable remote desktop functionality
    bool enable_remotedesktop;
    // whether to allow network access at all
    bool enable_network;
    // whether we were instructed to only register the VM.
    // useful for debugging VMs.
    bool register_only;
    // the following for optional port forwarding
    int pf_host_port;
        // specified in config file
    int pf_guest_port;
        // specified in config file
    // the following for optional remote desktop
    int rd_host_port;
        // dynamically assigned
    bool headless;

    int vm_pid;
    int vboxsvc_pid;
#ifdef _WIN32
    // the handle to the process for the VM
    // NOTE: we get a handle to the pid right after we parse it from the
    //   log files so we can adjust the process priority and retrieve the process
    //   exit code in case it crashed or was terminated.  Without an outstanding
    //   handle to the process, the OS is free to reuse the pid for some other
    //   executable.
    HANDLE vm_pid_handle;

    // the handle to the vboxsvc process created by us in the sandbox'ed environment
    HANDLE vboxsvc_pid_handle;
#endif

    int initialize();
    void poll(bool log_state = true);

    int create_vm();
    int register_vm();
    int deregister_vm(bool delete_media);
    int deregister_stale_vm();

    int run(bool restore_snapshot);
    void cleanup();

    int start();
    int stop();
    int poweroff();
    int pause();
    int resume();
    int createsnapshot(double elapsed_time);
    int cleanupsnapshots(bool delete_active);
    int restoresnapshot();
    void dumphypervisorlogs(bool include_error_logs);
    void dumphypervisorstatusreports();

    int is_registered();
    bool is_system_ready(std::string& message);
    bool is_vm_machine_configuration_available();
    bool is_hdd_registered();
    bool is_extpack_installed();
    bool is_logged_failure_vm_extensions_disabled();
    bool is_logged_failure_vm_extensions_in_use();
    bool is_logged_failure_vm_extensions_not_supported();
    bool is_logged_failure_host_out_of_memory();
    bool is_logged_failure_guest_job_out_of_memory();
    bool is_virtualbox_version_newer(int maj, int min, int rel);
    bool is_virtualbox_error_recoverable(int retval);

    int get_install_directory(std::string& dir);
    int get_slot_directory(std::string& dir);
    int get_port_forwarding_port();
    int get_remote_desktop_port();
    int get_vm_network_bytes_sent(double& sent);
    int get_vm_network_bytes_received(double& received);
    int get_vm_process_id();
    int get_vm_exit_code(unsigned long& exit_code);

    int get_system_log(std::string& log, bool tail_only = true);
    int get_vm_log(std::string& log, bool tail_only = true);
    int get_trace_log(std::string& log, bool tail_only = true);

    int set_network_access(bool enabled);
    int set_cpu_usage(int percentage);
    int set_network_usage(int kilobytes);

    int read_floppy(std::string& data);
    int write_floppy(std::string& data);

    void lower_vm_process_priority();
    void reset_vm_process_priority();

    int launch_vboxsvc();
    int launch_vboxvm();

    void sanitize_output(std::string& output);

    int vbm_popen(
        std::string& command, std::string& output, const char* item, bool log_error = true, bool retry_failures = true, unsigned int timeout = 45, bool log_trace = true
    );
    int vbm_popen_raw(
        std::string& command, std::string& output, unsigned int timeout
    );
    void vbm_replay(
        std::string& command
    );
    void vbm_trace(
        std::string& command, std::string& ouput, int retval
    );
};

#endif
