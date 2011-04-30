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
#include "vm.h"


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
            "%s vbm_popen popen failed! errno = %d\n",
            boinc_msg_prefix(buf, sizeof(buf)),
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
    char root_dir[256];

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


// Generate a deterministic yet unique UUID for a given medium (hard drive,
//   cd drive, hard drive image)
// Rules:
//   1. Must be unique
//   2. Must identifity itself as being part of BOINC
//   3. Must be based on the slot directory id
//   4. Must be in the form of a GUID
//      00000000-0000-0000-0000-000000000000
//
// Form/Meaning
//   A        B    C    D    E
//   00000000-0000-0000-0000-000000000000
//
//   A = Drive ID
//   B = Slot ID
//   C = Standalone Flag
//   D = Reserved
//   E = 'BOINC' ASCII converted to Hex
//
int virtualbox_generate_medium_uuid(  int drive_id, std::string& uuid ) {
    APP_INIT_DATA aid;
    char medium_uuid[256];

    boinc_get_init_data_p( &aid );

    sprintf(
        medium_uuid,        
        _T("%08d-%04d-%04d-0000-00424F494E43"),
        drive_id,
        aid.slot,
        boinc_is_standalone()
    );

    uuid = medium_uuid;

    if (!uuid.empty()) {
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


bool virtualbox_vm_is_hdd_uuid_registered() {
    std::string command;
    std::string output;
    std::string virtual_hdd_uuid;

    virtualbox_generate_medium_uuid(0, virtual_hdd_uuid);
    command = "showhdinfo " + virtual_hdd_uuid;

    if (VBOX_SUCCESS == virtualbox_vbm_popen(command, output)) {
        if (output.find("VBOX_E_FILE_ERROR") != std::string::npos) {
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
    std::string virtual_hdd_uuid;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    virtualbox_generate_vm_root_dir(virtual_machine_root_dir);
    virtualbox_generate_medium_uuid(0, virtual_hdd_uuid);

    fprintf(
        stderr,
        "%s Registering virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    // Create and register the VM
    //
    command  = "createvm ";
    command += "--name \"" + virtual_machine_name + "\" ";
    command += "--basefolder \"" + virtual_machine_root_dir + "\" ";
    command += "--settingsfile \"" + virtual_machine_root_dir + "/" + virtual_machine_name + "\" ";
    command += "--ostype \"" + vm.vm_os_name + "\" ";
    command += "--register";
    
    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error registering virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Tweak the VM from it's default configuration
    //
    command  = "modifyvm \"" + virtual_machine_name + "\" ";
    command += "--memory " + vm.vm_memory_size + " ";
    command += "--acpi on ";
    command += "--ioapic on ";
    command += "--boot1 disk ";
    command += "--boot2 none ";
    command += "--boot3 none ";
    command += "--boot4 none ";
    command += "--nic1 nat ";
    command += "--natdnsproxy1 on ";
    command += "--cableconnected1 off ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error modifing virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Add storage controller to VM
    //
    command  = "storagectl \"" + virtual_machine_name + "\" ";
    command += "--name \"IDE Controller\" ";
    command += "--add ide ";
    command += "--controller PIIX4 ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error adding storage controller to virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Adding virtual hard drive to Virtual Box Media Registry
    //
    command  = "openmedium ";
    command += "disk \"" + virtual_machine_root_dir + "/" + vm.vm_disk_image_name + "\" ";
    command += "--uuid " + virtual_hdd_uuid + " ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error adding virtual disk drive to virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Adding virtual hard drive to VM
    //
    command  = "storageattach \"" + virtual_machine_name + "\" ";
    command += "--storagectl \"IDE Controller\" ";
    command += "--port 0 ";
    command += "--device 0 ";
    command += "--type hdd ";
    command += "--medium " + virtual_hdd_uuid + " ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error adding virtual disk drive to virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Enable the network adapter if a network connection is required.
    // NOTE: Network access should never be allowed if the code running in a 
    //   shared directory or the VM itself is NOT signed.  Doing so opens up 
    //   the network behind the firewall to attack.
    //
    if (vm.enable_network) {
        command  = "modifyvm \"" + virtual_machine_name + "\" ";
        command += "--cableconnected1 on ";

        retval = virtualbox_vbm_popen(command, output);
        if (retval) {
            fprintf(
                stderr,
                "%s Error enabling network access for virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                retval,
                command.c_str(),
                output.c_str()
            );
            return retval;
        }
    }


    // Enable the shared folder if a shared folder is specified.
    //
    if (vm.enable_shared_directory) {
        command  = "sharedfolder add \"" + virtual_machine_name + "\" ";
        command += "--name \"" + vm.vm_shared_folder_name + "\" ";
        command += "--hostpath \"" + virtual_machine_root_dir + "/" + vm.vm_shared_folder_dir_name + "\" ";

        retval = virtualbox_vbm_popen(command, output);
        if (retval) {
            fprintf(
                stderr,
                "%s Error enabling shared directory for virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                retval,
                command.c_str(),
                output.c_str()
            );
            return retval;
        }
    }

    return VBOX_SUCCESS;
}


int virtualbox_deregister_vm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;
    std::string virtual_machine_root_dir;
    std::string virtual_hdd_uuid;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    virtualbox_generate_vm_root_dir(virtual_machine_root_dir);
    virtualbox_generate_medium_uuid(0, virtual_hdd_uuid);

    fprintf(
        stderr,
        "%s Deregistering virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    // First step in deregistering a VM is to delete its storage controller
    //
    command  = "storagectl \"" + virtual_machine_name + "\" ";
    command += "--name \"IDE Controller\" ";
    command += "--remove ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error removing storage controller from virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Next delete VM
    //
    command  = "unregistervm \"" + virtual_machine_name + "\" ";
    command += "--delete ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error removing virtual machine from virtual box! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Lastly delete medium from Virtual Box Media Registry
    //
    command  = "closemedium \"" + virtual_hdd_uuid + "\" ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error removing virtual hdd from virtual box! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }

    return VBOX_SUCCESS;
}


int virtualbox_cleanup() {
    return VBOX_SUCCESS;
}


int virtualbox_startvm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "startvm " + virtual_machine_name + " --type headless";
    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error starting virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }
    return VBOX_SUCCESS;
}


int virtualbox_stopvm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm " + virtual_machine_name + " savestate";
    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error stopping virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }
    return VBOX_SUCCESS;
}


int virtualbox_pausevm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm " + virtual_machine_name + " pause";
    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error pausing virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }
    return VBOX_SUCCESS;
}


int virtualbox_resumevm() {
    std::string command;
    std::string output;
    std::string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm " + virtual_machine_name + " resume";
    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error resuming virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }
    return VBOX_SUCCESS;
}


int virtualbox_monitor() {
    return VBOX_SUCCESS;
}

