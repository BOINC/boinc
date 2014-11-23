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


#ifndef _VBOX_WIN_H_
#define _VBOX_WIN_H_


class VBOX_VM : public VBOX_BASE {
public:
    VBOX_VM();
    ~VBOX_VM();


    CComPtr<IVirtualBox> m_pVirtualBox;
    CComPtr<ISession> m_pSession;
    CComPtr<IMachine> m_pMachine;

    // the pid/handle to the process for the VM/VboxSvc
    int vm_pid;
    HANDLE vm_pid_handle;
    int vboxsvc_pid;
    HANDLE vboxsvc_pid_handle;


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

    void dump_hypervisor_logs(bool include_error_logs);
    void dump_hypervisor_status_reports();
    void dump_vmguestlog_entries();
    void check_trickle_triggers();
    void check_intermediate_uploads();

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
    bool is_logged_completion_file_exists();
    bool is_virtualbox_version_newer(int maj, int min, int rel);
    bool is_virtualbox_error_recoverable(int retval);

    int get_install_directory(std::string& dir);
    int get_version_information(std::string& version);
    int get_guest_additions(std::string& dir);
    int get_slot_directory(std::string& dir);
    int get_default_network_interface(std::string& iface);
    int get_vm_network_bytes_sent(double& sent);
    int get_vm_network_bytes_received(double& received);
    int get_vm_process_id();
    int get_vm_exit_code(unsigned long& exit_code);
    double get_vm_cpu_time();

    int get_system_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);
    int get_vm_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);
    int get_trace_log(std::string& log, bool tail_only = true, unsigned int buffer_size = 8192);

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

};

#endif
