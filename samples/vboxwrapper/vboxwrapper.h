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


// Provide cross-platform interfaces for making changes to VirtualBox

#ifndef _VBOXWRAPPER_H_
#define _VBOXWRAPPER_H_

#define IMAGE_FILENAME_COMPLETE "vm_image.vdi"
#define IMAGE_FILENAME "vm_image"
#define IMAGE_FILENAME_EXTENSION "vdi"
#define FLOPPY_IMAGE_FILENAME "vm_floppy"
#define FLOPPY_IMAGE_FILENAME_EXTENSION "img"
#define JOB_FILENAME "vbox_job.xml"
#define CHECKPOINT_FILENAME "vbox_checkpoint.txt"
#define PORTFORWARD_FILENAME "vbox_port_forward.xml"
#define REMOTEDESKTOP_FILENAME "vbox_remote_desktop.xml"
#define POLL_PERIOD 1.0

extern char* vboxwrapper_msg_prefix(char*, int);

#endif
