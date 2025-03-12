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
#include <algorithm>
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
#include <fcntl.h>
#include <sys/mman.h>
#include <pwd.h>
#endif

using std::string;

#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "md5_file.h"
#include "network.h"
#include "boinc_api.h"
#include "floppyio.h"
#include "vboxlogging.h"
#include "vboxwrapper.h"
#include "vbox_common.h"
#include "vbox_vboxmanage.h"

VBOX_VM::VBOX_VM() {
}

VBOX_VM::~VBOX_VM() {
}

int VBOX_VM::initialize() {
    int rc = 0;
    string old_path;
    string new_path;
    string command;
    string output;

    get_install_directory(virtualbox_install_directory);

    // Prep the environment so we can execute the vboxmanage application
    //
#ifdef _WIN32
    if (!virtualbox_install_directory.empty()) {
        old_path = getenv("PATH");
        new_path = virtualbox_install_directory + ";" + old_path;

        if (!SetEnvironmentVariable("PATH", const_cast<char*>(new_path.c_str()))) {
            vboxlog_msg("Failed to modify the search path.");
        }
    }
#else
    old_path = getenv("PATH");
    if(boinc_file_exists("/usr/local/bin/VBoxManage")) {
        new_path = "/usr/local/bin/:" + old_path;
    }
    if(boinc_file_exists("/usr/bin/VBoxManage")) {
        new_path = "/usr/bin/:" + old_path;
    }
    // putenv does not copy its input buffer, so we must use setenv
    if (setenv("PATH", const_cast<char*>(new_path.c_str()), 1)) {
        vboxlog_msg("Failed to modify the search path.");
    }
#endif

    // Determine the 'VirtualBox profile directory'.
    // VboxSVC writes the main VM database and it's logfiles there.
    // The default location is OS based as well as user based and
    // can be modified via the VBOX_USER_HOME environment variable.
    //
    // See
    // https://docs.oracle.com/en/virtualization/virtualbox/6.1/admin/TechnicalBackground.html#3.1.3.-Summary-of-Configuration-Data-Locations

    // Check if specified by environment variable
    //
    char *p = getenv("VBOX_USER_HOME");
    if (p) {
        virtualbox_profile_directory = p;
    } else {
#ifdef _WIN32
        // Default vbox profile is located in '%USERPROFILE%\.VirtualBox'.
        // Check for '%USERPROFILE%' instead, since the vbox profile dir
        // doesn't exist until the user has started
        // a VirtualBox component at least once.
        //
        string vbox_profile_dir = getenv("USERPROFILE");

        if (vbox_profile_dir.size()) {
            virtualbox_profile_directory  = vbox_profile_dir;
            virtualbox_profile_directory += "/.VirtualBox";

            // If necessary VirtualBox automatically creates required dirs
            // at the default locations.
            //
        } else {
            // If '%USERPROFILE%' is not set.
            //
            virtualbox_profile_directory  = aid.boinc_dir;
            virtualbox_profile_directory += "/projects/VirtualBox";

            // create if not there already
            //
            boinc_mkdir(virtualbox_profile_directory.c_str());
        }
#else
#ifdef __APPLE__
        // If 'VBOX_USER_HOME' is not set
        // then make it point to the BOINC project directory.
        // Notes:
        // 1) we can't put it in the home dir;
        //  in a sandboxed config we're running as user 'boinc_projects',
        //  which doesn't have write access to the (real user) home dir.
        // 2) we can't put it in the BOINC data dir.
        //  boinc_projects can't write their either
        //
        virtualbox_profile_directory  = aid.boinc_dir;
        virtualbox_profile_directory += "/projects/VirtualBox";

        // create if not there already
        //
        boinc_mkdir(virtualbox_profile_directory.c_str());
#else
        // Default vbox profile is located in '/home/user/.config/VirtualBox'.
        // Check for '/home/user/.config' instead, since the vbox profile dir
        // doesn't exist until the user has started
        // a VirtualBox component at least once.
        //
        string linux_user = getenv("USER");
        string vbox_profile_dir = "/home/";
        vbox_profile_dir += linux_user;
        vbox_profile_dir += "/.config";

        if (is_dir(vbox_profile_dir.c_str())) {
            virtualbox_profile_directory  = vbox_profile_dir;
            virtualbox_profile_directory += "/VirtualBox";

            // If necessary VirtualBox automatically creates required dirs
            // at the default locations.
            //
        } else {
            // If BOINC runs as a service without a user's home directory.
            //
            virtualbox_profile_directory  = aid.boinc_dir;
            virtualbox_profile_directory += "/projects/VirtualBox";

            // create if not there already
            //
            boinc_mkdir(virtualbox_profile_directory.c_str());
        }
#endif
#endif

        // set env var telling VBox where to put log file
#ifdef _WIN32
        if (!SetEnvironmentVariable("VBOX_USER_HOME", const_cast<char*>(virtualbox_profile_directory.c_str()))) {
            vboxlog_msg("Failed to modify the search path.");
        }
#else
        // putenv does not copy its input buffer, so we must use setenv
        if (setenv("VBOX_USER_HOME", const_cast<char*>(virtualbox_profile_directory.c_str()), 1)) {
            vboxlog_msg("Failed to modify the VBOX_USER_HOME path.");
        }
#endif
    }

    rc = get_version_information(
        virtualbox_version_raw, virtualbox_version_display
    );
    if (rc) return rc;

    get_guest_additions(virtualbox_guest_additions);

    return 0;
}

int VBOX_VM::create_vm() {
    string command;
    string output;
    string needle;
    string default_interface;
    bool disable_acceleration = false;
    char buf[256];
    int retval;
    int save_retval;
    string vm_nictype1;
    size_t vm_nictype1_start;
    size_t vm_nictype1_end;

    vboxlog_msg("Create VM. (%s, slot#%d)", vm_master_name.c_str(), aid.slot);

    // Reset VM name in case it was changed while deregistering a stale VM
    //
    vm_name = vm_master_name;

    // Fixup chipset and drive controller information for known configurations
    //
    if (enable_isocontextualization) {
        if ("PIIX4" == vm_disk_controller_model) {
            vboxlog_msg("Updating drive controller type and model for desired configuration.");
            vm_disk_controller_type = "sata";
            vm_disk_controller_model = "IntelAHCI";
        }
    }

    // Create and register the VM
    //
    command  = "createvm ";
    command += "--name \"" + vm_name + "\" ";
    command += "--basefolder \"" + slot_dir_path + "\" ";
    command += "--ostype \"" + os_name + "\" ";
    command += "--register";

    retval = vbm_popen(command, output, "create");
    if (retval) return retval;

    // Tweak the VM's Description
    //
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--description \"" + vm_master_description + "\" ";

    vbm_popen(command, output, "modifydescription", false, false);

    // Tweak the VM's Memory Size
    //
    vboxlog_msg("Setting Memory Size for VM. (%dMB)", (int)memory_size_mb);
    snprintf(buf, sizeof(buf), "%d", (int)memory_size_mb);

    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--memory " + string(buf) + " ";

    retval = vbm_popen(command, output, "modifymem");
    if (retval) return retval;

    // Tweak the VM's CPU Count
    //
    vboxlog_msg("Setting CPU Count for VM. (%s)", vm_cpu_count.c_str());
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--cpus " + vm_cpu_count + " ";

    retval = vbm_popen(command, output, "modifycpu");
    if (retval) return retval;

    // Tweak the VM's Chipset Options
    //
    vboxlog_msg("Setting Chipset Options for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--acpi on ";
    command += "--ioapic on ";
    if (is_hostrtc_set_to_utc()) {
        command += "--rtcuseutc on ";
    } else {
        command += "--rtcuseutc off ";
    }

    retval = vbm_popen(command, output, "modifychipset");
    if (retval) return retval;

    // Tweak the VM's Graphics Controller Options
    //
    vboxlog_msg("Setting Graphics Controller Options for VM. (Driver: %s, %dMB)", vm_graphics_controller_type.c_str(), (int)vram_size_mb);
    snprintf(buf, sizeof(buf), "%d", (int)vram_size_mb);

    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--vram " + string(buf) + " ";
    command += "--graphicscontroller " + vm_graphics_controller_type + " ";

    retval = vbm_popen(command, output, "modifygraphicscontroller");
    if (retval) return retval;

    // Tweak the VM's Boot Options
    //
    vboxlog_msg("Setting Boot Options for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    if (boot_iso) {
        command += "--boot1 dvd ";
        command += "--boot2 disk ";
    } else {
        command += "--boot1 disk ";
        command += "--boot2 dvd ";
    }
    command += "--boot3 none ";
    command += "--boot4 none ";

    retval = vbm_popen(command, output, "modifyboot");
    if (retval) return retval;

    // Tweak the VM's Network Configuration
    //
    command  = "showvminfo \"" + vm_name + "\" ";
    command += "--machinereadable ";

    retval = vbm_popen(command, output, "get default networkadapter type", false, false);
    if (retval) return retval;

    vm_nictype1.clear();
    needle = "nictype1=\"";

    if ((vm_nictype1_start = output.find(needle.c_str())) != string::npos) {
        vm_nictype1_start += needle.size();
        vm_nictype1_end = output.find("\"", vm_nictype1_start);
        vm_nictype1 = output.substr(vm_nictype1_start, vm_nictype1_end - vm_nictype1_start);
    }

    // virtio-net is available since VirtualBox version 3.1.
    // use it for Linux guests.
    // see the VirtualBox manual for further details.
    //
    if (output.find("effparavirtprovider=\"kvm\"") != string::npos) {
        vm_nictype1 = "virtio";
    }

    // if a valid adapter name is set in vbox_job.xml use that one
    // see the VirtualBox manual for valid names.
    //
    if (vm_network_driver.size()) {
        command = "help modifyvm ";

        retval = vbm_popen(command, output, "verify custom networkadapter name", false, false);
        if (retval) return retval;

        if (output.find(vm_network_driver.c_str()) != string::npos) {
            vm_nictype1 = vm_network_driver;
        } else {
            vboxlog_msg("Invalid vm_network_driver '%s' detected in 'vbox_job.xml'.", vm_network_driver.c_str());
            vboxlog_msg("Will use '%s' instead.", vm_nictype1.c_str());
        }
    }

    if (network_bridged_mode) {
        vboxlog_msg("Setting Network Configuration for Bridged Mode. (Driver: %s)", vm_nictype1.c_str());
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--nic1 bridged ";
        if (is_virtualbox_version_newer(6, 9, 99)) {
            if (vm_nictype1.size()) {
                command += "--nic-type1 \"";
                command += vm_nictype1;
                command += "\" ";
            }
            command += "--cable-connected1 off ";
        } else {
            if (vm_nictype1.size()) {
                command += "--nictype1 \"";
                command += vm_nictype1;
                command += "\" ";
            }
            command += "--cableconnected1 off ";
        }

        retval = vbm_popen(command, output, "set bridged mode");
        if (retval) return retval;

        get_default_network_interface(default_interface);

        vboxlog_msg("Setting Bridged Interface. (%s)", default_interface.c_str());
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--bridgeadapter1 \"";
        command += default_interface;
        command += "\" ";

        retval = vbm_popen(command, output, "set bridged interface");
        if (retval) return retval;
    } else {
        vboxlog_msg("Setting Network Configuration for NAT. (Driver: %s)", vm_nictype1.c_str());
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--nic1 nat ";
        if (is_virtualbox_version_newer(6, 9, 99)) {
            if (vm_nictype1.size()) {
                command += "--nic-type1 \"";
                command += vm_nictype1;
                command += "\" ";
            }
            command += "--cable-connected1 off ";
            command += "--nat-dns-proxy1 on ";
            if (enable_nat_dns_host_resolver) {
                command += "--nat-dns-host-resolver1 on ";
            }
        } else {
            if (vm_nictype1.size()) {
                command += "--nictype1 \"";
                command += vm_nictype1;
                command += "\" ";
            }
            command += "--cableconnected1 off ";
            command += "--natdnsproxy1 on ";
            if (enable_nat_dns_host_resolver) {
                command += "--natdnshostresolver1 on ";
            }
        }

        retval = vbm_popen(command, output, "set nat mode");
        if (retval) return retval;
    }

    if (enable_network) {
        vboxlog_msg("Enabling VM Network Access.");
        command  = "modifyvm \"" + vm_name + "\" ";
        if (is_virtualbox_version_newer(6, 9, 99)) {
            command += "--cable-connected1 on ";
        } else {
            command += "--cableconnected1 on ";
        }
        retval = vbm_popen(command, output, "enable network");
        if (retval) return retval;
    } else {
        vboxlog_msg("Disabling VM Network Access.");
        command  = "modifyvm \"" + vm_name + "\" ";
        if (is_virtualbox_version_newer(6, 9, 99)) {
            command += "--cable-connected1 off ";
        } else {
            command += "--cableconnected1 off ";
        }
        retval = vbm_popen(command, output, "disable network");
        if (retval) return retval;
    }

    // Tweak the VM's USB Configuration
    //
    vboxlog_msg("Disabling USB Support for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--usb off ";

    vbm_popen(command, output, "modifyusb", false, false);

    // Tweak the VM's COM Port Support
    //
    vboxlog_msg("Disabling COM Port Support for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--uart1 off ";
    command += "--uart2 off ";

    vbm_popen(command, output, "modifycom", false, false);

#ifndef __APPLE__
    // Tweak the VM's LPT Port Support
    //
    vboxlog_msg("Disabling LPT Port Support for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--lpt1 off ";
    command += "--lpt2 off ";

    vbm_popen(command, output, "modifylpt", false, false);
#endif

    // Tweak the VM's Audio Support
    //
    vboxlog_msg("Disabling Audio Support for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    if (is_virtualbox_version_newer(7, 0, 4)) {
        command += "--audio-enabled off ";
    } else {
        command += "--audio none ";
    }


    vbm_popen(command, output, "modifyaudio", false, false);

    // Tweak the VM's Clipboard Support
    //
    vboxlog_msg("Disabling Clipboard Support for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--clipboard disabled ";

    vbm_popen(command, output, "modifyclipboard", false, false);

    // Tweak the VM's Drag & Drop Support
    //
    vboxlog_msg("Disabling Drag and Drop Support for VM.");
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--draganddrop disabled ";

    vbm_popen(command, output, "modifydragdrop", false, false);

    // Check to see if the processor supports hardware acceleration
    // for virtualization.
    // If it doesn't, disable the use of it in VirtualBox.
    // Multi-core jobs require hardware
    // acceleration and actually override this setting.
    //
    if (!strstr(aid.host_info.p_features, "vmx") && !strstr(aid.host_info.p_features, "svm")) {
        vboxlog_msg("Hardware acceleration CPU extensions not detected. Disabling VirtualBox hardware acceleration support.");
        disable_acceleration = true;
    }
    if (strstr(aid.host_info.p_features, "hypervisor")) {
        vboxlog_msg("Running under Hypervisor. Disabling VirtualBox hardware acceleration support.");
        disable_acceleration = true;
    }

    if (is_boinc_client_version_newer(7, 2, 16)) {
        if (aid.vm_extensions_disabled) {
            vboxlog_msg("Hardware acceleration failed with previous execution. Disabling VirtualBox hardware acceleration support.");
            disable_acceleration = true;
        }

    } else {
        if (vm_cpu_count == "1") {
            // Keep this around for older clients.
            // Removing this for older clients might
            // lead to a machine that will only return crashed VM reports.
            vboxlog_msg("Legacy fallback configuration detected. Disabling VirtualBox hardware acceleration support.");
            vboxlog_msg("NOTE: Upgrading to BOINC 7.2.16 or better may re-enable hardware acceleration.");
            disable_acceleration = true;
        }
    }

    if (boinc_is_standalone()) {
        disable_acceleration = false;
    }

    // Only allow disabling of hardware acceleration on 32-bit VM types,
    // 64-bit VM types require it.
    //
    if (os_name.find("_64") == string::npos) {
        if (disable_acceleration) {
            vboxlog_msg("Disabling hardware acceleration support for virtualization.");
            command  = "modifyvm \"" + vm_name + "\" ";
            command += "--hwvirtex off ";

            retval = vbm_popen(command, output, "VT-x/AMD-V support");
            if (retval) return retval;
        }
    } else if (os_name.find("_64") != string::npos) {
        if (disable_acceleration) {
            vboxlog_msg("ERROR: Invalid configuration.  VM type requires acceleration but the current configuration cannot support it.");
            return ERR_INVALID_PARAM;
        }
    }

    // Add storage controller to VM
    // See: http://www.virtualbox.org/manual/ch08.html#vboxmanage-storagectl
    // See: http://www.virtualbox.org/manual/ch05.html#iocaching
    //
    vboxlog_msg("Adding storage controller(s) to VM.");
    command  = "storagectl \"" + vm_name + "\" ";
    command += "--name \"Hard Disk Controller\" ";
    command += "--add \"" + vm_disk_controller_type + "\" ";
    command += "--controller \"" + vm_disk_controller_model + "\" ";
    if (
            (vm_disk_controller_type == "sata") || (vm_disk_controller_type == "SATA") ||
            (vm_disk_controller_type == "scsi") || (vm_disk_controller_type == "SCSI") ||
            (vm_disk_controller_type == "sas") || (vm_disk_controller_type == "SAS")
       ) {
        command += "--hostiocache off ";
    }
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
        vboxlog_msg("Adding virtual ISO 9660 disk drive to VM. (%s)", iso_image_filename.c_str());
        command  = "storageattach \"" + vm_name + "\" ";
        command += "--storagectl \"Hard Disk Controller\" ";
        command += "--port 0 ";
        command += "--device 0 ";
        command += "--type dvddrive ";
        command += "--medium \"" + slot_dir_path + "/" + iso_image_filename + "\" ";

        retval = vbm_popen(command, output, "storage attach (ISO 9660 image)");
        if (retval) return retval;

        // Add guest additions to the VM
        //
        if (virtualbox_guest_additions.size()) {
            vboxlog_msg("Adding VirtualBox Guest Additions to VM.");
            command  = "storageattach \"" + vm_name + "\" ";
            command += "--storagectl \"Hard Disk Controller\" ";
            command += "--port 2 ";
            command += "--device 0 ";
            command += "--type dvddrive ";
            command += "--medium \"" + virtualbox_guest_additions + "\" ";

            retval = vbm_popen(command, output, "storage attach (guest additions image)");
            if (retval) return retval;
        }

        // Add a virtual cache disk drive to VM
        //
        if (enable_cache_disk){
            vboxlog_msg("Adding virtual cache disk drive to VM. (%s)", cache_disk_filename.c_str());
            command  = "storageattach \"" + vm_name + "\" ";
            command += "--storagectl \"Hard Disk Controller\" ";
            command += "--port 1 ";
            command += "--device 0 ";
            command += "--type hdd ";
            command += "--setuuid \"\" ";
            command += "--medium \"" + slot_dir_path + "/" + cache_disk_filename + "\" ";

            retval = vbm_popen(command, output, "storage attach (cached disk)");
            if (retval) return retval;
        }

    } else {

        // Adding virtual hard drive to VM
        //
        string command_fix_part;

        command_fix_part  = "storageattach \"" + vm_name + "\" ";
        command_fix_part += "--storagectl \"Hard Disk Controller\" ";
        command_fix_part += "--port 0 ";
        command_fix_part += "--device 0 ";
        command_fix_part += "--type hdd ";

        if (!multiattach_vdi_file.size()) {
            // the traditional method:
            // copy the vdi file from the projects dir to the slots dir and rename it vm_image.vdi
            // each copy must get a new (random) UUID
            //
            vboxlog_msg("Adding virtual disk drive to VM. (%s)", image_filename.c_str());
            command  = command_fix_part;
            command += "--setuuid \"\" ";
            command += "--medium \"" + slot_dir_path + "/" + image_filename + "\" ";

            retval = vbm_popen(command, output, "storage attach (fixed disk)");
            if (retval) return retval;
        } else {
            // Use MultiAttach mode and differencing images
            // See: https://www.virtualbox.org/manual/ch05.html#hdimagewrites
            //      https://www.virtualbox.org/manual/ch05.html#diffimages
            // the vdi file downloaded to the projects dir becomes the parent (read only)
            // each task gets it's own differencing image (writable)
            // differencing images are written to the VM's snapshot folder
            //
            string medium_file = aid.project_dir;
            medium_file += "/" + multiattach_vdi_file;

            vboxlog_msg("Adding virtual disk drive to VM. (%s)", multiattach_vdi_file.c_str());

#ifdef _WIN32
            HANDLE fd_race_mitigator = NULL;
#else
            int fd_race_mitigator = 0;
#endif
            int retry_count = 0;
            bool log_error = false;
            bool vbox_bug_mitigation = false;
            string lock_name = "";

            retval = set_race_mitigation_lock(fd_race_mitigator, lock_name, medium_file);
            if (retval) {
                save_retval = retval;
                vboxlog_msg("Could not set race mitigation lock in 'create_vm'.");
                return save_retval;
            }

            do {
                string set_new_uuid = "";
                string type_line = "";
                size_t type_start;
                size_t type_end;

                command = "showhdinfo \"" + medium_file + "\" ";

                retval = vbm_popen(command, output, "check if parent hdd is registered", false);
                if (retval) {
                    save_retval = retval;
                    // showhdinfo implicitly registers unregistered hdds.
                    // Hence, this has to be handled first.
                    //
                    if ((output.rfind("VBoxManage: error:", 0) != string::npos) &&
                        (output.find("Cannot register the hard disk") != string::npos) &&
                        (output.find("because a hard disk") != string::npos) &&
                        (output.find("with UUID") != string::npos) &&
                        (output.find("already exists") != string::npos)) {
                            // May happen if the project admin didn't set a new UUID.
                            set_new_uuid = "--setuuid \"\" ";

                            vboxlog_msg("Disk UUID conflicts with an already existing disk.\nWill set a new UUID for '%s'.\nThe project admin should be informed to do this server side running:\nvboxmanage clonemedium <inputfile> <outputfile>\n",
                                multiattach_vdi_file.c_str()
                            );
                    } else {
                        // other errors
                        remove_race_mitigation_lock(fd_race_mitigator, lock_name);
                        vboxlog_msg("Error in check if parent hdd is registered.\nCommand:\n%s\nOutput:\n%s",
                            command.c_str(),
                            output.c_str()
                        );
                        return save_retval;
                    }
                }

                // Output from showhdinfo should look a little like this:
                //   UUID:           c119bcaf-636c-41f6-86c9-384739a31339
                //   Parent UUID:    base
                //   State:          created
                //   Type:           multiattach
                //   Location:       C:\Users\romw\VirtualBox VMs\test2\test2.vdi
                //   Storage format: VDI
                //   Format variant: dynamic default
                //   Capacity:       2048 MBytes
                //   Size on disk:   2 MBytes
                //   Encryption:     disabled
                //   Property:       AllocationBlockSize=1048576
                //   Child UUIDs:    dcb0daa5-3bf9-47cb-bfff-c65e74484615
                //

                type_line = output;
                type_start = type_line.find("\nType: ") + 1;
                type_end   = type_line.find("\n", type_start) - type_start;
                type_line  = type_line.substr(type_start, type_end);

                if (type_line.find("multiattach") == string::npos) {
                    // Parent hdd is not (yet) of type multiattach.
                    // Vdi files can't be registered and set to multiattach mode within 1 step.
                    // They must first be attached to a VM in normal mode, then detached from the VM

                    command  = command_fix_part;
                    command += set_new_uuid + "--medium \"" + medium_file + "\" ";

                    retval = vbm_popen(command, output, "register parent vdi");
                    if (retval) {
                        save_retval = retval;
                        remove_race_mitigation_lock(fd_race_mitigator, lock_name);
                        return save_retval;
                    }

                    command  = command_fix_part;
                    command += "--medium none ";

                    retval = vbm_popen(command, output, "detach parent vdi");
                    if (retval) {
                        save_retval = retval;
                        remove_race_mitigation_lock(fd_race_mitigator, lock_name);
                        return save_retval;
                    }
                    // the vdi file is now registered and ready
                    // to be attached in multiattach mode
                }

                do {
                    command  = command_fix_part;
                    command += "--mtype multiattach ";
                    command += "--medium \"" + medium_file + "\" ";

                    retval = vbm_popen(command, output, "storage attach (fixed disk - multiattach mode)", log_error);
                    if (retval) {
                        save_retval = retval;
                        // VirtualBox occasionally writes the 'MultiAttach'
                        // attribute to the disk entry in VirtualBox.xml
                        // although this is not allowed there.
                        // As a result all VMs trying to connect that disk fail.
                        // The error needs to be cleaned here
                        // to allow vboxwrapper to succeed even with
                        // uncorrected VirtualBox versions.
                        //
                        // After cleanup attaching the disk should be tried again.

                        if ((retry_count < 1) &&
                            (output.find("Cannot attach medium") != string::npos) &&
                            (output.find("the media type") != string::npos) &&
                            (output.find("MultiAttach") != string::npos) &&
                            (output.find("can only be attached to machines that were created with VirtualBox 4.0 or later") != string::npos)) {
                                // try to deregister the medium from the global media store
                                //
                                retval = remove_vbox_disk_orphans(medium_file.c_str());
                                if (retval) {
                                    save_retval = retval;
                                    remove_race_mitigation_lock(fd_race_mitigator, lock_name);
                                    return save_retval;
                                }

                                retry_count++;
                                log_error = true;
                                break;
                        }

                        if (retry_count >= 1) {
                            // in case of other errors or if retry also failed
                            //
                            remove_race_mitigation_lock(fd_race_mitigator, lock_name);
                            vboxlog_msg("Error in storage attach (fixed disk - multiattach mode).\nCommand:\n%s\nOutput:\n%s",
                                command.c_str(),
                                output.c_str()
                                );
                            return save_retval;
                        }

                        retry_count++;
                        log_error = true;

                    } else {
                        vbox_bug_mitigation = true;
                        break;
                    }
                }
                while (true);
            }
            while (!vbox_bug_mitigation);
            remove_race_mitigation_lock(fd_race_mitigator, lock_name);
        }


        // Add guest additions to the VM
        //
        if (virtualbox_guest_additions.size()) {
            vboxlog_msg("Adding VirtualBox Guest Additions to VM.");
            command  = "storageattach \"" + vm_name + "\" ";
            command += "--storagectl \"Hard Disk Controller\" ";
            command += "--port 1 ";
            command += "--device 0 ";
            command += "--type dvddrive ";
            command += "--medium \"" + virtualbox_guest_additions + "\" ";

            retval = vbm_popen(command, output, "storage attach (guest additions image)");
            if (retval) return retval;
        }
    }

    // Adding virtual floppy disk drive to VM
    //
    if (enable_floppyio) {

        // Put in place the FloppyIO abstraction
        //
        // NOTE: This creates the floppy.img file at runtime for use by the VM.
        //
        pFloppy = new FloppyIONS::FloppyIO(floppy_image_filename.c_str());
        if (!pFloppy->ready()) {
            vboxlog_msg("Creating virtual floppy image failed.");
            vboxlog_msg("Error Code '%d' Error Message '%s'", pFloppy->error, pFloppy->errorStr.c_str());
            return ERR_FWRITE;
        }

        vboxlog_msg("Adding virtual floppy disk drive to VM.");
        command  = "storageattach \"" + vm_name + "\" ";
        command += "--storagectl \"Floppy Controller\" ";
        command += "--port 0 ";
        command += "--device 0 ";
        command += "--medium \"" + slot_dir_path + "/" + floppy_image_filename + "\" ";

        retval = vbm_popen(command, output, "storage attach (floppy disk)");
        if (retval) return retval;
    }

    // Add network bandwidth throttle group
    //
    if (is_virtualbox_version_newer(4, 2, 0)) {
        vboxlog_msg("Adding network bandwidth throttle group to VM. (Defaulting to 1024GB)");
        command  = "bandwidthctl \"" + vm_name + "\" ";
        command += "add \"" + vm_name + "_net\" ";
        command += "--type network ";
        command += "--limit 1024G";
        command += " ";

        retval = vbm_popen(command, output, "network throttle group (add)");
        if (retval) return retval;
    }

    // Enable the network adapter if a network connection is required.
    //
    if (enable_network) {
        // set up port forwarding
        //
        if (pf_guest_port) {
            VBOX_PORT_FORWARD pf;
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
            VBOX_PORT_FORWARD& pf = port_forwards[i];

            vboxlog_msg("forwarding host port %d to guest port %d", pf.host_port, pf.guest_port);

            // Add new firewall rule
            //
            snprintf(buf, sizeof(buf), ",tcp,%s,%d,,%d",
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
        vboxlog_msg("Enabling remote desktop for VM.");
        if (!is_extpack_installed()) {
            vboxlog_msg("Required extension pack not installed, remote desktop not enabled.");
        } else {
            retval = boinc_get_port(false, rd_host_port);
            if (retval) return retval;

            snprintf(buf, sizeof(buf), "%d", rd_host_port);
            command  = "modifyvm \"" + vm_name + "\" ";
            command += "--vrde on ";
            command += "--vrdeextpack default ";
            command += "--vrdeauthlibrary default ";
            command += "--vrdeauthtype null ";
            command += "--vrdeport " + string(buf) + " ";
            if (is_virtualbox_version_newer(7, 0, 99)) {
                command += "--vrde-property \"Security/Method=RDP\" ";
            }

            retval = vbm_popen(command, output, "remote desktop");
            if (retval) return retval;
        }
    }

    // share slot/ or slot/shared
    //
    if (enable_shared_directory || share_slot_dir) {
        vboxlog_msg("Enabling shared directory for VM.");
        command  = "sharedfolder add \"" + vm_name + "\" ";
        command += "--name \"shared\" ";
        if (share_slot_dir) {
            command += "--hostpath \"" + slot_dir_path + "\"";
        } else {
            command += "--hostpath \"" + slot_dir_path + "/shared\"";
        }
        retval = vbm_popen(command, output, "enable shared dir");
        if (retval) return retval;
    }

    // share project/ or project/scratch
    //
    if (enable_scratch_directory || share_project_dir) {
        vboxlog_msg("Enabling shared project directory for VM.");
        command  = "sharedfolder add \"" + vm_name + "\" ";
        if (share_project_dir) {
            command += "--name \"project\" ";
            command += "--hostpath \"" + project_dir_path + "\"";
        } else {
            command += "--name \"scratch\" ";
            command += "--hostpath \"" + project_dir_path + "/scratch\"";
        }

        retval = vbm_popen(command, output, "enable shared project dir");
        if (retval) return retval;
    }

    return 0;
}

int VBOX_VM::register_vm() {
    string command;
    string output;
    int retval;

    vboxlog_msg("Register VM. (%s, slot#%d)", vm_master_name.c_str(), aid.slot);

    // Reset VM name in case it was changed while deregistering a stale VM
    //
    vm_name = vm_master_name;

    // Register the VM
    //
    command  = "registervm ";
    command += "\"" + slot_dir_path + "/" + vm_name + "/" + vm_name + ".vbox\" ";

    retval = vbm_popen(command, output, "register");
    if (retval) return retval;

    return 0;
}

int VBOX_VM::deregister_vm(bool delete_media) {
    int retval;
    string command;
    string output;
    string lock_name = "";
#ifdef _WIN32
    HANDLE fd_race_mitigator = NULL;
#else
    int fd_race_mitigator = 0;
#endif

    vboxlog_msg("Deregistering VM. (%s, slot#%d)", vm_name.c_str(), aid.slot);

    // Cleanup any left-over snapshots
    //
    cleanup_snapshots(true);

    // Delete network bandwidth throttle group
    //
    if (is_virtualbox_version_newer(4, 2, 0)) {
        vboxlog_msg("Removing network bandwidth throttle group from VM.");
        command  = "bandwidthctl \"" + vm_name + "\" ";
        command += "remove \"" + vm_name + "_net\" ";

        vbm_popen(command, output, "network throttle group (remove)", false, false);
    }

    if (enable_floppyio) {
        command  = "storagectl \"" + vm_name + "\" ";
        command += "--name \"Floppy Controller\" ";
        command += "--remove ";

        vbm_popen(command, output, "deregister storage controller (floppy disk)", false, false);
    }

    // Next, delete VM
    // This automatically deletes child disk images connected to the VM.
    //
    if (multiattach_vdi_file.size()) {
        string medium_file  = aid.project_dir;
        medium_file += "/" + multiattach_vdi_file;

        retval = set_race_mitigation_lock(fd_race_mitigator, lock_name, medium_file);
        if (retval) {
            vboxlog_msg("Could not set race mitigation lock in 'deregister_vm'.");
            vboxlog_msg("Warning: Will continue without a lock.");
        }
    }

    vboxlog_msg("Removing VM from VirtualBox.");
    command  = "unregistervm \"" + vm_name + "\" ";
    command += "--delete ";

    vbm_popen(command, output, "delete VM", false, false);
    remove_race_mitigation_lock(fd_race_mitigator, lock_name);

    // Lastly delete medium(s) from Virtual Box Media Registry
    //
    if (enable_isocontextualization) {
        vboxlog_msg("Removing virtual ISO 9660 disk from VirtualBox.");
        command  = "closemedium dvd \"" + slot_dir_path + "/" + iso_image_filename + "\" ";
        if (delete_media) {
            command += "--delete ";
        }
        vbm_popen(command, output, "remove virtual ISO 9660 disk", false, false);

        if (enable_cache_disk) {
            vboxlog_msg("Removing virtual cache disk from VirtualBox.");
            command  = "closemedium disk \"" + slot_dir_path + "/" + cache_disk_filename + "\" ";
            if (delete_media) {
                command += "--delete ";
            }

            vbm_popen(command, output, "remove virtual cache disk", false, false);
        }
    }

    if (enable_floppyio) {
        vboxlog_msg("Removing virtual floppy disk from VirtualBox.");
        command  = "closemedium floppy \"" + slot_dir_path + "/" + floppy_image_filename + "\" ";
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
    size_t uuid_start;
    size_t uuid_end;
    int retval;

    // Output from showhdinfo should look a little like this:
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
    if (enable_isocontextualization) {
        command  = "showhdinfo \"" + slot_dir_path + "/" + cache_disk_filename + "\" ";
        retval = vbm_popen(command, output, "get HDD info");
        if (retval) return retval;

        uuid_start = output.find("(UUID: ");
        if (uuid_start != string::npos) {
            // We can parse the virtual machine ID from the output
            uuid_start += 7;
            uuid_end = output.find(")", uuid_start);
            vm_name = output.substr(uuid_start, uuid_end - uuid_start);

            // Deregister stale VM by UUID
            return deregister_vm(false);
        } else {
            command  = "closemedium dvd \"" + slot_dir_path + "/" + iso_image_filename + "\" ";
            // coverity[CHECKED_RETURN]
            vbm_popen(command, output, "remove virtual ISO 9660 disk", false);
            if (enable_cache_disk) {
                 command  = "closemedium disk \"" + slot_dir_path + "/" + cache_disk_filename + "\" ";
                 // coverity[CHECKED_RETURN]
                 vbm_popen(command, output, "remove virtual cache disk", false);
            }
        }
    } else {
        command  = "showhdinfo \"" + slot_dir_path + "/" + image_filename + "\" ";
        retval = vbm_popen(command, output, "get HDD info");
        if (retval) return retval;

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
            command  = "closemedium disk \"" + slot_dir_path + "/" + image_filename + "\" ";
            vbm_popen(command, output, "remove virtual disk", false, false);
            if (enable_floppyio) {
                command  = "closemedium floppy \"" + slot_dir_path + "/" + floppy_image_filename + "\" ";
                vbm_popen(command, output, "remove virtual floppy disk", false, false);
            }
        }
    }
    return 0;
}

int VBOX_VM::poll(bool log_state) {
    int retval = ERR_EXEC;
    string command;
    string output;
    string::iterator iter;
    string vmstate;
    static string vmstate_old = "poweredoff";

    //
    // Is our environment still sane?
    //
#ifdef _WIN32
    if (started_successfully && vm_pid_handle && !process_exists(vm_pid_handle)) {
        vboxlog_msg("Status Report: virtualbox.exe/vboxheadless.exe is no longer running.");
    }
#else
    if (started_successfully && vm_pid && !process_exists(vm_pid)) {
        vboxlog_msg("Status Report: virtualbox/vboxheadless is no longer running.");
    }
#endif

    // What state is the VM in?
    //
    vmstate = read_vm_log();
    if (vmstate != "Error in parsing the log file") {

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
        } else if (vmstate == "deletingsnapshotonline" || vmstate == "deletingsnapshotlive") {
            online = true;
            saving = false;
            restoring = false;
            suspended = false;
            crashed = false;
        } else if (vmstate == "deletingsnapshotpaused" || vmstate == "deletingsnapshotlivepaused") {
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
                vboxlog_msg("VM is no longer is a running state. It is in '%s'.", vmstate.c_str());
            }
        }
        if (log_state && (vmstate_old != vmstate)) {
            vboxlog_msg("VM state change detected. (old = '%s', new = '%s')", vmstate_old.c_str(), vmstate.c_str());
            vmstate_old = vmstate;
        }

        retval = BOINC_SUCCESS;
    } else {
        vboxlog_msg("Error in parsing the log file");
    }
    return retval;
}

int VBOX_VM::poll2(bool log_state) {
    int retval = ERR_EXEC;
    string command;
    string output;
    string::iterator iter;
    string vmstate;
    static string vmstate_old = "poweroff";
    size_t vmstate_start;
    size_t vmstate_end;

    // Is our environment still sane?
    //
#ifdef _WIN32
    if (started_successfully && vm_pid_handle && !process_exists(vm_pid_handle)) {
        vboxlog_msg("Status Report: virtualbox.exe/vboxheadless.exe is no longer running.");
    }
#else
    if (started_successfully && vm_pid && !process_exists(vm_pid)) {
        vboxlog_msg("Status Report: virtualbox/vboxheadless is no longer running.");
    }
#endif

    //
    // What state is the VM in?
    //

    command = "showvminfo \"" + vm_name + "\" ";
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
                    vboxlog_msg(
                        "VM is no longer is a running state. It is in '%s'.",
                        vmstate.c_str()
                    );
                }
            }
            if (log_state && (vmstate_old != vmstate)) {
                vboxlog_msg(
                    "VM state change detected. (old = '%s', new = '%s')",
                     vmstate_old.c_str(), vmstate.c_str()
                );
                vmstate_old = vmstate;
            }

            retval = BOINC_SUCCESS;
        }
    }

    // Grab a snapshot of the latest log file.
    // Avoids multiple queries across several functions.
    //
    get_vm_log(vm_log);

    // Dump any new VM Guest Log entries
    //
    dump_vmguestlog_entries();

    return retval;
}

int VBOX_VM::start() {
    int retval;
    string command;
    string output;
    int timeout = 0;

    log_pointer = 0;
    vboxlog_msg(
        "Starting VM using VBoxManage interface. (%s, slot#%d)",
        vm_name.c_str(), aid.slot
    );

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

    if (BOINC_SUCCESS == retval) {
        vboxlog_msg("Successfully started VM. (PID = '%d')", vm_pid);
        started_successfully = true;
    } else {
        vboxlog_msg("VM failed to start.");
    }

    return retval;
}

int VBOX_VM::stop() {
    string command;
    string output;
    double timeout;
    int retval = 0;

    vboxlog_msg("Stopping VM.");
    if (online) {
        command = "controlvm \"" + vm_name + "\" savestate";
        retval = vbm_popen(command, output, "stop VM", true, false);

        // Wait for up to 5 minutes for the VM to switch states.
        // A system under load can take a while.
        // Since the poll function can wait for up to 45 seconds
        // to execute a command we need to make this time based instead
        // of iteration based.
        //
        if (!retval) {
            timeout = dtime() + 300;
            do {
            poll(false);
                if (!online && !saving) break;
                boinc_sleep(1.0);
            } while (timeout >= dtime());
        }

        if (!online) {
            vboxlog_msg("Successfully stopped VM.");
            retval = BOINC_SUCCESS;
        } else {
            vboxlog_msg("VM did not stop when requested.");

            // Attempt to terminate the VM
#ifdef _WIN32
            retval = kill_process(vm_pid_handle);
#else
            retval = kill_process(vm_pid);
#endif
            if (retval) {
                vboxlog_msg("VM was NOT successfully terminated.");
            } else {
                vboxlog_msg("VM was successfully terminated.");
            }
        }
    }

    return retval;
}

int VBOX_VM::poweroff() {
    string command;
    string output;
    double timeout;
    int retval = 0;

    vboxlog_msg("Powering off VM.");
    if (online) {
        command = "controlvm \"" + vm_name + "\" poweroff";
        retval = vbm_popen(command, output, "poweroff VM", true, false);

        // Wait for up to 5 minutes for the VM to switch states.
        // A system under load can take a while.
        // Since the poll function can wait for up to 45 seconds
        // to execute a command we need to make this time based instead
        // of iteration based.
        //
        if (!retval) {
            timeout = dtime() + 300;
            do {
                poll(false);
                if (!online && !saving) break;
                boinc_sleep(1.0);
            } while (timeout >= dtime());
        }

        if (!online) {
            vboxlog_msg("Successfully stopped VM.");
            retval = BOINC_SUCCESS;
        } else {
            vboxlog_msg("VM did not power off when requested.");

            // Attempt to terminate the VM
#ifdef _WIN32
            retval = kill_process(vm_pid_handle);
#else
            retval = kill_process(vm_pid);
#endif
            if (retval) {
                vboxlog_msg("VM was NOT successfully terminated.");
            } else {
                vboxlog_msg("VM was successfully terminated.");
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

int VBOX_VM::capture_screenshot() {
    if (enable_screenshots_on_error) {
        if (is_virtualbox_version_newer(5, 0, 0)) {
            string command;
            string output;
            int retval = BOINC_SUCCESS;

            vboxlog_msg("Capturing screenshot.");

            command = "controlvm \"" + vm_name + "\" ";
            command += "keyboardputscancode 39";
            vbm_popen(command, output, "put scancode", true, true, 0);
            boinc_sleep(1);

            command = "controlvm \"" + vm_name + "\" ";
            command += "screenshotpng \"";
            command += slot_dir_path;
            command += "/";
            command += SCREENSHOT_FILENAME;
            command += "\"";
            retval = vbm_popen(command, output, "capture screenshot", true, true, 0);
            if (retval) return retval;

            vboxlog_msg("Screenshot completed.");
        }
    }
    return 0;
}

int VBOX_VM::create_snapshot(double elapsed_time) {
    string command;
    string output;
    char buf[256];
    int retval = BOINC_SUCCESS;

    if (disable_automatic_checkpoints) return BOINC_SUCCESS;

    vboxlog_msg("Creating new snapshot for VM.");

    // Pause VM - Try and avoid the live snapshot and trigger an online
    // snapshot instead.
    pause();

    // Create new snapshot
    snprintf(buf, sizeof(buf), "%d", (int)elapsed_time);
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
    cleanup_snapshots(false);

    vboxlog_msg("Checkpoint completed.");

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
    // Traverse the list from newest to oldest.
    // Otherwise we end up with an error:
    //   VBoxManage.exe: error: Snapshot operation failed
    //   VBoxManage.exe: error: Hard disk 'C:\ProgramData\BOINC\slots\23\vm_image.vdi' has
    //     more than one child hard disk (2)
    //

    // Prepend a space and line feed to the output
    // since we are going to traverse it backwards
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

            vboxlog_msg("Deleting stale snapshot.");

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
    int retval = BOINC_SUCCESS;

    if (disable_automatic_checkpoints) return BOINC_SUCCESS;

    vboxlog_msg("Restore from previously saved snapshot.");

    command = "snapshot \"" + vm_name + "\" ";
    command += "restorecurrent ";
    retval = vbm_popen(command, output, "restore current snapshot", true, false, 0);
    if (retval) return retval;

    vboxlog_msg("Restore completed.");

    return retval;
}

void VBOX_VM::dump_hypervisor_status_reports() {
}

int VBOX_VM::is_registered() {
    string command;
    string output;
    string needle;
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
    vboxlog_msg(
        "Error in registration for VM: %d\nArguments:\n%s\nOutput:\n%s",
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
// Luckily both of the above conditions can be detected by attempting to detect the host information
// via vboxmanage and it is cross platform.
//
bool VBOX_VM::is_system_ready(string& message) {
    string command;
    string output;
    int retval;
    bool rc = false;

    command  = "list hostinfo ";
    retval = vbm_popen(command, output, "host info");
    if (BOINC_SUCCESS == retval) {
        rc = true;
    }

    if (output.size() == 0) {
        vboxlog_msg("WARNING: Communication with VM Hypervisor failed. (Possibly Out of Memory).");
        message = "Communication with VM Hypervisor failed. (Possibly Out of Memory).";
        rc = false;
    }

    if (output.find("Processor count:") == string::npos) {
        vboxlog_msg("WARNING: Communication with VM Hypervisor failed.");
        message = "Communication with VM Hypervisor failed.";
        rc = false;
    }

    if ((output.find("WARNING: The vboxdrv kernel module is not loaded.") != string::npos)
        || (output.find("WARNING: The VirtualBox kernel modules are not loaded.") != string::npos)
    ){
        vboxlog_msg("WARNING: The VirtualBox kernel modules are not loaded.");
        vboxlog_msg("WARNING: Please update/recompile VirtualBox kernel drivers.");
        message = "Please update/recompile VirtualBox kernel drivers.";
        rc = false;
    }

    if ((output.find("Warning: program compiled against ") != string::npos)
        && (output.find(" using older ") != string::npos)
    ){
        vboxlog_msg("WARNING: VirtualBox incompatible dependencies detected.");
        message = "Please update/reinstall VirtualBox";
        rc = false;
    }

    return rc;
}

bool VBOX_VM::is_disk_image_registered() {
    string command;
    string output;

    if (!multiattach_vdi_file.size()) {
        command = "showhdinfo \"" + slot_dir_path + "/" + image_filename + "\" ";
        if (vbm_popen(command, output, "hdd registration", false, false) == 0) {
            if ((output.find("VBOX_E_FILE_ERROR") == string::npos)
                && (output.find("VBOX_E_OBJECT_NOT_FOUND") == string::npos)
                && (output.find("does not match the value") == string::npos)
            ) {
                // Error message not found in text
                return true;
            }
        }
    }

    if (enable_isocontextualization && enable_cache_disk) {
        command = "showhdinfo \"" + slot_dir_path + "/" + cache_disk_filename + "\" ";
        if (vbm_popen(command, output, "hdd registration", false, false) == 0) {
            if ((output.find("VBOX_E_FILE_ERROR") == string::npos)
                && (output.find("VBOX_E_OBJECT_NOT_FOUND") == string::npos)
                && (output.find("does not match the value") == string::npos)
            ) {
                // Error message not found in text
                return true;
            }
        }
    }

    return false;
}

bool VBOX_VM::is_extpack_installed() {
    string command;
    string output;

    command = "list extpacks";

    if (vbm_popen(command, output, "extpack detection", false, false) == 0) {
        if ((output.find("VirtualBox Extension Pack") != string::npos) && (output.find("VBoxVRDP") != string::npos)) {
            return true;
        }
    }
    return false;
}

int VBOX_VM::get_install_directory(string& install_directory) {
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
        return ERR_FREAD;
    }
    return BOINC_SUCCESS;
#else
    install_directory = "";
    return 0;
#endif
}

int VBOX_VM::get_version_information(string& version_raw, string& version_display) {
    string command;
    string output;
    int vbox_major = 0, vbox_minor = 0, vbox_release = 0;
    int retval;
    char buf[256];

    // Record the VirtualBox version information for later use.
    command = "--version ";
    retval = vbm_popen(command, output, "version check");

    if (!retval) {
        // Remove \r or \n from the output spew
        string::iterator iter = output.begin();
        while (iter != output.end()) {
            if (*iter == '\r' || *iter == '\n') {
                iter = output.erase(iter);
            } else {
                ++iter;
            }
        }

        if (3 == sscanf(output.c_str(), "%d.%d.%d", &vbox_major, &vbox_minor, &vbox_release)) {
            snprintf(
                buf, sizeof(buf),
                "%d.%d.%d",
                vbox_major, vbox_minor, vbox_release
            );
            version_raw = buf;
            snprintf(
                buf, sizeof(buf),
                "VirtualBox VboxManage Interface (Version: %d.%d.%d)",
                vbox_major, vbox_minor, vbox_release
            );
            version_display = buf;
        } else {
            vboxlog_msg("VBoxManage version raw: %s", output.c_str());
            version_raw = "Unknown";
            version_display = "VirtualBox VboxManage Interface (Version: Unknown)";
        }
    }

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

    if (!boinc_file_exists(guest_additions.c_str())) {
        guest_additions.clear();
        return ERR_NOT_FOUND;
    }

    return retval;
}

int VBOX_VM::get_default_network_interface(string& iface) {
    string command;
    string output;
    size_t if_start;
    size_t if_end;
    int retval;

    // Get the location of where the guest additions are
    command = "list bridgedifs";
    retval = vbm_popen(command, output, "default interface");

    // Output should look like this:
    // Name:            Intel(R) Ethernet Connection I217-V
    // GUID:            4b8796d6-a4ed-4752-8e8e-bf23984fd93c
    // DHCP:            Enabled
    // IPAddress:       192.168.1.19
    // NetworkMask:     255.255.255.0
    // IPV6Address:     fe80:0000:0000:0000:31c2:0053:4f50:4e64
    // IPV6NetworkMaskPrefixLength: 64
    // HardwareAddress: bc:5f:f4:ba:cc:16
    // MediumType:      Ethernet
    // Status:          Up
    // VBoxNetworkName: HostInterfaceNetworking-Intel(R) Ethernet Connection I217-V

    if_start = output.find("Name:");
    if (if_start == string::npos) {
        return ERR_NOT_FOUND;
    }
    if_start += strlen("Name:");
    if_end = output.find("\n", if_start);
    iface = output.substr(if_start, if_end - if_start);
    strip_whitespace(iface);
    if (iface.size() <= 0) {
        return ERR_NOT_FOUND;
    }

    return retval;
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
    string virtualbox_vm_log;
    string::iterator iter;
    int retval = BOINC_SUCCESS;
    string line;
    string comp = "Process ID: ";
    string pid;
    std::size_t found;


    virtualbox_vm_log = vm_master_name + "/Logs/VBox.log";

    if (boinc_file_exists(virtualbox_vm_log.c_str())) {
        std::ifstream  src(virtualbox_vm_log.c_str(), std::ios::binary);
        while (std::getline(src, line)) {
            found = line.find(comp);
            if (found != string::npos) {
                found += comp.size();
                pid = line.substr(found, string::npos);
                strip_whitespace(pid);
                if (pid.size() <= 0) {
                    return ERR_NOT_FOUND;
                }

                vm_pid = atol(pid.c_str());
                break;
            }
        }

        if (pid.size() <= 0) {
            return ERR_NOT_FOUND;
        }
    } else {
        retval = ERR_NOT_FOUND;
    }

    return retval;
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
    int retval;

    network_suspended = !enabled;

    if (enabled) {
        vboxlog_msg("Enabling network access for VM.");
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--cableconnected1 on ";

        retval = vbm_popen(command, output, "enable network");
        if (retval) return retval;
    } else {
        vboxlog_msg("Disabling network access for VM.");
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
    vboxlog_msg("Setting CPU throttle for VM. (%d%%)", percentage);
    snprintf(buf, sizeof(buf), "%d", percentage);
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
        vboxlog_msg("Setting network throttle for VM. (1024GB)");
    } else {
        vboxlog_msg("Setting network throttle for VM. (%dKB)", kilobytes);
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
            snprintf(buf, sizeof(buf), "%d", kilobytes);
            command  = "bandwidthctl \"" + vm_name + "\" ";
            command += "set \"" + vm_name + "_net\" ";
            command += "--limit ";
            command += buf;
            command += "K ";

            retval = vbm_popen(command, output, "network throttle (set)");
            if (retval) return retval;
        }
    } else {
        snprintf(buf, sizeof(buf), "%d", kilobytes);
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--nicspeed1 ";
        command += buf;
        command += " ";

        retval = vbm_popen(command, output, "network throttle");
        if (retval) return retval;
    }

    return 0;
}

void VBOX_VM::lower_vm_process_priority() {
#ifdef _WIN32
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, BELOW_NORMAL_PRIORITY_CLASS);
    }
#else
    if (vm_pid) {
        setpriority(PRIO_PROCESS, vm_pid, PROCESS_MEDIUM_PRIORITY);
    }
#endif
}

void VBOX_VM::reset_vm_process_priority() {
#ifdef _WIN32
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, NORMAL_PRIORITY_CLASS);
    }
#else
    if (vm_pid) {
        setpriority(PRIO_PROCESS, vm_pid, PROCESS_NORMAL_PRIORITY);
    }
#endif
}

bool VBOX_VM::is_hostrtc_set_to_utc() {
#ifdef _WIN32
    bool  rtc_is_set_to_utc = false;
    LONG  lReturnValue;
    HKEY  hkSetupHive;

    // If the key is present and set to "1" assume the host's rtc is set to UTC.
    // Otherwise or in case of an error assume the host's rtc is set to localtime.
    lReturnValue = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        _T("SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"),
        0,
        KEY_READ,
        &hkSetupHive
    );

    if (lReturnValue == ERROR_SUCCESS) {
        DWORD dwvalue = 0;
        DWORD dwsize = sizeof(DWORD);

        lReturnValue = RegQueryValueEx(
            hkSetupHive,
            _T("RealTimeIsUniversal"),
            NULL,
            NULL,
            (LPBYTE)&dwvalue,
            &dwsize
        );

        if (lReturnValue == ERROR_SUCCESS && dwvalue == 1) rtc_is_set_to_utc = true;
    }

    if (hkSetupHive) RegCloseKey(hkSetupHive);
    return rtc_is_set_to_utc;
#else
    // Non-Windows Systems usually set their rtc to UTC.
    return true;
#endif
}

#ifdef _WIN32
void VBOX_VM::remove_race_mitigation_lock(HANDLE& fd_race_mitigator, string& lock_name) {
    DWORD err = BOINC_SUCCESS;
    bool retval;

    if (fd_race_mitigator) {
        retval = CloseHandle(fd_race_mitigator);
        err = GetLastError();
        if (!retval) {
            vboxlog_msg("Could not remove race mitigation lock.");
            vboxlog_msg("Lockname: %s", lock_name.c_str());
            vboxlog_msg("Error: %d, %s", err, strerror(err));
        }
    }
}
#else
void VBOX_VM::remove_race_mitigation_lock(int& fd_race_mitigator, string& lock_name) {
    int err = BOINC_SUCCESS;
    int retval;

    if (fd_race_mitigator > 0) {
        retval = shm_unlink(lock_name.c_str());
        err = errno;
        if (retval) {
            vboxlog_msg("Could not remove race mitigation lock.");
            vboxlog_msg("Lockname: %s", lock_name.c_str());
            vboxlog_msg("Error: %d, %s", err, strerror(err));
        }
    }
}
#endif

#ifdef _WIN32
int VBOX_VM::set_race_mitigation_lock(HANDLE& fd_race_mitigator, string& lock_name, const string& medium_file) {
#else
int VBOX_VM::set_race_mitigation_lock(int& fd_race_mitigator, string& lock_name, const string& medium_file) {
#endif
    int attempts = 1;
    double timeout = 0.0;
    double sleep_low = 0.7;
    double sleep_high = 2.4;

    // The lock ensures that only 1 vboxwrapper instance can
    // modify a given virtual disk entry at a given time.
    // This is a must for multiattach disks since some modifications
    // need more than 1 call to VBoxManage.
    //
    // lock_name should be derived from the full path of the disk's filename.
    //
    // Darwin limits the file name size to 32 characters (including the trailing '0').
    // On POSIX we set a '/' as prefix.
    // Hence, 'lock_name' must be shorter than 15 characters.
    //
    lock_name  = "boinc_lock_";
    lock_name += md5_string(medium_file).substr(0, 16);

    // Tests with Linux on a 16c/32t computer
    // typically register 30 VMs in less than 8 s.
    //
    timeout = dtime() + 90;

#ifdef _WIN32
    DWORD err = BOINC_SUCCESS;

    while (1) {
        // Parameter #5 (size) must not be 0 if INVALID_HANDLE_VALUE is used.
        //
        fd_race_mitigator = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READONLY,
            NULL,
            1,
            (LPTSTR)lock_name.c_str()
        );
        err = GetLastError();

        if (!err) {
            // Successfully set a fresh lock.
            // No error implies we also have a valid handle.
            //
            if (attempts > 1) {
                vboxlog_msg("Attempts: %d", attempts);
            }
            break;
        } else {
            // If we got a handle, it must not be used.
            // Close it immediately.
            //
            if (fd_race_mitigator) {
                CloseHandle(fd_race_mitigator);
                fd_race_mitigator = NULL;
            }

            if (err == ERROR_ALREADY_EXISTS) {
                // a lock exists, most likely set by another vboxwrapper
                //
                if (dtime() >= timeout) {
                    // Either the lock is stale
                    // or far too many VMs are starting concurrently.
                    //
                    vboxlog_msg("Could not set race mitigation lock.");
                    vboxlog_msg("Lockname: '%s'", lock_name.c_str());
                    vboxlog_msg("Error: ERR_TIMEOUT");
                    vboxlog_msg("Attempts: %d", attempts);

                    return ERR_TIMEOUT;
                }
                boinc_sleep(sleep_low + sleep_high * drand());
                attempts++;
            } else {
                vboxlog_msg("Could not set race mitigation lock.");
                vboxlog_msg("Lockname: '%s'", lock_name.c_str());
                vboxlog_msg("Error: %d, %s", err, strerror(err));
                vboxlog_msg("Attempts: %d", attempts);

                return ERR_FOPEN;
            }
        }
    }
#else
    int err = BOINC_SUCCESS;
    lock_name = "/" + lock_name;

    while (1) {
        fd_race_mitigator = shm_open(lock_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
        err = errno;

        if (fd_race_mitigator > 0) {
            // Successfully set a fresh lock.
            //
            if (attempts > 1) {
                vboxlog_msg("Attempts: %d", attempts);
            }
            break;
        } else {
            if ((fd_race_mitigator == -1)
                && (err == EEXIST)) {
                // a lock exists, most likely set by another vboxwrapper
                //
                if (dtime() >= timeout) {
                    // Either the lock is stale
                    // or far too many VMs are starting concurrently.
                    //
                    vboxlog_msg("Could not set race mitigation lock.");
                    vboxlog_msg("Lockname: '%s'", lock_name.c_str());
                    vboxlog_msg("Error: ERR_TIMEOUT");
                    vboxlog_msg("Attempts: %d", attempts);

                    fd_race_mitigator = 0;
                    return ERR_TIMEOUT;
                }
                boinc_sleep(sleep_low + sleep_high * drand());
                attempts++;
            } else {
                vboxlog_msg("Could not set race mitigation lock.");
                vboxlog_msg("Lockname: '%s'", lock_name.c_str());
                vboxlog_msg("Error: %d, %s", err, strerror(err));
                vboxlog_msg("Attempts: %d", attempts);

                fd_race_mitigator = 0;
                return ERR_FOPEN;
            }
        }
    }
#endif
    return BOINC_SUCCESS;
}

// To remove the parent disk entry use 'closemedium disk "<path to disk>"' instead of 'closemedium disk "<UUID>"'
// since the latter sometimes returns an error like this:
//
// VBoxManage closemedium disk "1d9935fd-37c9-4c34-946b-f6d252c6a1af"
// VBoxManage: error: The given path '1d9935fd-37c9-4c34-946b-f6d252c6a1af' is not fully qualified
// VBoxManage: error: Details: code VBOX_E_FILE_ERROR (0x80bb0004), component MediumWrap, interface IMedium, callee nsISupports
// VBoxManage: error: Context: "OpenMedium(Bstr(pszFilenameOrUuid).raw(), enmDevType, enmAccessMode, fForceNewUuidOnOpen, pMedium.asOutParam())" at line 197 of file VBoxManageDisk.cpp
//
// Output of 'VBoxManage showhdinfo "/path to/disk/parent_disk_name.vdi"' usually looks like this:
//
// UUID:           f81c0950-57ee-462e-b931-051193700d76
// Parent UUID:    base
// State:          created
// Type:           multiattach
// Location:       /path to/disk/parent_disk_name.vdi
// Storage format: VDI
// Format variant: dynamic default
// Capacity:       51200 MBytes
// Size on disk:   2 MBytes
// Encryption:     disabled
// Property:       AllocationBlockSize=1048576
// Child UUIDs:    67e34269-e52e-4957-a813-c85f72084fba
//                 5fa7905e-72e4-4f85-8405-dc6401417720
//                 7262b5e5-2b32-482a-8c6e-2ef49548740b
//
int VBOX_VM::remove_vbox_disk_orphans(string vbox_disk) {
    int retval;
    string command;
    string output;
    string needle;
    string childlist;
    string child_uuid;
    size_t childlist_start;
    string loc_line;
    size_t loc_start;
    size_t loc_end;

    command  = "showhdinfo \"" + vbox_disk + "\" ";
    retval = vbm_popen(command, output, "hdd registration", false, false);
    if (retval) {
        vboxlog_msg("Could not get disk details in 'remove_vbox_disk_orphans'.");
        return retval;
    }

    needle = "\nChild UUIDs:";

    if ((childlist_start = output.find(needle.c_str())) != string::npos) {
        size_t pos = 0;
        size_t uuid_length = 36;

        childlist = output.substr(childlist_start + needle.size());

        needle = " ";
        pos = childlist.find(needle);
        while (pos != string::npos) {
            childlist.replace(pos, 1, "");
            pos = childlist.find(needle, pos);
        }

        needle = "\n";
        pos = childlist.find(needle);
        while (pos != string::npos) {
            childlist.replace(pos, 1, "");
            pos = childlist.find(needle, pos);
        }

        pos = 0;
        while (pos < childlist.size()) {
            child_uuid = childlist.substr(pos, uuid_length);
            // recursively process child disks
            //
            retval = remove_vbox_disk_orphans(child_uuid);
            if (retval) {
                vboxlog_msg("Could not remove child disk '%s'.", child_uuid.c_str());
                return retval;
            }
            pos += uuid_length;
        }
    }

    // if no child disks are left, remove the disk itself
    //
    needle = "\nLocation:";
    if ((loc_start = output.find(needle.c_str())) != string::npos) {
        loc_start += needle.size();
        loc_start = output.find_first_not_of(" ", loc_start);
        loc_end = output.find("\n", loc_start) - loc_start;
        loc_line = output.substr(loc_start, loc_end);

        command = "closemedium disk \"" + loc_line + "\" ";
        retval = vbm_popen(command, output, "remove virtual disk", false, false);
        if (retval) {
            vboxlog_msg("Could not remove parent disk '%s'.", loc_line.c_str());
            return retval;
        }
    }
    return BOINC_SUCCESS;
}
