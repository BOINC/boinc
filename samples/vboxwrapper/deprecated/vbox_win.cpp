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
#include "win_util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "boinc_api.h"
#include "vbox.h"
#include "vm.h"
#include "mscom/VirtualBox.h"


IVirtualBox*    pVirtualBox;
ISession*       pSession;


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


bool virtualbox_vm_is_registered() {
    bool retval = false;
    HRESULT rc;
    std::string name;
    BSTR vm_name;
    IMachine* pMachine = NULL;

    virtualbox_generate_vm_name(name);
    vm_name = SysAllocString(A2W(name).c_str());

    rc = pVirtualBox->FindMachine(vm_name, &pMachine);
    if (VBOX_E_OBJECT_NOT_FOUND != rc) {
        retval = true;
    }

    SysFreeString(vm_name);
    return retval;
}


bool virtualbox_vm_is_running() {
    bool retval = false;
    HRESULT rc;
    MachineState machine_state;
    IMachine* pMachine = NULL;

    rc = pSession->get_Machine(&pMachine);
    if (FAILED(rc)) goto CLEANUP;
    rc = pMachine->get_State(&machine_state);
    if (FAILED(rc)) goto CLEANUP;

    if ((machine_state >= MachineState_FirstOnline) &&
        (machine_state <= MachineState_LastOnline)
    ) {
        retval = true;
    } 

CLEANUP:
    if (pMachine) {
        pMachine->Release();
        pMachine = NULL;
    }
    return retval;
}


int virtualbox_initialize() {
    int retval = 0;
    HRESULT rc;
    char buf[256];

    pVirtualBox = NULL;
    pSession = NULL;

    // Initialize the COM subsystem.
    CoInitialize(NULL);

    // Instantiate the VirtualBox root object.
    rc = CoCreateInstance(CLSID_VirtualBox,       /* the VirtualBox base object */
                          NULL,                   /* no aggregation */
                          CLSCTX_LOCAL_SERVER,    /* the object lives in a server process on this machine */
                          IID_IVirtualBox,        /* IID of the interface */
                          (void**)&pVirtualBox);
    if (!SUCCEEDED(rc))
    {
        fprintf(
            stderr,
            "%s Error creating VirtualBox instance! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        retval = rc;
        goto CLEANUP;
    }

    // Create the session object.
    rc = CoCreateInstance(CLSID_Session,        /* the Session base object */
                          NULL,                 /* no aggregation */
                          CLSCTX_INPROC_SERVER, /* the object lives in the current process on this machine */
                          IID_ISession,         /* IID of the interface */
                          (void**)&pSession);
    if (!SUCCEEDED(rc))
    {
        fprintf(
            stderr,
            "%s Error creating Session instance! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        retval = rc;
        goto CLEANUP;
    }

    // Cleanup
CLEANUP:
    return retval;
}


int virtualbox_register_vm() {
    int retval = 0;
    HRESULT rc;
    std::string name;
    std::string harddisk_uuid;
    std::string root_dir;
    BSTR vm_root_dir;
    BSTR vm_name;
    BSTR vm_os_name;
    BSTR vm_os_version;
    BSTR vm_disk_image_name;
    BSTR vm_disk_image_type;
    BSTR vm_shared_folder_name;
    BSTR vm_shared_folder_dir_name;
    BSTR vm_machine_uuid;
    BSTR vm_harddisk_uuid;
    BSTR strIDEController = SysAllocString(A2W(_T("IDE Controller")).c_str());
    BSTR strEmpty = SysAllocString(A2W(_T("")).c_str());
    char buf[256];
    IMachine* pMachine = NULL;
    IStorageController* pStorageController = NULL;
    INetworkAdapter* pNetworkInterface = NULL;
    IMedium* pHardDisk = NULL;


    fprintf(
        stderr,
        "%s Registering virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    virtualbox_generate_vm_root_dir(root_dir);

    virtualbox_generate_vm_name(name);
    virtualbox_generate_medium_uuid(0, harddisk_uuid);

    vm_root_dir = SysAllocString(A2W(root_dir).c_str());
    vm_name = SysAllocString(A2W(name).c_str());
    vm_os_name = SysAllocString(A2W(vm.vm_os_name).c_str());
    vm_os_version = SysAllocString(A2W(vm.vm_os_version).c_str());
    vm_disk_image_type = SysAllocString(A2W(vm.vm_disk_image_type).c_str());
    vm_shared_folder_name = SysAllocString(A2W(vm.vm_shared_folder_name).c_str());
    vm_harddisk_uuid = SysAllocString(A2W(harddisk_uuid).c_str());

    // VirtualBox likes to see both the hard disk image and shared folder paths
    // in absolute terms.  So prefix the VM root directory to the job description
    // entry.
    vm_disk_image_name = SysAllocString(A2W(root_dir + std::string("/") + vm.vm_disk_image_name).c_str());
    vm_shared_folder_dir_name = SysAllocString(A2W(root_dir + std::string("/") + vm.vm_shared_folder_dir_name).c_str());


    // First create a unnamed new VM. It will be unconfigured and not be saved
    // in the configuration until we explicitely choose to do so.
    rc = pVirtualBox->CreateMachine(vm_name, vm_os_name, vm_root_dir, NULL, false, &pMachine);
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


    // Set some properties
    pMachine->put_MemorySize(vm.vm_memory_size);
    pMachine->SetCPUProperty(CPUPropertyType_PAE, TRUE);


    // Register the VM. Note that this call also saves the VM config
    // to disk. It is also possible to save the VM settings but not
    // register the VM.
    //
    // Also note that due to current VirtualBox limitations, the machine
    // must be registered *before* we can attach hard disks to it.
    //
    rc = pVirtualBox->RegisterMachine(pMachine);
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


    //
    // In order to manipulate the registered machine, we must open a session
    // for that machine. Do it now.
    //
    pMachine->get_Id(&vm_machine_uuid);
    rc = pVirtualBox->OpenSession(pSession, vm_machine_uuid);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not open session! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }


    //
    // After the machine is registered, the initial machine object becomes
    // immutable. In order to get a mutable machine object, we must query
    // it from the opened session object.
    //
    rc = pSession->get_Machine(&pMachine);
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not get sessioned machine! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }


    //
    // Open a virtual harddisk
    //
    rc = pVirtualBox->OpenHardDisk(
        vm_disk_image_name,
        AccessMode_ReadWrite,
        TRUE,
        vm_harddisk_uuid,
        FALSE,
        strEmpty,
        &pHardDisk
    );
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not open virtual disk image! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }


    //
    // Add storage controller
    //
    rc = pMachine->AddStorageController(
        strIDEController,                 // controller identifier
        StorageBus_SATA,
        &pStorageController
    );
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not add a storage controller! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }


    //
    // Now that it's created, we can assign it to the VM.
    //
    rc = pMachine->AttachDevice(
        strIDEController,               // controller identifier
        0,                              // channel number on the controller
        0,                              // device number on the controller
        DeviceType_HardDisk,
        vm_harddisk_uuid
    );
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error could not attach virtual disk image! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }


    //
    // By default VMs are created with a network interface, so we need to check if it should
    // be disabled/deleted.
    //
    if (!vm.enable_network) {
        rc = pMachine->GetNetworkAdapter(0, &pNetworkInterface);
        if (SUCCEEDED(rc)) {
            pNetworkInterface->Detach();
            if (FAILED(rc))
            {
                fprintf(
                    stderr,
                    "%s Error could not remove network interface! rc = 0x%x\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    rc
                );
                virtualbox_dump_error();
                retval = rc;
                goto CLEANUP;
            }
        }
    }


    //
    // Create shared folder
    //
    if (vm.enable_shared_directory) {
        rc = pMachine->CreateSharedFolder(
            vm_shared_folder_name,
            vm_shared_folder_dir_name,
            TRUE
        );
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not create shared folder! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }
    }
    

    //
    // Save all changes we've just made.
    //
    rc = pMachine->SaveSettings();
    if (FAILED(rc)) {
        fprintf(
            stderr,
            "%s Error not save machine settings! rc = 0x%x\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            rc
        );
        virtualbox_dump_error();
        retval = rc;
        goto CLEANUP;
    }

CLEANUP:
    if (vm_name) SysFreeString(vm_name);
    if (vm_os_name) SysFreeString(vm_os_name);
    if (vm_os_version) SysFreeString(vm_os_version);
    if (vm_disk_image_name) SysFreeString(vm_disk_image_name);
    if (vm_shared_folder_name) SysFreeString(vm_shared_folder_name);
    if (vm_shared_folder_dir_name) SysFreeString(vm_shared_folder_dir_name);
    if (vm_machine_uuid) SysFreeString(vm_machine_uuid);
    if (vm_harddisk_uuid) SysFreeString(vm_harddisk_uuid);
    if (strIDEController) SysFreeString(strIDEController);
    if (strEmpty) SysFreeString(strEmpty);

    if (pHardDisk) {
        pHardDisk->Release();
        pHardDisk = NULL;
    }
    if (pNetworkInterface) {
        pNetworkInterface->Release();
        pNetworkInterface = NULL;
    }
    if (pStorageController) {
        pStorageController->Release();
        pStorageController = NULL;
    }
    if (pMachine) {
        pMachine->Release();
        pMachine = NULL;
    }
    if (pSession) {
        pSession->Close();
    }

    // If we failed in any part of the registration process, unregister and cleanup
    // whatever is left over.
    if (FAILED(rc)) virtualbox_deregister_vm();

    return retval;
}


int virtualbox_deregister_vm() {
    int retval = 0;
    HRESULT rc;
    std::string name;
    std::string harddisk_uuid;
    BSTR vm_name;
    BSTR vm_machine_uuid;
    BSTR vm_harddisk_uuid;
    BSTR strIDEController = SysAllocString(A2W(_T("IDE Controller")).c_str());
    char buf[256];
    IMachine* pMachine = NULL;
    IMedium* pHardDisk = NULL;


    fprintf(
        stderr,
        "%s Deregistering virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    virtualbox_generate_vm_name(name);
    virtualbox_generate_medium_uuid(0, harddisk_uuid);

    vm_name = SysAllocString(A2W(name).c_str());
    vm_harddisk_uuid = SysAllocString(A2W(harddisk_uuid).c_str());

    rc = pVirtualBox->FindMachine(vm_name, &pMachine);
    if (SUCCEEDED(rc)) {

        //
        // In order to manipulate the registered machine, we must open a session
        // for that machine. Do it now.
        //
        pMachine->get_Id(&vm_machine_uuid);
        rc = pVirtualBox->OpenSession(pSession, vm_machine_uuid);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not open session! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }


        //
        // After the machine is registered, the initial machine object becomes
        // immutable. In order to get a mutable machine object, we must query
        // it from the opened session object.
        //
        rc = pSession->get_Machine(&pMachine);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not get sessioned machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

    
        //
        // Next we need to detach the storage controller
        //
        pMachine->DetachDevice(strIDEController, 0, 0);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not detach storage controller! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }


        //
        // Save all changes we've just made.
        //
        rc = pMachine->SaveSettings();
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error not save machine settings! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }


        //
        // Close the session so we can unregister the virtual machine.
        //
        pSession->Close();

    
        //
        // In order to delete the registered machine, we must unregister it first
        //
        rc = pVirtualBox->UnregisterMachine(vm_machine_uuid, &pMachine);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not unregister virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }

        rc = pMachine->DeleteSettings();
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not delete virtual machine! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }


        //
        // Cleanup virtual disk image if it exists
        //
        rc = pVirtualBox->GetHardDisk(vm_harddisk_uuid, &pHardDisk);
        if (SUCCEEDED(rc)) {
            pHardDisk->Close();
        }
    }

CLEANUP:
    if (vm_name) SysFreeString(vm_name);
    if (vm_machine_uuid) SysFreeString(vm_machine_uuid);
    if (vm_harddisk_uuid) SysFreeString(vm_harddisk_uuid);
    if (strIDEController) SysFreeString(strIDEController);

    if (pHardDisk) {
        pHardDisk->Release();
        pHardDisk = NULL;
    }
    if (pMachine) {
        pMachine->Release();
        pMachine = NULL;
    }

    return retval;
}


int virtualbox_cleanup() {

    if (pSession) {
        pSession->Close();
        pSession->Release();
        pSession = NULL;
    }

    if (pVirtualBox) {
        pVirtualBox->Release();
        pVirtualBox = NULL;
    }

    CoUninitialize();

    return 0;
}


int virtualbox_startvm() {
    int retval = 0;
    HRESULT rc;
    std::string name;
    BSTR vm_name;
    BSTR vm_machine_uuid;
    BSTR pSessionType = SysAllocString(L"vrdp");
    char buf[256];
    IMachine* pMachine = NULL;
    IProgress* pProgress = NULL;


    fprintf(
        stderr,
        "%s Starting virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    virtualbox_generate_vm_name(name);
    vm_name = SysAllocString(A2W(name).c_str());

    rc = pVirtualBox->FindMachine(vm_name, &pMachine);
    if (SUCCEEDED(rc)) {

        // Get the VM UUID
        rc = pMachine->get_Id(&vm_machine_uuid);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error retrieving machine ID! rc = 0x%x\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                rc
            );
            virtualbox_dump_error();
            retval = rc;
            goto CLEANUP;
        }


        // Start a VM session
        rc = pVirtualBox->OpenRemoteSession(pSession, vm_machine_uuid, pSessionType, NULL, &pProgress);
        if (FAILED(rc)) {
            fprintf(
                stderr,
                "%s Error could not open remote session! rc = 0x%x\n",
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
    }

CLEANUP:
    if (vm_machine_uuid) SysFreeString(vm_machine_uuid);
    if (pSessionType) SysFreeString(pSessionType);

    if (pProgress) {
        pProgress->Release();
        pProgress = NULL;
    }

    if (pMachine) {
        pMachine->Release();
        pMachine = NULL;
    }

    return retval;
}


int virtualbox_stopvm() {
    int retval = 0;
    HRESULT rc;
    char buf[256];
    IConsole* pConsole = NULL;
    IProgress* pProgress = NULL;


    fprintf(
        stderr,
        "%s Stopping virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    // Get console object. 
    rc = pSession->get_Console(&pConsole);
    if (0x8000ffff == rc) {
        // The session state has been closed, the VM has shutdown on its own.
        goto CLEANUP;
    }
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

CLEANUP:
    if (pProgress) {
        pProgress->Release();
        pProgress = NULL;
    }
    if (pConsole) {
        pConsole->Release();
        pConsole = NULL;
    }
    if (pSession) {
        pSession->Close();
    }
    return retval;
}


int virtualbox_pausevm() {
    int retval = 0;
    HRESULT rc;
    char buf[256];
    IConsole* pConsole = NULL;


    fprintf(
        stderr,
        "%s Pausing virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    // Get console object. 
    rc = pSession->get_Console(&pConsole);
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

CLEANUP:
    if (pConsole) {
        pConsole->Release();
        pConsole = NULL;
    }
    return retval;
}


int virtualbox_resumevm() {
    int retval = 0;
    HRESULT rc;
    char buf[256];
    IConsole* pConsole = NULL;


    fprintf(
        stderr,
        "%s Resuming virtual machine.\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );


    // Get console object. 
    rc = pSession->get_Console(&pConsole);
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

CLEANUP:
    if (pConsole) {
        pConsole->Release();
        pConsole = NULL;
    }
    return retval;
}


int virtualbox_monitor() {
    return 0;
}

