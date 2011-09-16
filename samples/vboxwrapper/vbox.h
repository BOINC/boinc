// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

// represents a VirtualBox VM

struct VBOX_VM {
    VBOX_VM();
    ~VBOX_VM(){};

    // name of the OS the VM runs
    std::string os_name;
    // size of the memory allocation for the VM, in megabytes
    std::string memory_size_mb;
    // name of the virtual machine disk image file
    std::string image_filename;
    // unique name for the VM
    std::string vm_name;
    bool suspended;
    // whether network access is temporarily suspended
    bool network_suspended;
    // whether to allow network access at all
    bool enable_network;
    bool enable_shared_directory;
    // whether we were instructed to only register the VM.
    // useful for debugging VMs.
    bool register_only;

    void poll();
    int run();
    int stop();
    int pause();
    int resume();
    void cleanup();
    bool is_running();

    int register_vm();
    bool is_hdd_registered();
    bool is_registered();
    int deregister_stale_vm();
    int deregister_vm();
    int start();
    int set_network_access(bool enabled);
    int set_cpu_usage_fraction(double);
    int set_network_max_bytes_sec(double);
    int get_process_id(int& process_id);
    int get_network_bytes_sent(double& sent);
    int get_network_bytes_received(double& received);

    static int initialize();
    static int get_install_directory(std::string& dir);
    static int generate_vm_root_dir(std::string& dir);
    static int vbm_popen(
        std::string& command, std::string& output, const char* item
    );
};

#endif
