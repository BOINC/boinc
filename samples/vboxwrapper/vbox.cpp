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

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <unistd.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define popen       _popen
#define pclose      _pclose
#endif

#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "boinc_api.h"
#include "vbox.h"


// Execute the vbox manage application and copy the output to the
// designated buffer.
//
int virtualbox_vbm_popen(std::string& arguments, std::string& output) {
    FILE* fp;
    char buf[256];
    std::string command;

    // Initialize command line
    command = "VBoxManage -q " + arguments;

    // Execute command
    fp = popen(command.c_str(), "r");
    if (fp == NULL){
        fprintf(
            stderr,
            "%s vbm_popen popen failed! cmd = '%s', errno = %d\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            command.c_str(),
            errno
        );
        return VBOX_POPEN_ERROR;
    }

    // Copy output to buffer
    while (fgets(buf, 256, fp)) {
        output += buf;
    }

    // Close stream
    pclose(fp);

    return VBOX_SUCCESS;
}


// Returns the current directory in which the executable resides.
//
int virtualbox_generate_vm_root_dir( std::string& dir ) {
    TCHAR root_dir[256];

#ifdef _WIN32
    _getcwd(root_dir, (sizeof(root_dir)*sizeof(TCHAR)));
#else
    getcwd(root_dir, (sizeof(root_dir)*sizeof(TCHAR)));
#endif

    dir = root_dir;

    if (!dir.empty()) {
        return 1;
    }
    return 0;
}


// Generate a unique virtual machine name for a given task instance
// Rules:
//   1. Must be unique
//   2. Must identifity itself as being part of BOINC
//   3. Must be file system compatible
//
int virtualbox_generate_vm_name( std::string& name ) {
    APP_INIT_DATA aid;
    boinc_get_init_data_p( &aid );

    name.empty();
    name = "boinc_";

    if (boinc_is_standalone()) {
        name += "standalone";
    } else {
        name += aid.wu_name;
    }

    if (!name.empty()) {
        return 1;
    }
    return 0;
}


bool virtualbox_vm_is_registered() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "showvminfo " + virtual_machine_name;

    if (VBOX_SUCCESS == virtualbox_vbm_popen(command, output)) {
        if (output.find("VBOX_E_OBJECT_NOT_FOUND") != std::string::npos) {
            return true;
        }
    }

    return false;
}


bool virtualbox_vm_is_running() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "list runningvms";

    if (VBOX_SUCCESS == virtualbox_vbm_popen(command, output)) {
        if (output.find(virtual_machine_name) != std::string::npos) {
            return true;
        }
    }

    return false;
}


int virtualbox_initialize() {
    return VBOX_SUCCESS;
}


int virtualbox_register_vm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;
    std::string virtual_machine_root_dir;

    virtualbox_generate_vm_name(virtual_machine_name);
    virtualbox_generate_vm_root_dir(virtual_machine_root_dir);

    fprintf(
        stderr,
        "%s Registering virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );

    //command = "startvm " + virtual_machine_name + " --type headless";
    //virtualbox_vbm_popen(command, output);

    return VBOX_SUCCESS;
}


int virtualbox_deregister_vm() {
    return VBOX_SUCCESS;
}


int virtualbox_cleanup() {
    return VBOX_SUCCESS;
}


int virtualbox_startvm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "startvm " + virtual_machine_name + " --type headless";
    return virtualbox_vbm_popen(command, output);
}


int virtualbox_stopvm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm " + virtual_machine_name + " savestate";
    return virtualbox_vbm_popen(command, output);
}


int virtualbox_pausevm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm " + virtual_machine_name + " pause";
    return virtualbox_vbm_popen(command, output);
}


int virtualbox_resumevm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm " + virtual_machine_name + " resume";
    return virtualbox_vbm_popen(command, output);
}


int virtualbox_monitor() {
    return VBOX_SUCCESS;
}

