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


#ifndef BOINC_VBOXWRAPPER_H
#define BOINC_VBOXWRAPPER_H

#define MIN_MEMORY_SIZE_MB  512

#define IMAGE_FILENAME_COMPLETE "vm_image.vdi"
#define IMAGE_FILENAME "vm_image"
#define IMAGE_FILENAME_EXTENSION "vdi"
#define FLOPPY_IMAGE_FILENAME "vm_floppy"
#define FLOPPY_IMAGE_FILENAME_EXTENSION "img"
#define CACHE_DISK_FILENAME "vm_cache.vdi"
#define ISO_IMAGE_FILENAME "vm_isocontext.iso"

#define POLL_PERIOD 1.0

extern APP_INIT_DATA aid;
extern string slot_dir_path;
extern string project_dir_path;
extern string shared_dir;
    // 'shared' (if enable_shared_directory)
    // '.' (if share_slot_dir)

#endif
