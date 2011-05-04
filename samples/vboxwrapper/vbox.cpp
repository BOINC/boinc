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
#include <stdlib.h>
#include <vector>
#include <string>
#include <unistd.h>
#endif

using std::string;

#if defined(_MSC_VER) || defined(__MINGW32__)
#define popen       _popen
#define pclose      _pclose
#define getcwd      _getcwd
#define putenv      _putenv
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
int virtualbox_vbm_popen(string& arguments, string& output) {
    FILE* fp;
    char buf[256];
    string command;

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
int virtualbox_generate_vm_root_dir( string& dir ) {
    char root_dir[256];

    getcwd(root_dir, (sizeof(root_dir)*sizeof(char)));
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
int virtualbox_generate_vm_name( string& name ) {
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
    string command;
    string output;
    string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "showvminfo " + virtual_machine_name;

    if (VBOX_SUCCESS == virtualbox_vbm_popen(command, output)) {
        if (output.find("VBOX_E_OBJECT_NOT_FOUND") != string::npos) {
            return true;
        }
    }
    return false;
}


bool virtualbox_vm_is_hdd_registered() {
    string command;
    string output;
    string virtual_machine_root_dir;

    virtualbox_generate_vm_root_dir(virtual_machine_root_dir);

    command = "showhdinfo \"" + virtual_machine_root_dir + "/" + vm.vm_disk_image_name + "\" ";

    if (VBOX_SUCCESS == virtualbox_vbm_popen(command, output)) {
        if (output.find("VBOX_E_FILE_ERROR") != string::npos) {
            return true;
        }
    }
    return false;
}


bool virtualbox_vm_is_running() {
    string command;
    string output;
    string virtual_machine_name;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "list runningvms";

    if (VBOX_SUCCESS == virtualbox_vbm_popen(command, output)) {
        if (output.find(virtual_machine_name) != string::npos) {
            return true;
        }
    }

    return false;
}

int virtualbox_get_install_directory( string& virtualbox_install_directory ) {
#ifdef _WIN32
    LONG    lReturnValue;
    HKEY    hkSetupHive;
    LPTSTR  lpszRegistryValue = NULL;
    DWORD   dwSize = 0;

    // change the current directory to the boinc data directory if it exists
    lReturnValue = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, 
        _T("SOFTWARE\\Oracle\\VirtualBox"),  
        0, 
        KEY_READ,
        &hkSetupHive
    );
    if (lReturnValue == ERROR_SUCCESS) {
        // How large does our buffer need to be?
        lReturnValue = RegQueryValueEx(
            hkSetupHive,
            _T("InstallDir"),
            NULL,
            NULL,
            NULL,
            &dwSize
        );
        if (lReturnValue != ERROR_FILE_NOT_FOUND) {
            // Allocate the buffer space.
            lpszRegistryValue = (LPTSTR) malloc(dwSize);
            (*lpszRegistryValue) = NULL;

            // Now get the data
            lReturnValue = RegQueryValueEx( 
                hkSetupHive,
                _T("InstallDir"),
                NULL,
                NULL,
                (LPBYTE)lpszRegistryValue,
                &dwSize
            );

            virtualbox_install_directory = lpszRegistryValue;
        }
    }

    if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszRegistryValue) free(lpszRegistryValue);
#endif
    return VBOX_SUCCESS;
}


int virtualbox_initialize() {
    string virtualbox_install_directory;
    string old_path;
    string new_path;
    char buf[256];

    virtualbox_get_install_directory(virtualbox_install_directory);

    // Prep the environment so we can execute the vboxmanage application
    if (!virtualbox_install_directory.empty()) {
        old_path = getenv("path");

        new_path = "path=";

        new_path += virtualbox_install_directory;

        // Path environment variable seperator
#ifdef _WIN32
        new_path += ";";
#else
        new_path += ":";
#endif

        new_path += old_path;

        if (putenv((char*)new_path.c_str())) {
            fprintf(
                stderr,
                "%s Failed to modify the search path.\n",
                boinc_msg_prefix(buf, sizeof(buf))
            );
        }
    }

    return VBOX_SUCCESS;
}


int virtualbox_register_vm() {
    string command;
    string output;
    string virtual_machine_name;
    string virtual_machine_root_dir;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    virtualbox_generate_vm_root_dir(virtual_machine_root_dir);

    fprintf(
        stderr,
        "%s Registering virtual machine. (%s) \n",
        boinc_msg_prefix(buf, sizeof(buf)),
        virtual_machine_name.c_str()
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


    // Adding virtual hard drive to VM
    //
    command  = "storageattach \"" + virtual_machine_name + "\" ";
    command += "--storagectl \"IDE Controller\" ";
    command += "--port 0 ";
    command += "--device 0 ";
    command += "--type hdd ";
    command += "--medium \"" + virtual_machine_root_dir + "/" + vm.vm_disk_image_name + "\" ";

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


int virtualbox_deregister_vm_by_name( string virtual_machine_name ) {
    string command;
    string output;
    string virtual_machine_root_dir;
    char buf[256];
    int retval;

    virtualbox_generate_vm_root_dir(virtual_machine_root_dir);

    fprintf(
        stderr,
        "%s Deregistering virtual machine. (%s)\n",
        boinc_msg_prefix(buf, sizeof(buf)),
        virtual_machine_name.c_str()
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
    command  = "closemedium \"" + virtual_machine_root_dir + "/" + vm.vm_disk_image_name + "\" ";

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


int virtualbox_deregister_stale_vm() {
    string command;
    string output;
    string virtual_machine_root_dir;
    string virtual_machine_name;
    size_t uuid_location;
    size_t uuid_length;
    char buf[256];
    int retval;

    virtualbox_generate_vm_root_dir(virtual_machine_root_dir);

    // We need to determine what the name or uuid is of the previous VM which owns
    // this virtual disk
    //
    command  = "showhdinfo \"" + virtual_machine_root_dir + "/" + vm.vm_disk_image_name + "\" ";

    retval = virtualbox_vbm_popen(command, output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error retrieving virtual hard disk information from virtual box! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            command.c_str(),
            output.c_str()
        );
        return retval;
    }


    // Output should look a little like this:
    //   UUID:                 c119acaf-636c-41f6-86c9-38e639a31339
    //   Accessible:           yes
    //   Logical size:         10240 MBytes
    //   Current size on disk: 0 MBytes
    //   Type:                 normal (base)
    //   Storage format:       VDI
    //   Format variant:       dynamic default
    //   In use by VMs:        test2 (UUID: 000ab2be-1254-4c6a-9fdc-1536a478f601)
    //   Location:             C:\Users\romw\VirtualBox VMs\test2\test2.vdi
    //
    uuid_location = output.find("(UUID: ");
    if (uuid_location != string::npos) {
        // We can parse the virtual machine ID from the output

        uuid_location += 7;
        uuid_length = output.find(")", uuid_location);

        virtual_machine_name = output.substr(uuid_location, uuid_length);

        // Deregister stale VM by UUID
        return virtualbox_deregister_vm_by_name(virtual_machine_name);
    } else {
        // Did the user delete the VM in VirtualBox and not the medium?  If so,
        // just remove the medium.
        command  = "closemedium \"" + virtual_machine_root_dir + "/" + vm.vm_disk_image_name + "\" ";

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
    }

    return VBOX_SUCCESS;
}


int virtualbox_deregister_vm() {
    string virtual_machine_name;
    virtualbox_generate_vm_name(virtual_machine_name);
    return virtualbox_deregister_vm_by_name(virtual_machine_name);
}


int virtualbox_cleanup() {
    return VBOX_SUCCESS;
}


int virtualbox_startvm() {
    string command;
    string output;
    string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "startvm \"" + virtual_machine_name + "\" --type headless";
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
    string command;
    string output;
    string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm \"" + virtual_machine_name + "\" savestate";
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
    string command;
    string output;
    string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm \"" + virtual_machine_name + "\" pause";
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
    string command;
    string output;
    string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);
    command = "controlvm \"" + virtual_machine_name + "\" resume";
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


// Minium version for this feature to work correctly in BOINC related environments is
// 4.0.6. See bug: http://www.virtualbox.org/ticket/7872
//
int virtualbox_execute_task( string& command, string arguments ) {
    string popen_command;
    string popen_output;
    string virtual_machine_name;
    char buf[256];
    int retval;

    virtualbox_generate_vm_name(virtual_machine_name);

    popen_command = "guestcontrol exec \"" + virtual_machine_name + "\" ";
    popen_command += "\"" + command + "\" ";
    popen_command += "--username \"" + vm.vm_task_execution_username + "\" ";
    popen_command += "--password \"" + vm.vm_task_execution_password + "\" ";
    popen_command += "--arguments \"" + arguments + "\" ";
    popen_command += "--wait-for exit ";

    retval = virtualbox_vbm_popen(popen_command, popen_output);
    if (retval) {
        fprintf(
            stderr,
            "%s Error executing task in virtual machine! rc = 0x%x\nCommand:\n%s\nOutput:\n%s\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval,
            popen_command.c_str(),
            popen_output.c_str()
        );
        return retval;
    }
    return VBOX_SUCCESS;
}


int virtualbox_monitor() {
    return VBOX_SUCCESS;
}

