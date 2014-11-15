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

#if defined(_MSC_VER) || defined(__MINGW32__)
#define stricmp     _stricmp
#endif

#include "win_util.h"
#include "atlcomcli.h"
#include "atlsafe.h"
#include "atlcoll.h"
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
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    APP_INIT_DATA aid;
    CComBSTR vm_machine_uuid;
    CComPtr<IMachine> pMachine;
    CComPtr<ISession> pSession;
    CComPtr<IBIOSSettings> pBIOSSettings;
    CComPtr<INetworkAdapter> pNetworkAdapter;
    CComPtr<INATEngine> pNATEngine;
    CComPtr<ISerialPort> pSerialPort;
    CComPtr<IParallelPort> pParallelPort;
    CComPtr<IAudioAdapter> pAudioAdapter;
    CComPtr<IStorageController> pDiskController;
    CComPtr<IStorageController> pFloppyController;
    CComPtr<IBandwidthControl> pBandwidthControl;
    CComPtr<IVRDEServer> pVRDEServer;
    ULONG lOHCICtrls = 0;
    bool disable_acceleration = false;
    string virtual_machine_slot_directory;
    string default_interface;

    boinc_get_init_data_p(&aid);
    get_slot_directory(virtual_machine_slot_directory);


    // Reset VM name in case it was changed while deregistering a stale VM
    //
    vm_name = vm_master_name;

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

    // Start the VM creation process
    //
    fprintf(
        stderr,
        "%s Create VM. (%s, slot#%d) \n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        vm_name.c_str(),
        aid.slot
    );

    rc = m_pVirtualBox->CreateMachine(
        CComBSTR(string(virtual_machine_slot_directory + string("/") + vm_name + string(".vbox")).c_str()),
        CComBSTR(vm_name.c_str()),
        NULL,
        CComBSTR(os_name.c_str()),
        CComBSTR(""),
        &pMachine
    );
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error creating virtual machine instance! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Register the VM. Note that this call also saves the VM config
    // to disk. It is also possible to save the VM settings but not
    // register the VM.
    //
    // Also note that due to current VirtualBox limitations, the machine
    // must be registered *before* we can attach hard disks to it.
    //
    rc = m_pVirtualBox->RegisterMachine(pMachine);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error registering virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }
    
    rc = pMachine->LockMachine(pSession, LockType_Write);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error locking virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pSession->get_Machine(&pMachine);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving mutable virtual machine object! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }


    rc = pMachine->get_BIOSSettings(&pBIOSSettings);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving the BIOS settings for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pMachine->get_BandwidthControl(&pBandwidthControl);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving the Bandwidth Control settings for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pMachine->get_VRDEServer(&pVRDEServer);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving the Remote Desktop settings for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pMachine->GetNetworkAdapter(0, &pNetworkAdapter);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving the Network Adapter for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pNetworkAdapter->get_NATEngine(&pNATEngine);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving the NAT Engine for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pMachine->get_AudioAdapter(&pAudioAdapter);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving the Audio Adapter for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Set some properties
    pMachine->put_Description(CComBSTR(vm_master_description.c_str()));

    // Tweak the VM's Memory Size
    //
    fprintf(
        stderr,
        "%s Setting Memory Size for VM. (%dMB)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        (int)memory_size_mb
    );
    rc = pMachine->put_MemorySize((int)(memory_size_mb*1024));
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error memory size for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Tweak the VM's CPU Count
    //
    fprintf(
        stderr,
        "%s Setting CPU Count for VM. (%s)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        vm_cpu_count.c_str()
    );
    rc = pMachine->put_CPUCount((int)atoi(vm_cpu_count.c_str()));
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error CPU count for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Tweak the VM's Chipset Options
    //
    fprintf(
        stderr,
        "%s Setting Chipset Options for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = pBIOSSettings->put_ACPIEnabled(TRUE);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error setting ACPI enabled for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pBIOSSettings->put_IOAPICEnabled(TRUE);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error setting IOAPIC enabled for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    // Tweak the VM's Boot Options
    //
    fprintf(
        stderr,
        "%s Setting Boot Options for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = pMachine->SetBootOrder(1, DeviceType_HardDisk);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error setting hard disk boot order for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }
    
    rc = pMachine->SetBootOrder(2, DeviceType_DVD);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error setting DVD boot order for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    pMachine->SetBootOrder(3, DeviceType_Null);
    pMachine->SetBootOrder(4, DeviceType_Null);

    // Tweak the VM's Network Configuration
    //
    fprintf(
        stderr,
        "%s Disconnecting VM Network Access.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = pNetworkAdapter->put_Enabled(FALSE);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error disabling network access for the virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    if (network_bridged_mode) {

        fprintf(
            stderr,
            "%s Setting Network Configuration for Bridged Mode.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        rc = pNetworkAdapter->put_AttachmentType(NetworkAttachmentType_Bridged);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error setting network configuration for the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        get_default_network_interface(default_interface);
        fprintf(
            stderr,
            "%s Setting Bridged Interface. (%s)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            default_interface.c_str()
        );
        rc = pNetworkAdapter->put_BridgedInterface(CComBSTR(CA2W(default_interface.c_str())));
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error setting network configuration (brigded interface) for the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

    } else {

        fprintf(
            stderr,
            "%s Setting Network Configuration for NAT.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        rc = pNetworkAdapter->put_AttachmentType(NetworkAttachmentType_NAT);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error setting network configuration for the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        rc = pNATEngine->put_DNSProxy(TRUE);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error setting network configuration (DNS Proxy) for the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
    }

    // Tweak the VM's USB Configuration
    //
    fprintf(
        stderr,
        "%s Disabling USB Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = pMachine->GetUSBControllerCountByType(USBControllerType_OHCI, &lOHCICtrls);
    if (SUCCEEDED(rc) && lOHCICtrls) {
        pMachine->RemoveUSBController(CComBSTR("OHCI"));
    }

    // Tweak the VM's COM Port Support
    //
    fprintf(
        stderr,
        "%s Disabling COM Port Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = pMachine->GetSerialPort(0, &pSerialPort);
    if (SUCCEEDED(rc)) {
        pSerialPort->put_Enabled(FALSE);
    }
    rc = pMachine->GetSerialPort(1, &pSerialPort);
    if (SUCCEEDED(rc)) {
        pSerialPort->put_Enabled(FALSE);
    }

    // Tweak the VM's LPT Port Support
    //
    fprintf(
        stderr,
        "%s Disabling LPT Port Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = pMachine->GetParallelPort(0, &pParallelPort);
    if (SUCCEEDED(rc)) {
        pParallelPort->put_Enabled(FALSE);
    }
    rc = pMachine->GetParallelPort(1, &pParallelPort);
    if (SUCCEEDED(rc)) {
        pParallelPort->put_Enabled(FALSE);
    }

    // Tweak the VM's Audio Support
    //
    fprintf(
        stderr,
        "%s Disabling Audio Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    pAudioAdapter->put_Enabled(FALSE);

    // Tweak the VM's Clipboard Support
    //
    fprintf(
        stderr,
        "%s Disabling Clipboard Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    pMachine->put_ClipboardMode(ClipboardMode_Disabled);

    // Tweak the VM's Drag & Drop Support
    //
    fprintf(
        stderr,
        "%s Disabling Drag and Drop Support for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    pMachine->put_DragAndDropMode(DragAndDropMode_Disabled);

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
            rc = pMachine->SetHWVirtExProperty(HWVirtExPropertyType_Enabled, FALSE);
            if (FAILED(rc)) {
                fprintf(
                    stderr,
                    "%s Error disabling hardware acceleration support for the virtual machine! rc = 0x%x\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    rc
                );
                virtualbox_dump_error();
                retval = rc;
                goto CLEANUP;
            }
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
        "%s Adding storage controller(s) to VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    if (0 == stricmp(vm_disk_controller_type.c_str(), "ide")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_IDE, &pDiskController);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding storage controller (IDE) to the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
    }
    if (0 == stricmp(vm_disk_controller_type.c_str(), "sata")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_SATA, &pDiskController);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding storage controller (SATA) to the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
        pDiskController->put_UseHostIOCache(FALSE);
        pDiskController->put_PortCount(3);
    }
    if (0 == stricmp(vm_disk_controller_type.c_str(), "sas")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_SAS, &pDiskController);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding storage controller (SAS) to the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
        pDiskController->put_UseHostIOCache(FALSE);
    }
    if (0 == stricmp(vm_disk_controller_type.c_str(), "scsi")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_SCSI, &pDiskController);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding storage controller (SCSI) to the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
        pDiskController->put_UseHostIOCache(FALSE);
    }

    if (0 == stricmp(vm_disk_controller_model.c_str(), "lsilogic")) {
        pDiskController->put_ControllerType(StorageControllerType_LsiLogic);
    } else if (0 == stricmp(vm_disk_controller_model.c_str(), "buslogic")) {
        pDiskController->put_ControllerType(StorageControllerType_BusLogic);
    } else if (0 == stricmp(vm_disk_controller_model.c_str(), "intelahci")) {
        pDiskController->put_ControllerType(StorageControllerType_IntelAhci);
    } else if (0 == stricmp(vm_disk_controller_model.c_str(), "piix3")) {
        pDiskController->put_ControllerType(StorageControllerType_PIIX3);
    } else if (0 == stricmp(vm_disk_controller_model.c_str(), "piix4")) {
        pDiskController->put_ControllerType(StorageControllerType_PIIX4);
    } else if (0 == stricmp(vm_disk_controller_model.c_str(), "ich6")) {
        pDiskController->put_ControllerType(StorageControllerType_ICH6);
    } else if (0 == stricmp(vm_disk_controller_model.c_str(), "i82078")) {
        pDiskController->put_ControllerType(StorageControllerType_I82078);
    } else if (0 == stricmp(vm_disk_controller_model.c_str(), "lsilogicsas")) {
        pDiskController->put_ControllerType(StorageControllerType_LsiLogicSas);
    }

    // Add storage controller for a floppy device if desired
    //
    if (enable_floppyio) {
        rc = pMachine->AddStorageController(CComBSTR("Floppy Controller"), StorageBus_Floppy, &pFloppyController);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding storage controller (Floppy) to the virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
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
        CComPtr<IMedium> pISOImage;
        rc = m_pVirtualBox->OpenMedium(
            CComBSTR(string(virtual_machine_slot_directory + "/" + iso_image_filename).c_str()),
            DeviceType_DVD,
            AccessMode_ReadOnly,
            FALSE,
            &pISOImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding virtual ISO 9660 disk drive to VirtualBox! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            0,
            0,
            DeviceType_DVD,
            pISOImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding virtual ISO 9660 disk drive to virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        // Add guest additions to the VM
        //
        fprintf(
            stderr,
            "%s Adding VirtualBox Guest Additions to VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        CComPtr<IMedium> pGuestAdditionsImage;
        rc = m_pVirtualBox->OpenMedium(
            CComBSTR(virtualbox_guest_additions.c_str()),
            DeviceType_DVD,
            AccessMode_ReadOnly,
            FALSE,
            &pGuestAdditionsImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding VirtualBox Guest Additions to VirtualBox! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            2,
            0,
            DeviceType_DVD,
            pGuestAdditionsImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding VirtualBox Guest Additions to virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        // Add a virtual cache disk drive to VM
        //
        if (enable_cache_disk){
            fprintf(
                stderr,
                "%s Adding virtual cache disk drive to VM. (%s)\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
    		    cache_disk_filename.c_str()
            );
            CComPtr<IMedium> pCacheImage;
            rc = m_pVirtualBox->OpenMedium(
                CComBSTR(string(virtual_machine_slot_directory + "/" + cache_disk_filename).c_str()),
                DeviceType_HardDisk,
                AccessMode_ReadWrite,
                TRUE,
                &pCacheImage
            );
            if (FAILED(rc)) {
                fprintf(
                    stderr,
                    "%s Error adding virtual cache disk drive to VirtualBox! rc = 0x%x\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    rc
                );
                virtualbox_dump_error();
                retval = rc;
                goto CLEANUP;
            }

            rc = pMachine->AttachDevice(
                CComBSTR("Hard Disk Controller"),
                1,
                0,
                DeviceType_HardDisk,
                pCacheImage
            );
            if (FAILED(rc)) {
                fprintf(
                    stderr,
                    "%s Error adding virtual cache disk drive to virtual machine! rc = 0x%x\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    rc
                );
                virtualbox_dump_error();
                retval = rc;
                goto CLEANUP;
            }
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
        CComPtr<IMedium> pDiskImage;
        rc = m_pVirtualBox->OpenMedium(
            CComBSTR(string(virtual_machine_slot_directory + "/" + cache_disk_filename).c_str()),
            DeviceType_HardDisk,
            AccessMode_ReadWrite,
            TRUE,
            &pDiskImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding virtual disk drive to VirtualBox! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            0,
            0,
            DeviceType_HardDisk,
            pDiskImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding virtual disk drive to virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        // Add guest additions to the VM
        //
        fprintf(
            stderr,
            "%s Adding VirtualBox Guest Additions to VM.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        CComPtr<IMedium> pGuestAdditionsImage;
        rc = m_pVirtualBox->OpenMedium(
            CComBSTR(virtualbox_guest_additions.c_str()),
            DeviceType_DVD,
            AccessMode_ReadOnly,
            FALSE,
            &pGuestAdditionsImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding VirtualBox Guest Additions to VirtualBox! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            1,
            0,
            DeviceType_DVD,
            pGuestAdditionsImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding VirtualBox Guest Additions to virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

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
        CComPtr<IMedium> pFloppyImage;
        rc = m_pVirtualBox->OpenMedium(
            CComBSTR(string(virtual_machine_slot_directory + "/" + floppy_image_filename).c_str()),
            DeviceType_Floppy,
            AccessMode_ReadWrite,
            TRUE,
            &pFloppyImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding virtual floppy disk image to VirtualBox! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        rc = pMachine->AttachDevice(
            CComBSTR("Floppy Controller"),
            0,
            0,
            DeviceType_Floppy,
            pFloppyImage
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error adding virtual floppy disk image to virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

    }

    // Add network bandwidth throttle group
    //
    fprintf(
        stderr,
        "%s Adding network bandwidth throttle group to VM. (Defaulting to 1024GB)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = pBandwidthControl->CreateBandwidthGroup(
        CComBSTR(string(vm_name + "_net").c_str()),
        BandwidthGroupType_Network,
        (LONG64)1024*1024*1024*1024
    );
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error adding network bandwidth group to virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
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
            rc = pNATEngine->AddRedirect(
                CComBSTR(""),
                NATProtocol_TCP,
                pf.is_remote?CComBSTR(""):CComBSTR("127.0.0.1"),
                pf.host_port,
                CComBSTR(""),
                pf.guest_port
            );
            if (FAILED(rc)) {
                fprintf(
                    stderr,
                    "%s Error adding port forward to virtual machine! rc = 0x%x\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    rc
                );
                virtualbox_dump_error();
                retval = rc;
                goto CLEANUP;
            }
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

            pVRDEServer->put_Enabled(TRUE);
            pVRDEServer->put_VRDEExtPack(CComBSTR(""));
            pVRDEServer->put_AuthLibrary(CComBSTR(""));
            pVRDEServer->put_AuthType(AuthType_Null);
            pVRDEServer->SetVRDEProperty(CComBSTR("TCP/Ports"), CComBSTR(buf));
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
        rc = pMachine->CreateSharedFolder(
            CComBSTR("shared"),
            CComBSTR(string(virtual_machine_slot_directory + "/shared").c_str()),
            TRUE,
            TRUE
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not create shared folder for virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
    }

    rc = pMachine->SaveSettings();
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not save settings for virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

CLEANUP:
    return retval;
}

int VBOX_VM::register_vm() {
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    string virtual_machine_slot_directory;
    APP_INIT_DATA aid;
    CComPtr<IMachine> pMachine;

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

    rc = m_pVirtualBox->OpenMachine(
        CComBSTR(string(virtual_machine_slot_directory + "/" + vm_name + "/" + vm_name + ".vbox").c_str()),
        &pMachine
    );
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error opening virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = m_pVirtualBox->RegisterMachine(pMachine);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error registering virtual machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

CLEANUP:
    return retval;
}

int VBOX_VM::deregister_vm(bool delete_media) {
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    CComPtr<IBandwidthControl> pBandwidthControl;
    CComPtr<IProgress> pProgress;
    SAFEARRAY* pHardDisks;
    string virtual_machine_slot_directory;

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
    fprintf(
        stderr,
        "%s Removing network bandwidth throttle group from VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    rc = m_pMachine->get_BandwidthControl(&pBandwidthControl);
    if (SUCCEEDED(rc)) {
        pBandwidthControl->DeleteBandwidthGroup(CComBSTR(string(vm_name + "_net").c_str()));
    }

    // Delete its storage controller(s)
    //
    fprintf(
        stderr,
        "%s Removing storage controller(s) from VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    m_pMachine->RemoveStorageController(CComBSTR("Hard Disk Controller"));

    if (enable_floppyio) {
        m_pMachine->RemoveStorageController(CComBSTR("Floppy Controller"));
    }

    // Next, delete VM
    //
    fprintf(
        stderr,
        "%s Removing VM from VirtualBox.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    if (!delete_media) {
        rc = m_pMachine->Unregister(CleanupMode_UnregisterOnly, &pHardDisks);
        if (SUCCEEDED(rc)) {
            m_pMachine->DeleteConfig(pHardDisks, &pProgress);
            if (SUCCEEDED(rc)) {
                pProgress->WaitForCompletion(-1);
            }
        }
    } else {
        rc = m_pMachine->Unregister(CleanupMode_DetachAllReturnHardDisksOnly, &pHardDisks);
        if (SUCCEEDED(rc)) {
            fprintf(
                stderr,
                "%s Removing virtual disk drive(s) from VirtualBox.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            m_pMachine->DeleteConfig(pHardDisks, &pProgress);
            if (SUCCEEDED(rc)) {
                pProgress->WaitForCompletion(-1);
            }
        }
    }
    return 0;
}

int VBOX_VM::deregister_stale_vm() {
    HRESULT rc;
    SAFEARRAY* pHardDisks;
    SAFEARRAY* pMachines;
    CComSafeArray<LPDISPATCH> aHardDisks;
    CComSafeArray<LPDISPATCH> aMachines;
    CComPtr<IMedium> pHardDisk;
    CComPtr<IMachine> pMachine;
    CComBSTR tmp;
    string virtual_machine_root_dir;
    string hdd_image_location;

    get_slot_directory(virtual_machine_root_dir);
    hdd_image_location = string(virtual_machine_root_dir + "/" + image_filename);

    rc = m_pVirtualBox->get_HardDisks(&pHardDisks);
    if (SUCCEEDED(rc)) {
        aHardDisks.Attach(pHardDisks);
        for (int i = 0; i < (int)aHardDisks.GetCount(); i++) {
            pHardDisk = aHardDisks[i];
            pHardDisk->get_Location(&tmp);

            // Did we find that our disk has already been registered in the media registry?
            //
            if (0 == stricmp(hdd_image_location.c_str(), CW2A(tmp))) {

                // Disk found
                //
                rc = pHardDisk->get_MachineIds(&pMachines);
                if (SUCCEEDED(rc)) {
                    aMachines.Attach(pMachines);
                    // Delete all registered VMs attached to this disk image
                    //
                    for (int j = 0; j < (int)aMachines.GetCount(); j++) {
                        pMachine = aMachines[j];
                        rc = pMachine->get_Id(&tmp);
                        if (SUCCEEDED(rc)) {
                            vm_name = CW2A(tmp);
                            deregister_vm(false);
                        }
                    }
                }
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
    char buf[256];
    HRESULT rc;
    CComBSTR vm_name(vm_master_name.c_str());
    CComBSTR session_type;
    CComPtr<IProgress> pProgress;
    BOOL bCompleted;
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
    char buf[256];
    HRESULT rc;
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
    char buf[256];
    HRESULT rc;
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
    char buf[256];
    HRESULT rc;
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
    char buf[256];
    HRESULT rc;
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
    int retval = ERR_EXEC;
    char buf[256];
    HRESULT rc;
    CComPtr<IConsole> pConsole;
    CComPtr<IProgress> pProgress;

    fprintf(
        stderr,
        "%s Creating new snapshot for VM.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

    // Pause VM - Try and avoid the live snapshot and trigger an online
    // snapshot instead.
    pause();

    // Create new snapshot
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
    } else {
        sprintf(buf, "%d", (int)elapsed_time);
        rc = pConsole->TakeSnapshot(CComBSTR(string(string("boinc_") + buf).c_str()), CComBSTR(""), &pProgress);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error taking snapshot! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
        } else {
            rc = pProgress->WaitForCompletion(-1);
            if (FAILED(rc)) {
                fprintf(
                    stderr,
                    "%s Error could not wait for snapshot creation completion! rc = 0x%x\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    rc
                );
                virtualbox_dump_error();
                retval = rc;
            }
        }
    }

    // Resume VM
    resume();

    if (ERR_EXEC != retval) goto CLEANUP;

    // Set the suspended flag back to false before deleting the stale
    // snapshot
    poll(false);

    // Delete stale snapshot(s), if one exists
    retval = cleanup_snapshots(false);
    if (retval) {
        return retval;
    }

    fprintf(
        stderr,
        "%s Checkpoint completed.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

CLEANUP:
    return retval;
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
    int retval = ERR_EXEC;
    char buf[256];
    HRESULT rc;
    CComPtr<IConsole> pConsole;
    CComPtr<ISnapshot> pSnapshot;
    CComPtr<IProgress> pProgress;

    fprintf(
        stderr,
        "%s Restore from previously saved snapshot.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

    rc = m_pMachine->get_CurrentSnapshot(&pSnapshot);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error retrieving current snapshot object! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

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

    rc = pConsole->RestoreSnapshot(pSnapshot, &pProgress);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error restoring snapshot! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    rc = pProgress->WaitForCompletion(-1);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not wait for restore completion! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

    retval = BOINC_SUCCESS;

    fprintf(
        stderr,
        "%s Restore completed.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

CLEANUP:
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
    CComPtr<IMachine> pMachine;

    rc = m_pVirtualBox->FindMachine(CComBSTR(vm_master_name.c_str()), &pMachine);
    if (VBOX_E_OBJECT_NOT_FOUND != rc) {
        retval = BOINC_SUCCESS;
    }

    return retval;
}

bool VBOX_VM::is_system_ready(std::string& message) {
    return true;
}

bool VBOX_VM::is_hdd_registered() {
    HRESULT rc;
    SAFEARRAY* pHardDisks;
    CComSafeArray<LPDISPATCH> aHardDisks;
    CComPtr<IMedium> pHardDisk;
    CComBSTR tmp;
    string virtual_machine_root_dir;
    string hdd_image_location;

    get_slot_directory(virtual_machine_root_dir);
    hdd_image_location = string(virtual_machine_root_dir + "/" + image_filename);

    rc = m_pVirtualBox->get_HardDisks(&pHardDisks);
    if (SUCCEEDED(rc)) {
        aHardDisks.Attach(pHardDisks);
        for (int i = 0; i < (int)aHardDisks.GetCount(); i++) {
            pHardDisk = aHardDisks[i];
            pHardDisk->get_Location(&tmp);
            if (0 == stricmp(hdd_image_location.c_str(), CW2A(tmp))) {
                return true;
            }
        }
    }
    return false;
}

bool VBOX_VM::is_extpack_installed() {
    CComPtr<IExtPackManager> pExtPackManager;
    CComPtr<IExtPack> pExtPack;
    BOOL bUsable = FALSE;
    HRESULT rc;

    rc = m_pVirtualBox->get_ExtensionPackManager(&pExtPackManager);
    if (SUCCEEDED(rc)) {
        rc = pExtPackManager->IsExtPackUsable(CComBSTR("Oracle VM VirtualBox Extension Pack"), &bUsable);
        if (SUCCEEDED(rc)) {
            if (bUsable) {
                return true;
            }
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
    int retval = ERR_EXEC;
    HRESULT rc;
    SAFEARRAY* pNICS;
    CComPtr<IHost> pHost;
    CComPtr<IHostNetworkInterface> pNIC;
    CComBSTR tmp;
    CComSafeArray<LPDISPATCH> aNICS;

    rc = m_pVirtualBox->get_Host(&pHost);
    if (SUCCEEDED(rc)) {
        rc = pHost->FindHostNetworkInterfacesOfType(HostNetworkInterfaceType_Bridged, &pNICS);
        if (SUCCEEDED(rc)) {
            // Automatically clean up array after use
            aNICS.Attach(pNICS);

            // We only need the 'default' nic, which is usally the first one.
            pNIC = aNICS[0];

            // Get the name for future use
            rc = pNIC->get_Name(&tmp);
            if (SUCCEEDED(rc)) {
                iface = CW2A(tmp);
                retval = BOINC_SUCCESS;
            }
        }
    }

    return retval;
}

int VBOX_VM::get_vm_network_bytes_sent(double& sent) {
    int retval = ERR_EXEC;
    char buf[256];
    HRESULT rc;
    CComPtr<IConsole> pConsole;
    CComPtr<IMachineDebugger> pDebugger;
    CComBSTR strPattern("/Devices/*/TransmitBytes");
    CComBSTR strOutput;
    string output;
    string counter_value;
    size_t counter_start;
    size_t counter_end;

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


    // Get debugger object
    rc = pConsole->get_Debugger(&pDebugger);
    if (SUCCEEDED(rc)) {
        rc = pDebugger->GetStats(strPattern, false, &strOutput);
        if (SUCCEEDED(rc)) {
            output = CW2A(strOutput);

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

            retval = BOINC_SUCCESS;
        }
    }

CLEANUP:
    return retval;
}

int VBOX_VM::get_vm_network_bytes_received(double& received) {
    int retval = ERR_EXEC;
    char buf[256];
    HRESULT rc;
    CComPtr<IConsole> pConsole;
    CComPtr<IMachineDebugger> pDebugger;
    CComBSTR strPattern("/Devices/*/ReceiveBytes");
    CComBSTR strOutput;
    string output;
    string counter_value;
    size_t counter_start;
    size_t counter_end;

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


    // Get debugger object
    rc = pConsole->get_Debugger(&pDebugger);
    if (SUCCEEDED(rc)) {
        rc = pDebugger->GetStats(strPattern, false, &strOutput);
        if (SUCCEEDED(rc)) {
            output = CW2A(strOutput);

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

            retval = BOINC_SUCCESS;
        }
    }

CLEANUP:
    return retval;
}

int VBOX_VM::get_vm_process_id() {
    return vm_pid;
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
    char buf[256];
    fprintf(
        stderr,
        "%s Setting CPU throttle for VM. (%d%%)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        percentage
    );
    m_pMachine->put_CPUExecutionCap(percentage);
    return 0;
}

int VBOX_VM::set_network_usage(int kilobytes) {
    char buf[256];
    HRESULT rc;
    CComPtr<IBandwidthControl> pBandwidthControl;
    CComPtr<IBandwidthGroup> pBandwidthGroup;

    rc = m_pMachine->get_BandwidthControl(&pBandwidthControl);
    if (SUCCEEDED(rc)) {
        rc = pBandwidthControl->GetBandwidthGroup(CComBSTR(string(vm_name + "_net").c_str()), &pBandwidthGroup);
        if (SUCCEEDED(rc)) {
            if (kilobytes == 0) {
                fprintf(
                    stderr,
                    "%s Setting network throttle for VM. (1024GB)\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
                rc = pBandwidthGroup->put_MaxBytesPerSec((LONG64)1024*1024*1024*1024);
            } else {
                fprintf(
                    stderr,
                    "%s Setting network throttle for VM. (%dKB)\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    kilobytes
                );
                rc = pBandwidthGroup->put_MaxBytesPerSec((LONG64)kilobytes*1024);
            }
            if (FAILED(rc)) {
                fprintf(
                    stderr,
                    "%s Error setting network throttle for the virtual machine! rc = 0x%x\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    rc
                );
            }
        }
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
