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

// raw floppy drive device
class FloppyIO;

// represents a VirtualBox VM
struct VBOX_VM {
    VBOX_VM();
    ~VBOX_VM();

    // Floppy IO abstraction
    FloppyIO* pFloppy;

    // unique master name for the VM
    std::string vm_master_name;
    // unique name for the VM or UUID of a stale VM if deregistering a stale VM
    std::string vm_name;
    // required CPU core count
    std::string vm_cpu_count;
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
    // is the VM suspended?
    bool suspended;
    // is network access temporarily suspended?
    bool network_suspended;
    // is VM even online?
    bool online;
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
#ifdef _WIN32
    // the handle to the process for the VM
    // NOTE: we get a handle to the pid right after we parse it from the
    //   log files so we can adjust the process priority and retrieve the process
    //   exit code in case it crashed or was terminated.  Without an outstanding
    //   handle to the process, the OS is free to reuse the pid for some other
    //   executable.
    HANDLE vm_pid_handle;
#else
    // the pid to the VM process
    int vm_pid;
#endif

    int initialize();
    int run(double elapsed_time);
    int start();
    int stop();
    int poweroff();
    int pause();
    int resume();
    int createsnapshot(double elapsed_time, double checkpoint_cpu_time);
    int restoresnapshot();
    void cleanup();
    void poll(bool log_state = true);

    bool is_hdd_registered();
    bool is_registered();
    bool is_extpack_installed();

    int register_vm();
    int deregister_vm();
    int deregister_stale_vm();

    int get_install_directory(std::string& dir);
    int get_slot_directory(std::string& dir);
    int get_network_bytes_sent(double& sent);
    int get_network_bytes_received(double& received);
    int get_system_log(std::string& log);
    int get_vm_log(std::string& log);
    int get_vm_exit_code(unsigned long& exit_code);
    int get_vm_process_id(int& process_id);
    int get_port_forwarding_port();
    int get_remote_desktop_port();

    int set_network_access(bool enabled);
    int set_cpu_usage_fraction(double);
    int set_network_max_bytes_sec(double);

    int read_floppy(std::string& data);
    int write_floppy(std::string& data);

    void lower_vm_process_priority();
    void reset_vm_process_priority();

    int vbm_popen(
        std::string& command, std::string& output, const char* item, bool log_error = true, bool retry_failures = true
    );
    int vbm_popen_raw(
        std::string& command, std::string& output
    );
};

#endif
