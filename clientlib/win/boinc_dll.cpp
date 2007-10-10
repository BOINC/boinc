// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


#include "stdafx.h"
#include "resource.h"
#include "win_util.h"
#include "IdleTracker.h"

// Declare a global hModule variable for this process, which will
//   be initialized when the DLL is loaded.
HMODULE g_hModule = NULL;

BOOL APIENTRY DllMain(
	HMODULE hModule, DWORD  ul_reason_for_call, LPVOID /* lpReserved */
)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hModule = hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
    return TRUE;
}

EXTERN_C __declspec(dllexport) BOOL ClientLibraryStartup()
{
	if (!IdleTrackerStartup())
        return FALSE;

    return TRUE;
}

EXTERN_C __declspec(dllexport) void ClientLibraryShutdown()
{
    IdleTrackerShutdown();
}

