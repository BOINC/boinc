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

#define _VIRTUALBOX43_
#define _VIRTUALBOX_IMPORT_FUNCTIONS_

#include "vbox_mscom43.h"

using vbox43::ISnapshot;
using vbox43::MachineState;
#define MachineState_Null	                vbox43::MachineState_Null
#define MachineState_PoweredOff	            vbox43::MachineState_PoweredOff	
#define MachineState_Saved	                vbox43::MachineState_Saved	
#define MachineState_Teleported	            vbox43::MachineState_Teleported	
#define MachineState_Aborted	            vbox43::MachineState_Aborted	
#define MachineState_Running	            vbox43::MachineState_Running	
#define MachineState_Paused	                vbox43::MachineState_Paused	
#define MachineState_Stuck	                vbox43::MachineState_Stuck	
#define MachineState_Teleporting	        vbox43::MachineState_Teleporting	
#define MachineState_LiveSnapshotting	    vbox43::MachineState_LiveSnapshotting	
#define MachineState_Starting	            vbox43::MachineState_Starting	
#define MachineState_Stopping	            vbox43::MachineState_Stopping	
#define MachineState_Saving	                vbox43::MachineState_Saving	
#define MachineState_Restoring	            vbox43::MachineState_Restoring	
#define MachineState_TeleportingPausedVM	vbox43::MachineState_TeleportingPausedVM	
#define MachineState_TeleportingIn	        vbox43::MachineState_TeleportingIn	
#define MachineState_FaultTolerantSyncing	vbox43::MachineState_FaultTolerantSyncing	
#define MachineState_DeletingSnapshotOnline	vbox43::MachineState_DeletingSnapshotOnline	
#define MachineState_DeletingSnapshotPaused	vbox43::MachineState_DeletingSnapshotPaused	
#define MachineState_RestoringSnapshot  	vbox43::MachineState_RestoringSnapshot	
#define MachineState_DeletingSnapshot	    vbox43::MachineState_DeletingSnapshot	
#define MachineState_SettingUp	            vbox43::MachineState_SettingUp	
#define MachineState_FirstOnline	        vbox43::MachineState_FirstOnline	
#define MachineState_LastOnline	            vbox43::MachineState_LastOnline	
#define MachineState_FirstTransient	        vbox43::MachineState_FirstTransient	
#define MachineState_LastTransient          vbox43::MachineState_LastTransient

using vbox43::VBOX_VM;

#include "vbox_mscom_impl.cpp"
