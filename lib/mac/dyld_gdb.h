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
/*
 *  dyld_gdb.h
 *
 */

/* This is part of a backtrace generator for boinc project applications.
*
* Adapted from Apple Developer Technical Support Sample Code QCrashReport
*
* This code handles Mac OS X 10.3.x through 10.4.9.  It may require some
* adjustment for future OS versions; see the discussion of _sigtramp and
* PowerPC Signal Stack Frames in file QBacktrace.c.
*
*  For useful tips on using backtrace information, see Apple Tech Note 2123:
*  http://developer.apple.com/technotes/tn2004/tn2123.html#SECNOSYMBOLS
*
*  To convert addresses to correct symbols, use the atos command-line tool:
*  atos -o path/to/executable/with/symbols address
*  Note: if address 1a23 is hex, use 0x1a23.
*
*  To demangle mangled C++ symbols, use the c++filt command-line tool.
*  You may need to prefix C++ symbols with an additional underscore before
*  passing them to c++filt (so they begin with two underscore characters).
*
* A very useful shell script to add symbols to a crash dump can be found at:
*  http://developer.apple.com/tools/xcode/symbolizingcrashdumps.html
* Pipe the output of the shell script through c++filt to demangle C++ symbols.
*/

/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef _DYLD_GDB_
#define _DYLD_GDB_
/*
 * This file describes the interface between gdb and dyld created for
 * MacOS X GM.  Prior to MacOS X GM gdb used the dyld_debug interfaces
 * described in <mach-o/dyld_debug.h>.
 */


#ifdef __cplusplus
extern "C" {
#endif


#define OLD_GDB_DYLD_INTERFACE __ppc__ || __i386__

#if OLD_GDB_DYLD_INTERFACE
/*
 * gdb_dyld_version is the version of gdb interface that dyld is currently
 * exporting.  For the interface described in this header file gdb_dyld_version
 * is 2.  As the gdb/dyld interface changes this number will be incremented and
 * comments will be added as to what are the are changes for the various
 * versions.
 */
extern unsigned int gdb_dyld_version;

/*
 * gdb_dyld_state_changed is the internal dyld routine called by dyld to notify
 * gdb that the state of the data structures has changed.  gdb is expected to
 * put a break point on this routine and re-read the internal dyld data
 * structures below when this break point is hit.
 */
extern void gdb_dyld_state_changed(void);

/*
 * gdb looks directly at parts of two of dyld's internal data structures.  The
 * list of object file images and the list of library images.  The parts of
 * these structures that gdb looks at will not change unless the value of
 * gdb_dyld_version changes.  The size of these structures and the other fields
 * that gdb does not look at may change.
 *
 *  struct object_images {
 *      struct object_image images[NOBJECT_IMAGES];
 *      unsigned long nimages;
 *      struct object_images *next_images;
 *      ...
 *  };
 *
 *  struct library_images {
 *      struct library_image images[NLIBRARY_IMAGES];
 *      unsigned long nimages;
 *      struct library_images *next_images;
 *      ...
 *  };
 *
 * Both the object_image structure and the library_image structure
 * start with a structure containing the following fields:
 *
 *  struct image {
 *      char *physical_name;        physical image name (file name)
 *      unsigned long vmaddr_slide; the slide from the staticly linked address
 *      struct mach_header *mh;     address of the mach header of the image
 *      unsigned long valid;        TRUE if this is struct is valid
 *      char *name;                 image name for reporting errors
 *      ...
 *  };
 *
 * In gdb_dyld_version 1 the first field was "name".  In gdb_dyld_version 2 the
 * first field was changed to "physical_name" and a new fifth field "name" was
 * added.  These two fields are set to the same values except in the case of
 * zero-link.  In zero-link the NSLinkModule() option
 * NSLINKMODULE_OPTION_TRAILING_PHYS_NAME is used and then the physical_name is
 * the file name of the module zero-link loaded that is part of the logical
 * image "name".
 */

/* object_images is the global object_images structure */

/* the number of gdb_object_image structures present per bucket */
extern unsigned int gdb_nobject_images;

/* the size of each gdb_object_image structure */
extern unsigned int gdb_object_image_size;

/* library_images is the global library_images structure */

/* the number of gdb_library_image structures present per bucket */
extern unsigned int gdb_nlibrary_images;

/* the size of each gdb_library_image structure */
extern unsigned int gdb_library_image_size;

#endif /* OLD_GDB_DYLD_INTERFACE */


/*
 *	Beginning in Mac OS X 10.4, there is a new mechanism for dyld to notify gdb and other about new images.
 *
 *
 */

enum dyld_image_mode { dyld_image_adding=0, dyld_image_removing=1 };

struct dyld_image_info {
	const struct mach_header*	imageLoadAddress;	/* base address image is mapped into */
	const char*					imageFilePath;		/* path dyld used to load the image */
	uintptr_t					imageFileModDate;	/* time_t of image file */
													/* if stat().st_mtime of imageFilePath does not match imageFileModDate, */
													/* then file has been modified since dyld loaded it */
};


typedef void (*dyld_image_notifier)(enum dyld_image_mode mode, uint32_t infoCount, const struct dyld_image_info info[]);

/*
 *	gdb looks for the symbol "_dyld_all_image_infos" in dyld.  It contains the fields below.
 *
 *	For a snap shot of what images are currently loaded, the infoArray fields contain a pointer
 *	to an array of all images. If infoArray is NULL, it means it is being modified, come back later.
 *
 *	To be notified of changes, gdb sets a break point on the notification field.  The function
 *	it points to is called by dyld with an array of information about what images have been added
 *	(dyld_image_adding) or are about to be removed (dyld_image_removing).
 *
 * The notification is called after infoArray is updated.  This means that if gdb attaches to a process
 * and infoArray is NULL, gdb can set a break point on notification and let the process continue to
 * run until the break point.  Then gdb can inspect the full infoArray.
 */
 struct dyld_all_image_infos {
	uint32_t						version;		/* == 1 in Mac OS X 10.4 */
	uint32_t						infoArrayCount;
	const struct dyld_image_info*	infoArray;
	dyld_image_notifier				notification;
	bool							processDetachedFromSharedRegion;
};
extern struct dyld_all_image_infos  dyld_all_image_infos;




#ifdef __cplusplus
}
#endif

#endif /* _DYLD_GDB_ */
