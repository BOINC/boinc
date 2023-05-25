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


//
//
extern bool is_boinc_client_version_newer(APP_INIT_DATA& aid, int maj, int min, int rel);


// represents a VirtualBox Guest Log Timestamp
struct VBOX_TIMESTAMP {
    int hours;
    int minutes;
    int seconds;
    int milliseconds;
};


class VBOX_BASE : public VBOX_JOB {
    public:
        VBOX_BASE();
        virtual ~VBOX_BASE();

        std::string virtualbox_home_directory;
        std::string virtualbox_scratch_directory;
        std::string virtualbox_install_directory;
        std::string virtualbox_guest_additions;
        std::string virtualbox_version_raw;
        std::string virtualbox_version_display;

        FloppyIONS::FloppyIO* pFloppy;

        // last polled copy of the log file
        std::string vm_log;
        // last VM guest log entry detected
        VBOX_TIMESTAMP vm_log_timestamp;
        // unique name for the VM
        std::string vm_master_name;
        // unique description for the VM
        std::string vm_master_description;
        // unique name for the VM or UUID of a stale VM if deregistering it
        std::string vm_name;
        // required CPU core count
        std::string vm_cpu_count;
        // name of the virtual machine disk image file
        std::string image_filename;
        // name of the virtual machine iso9660 disk image file
        std::string iso_image_filename;
        // name of the virtual machine cache disk image file
        std::string cache_disk_filename;
        // name of the virtual machine floppy disk image file
        std::string floppy_image_filename;
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

        virtual int initialize() = 0;
        virtual int create_vm() = 0;
        virtual int register_vm() = 0;
        virtual int deregister_vm(bool delete_media) = 0;
        virtual int deregister_stale_vm() = 0;
        virtual int poll(bool log_state = true) = 0;
	virtual int poll2(bool log_state = true) = 0;
        virtual int start() = 0;
        virtual int stop() = 0;
        virtual int poweroff() = 0;
        virtual int pause() = 0;
        virtual int resume() = 0;
        virtual int capture_screenshot() = 0;
        virtual int create_snapshot(double elapsed_time) = 0;
        virtual int cleanup_snapshots(bool delete_active) = 0;
        virtual int restore_snapshot() = 0;

        virtual int run(bool do_restore_snapshot);
        virtual void cleanup();

        virtual void dump_hypervisor_logs(bool include_error_logs);
        virtual void dump_hypervisor_status_reports() = 0;
        virtual void dump_vmguestlog_entries();
        virtual int dump_screenshot();
        virtual std::string read_vm_log();


        virtual int is_registered() = 0;
        virtual bool is_system_ready(std::string& message) = 0;
        virtual bool is_vm_machine_configuration_available();
        virtual bool is_disk_image_registered() = 0;
        virtual bool is_extpack_installed() = 0;
        virtual bool is_logged_failure_vm_extensions_disabled();
        virtual bool is_logged_failure_vm_extensions_in_use();
        virtual bool is_logged_failure_vm_extensions_not_supported();
        virtual bool is_logged_failure_vm_powerup();
        virtual bool is_logged_failure_host_out_of_memory();
        virtual bool is_logged_failure_guest_job_out_of_memory();
        virtual bool is_virtualbox_version_newer(int maj, int min, int rel);

        static int get_install_directory(std::string& dir);
        static int get_scratch_directory(std::string& dir);
        static int get_version_information(std::string& version_raw, std::string& version_display);
        virtual int get_guest_additions(std::string& dir) = 0;
        virtual int get_slot_directory(std::string& dir);
        virtual int get_default_network_interface(std::string& iface) = 0;
        virtual int get_vm_network_bytes_sent(double& sent) = 0;
        virtual int get_vm_network_bytes_received(double& received) = 0;
        virtual int get_vm_process_id() = 0;
        virtual int get_vm_exit_code(unsigned long& exit_code) = 0;
        virtual double get_vm_cpu_time() = 0;

        virtual int get_system_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);
        virtual int get_vm_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);
        virtual int get_trace_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);
        virtual int get_startup_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);

        virtual int set_network_access(bool enabled) = 0;
        virtual int set_cpu_usage(int percentage) = 0;
        virtual int set_network_usage(int kilobytes) = 0;

        virtual int read_floppy(std::string& data);
        virtual int write_floppy(std::string& data);

        virtual void lower_vm_process_priority() = 0;
        virtual void reset_vm_process_priority() = 0;

        static void sanitize_format(std::string& output);
        static void sanitize_output(std::string& output);

        virtual int launch_vboxsvc();
        virtual int launch_vboxvm();

        int vbm_popen(
                std::string& command, std::string& output, const char* item,
                bool log_error = true, bool retry_failures = true,
                unsigned int timeout = 45, bool log_trace = true
                );
        int vbm_popen_raw(
                std::string& command, std::string& output , unsigned int timeout
                );
        static void vbm_replay(std::string& command);
        static void vbm_trace(std::string& command, std::string& ouput, int retval);

        virtual std::string get_error(int choice);

        virtual void report_clean(
            bool unrecoverable_error,
            bool skip_cleanup,
            bool do_dump_hypervisor_logs,
            int retval,
            std::string error_reason,
            int temp_delay,
            std::string temp_reason,
            double current_cpu_time,
            double last_checkpoint_cpu_time,
            double fraction_done,
            double bytes_sent,
            double bytes_received
        );
};

class VBOX_VM : public VBOX_BASE {
    public:
        VBOX_VM();
        ~VBOX_VM();
};

#endif

