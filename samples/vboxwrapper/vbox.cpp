// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#endif

using std::string;

#if defined(_MSC_VER)
#define getcwd      _getcwd
#define stricmp     _stricmp
#endif

#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "network.h"
#include "boinc_api.h"
#include "floppyio.h"
#include "vboxwrapper.h"
#include "vbox.h"

//#define NEW_EXECUTION_PATH 1

VBOX_VM::VBOX_VM() {
    virtualbox_home_directory.clear();
    virtualbox_install_directory.clear();
    virtualbox_guest_additions.clear();
    virtualbox_version.clear();
    pFloppy = NULL;
    vm_log.clear();
    vm_log_timestamp.hours = 0;
    vm_log_timestamp.minutes = 0;
    vm_log_timestamp.seconds = 0;
    vm_log_timestamp.milliseconds = 0;
    vm_master_name.clear();
    vm_master_description.clear();
    vm_name.clear();
    vm_cpu_count.clear();
    vm_disk_controller_type.clear();
    vm_disk_controller_model.clear();
    os_name.clear();
    memory_size_mb.clear();
    image_filename.clear();
    iso_image_filename.clear();
    cache_disk_filename.clear();
    floppy_image_filename.clear();
    job_duration = 0.0;
    current_cpu_time = 0.0;
    minimum_checkpoint_interval = 600.0;
    fraction_done_filename.clear();
    suspended = false;
    network_suspended = false;
    online = false;
    saving = false;
    restoring = false;
    crashed = false;
    enable_cern_dataformat = false;
    enable_shared_directory = false;
    enable_floppyio = false;
    enable_cache_disk = false;
    enable_isocontextualization = false;
    enable_remotedesktop = false;
    register_only = false;
    enable_network = false;
    pf_guest_port = 0;
    pf_host_port = 0;
    headless = true;
    vm_pid = 0;
    vboxsvc_pid = 0;
#ifdef _WIN32
    vm_pid_handle = 0;
    vboxsvc_pid_handle = 0;
#endif

    // Initialize default values
    vm_disk_controller_type = "ide";
    vm_disk_controller_model = "PIIX4";
}

VBOX_VM::~VBOX_VM() {
    if (pFloppy) {
        delete pFloppy;
        pFloppy = NULL;
    }
#ifdef _WIN32
    if (vm_pid_handle) {
        CloseHandle(vm_pid_handle);
        vm_pid_handle = NULL;
    }
    if (vboxsvc_pid_handle) {
        CloseHandle(vboxsvc_pid_handle);
        vboxsvc_pid_handle = NULL;
    }
#endif
}

int VBOX_VM::initialize() {
    int rc = 0;
    string old_path;
    string new_path;
    string command;
    string output;
    APP_INIT_DATA aid;
    bool force_sandbox = false;
    char buf[256];

    boinc_get_init_data_p(&aid);
    get_install_directory(virtualbox_install_directory);

    // Prep the environment so we can execute the vboxmanage application
    //
    // TODO: Fix for non-Windows environments if we ever find another platform
    // where vboxmanage is not already in the search path
#ifdef _WIN32
    if (!virtualbox_install_directory.empty())
    {
        old_path = getenv("PATH");
        new_path = virtualbox_install_directory + ";" + old_path;

        if (!SetEnvironmentVariable("PATH", const_cast<char*>(new_path.c_str()))) {
            fprintf(
                stderr,
                "%s Failed to modify the search path.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        }
    }
#endif

    // Determine the VirtualBox home directory.  Overwrite as needed.
    //
    if (getenv("VBOX_USER_HOME")) {
        virtualbox_home_directory = getenv("VBOX_USER_HOME");
    } else {
        // If the override environment variable isn't specified then
        // it is based of the current users HOME directory.
#ifdef _WIN32
        virtualbox_home_directory = getenv("USERPROFILE");
#else
        virtualbox_home_directory = getenv("HOME");
#endif
        virtualbox_home_directory += "/.VirtualBox";
    }

    // On *nix style systems, VirtualBox expects that there is a home directory specified
    // by environment variable.  When it doesn't exist it attempts to store logging information
    // in root's home directory.  Bad things happen if the process isn't owned by root.
    //
    // if the HOME environment variable is missing force VirtualBox to use a directory it
    // has a reasonable chance of writing log files too.
#ifndef _WIN32
    if (NULL == getenv("HOME")) {
        force_sandbox = true;
    }
#endif

    // Set the location in which the VirtualBox Configuration files can be
    // stored for this instance.
    if (aid.using_sandbox || force_sandbox) {
        virtualbox_home_directory = aid.project_dir;
        virtualbox_home_directory += "/../virtualbox";

        if (!boinc_file_exists(virtualbox_home_directory.c_str())) boinc_mkdir(virtualbox_home_directory.c_str());

#ifdef _WIN32
        if (!SetEnvironmentVariable("VBOX_USER_HOME", const_cast<char*>(virtualbox_home_directory.c_str()))) {
            fprintf(
                stderr,
                "%s Failed to modify the search path.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        }
#else
        // putenv does not copy its input buffer, so we must use setenv
        if (setenv("VBOX_USER_HOME", const_cast<char*>(virtualbox_home_directory.c_str()), 1)) {
            fprintf(
                stderr,
                "%s Failed to modify the VBOX_USER_HOME path.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        }
#endif
    }

    rc = get_version_information(virtualbox_version);
    if (rc) return rc;

    rc = get_guest_additions(virtualbox_guest_additions);
    if (rc) return rc;

    return 0;
}

void VBOX_VM::poll(bool log_state) {
    char buf[256];
    APP_INIT_DATA aid;
    string command;
    string output;
    string::iterator iter;
    string vmstate;
    static string vmstate_old = "poweroff";
    size_t vmstate_start;
    size_t vmstate_end;

    boinc_get_init_data_p(&aid);

    //
    // Is our environment still sane?
    //
#ifdef _WIN32
    if (aid.using_sandbox && vboxsvc_pid_handle && !process_exists(vboxsvc_pid_handle)) {
        fprintf(
            stderr,
            "%s Status Report: vboxsvc.exe is no longer running.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
    if (vm_pid_handle && !process_exists(vm_pid_handle)) {
        fprintf(
            stderr,
            "%s Status Report: virtualbox.exe/vboxheadless.exe is no longer running.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
#else
    if (vm_pid && !process_exists(vm_pid)) {
        fprintf(
            stderr,
            "%s Status Report: virtualbox/vboxheadless is no longer running.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
#endif

    //
    // What state is the VM in?
    //

    command  = "showvminfo \"" + vm_name + "\" ";
    command += "--machinereadable ";

    if (vbm_popen(command, output, "VM state", false, false, 45, false) == 0) {
        vmstate_start = output.find("VMState=\"");
        if (vmstate_start != string::npos) {
            vmstate_start += 9;
            vmstate_end = output.find("\"", vmstate_start);
            vmstate = output.substr(vmstate_start, vmstate_end - vmstate_start);

            // VirtualBox Documentation suggests that that a VM is running when its
            // machine state is between MachineState_FirstOnline and MachineState_LastOnline
            // which as of this writing is 5 and 17.
            //
            // VboxManage's source shows more than that though:
            // see: http://www.virtualbox.org/browser/trunk/src/VBox/Frontends/VBoxManage/VBoxManageInfo.cpp
            //
            // So for now, go with what VboxManage is reporting.
            //
            if (vmstate == "running") {
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
            } else if (vmstate == "paused") {
                online = true;
                saving = false;
                restoring = false;
                suspended = true;
                crashed = false;
            } else if (vmstate == "starting") {
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
            } else if (vmstate == "stopping") {
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
            } else if (vmstate == "saving") {
                online = true;
                saving = true;
                restoring = false;
                suspended = false;
                crashed = false;
            } else if (vmstate == "restoring") {
                online = true;
                saving = false;
                restoring = true;
                suspended = false;
                crashed = false;
            } else if (vmstate == "livesnapshotting") {
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
            } else if (vmstate == "deletingsnapshotlive") {
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
            } else if (vmstate == "deletingsnapshotlivepaused") {
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
            } else if (vmstate == "aborted") {
                online = false;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = true;
            } else if (vmstate == "gurumeditation") {
                online = false;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = true;
            } else {
                online = false;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                if (log_state) {
                    fprintf(
                        stderr,
                        "%s VM is no longer is a running state. It is in '%s'.\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        vmstate.c_str()
                    );
                }
            }
            if (log_state && (vmstate_old != vmstate)) {
                fprintf(
                    stderr,
                    "%s VM state change detected. (old = '%s', new = '%s')\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    vmstate_old.c_str(),
                    vmstate.c_str()
                );
                vmstate_old = vmstate;
            }
        }
    }

    //
    // Grab a snapshot of the latest log file.  Avoids multiple queries across several
    // functions.
    //
    get_vm_log(vm_log);

    //
    // Dump any new VM Guest Log entries
    //
    dump_vmguestlog_entries();
}

int VBOX_VM::create_vm() {
    string command;
    string output;
    string virtual_machine_slot_directory;
    APP_INIT_DATA aid;
    bool disable_acceleration = false;
    char buf[256];
    int retval;

    boinc_get_init_data_p(&aid);
    get_slot_directory(virtual_machine_slot_directory);


    // Reset VM name in case it was changed while deregistering a stale VM
    //
    vm_name = vm_master_name;


    fprintf(
        stderr,
        "%s Create VM. (%s, slot#%d) \n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        vm_name.c_str(),
        aid.slot
    );

    // Fixup chipset and drive controller information for known configurations
    //
    if (enable_isocontextualization) {
        if ("PIIX4" == vm_disk_controller_model) {
            fprintf(
                stderr,
                "%s Updating drive controller type and model for desired configuration.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            vm_disk_controller_type = "sata";
            vm_disk_controller_model = "AHCI";
        }
    }

    // Create and register the VM
    //
    command  = "createvm ";
    command += "--name \"" + vm_name + "\" ";
    command += "--basefolder \"" + virtual_machine_slot_directory + "\" ";
    command += "--ostype \"" + os_name + "\" ";
    command += "--register";
    
    retval = vbm_popen(command, output, "create");
    if (retval) return retval;

    // Tweak the VM's Description
    //
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--description \"" + vm_master_description + "\" ";

    vbm_popen(command, output, "modifydescription", false, false);

    // Tweak the VM's CPU Count
    //
    fprintf(
        stderr,
        "%s Setting CPU Count for VM. (%s)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        vm_cpu_count.c_str()
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--cpus " + vm_cpu_count + " ";

    retval = vbm_popen(command, output, "modifycpu");
    if (retval) return retval;

    // Tweak the VM's Memory Size
    //
    fprintf(
        stderr,
        "%s Setting Memory Size for VM. (%sMB)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        memory_size_mb.c_str()
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--memory " + memory_size_mb + " ";

    retval = vbm_popen(command, output, "modifymem");
    if (retval) return retval;

    // Tweak the VM's Chipset Options
    //
    fprintf(
        stderr,
        "%s Setting Chipset Options for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--acpi on ";
    command += "--ioapic on ";

    retval = vbm_popen(command, output, "modifychipset");
    if (retval) return retval;

    // Tweak the VM's Boot Options
    //
    fprintf(
        stderr,
        "%s Setting Boot Options for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--boot1 disk ";
    command += "--boot2 dvd ";
    command += "--boot3 none ";
    command += "--boot4 none ";

    retval = vbm_popen(command, output, "modifyboot");
    if (retval) return retval;

    // Tweak the VM's Network Configuration
    //
    fprintf(
        stderr,
        "%s Setting Network Configuration for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--nic1 nat ";
    command += "--natdnsproxy1 on ";
    command += "--cableconnected1 off ";

    retval = vbm_popen(command, output, "modifynetwork");
    if (retval) return retval;

    // Tweak the VM's USB Configuration
    //
    fprintf(
        stderr,
        "%s Disabling USB Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--usb off ";

    vbm_popen(command, output, "modifyusb", false, false);

    // Tweak the VM's COM Port Support
    //
    fprintf(
        stderr,
        "%s Disabling COM Port Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--uart1 off ";
    command += "--uart2 off ";

    vbm_popen(command, output, "modifycom", false, false);

    // Tweak the VM's LPT Port Support
    //
    fprintf(
        stderr,
        "%s Disabling LPT Port Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--lpt1 off ";
    command += "--lpt2 off ";

    vbm_popen(command, output, "modifylpt", false, false);

    // Tweak the VM's Audio Support
    //
    fprintf(
        stderr,
        "%s Disabling Audio Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--audio none ";

    vbm_popen(command, output, "modifyaudio", false, false);

    // Tweak the VM's Clipboard Support
    //
    fprintf(
        stderr,
        "%s Disabling Clipboard Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--clipboard disabled ";

    vbm_popen(command, output, "modifyclipboard", false, false);

    // Tweak the VM's Drag & Drop Support
    //
    fprintf(
        stderr,
        "%s Disabling Drag and Drop Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--draganddrop disabled ";

    vbm_popen(command, output, "modifydragdrop", false, false);

    // Check to see if the processor supports hardware acceleration for virtualization
    // If it doesn't, disable the use of it in VirtualBox. Multi-core jobs require hardware
    // acceleration and actually override this setting.
    //
    if (!strstr(aid.host_info.p_features, "vmx") && !strstr(aid.host_info.p_features, "svm")) {
        fprintf(
            stderr,
            "%s Hardware acceleration CPU extensions not detected. Disabling VirtualBox hardware acceleration support.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        disable_acceleration = true;
    }
    if (strstr(aid.host_info.p_features, "hypervisor")) {
        fprintf(
            stderr,
            "%s Running under Hypervisor. Disabling VirtualBox hardware acceleration support.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        disable_acceleration = true;
    }
    if (is_boinc_client_version_newer(aid, 7, 2, 16)) {
        if (aid.vm_extensions_disabled) {
            fprintf(
                stderr,
                "%s Hardware acceleration failed with previous execution. Disabling VirtualBox hardware acceleration support.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            disable_acceleration = true;
        }
    } else {
        if (vm_cpu_count == "1") {
            // Keep this around for older clients.  Removing this for older clients might
            // lead to a machine that will only return crashed VM reports.
            vboxwrapper_msg_prefix(buf, sizeof(buf));
            fprintf(
                stderr,
                "%s Legacy fallback configuration detected. Disabling VirtualBox hardware acceleration support.\n"
                "%s NOTE: Upgrading to BOINC 7.2.16 or better may re-enable hardware acceleration.\n",
                buf,
                buf
            );
            disable_acceleration = true;
        }
    }

    // Only allow disabling of hardware acceleration on 32-bit VM types, 64-bit VM types require it.
    //
    if (os_name.find("_64") == std::string::npos) {
        if (disable_acceleration) {
            fprintf(
                stderr,
                "%s Disabling hardware acceleration support for virtualization.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            command  = "modifyvm \"" + vm_name + "\" ";
            command += "--hwvirtex off ";

            retval = vbm_popen(command, output, "VT-x/AMD-V support");
            if (retval) return retval;
        }
    } else if (os_name.find("_64") != std::string::npos) {
        if (disable_acceleration) {
            fprintf(
                stderr,
                "%s ERROR: Invalid configuration.  VM type requires acceleration but the current configuration cannot support it.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            return ERR_INVALID_PARAM;
        }
    }

    // Add storage controller to VM
    // See: http://www.virtualbox.org/manual/ch08.html#vboxmanage-storagectl
    // See: http://www.virtualbox.org/manual/ch05.html#iocaching
    //
    fprintf(
        stderr,
        "%s Adding storage controller to VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "storagectl \"" + vm_name + "\" ";
    command += "--name \"Hard Disk Controller\" ";
    command += "--add \"" + vm_disk_controller_type + "\" ";
    if (is_virtualbox_version_newer(4, 3, 0)) {
        command += "--controller \"" + vm_disk_controller_model + "\" ";
    } else {
        if ((vm_disk_controller_type == "sata") || (vm_disk_controller_type == "SATA")) {
            command += "--controller \"IntelAHCI\" ";
        } else {
            command += "--controller \"" + vm_disk_controller_model + "\" ";
        }
    }
    command += "--hostiocache off ";
    if ((vm_disk_controller_type == "sata") || (vm_disk_controller_type == "SATA")) {
        if (is_virtualbox_version_newer(4, 3, 0)) {
            command += "--portcount 3";
        } else {
            command += "--sataportcount 3";
        }
    }

    retval = vbm_popen(command, output, "add storage controller (fixed disk)");
    if (retval) return retval;

    // Add storage controller for a floppy device if desired
    //
    if (enable_floppyio) {
        command  = "storagectl \"" + vm_name + "\" ";
        command += "--name \"Floppy Controller\" ";
        command += "--add floppy ";

        retval = vbm_popen(command, output, "add storage controller (floppy)");
        if (retval) return retval;
    }

    if (enable_isocontextualization) {

        // Add virtual ISO 9660 disk drive to VM
        //
        fprintf(
            stderr,
            "%s Adding virtual ISO 9660 disk drive to VM. (%s)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            iso_image_filename.c_str()
        );
        command  = "storageattach \"" + vm_name + "\" ";
        command += "--storagectl \"Hard Disk Controller\" ";
        command += "--port 0 ";
        command += "--device 0 ";
        command += "--type dvddrive ";
        command += "--medium \"" + virtual_machine_slot_directory + "/" + iso_image_filename + "\" ";

        retval = vbm_popen(command, output, "storage attach (ISO 9660 image)");
        if (retval) return retval;

        // Add guest additions to the VM
        //
        fprintf(
            stderr,
            "%s Adding VirtualBox Guest Additions to VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "storageattach \"" + vm_name + "\" ";
        command += "--storagectl \"Hard Disk Controller\" ";
        command += "--port 2 ";
        command += "--device 0 ";
        command += "--type dvddrive ";
        command += "--medium \"" + virtualbox_guest_additions + "\" ";

        retval = vbm_popen(command, output, "storage attach (guest additions image)");
        if (retval) return retval;

        // Add a virtual cache disk drive to VM
        //
        if (enable_cache_disk){
            fprintf(
                stderr,
                "%s Adding virtual cache disk drive to VM. (%s)\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
    		    cache_disk_filename.c_str()
            );
            command  = "storageattach \"" + vm_name + "\" ";
            command += "--storagectl \"Hard Disk Controller\" ";
            command += "--port 1 ";
            command += "--device 0 ";
            command += "--type hdd ";
            command += "--setuuid \"\" ";
            command += "--medium \"" + virtual_machine_slot_directory + "/" + cache_disk_filename + "\" ";

            retval = vbm_popen(command, output, "storage attach (cached disk)");
            if (retval) return retval;
        }

    } else {

        // Adding virtual hard drive to VM
        //
        fprintf(
            stderr,
            "%s Adding virtual disk drive to VM. (%s)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
		    image_filename.c_str()
        );
        command  = "storageattach \"" + vm_name + "\" ";
        command += "--storagectl \"Hard Disk Controller\" ";
        command += "--port 0 ";
        command += "--device 0 ";
        command += "--type hdd ";
        command += "--setuuid \"\" ";
        command += "--medium \"" + virtual_machine_slot_directory + "/" + image_filename + "\" ";

         retval = vbm_popen(command, output, "storage attach (fixed disk)");
        if (retval) return retval;

    }


    // Add network bandwidth throttle group
    //
    if (is_virtualbox_version_newer(4, 2, 0)) {
        fprintf(
            stderr,
            "%s Adding network bandwidth throttle group to VM. (Defaulting to 1024GB)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "bandwidthctl \"" + vm_name + "\" ";
        command += "add \"" + vm_name + "_net\" ";
        command += "--type network ";
        command += "--limit 1024G";
        command += " ";

        retval = vbm_popen(command, output, "network throttle group (add)");
        if (retval) return retval;
    }

    // Adding virtual floppy disk drive to VM
    //
    if (enable_floppyio) {

        // Put in place the FloppyIO abstraction
        //
        // NOTE: This creates the floppy.img file at runtime for use by the VM.
        //
        pFloppy = new FloppyIO(floppy_image_filename.c_str());
        if (!pFloppy->ready()) {
            vboxwrapper_msg_prefix(buf, sizeof(buf));
            fprintf(
                stderr,
                "%s Creating virtual floppy image failed.\n"
                "%s Error Code '%d' Error Message '%s'\n",
                buf,
                buf,
                pFloppy->error,
                pFloppy->errorStr.c_str()
            );
            return ERR_FWRITE;
        }

        fprintf(
            stderr,
            "%s Adding virtual floppy disk drive to VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "storageattach \"" + vm_name + "\" ";
        command += "--storagectl \"Floppy Controller\" ";
        command += "--port 0 ";
        command += "--device 0 ";
        command += "--medium \"" + virtual_machine_slot_directory + "/" + floppy_image_filename + "\" ";

        retval = vbm_popen(command, output, "storage attach (floppy disk)");
        if (retval) return retval;

    }

    // Enable the network adapter if a network connection is required.
    //
    if (enable_network) {
        set_network_access(true);

        // set up port forwarding
        //
        if (pf_guest_port) {
            PORT_FORWARD pf;
            pf.guest_port = pf_guest_port;
            pf.host_port = pf_host_port;
            if (!pf_host_port) {
                retval = boinc_get_port(false, pf.host_port);
                if (retval) return retval;
                pf_host_port = pf.host_port;
            }
            port_forwards.push_back(pf);
        }
        for (unsigned int i=0; i<port_forwards.size(); i++) {
            PORT_FORWARD& pf = port_forwards[i];
            fprintf(
                stderr,
                "%s forwarding host port %d to guest port %d\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                pf.host_port, pf.guest_port
            );

            // Add new firewall rule
            //
            sprintf(buf, ",tcp,%s,%d,,%d",
                pf.is_remote?"":"127.0.0.1",
                pf.host_port, pf.guest_port
            );
            command  = "modifyvm \"" + vm_name + "\" ";
            command += "--natpf1 \"" + string(buf) + "\" ";

            retval = vbm_popen(command, output, "add updated port forwarding rule");
            if (retval) return retval;
        }
    }

    // If the VM wants to enable remote desktop for the VM do it here
    //
    if (enable_remotedesktop) {
        fprintf(
            stderr,
            "%s Enabling remote desktop for VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        if (!is_extpack_installed()) {
            fprintf(
                stderr,
                "%s Required extension pack not installed, remote desktop not enabled.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        } else {
            retval = boinc_get_port(false, rd_host_port);
            if (retval) return retval;

            sprintf(buf, "%d", rd_host_port);
            command  = "modifyvm \"" + vm_name + "\" ";
            command += "--vrde on ";
            command += "--vrdeextpack default ";
            command += "--vrdeauthlibrary default ";
            command += "--vrdeauthtype null ";
            command += "--vrdeport " + string(buf) + " ";

            retval = vbm_popen(command, output, "remote desktop");
            if(retval) return retval;
        }
    }

    // Enable the shared folder if a shared folder is specified.
    //
    if (enable_shared_directory) {
        fprintf(
            stderr,
            "%s Enabling shared directory for VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "sharedfolder add \"" + vm_name + "\" ";
        command += "--name \"shared\" ";
        command += "--hostpath \"" + virtual_machine_slot_directory + "/shared\"";

        retval = vbm_popen(command, output, "enable shared dir");
        if (retval) return retval;
    }

    return 0;
}

int VBOX_VM::register_vm() {
    string command;
    string output;
    string virtual_machine_slot_directory;
    APP_INIT_DATA aid;
    char buf[256];
    int retval;

    boinc_get_init_data_p(&aid);
    get_slot_directory(virtual_machine_slot_directory);


    // Reset VM name in case it was changed while deregistering a stale VM
    //
    vm_name = vm_master_name;


    fprintf(
        stderr,
        "%s Register VM. (%s, slot#%d) \n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        vm_name.c_str(),
        aid.slot
    );


    // Register the VM
    //
    command  = "registervm ";
    command += "\"" + virtual_machine_slot_directory + "/" + vm_name + "/" + vm_name + ".vbox\" ";
    
    retval = vbm_popen(command, output, "register");
    if (retval) return retval;

    return 0;
}

int VBOX_VM::deregister_vm(bool delete_media) {
    string command;
    string output;
    string virtual_machine_slot_directory;
    char buf[256];

    get_slot_directory(virtual_machine_slot_directory);

    fprintf(
        stderr,
        "%s Deregistering VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );


    // Cleanup any left-over snapshots
    //
    cleanup_snapshots(true);

    // Delete network bandwidth throttle group
    //
    if (is_virtualbox_version_newer(4, 2, 0)) {
        fprintf(
            stderr,
            "%s Removing network bandwidth throttle group from VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "bandwidthctl \"" + vm_name + "\" ";
        command += "remove \"" + vm_name + "_net\" ";

        vbm_popen(command, output, "network throttle group (remove)", false, false);
    }

    // Delete its storage controller(s)
    //
    fprintf(
        stderr,
        "%s Removing storage controller(s) from VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "storagectl \"" + vm_name + "\" ";
    command += "--name \"Hard Disk Controller\" ";
    command += "--remove ";

    vbm_popen(command, output, "deregister storage controller (fixed disk)", false, false);

    if (enable_floppyio) {
        command  = "storagectl \"" + vm_name + "\" ";
        command += "--name \"Floppy Controller\" ";
        command += "--remove ";

        vbm_popen(command, output, "deregister storage controller (floppy disk)", false, false);
    }

    // Next, delete VM
    //
    fprintf(
        stderr,
        "%s Removing VM from VirtualBox.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    command  = "unregistervm \"" + vm_name + "\" ";
    command += "--delete ";

    vbm_popen(command, output, "delete VM", false, false);

    // Lastly delete medium(s) from Virtual Box Media Registry
    //
    if (enable_isocontextualization) {
        fprintf(
            stderr,
            "%s Removing virtual ISO 9660 disk from VirtualBox.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "closemedium dvd \"" + virtual_machine_slot_directory + "/" + iso_image_filename + "\" ";
        if (delete_media) {
            command += "--delete ";
        }
        vbm_popen(command, output, "remove virtual ISO 9660 disk", false, false);

        if (enable_cache_disk) {
            fprintf(
                stderr,
                "%s Removing virtual cache disk from VirtualBox.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            command  = "closemedium disk \"" + virtual_machine_slot_directory + "/" + cache_disk_filename + "\" ";
            if (delete_media) {
                command += "--delete ";
            }

            vbm_popen(command, output, "remove virtual cache disk", false, false);
        }
    } else {
        fprintf(
            stderr,
            "%s Removing virtual disk drive from VirtualBox.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "closemedium disk \"" + virtual_machine_slot_directory + "/" + image_filename + "\" ";
        if (delete_media) {
            command += "--delete ";
        }
        vbm_popen(command, output, "remove virtual disk", false, false);
    }

    if (enable_floppyio) {
        fprintf(
            stderr,
            "%s Removing virtual floppy disk from VirtualBox.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "closemedium floppy \"" + virtual_machine_slot_directory + "/" + floppy_image_filename + "\" ";
        if (delete_media) {
            command += "--delete ";
        }

        vbm_popen(command, output, "remove virtual floppy disk", false, false);
    }

    return 0;
}

int VBOX_VM::deregister_stale_vm() {
    string command;
    string output;
    string virtual_machine_slot_directory;
    size_t uuid_start;
    size_t uuid_end;
    int retval;

    get_slot_directory(virtual_machine_slot_directory);

    // We need to determine what the name or uuid is of the previous VM which owns
    // this virtual disk
    //
    command  = "showhdinfo \"" + virtual_machine_slot_directory + "/" + image_filename + "\" ";

    retval = vbm_popen(command, output, "get HDD info");
    if (retval) return retval;

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
    uuid_start = output.find("(UUID: ");
    if (uuid_start != string::npos) {
        // We can parse the virtual machine ID from the output
        uuid_start += 7;
        uuid_end = output.find(")", uuid_start);
        vm_name = output.substr(uuid_start, uuid_end - uuid_start);

        // Deregister stale VM by UUID
        return deregister_vm(false);
    } else {
        // Did the user delete the VM in VirtualBox and not the medium?  If so,
        // just remove the medium.
        command  = "closemedium disk \"" + virtual_machine_slot_directory + "/" + image_filename + "\" ";
        vbm_popen(command, output, "remove virtual disk", false, false);
        if (enable_floppyio) {
            command  = "closemedium floppy \"" + virtual_machine_slot_directory + "/" + floppy_image_filename + "\" ";
            vbm_popen(command, output, "remove virtual floppy disk", false, false);
        }
        if (enable_isocontextualization) {
            command  = "closemedium dvd \"" + virtual_machine_slot_directory + "/" + iso_image_filename + "\" ";
            vbm_popen(command, output, "remove virtual ISO 9660 disk", false);
            if (enable_cache_disk) {
                command  = "closemedium disk \"" + virtual_machine_slot_directory + "/" + cache_disk_filename + "\" ";
                vbm_popen(command, output, "remove virtual cache disk", false);
            }
        }
    }
    return 0;
}

int VBOX_VM::run(bool do_restore_snapshot) {
    int retval;

    retval = is_registered();
    if (ERR_TIMEOUT == retval) {

        return VBOXWRAPPER_ERR_RECOVERABLE;

    } else if (ERR_NOT_FOUND == retval) {

        if (is_vm_machine_configuration_available()) {
            retval = register_vm();
            if (retval) return retval;
        } else {
            if (is_hdd_registered()) {
                // Handle the case where a previous instance of the same projects VM
                // was already initialized for the current slot directory but aborted
                // while the task was suspended and unloaded from memory.
                retval = deregister_stale_vm();
                if (retval) return retval;
            }
            retval = create_vm();
            if (retval) return retval;
        }

    }

    // The user has requested that we exit after registering the VM, so return an
    // error to stop further processing.
    if (register_only) return ERR_FOPEN;

    // If we are restarting an already registered VM, then the vm_name variable
    // will be empty right now, so populate it with the master name so all of the
    // various other functions will work.
    vm_name = vm_master_name;

    // Check to see if the VM is already in a running state, if so, poweroff.
    poll(false);
    if (online) {
        retval = poweroff();
        if (retval) return ERR_NOT_EXITED;
    }

    // If our last checkpoint time is greater than 0, restore from the previously
    // saved snapshot
    if (do_restore_snapshot) {
        retval = restore_snapshot();
        if (retval) return retval;
    }

    // Start the VM
    retval = start();
    if (retval) return retval;

    return 0;
}

void VBOX_VM::cleanup() {
    poweroff();
    deregister_vm(true);

    // Give time enough for external processes to finish the cleanup process
    boinc_sleep(5.0);
}

int VBOX_VM::start() {
    char buf[256];
    int retval;

    fprintf(
        stderr,
        "%s Starting VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

#ifdef NEW_EXECUTION_PATH
    retval = launch_vboxvm();
#else
    string command;
    string output;
    int timeout = 0;

    command = "startvm \"" + vm_name + "\"";
    if (headless) {
        command += " --type headless";
    }
    retval = vbm_popen(command, output, "start VM", true, false, 0);

    // Get the VM pid as soon as possible
    while (!retval) {
        boinc_sleep(1.0);
        timeout += 1;

        get_vm_process_id();

        if (vm_pid) break;

        if (timeout > 45) {
            retval = ERR_TIMEOUT;
            break;
        }
    }
#endif

    if (BOINC_SUCCESS == retval) {
        fprintf(
            stderr,
            "%s Successfully started VM. (PID = '%d')\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            vm_pid
        );
    } else {
        fprintf(
            stderr,
            "%s VM failed to start.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }

    return retval;
}

int VBOX_VM::stop() {
    string command;
    string output;
    double timeout;
    char buf[256];
    int retval = 0;

    fprintf(
        stderr,
        "%s Stopping VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    if (online) {
        command = "controlvm \"" + vm_name + "\" savestate";
        retval = vbm_popen(command, output, "stop VM", true, false);

        // Wait for up to 5 minutes for the VM to switch states.  A system
        // under load can take a while.  Since the poll function can wait for up
        // to 45 seconds to execute a command we need to make this time based instead
        // of iteration based.
        if (!retval) {
            timeout = dtime() + 300;
            do {
                poll(false);
                if (!online && !saving) break;
                boinc_sleep(1.0);
            } while (timeout >= dtime());
        }

        if (!online) {
            fprintf(
                stderr,
                "%s Successfully stopped VM.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            retval = BOINC_SUCCESS;
        } else {
            fprintf(
                stderr,
                "%s VM did not stop when requested.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );

            // Attempt to terminate the VM
            retval = kill_program(vm_pid);
            if (retval) {
                fprintf(
                    stderr,
                    "%s VM was NOT successfully terminated.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
            } else {
                fprintf(
                    stderr,
                    "%s VM was successfully terminated.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
            }
        }
    }

    return retval;
}

int VBOX_VM::poweroff() {
    string command;
    string output;
    double timeout;
    char buf[256];
    int retval = 0;

    fprintf(
        stderr,
        "%s Powering off VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    if (online) {
        command = "controlvm \"" + vm_name + "\" poweroff";
        retval = vbm_popen(command, output, "poweroff VM", true, false);

        // Wait for up to 5 minutes for the VM to switch states.  A system
        // under load can take a while.  Since the poll function can wait for up
        // to 45 seconds to execute a command we need to make this time based instead
        // of iteration based.
        if (!retval) {
            timeout = dtime() + 300;
            do {
                poll(false);
                if (!online && !saving) break;
                boinc_sleep(1.0);
            } while (timeout >= dtime());
        }

        if (!online) {
            fprintf(
                stderr,
                "%s Successfully powered off VM.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            retval = BOINC_SUCCESS;
        } else {
            fprintf(
                stderr,
                "%s VM did not power off when requested.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );

            // Attempt to terminate the VM
            retval = kill_program(vm_pid);
            if (retval) {
                fprintf(
                    stderr,
                    "%s VM was NOT successfully terminated.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
            } else {
                fprintf(
                    stderr,
                    "%s VM was successfully terminated.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
            }
        }
    }

    return retval;
}

int VBOX_VM::pause() {
    string command;
    string output;
    int retval;

    // Restore the process priority back to the default process priority
    // to speed up the last minute maintenance tasks before the VirtualBox
    // VM goes to sleep
    //
    reset_vm_process_priority();

    command = "controlvm \"" + vm_name + "\" pause";
    retval = vbm_popen(command, output, "pause VM");
    if (retval) return retval;
    suspended = true;
    return 0;
}

int VBOX_VM::resume() {
    string command;
    string output;
    int retval;

    // Set the process priority back to the lowest level before resuming
    // execution
    //
    lower_vm_process_priority();

    command = "controlvm \"" + vm_name + "\" resume";
    retval = vbm_popen(command, output, "resume VM");
    if (retval) return retval;
    suspended = false;
    return 0;
}

int VBOX_VM::create_snapshot(double elapsed_time) {
    string command;
    string output;
    char buf[256];
    int retval;

    fprintf(
        stderr,
        "%s Creating new snapshot for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

    // Pause VM - Try and avoid the live snapshot and trigger an online
    // snapshot instead.
    pause();

    // Create new snapshot
    sprintf(buf, "%d", (int)elapsed_time);
    command = "snapshot \"" + vm_name + "\" ";
    command += "take boinc_";
    command += buf;
    retval = vbm_popen(command, output, "create new snapshot", true, true, 0);
    if (retval) return retval;

    // Resume VM
    resume();

    // Set the suspended flag back to false before deleting the stale
    // snapshot
    poll(false);

    // Delete stale snapshot(s), if one exists
    retval = cleanup_snapshots(false);
    if (retval) return retval;

    fprintf(
        stderr,
        "%s Checkpoint completed.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

    return 0;
}

int VBOX_VM::cleanup_snapshots(bool delete_active) {
    string command;
    string output;
    string snapshotlist;
    string line;
    string uuid;
    size_t eol_pos;
    size_t eol_prev_pos;
    size_t uuid_start;
    size_t uuid_end;
    char buf[256];
    int retval;


    // Enumerate snapshot(s)
    command = "snapshot \"" + vm_name + "\" ";
    command += "list ";

    // Only log the error if we are not attempting to deregister the VM.
    // delete_active is only set to true when we are deregistering the VM.
    retval = vbm_popen(command, snapshotlist, "enumerate snapshot(s)", !delete_active, false, 0);
    if (retval) return retval;

    // Output should look a little like this:
    //   Name: Snapshot 2 (UUID: 1751e9a6-49e7-4dcc-ab23-08428b665ddf)
    //      Name: Snapshot 3 (UUID: 92fa8b35-873a-4197-9d54-7b6b746b2c58)
    //         Name: Snapshot 4 (UUID: c049023a-5132-45d5-987d-a9cfadb09664) *
    //
    // Traverse the list from newest to oldest.  Otherwise we end up with an error:
    //   VBoxManage.exe: error: Snapshot operation failed
    //   VBoxManage.exe: error: Hard disk 'C:\ProgramData\BOINC\slots\23\vm_image.vdi' has 
    //     more than one child hard disk (2)
    //

    // Prepend a space and line feed to the output since we are going to traverse it backwards
    snapshotlist = " \n" + snapshotlist;

    eol_prev_pos = snapshotlist.rfind("\n");
    eol_pos = snapshotlist.rfind("\n", eol_prev_pos - 1);
    while (eol_pos != string::npos) {
        line = snapshotlist.substr(eol_pos, eol_prev_pos - eol_pos);

        // Find the previous line to use in the next iteration
        eol_prev_pos = eol_pos;
        eol_pos = snapshotlist.rfind("\n", eol_prev_pos - 1);

        // This VM does not yet have any snapshots
        if (line.find("does not have any snapshots") != string::npos) break;

        // The * signifies that it is the active snapshot and one we do not want to delete
        if (!delete_active && (line.rfind("*") != string::npos)) continue;

        uuid_start = line.find("(UUID: ");
        if (uuid_start != string::npos) {
            // We can parse the virtual machine ID from the line
            uuid_start += 7;
            uuid_end = line.find(")", uuid_start);
            uuid = line.substr(uuid_start, uuid_end - uuid_start);

            fprintf(
                stderr,
                "%s Deleting stale snapshot.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );

            // Delete stale snapshot, if one exists
            command = "snapshot \"" + vm_name + "\" ";
            command += "delete \"";
            command += uuid;
            command += "\" ";
            
            // Only log the error if we are not attempting to deregister the VM.
            // delete_active is only set to true when we are deregistering the VM.
            retval = vbm_popen(command, output, "delete stale snapshot", !delete_active, false, 0);
            if (retval) return retval;
        }
    }

    return 0;
}

int VBOX_VM::restore_snapshot() {
    string command;
    string output;
    char buf[256];
    int retval = BOINC_SUCCESS;

    fprintf(
        stderr,
        "%s Restore from previously saved snapshot.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

    command = "snapshot \"" + vm_name + "\" ";
    command += "restorecurrent ";
    retval = vbm_popen(command, output, "restore current snapshot", true, false, 0);
    if (retval) return retval;

    fprintf(
        stderr,
        "%s Restore completed.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

    return retval;
}

void VBOX_VM::dump_hypervisor_logs(bool include_error_logs) {
    string local_system_log;
    string local_vm_log;
    string local_trace_log;
    unsigned long vm_exit_code = 0;

    get_system_log(local_system_log);
    get_vm_log(local_vm_log);
    get_trace_log(local_trace_log);
    get_vm_exit_code(vm_exit_code);

    if (include_error_logs) {
        fprintf(
            stderr,
            "\n"
            "    Hypervisor System Log:\n\n"
            "%s\n"
            "    VM Execution Log:\n\n"
            "%s\n"
            "    VM Trace Log:\n\n"
            "%s",
            local_system_log.c_str(),
            local_vm_log.c_str(),
            local_trace_log.c_str()
        );
    }

    if (vm_exit_code) {
        fprintf(
            stderr,
            "\n"
            "    VM Exit Code: %d (0x%x)\n\n",
            (unsigned int)vm_exit_code,
            (unsigned int)vm_exit_code
        );
    }
}

void VBOX_VM::dump_hypervisor_status_reports() {

#ifdef _WIN32
    char buf[256];
    SIZE_T ulMinimumWorkingSetSize;
    SIZE_T ulMaximumWorkingSetSize;

    if (
        GetProcessWorkingSetSize(
            vboxsvc_pid_handle,
            &ulMinimumWorkingSetSize,
            &ulMaximumWorkingSetSize)
    ) {
        fprintf(
            stderr,
            "%s Status Report (VirtualBox VboxSvc.exe): Minimum WSS: '%dKB', Maximum WSS: '%dKB'\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            ulMinimumWorkingSetSize/1024,
            ulMaximumWorkingSetSize/1024
        );
    }

    if (
        GetProcessWorkingSetSize(
            vm_pid_handle,
            &ulMinimumWorkingSetSize,
            &ulMaximumWorkingSetSize)
    ) {
        fprintf(
            stderr,
            "%s Status Report (VirtualBox Vboxheadless.exe/VirtualBox.exe): Minimum WSS: '%dKB', Maximum WSS: '%dKB'\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            ulMinimumWorkingSetSize/1024,
            ulMaximumWorkingSetSize/1024
        );
    }
#endif
}

// t1 > t2
static bool is_timestamp_newer(VBOX_TIMESTAMP& t1, VBOX_TIMESTAMP& t2) {
    if (t1.hours > t2.hours) return true;
    if (t1.hours < t2.hours) return false;
    if (t1.minutes > t2.minutes) return true;
    if (t1.minutes < t2.minutes) return false;
    if (t1.seconds > t2.seconds) return true;
    if (t1.seconds < t2.seconds) return false;
    if (t1.milliseconds > t2.milliseconds) return true;
    return false;
}

// Dump any new guest log messages which are generated by applications running within
// the guest VM.
void VBOX_VM::dump_vmguestlog_entries() {
    string line;
    size_t eol_pos;
    size_t eol_prev_pos;
    size_t line_pos;
    VBOX_TIMESTAMP current_timestamp;
    string msg;
    char buf[256];
    
    eol_prev_pos = 0;
    eol_pos = vm_log.find("\n");
    while (eol_pos != string::npos) {
        line = vm_log.substr(eol_prev_pos, eol_pos - eol_prev_pos);

        line_pos = line.find("Guest Log:");
        if (line_pos != string::npos) {
            sscanf(
                line.c_str(),
                "%d:%d:%d.%d",
                &current_timestamp.hours, &current_timestamp.minutes,
                &current_timestamp.seconds, &current_timestamp.milliseconds
            );

            if (is_timestamp_newer(current_timestamp, vm_log_timestamp)) {
                vm_log_timestamp = current_timestamp;
                msg = line.substr(line_pos, line.size() - line_pos);

                fprintf(
                    stderr,
                    "%s %s\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    msg.c_str()
                );
            }
        }

        eol_prev_pos = eol_pos + 1;
        eol_pos = vm_log.find("\n", eol_prev_pos);
    }
}

int VBOX_VM::is_registered() {
    string command;
    string output;
    string needle;
    char buf[256];
    int retval;

    command  = "showvminfo \"" + vm_master_name + "\" ";
    command += "--machinereadable ";

    // Look for this string in the output
    //
    needle = "name=\"" + vm_master_name + "\"";

    retval = vbm_popen(command, output, "registration detection", false, false);

    // Handle explicit cases first
    if (ERR_TIMEOUT == retval) {
        return ERR_TIMEOUT;
    }
    if (output.find("VBOX_E_OBJECT_NOT_FOUND") != string::npos) {
        return ERR_NOT_FOUND;
    }
    if (!retval && output.find(needle.c_str()) != string::npos) {
        return BOINC_SUCCESS;
    }

    // Something unexpected has happened.  Dump diagnostic output.
    fprintf(
        stderr,
        "%s Error in registration for VM: %d\nArguments:\n%s\nOutput:\n%s\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        retval,
        command.c_str(),
        output.c_str()
    );

    return retval;
}

// Attempt to detect any condition that would prevent VirtualBox from running a VM properly, like:
// 1. The DCOM service not being started on Windows
// 2. Vboxmanage not being able to communicate with vboxsvc for some reason
// 3. VirtualBox driver not loaded for the current Linux kernel.
//
// Luckly both of the above conditions can be detected by attempting to detect the host information
// via vboxmanage and it is cross platform.
//
bool VBOX_VM::is_system_ready(std::string& message) {
    string command;
    string output;
    char buf[256];
    int retval;
    bool rc = false;

    command  = "list hostinfo ";
    retval = vbm_popen(command, output, "host info");
    if (BOINC_SUCCESS == retval) {
        rc = true;
    }

    if (output.size() == 0) {
        fprintf(
            stderr,
            "%s WARNING: Communication with VM Hypervisor failed. (Possibly Out of Memory).\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        message = "Communication with VM Hypervisor failed. (Possibly Out of Memory).";
        rc = false;
    }

    if (output.find("Processor count:") == string::npos) {
        fprintf(
            stderr,
            "%s WARNING: Communication with VM Hypervisor failed.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        message = "Communication with VM Hypervisor failed.";
        rc = false;
    }

    if (output.find("WARNING: The vboxdrv kernel module is not loaded.") != string::npos) {
        vboxwrapper_msg_prefix(buf, sizeof(buf));
        fprintf(
            stderr,
            "%s WARNING: The vboxdrv kernel module is not loaded.\n"
            "%s WARNING: Please update/recompile VirtualBox kernel drivers.\n",
            buf,
            buf
        );
        message = "Please update/recompile VirtualBox kernel drivers.";
        rc = false;
    }

    return rc;
}

bool VBOX_VM::is_vm_machine_configuration_available() {
    string virtual_machine_slot_directory;
    string vm_machine_configuration_file;
    APP_INIT_DATA aid;

    boinc_get_init_data_p(&aid);
    get_slot_directory(virtual_machine_slot_directory);

    vm_machine_configuration_file = virtual_machine_slot_directory + "/" + vm_master_name + "/" + vm_master_name + ".vbox";
    if (boinc_file_exists(vm_machine_configuration_file.c_str())) {
        return true;
    }
    return false;
}

bool VBOX_VM::is_hdd_registered() {
    string command;
    string output;
    string virtual_machine_root_dir;

    get_slot_directory(virtual_machine_root_dir);

    command = "showhdinfo \"" + virtual_machine_root_dir + "/" + image_filename + "\" ";

    if (vbm_popen(command, output, "hdd registration", false, false) == 0) {
        if ((output.find("VBOX_E_FILE_ERROR") == string::npos) && 
            (output.find("VBOX_E_OBJECT_NOT_FOUND") == string::npos) &&
            (output.find("does not match the value") == string::npos)
        ) {
            // Error message not found in text
            return true;
        }
    }
    return false;
}

bool VBOX_VM::is_extpack_installed() {
    string command;
    string output;

    command = "list extpacks";

    if (vbm_popen(command, output, "extpack detection", false, false) == 0) {
        if ((output.find("Oracle VM VirtualBox Extension Pack") != string::npos) && (output.find("VBoxVRDP") != string::npos)) {
            return true;
        }
    }
    return false;
}

bool VBOX_VM::is_logged_failure_vm_extensions_disabled() {
    if (vm_log.find("VERR_VMX_MSR_LOCKED_OR_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_SVM_DISABLED") != string::npos) return true;

    // VirtualBox 4.3.x or better
    if (vm_log.find("VERR_VMX_MSR_VMXON_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_VMX_MSR_SMX_VMXON_DISABLED") != string::npos) return true;

    return false;
}

bool VBOX_VM::is_logged_failure_vm_extensions_in_use() {
    if (vm_log.find("VERR_VMX_IN_VMX_ROOT_MODE") != string::npos) return true;
    if (vm_log.find("VERR_SVM_IN_USE") != string::npos) return true;
    return false;
}

bool VBOX_VM::is_logged_failure_vm_extensions_not_supported() {
    if (vm_log.find("VERR_VMX_NO_VMX") != string::npos) return true;
    if (vm_log.find("VERR_SVM_NO_SVM") != string::npos) return true;
    return false;
}

bool VBOX_VM::is_logged_failure_host_out_of_memory() {
    if (vm_log.find("VERR_EM_NO_MEMORY") != string::npos) return true;
    if (vm_log.find("VERR_NO_MEMORY") != string::npos) return true;
    return false;
}

bool VBOX_VM::is_logged_failure_guest_job_out_of_memory() {
    if (vm_log.find("EXIT_OUT_OF_MEMORY") != string::npos) return true;
    return false;
}

bool VBOX_VM::is_virtualbox_version_newer(int maj, int min, int rel) {
    int vbox_major = 0, vbox_minor = 0, vbox_release = 0;
    if (3 == sscanf(virtualbox_version.c_str(), "%d.%d.%d", &vbox_major, &vbox_minor, &vbox_release)) {
        if (maj < vbox_major) return true;
        if (maj > vbox_major) return false;
        if (min < vbox_minor) return true;
        if (min > vbox_minor) return false;
        if (rel < vbox_release) return true;
    }
    return false;
}

bool VBOX_VM::is_virtualbox_error_recoverable(int retval) {
    // See comments for VBOX_VM::vbm_popen about session lock issues.
    if (VBOX_E_INVALID_OBJECT_STATE == (unsigned int)retval) return true;
    return false;
}

int VBOX_VM::get_install_directory(string& install_directory ) {
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

            install_directory = lpszRegistryValue;
        }
    }

    if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszRegistryValue) free(lpszRegistryValue);
    if (install_directory.empty()) {
        return 1;
    }
    return 0;
#else
    install_directory = "";
    return 0;
#endif
}

int VBOX_VM::get_version_information(string& version) {
    string command;
    string output;
    int retval;

    // Record the VirtualBox version information for later use.
    command = "--version ";
    retval = vbm_popen(command, output, "version check");

    // Remove \r or \n from the output spew
    string::iterator iter = output.begin();
    while (iter != output.end()) {
        if (*iter == '\r' || *iter == '\n') {
            iter = output.erase(iter);
        } else {
            ++iter;
        }
    }

    version = output;
    return retval;
}

int VBOX_VM::get_guest_additions(string& guest_additions) {
    string command;
    string output;
    size_t ga_start;
    size_t ga_end;
    int retval;

    // Get the location of where the guest additions are
    command = "list systemproperties";
    retval = vbm_popen(command, output, "guest additions");

    // Output should look like this:
    // API version:                     4_3
    // Minimum guest RAM size:          4 Megabytes
    // Maximum guest RAM size:          2097152 Megabytes
    // Minimum video RAM size:          1 Megabytes
    // Maximum video RAM size:          256 Megabytes
    // ...
    // Default Guest Additions ISO:     C:\Program Files\Oracle\VirtualBox/VBoxGuestAdditions.iso
    //

    ga_start = output.find("Default Guest Additions ISO:");
    if (ga_start == string::npos) {
        return ERR_NOT_FOUND;
    }
    ga_start += strlen("Default Guest Additions ISO:");
    ga_end = output.find("\n", ga_start);
    guest_additions = output.substr(ga_start, ga_end - ga_start);
    strip_whitespace(guest_additions);
    if (guest_additions.size() <= 0) {
        return ERR_NOT_FOUND;
    }

    return retval;
}

// Returns the current directory in which the executable resides.
//
int VBOX_VM::get_slot_directory(string& dir) {
    char slot_dir[256];

    getcwd(slot_dir, sizeof(slot_dir));
    dir = slot_dir;

    if (!dir.empty()) {
        return 1;
    }
    return 0;
}

int VBOX_VM::get_vm_network_bytes_sent(double& sent) {
    string command;
    string output;
    string counter_value;
    size_t counter_start;
    size_t counter_end;
    int retval;

    command  = "debugvm \"" + vm_name + "\" ";
    command += "statistics --pattern \"/Devices/*/TransmitBytes\" ";

    retval = vbm_popen(command, output, "get bytes sent");
    if (retval) return retval;

    // Output should look like this:
    // <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    // <Statistics>
    // <Counter c="397229" unit="bytes" name="/Devices/PCNet0/TransmitBytes"/>
    // <Counter c="256" unit="bytes" name="/Devices/PCNet1/TransmitBytes"/>
    // </Statistics>

    // add up the counter(s)
    //
    sent = 0;
    counter_start = output.find("c=\"");
    while (counter_start != string::npos) {
        counter_start += 3;
        counter_end = output.find("\"", counter_start);
        counter_value = output.substr(counter_start, counter_end - counter_start);
        sent += atof(counter_value.c_str());
        counter_start = output.find("c=\"", counter_start);
    }
    return 0;
}

int VBOX_VM::get_vm_network_bytes_received(double& received) {
    string command;
    string output;
    string counter_value;
    size_t counter_start;
    size_t counter_end;
    int retval;

    command  = "debugvm \"" + vm_name + "\" ";
    command += "statistics --pattern \"/Devices/*/ReceiveBytes\" ";

    retval = vbm_popen(command, output, "get bytes received");
    if (retval) return retval;

    // Output should look like this:
    // <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    // <Statistics>
    // <Counter c="9423150" unit="bytes" name="/Devices/PCNet0/ReceiveBytes"/>
    // <Counter c="256" unit="bytes" name="/Devices/PCNet1/ReceiveBytes"/>
    // </Statistics>

    // add up the counter(s)
    //
    received = 0;
    counter_start = output.find("c=\"");
    while (counter_start != string::npos) {
        counter_start += 3;
        counter_end = output.find("\"", counter_start);
        counter_value = output.substr(counter_start, counter_end - counter_start);
        received += atof(counter_value.c_str());
        counter_start = output.find("c=\"", counter_start);
    }

    return 0;
}

int VBOX_VM::get_vm_process_id() {
    string output;
    string pid;
    size_t pid_start;
    size_t pid_end;

    get_vm_log(output, false);

    // Output should look like this:
    // VirtualBox 4.1.0 r73009 win.amd64 (Jul 19 2011 13:05:53) release log
    // 00:00:06.008 Log opened 2011-09-01T23:00:59.829170900Z
    // 00:00:06.008 OS Product: Windows 7
    // 00:00:06.009 OS Release: 6.1.7601
    // 00:00:06.009 OS Service Pack: 1
    // 00:00:06.015 Host RAM: 4094MB RAM, available: 876MB
    // 00:00:06.015 Executable: C:\Program Files\Oracle\VirtualBox\VirtualBox.exe
    // 00:00:06.015 Process ID: 6128
    // 00:00:06.015 Package type: WINDOWS_64BITS_GENERIC
    // 00:00:06.015 Installed Extension Packs:
    // 00:00:06.015   None installed!
    //
    pid_start = output.find("Process ID: ");
    if (pid_start == string::npos) {
        return ERR_NOT_FOUND;
    }
    pid_start += strlen("Process ID: ");
    pid_end = output.find("\n", pid_start);
    pid = output.substr(pid_start, pid_end - pid_start);
    strip_whitespace(pid);
    if (pid.size() <= 0) {
        return ERR_NOT_FOUND;
    }

    vm_pid = atol(pid.c_str());

#ifdef _WIN32
    vm_pid_handle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION,
        FALSE,
        vm_pid
    );
#endif

    return 0;
}

int VBOX_VM::get_vm_exit_code(unsigned long& exit_code) {
#ifdef _WIN32
    if (vm_pid_handle) {
        GetExitCodeProcess(vm_pid_handle, &exit_code);
    }
#else
    int ec = 0;
    waitpid(vm_pid, &ec, WNOHANG);
    exit_code = ec;
#endif
    return 0;
}

double VBOX_VM::get_vm_cpu_time() {
    double x = process_tree_cpu_time(vm_pid);
    if (x > current_cpu_time) {
        current_cpu_time = x;
    }
    return current_cpu_time;
}

int VBOX_VM::get_system_log(string& log, bool tail_only) {
    string slot_directory;
    string virtualbox_system_log_src;
    string virtualbox_system_log_dst;
    string::iterator iter;
    int retval = BOINC_SUCCESS;
    char buf[256];

    // Where should we copy temp files to?
    get_slot_directory(slot_directory);

    // Locate and read log file
    virtualbox_system_log_src = virtualbox_home_directory + "/VBoxSVC.log";
    virtualbox_system_log_dst = slot_directory + "/VBoxSVC.log";

    if (boinc_file_exists(virtualbox_system_log_src.c_str())) {
        // Skip having to deal with various forms of file locks by just making a temp
        // copy of the log file.
        boinc_copy(virtualbox_system_log_src.c_str(), virtualbox_system_log_dst.c_str());
    } else {
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox System Log used.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }

    if (boinc_file_exists(virtualbox_system_log_dst.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(virtualbox_system_log_dst.c_str(), log, 8192, true);
        } else {
            read_file_string(virtualbox_system_log_dst.c_str(), log);
        }

        sanitize_output(log);

        if (tail_only) {
            if (log.size() >= 8000) {
                // Look for the next whole line of text.
                iter = log.begin();
                while (iter != log.end()) {
                    if (*iter == '\n') {
                        log.erase(iter);
                        break;
                    }
                    iter = log.erase(iter);
                }
            }
        }
    } else {
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox System Log Not Found.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        retval = ERR_NOT_FOUND;
    }

    return retval;
}

int VBOX_VM::get_vm_log(string& log, bool tail_only) {
    string slot_directory;
    string virtualbox_vm_log_src;
    string virtualbox_vm_log_dst;
    string::iterator iter;
    int retval = BOINC_SUCCESS;
    char buf[256];

    // Where should we copy temp files to?
    get_slot_directory(slot_directory);

    // Locate and read log file
    virtualbox_vm_log_src = vm_master_name + "/Logs/VBox.log";
    virtualbox_vm_log_dst = slot_directory + "/VBox.log";

    if (boinc_file_exists(virtualbox_vm_log_src.c_str())) {
        // Skip having to deal with various forms of file locks by just making a temp
        // copy of the log file.
        boinc_copy(virtualbox_vm_log_src.c_str(), virtualbox_vm_log_dst.c_str());
    } else {
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox VM Log used.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }

    if (boinc_file_exists(virtualbox_vm_log_dst.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(virtualbox_vm_log_dst.c_str(), log, 8192, true);
        } else {
            read_file_string(virtualbox_vm_log_dst.c_str(), log);
        }

        sanitize_output(log);

        if (tail_only) {
            if (log.size() >= 8000) {
                // Look for the next whole line of text.
                iter = log.begin();
                while (iter != log.end()) {
                    if (*iter == '\n') {
                        log.erase(iter);
                        break;
                    }
                    iter = log.erase(iter);
                }
            }
        }

    } else {
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox VM Log Not Found.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        retval = ERR_NOT_FOUND;
    }

    return retval;
}

int VBOX_VM::get_trace_log(string& log, bool tail_only) {
    string slot_directory;
    string vm_trace_log;
    string::iterator iter;
    int retval = BOINC_SUCCESS;

    // Where should we copy temp files to?
    get_slot_directory(slot_directory);

    // Locate and read log file
    vm_trace_log = slot_directory + "/" + TRACELOG_FILENAME;

    if (boinc_file_exists(vm_trace_log.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(vm_trace_log.c_str(), log, 8192, true);
        } else {
            read_file_string(vm_trace_log.c_str(), log);
        }

        sanitize_output(log);

        if (tail_only) {
            if (log.size() >= 8000) {
                // Look for the next whole line of text.
                iter = log.begin();
                while (iter != log.end()) {
                    if (*iter == '\n') {
                        log.erase(iter);
                        break;
                    }
                    iter = log.erase(iter);
                }
            }
        }

    } else {
        retval = ERR_NOT_FOUND;
    }

    return retval;
}

// Enable the network adapter if a network connection is required.
// NOTE: Network access should never be allowed if the code running in a 
//   shared directory or the VM image itself is NOT signed.  Doing so
//   opens up the network behind the company firewall to attack.
//
//   Imagine a doomsday scenario where a project has been compromised and
//   an unsigned executable/VM image has been tampered with.  Volunteer
//   downloads compromised code and executes it on a company machine.
//   Now the compromised VM starts attacking other machines on the company
//   network.  The company firewall cannot help because the attacking
//   machine is already behind the company firewall.
//
int VBOX_VM::set_network_access(bool enabled) {
    string command;
    string output;
    char buf[256];
    int retval;

    network_suspended = !enabled;

    if (enabled) {
        fprintf(
            stderr,
            "%s Enabling network access for VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--cableconnected1 on ";

        retval = vbm_popen(command, output, "enable network");
        if (retval) return retval;
    } else {
        fprintf(
            stderr,
            "%s Disabling network access for VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--cableconnected1 off ";

        retval = vbm_popen(command, output, "disable network");
        if (retval) return retval;
    }
    return 0;
}

int VBOX_VM::set_cpu_usage(int percentage) {
    string command;
    string output;
    char buf[256];
    int retval;

    // the arg to controlvm is percentage
    //
    fprintf(
        stderr,
        "%s Setting CPU throttle for VM. (%d%%)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        percentage
    );
    sprintf(buf, "%d", percentage);
    command  = "controlvm \"" + vm_name + "\" ";
    command += "cpuexecutioncap ";
    command += buf;
    command += " ";

    retval = vbm_popen(command, output, "CPU throttle");
    if (retval) return retval;
    return 0;
}

int VBOX_VM::set_network_usage(int kilobytes) {
    string command;
    string output;
    char buf[256];
    int retval;

    // the argument to modifyvm is in KB
    //
    if (kilobytes == 0) {
        fprintf(
            stderr,
            "%s Setting network throttle for VM. (1024GB)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    } else {
        fprintf(
            stderr,
            "%s Setting network throttle for VM. (%dKB)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            kilobytes
        );
    }

    if (is_virtualbox_version_newer(4, 2, 0)) {

        // Update bandwidth group limits
        //
        if (kilobytes == 0) {
            command  = "bandwidthctl \"" + vm_name + "\" ";
            command += "set \"" + vm_name + "_net\" ";
            command += "--limit 1024G ";

            retval = vbm_popen(command, output, "network throttle (set default value)");
            if (retval) return retval;
        } else {
            sprintf(buf, "%d", kilobytes);
            command  = "bandwidthctl \"" + vm_name + "\" ";
            command += "set \"" + vm_name + "_net\" ";
            command += "--limit ";
            command += buf;
            command += "K ";

            retval = vbm_popen(command, output, "network throttle (set)");
            if (retval) return retval;
        }

    } else {

        sprintf(buf, "%d", kilobytes);
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--nicspeed1 ";
        command += buf;
        command += " ";

        retval = vbm_popen(command, output, "network throttle");
        if (retval) return retval;

    }

    return 0;
}

int VBOX_VM::read_floppy(std::string& data) {
    if (enable_floppyio && pFloppy) {
        data = pFloppy->receive();
        return 0;
    }
    return 1;
}

int VBOX_VM::write_floppy(std::string& data) {
    if (enable_floppyio && pFloppy) {
        pFloppy->send(data);
        return 0;
    }
    return 1;
}

void VBOX_VM::lower_vm_process_priority() {
    char buf[256];
#ifdef _WIN32
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, BELOW_NORMAL_PRIORITY_CLASS);
        fprintf(
            stderr,
            "%s Lowering VM Process priority.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
#else
    if (vm_pid) {
        setpriority(PRIO_PROCESS, vm_pid, PROCESS_MEDIUM_PRIORITY);
        fprintf(
            stderr,
            "%s Lowering VM Process priority.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
#endif
}

void VBOX_VM::reset_vm_process_priority() {
    char buf[256];
#ifdef _WIN32
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, NORMAL_PRIORITY_CLASS);
        fprintf(
            stderr,
            "%s Restoring VM Process priority.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
#else
    if (vm_pid) {
        setpriority(PRIO_PROCESS, vm_pid, PROCESS_NORMAL_PRIORITY);
        fprintf(
            stderr,
            "%s Restoring VM Process priority.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
#endif
}

// Launch VboxSVC.exe before going any further. if we don't, it'll be launched by
// svchost.exe with its environment block which will not contain the reference
// to VBOX_USER_HOME which is required for running in the BOINC account-based
// sandbox on Windows.
int VBOX_VM::launch_vboxsvc() {
    APP_INIT_DATA aid;
    PROC_MAP pm;
    PROCINFO p;
    string command;
    int retval = ERR_EXEC;

#ifdef _WIN32
    char buf[256];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int pidVboxSvc = 0;
    HANDLE hVboxSvc = NULL;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    boinc_get_init_data_p(&aid);

    if (aid.using_sandbox) {

        if (!vboxsvc_pid_handle || !process_exists(vboxsvc_pid_handle)) {

            if (vboxsvc_pid_handle) CloseHandle(vboxsvc_pid_handle);

            procinfo_setup(pm);
            for (PROC_MAP::iterator i = pm.begin(); i != pm.end(); ++i) {
                p = i->second;

                // We are only looking for vboxsvc
                if (0 != stricmp(p.command, "vboxsvc.exe")) continue;

                // Store process id for later use
                pidVboxSvc = p.id;

                // Is this the vboxsvc for the current user?
                // Non-service install it would be the current username
                // Service install it would be boinc_project
                hVboxSvc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, p.id);
                if (hVboxSvc) break;
            }
            
            if (pidVboxSvc && hVboxSvc) {
                
                fprintf(
                    stderr,
                    "%s Status Report: Detected vboxsvc.exe. (PID = '%d')\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    pidVboxSvc
                );
                vboxsvc_pid = pidVboxSvc;
                vboxsvc_pid_handle = hVboxSvc;
                retval = BOINC_SUCCESS;

            } else {

                si.cb = sizeof(STARTUPINFO);
                si.dwFlags |= STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;

                command = "\"VBoxSVC.exe\" --logrotate 1";

                CreateProcess(NULL, (LPTSTR)command.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

                if (pi.hThread) CloseHandle(pi.hThread);
                if (pi.hProcess) {
                    fprintf(
                        stderr,
                        "%s Status Report: Launching vboxsvc.exe. (PID = '%d')\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        pi.dwProcessId
                    );
                    vboxsvc_pid = pi.dwProcessId;
                    vboxsvc_pid_handle = pi.hProcess;
                    retval = BOINC_SUCCESS;
                } else {
                    fprintf(
                        stderr,
                        "%s Status Report: Launching vboxsvc.exe failed!.\n"
                        "           Error: %s",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        windows_format_error_string(GetLastError(), buf, sizeof(buf))
                    );
                }

                vbm_trace(command, std::string(""), retval);
            }
        }
    }
#endif

    return retval;
}

// Launch the VM.
int VBOX_VM::launch_vboxvm() {
    char buf[256];
    char cmdline[1024];
    char* argv[5];
    int argc;
    std::string output;
    int retval = ERR_EXEC;

    // Construct the command line parameters
    //
    if (headless) {
        argv[0] = const_cast<char*>("VboxHeadless.exe");
    } else {
        argv[0] = const_cast<char*>("VirtualBox.exe");
    }
    argv[1] = const_cast<char*>("--startvm");
    argv[2] = const_cast<char*>(vm_name.c_str());
    if (headless) {
        argv[3] = const_cast<char*>("--vrde config");
    } else {
        argv[3] = const_cast<char*>("--no-startvm-errormsgbox");
    }
    argv[4] = NULL;
    argc = 4;

    strcpy(cmdline, "");
    for (int i=0; i<argc; i++) {
        strcat(cmdline, argv[i]);
        if (i<argc-1) {
            strcat(cmdline, " ");
        }
    }

#ifdef _WIN32
    char error_msg[256];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    void* pBuf = NULL;
    DWORD dwCount = 0;
    unsigned long ulExitCode = 0;
    unsigned long ulExitTimeout = 0;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    memset(&sa, 0, sizeof(sa));
    memset(&sd, 0, sizeof(sd));

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, true, NULL, false);

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, NULL)) {
        fprintf(
            stderr,
            "%s CreatePipe failed! (%d).\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            GetLastError()
        );
        goto CLEANUP;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.hStdInput = NULL;

    // Execute command
    if (!CreateProcess(
        NULL, 
        cmdline,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        vboxwrapper_msg_prefix(buf, sizeof(buf));
        fprintf(
            stderr,
            "%s Status Report: Launching virtualbox.exe/vboxheadless.exe failed!.\n"
            "%s         Error: %s (%d)\n",
            buf,
            buf,
            windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg)),
            GetLastError()
        );

        goto CLEANUP;
    } 

    while(1) {
        GetExitCodeProcess(pi.hProcess, &ulExitCode);

        // Copy stdout/stderr to output buffer, handle in the loop so that we can
        // copy the pipe as it is populated and prevent the child process from blocking
        // in case the output is bigger than pipe buffer.
        PeekNamedPipe(hReadPipe, NULL, NULL, NULL, &dwCount, NULL);
        if (dwCount) {
            pBuf = malloc(dwCount+1);
            memset(pBuf, 0, dwCount+1);

            if (ReadFile(hReadPipe, pBuf, dwCount, &dwCount, NULL)) {
                output += (char*)pBuf;
            }

            free(pBuf);
        }

        if ((ulExitCode != STILL_ACTIVE) || (ulExitTimeout >= 1000)) break;

        Sleep(250);
        ulExitTimeout += 250;
    }

    if (ulExitCode != STILL_ACTIVE) {
        sanitize_output(output);
        vboxwrapper_msg_prefix(buf, sizeof(buf));
        fprintf(
            stderr,
            "%s Status Report: Virtualbox.exe/Vboxheadless.exe exited prematurely!.\n"
            "%s        Exit Code: %d\n"
            "%s        Output:\n"
            "%s\n",
            buf,
            buf,
            ulExitCode,
            buf,
            output.c_str()
        );
    }

    if (pi.hProcess && (ulExitCode == STILL_ACTIVE)) {
        vm_pid = pi.dwProcessId;
        vm_pid_handle = pi.hProcess;
        retval = BOINC_SUCCESS;
    }

CLEANUP:
    if (pi.hThread) CloseHandle(pi.hThread);
    if (hReadPipe) CloseHandle(hReadPipe);
    if (hWritePipe) CloseHandle(hWritePipe);

#else
    int pid = fork();
    if (-1 == pid) {
        output = strerror(errno);
        fprintf(
            stderr,
            "%s Status Report: Launching virtualbox.exe/vboxheadless.exe failed!.\n"
            "           Error: %s",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            output.c_str()
        );
        retval = ERR_FORK;
    } else if (0 == pid) {
        if (-1 == execv(argv[0], argv)) {
            _exit(errno);
        }
    } else {
        vm_pid = pid;
        retval = BOINC_SUCCESS;
    }
#endif

    string cmd_line = cmdline;
    vbm_trace(cmd_line, output, retval);

    return retval;
}

void VBOX_VM::sanitize_output(std::string& output) {
#ifdef _WIN32
    // Remove \r from the log spew
    string::iterator iter = output.begin();
    while (iter != output.end()) {
        if (*iter == '\r') {
            iter = output.erase(iter);
        } else {
            ++iter;
        }
    }
#endif
}

// If there are errors we can recover from, process them here.
//
int VBOX_VM::vbm_popen(string& command, string& output, const char* item, bool log_error, bool retry_failures, unsigned int timeout, bool log_trace) {
    int retval = 0;
    int retry_count = 0;
    double sleep_interval = 1.0;
    char buf[256];
    string retry_notes;

    // Initialize command line
    command = "VBoxManage -q " + command;

    do {
        retval = vbm_popen_raw(command, output, timeout);
        sanitize_output(command);
        sanitize_output(output);

        if (log_trace) {
            vbm_trace(command, output, retval);
        }

        if (retval) {

            // VirtualBox designed the concept of sessions to prevent multiple applications using
            // the VirtualBox COM API (virtualbox.exe, vboxmanage.exe) from modifying the same VM
            // at the same time.
            //
            // The problem here is that vboxwrapper uses vboxmanage.exe to modify and control the
            // VM.  Vboxmanage.exe can only maintain the session lock for as long as it takes it
            // to run.  So that means 99% of the time that a VM is running under BOINC technology
            // it is running without a session lock.
            //
            // If a volunteer opens another VirtualBox management application and goes poking around
            // that application can aquire the session lock and not give it up for some time.
            //
            // If we detect that condition retry the desired command.
            //
            // Experiments performed by jujube suggest changing the sleep interval to an exponential
            // style backoff would increase our chances of success in situations where the previous
            // lock is held by a previous instance of vboxmanage whos instance data hasn't been
            // cleaned up within vboxsvc yet.
            //
            // Error Code: VBOX_E_INVALID_OBJECT_STATE (0x80bb0007) 
            //
            if (VBOX_E_INVALID_OBJECT_STATE == (unsigned int)retval) {
                if (retry_notes.find("Another VirtualBox management") == string::npos) {
                    retry_notes += "Another VirtualBox management application has locked the session for\n";
                    retry_notes += "this VM. BOINC cannot properly monitor this VM\n";
                    retry_notes += "and so this job will be aborted.\n\n";
                }
                if (retry_count) {
                    sleep_interval *= 2;
                }
            }

            // VboxManage has to be able to communicate with vboxsvc in order to actually issue a
            // command.  In cases where we detect CO_E_SERVER_EXEC_FAILURE, we should just automatically
            // try the command again.  Vboxmanage wasn't even able to issue the desired command
            // anyway.
            //
            // Experiments performed by jujube suggest changing the sleep interval to an exponential
            // style backoff would increase our chances of success.
            //
            // Error Code: CO_E_SERVER_EXEC_FAILURE (0x80080005) 
            //
            if (CO_E_SERVER_EXEC_FAILURE == (unsigned int)retval) {
                if (retry_notes.find("Unable to communicate with VirtualBox") == string::npos) {
                    retry_notes += "Unable to communicate with VirtualBox.  VirtualBox may need to\n";
                    retry_notes += "be reinstalled.\n\n";
                }
                if (retry_count) {
                    sleep_interval *= 2;
                }
            }
            
            // Retry?
            if (!retry_failures && 
                (VBOX_E_INVALID_OBJECT_STATE != (unsigned int)retval) && 
                (CO_E_SERVER_EXEC_FAILURE != (unsigned int)retval)
            ) {
                break;
            }

            // Timeout?
            if (retry_count >= 5) break;

            retry_count++;
            boinc_sleep(sleep_interval);
        }
    }
    while (retval);

    // Add all relivent notes to the output string and log errors
    //
    if (retval && log_error) {
        if (!retry_notes.empty()) {
            output += "\nNotes:\n\n" + retry_notes;
        }

        fprintf(
            stderr,
            "%s Error in %s for VM: %d\nCommand:\n%s\nOutput:\n%s\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            item,
            retval,
            command.c_str(),
            output.c_str()
        );
    }

    return retval;
}

// Execute the vbox manage application and copy the output to the buffer.
//
int VBOX_VM::vbm_popen_raw(string& command, string& output, unsigned int timeout) {
    char buf[256];
    size_t errcode_start;
    size_t errcode_end;
    string errcode;
    int retval = BOINC_SUCCESS;

    // Launch vboxsvc in case it was shutdown for being idle
    launch_vboxsvc();

    // Reset output buffer
    output.clear();

#ifdef _WIN32

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    void* pBuf = NULL;
    DWORD dwCount = 0;
    unsigned long ulExitCode = 0;
    unsigned long ulExitTimeout = 0;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    memset(&sa, 0, sizeof(sa));
    memset(&sd, 0, sizeof(sd));

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, true, NULL, false);

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, NULL)) {
        fprintf(
            stderr,
            "%s CreatePipe failed! (%d).\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            GetLastError()
        );
        goto CLEANUP;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.hStdInput = NULL;

    // Execute command
    if (!CreateProcess(
        NULL, 
        (LPTSTR)command.c_str(),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        fprintf(
            stderr,
            "%s CreateProcess failed! (%d).\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            GetLastError()
        );
        goto CLEANUP;
    }

    // Wait until process has completed
    while(1) {
        if (timeout == 0) {
            WaitForSingleObject(pi.hProcess, INFINITE);
        }

        GetExitCodeProcess(pi.hProcess, &ulExitCode);

        // Copy stdout/stderr to output buffer, handle in the loop so that we can
        // copy the pipe as it is populated and prevent the child process from blocking
        // in case the output is bigger than pipe buffer.
        PeekNamedPipe(hReadPipe, NULL, NULL, NULL, &dwCount, NULL);
        if (dwCount) {
            pBuf = malloc(dwCount+1);
            memset(pBuf, 0, dwCount+1);

            if (ReadFile(hReadPipe, pBuf, dwCount, &dwCount, NULL)) {
                output += (char*)pBuf;
            }

            free(pBuf);
        }

        if (ulExitCode != STILL_ACTIVE) break;

        // Timeout?
        if (ulExitTimeout >= (timeout * 1000)) {
            if (!TerminateProcess(pi.hProcess, EXIT_FAILURE)) {
                fprintf(
                    stderr,
                    "%s TerminateProcess failed! (%d).\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    GetLastError()
                );
            }
            ulExitCode = 0;
            retval = ERR_TIMEOUT;
            Sleep(1000);
        }

        Sleep(250);
        ulExitTimeout += 250;
    }

CLEANUP:
    if (pi.hThread) CloseHandle(pi.hThread);
    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (hReadPipe) CloseHandle(hReadPipe);
    if (hWritePipe) CloseHandle(hWritePipe);

    if ((ulExitCode != 0) || (!pi.hProcess)) {

        // Determine the real error code by parsing the output
        errcode_start = output.find("(0x");
        if (errcode_start != string::npos) {
            errcode_start += 1;
            errcode_end = output.find(")", errcode_start);
            errcode = output.substr(errcode_start, errcode_end - errcode_start);

            sscanf(errcode.c_str(), "%x", &retval);
        }

        // If something couldn't be found, just return ERR_FOPEN
        if (!retval) retval = ERR_FOPEN;

    }

#else

    FILE* fp;

    // redirect stderr to stdout for the child process so we can trap it with popen.
    string modified_command = command + " 2>&1";

    // Execute command
    fp = popen(modified_command.c_str(), "r");
    if (fp == NULL){
        fprintf(
            stderr,
            "%s vbm_popen popen failed! errno = %d\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            errno
        );
        retval = ERR_FOPEN;
    } else {
        // Copy output to buffer
        while (fgets(buf, 256, fp)) {
            output += buf;
        }

        // Close stream
        pclose(fp);

        // Determine the real error code by parsing the output
        errcode_start = output.find("(0x");
        if (errcode_start != string::npos) {
            errcode_start += 1;
            errcode_end = output.find(")", errcode_start);
            errcode = output.substr(errcode_start, errcode_end - errcode_start);

            sscanf(errcode.c_str(), "%x", &retval);
        }
    }

#endif

    // Is this a RPC_S_SERVER_UNAVAILABLE returned by vboxmanage?
    if (output.find("RPC_S_SERVER_UNAVAILABLE") != string::npos) {
        retval = RPC_S_SERVER_UNAVAILABLE;
    }

    return retval;
}

void VBOX_VM::vbm_replay(std::string& command) {
    FILE* f = fopen(REPLAYLOG_FILENAME, "a");
    if (f) {
        fprintf(f, "%s\n", command.c_str());
        fclose(f);
    }
}

void VBOX_VM::vbm_trace(std::string& command, std::string& output, int retval) {
    vbm_replay(command);

    char buf[256];
    FILE* f = fopen(TRACELOG_FILENAME, "a");
    if (f) {
        fprintf(
            f,
            "%s\nCommand: %s\nExit Code: %d\nOutput:\n%s\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            command.c_str(),
            retval,
            output.c_str()
        );
        fclose(f);
    }
}

