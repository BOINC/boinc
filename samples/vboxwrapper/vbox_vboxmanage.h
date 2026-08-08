// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2022 University of California
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

// functions for controlling a VM

#ifndef BOINC_VBOX_VBOXMANAGE_H
#define BOINC_VBOX_VBOXMANAGE_H

#include "vbox_common.h"

struct VBOX_VM : VBOX_BASE {
    VBOX_VM();
    ~VBOX_VM();

    int initialize();
    int create_vm();
    int register_vm();
    int deregister_vm(bool delete_media);
    int deregister_stale_vm();
    int poll(bool log_state = true);
    int poll2(bool log_state = true);
    int start();
    int stop();
    int poweroff();
    int pause();
    int resume();
    int capture_screenshot();
    int create_snapshot(double elapsed_time);
    int cleanup_snapshots(bool delete_active);
    int restore_snapshot();
    int run(bool do_restore_snapshot);
    void cleanup();
    void dump_hypervisor_logs(bool include_error_logs);
    void dump_hypervisor_status_reports();

    int is_registered();
    bool is_system_ready(string& message);
    bool is_disk_image_registered();
    bool is_extpack_installed();
    bool is_virtualbox_installed();
    bool is_hostrtc_set_to_utc();

    int get_install_directory(string& dir);
    int get_version_information(string& version_raw, string& version_display);
    int get_guest_additions(string& dir);
    int get_default_network_interface(string& iface);
    int get_vm_network_bytes_sent(double& sent);
    int get_vm_network_bytes_received(double& received);
    int get_vm_process_id();
    int get_vm_exit_code(unsigned long& exit_code);
    double get_vm_cpu_time();

    int set_network_access(bool enabled);
    int set_cpu_usage(int percentage);
    int set_network_usage(int kilobytes);

    void lower_vm_process_priority();
    void reset_vm_process_priority();
    void report_clean(
        bool unrecoverable_error,
        bool skip_cleanup,
        bool do_dump_hypervisor_logs,
        int retval,
        string error_reason,
        int temp_delay,
        string temp_reason,
        double current_cpu_time,
        double last_checkpoint_cpu_time,
        double fraction_done,
        double bytes_sent,
        double bytes_received
    );
#ifdef _WIN32
    int set_race_mitigation_lock(HANDLE& fd_race_mitigator, string& lock_name, const string& medium_file);
    void remove_race_mitigation_lock(HANDLE& fd_race_mitigator, string& lock_name);
#else
    int set_race_mitigation_lock(int& fd_race_mitigator, string& lock_name, const string& medium_file);
    void remove_race_mitigation_lock(int& fd_race_mitigator, string& lock_name);
#endif
    int remove_vbox_disk_orphans(string vbox_disk);
};

#endif
