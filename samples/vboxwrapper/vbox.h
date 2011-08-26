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

    std::string os_name;
        // name of the OS the VM runs
    std::string memory_size_mb;
        // size of the memory allocation for the VM, in megabytes
    std::string image_filename;
        // name of the virtual machine disk image file
    std::string vm_name;
        // unique name for the VM
    bool suspended;
    bool enable_network;
    bool enable_shared_directory;

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
    int startvm();
    int set_network_access(bool enabled);
    int set_cpu_throttle(int throttle_speed);
    int set_network_throttle(int throttle_speed);

    static int initialize();
    static int generate_vm_root_dir( std::string& dir );
    static int vbm_popen(std::string&, std::string&);
    static int get_install_directory(std::string&);
};

#endif
