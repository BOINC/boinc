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
#include "mscom/VirtualBox.h"


IVirtualBox*    pVirtualBox;
ISession*       pSession;
IMachine*       pMachine;


int virtualbox_initialize() {
    int retval = 0;
    HRESULT rc;
    std::string name;
    BSTR vm_name;

    pVirtualBox = NULL;
    pSession = NULL;
    pMachine = NULL;


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
        fprintf(stderr, "Error creating VirtualBox instance! rc = 0x%x\n", rc);
        retval = 1;
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
        fprintf(stderr, "Error creating Session instance! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // What should the virtual machine be named?
    virtualbox_generate_vm_name(name);
    vm_name = SysAllocString(A2W(name).c_str());

    // Do we need to create a new VM instance or use an already registered
    // one?
    rc = pVirtualBox->FindMachine(vm_name, &pMachine);
    if (VBOX_E_OBJECT_NOT_FOUND == rc) {
        // We must create and register the virtual machine

    } else if (!SUCCEEDED(rc)) {
        fprintf(stderr, "Error searching for VM instance! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Cleanup
CLEANUP:
    SysFreeString(vm_name);

    return retval;
}

int virtualbox_cleanup() {

    // Cleanup.
    if (pSession) {
        pSession->Release();
        pSession = NULL;
    }

    if (pMachine) {
        pMachine->Release();
        pMachine = NULL;
    }

    CoUninitialize();

    return 0;
}

int virtualbox_startvm() {
    int retval = 0;
    HRESULT rc;
    IProgress* pProgress;
    BSTR pMachineUUID;
    BSTR pSessionType = SysAllocString(L"vrdp");

    // Get the VM UUID
    rc = pMachine->get_Id(&pMachineUUID);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Error retrieving machine ID! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Start a VM session
    rc = pVirtualBox->OpenRemoteSession(pSession, pMachineUUID, pSessionType, NULL, &pProgress);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Could not open remote session! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Wait until VM is running.
    rc = pProgress->WaitForCompletion(-1);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Could not wait for VM start completion! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Cleanup
CLEANUP:
    if (pProgress) {
        pProgress->Release();
        pProgress = NULL;
    }
    SysFreeString(pMachineUUID);
    SysFreeString(pSessionType);

    return retval;
}

int virtualbox_stopvm() {
    int retval = 0;
    HRESULT rc;
    IConsole* pConsole;
    IProgress* pProgress;

    // Get console object. 
    rc = pSession->get_Console(&pConsole);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Error retrieving console object! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Power down the machine.
    rc = pConsole->PowerDown(&pProgress);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Could not stop VM! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Wait until VM is powered down.
    rc = pProgress->WaitForCompletion(-1);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Could not wait for VM stop completion! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Close the session.
    pSession->Close();

    // Cleanup
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
        pSession->Release();
        pSession = NULL;
    }
    return retval;
}

int virtualbox_pausevm() {
    int retval = 0;
    HRESULT rc;
    IConsole* pConsole;

    // Get console object. 
    rc = pSession->get_Console(&pConsole);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Error retrieving console object! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Pause the machine.
    rc = pConsole->Pause();
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Could not pause VM! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Cleanup
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
    IConsole* pConsole;

    // Get console object. 
    rc = pSession->get_Console(&pConsole);
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Error retrieving console object! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Resume the machine.
    rc = pConsole->Resume();
    if (!SUCCEEDED(rc))
    {
        fprintf(stderr, "Could not resume VM! rc = 0x%x\n", rc);
        retval = 1;
        goto CLEANUP;
    }

    // Cleanup
CLEANUP:
    if (pConsole) {
        pConsole->Release();
        pConsole = NULL;
    }
    return retval;
}
