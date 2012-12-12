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

#ifndef _VM_H_
#define _VM_H_

class VM {
public:
    VM();
    ~VM();

    std::string stdin_filename;
    std::string stdout_filename;
    std::string stderr_filename;
    // name of the OS the VM runs
    std::string vm_os_name;
    // name of the version of the VM the OS runs
    std::string vm_os_version;
    // size of the memory allocation for the VM
    int vm_memory_size;
    // name of the virtual machine disk image file
    std::string vm_disk_image_name;
    // name of the virtual machine disk image type
    std::string vm_disk_image_type;
    // name of shared folder
    std::string vm_shared_folder_name;
    // shared folder directory name
    std::string vm_shared_folder_dir_name;
    bool suspended;
    bool enable_network;
    bool enable_shared_directory;

    int parse( XML_PARSER& );
    void poll();
    int run();
    void stop();
    void pause();
    void resume();
    void cleanup();
    bool is_running();
};

extern VM vm;

#endif
