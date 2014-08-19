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

// represents a VirtualBox Guest Log Timestamp
struct VBOX_TIMESTAMP {
    int hours;
    int minutes;
    int seconds;
    int milliseconds;
};

struct PORT_FORWARD {
    int host_port;      // 0 means assign dynamically
    int guest_port;
    bool is_remote;

    PORT_FORWARD() {
        host_port = 0;
        guest_port = 0;
        is_remote = false;
    }
    int get_host_port();    // assign host port
};

// represents a VirtualBox VM
class VBOX_VM {
public:
    VBOX_VM();
    ~VBOX_VM();

    std::string virtualbox_home_directory;
    std::string virtualbox_install_directory;
    std::string virtualbox_guest_additions;
    std::string virtualbox_version;

    FloppyIO* pFloppy;

    std::string vm_log;
        // last polled copy of the log file
    VBOX_TIMESTAMP vm_log_timestamp;
        // last VM guest log entry detected
    std::string vm_master_name;
        // unique name for the VM
    std::string vm_master_description;
        // unique description for the VM
    std::string vm_name;
        // unique name for the VM or UUID of a stale VM if deregistering it
    std::string vm_cpu_count;
        // required CPU core count
    std::string memory_size_mb;
        // size of the memory allocation for the VM, in megabytes
    std::string image_filename;
        // name of the virtual machine disk image file
    std::string iso_image_filename;
        // name of the virtual machine iso9660 disk image file
    std::string cache_disk_filename;
        // name of the virtual machine cache disk image file
    std::string floppy_image_filename;
        // name of the virtual machine floppy disk image file
    double current_cpu_time;
        // amount of CPU time consumed by the VM (note: use get_vm_cpu_time())
    bool suspended;
        // is the VM suspended?
    bool network_suspended;
        // is network access temporarily suspended?
    bool online;
        // is VM even online?
    bool saving;
        // Is VM saving from checkpoint?
    bool restoring;
        // Is VM restoring from checkpoint?
    bool crashed;
        // Has the VM crashed?
    bool enable_cern_dataformat;
        // whether to use CERN specific data structures
    bool register_only;
        // whether we were instructed to only register the VM.
        // useful for debugging VMs.
    int rd_host_port;
        // for optional remote desktop; dynamically assigned
    bool headless;

    /////////// THE FOLLOWING SPECIFIED IN VBOX_JOB.XML //////////////
    // some of these don't really belong in this class

    std::string os_name;
        // name of the OS the VM runs
    std::string vm_disk_controller_type;
        // the type of disk controller to emulate
    std::string vm_disk_controller_model;
        // the disk controller model to emulate
    bool enable_isocontextualization;
        // whether to use an iso9660 image to implement VM contextualization (e.g. uCernVM)
    bool enable_cache_disk;
        // whether to add an extra cache disk for systems like uCernVM
    bool enable_network;
        // whether to allow network access
    bool bridged_mode;
        // use bridged mode for network
    bool enable_shared_directory;
        // whether to use shared directory infrastructure
    bool enable_floppyio;
        // whether to use floppy io infrastructure
    bool enable_remotedesktop;
        // whether to enable remote desktop functionality
    double job_duration;
        // maximum amount of wall-clock time this VM is allowed to run before
        // considering itself done.
    std::string fraction_done_filename;
        // name of file where app will write its fraction done
    int pf_guest_port;      // if nonzero, do port forwarding for Web GUI
    int pf_host_port;       // AFAIK this isn't needed
    std::vector<PORT_FORWARD> port_forwards;
    double minimum_checkpoint_interval;
        // minimum time between checkpoints
    std::vector<std::string> copy_to_shared;
        // list of files to copy from slot dir to shared/
    std::vector<std::string> trickle_trigger_files;
        // if find file of this name in shared/, send trickle-up message
        // with variety = filename, contents = file contents
    std::string completion_trigger_file;
        // if find this file in shared/, task is over.
        // File can optionally contain exit code (first line)
        // and stderr text (subsequent lines).
        // Addresses a problem where VM doesn't shut down properly

    /////////// END VBOX_JOB.XML ITEMS //////////////

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
    int parse_port_forward(XML_PARSER&);
    void set_web_graphics_url();
    void poll(bool log_state = true);

    int create_vm();
    int register_vm();
    int deregister_vm(bool delete_media);
    int deregister_stale_vm();

    int run(bool do_restore_snapshot);
    void cleanup();

    int start();
    int stop();
    int poweroff();
    int pause();
    int resume();
    void check_trickle_triggers();
    void check_completion_trigger();
    int create_snapshot(double elapsed_time);
    int cleanup_snapshots(bool delete_active);
    int restore_snapshot();
    void dump_hypervisor_logs(bool include_error_logs);
    void dump_hypervisor_status_reports();
    void dump_vmguestlog_entries();

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
    int get_version_information(std::string& version);
    int get_guest_additions(std::string& dir);
    int get_slot_directory(std::string& dir);
    int get_vm_network_bytes_sent(double& sent);
    int get_vm_network_bytes_received(double& received);
    int get_vm_process_id();
    int get_vm_exit_code(unsigned long& exit_code);
    double get_vm_cpu_time();

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
