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
#include "vm.h"

VM vm;

VM::VM() {
    stdin_filename.clear();
    stdout_filename.clear();
    stderr_filename.clear();
    vm_os_name.clear();
    vm_os_version.clear();
    vm_memory_size = 0;
    vm_disk_image_name.clear();
    vm_disk_image_type.clear();
    vm_shared_folder_name.clear();
    vm_shared_folder_dir_name.clear();
    suspended = false;
    enable_network = false;
    enable_shared_directory = false;
}

VM::~VM() {
}

int VM::parse(XML_PARSER& xp) {
    char tag[1024], buf[8192];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "%s VM::parse(): unexpected text %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), tag
            );
            continue;
        }
        if (!strcmp(tag, "/vm")) {
            return 0;
        }
        else if (xp.parse_string(tag, "stdin_filename", stdin_filename)) continue;
        else if (xp.parse_string(tag, "stdout_filename", stdout_filename)) continue;
        else if (xp.parse_string(tag, "stderr_filename", stderr_filename)) continue;
        else if (xp.parse_string(tag, "vm_os_name", vm_os_name)) continue;
        else if (xp.parse_string(tag, "vm_os_version", vm_os_version)) continue;
        else if (xp.parse_int(tag, "vm_memory_size", vm_memory_size)) continue;
        else if (xp.parse_string(tag, "vm_disk_image_name", vm_disk_image_name)) continue;
        else if (xp.parse_string(tag, "vm_disk_image_type", vm_disk_image_type)) continue;
        else if (xp.parse_string(tag, "vm_shared_folder_name", vm_shared_folder_name)) continue;
        else if (xp.parse_string(tag, "vm_shared_folder_dir_name", vm_shared_folder_dir_name)) continue;
        else if (xp.parse_bool(tag, "enable_network", enable_network)) continue;
        else if (xp.parse_bool(tag, "enable_shared_directory", enable_shared_directory)) continue;
    }
    return ERR_XML_PARSE;
}

int VM::run() {
    int retval;

    retval = virtualbox_initialize();
    if (retval) return retval;

    if (!virtualbox_vm_is_registered()) {
        retval = virtualbox_register_vm();
        if (retval) return retval;
    }

    retval = virtualbox_startvm();
    if (retval) return retval;

    return 0;
}

void VM::poll() {
    virtualbox_monitor();
}

void VM::stop() {
    virtualbox_stopvm();
    virtualbox_cleanup();
}

void VM::pause() {
    virtualbox_pausevm();
    suspended = true;
}

void VM::resume() {
    virtualbox_resumevm();
    suspended = false;
}

void VM::cleanup() {
    virtualbox_stopvm();
    virtualbox_deregister_vm();
    virtualbox_cleanup();

    // Give time enough for external processes to finish the cleanup process
    boinc_sleep(5.0);
}

bool VM::is_running() {
    return (bool)virtualbox_vm_is_running();
}
