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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#include "stdafx.h"
#include "resource.h"
#include "Identification.h"
#include "IdleTracker.h"
#include "NetworkTracker.h"


// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, 
         uuid = "{16B09F41-6216-4131-AADD-D66276A88089}", 
		 name = "BOINCSENS", 
		 helpstring = "BOINCSENS 1.0 Type Library",
		 resource_name = "IDR_BOINCSENS") ]
class CBOINCSENSModule
{
public:
// Override CAtlDllModuleT members
};


EXTERN_C __declspec(dllexport) BOOL ClientLibraryStartup()
{
    if (!IdleTrackerStartup())
        return FALSE;
    if (IsWindows2000Compatible()) {
        if (!NetworkTrackerStartup())
            return FALSE;
    }

    return TRUE;
}

EXTERN_C __declspec(dllexport) void ClientLibraryShutdown()
{
    IdleTrackerShutdown();
    if (IsWindows2000Compatible()) {
        NetworkTrackerShutdown();
    }
}

