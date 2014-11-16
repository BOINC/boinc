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

// represents the state of a intermediate upload
struct INTERMEDIATE_UPLOAD {
    std::string file;
    bool reported;
    bool ignore;

    INTERMEDIATE_UPLOAD() {
        clear();
    }
    void clear() {
        file = "";
        reported = false;
        ignore = false;
    }
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

class VBOX_BASE {
public:
    VBOX_BASE();
    ~VBOX_BASE();

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
    double memory_size_mb;
        // size of the memory allocation for the VM, in megabytes
    bool enable_cern_dataformat;
        // whether to use CERN specific data structures
    bool enable_isocontextualization;
        // whether to use an iso9660 image to implement VM contextualization (e.g. uCernVM)
    bool enable_cache_disk;
        // whether to add an extra cache disk for systems like uCernVM
    bool enable_network;
        // whether to allow network access
    bool network_bridged_mode;
        // use bridged mode for network
    bool enable_shared_directory;
        // whether to use shared directory infrastructure
    bool enable_floppyio;
        // whether to use floppy io infrastructure
    bool enable_remotedesktop;
        // whether to enable remote desktop functionality
    bool enable_gbac;
        // whether to enable GBAC functionality
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
    std::vector<INTERMEDIATE_UPLOAD> intermediate_upload_files;
        // if find file of this name in shared/, send specified file
    std::string completion_trigger_file;
        // if find this file in shared/, task is over.
        // File can optionally contain exit code (first line)
        // File can optionally contain is_notice bool (second line)
        // and stderr text (subsequent lines).
        // Addresses a problem where VM doesn't shut down properly
    std::string temporary_exit_trigger_file;
        // if find this file in shared/, task is restarted at a later date.
        // File can optionally contain restart delay (first line)
        // File can optionally contain is_notice bool (second line)
        // and stderr text (subsequent lines).
        // Addresses a problem where VM doesn't shut down properly

    /////////// END VBOX_JOB.XML ITEMS //////////////


    virtual int initialize();
    virtual int create_vm();
    virtual int register_vm();
    virtual int deregister_vm(bool delete_media);
    virtual int deregister_stale_vm();
    virtual void poll(bool log_state = true);
    virtual int start();
    virtual int stop();
    virtual int poweroff();
    virtual int pause();
    virtual int resume();
    virtual int create_snapshot(double elapsed_time);
    virtual int cleanup_snapshots(bool delete_active);
    virtual int restore_snapshot();

    virtual int run(bool do_restore_snapshot);
    virtual void cleanup();

    virtual void dump_hypervisor_logs(bool include_error_logs);
    virtual void dump_hypervisor_status_reports();
    virtual void dump_vmguestlog_entries();

    virtual int is_registered();
    virtual bool is_system_ready(std::string& message);
    virtual bool is_vm_machine_configuration_available();
    virtual bool is_hdd_registered();
    virtual bool is_extpack_installed();
    virtual bool is_logged_failure_vm_extensions_disabled();
    virtual bool is_logged_failure_vm_extensions_in_use();
    virtual bool is_logged_failure_vm_extensions_not_supported();
    virtual bool is_logged_failure_host_out_of_memory();
    virtual bool is_logged_failure_guest_job_out_of_memory();
    virtual bool is_virtualbox_version_newer(int maj, int min, int rel);

    virtual int get_install_directory(std::string& dir);
    virtual int get_version_information(std::string& version);
    virtual int get_guest_additions(std::string& dir);
    virtual int get_slot_directory(std::string& dir);
    virtual int get_default_network_interface(std::string& iface);
    virtual int get_vm_network_bytes_sent(double& sent);
    virtual int get_vm_network_bytes_received(double& received);
    virtual int get_vm_process_id();
    virtual int get_vm_exit_code(unsigned long& exit_code);
    virtual double get_vm_cpu_time();

    virtual int get_system_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);
    virtual int get_vm_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);
    virtual int get_trace_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);

    virtual int set_network_access(bool enabled);
    virtual int set_cpu_usage(int percentage);
    virtual int set_network_usage(int kilobytes);

    virtual int read_floppy(std::string& data);
    virtual int write_floppy(std::string& data);

    virtual void lower_vm_process_priority();
    virtual void reset_vm_process_priority();

    virtual void sanitize_output(std::string& output);


};

#endif
