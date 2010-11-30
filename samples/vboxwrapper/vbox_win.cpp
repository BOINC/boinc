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
#include "mscom/VirtualBox.h"


IVirtualBox*    pVirtualBox;
ISession*       pSession;
IMachine*       pMachine;


int virtualbox_initialize() {
    HRESULT rc;

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
        return rc;
    }

    return 0;
}

int virtualbox_cleanup() {

    // Cleanup.
    CoUninitialize();

    return 0;
}

int virtualbox_enumeratevms() {
    HRESULT rc;

    SAFEARRAY *machinesArray = NULL;

    rc = pVirtualBox->get_Machines(&machinesArray);
    if (SUCCEEDED(rc))
    {
        IMachine **machines;
        rc = SafeArrayAccessData (machinesArray, (void **) &machines);
        if (SUCCEEDED(rc))
        {
            for (ULONG i = 0; i < machinesArray->rgsabound[0].cElements; ++i)
            {
                BSTR str;

                rc = machines[i]->get_Name(&str);
                if (SUCCEEDED(rc))
                {
                    fprintf(stderr, "Name: %S\n", str);
                    SysFreeString(str);
                }
            }

            SafeArrayUnaccessData (machinesArray);
        }

        SafeArrayDestroy (machinesArray);
    }

    return 0;
}

int virtualbox_startvm() {
    return 1;
}

int virtualbox_stopvm() {
    return 0;
}

int virtualbox_pausevm() {
    return 0;
}

int virtualbox_resumevm() {
    return 0;
}
