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
    void stop();
    void pause();
    void resume();
    void cleanup();
    bool is_running();
};

extern VM vm;

#endif
