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

// Return codes
#define     VBOX_SUCCESS                0x00000000
#define     VBOX_POPEN_ERROR            0x00000001
#define     VBOX_PARSER_ERROR           0x00000002

// represents a VirtualBox VM

class VBOX_VM {
public:
    VBOX_VM();
    ~VBOX_VM(){};

    std::string os_name;
        // name of the OS the VM runs
    std::string memory_size_mb;
        // size of the memory allocation for the VM, in megabytes
    std::string image_filename;
        // name of the virtual machine disk image file
    bool suspended;
    bool enable_network;
    bool enable_shared_directory;

    int parse( XML_PARSER& );
    void poll();
    int run();
    int stop();
    int pause();
    int resume();
    void cleanup();
    bool is_running();

    int register_vm();
    bool is_hdd_registered();
    int deregister_stale_vm();
    int deregister_vm();
    int deregister_vm_by_name(std::string&);
    int startvm();

    static int initialize();
    static int generate_vm_root_dir( std::string& dir );
    static int generate_vm_name( std::string& name );
    static bool is_registered();
    static int vbm_popen(std::string&, std::string&);
    static int get_install_directory(std::string&);
};

#endif
