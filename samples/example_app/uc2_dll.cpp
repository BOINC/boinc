// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
#else
#include "config.h"
#endif

#ifdef _WIN32

extern "C" {

    __declspec( dllexport ) BOOL WINAPI DllMain(
        HINSTANCE /*hinstDLL*/,  // handle to DLL module
        DWORD fdwReason,         // reason for calling function
        LPVOID /*lpReserved*/    // reserved
    ){
        // Perform actions based on the reason for calling.
        switch( fdwReason )
        {
            case DLL_PROCESS_ATTACH:
             // Initialize once for each new process.
             // Return FALSE to fail DLL load.
                break;

            case DLL_THREAD_ATTACH:
             // Do thread-specific initialization.
                break;

            case DLL_THREAD_DETACH:
             // Do thread-specific cleanup.
                break;

            case DLL_PROCESS_DETACH:
             // Perform any necessary cleanup.
                break;
        }
        return TRUE;  // Successful DLL_PROCESS_ATTACH.
    }

    __declspec( dllexport ) char* __cdecl uc2_dll_version() {
        return "example app DLL v1.0";
    }

}

#endif
