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

#ifdef _VIRTUALBOX_IMPORT_FUNCTIONS_

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


// Helper function to print MSCOM exception information set on the current
// thread after a failed MSCOM method call. This function will also print
// extended VirtualBox error info if it is available.
//
#define CHECK_ERROR(rc) \
    retval = virtualbox_check_error(rc, __FUNCTION__, __FILE__, __LINE__)

int virtualbox_check_error(HRESULT rc, char* szFunction, char* szFile, int iLine) {
    HRESULT local_rc;
    CComPtr<IErrorInfo> pErrorInfo;
    CComBSTR strDescription;

    if (FAILED(rc)) {
        local_rc = GetErrorInfo(0, &pErrorInfo);
        if (SUCCEEDED(local_rc)) {
            vboxlog_msg("Error in %s (%s:%d)", szFunction, szFile, iLine);
            rc = pErrorInfo->GetDescription(&strDescription);
            if (SUCCEEDED(rc) && strDescription) {
                vboxlog_msg("Error description: %S", strDescription);
            }
        } else {
            vboxlog_msg("Error: getting error info! rc = 0x%x", rc);
        }
    }
    return rc;
}


// We want to recurisively walk the snapshot tree so that we can get the most recent children first.
// We also want to skip whatever the most current snapshot is.
//
void TraverseSnapshots(std::string& current_snapshot_id, std::vector<std::string>& snapshots, ISnapshot* pSnapshot) {
    HRESULT rc;
    SAFEARRAY* pSnapshots = NULL;
    CComSafeArray<LPDISPATCH> aSnapshots;
    CComBSTR tmp;
    ULONG lCount;
    std::string snapshot_id;

    // Check to see if we have any children
    //
    rc = pSnapshot->GetChildrenCount(&lCount);
    if (SUCCEEDED(rc) && lCount) {
        rc = pSnapshot->get_Children(&pSnapshots);
        if (SUCCEEDED(rc)) {
            aSnapshots.Attach(pSnapshots);
            if (aSnapshots.GetCount() > 0) {
                for (int i = 0; i < (int)aSnapshots.GetCount(); i++) {
                    TraverseSnapshots(current_snapshot_id, snapshots, (ISnapshot*)(LPDISPATCH)aSnapshots[i]);
                }
            }
        }
    }

    // Check to see if we are the most recent snapshot.
    // if not, add the snapshot id to the list of snapshots to be deleted.
    //
    pSnapshot->get_Id(&tmp);
    if (SUCCEEDED(rc)) {
        snapshot_id = CW2A(tmp);
        if (current_snapshot_id == snapshot_id) {
            return;
        } else {
            snapshots.push_back(snapshot_id);
        }
    }
}


// We want to recurisively walk the medium tree so that we can get the most recent children first.
//
void TraverseMediums(std::vector<CComPtr<IMedium>>& mediums, IMedium* pMedium) {
    HRESULT rc;
    SAFEARRAY* pMediums = NULL;
    CComSafeArray<LPDISPATCH> aMediums;

    // Check to see if we have any children
    //
    rc = pMedium->get_Children(&pMediums);
    if (SUCCEEDED(rc)) {
        aMediums.Attach(pMediums);
        if (aMediums.GetCount() > 0) {
            for (int i = 0; i < (int)aMediums.GetCount(); i++) {
                TraverseMediums(mediums, (IMedium*)(LPDISPATCH)aMediums[i]);
            }
        }
    }

    mediums.push_back(CComPtr<IMedium>(pMedium));
}


VBOX_VM::VBOX_VM() {
    m_pPrivate = new VBOX_PRIV();
}

VBOX_VM::~VBOX_VM() {
    if (m_pPrivate) {
        delete m_pPrivate;
        m_pPrivate = NULL;
    }
}

int VBOX_VM::initialize() {
    int rc = BOINC_SUCCESS;
    string old_path;
    string new_path;
    string version;
    APP_INIT_DATA aid;
    bool force_sandbox = false;

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
            vboxlog_msg("Failed to modify the search path");
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
            vboxlog_msg("Failed to modify the search path");
        }
    }

    // Launch vboxsvc manually so that the DCOM subsystem won't be able too.  Our version
    // will have permission and direction to write its state information to the BOINC
    // data directory.
    //
    launch_vboxsvc();

    // Instantiate the VirtualBox root object.
    rc = m_pPrivate->m_pVirtualBox.CreateInstance(CLSID_VirtualBox);
    if (FAILED(rc)) {
        vboxlog_msg("Error creating VirtualBox instance! rc = 0x%x", rc);
        return rc;
    }

    // Create the session object.
    rc = m_pPrivate->m_pSession.CreateInstance(CLSID_Session);
    if (FAILED(rc)) {
        vboxlog_msg("Error creating Session instance! rc = 0x%x", rc);
        return rc;
    }

    rc = get_version_information(version);
    if (rc) return rc;

    // Fix-up version string
    virtualbox_version = "VirtualBox COM Interface (Version: " + version + ")";

    // Get the guest addition information
    get_guest_additions(virtualbox_guest_additions);

    return rc;
}

int VBOX_VM::create_vm() {
    int retval = ERR_EXEC;
    HRESULT rc;
    char buf[256];
    APP_INIT_DATA aid;
    CComBSTR vm_machine_uuid;
    CComPtr<IMachine> pMachineRO;
    CComPtr<IMachine> pMachine;
    CComPtr<ISession> pSession;
    CComPtr<IBIOSSettings> pBIOSSettings;
    CComPtr<INetworkAdapter> pNetworkAdapter;
    CComPtr<INATEngine> pNATEngine;
    CComPtr<IUSBController> pUSBContoller;
    CComPtr<ISerialPort> pSerialPort1;
    CComPtr<ISerialPort> pSerialPort2;
    CComPtr<IParallelPort> pParallelPort1;
    CComPtr<IParallelPort> pParallelPort2;
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


    rc = pSession.CoCreateInstance(CLSID_Session);
    if (CHECK_ERROR(rc)) goto CLEANUP;


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

    // Start the VM creation process
    //
    rc = m_pPrivate->m_pVirtualBox->CreateMachine(
        CComBSTR(string(virtual_machine_slot_directory + "\\" + vm_name + "\\" + vm_name + ".vbox").c_str()),
        CComBSTR(vm_name.c_str()),
        NULL,
        CComBSTR(os_name.c_str()),
        CComBSTR(""),
        &pMachineRO
    );
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Register the VM. Note that this call also saves the VM config
    // to disk. It is also possible to save the VM settings but not
    // register the VM.
    //
    // Also note that due to current VirtualBox limitations, the machine
    // must be registered *before* we can attach hard disks to it.
    //
    rc = m_pPrivate->m_pVirtualBox->RegisterMachine(pMachineRO);
    if (CHECK_ERROR(rc)) goto CLEANUP;
    
    rc = pMachineRO->LockMachine(pSession, LockType_Write);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pSession->get_Machine(&pMachine);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pMachine->get_BIOSSettings(&pBIOSSettings);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pMachine->get_BandwidthControl(&pBandwidthControl);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pMachine->get_VRDEServer(&pVRDEServer);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pMachine->GetNetworkAdapter(0, &pNetworkAdapter);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pNetworkAdapter->get_NATEngine(&pNATEngine);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pMachine->get_AudioAdapter(&pAudioAdapter);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Set some properties
    //
    pMachine->put_Description(CComBSTR(vm_master_description.c_str()));

    // Tweak the VM's Memory Size
    //
    vboxlog_msg("Setting Memory Size for VM. (%dMB)", (int)memory_size_mb);
    rc = pMachine->put_MemorySize((int)(memory_size_mb));
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Tweak the VM's CPU Count
    //
    vboxlog_msg("Setting CPU Count for VM. (%s)", vm_cpu_count.c_str());
    rc = pMachine->put_CPUCount((int)atoi(vm_cpu_count.c_str()));
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Tweak the VM's Chipset Options
    //
    vboxlog_msg("Setting Chipset Options for VM.");
    rc = pBIOSSettings->put_ACPIEnabled(TRUE);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = pBIOSSettings->put_IOAPICEnabled(TRUE);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Tweak the VM's Boot Options
    //
    vboxlog_msg("Setting Boot Options for VM.");
    rc = pMachine->SetBootOrder(1, DeviceType_HardDisk);
    if (CHECK_ERROR(rc)) goto CLEANUP;
    
    rc = pMachine->SetBootOrder(2, DeviceType_DVD);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    pMachine->SetBootOrder(3, DeviceType_Null);
    pMachine->SetBootOrder(4, DeviceType_Null);

    // Tweak the VM's Network Configuration
    //
    if (enable_network) {
        vboxlog_msg("Enabling VM Network Access.");
        rc = pNetworkAdapter->put_Enabled(TRUE);
        if (CHECK_ERROR(rc)) goto CLEANUP;
    } else {
        vboxlog_msg("Disabling VM Network Access.");
        rc = pNetworkAdapter->put_Enabled(FALSE);
        if (CHECK_ERROR(rc)) goto CLEANUP;
    }

    if (network_bridged_mode) {
        vboxlog_msg("Setting Network Configuration for Bridged Mode.");
        rc = pNetworkAdapter->put_AttachmentType(NetworkAttachmentType_Bridged);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        get_default_network_interface(default_interface);

        vboxlog_msg("Setting Bridged Interface. (%s)", default_interface.c_str());
        rc = pNetworkAdapter->put_BridgedInterface(CComBSTR(CA2W(default_interface.c_str())));
        if (CHECK_ERROR(rc)) goto CLEANUP;
    } else {
        vboxlog_msg("Setting Network Configuration for NAT.");
        rc = pNetworkAdapter->put_AttachmentType(NetworkAttachmentType_NAT);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pNATEngine->put_DNSProxy(TRUE);
        if (CHECK_ERROR(rc)) goto CLEANUP;
    }

    // Tweak the VM's USB Configuration
    //
    vboxlog_msg("Disabling USB Support for VM.");
#ifdef _VIRTUALBOX43_
    rc = pMachine->GetUSBControllerCountByType(USBControllerType_OHCI, &lOHCICtrls);
    if (SUCCEEDED(rc) && lOHCICtrls) {
        pMachine->RemoveUSBController(CComBSTR("OHCI"));
    }
#endif
#ifdef _VIRTUALBOX42_
    rc = pMachine->get_USBController(&pUSBContoller);
    if (SUCCEEDED(rc)) {
        pUSBContoller->put_Enabled(FALSE);
    }
#endif

    // Tweak the VM's COM Port Support
    //
    vboxlog_msg("Disabling COM Port Support for VM.");
    rc = pMachine->GetSerialPort(0, &pSerialPort1);
    if (SUCCEEDED(rc)) {
        pSerialPort1->put_Enabled(FALSE);
    }
    rc = pMachine->GetSerialPort(1, &pSerialPort2);
    if (SUCCEEDED(rc)) {
        pSerialPort2->put_Enabled(FALSE);
    }

    // Tweak the VM's LPT Port Support
    //
    vboxlog_msg("Disabling LPT Port Support for VM.");
    rc = pMachine->GetParallelPort(0, &pParallelPort1);
    if (SUCCEEDED(rc)) {
        pParallelPort1->put_Enabled(FALSE);
    }
    rc = pMachine->GetParallelPort(1, &pParallelPort2);
    if (SUCCEEDED(rc)) {
        pParallelPort2->put_Enabled(FALSE);
    }

    // Tweak the VM's Audio Support
    //
    vboxlog_msg("Disabling Audio Support for VM.");
    pAudioAdapter->put_Enabled(FALSE);

    // Tweak the VM's Clipboard Support
    //
    vboxlog_msg("Disabling Clipboard Support for VM.");
    pMachine->put_ClipboardMode(ClipboardMode_Disabled);

    // Tweak the VM's Drag & Drop Support
    //
    vboxlog_msg("Disabling Drag and Drop Support for VM.");
    pMachine->put_DragAndDropMode(DragAndDropMode_Disabled);

    // Check to see if the processor supports hardware acceleration for virtualization
    // If it doesn't, disable the use of it in VirtualBox. Multi-core jobs require hardware
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
    if (is_boinc_client_version_newer(aid, 7, 2, 16)) {
        if (aid.vm_extensions_disabled) {
            vboxlog_msg("Hardware acceleration failed with previous execution. Disabling VirtualBox hardware acceleration support.");
            disable_acceleration = true;
        }
    } else {
        if (vm_cpu_count == "1") {
            // Keep this around for older clients.  Removing this for older clients might
            // lead to a machine that will only return crashed VM reports.
            vboxlog_msg("Legacy fallback configuration detected. Disabling VirtualBox hardware acceleration support.");
            vboxlog_msg("NOTE: Upgrading to BOINC 7.2.16 or better may re-enable hardware acceleration.");
            disable_acceleration = true;
        }
    }

    // Only allow disabling of hardware acceleration on 32-bit VM types, 64-bit VM types require it.
    //
    if (os_name.find("_64") == std::string::npos) {
        if (disable_acceleration) {
            vboxlog_msg("Disabling hardware acceleration support for virtualization.");
            rc = pMachine->SetHWVirtExProperty(HWVirtExPropertyType_Enabled, FALSE);
            if (CHECK_ERROR(rc)) goto CLEANUP;
        }
    } else if (os_name.find("_64") != std::string::npos) {
        if (disable_acceleration) {
            vboxlog_msg("ERROR: Invalid configuration.  VM type requires acceleration but the current configuration cannot support it.");
            retval = ERR_INVALID_PARAM;
            goto CLEANUP;
        }
    }

    // Add storage controller to VM
    // See: http://www.virtualbox.org/manual/ch08.html#vboxmanage-storagectl
    // See: http://www.virtualbox.org/manual/ch05.html#iocaching
    //
    vboxlog_msg("Adding storage controller(s) to VM.");
    if (0 == stricmp(vm_disk_controller_type.c_str(), "ide")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_IDE, &pDiskController);
        if (CHECK_ERROR(rc)) goto CLEANUP;
    }
    if (0 == stricmp(vm_disk_controller_type.c_str(), "sata")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_SATA, &pDiskController);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        pDiskController->put_UseHostIOCache(FALSE);
        pDiskController->put_PortCount(3);
    }
    if (0 == stricmp(vm_disk_controller_type.c_str(), "sas")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_SAS, &pDiskController);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        pDiskController->put_UseHostIOCache(FALSE);
    }
    if (0 == stricmp(vm_disk_controller_type.c_str(), "scsi")) {
        rc = pMachine->AddStorageController(CComBSTR("Hard Disk Controller"), StorageBus_SCSI, &pDiskController);
        if (CHECK_ERROR(rc)) goto CLEANUP;

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
        if (CHECK_ERROR(rc)) goto CLEANUP;
    }

    if (enable_isocontextualization) {
        // Add virtual ISO 9660 disk drive to VM
        //
        vboxlog_msg("Adding virtual ISO 9660 disk drive to VM. (%s)", iso_image_filename.c_str());
        CComPtr<IMedium> pISOImage;
        rc = m_pPrivate->m_pVirtualBox->OpenMedium(
            CComBSTR(string(virtual_machine_slot_directory + "\\" + iso_image_filename).c_str()),
            DeviceType_DVD,
            AccessMode_ReadOnly,
            TRUE,
            &pISOImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            0,
            0,
            DeviceType_DVD,
            pISOImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Add guest additions to the VM
        //
        vboxlog_msg("Adding VirtualBox Guest Additions to VM.");
        CComPtr<IMedium> pGuestAdditionsImage;
        rc = m_pPrivate->m_pVirtualBox->OpenMedium(
            CComBSTR(virtualbox_guest_additions.c_str()),
            DeviceType_DVD,
            AccessMode_ReadOnly,
            FALSE,
            &pGuestAdditionsImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            2,
            0,
            DeviceType_DVD,
            pGuestAdditionsImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Add a virtual cache disk drive to VM
        //
        if (enable_cache_disk){
            vboxlog_msg("Adding virtual cache disk drive to VM. (%s)", cache_disk_filename.c_str());
            CComPtr<IMedium> pCacheImage;
            rc = m_pPrivate->m_pVirtualBox->OpenMedium(
                CComBSTR(string(virtual_machine_slot_directory + "\\" + cache_disk_filename).c_str()),
                DeviceType_HardDisk,
                AccessMode_ReadWrite,
                TRUE,
                &pCacheImage
            );
            if (CHECK_ERROR(rc)) goto CLEANUP;

            rc = pMachine->AttachDevice(
                CComBSTR("Hard Disk Controller"),
                1,
                0,
                DeviceType_HardDisk,
                pCacheImage
            );
            if (CHECK_ERROR(rc)) goto CLEANUP;
        }
    } else {
        // Adding virtual hard drive to VM
        //
        vboxlog_msg("Adding virtual disk drive to VM. (%s)", image_filename.c_str());
        CComPtr<IMedium> pDiskImage;
        rc = m_pPrivate->m_pVirtualBox->OpenMedium(
            CComBSTR(string(virtual_machine_slot_directory + "\\" + image_filename).c_str()),
            DeviceType_HardDisk,
            AccessMode_ReadWrite,
            TRUE,
            &pDiskImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            0,
            0,
            DeviceType_HardDisk,
            pDiskImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Add guest additions to the VM
        //
        vboxlog_msg("Adding VirtualBox Guest Additions to VM.");
        CComPtr<IMedium> pGuestAdditionsImage;
        rc = m_pPrivate->m_pVirtualBox->OpenMedium(
            CComBSTR(virtualbox_guest_additions.c_str()),
            DeviceType_DVD,
            AccessMode_ReadOnly,
            FALSE,
            &pGuestAdditionsImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pMachine->AttachDevice(
            CComBSTR("Hard Disk Controller"),
            1,
            0,
            DeviceType_DVD,
            pGuestAdditionsImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;
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
            retval = ERR_FWRITE;
            goto CLEANUP;
        }

        vboxlog_msg("Adding virtual floppy disk drive to VM.");
        CComPtr<IMedium> pFloppyImage;
        rc = m_pPrivate->m_pVirtualBox->OpenMedium(
            CComBSTR(string(virtual_machine_slot_directory + "\\" + floppy_image_filename).c_str()),
            DeviceType_Floppy,
            AccessMode_ReadWrite,
            TRUE,
            &pFloppyImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pMachine->AttachDevice(
            CComBSTR("Floppy Controller"),
            0,
            0,
            DeviceType_Floppy,
            pFloppyImage
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;
    }

    // Add network bandwidth throttle group
    //
    vboxlog_msg("Adding network bandwidth throttle group to VM. (Defaulting to 1024GB)");
    rc = pBandwidthControl->CreateBandwidthGroup(
        CComBSTR(string(vm_name + "_net").c_str()),
        BandwidthGroupType_Network,
        (LONG64)1024*1024*1024*1024
    );
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Configure port forwarding
    //
    if (enable_network) {
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
            rc = pNATEngine->AddRedirect(
                CComBSTR(""),
                NATProtocol_TCP,
                pf.is_remote?CComBSTR(""):CComBSTR("127.0.0.1"),
                pf.host_port,
                CComBSTR(""),
                pf.guest_port
            );
            if (CHECK_ERROR(rc)) goto CLEANUP;
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
            if (retval) goto CLEANUP;

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
        vboxlog_msg("Enabling shared directory for VM.");
        rc = pMachine->CreateSharedFolder(
            CComBSTR("shared"),
            CComBSTR(string(virtual_machine_slot_directory + "\\shared").c_str()),
            TRUE,
            TRUE
        );
        if (CHECK_ERROR(rc)) goto CLEANUP;
    }

CLEANUP:
    if (pMachine) {
        pMachine->SaveSettings();
    }
    if (pSession) {
        pSession->UnlockMachine();
    }

    return retval;
}

int VBOX_VM::register_vm() {
    int retval = ERR_EXEC;
    HRESULT rc;
    string virtual_machine_slot_directory;
    APP_INIT_DATA aid;
    CComPtr<IMachine> pMachine;

    boinc_get_init_data_p(&aid);
    get_slot_directory(virtual_machine_slot_directory);


    vboxlog_msg("Register VM. (%s, slot#%d)", vm_master_name.c_str(), aid.slot);


    // Reset VM name in case it was changed while deregistering a stale VM
    //
    vm_name = vm_master_name;

    rc = m_pPrivate->m_pVirtualBox->OpenMachine(
        CComBSTR(string(virtual_machine_slot_directory + "\\" + vm_name + "\\" + vm_name + ".vbox").c_str()),
        &pMachine
    );
    if (CHECK_ERROR(rc)) goto CLEANUP;

    rc = m_pPrivate->m_pVirtualBox->RegisterMachine(pMachine);
    if (CHECK_ERROR(rc)) goto CLEANUP;

CLEANUP:
    return retval;
}

int VBOX_VM::deregister_vm(bool delete_media) {
    int retval = ERR_EXEC;
    HRESULT rc;
    SAFEARRAY* pHardDisks = NULL;
    SAFEARRAY* pEmptyHardDisks = NULL;
    SAFEARRAY* pMediumAttachments = NULL;
    CComSafeArray<LPDISPATCH> aMediumAttachments;
    CComSafeArray<LPDISPATCH> aHardDisks;
    CComPtr<ISession> pSession;
    CComPtr<IConsole> pConsole;
    CComPtr<IMachine> pMachineRO;
    CComPtr<IMachine> pMachine;
    CComPtr<IProgress> pProgress;
    CComPtr<IBandwidthControl> pBandwidthControl;
    CComPtr<ISnapshot> pRootSnapshot;
    std::vector<CComPtr<IMedium>> clean_mediums;
    std::vector<CComPtr<IMedium>> mediums;
    std::vector<std::string> snapshots;
    DeviceType device_type; 
    LONG lDevice;
    LONG lPort;
    string virtual_machine_slot_directory;
    APP_INIT_DATA aid;


    boinc_get_init_data_p(&aid);
    get_slot_directory(virtual_machine_slot_directory);


    vboxlog_msg("Deregistering VM. (%s, slot#%d)", vm_name.c_str(), aid.slot);


    rc = m_pPrivate->m_pVirtualBox->FindMachine(CComBSTR(vm_name.c_str()), &pMachineRO);
    if (SUCCEEDED(rc)) {
        if (delete_media) {
            rc = pSession.CoCreateInstance(CLSID_Session);
            if (CHECK_ERROR(rc)) goto CLEANUP;

            rc = pMachineRO->LockMachine(pSession, LockType_Write);
            if (CHECK_ERROR(rc)) goto CLEANUP;

            rc = pSession->get_Console(&pConsole);
            if (CHECK_ERROR(rc)) goto CLEANUP;

            rc = pSession->get_Machine(&pMachine);
            if (CHECK_ERROR(rc)) goto CLEANUP;


            // Delete snapshots
            //
            rc = pMachine->FindSnapshot(CComBSTR(""), &pRootSnapshot);
            if (SUCCEEDED(rc) && pRootSnapshot) {
                TraverseSnapshots(string(""), snapshots, pRootSnapshot);
            }
            if (snapshots.size()) {
                for (size_t i = 0; i < snapshots.size(); i++) {
                    CComPtr<IProgress> pProgress;
                    rc = pConsole->DeleteSnapshot(CComBSTR(snapshots[i].c_str()), &pProgress);
                    if (SUCCEEDED(rc)) {
                        pProgress->WaitForCompletion(-1);
                    } else {
                        CHECK_ERROR(rc);
                    }
                }
            }

            // Close Hard Disk, DVD, and floppy mediums
            //
           vboxlog_msg("Removing virtual disk drive(s) from VM.");
            rc = pMachine->get_MediumAttachments(&pMediumAttachments);
            if (SUCCEEDED(rc)) {
                aMediumAttachments.Attach(pMediumAttachments);
                for (int i = 0; i < (int)aMediumAttachments.GetCount(); i++) {
                    CComPtr<IMediumAttachment> pMediumAttachment((IMediumAttachment*)(LPDISPATCH)aMediumAttachments[i]);
                    rc = pMediumAttachment->get_Type(&device_type);
                    if (SUCCEEDED(rc) && 
                       ((DeviceType_HardDisk == device_type) ||
                        (DeviceType_DVD == device_type) ||
                        (DeviceType_Floppy == device_type))
                    ) {
                        CComPtr<IMedium> pMedium;
                        CComBSTR strController;

                        if ((DeviceType_HardDisk == device_type) || (DeviceType_DVD == device_type)) {
                            strController = "Hard Disk Controller";
                        } else {
                            strController = "Floppy Controller";
                        }

                        pMediumAttachment->get_Device(&lDevice);
                        pMediumAttachment->get_Port(&lPort);
                        pMediumAttachment->get_Medium(&pMedium);
                        
                        // If the device in question is a DVD/CD-ROM drive, the medium may have been ejected.
                        // If so, pMedium will be NULL.
                        if (pMedium) {
                            mediums.push_back(CComPtr<IMedium>(pMedium));
                        }
                        
                        rc = pMachine->DetachDevice(strController, lPort, lDevice);
                        CHECK_ERROR(rc);
                    }
                }
            }

            // Delete network bandwidth throttle group
            //
            vboxlog_msg("Removing network bandwidth throttle group from VM.");
            rc = pMachine->get_BandwidthControl(&pBandwidthControl);
            if (SUCCEEDED(rc)) {
                pBandwidthControl->DeleteBandwidthGroup(CComBSTR(string(vm_name + "_net").c_str()));
            }

            // Delete its storage controller(s)
            //
            vboxlog_msg("Removing storage controller(s) from VM.");
            pMachine->RemoveStorageController(CComBSTR("Hard Disk Controller"));
            if (enable_floppyio) {
                pMachine->RemoveStorageController(CComBSTR("Floppy Controller"));
            }

            // Save the VM Settings so the state is stored
            //
            rc = pMachine->SaveSettings();
            CHECK_ERROR(rc);

            // Now it should be safe to close down the mediums we detached.
            //
            for (int i = 0; i < (int)mediums.size(); i++) {
                mediums[i]->Close();
            }

            // Now free the session lock
            //
            rc = pSession->UnlockMachine();
            CHECK_ERROR(rc);
        }

        // Next, delete VM
        //
        vboxlog_msg("Removing VM from VirtualBox.");
        rc = pMachineRO->Unregister(CleanupMode_Full, &pHardDisks);
        if (SUCCEEDED(rc)) {

            // We only want to close(remove from media registry) the hard disks
            // instead of deleting them, order them by most recent image first
            // and then walk back to the root.
            //
            aHardDisks.Attach(pHardDisks);
            for (int i = 0; i < (int)aHardDisks.GetCount(); i++) {
                CComPtr<IMedium> pMedium((IMedium*)(LPDISPATCH)aHardDisks[i]);
                TraverseMediums(mediums, pMedium);
            }
            for (int i = 0; i < (int)mediums.size(); i++) {
                CComPtr<IMedium> pMedium(mediums[i]);
                pMedium->Close();
            }

#ifdef _VIRTUALBOX43_
            pMachineRO->DeleteConfig(pEmptyHardDisks, &pProgress);
            if (SUCCEEDED(rc)) {
                pProgress->WaitForCompletion(-1);
            } else {
                CHECK_ERROR(rc);
            }
#endif
#ifdef _VIRTUALBOX42_
            pMachineRO->Delete(pEmptyHardDisks, &pProgress);
            if (SUCCEEDED(rc)) {
                pProgress->WaitForCompletion(-1);
            } else {
                CHECK_ERROR(rc);
            }
#endif

        } else {
            CHECK_ERROR(rc);
        }
    }

CLEANUP:
    return 0;
}

int VBOX_VM::deregister_stale_vm() {
    HRESULT rc;
    SAFEARRAY* pHardDisks = NULL;
    SAFEARRAY* pISOImages = NULL;
    SAFEARRAY* pCacheDisks = NULL;
    SAFEARRAY* pMachines = NULL;
    SAFEARRAY* pISOMachines = NULL;
    SAFEARRAY* pCacheMachines = NULL;
    CComSafeArray<LPDISPATCH> aHardDisks;
    CComSafeArray<LPDISPATCH> aISOImages;
    CComSafeArray<LPDISPATCH> aCacheDisks;
    CComSafeArray<BSTR> aMachines;
    CComSafeArray<BSTR> aISOMachines;
    CComSafeArray<BSTR> aCacheMachines;
    CComPtr<IMedium> pHardDisk;
    CComPtr<IMedium> pISOImage;
    CComPtr<IMedium> pCacheDisk;
    CComPtr<IMachine> pMachine;
    CComBSTR strLocation;
    CComBSTR strMachineId;
    string virtual_machine_root_dir;
    string hdd_image_location;
    string iso_image_location;
    string cache_image_location;

    get_slot_directory(virtual_machine_root_dir);

    hdd_image_location = string(virtual_machine_root_dir + "\\" + image_filename);
    rc = m_pPrivate->m_pVirtualBox->get_HardDisks(&pHardDisks);
    if (SUCCEEDED(rc)) {
        aHardDisks.Attach(pHardDisks);
        for (int i = 0; i < (int)aHardDisks.GetCount(); i++) {
            pHardDisk = aHardDisks[i];
            pHardDisk->get_Location(&strLocation);
            if (0 == stricmp(hdd_image_location.c_str(), CW2A(strLocation))) {

                // Disk found
                //
                rc = pHardDisk->get_MachineIds(&pMachines);
                if (SUCCEEDED(rc) && pMachines) {
                    aMachines.Attach(pMachines);
                    // Delete all registered VMs attached to this disk image
                    //
                    for (int j = 0; j < (int)aMachines.GetCount(); j++) {
                        strMachineId = aMachines[j];
                        vm_name = CW2A(strMachineId);
                        deregister_vm(false);
                    }
                } else {
                    // Disk is in the Media Registry but now currently attached
                    // to a VM.  Close it.
                    pHardDisk->Close();
                }
            }
        }
    }

    if (enable_isocontextualization) {
        iso_image_location = string(virtual_machine_root_dir + "\\" + iso_image_filename);
        rc = m_pPrivate->m_pVirtualBox->get_DVDImages(&pISOImages);
        if (SUCCEEDED(rc)) {
            aISOImages.Attach(pISOImages);
            for (int i = 0; i < (int)aISOImages.GetCount(); i++) {
                pISOImage = aISOImages[i];
                pISOImage->get_Location(&strLocation);
                if (0 == stricmp(iso_image_location.c_str(), CW2A(strLocation))) {

                    // Image found
                    //
                    rc = pISOImage->get_MachineIds(&pISOMachines);
                    if (SUCCEEDED(rc) && pISOMachines) {
                        aISOMachines.Attach(pISOMachines);

                        // Delete all registered VMs attached to this disk image
                        //
                        for (int j = 0; j < (int)aISOMachines.GetCount(); j++) {
                            strMachineId = aISOMachines[j];
                            vm_name = CW2A(strMachineId);
                            deregister_vm(false);
                        }
                    } else {
                        // Disk is in the Media Registry but now currently attached
                        // to a VM.  Close it.
                        pISOImage->Close();
                    }
                }
            }
        }
    }

    if (enable_isocontextualization && enable_cache_disk) {
        cache_image_location = string(virtual_machine_root_dir + "\\" + cache_disk_filename);
        rc = m_pPrivate->m_pVirtualBox->get_HardDisks(&pCacheDisks);
        if (SUCCEEDED(rc)) {
            aCacheDisks.Attach(pCacheDisks);
            for (int i = 0; i < (int)aCacheDisks.GetCount(); i++) {
                pCacheDisk = aCacheDisks[i];
                pCacheDisk->get_Location(&strLocation);
                if (0 == stricmp(cache_image_location.c_str(), CW2A(strLocation))) {

                    // Disk found
                    //
                    rc = pCacheDisk->get_MachineIds(&pCacheMachines);
                    if (SUCCEEDED(rc) && pCacheMachines) {
                        aCacheMachines.Attach(pCacheMachines);

                        // Delete all registered VMs attached to this disk image
                        //
                        for (int j = 0; j < (int)aCacheMachines.GetCount(); j++) {
                            strMachineId = aCacheMachines[j];
                            vm_name = CW2A(strMachineId);
                            deregister_vm(false);
                        }
                    } else {
                        // Disk is in the Media Registry but now currently attached
                        // to a VM.  Close it.
                        pCacheDisk->Close();
                    }
                }
            }
        }
    }

    return 0;
}

void VBOX_VM::poll(bool log_state) {
    APP_INIT_DATA aid;
    HRESULT rc;
    CComPtr<IMachine> pMachine;
    MachineState vmstate;
    static MachineState vmstate_old = MachineState_PoweredOff;

    boinc_get_init_data_p(&aid);

    //
    // Is our environment still sane?
    //
    if (aid.using_sandbox && vboxsvc_pid_handle && !process_exists(vboxsvc_pid_handle)) {
        vboxlog_msg("Status Report: vboxsvc.exe is no longer running.");
    }
    if (vm_pid_handle && !process_exists(vm_pid_handle)) {
        vboxlog_msg("Status Report: virtualbox.exe/vboxheadless.exe is no longer running.");
    }

    //
    // What state is the VM in?
    //
    rc = m_pPrivate->m_pVirtualBox->FindMachine(CComBSTR(vm_master_name.c_str()), &pMachine);
    if (SUCCEEDED(rc) && pMachine) {
        rc = pMachine->get_State(&vmstate);
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
                    if (log_state) {
                        vboxlog_msg("VM is no longer is a running state. It is in '%s'.", MachineStateToName(vmstate));
                    }
                    break;
            }
            if (log_state && (vmstate_old != vmstate)) {
                vboxlog_msg(
                    "VM state change detected. (old = '%s', new = '%s')",
                    MachineStateToName(vmstate_old),
                    MachineStateToName(vmstate)
                );
                vmstate_old = vmstate;
            }
        }
    }

    //
    // Grab a snapshot of the latest log file.  Avoids multiple queries across several
    // functions.
    //
    if (online) {
        get_vm_log(vm_log);
    }

    //
    // Dump any new VM Guest Log entries
    //
    dump_vmguestlog_entries();
}

int VBOX_VM::start() {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComBSTR session_type;
    CComPtr<IMachine> pMachineRO;
    CComPtr<IProgress> pProgress;
    APP_INIT_DATA aid;
    long bCompleted = 0;
    double timeout;

    boinc_get_init_data_p(&aid);


    vboxlog_msg("Starting VM. (%s, slot#%d)", vm_name.c_str(), aid.slot);


    if (!headless) {
        session_type = _T("gui");
    } else {
        session_type = _T("headless");
    }

    rc = m_pPrivate->m_pVirtualBox->FindMachine(CComBSTR(vm_master_name.c_str()), &pMachineRO);
    if (SUCCEEDED(rc)) {

        // Start a VM session
        rc = pMachineRO->LaunchVMProcess(m_pPrivate->m_pSession, session_type, NULL, &pProgress);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Wait until VM is running.
        rc = pProgress->WaitForCompletion(-1);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        pProgress->get_Completed(&bCompleted);
        if (bCompleted) {

            // We should now own what goes on with the VM.
            //
            pMachineRO->LockMachine(m_pPrivate->m_pSession, LockType_Write);
            m_pPrivate->m_pSession->get_Machine(&m_pPrivate->m_pMachine);

            rc = m_pPrivate->m_pMachine->get_SessionPID((ULONG*)&vm_pid);
            if (CHECK_ERROR(rc)) goto CLEANUP;

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

            vboxlog_msg("Successfully started VM. (PID = '%d')", vm_pid);
            retval = BOINC_SUCCESS;
        } else {
            vboxlog_msg("VM failed to start.");
        }
    }

CLEANUP:
    return retval;
}

int VBOX_VM::stop() {
    int retval = ERR_EXEC;
    HRESULT rc;
    double timeout;
    CComPtr<IConsole> pConsole;
    CComPtr<IProgress> pProgress;


    vboxlog_msg("Stopping VM.");
    if (online) {
        // Get console object. 
        rc = m_pPrivate->m_pSession->get_Console(&pConsole);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Save the state of the machine.
        rc = pConsole->SaveState(&pProgress);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Wait until VM is powered down.
        rc = pProgress->WaitForCompletion(-1);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Wait for up to 5 minutes for the VM to switch states.  A system
        // under load can take a while.  Since the poll function can wait for up
        // to 45 seconds to execute a command we need to make this time based instead
        // of iteration based.
        timeout = dtime() + 300;
        do {
            poll(false);
            if (!online && !saving) break;
            boinc_sleep(1.0);
        } while (timeout >= dtime());

        if (!online) {
            vboxlog_msg("Successfully stopped VM.");
            retval = BOINC_SUCCESS;
        } else {
            vboxlog_msg("VM did not stop when requested.");

            // Attempt to terminate the VM
            retval = kill_program(vm_pid);
            if (retval) {
                vboxlog_msg("VM was NOT successfully terminated.");
            } else {
                vboxlog_msg("VM was successfully terminated.");
            }
        }

        m_pPrivate->m_pSession->UnlockMachine();
        boinc_sleep(5.0);
    }

CLEANUP:
    return retval;
}

int VBOX_VM::poweroff() {
    int retval = ERR_EXEC;
    HRESULT rc;
    double timeout;
    CComPtr<IConsole> pConsole;
    CComPtr<IProgress> pProgress;


    vboxlog_msg("Powering off VM.");
    if (online) {
        // Get console object. 
        rc = m_pPrivate->m_pSession->get_Console(&pConsole);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Power down the VM as quickly as possible.
        rc = pConsole->PowerDown(&pProgress);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Wait until VM is powered down.
        rc = pProgress->WaitForCompletion(-1);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        // Wait for up to 5 minutes for the VM to switch states.  A system
        // under load can take a while.  Since the poll function can wait for up
        // to 45 seconds to execute a command we need to make this time based instead
        // of iteration based.
        timeout = dtime() + 300;
        do {
            poll(false);
            if (!online && !saving) break;
            boinc_sleep(1.0);
        } while (timeout >= dtime());

        if (!online) {
            vboxlog_msg("Successfully stopped VM.");
            retval = BOINC_SUCCESS;
        } else {
            vboxlog_msg("VM did not power off when requested.");

            // Attempt to terminate the VM
            retval = kill_program(vm_pid);
            if (retval) {
                vboxlog_msg("VM was NOT successfully terminated.");
            } else {
                vboxlog_msg("VM was successfully terminated.");
            }
        }

        m_pPrivate->m_pSession->UnlockMachine();
        boinc_sleep(5.0);
    }

CLEANUP:
    return retval;
}

int VBOX_VM::pause() {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<IConsole> pConsole;


    // Restore the process priority back to the default process priority
    // to speed up the last minute maintenance tasks before the VirtualBox
    // VM goes to sleep
    //
    reset_vm_process_priority();


    // Get console object. 
    rc = m_pPrivate->m_pSession->get_Console(&pConsole);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Pause the machine.
    rc = pConsole->Pause();
    if (CHECK_ERROR(rc)) goto CLEANUP;

    retval = BOINC_SUCCESS;

CLEANUP:
    return retval;
}


int VBOX_VM::resume() {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<IConsole> pConsole;


    // Set the process priority back to the lowest level before resuming
    // execution
    //
    lower_vm_process_priority();


    // Get console object. 
    rc = m_pPrivate->m_pSession->get_Console(&pConsole);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Resume the machine.
    rc = pConsole->Resume();
    if (CHECK_ERROR(rc)) goto CLEANUP;

    retval = BOINC_SUCCESS;

CLEANUP:
    return retval;
}

int VBOX_VM::create_snapshot(double elapsed_time) {
    int retval = ERR_EXEC;
    char buf[256];
    HRESULT rc;
    CComPtr<IConsole> pConsole;
    CComPtr<IProgress> pProgress;

    vboxlog_msg("Creating new snapshot for VM.");

    // Pause VM - Try and avoid the live snapshot and trigger an online
    // snapshot instead.
    pause();

    // Create new snapshot
    rc = m_pPrivate->m_pSession->get_Console(&pConsole);
    if (CHECK_ERROR(rc)) {
    } else {
        sprintf(buf, "%d", (int)elapsed_time);
        rc = pConsole->TakeSnapshot(CComBSTR(string(string("boinc_") + buf).c_str()), CComBSTR(""), &pProgress);
        if (CHECK_ERROR(rc)) {
        } else {
            rc = pProgress->WaitForCompletion(-1);
            if (CHECK_ERROR(rc)) {
            }
        }
    }

    // Resume VM
    resume();

    // Set the suspended flag back to false before deleting the stale
    // snapshot
    poll(false);

    // Delete stale snapshot(s), if one exists
    cleanup_snapshots(false);

    if (BOINC_SUCCESS == retval) {
        vboxlog_msg("Checkpoint completed.");
    }

    return retval;
}

int VBOX_VM::cleanup_snapshots(bool delete_active) {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<IConsole> pConsole;
    CComPtr<ISnapshot> pCurrentSnapshot;
    CComPtr<ISnapshot> pRootSnapshot;
    CComBSTR tmp;
    std::string current_snapshot_id;
    std::vector<std::string> snapshots;

    rc = m_pPrivate->m_pSession->get_Console(&pConsole);
    if (CHECK_ERROR(rc)) goto CLEANUP;

    // Get the current snapshot, if we do not need to delete the active snapshot
    //
    if (!delete_active) {
        rc = m_pPrivate->m_pMachine->get_CurrentSnapshot(&pCurrentSnapshot);
        if (SUCCEEDED(rc) && pCurrentSnapshot) {
            rc = pCurrentSnapshot->get_Id(&tmp);
            if (SUCCEEDED(rc)) {
                current_snapshot_id = CW2A(tmp);
            }
        }
    }

    // Get the root snapshot and traverse the tree
    //
    rc = m_pPrivate->m_pMachine->FindSnapshot(CComBSTR(""), &pRootSnapshot);
    if (SUCCEEDED(rc) && pRootSnapshot) {
        TraverseSnapshots(current_snapshot_id, snapshots, pRootSnapshot);
    }

    // Delete stale snapshots
    //
    if (snapshots.size()) {
        for (size_t i = 0; i < snapshots.size(); i++) {
            CComPtr<IProgress> pProgress;

            vboxlog_msg("Deleting stale snapshot.");

            rc = pConsole->DeleteSnapshot(CComBSTR(snapshots[i].c_str()), &pProgress);
            if (SUCCEEDED(rc)) {
                pProgress->WaitForCompletion(-1);
            } else {
                CHECK_ERROR(rc);
            }
        }
    }

    retval = BOINC_SUCCESS;

CLEANUP:
    return retval;
}

int VBOX_VM::restore_snapshot() {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<ISession> pSession;
    CComPtr<IMachine> pMachineRO;
    CComPtr<IMachine> pMachine;
    CComPtr<IConsole> pConsole;
    CComPtr<ISnapshot> pSnapshot;
    CComPtr<IProgress> pProgress;

    rc = m_pPrivate->m_pVirtualBox->FindMachine(CComBSTR(vm_name.c_str()), &pMachineRO);
    if (SUCCEEDED(rc)) {
        rc = pSession.CoCreateInstance(CLSID_Session);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pMachineRO->LockMachine(pSession, LockType_Write);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pSession->get_Machine(&pMachine);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pSession->get_Console(&pConsole);
        if (CHECK_ERROR(rc)) goto CLEANUP;

        rc = pMachine->get_CurrentSnapshot(&pSnapshot);
        if (SUCCEEDED(rc)) {
            vboxlog_msg("Restore from previously saved snapshot.");
            rc = pConsole->RestoreSnapshot(pSnapshot, &pProgress);
            if (CHECK_ERROR(rc)) goto CLEANUP;

            rc = pProgress->WaitForCompletion(-1);
            if (CHECK_ERROR(rc)) goto CLEANUP;

            vboxlog_msg("Restore completed.");
        }

        retval = BOINC_SUCCESS;
    }

CLEANUP:
    if (pMachine) {
        pMachine->SaveSettings();
    }
    if (pSession) {
        pSession->UnlockMachine();
    }

    return retval;
}

void VBOX_VM::dump_hypervisor_status_reports() {
    SIZE_T ulMinimumWorkingSetSize;
    SIZE_T ulMaximumWorkingSetSize;

    if (
        GetProcessWorkingSetSize(
            vboxsvc_pid_handle,
            &ulMinimumWorkingSetSize,
            &ulMaximumWorkingSetSize)
    ) {
        vboxlog_msg(
            "Status Report (VirtualBox VboxSvc.exe): Minimum WSS: '%dKB', Maximum WSS: '%dKB'",
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
        vboxlog_msg(
            "Status Report (VirtualBox Vboxheadless.exe/VirtualBox.exe): Minimum WSS: '%dKB', Maximum WSS: '%dKB'",
            ulMinimumWorkingSetSize/1024,
            ulMaximumWorkingSetSize/1024
        );
    }
}

int VBOX_VM::is_registered() {
    int retval = ERR_NOT_FOUND;
    HRESULT rc;
    CComPtr<IMachine> pMachine;

    rc = m_pPrivate->m_pVirtualBox->FindMachine(CComBSTR(vm_master_name.c_str()), &pMachine);
    if (VBOX_E_OBJECT_NOT_FOUND != rc) {
        retval = BOINC_SUCCESS;
    }

    return retval;
}

bool VBOX_VM::is_system_ready(std::string& message) {
    return true;
}

bool VBOX_VM::is_disk_image_registered() {
    HRESULT rc;
    SAFEARRAY* pHardDisks = NULL;
    SAFEARRAY* pISOImages = NULL;
    SAFEARRAY* pCacheDisks = NULL;
    CComSafeArray<LPDISPATCH> aHardDisks;
    CComSafeArray<LPDISPATCH> aISOImages;
    CComSafeArray<LPDISPATCH> aCacheDisks;
    CComBSTR tmp;
    IMedium* pHardDisk;
    IMedium* pISOImage;
    IMedium* pCacheDisk;
    string virtual_machine_root_dir;
    string hdd_image_location;
    string iso_image_location;
    string cache_image_location;

    get_slot_directory(virtual_machine_root_dir);

    hdd_image_location = string(virtual_machine_root_dir + "\\" + image_filename);
    rc = m_pPrivate->m_pVirtualBox->get_HardDisks(&pHardDisks);
    if (SUCCEEDED(rc)) {
        aHardDisks.Attach(pHardDisks);
        for (int i = 0; i < (int)aHardDisks.GetCount(); i++) {
            pHardDisk = (IMedium*)(LPDISPATCH)aHardDisks[i];
            pHardDisk->get_Location(&tmp);
            if (0 == stricmp(hdd_image_location.c_str(), CW2A(tmp))) {
                return true;
            }
        }
    }

    if (enable_isocontextualization) {
        iso_image_location = string(virtual_machine_root_dir + "\\" + iso_image_filename);
        rc = m_pPrivate->m_pVirtualBox->get_DVDImages(&pISOImages);
        if (SUCCEEDED(rc)) {
            aISOImages.Attach(pISOImages);
            for (int i = 0; i < (int)aISOImages.GetCount(); i++) {
                pISOImage = (IMedium*)(LPDISPATCH)aISOImages[i];
                pISOImage->get_Location(&tmp);
                if (0 == stricmp(iso_image_location.c_str(), CW2A(tmp))) {
                    return true;
                }
            }
        }
    }

    if (enable_isocontextualization && enable_cache_disk) {
        cache_image_location = string(virtual_machine_root_dir + "\\" + cache_disk_filename);
        rc = m_pPrivate->m_pVirtualBox->get_HardDisks(&pCacheDisks);
        if (SUCCEEDED(rc)) {
            aCacheDisks.Attach(pHardDisks);
            for (int i = 0; i < (int)aCacheDisks.GetCount(); i++) {
                pCacheDisk = (IMedium*)(LPDISPATCH)aCacheDisks[i];
                pCacheDisk->get_Location(&tmp);
                if (0 == stricmp(cache_image_location.c_str(), CW2A(tmp))) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool VBOX_VM::is_extpack_installed() {
    CComPtr<IExtPackManager> pExtPackManager;
    CComPtr<IExtPack> pExtPack;
    long bUsable = 0;
    HRESULT rc;

    rc = m_pPrivate->m_pVirtualBox->get_ExtensionPackManager(&pExtPackManager);
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
        return ERR_FREAD;
    }
    return BOINC_SUCCESS;
}

int VBOX_VM::get_version_information(string& version) {
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
            _T("VersionExt"),
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
                _T("VersionExt"),
                NULL,
                NULL,
                (LPBYTE)lpszRegistryValue,
                &dwSize
            );

            version = lpszRegistryValue;
        }
    }

    if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszRegistryValue) free(lpszRegistryValue);
    if (version.empty()) {
        return ERR_FREAD;
    }
    return BOINC_SUCCESS;
}

int VBOX_VM::get_guest_additions(string& guest_additions) {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<ISystemProperties> properties;
    CComBSTR tmp;

    rc = m_pPrivate->m_pVirtualBox->get_SystemProperties(&properties);
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
    SAFEARRAY* pNICS = NULL;
    CComSafeArray<LPDISPATCH> aNICS;
    CComPtr<IHost> pHost;
    CComBSTR tmp;
    IHostNetworkInterface* pNIC;

    rc = m_pPrivate->m_pVirtualBox->get_Host(&pHost);
    if (SUCCEEDED(rc)) {
        rc = pHost->FindHostNetworkInterfacesOfType(HostNetworkInterfaceType_Bridged, &pNICS);
        if (SUCCEEDED(rc)) {
            // Automatically clean up array after use
            aNICS.Attach(pNICS);

            // We only need the 'default' nic, which is usally the first one.
            pNIC = (IHostNetworkInterface*)((LPDISPATCH)aNICS[0]);

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
    rc = m_pPrivate->m_pSession->get_Console(&pConsole);
    if (CHECK_ERROR(rc)) goto CLEANUP;

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
    rc = m_pPrivate->m_pSession->get_Console(&pConsole);
    if (CHECK_ERROR(rc)) goto CLEANUP;

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
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<INetworkAdapter> pNetworkAdapter;

    network_suspended = !enabled;

    if (enabled) {
        vboxlog_msg("Enabling network access for VM.");
        rc = m_pPrivate->m_pMachine->GetNetworkAdapter(0, &pNetworkAdapter);
        if (CHECK_ERROR(rc)) {
        } else {
            rc = pNetworkAdapter->put_Enabled(TRUE);
            if (SUCCEEDED(rc)) {
                retval = BOINC_SUCCESS;
            }
        }
    } else {
        vboxlog_msg("Disabling network access for VM.");
        rc = m_pPrivate->m_pMachine->GetNetworkAdapter(0, &pNetworkAdapter);
        if (CHECK_ERROR(rc)) {
        } else {
            rc = pNetworkAdapter->put_Enabled(FALSE);
            if (SUCCEEDED(rc)) {
                retval = BOINC_SUCCESS;
            }
        }
    }

    return retval;
}

int VBOX_VM::set_cpu_usage(int percentage) {
    vboxlog_msg("Setting CPU throttle for VM. (%d%%)", percentage);
    m_pPrivate->m_pMachine->put_CPUExecutionCap(percentage);
    return BOINC_SUCCESS;
}

int VBOX_VM::set_network_usage(int kilobytes) {
    int retval = ERR_EXEC;
    HRESULT rc;
    CComPtr<IBandwidthControl> pBandwidthControl;
    CComPtr<IBandwidthGroup> pBandwidthGroup;

    rc = m_pPrivate->m_pMachine->get_BandwidthControl(&pBandwidthControl);
    if (SUCCEEDED(rc)) {
        rc = pBandwidthControl->GetBandwidthGroup(CComBSTR(string(vm_name + "_net").c_str()), &pBandwidthGroup);
        if (SUCCEEDED(rc)) {
            if (kilobytes == 0) {
                vboxlog_msg("Setting network throttle for VM. (1024GB)");
                rc = pBandwidthGroup->put_MaxBytesPerSec((LONG64)1024*1024*1024*1024);
            } else {
                vboxlog_msg("Setting network throttle for VM. (%dKB)", kilobytes);
                rc = pBandwidthGroup->put_MaxBytesPerSec((LONG64)kilobytes*1024);
            }
            if (CHECK_ERROR(rc)) {
            } else {
                retval = BOINC_SUCCESS;
            }
        }
    }

    return retval;
}

void VBOX_VM::lower_vm_process_priority() {
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, BELOW_NORMAL_PRIORITY_CLASS);
    }
}

void VBOX_VM::reset_vm_process_priority() {
    if (vm_pid_handle) {
        SetPriorityClass(vm_pid_handle, NORMAL_PRIORITY_CLASS);
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
                vboxlog_msg("Status Report: Detected vboxsvc.exe. (PID = '%d')", pidVboxSvc);
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
                    vboxlog_msg("Status Report: Launching vboxsvc.exe. (PID = '%d')", pi.dwProcessId);
                    vboxsvc_pid = pi.dwProcessId;
                    vboxsvc_pid_handle = pi.hProcess;
                    retval = BOINC_SUCCESS;
                } else {
                    vboxlog_msg("Status Report: Launching vboxsvc.exe failed!.");
                    vboxlog_msg("        Error: %s", windows_format_error_string(GetLastError(), buf, sizeof(buf)));
                }
            }
        }
    }

    return retval;
}

// Launch the VM.
int VBOX_VM::launch_vboxvm() {
    char cmdline[1024];
    char* argv[5];
    int argc;
    std::string output;
    int retval = ERR_EXEC;
    char buf[256];
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
        vboxlog_msg("CreatePipe failed! (%d).", GetLastError());
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
        vboxlog_msg(
            "Status Report: Launching virtualbox.exe/vboxheadless.exe failed!."
        );
        vboxlog_msg(
            "        Error: %s (%d)",
            windows_format_error_string(GetLastError(), buf, sizeof(buf)),
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
        vboxlog_msg(
            "Status Report: Virtualbox.exe/Vboxheadless.exe exited prematurely!."
        );
        vboxlog_msg(
            "    Exit Code: %d",
            ulExitCode
        );
        vboxlog_msg(
            "       Output: %s",
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

#endif
