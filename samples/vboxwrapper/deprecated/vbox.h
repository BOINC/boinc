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


// Provide cross-platform interfaces for making changes to VirtualBox

#ifndef _VBOX_H_
#define _VBOX_H_

extern int virtualbox_generate_vm_root_dir( std::string& dir );
extern int virtualbox_generate_vm_name( std::string& name );
extern int virtualbox_generate_medium_uuid( int drive_id, std::string& uuid );

extern bool virtualbox_vm_is_registered();
extern bool virtualbox_vm_is_running();

extern int virtualbox_initialize();
extern int virtualbox_register_vm();
extern int virtualbox_deregister_vm();
extern int virtualbox_cleanup();

extern int virtualbox_startvm();
extern int virtualbox_stopvm();
extern int virtualbox_pausevm();
extern int virtualbox_resumevm();

extern int virtualbox_monitor();

#endif
