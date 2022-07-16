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


// Provide cross-platform interfaces for making changes to VirtualBox

#ifndef BOINC_VBOX_VBOXMANAGE_H
#define BOINC_VBOX_VBOXMANAGE_H

namespace vboxmanage {

    class VBOX_VM : public VBOX_BASE {
    public:
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

        void dump_hypervisor_status_reports();

        int is_registered();
        bool is_system_ready(std::string& message);
        bool is_disk_image_registered();
        bool is_extpack_installed();
        bool is_virtualbox_installed();
        bool is_hostrtc_set_to_utc();

        int get_install_directory(std::string& dir);
        int get_version_information(std::string& version_raw, std::string& version_display);
        int get_guest_additions(std::string& dir);
        int get_default_network_interface(std::string& iface);
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
    };

};

#endif
