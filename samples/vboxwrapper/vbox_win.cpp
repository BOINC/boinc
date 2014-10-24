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

#include "boinc_win.h"
#include "win_util.h"
#include "atlcomcli.h"
#include "atlstr.h"
#include "mscom/VirtualBox.h"
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
#include "vbox_win.h"


using std::string;


const char *MachineStateToName(MachineState State) 
{ 
    switch (State) 
    { 
        case MachineState_PoweredOff: 
            return "poweroff"; 
        case MachineState_Saved: 
            return "saved"; 
        case MachineState_Aborted: 
            return "aborted"; 
        case MachineState_Teleported: 
            return "teleported"; 
        case MachineState_Running: 
            return "running"; 
        case MachineState_Paused: 
            return "paused"; 
        case MachineState_Stuck: 
            return "gurumeditation"; 
        case MachineState_LiveSnapshotting: 
            return "livesnapshotting"; 
        case MachineState_Teleporting: 
            return "teleporting"; 
        case MachineState_Starting: 
            return "starting"; 
        case MachineState_Stopping: 
            return "stopping"; 
        case MachineState_Saving: 
            return "saving"; 
        case MachineState_Restoring: 
            return "restoring"; 
        case MachineState_TeleportingPausedVM: 
            return "teleportingpausedvm"; 
        case MachineState_TeleportingIn: 
            return "teleportingin"; 
        case MachineState_RestoringSnapshot: 
            return "restoringsnapshot"; 
        case MachineState_DeletingSnapshot: 
            return "deletingsnapshot"; 
        case MachineState_DeletingSnapshotOnline: 
            return "deletingsnapshotlive"; 
        case MachineState_DeletingSnapshotPaused: 
            return "deletingsnapshotlivepaused"; 
        case MachineState_SettingUp: 
            return "settingup"; 
        default: 
            break; 
    } 
    return "unknown"; 
} 


//
// Helper function to print MSCOM exception information set on the current
// thread after a failed MSCOM method call. This function will also print
// extended VirtualBox error info if it is available.
//
void virtualbox_dump_error() {
    HRESULT rc;
    BSTR strDescription = NULL;
    char buf[256];
    IErrorInfo* pErrorInfo;

    rc = GetErrorInfo(0, &pErrorInfo);

    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error: getting error info! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
    } else {
        rc = pErrorInfo->GetDescription(&strDescription);
        if (FAILED(rc) || !strDescription) {
            fprintf(
                stderr,
                "%s Error: getting error description! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
        } else {
            fprintf(
                stderr,
                "%s Error description: %S\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                strDescription
            );
            SysFreeString(strDescription);
        }
        pErrorInfo->Release();
    }
}


VBOX_VM::VBOX_VM()
{
    VBOX_BASE::VBOX_BASE();

    m_pVirtualBox;
    m_pSession;
    m_pMachine;

    vm_pid = 0;
    vm_pid_handle = 0;
    vboxsvc_pid = 0;
    vboxsvc_pid_handle = 0;
}


VBOX_VM::~VBOX_VM()
{
    VBOX_BASE::~VBOX_BASE();

    if (vm_pid_handle) {
        CloseHandle(vm_pid_handle);
        vm_pid_handle = NULL;
    }
    if (vboxsvc_pid_handle) {
        CloseHandle(vboxsvc_pid_handle);
        vboxsvc_pid_handle = NULL;
    }
}


int VBOX_VM::initialize() {
    int rc = BOINC_SUCCESS;
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

    // Determine the VirtualBox home directory.  Overwrite as needed.
    //
    if (getenv("VBOX_USER_HOME")) {
        virtualbox_home_directory = getenv("VBOX_USER_HOME");
    } else {
        // If the override environment variable isn't specified then
        // it is based of the current users HOME directory.
        virtualbox_home_directory = getenv("USERPROFILE");
        virtualbox_home_directory += "/.VirtualBox";
    }

    // Set the location in which the VirtualBox Configuration files can be
    // stored for this instance.
    if (aid.using_sandbox || force_sandbox) {
        virtualbox_home_directory = aid.project_dir;
        virtualbox_home_directory += "/../virtualbox";

        if (!boinc_file_exists(virtualbox_home_directory.c_str())) {
            boinc_mkdir(virtualbox_home_directory.c_str());
        }

        if (!SetEnvironmentVariable("VBOX_USER_HOME", const_cast<char*>(virtualbox_home_directory.c_str()))) {
            fprintf(
                stderr,
                "%s Failed to modify the search path.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        }
    }

    // Initialize the COM subsystem.
    CoInitialize(NULL);

    // Instantiate the VirtualBox root object.
    rc = m_pVirtualBox.CoCreateInstance(CLSID_VirtualBox);
    if (!SUCCEEDED(rc))
    {
        fprintf(
            stderr,
            "%s Error creating VirtualBox instance! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        return rc;
    }

    // Create the session object.
    rc = m_pSession.CoCreateInstance(CLSID_Session);
    if (!SUCCEEDED(rc))
    {
        fprintf(
            stderr,
            "%s Error creating Session instance! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        return rc;
    }

    rc = get_version_information(virtualbox_version);
    if (rc) return rc;

    get_guest_additions(virtualbox_guest_additions);

    return rc;
}

int VBOX_VM::create_vm() {
    string command;
    string output;
    string virtual_machine_slot_directory;
    string default_interface;
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
            vm_disk_controller_model = "IntelAHCI";
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
        vm_memory_size_mb.c_str()
    );
    command  = "modifyvm \"" + vm_name + "\" ";
    command += "--memory " + vm_memory_size_mb + " ";

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
    if (network_bridged_mode) {
        fprintf(
            stderr,
            "%s Setting Network Configuration for Bridged Mode.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--nic1 bridged ";
        command += "--cableconnected1 off ";

        retval = vbm_popen(command, output, "set bridged mode");
        if (retval) return retval;

        get_default_network_interface(default_interface);
        fprintf(
            stderr,
            "%s Setting Bridged Interface. (%s)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            default_interface.c_str()
        );
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--bridgeadapter1 \"";
        command += default_interface;
        command += "\" ";

        retval = vbm_popen(command, output, "set bridged interface");
        if (retval) return retval;
    } else {
        fprintf(
            stderr,
            "%s Setting Network Configuration for NAT.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "modifyvm \"" + vm_name + "\" ";
        command += "--nic1 nat ";
        command += "--natdnsproxy1 on ";
        command += "--cableconnected1 off ";

        retval = vbm_popen(command, output, "modifynetwork");
        if (retval) return retval;
    }

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

        // Add guest additions to the VM
        //
        fprintf(
            stderr,
            "%s Adding VirtualBox Guest Additions to VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        command  = "storageattach \"" + vm_name + "\" ";
        command += "--storagectl \"Hard Disk Controller\" ";
        command += "--port 1 ";
        command += "--device 0 ";
        command += "--type dvddrive ";
        command += "--medium \"" + virtualbox_guest_additions + "\" ";

        retval = vbm_popen(command, output, "storage attach (guest additions image)");
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

void VBOX_VM::poll(bool log_state) {
    char buf[256];
    APP_INIT_DATA aid;
    HRESULT rc;
    MachineState vmstate;
    static MachineState vmstate_old = MachineState_PoweredOff;

    boinc_get_init_data_p(&aid);

    //
    // Is our environment still sane?
    //
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

    //
    // What state is the VM in?
    //
    rc = m_pMachine->get_State(&vmstate);
    if (SUCCEEDED(rc)) {

        // VirtualBox Documentation suggests that that a VM is running when its
        // machine state is between MachineState_FirstOnline and MachineState_LastOnline
        // which as of this writing is 5 and 17.
        //
        // VboxManage's source shows more than that though:
        // see: http://www.virtualbox.org/browser/trunk/src/VBox/Frontends/VBoxManage/VBoxManageInfo.cpp
        //
        // So for now, go with what VboxManage is reporting.
        //
        switch(vmstate)
        {
            case MachineState_Running:
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
            case MachineState_Paused:
                online = true;
                saving = false;
                restoring = false;
                suspended = true;
                crashed = false;
                break;
            case MachineState_Starting:
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
            case MachineState_Stopping:
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
            case MachineState_Saving:
                online = true;
                saving = true;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
            case MachineState_Restoring:
                online = true;
                saving = false;
                restoring = true;
                suspended = false;
                crashed = false;
                break;
            case MachineState_LiveSnapshotting:
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
            case MachineState_DeletingSnapshotOnline:
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
            case MachineState_DeletingSnapshotPaused:
                online = true;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
            case MachineState_Aborted:
                online = false;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = true;
                break;
            case MachineState_Stuck:
                online = false;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = true;
                break;
            default:
                online = false;
                saving = false;
                restoring = false;
                suspended = false;
                crashed = false;
                break;
        }

        if (log_state) {
            fprintf(
                stderr,
                "%s VM is no longer is a running state. It is in '%s'.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                MachineStateToName(vmstate)
            );
        }
        if (log_state && (vmstate_old != vmstate)) {
            fprintf(
                stderr,
                "%s VM state change detected. (old = '%s', new = '%s')\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                MachineStateToName(vmstate_old),
                MachineStateToName(vmstate)
            );
            vmstate_old = vmstate;
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

int VBOX_VM::start() {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComBSTR vm_name(vm_master_name.c_str());
    CComBSTR session_type;
    CComPtr<IProgress> pProgress;
    BOOL bCompleted;
    char buf[256];
    double timeout;


    fprintf(
        stderr,
        "%s Starting VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );


    if (!headless) {
        session_type = _T("vrdp");
    } else {
        session_type = _T("headless");
    }

    rc = m_pVirtualBox->FindMachine(vm_name, &m_pMachine);
    if (SUCCEEDED(rc)) {

        // Start a VM session
        rc = m_pMachine->LaunchVMProcess(m_pSession, session_type, NULL, &pProgress);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not launch VM process! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        // Wait until VM is running.
        rc = pProgress->WaitForCompletion(-1);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not wait for VM start completion! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        pProgress->get_Completed(&bCompleted);
        if (bCompleted) {

            // We should now own what goes on with the VM.
            //
            m_pMachine->LockMachine(m_pSession, LockType_Write);

            rc = m_pMachine->get_SessionPID((ULONG*)&vm_pid);
            if (FAILED(rc)) {
                fprintf(
                    stderr,
                    "%s Error could not get VM PID! rc = 0x%x\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    rc
                );
            }

            vm_pid_handle = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION,
                FALSE,
                vm_pid
            );

            // Make sure we are in a running state before proceeding
            //
            timeout = dtime() + 300;
            do {
                poll(false);
                if (online) break;
                boinc_sleep(1.0);
            } while (timeout >= dtime());

            fprintf(
                stderr,
                "%s Successfully started VM. (PID = '%d')\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                vm_pid
            );
            retval = BOINC_SUCCESS;
        } else {
            fprintf(
                stderr,
                "%s VM failed to start.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        }
    }

CLEANUP:
    return retval;
}

int VBOX_VM::stop() {
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    double timeout;
    CComPtr<IConsole> pConsole;
    CComPtr<IProgress> pProgress;


    fprintf(
        stderr,
        "%s Stopping VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );


    // Get console object. 
    rc = m_pSession->get_Console(&pConsole);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving console object! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Save the state of the machine.
    rc = pConsole->SaveState(&pProgress);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not save the state of the VM! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Wait until VM is powered down.
    rc = pProgress->WaitForCompletion(-1);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not wait for VM save state completion! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

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

CLEANUP:
    return retval;
}

int VBOX_VM::poweroff() {
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    double timeout;
    CComPtr<IConsole> pConsole;
    CComPtr<IProgress> pProgress;


    fprintf(
        stderr,
        "%s Powering off VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

    if (online) {
        // Get console object. 
        rc = m_pSession->get_Console(&pConsole);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error retrieving console object! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        // Power down the VM as quickly as possible.
        rc = pConsole->PowerDown(&pProgress);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not save the state of the VM! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        // Wait until VM is powered down.
        rc = pProgress->WaitForCompletion(-1);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not wait for VM save state completion! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

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

CLEANUP:
    return retval;
}

int VBOX_VM::pause() {
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    CComPtr<IConsole> pConsole;


    // Restore the process priority back to the default process priority
    // to speed up the last minute maintenance tasks before the VirtualBox
    // VM goes to sleep
    //
    reset_vm_process_priority();


    // Get console object. 
    rc = m_pSession->get_Console(&pConsole);
    if (FAILED(rc))
    {
        fprintf(
            stderr,
            "%s Error retrieving console object! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Pause the machine.
    rc = pConsole->Pause();
    if (FAILED(rc))
    {
        fprintf(
            stderr,
            "%s Error could not pause VM! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }
    else
    {
        retval = BOINC_SUCCESS;
    }

CLEANUP:
    return retval;
}

int VBOX_VM::resume() {
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    CComPtr<IConsole> pConsole;


    // Set the process priority back to the lowest level before resuming
    // execution
    //
    lower_vm_process_priority();


    // Get console object. 
    rc = m_pSession->get_Console(&pConsole);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving console object! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Resume the machine.
    rc = pConsole->Resume();
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not resume VM! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }
    else
    {
        retval = BOINC_SUCCESS;
    }

CLEANUP:
    return retval;
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

void VBOX_VM::dump_hypervisor_status_reports() {
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
}

int VBOX_VM::is_registered() {
    int retval = ERR_NOT_FOUND;
    HRESULT rc;
    CComBSTR vm_name(vm_master_name.c_str());
    CComPtr<IMachine> pMachine;

    rc = m_pVirtualBox->FindMachine(vm_name, &pMachine);
    if (VBOX_E_OBJECT_NOT_FOUND != rc) {
        retval = BOINC_SUCCESS;
    }

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

int VBOX_VM::get_install_directory(string& install_directory ) {
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
}

int VBOX_VM::get_version_information(string& version) {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComBSTR tmp;

    rc = m_pVirtualBox->get_VersionNormalized(&tmp);
    if (SUCCEEDED(rc)) {
        version = CW2A(tmp);
        retval = BOINC_SUCCESS;
    }

    return retval;
}

int VBOX_VM::get_guest_additions(string& guest_additions) {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<ISystemProperties> properties;
    CComBSTR tmp;

    rc = m_pVirtualBox->get_SystemProperties(&properties);
    if (SUCCEEDED(rc)) {
        rc = properties->get_DefaultAdditionsISO(&tmp);
        if (SUCCEEDED(rc)) {
            guest_additions = CW2A(tmp);
            retval = BOINC_SUCCESS;
        }
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
    return 0;
}

int VBOX_VM::get_vm_exit_code(unsigned long& exit_code) {
    if (vm_pid_handle) {
        GetExitCodeProcess(vm_pid_handle, &exit_code);
    }
    return 0;
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
    int retval;
    char buf[256];
    CComPtr<INetworkAdapter> pNetworkAdapter;
    HRESULT rc = ERR_EXEC;

    network_suspended = !enabled;

    if (enabled) {
        fprintf(
            stderr,
            "%s Enabling network access for VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );

        rc = m_pMachine->GetNetworkAdapter(0, &pNetworkAdapter);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error retrieving virtualized network adapter for VM! rc = 0x%x\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                rc
            );
        }

        rc = pNetworkAdapter->put_Enabled(TRUE);
        if (SUCCEEDED(rc)) {
            retval = BOINC_SUCCESS;
        }
    } else {
        fprintf(
            stderr,
            "%s Disabling network access for VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );

        rc = m_pMachine->GetNetworkAdapter(0, &pNetworkAdapter);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error retrieving virtualized network adapter for VM! rc = 0x%x\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                rc
            );
        }

        rc = pNetworkAdapter->put_Enabled(FALSE);
        if (SUCCEEDED(rc)) {
            retval = BOINC_SUCCESS;
        }
    }

    return retval;
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

    return 0;
}

void VBOX_VM::lower_vm_process_priority() {
    char buf[256];
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, BELOW_NORMAL_PRIORITY_CLASS);
        fprintf(
            stderr,
            "%s Lowering VM Process priority.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
}

void VBOX_VM::reset_vm_process_priority() {
    char buf[256];
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, NORMAL_PRIORITY_CLASS);
        fprintf(
            stderr,
            "%s Restoring VM Process priority.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }
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
            }
        }
    }

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

    return retval;
}
