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
 *  QMachOImage.c
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
*  You may need to prefix C++ symbols with an additonal underscore before 
*  passing them to c++filt (so they begin with two underscore characters).
*
* A very useful shell script to add symbols to a crash dump can be found at:
*  http://developer.apple.com/tools/xcode/symbolizingcrashdumps.html
* Pipe the output of the shell script through c++filt to demangle C++ symbols.
*/

/*
    File:       QMachOImage.c

    Contains:   Mach-O image access.

    Written by: DTS

    Copyright:  Copyright (c) 2007 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Inc.
                ("Apple") in consideration of your agreement to the following
                terms, and your use, installation, modification or
                redistribution of this Apple software constitutes acceptance of
                these terms.  If you do not agree with these terms, please do
                not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following
                terms, and subject to these terms, Apple grants you a personal,
                non-exclusive license, under Apple's copyrights in this
                original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or
                without modifications, in source and/or binary forms; provided
                that if you redistribute the Apple Software in its entirety and
                without modifications, you must retain this notice and the
                following text and disclaimers in all such redistributions of
                the Apple Software. Neither the name, trademarks, service marks
                or logos of Apple Inc. may be used to endorse or promote
                products derived from the Apple Software without specific prior
                written permission from Apple.  Except as expressly stated in
                this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any
                patent rights that may be infringed by your derivative works or
                by other works in which the Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis. 
                APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
                WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
                MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
                THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT,
                INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
                TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
                DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY
                OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
                OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
                OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF
                SUCH DAMAGE.

    Change History (most recent first):

$Log: QMachOImage.c,v $
Revision 1.1  2007/03/02 12:20:14         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes
#include "config.h"
#include "QMachOImage.h"

// Basic system interfaces

#include <TargetConditionals.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

// Mach-O interfaces

#include <mach-o/arch.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>

#if TARGET_CPU_X86 || TARGET_CPU_X86_64
#include <mach/mach_vm.h>
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** Compile-Time Assert Information

// At various places this code makes assumptions about the layout of various 
// structures.  Where I noticed these assumptions, I've used a compile-time 
// assert to check them.  Don't ask me how this works or I'll start to whimper.

#define compile_time_assert__(name, line) name ## line
#define compile_time_assert_(mustBeTrue, name, line)  \
    struct compile_time_assert__(name, line) { unsigned int msg: (mustBeTrue); }
#define compile_time_assert(mustBeTrue)         \
    compile_time_assert_(mustBeTrue, CompileTimeAssert, __LINE__)

/////////////////////////////////////////////////////////////////
#pragma mark ***** Core Code

// QMOSegment records information about a segment within an image.  This is helpful for 
// a bunch of reasons.
//
// o We store an array of these structures, which makes it easy to access the segments 
//   by simple array indexing.
//
// o The structure always store information about the segment as 64-bits, which means 
//   that, as we access the information, we don't need to worry whether it's 32- or 64-bit. 
//
// o The structure always store information about the segment in native byte endian format.
//
// o It records whether we have the segment mapped, and the information required to 
//   unmap it

struct QMOSegment {
	const struct load_command *	cmd;			// pointer to original load command
	struct segment_command_64	seg;			// copy of segment information, promoted to 64-bits if necessary
	const char *				segBase;		// base address of mapped segment (or NULL if not mapped)
	void *						segMapRefCon;	// place for image type to store information about the mapping
};
typedef struct QMOSegment QMOSegment;

// Likewise, we keep an array of information about all the sections in the image. 
// This is primarily to support the various section access APIs.

struct QMOSection {
	const QMOSegment *          seg;			// parent segment
	struct section_64           sect;			// copy of section information, promoted to 64-bits if necessary
};
typedef struct QMOSection QMOSection;

typedef struct QMOImage QMOImage;
	// forward declaration

// Image Type Callbacks
// --------------------
// There are various types of QMOImage (local, from a remote task, from disk) that have 
// lots in common.  However, in some cases it's necessary to do things diffently for each 
// type of image to do different things.  In that case, the common code calls the image 
// type specific code to do the work.

typedef int  (*QMOMapRangeProc)(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void **   basePtr, 
    void **         mapRefConPtr
);
	// Map a chunk of an image into the local address space.  qmoImage is a reference 
	// to the image itself.  offset and length define the chunk of the image.  The 
	// callback can store a value in *mapRefConPtr to assist in its QMOUnmapRangeProc 
	// implementation.
	// 
	// On entry, qmoImage must not be NULL
	// On entry, offset is the offset, in bytes, of the chunk to map, relative to the 
	// machHeaderOffset supplied when the image was created
	// On entry, length is the number of bytes to map; must be greater than 0
	// On entry, basePtr must not be NULL
	// On entry, *basePtr must be NULL
	// On entry, mapRefConPtr must not be NULL
	// Returns an errno-style error code
	// On success, *basePtr must be the (non-NULL) address of the mapped chunk

typedef void  (*QMOUnmapRangeProc)(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void *    base, 
    void *          mapRefCon
);
	// Unmap a chunk of an image from the local address space.  qmoImage is a reference 
	// to the image itself.  offset and length define the chunk of the image; these
	// will be exactly equal to one of the chunks previously mapped (that is, we 
	// won't map [100..200) and then unmap [100..150)).  base is the address of the 
	// mapping, as returned by a previous QMOMapRangeProc call.  mapRefCon is the 
	// per-mapping data, as returned by a previous QMOMapRangeProc call.
	//
	// On entry, qmoImage must not be NULL
	// On entry, offset is the offset, in bytes, of the chunk to unmap, relative to the 
	// machHeaderOffset supplied when the image was created
	// On entry, length is the number of bytes to unmap; must be greater than 0
	// On entry, base must not be NULL

typedef int (*QMOCreateProc)(QMOImage *qmoImage, void *refCon);
    // Called to tell the image type specific code to initialise itself.  
    // The purpose of this callback is to commit the refCon to the 
    // image.  Regardless of whether this routine succeeds or not, it 
    // must leave the refCon in a state that can be correctly handled 
    // by the destroy callback (if any).
    //
    // This callback is optional.  If it's not present, the core code 
    // commits the refCon by simple assignment.
    //
	// On entry, qmoImage must not be NULL

typedef void (*QMODestroyProc)(QMOImage *qmoImage);
	// Destroy the type-specific data associated with the image.  If you have 
    // a create callback, you are guaranteed that it will be called before 
    // this callback (although it might have failed).
    //
    // This callback is optional.  If it's not present, the core code does 
    // nothing to destroy the refCon.
	//
	// On entry, qmoImage must not be NULL

// QMOImage represents a Mach-O image.  It's the backing for the exported QMOImageRef
// type.

struct QMOImage {
    const char *                filePath;               // can be NULL
    
	// Mach-O header information
	
	QTMAddr						machHeaderOffset;		// offset within 'container'
	const struct mach_header *	machHeader;				// address of Mach-O header mapped into local process; includes all load commands
	void *						machHeaderRefCon;		// refCon for unmapping
	size_t						machHeaderSize;			// size of Mach-O header, including load commands

    // Cached information
    
    bool                        imageIDCached;          // true iff imageID is valid
    const char *                imageID;                // NULL is a valid value

	// General information
	
	QTMAddr						slide;					// amount to add to segment vmaddr to get actual addr in memory
	bool						prepared;				// false if coming from file, true if coming from memory
	bool						is64Bit;				// true if 64-bit Mach-O
	bool						byteSwapped;			// true if byte swapped relative to current process
	
	uint32_t					segCount;				// number of segments
	QMOSegment *				segments;				// per-segment information
    uint32_t                    sectCount;              // number of sections
    QMOSection *                sections;               // per-section information
	
	QMOMapRangeProc				mapRange;				// type-specific mapping routine
	QMOUnmapRangeProc			unmapRange;				// type-specific unmapping routine
	QMODestroyProc				destroy;				// type-specific destroy routine (optional)
	
	void *						refCon;					// storage for type-specific information
};

#if ! defined(NDEBUG)

    static bool QMOImageIsValidLimited(QMOImageRef qmoImage)
        // A limited form of QMOImageIsValid that's callback while qmoImage is still 
        // being constructed.
    {
        return (qmoImage != NULL)
            && (qmoImage->mapRange != NULL)
            && (qmoImage->unmapRange != NULL);
    }

    static bool QMOImageIsValid(QMOImageRef qmoImage)
        // Returns true if qmoImage is valid.
    {
        return (qmoImage != NULL)
            && (qmoImage->machHeader != NULL)
            && (qmoImage->machHeaderSize >= sizeof(struct mach_header))
            && (qmoImage->segCount > 0)
            && (qmoImage->segments != NULL)
            && (qmoImage->segments[0].cmd != NULL)
            && (qmoImage->sectCount > 0)
            && (qmoImage->sections != NULL)
            && (qmoImage->sections[0].seg != NULL)
            && (qmoImage->mapRange != NULL)
            && (qmoImage->unmapRange != NULL);
    }

#endif

typedef bool (*LoadCommandCompareProc)(QMOImageRef qmoImage, const struct load_command *cmd, void *compareRefCon);
	// Callback for FindLoadCommand.
	//
	// IMPORTANT:
	// The contents of the load command pointed to be cmd may be byte swapped.
	//
	// On entry, qmoImage will not be NULL
	// On entry, cmd will not be NULL
	// Return true if this load command is the one you're looking for

static const struct load_command * FindLoadCommand(
	QMOImageRef				qmoImage, 
	LoadCommandCompareProc	compareProc, 
	void *					compareRefCon
)
	// Finds a load command within a Mach-O image.  Iterates through the load commands, 
	// from first to last, calling compareProc on each one.  If compareProc returns 
	// true, that load command becomes the function result.  If compareProc never returns 
	// true, the function result is NULL.
{
	uint32_t	cmdIndex;
	uint32_t	cmdCount;
	const struct load_command *	firstCommand;
	const struct load_command *	thisCommand;
	const char *				commandLimit;
	const struct load_command *	result;
	
	assert(qmoImage    != NULL);
	assert(qmoImage->machHeader != NULL);
	assert(compareProc != NULL);
	
	// Get the command count.
	
	cmdCount = QMOImageToLocalUInt32(qmoImage, qmoImage->machHeader->ncmds);
	
	// Get the start of the first command.
	
	if (qmoImage->is64Bit) {
		firstCommand = (const struct load_command *) (((const char *) qmoImage->machHeader) + sizeof(struct mach_header_64));
	} else {
		firstCommand = (const struct load_command *) (((const char *) qmoImage->machHeader) + sizeof(struct mach_header));
	}
	
	// Iterate through the commands until compareProc returns true.
	
	result = NULL;
	cmdIndex = 0;
	commandLimit = ((const char *) qmoImage->machHeader) + qmoImage->machHeaderSize;
	thisCommand = firstCommand;
	while ( (result == NULL) && (cmdIndex < cmdCount) ) {
		uint32_t	thisCommandSize;
		
		thisCommandSize = QMOImageToLocalUInt32(qmoImage, thisCommand->cmdsize);
		
		assert( ((const char *) thisCommand) >= ((const char *) firstCommand) );
		assert( ((const char *) thisCommand) <  commandLimit );
		assert( (((const char *) thisCommand) + thisCommandSize) <= commandLimit );
		
		if ( compareProc(qmoImage, thisCommand, compareRefCon) ) {
			result = thisCommand;
		} else {
			thisCommand = (const struct load_command *) (((const char *) thisCommand) + QMOImageToLocalUInt32(qmoImage, thisCommand->cmdsize));
			cmdIndex += 1;
		}
	}
	
	return result;
}

static int MapMachHeader(QMOImageRef qmoImage)
	// Map an image's Mach-O header (including its load commands) into memory, 
	// recording that mapping in the fields in qmoImage.
{
	int	err;

	assert(qmoImage != NULL);
	assert(qmoImage->machHeader == NULL);			// that is, we haven't already mapped it
	
	compile_time_assert( offsetof(struct mach_header, magic)      == offsetof(struct mach_header_64, magic) );
	compile_time_assert( offsetof(struct mach_header, sizeofcmds) == offsetof(struct mach_header_64, sizeofcmds) );
	
	// Temporarily map the mach_header itself, so we can figure out what sort of 
    // header we're dealing with and get the size of the load commands.
	
	{
		size_t						headerSize;
		const struct mach_header *	headerPtr;
		void *						headerRefCon;
		
		headerPtr  = NULL;

		err = qmoImage->mapRange(
			qmoImage, 
			qmoImage->machHeaderOffset, 
			sizeof(struct mach_header), 
			(const void **) &headerPtr, 
			&headerRefCon
		);
		if (err == 0) {

			// Test the magic to check for Mach-O validity, size and byte order.
			
			switch (headerPtr->magic) {
				case MH_MAGIC_64:
				case MH_CIGAM_64:
				case MH_MAGIC:
				case MH_CIGAM:
					qmoImage->is64Bit     = ( (headerPtr->magic == MH_MAGIC_64) || (headerPtr->magic == MH_CIGAM_64) );
					qmoImage->byteSwapped = ( (headerPtr->magic == MH_CIGAM)    || (headerPtr->magic == MH_CIGAM_64) );
					if (qmoImage->is64Bit) {
						headerSize = sizeof(struct mach_header_64);
					} else {
						headerSize = sizeof(struct mach_header);
					}
					
					// Now that we have set up qmoImage->byteSwapped, we can call QMOImageToLocalUInt32.
					
					qmoImage->machHeaderSize = headerSize + QMOImageToLocalUInt32(qmoImage, headerPtr->sizeofcmds);
					break;
				default:
					err = EINVAL;
					break;
			}
			
			// Undo our temporary mapping.
			
			qmoImage->unmapRange(qmoImage, qmoImage->machHeaderOffset, sizeof(struct mach_header), headerPtr, headerRefCon);
		}
	}
	
	// Map the entire header permanently. 
	
	if (err == 0) {
		err = qmoImage->mapRange(
			qmoImage, 
			qmoImage->machHeaderOffset, 
			qmoImage->machHeaderSize, 
			(const void **) &qmoImage->machHeader, 
			&qmoImage->machHeaderRefCon
		);
	}
	
	assert( (err == 0) == (qmoImage->machHeader != NULL) );
	
	return err;
}

static bool CountSegmentsAndSections(QMOImageRef qmoImage, const struct load_command *cmd, void *compareRefCon)
	// A LoadCommandCompareProc callback used to count the number of segments 
	// and sections in the image.  For each segment load command, increment the 
    // image's segment count by one and then extract the nsect field of the segment 
    // and increment the image's section count by that.  Always return false so 
    // we iterate all load commands.
	//
	// See LoadCommandCompareProc for a description of the other parameters.
{
	uint32_t	cmdID;
	
	assert(qmoImage != NULL);
	assert(cmd      != NULL);
	assert(compareRefCon == NULL);
	
	cmdID = QMOImageToLocalUInt32(qmoImage, cmd->cmd);
	if ( (cmdID == LC_SEGMENT) || (cmdID == LC_SEGMENT_64) ) {
		qmoImage->segCount += 1;
		if (cmdID == LC_SEGMENT_64) {
			qmoImage->sectCount += QMOImageToLocalUInt32(qmoImage, ((const struct segment_command_64 *) cmd)->nsects);
		} else {
			qmoImage->sectCount += QMOImageToLocalUInt32(qmoImage, ((const struct segment_command *)    cmd)->nsects);
		}
	}
	return false;
}

static bool InitSegment(QMOImageRef qmoImage, const struct load_command *cmd, void *compareRefCon)
	// A LoadCommandCompareProc callback used to initialise the segments array.
	// For each segment load command, set up the corresponding element of 
	// qmoImage->segments and increment the counter pointed to be compareRefCon.
	// Always return false so we iterate all load commands.
	//
	// See LoadCommandCompareProc for a description of the other parameters.
{
	uint32_t *					segIndexPtr;
	uint32_t					cmdID;
	struct segment_command_64 *	outputSegCmd;

	assert(qmoImage != NULL);
	assert(cmd      != NULL);
	assert(compareRefCon != NULL);
	assert(qmoImage->segments != NULL);					// that is, we've already allocated the segment array

	cmdID = QMOImageToLocalUInt32(qmoImage, cmd->cmd);
	if ( (cmdID == LC_SEGMENT) || (cmdID == LC_SEGMENT_64) ) {
		segIndexPtr = (uint32_t *) compareRefCon;
		
		// Allocate a segment array element.
		
		assert(*segIndexPtr < qmoImage->segCount);		// segCount was set up by CountSegmentsAndSections, so we should never run out of elements
		qmoImage->segments[*segIndexPtr].cmd = cmd;
		outputSegCmd = &qmoImage->segments[*segIndexPtr].seg;
		*segIndexPtr += 1;
		
		// Convert the incoming segment command into the canonical 
		// segment_command_64 that we use internally.  For all segments, this 
		// involves possible byte swapping.  For 32-bit segments, we also have 
		// to promote 32-bit values to 64 bits.
		
		if (cmdID == LC_SEGMENT_64) {
			const struct segment_command_64 *	segCmd64;

			segCmd64 = (const struct segment_command_64 *) cmd;

			outputSegCmd->cmd      = QMOImageToLocalUInt32(qmoImage, segCmd64->cmd);
			outputSegCmd->cmdsize  = QMOImageToLocalUInt32(qmoImage, segCmd64->cmdsize);
			memcpy(outputSegCmd->segname, segCmd64->segname, sizeof(outputSegCmd->segname));
			outputSegCmd->vmaddr   = QMOImageToLocalUInt64(qmoImage, segCmd64->vmaddr);
			outputSegCmd->vmsize   = QMOImageToLocalUInt64(qmoImage, segCmd64->vmsize);
			outputSegCmd->fileoff  = QMOImageToLocalUInt64(qmoImage, segCmd64->fileoff);
			outputSegCmd->filesize = QMOImageToLocalUInt64(qmoImage, segCmd64->filesize);
			outputSegCmd->maxprot  = QMOImageToLocalUInt32(qmoImage, segCmd64->maxprot);
			outputSegCmd->initprot = QMOImageToLocalUInt32(qmoImage, segCmd64->initprot);
			outputSegCmd->nsects   = QMOImageToLocalUInt32(qmoImage, segCmd64->nsects);
			outputSegCmd->flags    = QMOImageToLocalUInt32(qmoImage, segCmd64->flags);
		} else {
			const struct segment_command *		segCmd;

			segCmd = (const struct segment_command *) cmd;

			outputSegCmd->cmd      = QMOImageToLocalUInt32(qmoImage, segCmd->cmd);
			outputSegCmd->cmdsize  = QMOImageToLocalUInt32(qmoImage, segCmd->cmdsize);
			memcpy(outputSegCmd->segname, segCmd->segname, sizeof(outputSegCmd->segname));
			outputSegCmd->vmaddr   = QMOImageToLocalUInt32(qmoImage, segCmd->vmaddr);
			outputSegCmd->vmsize   = QMOImageToLocalUInt32(qmoImage, segCmd->vmsize);
			outputSegCmd->fileoff  = QMOImageToLocalUInt32(qmoImage, segCmd->fileoff);
			outputSegCmd->filesize = QMOImageToLocalUInt32(qmoImage, segCmd->filesize);
			outputSegCmd->maxprot  = QMOImageToLocalUInt32(qmoImage, segCmd->maxprot);
			outputSegCmd->initprot = QMOImageToLocalUInt32(qmoImage, segCmd->initprot);
			outputSegCmd->nsects   = QMOImageToLocalUInt32(qmoImage, segCmd->nsects);
			outputSegCmd->flags    = QMOImageToLocalUInt32(qmoImage, segCmd->flags);
		}
	}
	return false;
}

static void InitSegmentSections(QMOImageRef qmoImage, uint32_t segIndex, uint32_t *sectIndexPtr)
    // Called to initialise the sections array.  segIndex indicates that segment 
    // whose sections we should parse.  On entry, *sectIndexPtr is an index 
    // into the sections array of the place where we should start placing 
    // section information.  On return, we've update *sectIndexPtr for every 
    // section that we filled in.
{
    uint32_t                    sectCount;
    uint32_t                    sectIndex;
    const struct section_64 *   sectArray64;
    const struct section *      sectArray;
    bool                        is64;
    
    // The section information is stored in an array with fixed size elements. 
    // However, both the base of the array and size of the elements varies for 
    // 32- and 64-bit segments. 

    is64 = (qmoImage->segments[segIndex].seg.cmd == LC_SEGMENT_64);
    if (is64) {
        sectArray64 = (const struct section_64 *) (((const char *) qmoImage->segments[segIndex].cmd) + sizeof(struct segment_command_64));
        sectArray   = NULL;
    } else {
        sectArray64 = NULL;
        sectArray   = (const struct section    *) (((const char *) qmoImage->segments[segIndex].cmd) + sizeof(struct segment_command   ));
    }
    
    // Iterate the sections in the segment.
    
    sectCount = qmoImage->segments[segIndex].seg.nsects;
    for (sectIndex = 0; sectIndex < sectCount; sectIndex++) {
        struct section_64 *     outputSect;
        
        // Set up the segment pointer for this section.
        
        qmoImage->sections[*sectIndexPtr + sectIndex].seg = &qmoImage->segments[segIndex];
        
		// Convert the incoming section structure into the canonical 
		// section_64 structure that we use internally.  For all sections, this 
		// involves possible byte swapping.  For sections in 32-bit segments, we 
        // also have to promote 32-bit values to 64 bits.
        
        assert( (*sectIndexPtr + sectIndex) < qmoImage->sectCount );
        outputSect = &qmoImage->sections[*sectIndexPtr + sectIndex].sect;
        if (is64) {
			const struct section_64 *	sect64;

			sect64 = &sectArray64[sectIndex];

            memcpy(outputSect->sectname, sect64->sectname, sizeof(outputSect->sectname));
            memcpy(outputSect->segname,  sect64->segname,  sizeof(outputSect->segname));
            outputSect->addr      = QMOImageToLocalUInt64(qmoImage, sect64->addr);
            outputSect->size      = QMOImageToLocalUInt64(qmoImage, sect64->size);
            outputSect->offset    = QMOImageToLocalUInt32(qmoImage, sect64->offset);
            outputSect->align     = QMOImageToLocalUInt32(qmoImage, sect64->align);
            outputSect->reloff    = QMOImageToLocalUInt32(qmoImage, sect64->reloff);
            outputSect->nreloc    = QMOImageToLocalUInt32(qmoImage, sect64->nreloc);
            outputSect->flags     = QMOImageToLocalUInt32(qmoImage, sect64->flags);
            outputSect->reserved1 = QMOImageToLocalUInt32(qmoImage, sect64->reserved1);
            outputSect->reserved2 = QMOImageToLocalUInt32(qmoImage, sect64->reserved2);
            outputSect->reserved3 = QMOImageToLocalUInt32(qmoImage, sect64->reserved3);
        } else {
			const struct section *      sect;

			sect = &sectArray[sectIndex];

            memcpy(outputSect->sectname, sect->sectname, sizeof(outputSect->sectname));
            memcpy(outputSect->segname,  sect->segname,  sizeof(outputSect->segname));
            outputSect->addr      = QMOImageToLocalUInt32(qmoImage, sect->addr);
            outputSect->size      = QMOImageToLocalUInt32(qmoImage, sect->size);
            outputSect->offset    = QMOImageToLocalUInt32(qmoImage, sect->offset);
            outputSect->align     = QMOImageToLocalUInt32(qmoImage, sect->align);
            outputSect->reloff    = QMOImageToLocalUInt32(qmoImage, sect->reloff);
            outputSect->nreloc    = QMOImageToLocalUInt32(qmoImage, sect->nreloc);
            outputSect->flags     = QMOImageToLocalUInt32(qmoImage, sect->flags);
            outputSect->reserved1 = QMOImageToLocalUInt32(qmoImage, sect->reserved1);
            outputSect->reserved2 = QMOImageToLocalUInt32(qmoImage, sect->reserved2);
            outputSect->reserved3 = 0;
        }
    }
    
    // Increment *sectIndexPtr for the number of sections that we added.
    
    *sectIndexPtr += sectCount;
}

static int GetSegmentIndexByName(QMOImageRef qmoImage, const char *segName, uint32_t *segIndexPtr)
    // Search the segments array for the first segment with the specified 
    // name.  This is the guts of QMOImageGetSegmentByName.
    //
    // *** Should merge this into QMOImageGetSegmentByName.
{
	int			err;
	bool		found;
	uint32_t	segIndex;
	
	assert(QMOImageIsValid(qmoImage));
	assert(segName != NULL);
	assert(segIndexPtr != NULL);

	found = false;
	segIndex = 0;
	while ( ! found && segIndex < qmoImage->segCount ) {
		found = ( strcmp(qmoImage->segments[segIndex].seg.segname, segName) == 0 );
		if ( ! found ) {
			segIndex += 1;
		}
	}

	if (found) {
		*segIndexPtr = segIndex;
		err = 0;
	} else {
		err = ESRCH;
	}
	return err;
}

static int CalculateSlide(QMOImageRef qmoImage)
	// Calculate the slide associated with an image.  In some cases, we can get 
	// the slide (for example, for a local image, you can get the slide using 
	// _dyld_get_image_vmaddr_slide), but in other cases you always have to 
	// calculate it (for example, when using new-style remote image access). 
	// So, rather than messing around with conditional code, I always just 
	// calculate it myself.
{
	int							err;
	
	assert(QMOImageIsValid(qmoImage));
	
	if ( ! qmoImage->prepared ) {
		// For file-based images, we assume a slide of 0.
		
		assert(qmoImage->slide == 0);
		err = 0;
	} else {
		uint32_t					textSegIndex;
		struct segment_command_64	textSeg;

		// For in-memory images, we have to do the maths.  We find the __TEXT 
		// segment, find its virtual address (the address it wanted to load), 
		// and subtract that away from the start of the image (the address 
		// that the __TEXT segment ended up loading).  This is slightly bogus 
		// because it's possible to construct a Mach-O image where the 
		// header isn't embedded in the __TEXT segment.  But, in reality, this 
		// doesn't happen (and, if it did, other tools, like vmutils) would also 
		// fail).
		
		err = GetSegmentIndexByName(qmoImage, "__TEXT", &textSegIndex);
		if (err == 0) {
			err = QMOImageGetSegmentByIndex(qmoImage, textSegIndex, &textSeg);
		}
		if (err == 0) {
			qmoImage->slide = qmoImage->machHeaderOffset - textSeg.vmaddr;
		}
	}
	return err;
}

static int QMOImageCreate(
	QTMAddr				machHeaderOffset, 
    const char *        filePath,
	bool				prepared, 
	QMOMapRangeProc		mapRange, 
	QMOUnmapRangeProc	unmapRange, 
	QMOCreateProc		create, 
	QMODestroyProc		destroy, 
	void *				refCon, 
	QMOImageRef *		qmoImagePtr
)
	// Common code use by all types of images to create an image object. 
	//
	// mapRange, unmapRange, create and destroy are callbacks used to implement 
	// type-specific behaviour.  See the description of their corresponding 
	// function pointer definition for details.
	//
	// refCon is data storage for the type-specific code.  For example, for 
	// a file-based image, it is used to hold the file descriptor of the mapped 
	// file.
	// 
    // filePath is the Mach-O backing store file for the image.  It may be NULL.
    //
	// prepared indicates whether the image is in memory, having been prepared 
	// by dyld, or is coming from a Mach-O file.  In the first case, the image 
	// operates using virtual addresses.  In the second case, the image operates 
	// using file offsets.
	//
	// machHeaderOffset is the offset, within the container defined by the 
	// type-specific code, of the Mach-O header.  Specifically:
	//
	// o for a file-based image (unprepared), this is the offset within the file
	// o for a memory-based image (prepared), this is the virtual address of the 
	//   mach_header
	//
	// When calling the mapRange and unmapRange callbacks, the core code assumes that:
	// 
	// o the Mach-O header is available at machHeaderOffset
	// o for an unprepared image, it adds machHeaderOffset to file-relative values to
	//   get the actual file offset of a segment
	// o for prepared images, it ignores machHeaderOffset and accesses the segment 
	//   via its virtual address
{
	int			err;
	QMOImageRef	qmoImage;
    uint32_t	segIndex;
    uint32_t    sectIndex;
	
    // machHeaderOffset could be zero
    // filePath may be NULL
	assert(mapRange     != NULL);
	assert(unmapRange   != NULL);
    // create may be NULL
    // destroy may be NULL
	assert( qmoImagePtr != NULL);
	assert(*qmoImagePtr == NULL);
	
	// Allocate the memory for the image object, and fill out out the basic fields. 
	// This involves mapping in the Mach-O header to figure out the width (32- or 
	// 64-bit) and byte order of the image.
	
	err = 0;
	qmoImage = calloc(1, sizeof(*qmoImage));
	if (qmoImage == NULL) {
		err = ENOMEM;
	}
	if (err == 0) {
		qmoImage->machHeaderOffset = machHeaderOffset;
		qmoImage->prepared   = prepared;
		qmoImage->mapRange   = mapRange;
		qmoImage->unmapRange = unmapRange;
		qmoImage->destroy    = destroy;

        // It is vital that we call the create callback (if any) before any 
        // other failure (other than the calloc).  If there was a failure 
        // point before this, we could end up calling the destroy callback 
        // before calling the create callback.  Thatd woul be bad for image 
        // types where a NULL refcon isn't appropriate as a nil value.  
        //
        // An example of this is the file image type.  The in this case, the 
        // nil value for the refcon is -1.  The create callback is expected 
        // to set up the refCon correctly; if it fails, it must set it to 
        // an appropriate nil value.

        if (create == NULL) {
            qmoImage->refCon = refCon;
        } else {
            err = create(qmoImage, refCon);
        }
    }
    if ( (err == 0) && (filePath != NULL) ) {
        qmoImage->filePath = strdup(filePath);
        if (qmoImage->filePath == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
		err = MapMachHeader(qmoImage);
	}
	
	// Count the segments, allocate the array used to hold information about them, 
	// and then fill in that array.  In the process, also fill out the sections 
    // array.
	
	if (err == 0) {
		(void) FindLoadCommand(qmoImage, CountSegmentsAndSections, NULL);
		
		qmoImage->segments = calloc(qmoImage->segCount,  sizeof(*qmoImage->segments));
		qmoImage->sections = calloc(qmoImage->sectCount, sizeof(*qmoImage->sections));
		if ( (qmoImage->segments == NULL) && (qmoImage->sections == NULL) ) {
			err = ENOMEM;
		}
		
	}
	if (err == 0) {
		segIndex = 0;

		(void) FindLoadCommand(qmoImage, InitSegment, &segIndex);

        assert(segIndex == qmoImage->segCount);
	}
    if (err == 0) {
        sectIndex = 0;

        for (segIndex = 0; segIndex < qmoImage->segCount; segIndex++) {
            InitSegmentSections(qmoImage, segIndex, &sectIndex);
        }

        assert(sectIndex == qmoImage->sectCount);
    }
	
	// Once we have the segment information, we can calculate the slide.
	
	if (err == 0) {
		err = CalculateSlide(qmoImage);
	}

	// Clean up on error.
	
	if (err != 0) {
		QMOImageDestroy(qmoImage);
		qmoImage = NULL;
	}
	*qmoImagePtr = qmoImage;
	
	assert( (err == 0) == QMOImageIsValid(*qmoImagePtr) );
	
	return err;
}

extern void QMOImageDestroy(QMOImageRef qmoImage)
	// See comment in header.
{
	uint32_t	segIndex;
	
	if (qmoImage != NULL) {

		// Destroy the segments array.

		if (qmoImage->segments != NULL) {
			// Unmap any mapped segments.
				
			for (segIndex = 0; segIndex < qmoImage->segCount; segIndex++) {
				if (qmoImage->segments[segIndex].segBase != NULL) {
					QTMAddr offset;
					QTMAddr size;
					
					if (qmoImage->prepared) {
						offset = qmoImage->segments[segIndex].seg.vmaddr + qmoImage->slide;
						size   = qmoImage->segments[segIndex].seg.vmsize;
					} else {
						offset = qmoImage->segments[segIndex].seg.fileoff + qmoImage->machHeaderOffset;
						size   = qmoImage->segments[segIndex].seg.filesize;
					}
					qmoImage->unmapRange(qmoImage, offset, size, qmoImage->segments[segIndex].segBase, qmoImage->segments[segIndex].segMapRefCon);
					
					qmoImage->segments[segIndex].segBase = 0;
					qmoImage->segments[segIndex].segMapRefCon = NULL;
				}
			}
			free(qmoImage->segments);
		}

		// Unmap the Mach-O header.
		
		if (qmoImage->machHeader != NULL) {
			qmoImage->unmapRange(qmoImage, qmoImage->machHeaderOffset, qmoImage->machHeaderSize, qmoImage->machHeader, qmoImage->machHeaderRefCon);
		}
		
		// Now that we're done unmapping, call the type-specific destroy callback 
		// so that it can clean up any information that it needed to have 
		// (typically this is hung off the refCon).
		
        if (qmoImage->destroy != NULL) {
            qmoImage->destroy(qmoImage);
        }

		// Free the memory for the object itself.
		
        free( (void *) qmoImage->filePath );
		free(qmoImage);
	}
}

static int QMOImageMapSegmentByIndex(QMOImageRef qmoImage, uint32_t segIndex, const char **segBasePtr)
	// Map a segment from a Mach-O image into the local process, returning the address 
	// of the mapped data.
{
	int			err;
	
	assert(QMOImageIsValid(qmoImage));
	assert(segIndex < qmoImage->segCount);
	assert(segBasePtr != NULL);

	// Have we mapped it yet?  If not, we have to do some heavy lifting.

	err = 0;
	if (qmoImage->segments[segIndex].segBase == NULL) {
		QTMAddr offset;
		QTMAddr size;
		
		// No, we need to map it now.
		
		// Work out what to map.  If the image has been prepared (that is, 
		// we're dealing with an image that's been mapped into memory 
		// by dyld), we operate on virtual addresses.  OTOH, if the image 
		// is coming from a file, we operate in file-relative addresses.
		
		if (qmoImage->prepared) {
			offset = qmoImage->segments[segIndex].seg.vmaddr + qmoImage->slide;
			size   = qmoImage->segments[segIndex].seg.vmsize;
		} else {
			offset = qmoImage->segments[segIndex].seg.fileoff + qmoImage->machHeaderOffset;
			size   = qmoImage->segments[segIndex].seg.filesize;
		}
		err = qmoImage->mapRange(
			qmoImage, 
			offset, 
			size, 
			(const void **) &qmoImage->segments[segIndex].segBase, 
			&qmoImage->segments[segIndex].segMapRefCon
		);
	}
	
	// If all went well, return the address to the caller.
	
	if (err == 0) {
		*segBasePtr = qmoImage->segments[segIndex].segBase;
	}
	return err;
}

#pragma mark ***** Accessors

extern QTMAddr	QMOImageGetSlide(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));
	
	return qmoImage->slide;
}

extern bool QMOImageIs64Bit(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

	return qmoImage->is64Bit;
}

extern bool QMOImageIsByteSwapped(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

	return qmoImage->byteSwapped;
}

extern QTMAddr QMOImageGetMachHeaderOffset(QMOImageRef qmoImage)
{
	assert(QMOImageIsValid(qmoImage));

	return qmoImage->machHeaderOffset;
}

extern const struct mach_header * QMOImageGetMachHeader(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

	return qmoImage->machHeader;
}

extern const char * QMOImageGetFilePath(QMOImageRef qmoImage)
{
	assert(QMOImageIsValid(qmoImage));

    return qmoImage->filePath;
}

extern uint32_t QMOImageGetFileType(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

	compile_time_assert( offsetof(struct mach_header, filetype) == offsetof(struct mach_header_64, filetype) );

	return QMOImageToLocalUInt32(qmoImage, qmoImage->machHeader->filetype);
}

extern uint32_t QMOImageGetCPUType(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

	compile_time_assert( offsetof(struct mach_header, cputype) == offsetof(struct mach_header_64, cputype) );

	return QMOImageToLocalUInt32(qmoImage, qmoImage->machHeader->cputype);
}

extern uint32_t QMOImageGetCPUSubType(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

	assert( offsetof(struct mach_header, cpusubtype) == offsetof(struct mach_header_64, cpusubtype) );
	assert( offsetof(struct mach_header, cpusubtype) == offsetof(struct mach_header_64, cpusubtype) );

	return QMOImageToLocalUInt32(qmoImage, qmoImage->machHeader->cpusubtype);
}

static bool CommandIDCompareProc(QMOImageRef qmoImage, const struct load_command *cmd, void *compareRefCon)
	// A LoadCommandCompareProc callback used to implement FindLoadCommandByID.
	// compareRefCon is a pointer to the Mach-O command ID that we're looking for.
	//
	// See LoadCommandCompareProc for a description of the other parameters.
{
	assert(QMOImageIsValid(qmoImage));
	assert(cmd != NULL);

	return ( QMOImageToLocalUInt32(qmoImage, cmd->cmd) == *(uint32_t *) compareRefCon );
}

extern const struct load_command * QMOImageFindLoadCommandByID(QMOImageRef qmoImage, uint32_t cmdID)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));
	
	return FindLoadCommand(qmoImage, CommandIDCompareProc, (void *) &cmdID);
}

extern uint32_t	QMOImageGetSegmentCount(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

	return qmoImage->segCount;
}

extern int		QMOImageGetSegmentByName(
    QMOImageRef                 qmoImage, 
    const char *                segName, 
    uint32_t *                  segIndexPtr, 
    struct segment_command_64 * segPtr
)
	// See comment in header.
{
	int			err;
	uint32_t	segIndex;

	assert(QMOImageIsValid(qmoImage));
	assert(segName != NULL);
	assert( (segIndexPtr != NULL) || (segPtr  != NULL) );
	
	err = GetSegmentIndexByName(qmoImage, segName, &segIndex);
	if (err == 0) {
        if (segIndexPtr != NULL) {
            *segIndexPtr = segIndex;
        }
        if (segPtr != NULL) {
            *segPtr = qmoImage->segments[segIndex].seg;
        }
	}
	
	return err;
}

extern int	QMOImageGetSegmentByIndex(QMOImageRef qmoImage, uint32_t segIndex, struct segment_command_64 *segPtr)
	// See comment in header.
{
	int	err;

	assert(QMOImageIsValid(qmoImage));
	assert(segPtr != NULL);
	
	if (segIndex < qmoImage->segCount) {
		*segPtr = qmoImage->segments[segIndex].seg;
		err = 0;
	} else {
		err = EINVAL;
	}
	return err;
}

extern uint32_t	QMOImageGetSectionCount(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

    return qmoImage->sectCount;
}

extern int		QMOImageGetSectionByName(
    QMOImageRef                 qmoImage, 
    const char *                segName,
    const char *                sectName,
    uint32_t *                  sectIndexPtr,
    struct section_64 *         sectPtr
)
	// See comment in header.
{
    int         err;
    bool        found;
    uint32_t    sectIndex;
    
	assert(QMOImageIsValid(qmoImage));
    assert( (segName != NULL) || (sectName != NULL) );
    assert( (sectIndexPtr != NULL) || (sectPtr != NULL) );
    
    // Start by looking for the section.
    
    sectIndex = 0;
    found = false;
    while ( ! found && (sectIndex < qmoImage->sectCount) ) {
        found = ( (segName  == NULL) || (strcmp(segName,  qmoImage->sections[sectIndex].sect.segname) == 0) )
             && ( (sectName == NULL) || (strcmp(sectName, qmoImage->sections[sectIndex].sect.sectname) == 0) );
        if ( ! found ) {
            sectIndex += 1;
        }
    }
    
    // If we find it, copy out the details to our client.
    
    if (found) {
        if (sectIndexPtr != NULL) {
            *sectIndexPtr = sectIndex;
        }
        if (sectPtr != NULL) {
            *sectPtr = qmoImage->sections[sectIndex].sect;
        }
        err = 0;
    } else {
        err = ESRCH;
    }
    
    return err;
}

extern int		QMOImageGetSectionByIndex(
    QMOImageRef                 qmoImage, 
    uint32_t                    sectIndex, 
    struct section_64 *         sectPtr
)
	// See comment in header.
{
    int     err;
    
	assert(QMOImageIsValid(qmoImage));

	if (sectIndex < qmoImage->sectCount) {
		*sectPtr = qmoImage->sections[sectIndex].sect;
		err = 0;
	} else {
		err = EINVAL;
	}
	return err;
}

#pragma mark ***** Utilities

extern uint8_t QMOImageToLocalUInt8(QMOImageRef qmoImage, uint8_t value)
	// See comment in header.
{
	assert(QMOImageIsValidLimited(qmoImage));

	return value;
}

extern uint16_t QMOImageToLocalUInt16(QMOImageRef qmoImage, uint16_t value)
	// See comment in header.
{
	assert(QMOImageIsValidLimited(qmoImage));

	if ( qmoImage->byteSwapped ) {
		value = OSSwapInt16(value);
	}
	return value;
}

extern uint32_t QMOImageToLocalUInt32(QMOImageRef qmoImage, uint32_t value)
	// See comment in header.
{
	assert(QMOImageIsValidLimited(qmoImage));

	if ( qmoImage->byteSwapped ) {
		value = OSSwapInt32(value);
	}
	return value;
}

extern uint64_t QMOImageToLocalUInt64(QMOImageRef qmoImage, uint64_t value)
	// See comment in header.
{
	assert(QMOImageIsValidLimited(qmoImage));

	if ( qmoImage->byteSwapped ) {
		value = OSSwapInt64(value);
	}
	return value;
}

#pragma mark ***** QMOFileImage

static int  QMOFileImageMapRange(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void **   basePtr, 
    void **         mapRefConPtr
)
    // The map range callback for file-based images.  This does its magic 
    // via mmap.
    //
    // See the comments for QMOMapRangeProc for a discussion of the parameters.
{
	int		err;
	int		fd;
	void *	base;

	assert(qmoImage != NULL);
	assert(length > 0);
	assert( basePtr != NULL);
	assert(*basePtr == NULL);
	assert(mapRefConPtr != NULL);

	fd = (int) (intptr_t) qmoImage->refCon;
	
	err = 0;
	base = mmap(NULL, length, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, offset);
	if (base == MAP_FAILED) {
		base = NULL;
		err = errno;
	}
	*basePtr = base;

	assert( (err == 0) == (*basePtr != NULL) );
	
	return err;
}

static void QMOFileImageUnmapRange(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void *    base, 
    void *          mapRefCon
)
    // The unmap range callback for file-based images.
    //
    // See the comments for QMOUnmapRangeProc for a discussion of the parameters.
{
    #pragma unused(offset, mapRefCon)
	int		junk;
	
	assert(qmoImage != NULL);
	assert(length > 0);
	assert(base != NULL);

	junk = munmap( (void *) base, length);
	assert(junk == 0);
}

static int QMOFileImageCreate(QMOImage *qmoImage, void *refCon)
    // The create callback for file-based images.  A file descriptor 
    // of the file to work on is in refCon.  We dup this into the refCon 
    // so that, when the destroy callback is called, it can close safely 
    // close the file in the refCon.
    //
    // See the comments for QMOCreateProc for a discussion of the parameters.
{
    int     err;
    int     fd;
    
	assert(qmoImage != NULL);

	fd = (int) (intptr_t) refCon;
    
    fd = dup(fd);
    if (fd < 0) {
        err = errno;
    } else {
        err = 0;
    }
    qmoImage->refCon = (void *) (intptr_t) fd;

    return err;
}

static void QMOFileImageDestroy(QMOImage *qmoImage)
    // The destroy callback for file-based images.
    //
    // See the comments for QMODestroyProc for a discussion of the parameters.
{
	int	fd;
	int	junk;
	
	assert(qmoImage != NULL);
	
	fd = (int) (intptr_t) qmoImage->refCon;
	
    if (fd != -1) {
        junk = close(fd);
        assert(junk == 0);
    }
}

static const NXArchInfo *NXGetLocalArchInfoFixed(void)
    // A version of NXGetLocalArchInfo that works around the gotchas described 
    // below.
{
    static const NXArchInfo * sCachedArch = NULL;
    
    // NXGetLocalArchInfo (and related routines like NXGetArchInfoFromCpuType) may 
    // or may not return a pointer that you have to free <rdar://problem/5000965>.  
    // To limit the potential for leaks, I only ever call through to the underlying 
    // routine once.  [This can still leak (if two threads initialise it at the 
    // same time, or if we need to apply the x86-64 workaround), but the leak 
    // is bounded.]
    
    if (sCachedArch == NULL) {
        sCachedArch = NXGetLocalArchInfo();

        // For some reason, when running 64-bit on 64-bit architectures, 
        // NXGetLocalArchInfo returns the 32-bit architecture <rdar://problem/4996965>.  
        // If that happens, we change it to what we expect.

        if (sCachedArch != NULL) {
            #if TARGET_CPU_X86_64
                if (sCachedArch->cputype == CPU_TYPE_X86) {
                    sCachedArch = NXGetArchInfoFromCpuType(CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL);
                }
            #elif TARGET_CPU_PPC64
                if (sCachedArch->cputype == CPU_TYPE_POWERPC) {
                    sCachedArch = NXGetArchInfoFromCpuType(CPU_TYPE_POWERPC64, CPU_SUBTYPE_POWERPC_ALL);
                }
            #endif
        }
    }

    return sCachedArch;
}

extern cpu_type_t QMOGetLocalCPUType(void)
    // See comment in header.
{
    return NXGetLocalArchInfoFixed()->cputype;
}

static int FindBestFatArchitecture(
    int                         fd, 
    const struct fat_header *   fatHeader, 
    cpu_type_t                  cputype, 
    cpu_subtype_t               cpusubtype, 
    off_t *                     machHeaderOffsetPtr
)
    // For fat Mach-O file that starts with fatHeader, looking for the 
    // architecture that best matches cputype and cpusubtype.  Return 
    // the file offset of that image in *machHeaderOffsetPtr.
    //
    // Keep in mind that a fat header is always big endian.
{
	int						err;
	uint32_t				archCount;
	struct fat_arch *		arches;
	const struct fat_arch *	bestArch = NULL;

	assert( (cputype != CPU_TYPE_ANY) || (cpusubtype == 0) );
	
	archCount = OSSwapBigToHostInt32(fatHeader->nfat_arch);
	
	// Allocate a fat_arch array and fill it in by reading the file.

	err = 0;
	arches = malloc(archCount * sizeof(*arches));
	if (arches == NULL) {
		err = ENOMEM;
	}
	if (err == 0) {
		ssize_t		bytesRead;
		
		bytesRead = read(fd, arches, archCount * sizeof(*arches));
		if (bytesRead < 0) {
			err = errno;
		} else if (bytesRead != (archCount * sizeof(*arches))) {
			err = EPIPE;
		}
	}

	// Find the fat_arch that best matches the user's requirements.
	
	if (err == 0) {
		// This would do nothing on a big endian system, but it probably would take a 
		// whole bunch of code to do nothing.  So we conditionalise it away.

		#if TARGET_RT_LITTLE_ENDIAN
			{
				uint32_t	archIndex;
				
				for (archIndex = 0; archIndex < archCount; archIndex++) {
					arches[archIndex].cputype    = OSSwapBigToHostInt32(arches[archIndex].cputype);
					arches[archIndex].cpusubtype = OSSwapBigToHostInt32(arches[archIndex].cpusubtype);
					arches[archIndex].offset     = OSSwapBigToHostInt32(arches[archIndex].offset);
					arches[archIndex].size       = OSSwapBigToHostInt32(arches[archIndex].size);
					arches[archIndex].align      = OSSwapBigToHostInt32(arches[archIndex].align);
				}
			}
		#endif

		// If the user requested any CPU type, first try to give them the current 
		// architecture, and if that fails just give them the first.
		
		if (cputype == CPU_TYPE_ANY) {
			const NXArchInfo *	localArch;

            assert(cpusubtype == 0);

			localArch = NXGetLocalArchInfoFixed();

			bestArch = NULL;
			if (localArch != NULL) {
				bestArch = NXFindBestFatArch(localArch->cputype, localArch->cpusubtype, arches, archCount);
                if (bestArch == NULL) {
                    bestArch = NXFindBestFatArch(localArch->cputype, 0, arches, archCount);
                }
			}
			if (bestArch == NULL) {
				bestArch = NXFindBestFatArch(arches[0].cputype, cpusubtype, arches, archCount);
			}
		} else {
			bestArch = NXFindBestFatArch(cputype, cpusubtype, arches, archCount);
		}
		if (bestArch == NULL) {
			err = ESRCH;
		}
	}
	
	// Return that architecture's offset.
	
	if (err == 0) {
		*machHeaderOffsetPtr = bestArch->offset;
	}
	
	free(arches);
	
	return err;
}

extern int QMOImageCreateFromFile(const char *filePath, cpu_type_t cputype, cpu_subtype_t cpusubtype, QMOImageRef *qmoImagePtr)
	// See comment in header.
{
	int					err;
	int					junk;
	int					fd;
	ssize_t				bytesRead;
	struct fat_header	fatHeader;
	off_t				machHeaderOffset;
	struct mach_header  machHeader;
	struct fat_arch		arch;
	const struct fat_arch *	bestArch;

    assert(filePath != NULL);
	assert( (cputype != CPU_TYPE_ANY) || (cpusubtype == 0) );
    assert( qmoImagePtr != NULL);
    assert(*qmoImagePtr == NULL);
	
    machHeaderOffset = 0;       // quieten a warning
    
	// Open the file.
	
	err = 0;
	fd = open(filePath, O_RDONLY);
	if (fd < 0) {
		err = errno;
	}
	
	// Read a potential fat header.  Keep in mind that this is always big endian.
	
	if (err == 0) {
		bytesRead = read(fd, &fatHeader, sizeof(fatHeader));
		if (bytesRead < 0) {
			err = errno;
		} else if (bytesRead != sizeof(fatHeader)) {
			err = EPIPE;
		}
	}
	
	// If it's there, deal with it.  If not, we treat this as a thin file, that 
    // is, the Mach-O image starts at the front of the file.
	
	if (err == 0) {
		if ( OSSwapBigToHostInt32(fatHeader.magic) == FAT_MAGIC) {
			err = FindBestFatArchitecture(fd, &fatHeader, cputype, cpusubtype, &machHeaderOffset);
		} else {
			machHeaderOffset = 0;
		}
	}
	
	// Read the Mach-O header from the requested offset, and do a quick check to make sure 
	// that its magic is correct and that the architecture is compatible with the one the 
	// user requested.  We do this by constructing a dummy fat_arch and passing it as 
    // a single element array to NXFindBestFatArch.  If NXFindBestFatArch returns NULL, 
    // this single architecture isn't compatible with the architecture that the user 
    // requested.
	
	if (err == 0) {
		bytesRead = pread(fd, &machHeader, sizeof(machHeader), machHeaderOffset);
		if (bytesRead < 0) {
			err = errno;
		} else if (bytesRead != sizeof(machHeader)) {
			err = EPIPE;
		}
	}
	if (err == 0) {
		arch.cputype    = machHeader.cputype;
		arch.cpusubtype = machHeader.cpusubtype;
		arch.offset     = 0;						// NXFindBestFatArch ignores these fields
		arch.size       = 0;
		arch.align      = 0;

		if ( (machHeader.magic == MH_MAGIC) || (machHeader.magic == MH_MAGIC_64) ) {
			// do nothing
		} else if ( (machHeader.magic == MH_CIGAM) || (machHeader.magic == MH_CIGAM_64) ) {
			arch.cputype    = OSSwapInt32(arch.cputype);
			arch.cpusubtype = OSSwapInt32(arch.cpusubtype);
		} else {
			err = EINVAL;
		}
	}
	if (err == 0) {
		if (cputype == CPU_TYPE_ANY) {
			cputype = arch.cputype;
		}
		bestArch = NXFindBestFatArch(cputype, cpusubtype, &arch, 1);
		if (bestArch == NULL) {
			err = ESRCH;
		}
	}

	// Create an QMOImage for this file.
	
	if (err == 0) {
		err = QMOImageCreate(machHeaderOffset, filePath, false, QMOFileImageMapRange, QMOFileImageUnmapRange, QMOFileImageCreate, QMOFileImageDestroy, (void *) (intptr_t) fd, qmoImagePtr);
	}
	
	// Clean up.  We can safe close fd because QMOFileImageCreate has dup'd it.
	
	if ( (err != 0) && (fd != -1) ) {
		junk = close(fd);
		assert(junk == 0);
	}
	assert( (err == 0) == QMOImageIsValid(*qmoImagePtr) );

	return err;
}

#pragma mark ***** QMOTaskImage

static int  QMOTaskImageMapRange(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void **   basePtr, 
    void **         mapRefConPtr
)
    // The map range callback for remote images.  This is much easier now that 
    // the QTaskMemory takes care of all the Mach rubbish.
    //
    // See the comments for QMOMapRangeProc for a discussion of the parameters.
{
	int					err;
	task_t				task;
	const void *        base;

	assert(qmoImage != NULL);
	assert(length > 0);
	assert( basePtr != NULL);
	assert(*basePtr == NULL);
	assert(mapRefConPtr != NULL);
	
	task = (task_t) (uintptr_t) qmoImage->refCon;

    base = NULL;
    
    // First try to remap the memory into our address space.
    
    err = QTMRemap(task, offset, length, &base);

	if (err == EFAULT) {

        // If the mapping fails, just read the memory.  This happens if the address 
        // is within the system shared region.
        
        assert(base == NULL);
        err = QTMReadAllocated(task, offset, length, &base);
	}
	if (err == 0) {
		*basePtr   = base;
		*mapRefConPtr = NULL;
	}
	
	assert( (err == 0) == (*basePtr != NULL) );
	
	return err;
}

static void QMOTaskImageUnmapRange(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void *    base, 
    void *          mapRefCon
)
    // The unmap range callback for remote images.
    //
    // See the comments for QMOUnmapRangeProc for a discussion of the parameters.
{
    #pragma unused(offset, mapRefCon)
	assert(qmoImage != NULL);
	assert(length > 0);
	assert(base != NULL);
	
    QTMFree(base, length);
}

bool kQMachOImageTestSelfShortCircuit = true;
	// By changing this variable to false you can disable the short circuit that 
    // we apply when targetting the local task.
    //
    // This is exported for the benefit of the unit test.

extern int QMOImageCreateFromTask(
    task_t          task, 
    QTMAddr         machHeader, 
    const char *    filePath,
    QMOImageRef *   qmoImagePtr
)
	// See comment in header.
{
    int     err;

    assert(task != MACH_PORT_NULL);
    // machHeader could potentially be at zero
    // filePath may be NULL
    assert( qmoImagePtr != NULL);
    assert(*qmoImagePtr == NULL);
    
    if ( (task == mach_task_self()) && kQMachOImageTestSelfShortCircuit ) {
        err = QMOImageCreateFromLocalImage( (const struct mach_header *) (uintptr_t) machHeader, filePath, qmoImagePtr);
    } else {
        err = QMOImageCreate(machHeader, filePath, true, QMOTaskImageMapRange, QMOTaskImageUnmapRange, NULL, NULL, (void *) (uintptr_t) task, qmoImagePtr);
    }

	assert( (err == 0) == QMOImageIsValid(*qmoImagePtr) );

    return err;
}

static int FindTaskDyldWithNonNativeRetry(task_t task, cpu_type_t cputype, QTMAddr *dyldAddrPtr);
    // forward declaration

extern int QMOImageCreateFromTaskDyld(
    task_t          task, 
    cpu_type_t      cputype, 
    QMOImageRef *   qmoImagePtr
)
	// See comment in header.
{
    int         err;
    QMOImageRef qmoImage;
    QTMAddr     dyldAddr;
    
    assert(task != MACH_PORT_NULL);
    assert( qmoImagePtr != NULL);
    assert(*qmoImagePtr == NULL);

    qmoImage = NULL;
    
    // Find dyld within the task and then create a remote image based on that. 
    // Seems easy huh?  No way.
    
    err = FindTaskDyldWithNonNativeRetry(task, cputype, &dyldAddr);
    if (err == 0) {
        err = QMOImageCreateFromTask(task, dyldAddr, NULL, &qmoImage);
    }
    
    // In the case of dyld, we assume that the dynamic linker ID is the file path. 
    // It's hard to get the dynamic linker ID before we create the image, so we create 
    // the image with a NULL filePath and then fill it in afterwards.  If this 
    // fails, I leave the file path set to NULL; I don't consider this failure 
    // bad enough to warrant failing the entire routine.
    
    if (err == 0) {
        const struct dylinker_command * dyldCommand;
        const char *                    filePath;
        
        assert(qmoImage->filePath == NULL);

        filePath = NULL;

        dyldCommand = (const struct dylinker_command *) QMOImageFindLoadCommandByID(
            qmoImage,
            LC_ID_DYLINKER
        );
        if (dyldCommand != NULL) {
            filePath = ((const char *) dyldCommand) + QMOImageToLocalUInt32(qmoImage, dyldCommand->name.offset);
        }
        if (filePath != NULL) {
            qmoImage->filePath = strdup(filePath);
        }
        
        assert(qmoImage->filePath != NULL);
        
        // Copy result out to client.
        
        *qmoImagePtr = qmoImage;
    }
    
	assert( (err == 0) == QMOImageIsValid(*qmoImagePtr) );
    
    return err;
}

#pragma mark ***** QMOLocalImage

static int  QMOLocalImageMapRange(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void **   basePtr, 
    void **         mapRefConPtr
)
    // The map range callback for local images.  This one is easy (-:
    //
    // See the comments for QMOMapRangeProc for a discussion of the parameters.
{
	int		err;
	
	assert(qmoImage != NULL);
	assert(length > 0);
	assert( basePtr != NULL);
	assert(*basePtr == NULL);
	assert(mapRefConPtr != NULL);

	*basePtr   = (const void *) (uintptr_t) offset;
	*mapRefConPtr = NULL;
	err = 0;
	
	assert( (err == 0) == (*basePtr != NULL) );
	
	return err;
}

static void QMOLocalImageUnmapRange(
    QMOImage *      qmoImage, 
    QTMAddr         offset, 
    QTMAddr         length, 
    const void *    base, 
    void *          mapRefCon
)
    // The unmap range callback for local images.
    //
    // See the comments for QMOUnmapRangeProc for a discussion of the parameters.
{
    #pragma unused(offset, mapRefCon)
	assert(qmoImage != NULL);
	assert(length > 0);
	assert(base != NULL);

	// do nothing
}

extern int QMOImageCreateFromLocalImage(
    const struct mach_header *  machHeader, 
    const char *                filePath,
    QMOImageRef *               qmoImagePtr
)
	// See comment in header.
{
    // machHeader could potentially be at zero
    // filePath may be NULL
    assert( qmoImagePtr != NULL);
    assert(*qmoImagePtr == NULL);

	return QMOImageCreate( (QTMAddr) (uintptr_t) machHeader, filePath, true, QMOLocalImageMapRange, QMOLocalImageUnmapRange, NULL, NULL, NULL, qmoImagePtr);
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Finding dyld

// This stuff is fairly well commented in the routines comments.  Start with 
// the comments for FindTaskDyldWithNonNativeRetry.

// In the debug version, we support an environment variable that logs progress 
// for our dyld search.  This is nice because is can be hard to debug otherwise 
// (for example, when you're running PowerPC program using Rosetta).

#if defined(NDEBUG)

    static inline bool LogDyldSearch(void)
    {
        return false;
    }

#else

    static bool LogDyldSearch(void)
    {
        static bool sInited = false;
        static bool sLogDyldSearch = false;
        
        if ( ! sInited ) {
            sLogDyldSearch = (getenv("QMACHOIMAGE_LOG_DYLD_SEARCH") != NULL);
        
            sInited = true;
        }
        
        return sLogDyldSearch;
    }

#endif

static const char * GetLocalDyldPath(void)
	// Get the path for the dyld associated with the current task. 
	// We do this by iterating through the list of images in the 
	// current task looking for the main executable's image (whose 
	// file type is MH_EXECUTE).  In that we look for the 
	// LC_LOAD_DYLINKER load command, which contains the path to the 
	// dynamic linker requested by this image.
{
	int							err;
	uint32_t					imageCount;
	uint32_t					imageIndex;
	const struct mach_header *	thisImage;
    static const char *         sCachedResult = NULL;
    
    if (sCachedResult == NULL) {
        // Iterate the images in the current process looking for the main 
        // executable image (MH_EXECUTE).
        
        imageCount = _dyld_image_count();
        for (imageIndex = 0; imageIndex < imageCount; imageIndex++) {

            compile_time_assert( offsetof(struct mach_header_64, filetype) == offsetof(struct mach_header, filetype) );

            thisImage = _dyld_get_image_header(imageIndex);
            if (thisImage->filetype == MH_EXECUTE) {										// no need to byte swap because this is local only
                QMOImageRef						qmoImage;
                const struct dylinker_command *	loadDyldCmd;

                if ( LogDyldSearch() ) {
                    fprintf(stderr, "GetLocalDyldPath: Found executable at %p.\n", thisImage);
                }

                // Within the main executable image, look for the LC_LOAD_DYLINKER 
                // load command.  Extract the dyld's path from that.
                
                qmoImage = NULL;
                
                err = QMOImageCreateFromLocalImage(
                    thisImage, 
                    NULL,
                    &qmoImage
                );
                if (err == 0) {
                    loadDyldCmd = (const struct dylinker_command *) QMOImageFindLoadCommandByID(
                        qmoImage, 
                        LC_LOAD_DYLINKER
                    );
                    if (loadDyldCmd != NULL) {
                        sCachedResult = (((const char *) loadDyldCmd) + loadDyldCmd->name.offset);	// no need to byte swap because this is local only
                        if ( LogDyldSearch() ) {
                            fprintf(stderr, "GetLocalDyldPath: Local dyld is '%s'.\n", sCachedResult);
                        }
                    }
                }
                QMOImageDestroy(qmoImage);
                
                break;
            }
        }
    }
	
	return sCachedResult;
}

static QTMAddr GetLocalDyldAddr(void)
	// Get the address that the current task /intended/ to load 
	// dyld.  That is, get the default load address of the dyld 
	// requested by the current task.  So, if the dyld was slid, 
	// this returns that address that it wanted to load at.
	//
	// If anything goes wrong, return 0.  The caller can either 
	// ignore the error, or specifically check for 0, which is a 
	// very improbable load address for dyld.
{
	int							err;
	const char *				dyldPath;
	QMOImageRef					qmoImage;
	struct segment_command_64	textSeg;
	static QTMAddr              sCachedResult = 0;
    
    if (sCachedResult == 0) {
        qmoImage = NULL;

        // Get the path to the current task's dyld, using a hard-wired 
        // default if any goes wrong.
        
        dyldPath = GetLocalDyldPath();
        if (dyldPath == NULL) {
            dyldPath = "/usr/lib/dyld";
        }	
        
        // Create a file-based QMOImage from that, and then look up the __TEXT 
        // segment's vmaddr.  Note that, if dyld is fat, QMOImageCreateFromFile 
        // will use the architecture that matches the local architecture (if 
        // possible).
        
        err = QMOImageCreateFromFile(dyldPath, CPU_TYPE_ANY, 0, &qmoImage);
        if (err == 0) {
            err = QMOImageGetSegmentByName(qmoImage, "__TEXT", NULL, &textSeg);
        }
        if (err == 0) {
            sCachedResult = textSeg.vmaddr;
            if ( LogDyldSearch() ) {
                fprintf(stderr, "GetLocalDyldAddr: Local dyld is at %p; based on fat file member with CPU type/subtype of %#x/%#x.\n", (void *) (uintptr_t) sCachedResult, (int) QMOImageGetCPUType(qmoImage), (int) QMOImageGetCPUSubType(qmoImage));
            }
        }
        
        // Clean up.
        
        QMOImageDestroy(qmoImage);
    }

	return sCachedResult;
}

static bool IsDyldAtAddress(task_t task, cpu_type_t cputype, QTMAddr addr, vm_prot_t prot)
	// Returns true if the address within the specified task looks 
	// like it points to dyld.  We first check that prot is what we'd 
    // expect for dydl.  Then we read a mach_header from the 
	// address and check its magic, filetype, and cputype fields.
{
	int						err;
	bool					result;
	struct mach_header		machHeader;

	assert(task != MACH_PORT_NULL);
		
	result = false;

    compile_time_assert( offsetof(struct mach_header_64, magic)    == offsetof(struct mach_header, magic) );
	compile_time_assert( offsetof(struct mach_header_64, filetype) == offsetof(struct mach_header, filetype) );
	
    if ( (prot & (VM_PROT_READ | VM_PROT_EXECUTE)) == (VM_PROT_READ | VM_PROT_EXECUTE) ) {
        err = QTMRead(task, addr, sizeof(machHeader), &machHeader);
        if (err == 0) {
            if ( (machHeader.magic == MH_MAGIC) || (machHeader.magic == MH_MAGIC_64) ) {
                result = true;
            } else if ( (machHeader.magic == MH_CIGAM) || (machHeader.magic == MH_CIGAM_64) ) {
                machHeader.filetype = OSSwapInt32(machHeader.filetype);
                machHeader.cputype  = OSSwapInt32(machHeader.cputype);
                result = true;
            }
            if (result) {
                result = ( machHeader.filetype == MH_DYLINKER );
                if (result) {
                    result = ((cputype == CPU_TYPE_ANY) || (cputype == machHeader.cputype));
                    if (result) {
                        if ( LogDyldSearch() ) {
                            fprintf(stderr, "IsDyldAtAddress: Found dyld at %#llx; CPU type is %#x.\n", addr, (int) machHeader.cputype);
                        }
                    } else {
                        if ( LogDyldSearch() ) {
                            fprintf(stderr, "IsDyldAtAddress: Mach header at %#llx is dyld but wrong CPU type (wanted %#x, got %#x).\n", addr, (int) cputype, (int) machHeader.cputype);
                        }
                    }
                } else {
                    if ( LogDyldSearch() ) {
                        fprintf(stderr, "IsDyldAtAddress: Mach header at %#llx but not dyld.\n", addr);
                    }
                }
            } else {
                if ( LogDyldSearch() ) {
                    fprintf(stderr, "IsDyldAtAddress: No Mach header at %#llx.\n", addr);
                }
            }
        } else {
            if ( LogDyldSearch() ) {
                fprintf(stderr, "IsDyldAtAddress: Error %#x reading Mach header at %#llx.\n", err, addr);
            }
        }
    } else {
        if ( LogDyldSearch() ) {
            fprintf(stderr, "IsDyldAtAddress: Skipped region at %#llx because of protection (%d).\n", addr, (int) prot);
        }
    }
	
	return result;
}

static int FindTaskDyld(task_t task, cpu_type_t cputype, QTMAddr *dyldAddrPtr)
	// Finds the address of dyld within a given task.  Unfortunately, there is 
	// just no good way to do this because a) the task might have a different 
	// architecture, which might load dyld at a different address, and 
	// b) dyld can slide.  So, we go hunting for dyld the hard way.  We start 
	// by assuming that dyld loaded in the same place as it loaded in our 
	// task, which is a very strong heuristic (assuming that the target is the 
    // same architecture as us, which is also quite likely).  If that doesn't pan 
    // out, we iterate through every memory region in the task look for one that 
	// starts with something that looks like dyld.
	//
	// Yetch!
	//
	// This is pretty much the same thing that's done by GDB and vmutils.
{
	int                     err;
        kern_return_t           kr;
	QTMAddr                 localDyldAddr;
#if TARGET_CPU_X86 || TARGET_CPU_X86_64
	mach_vm_address_t	thisRegion;
#else
	vm_address_t		thisRegion;
#endif	
	assert(task != MACH_PORT_NULL);
	assert(dyldAddrPtr != NULL);
	
    // Find our dyld's address and see if the task has dyld at the same address.
    
	localDyldAddr = GetLocalDyldAddr();
	if ( IsDyldAtAddress(task, cputype, localDyldAddr, VM_PROT_READ | VM_PROT_EXECUTE) ) {
        if ( LogDyldSearch() ) {
            fprintf(stderr, "FindTaskDyld: Got dyld at local address (%#llx).\n", localDyldAddr);
        }
		*dyldAddrPtr = localDyldAddr;
		err = 0;
	} else {
		bool						found;
		mach_port_t					junkObjName;
		
        if ( LogDyldSearch() ) {
            fprintf(stderr, "FindTaskDyld: Searching for dyld the hard way.\n");
        }

        // Well, that didn't work.  Let's look the hard way.
        
		found = false;
		thisRegion = 0;
		do {
            // Because we don't actually look at the pointer size fields of the 
            // resulting structure, we should just be able to use VM_REGION_BASIC_INFO. 
            // However, it seems that VM_REGION_BASIC_INFO is not compatible with 
            // the 64-bit call variant (that is, mach_vm_region as opposed to 
            // vm_region).  I really haven't investigated this properly because the 
            // workaround is pretty easy: use VM_REGION_BASIC_INFO_64 for 
            // everything.  There may be compatibility consequences for this 
            // (for example, I suspect that VM_REGION_BASIC_INFO_COUNT_64 is not 
            // supported on 10.3).  I'll deal with these when I come to them.

            // For BOINC changes, See Mach Compatibility comments in QTaskMemory.c

                mach_msg_type_number_t                  infoCount;

#if TARGET_CPU_X86 || TARGET_CPU_X86_64

                mach_vm_size_t				thisRegionSize;
                vm_region_basic_info_data_64_t          info;

                infoCount = VM_REGION_BASIC_INFO_COUNT_64;
                kr = mach_vm_region(
                    task,
                    &thisRegion,
                    &thisRegionSize,
                    VM_REGION_BASIC_INFO_64,
                    (vm_region_info_t) &info,
                    &infoCount,
                    &junkObjName
                );

#else

                vm_size_t                               thisRegionSize;
                vm_region_basic_info_data_t             info;

                infoCount = VM_REGION_BASIC_INFO_COUNT;
                kr = vm_region(
                    task,
                    &thisRegion,
                    &thisRegionSize,
                    VM_REGION_BASIC_INFO,
                    (vm_region_info_t) &info,
                    &infoCount,
                    &junkObjName
                );
#endif
            err = QTMErrnoFromMachError(kr);

			if (err == 0) {
				assert(infoCount == infoCount);

				// We've found dyld if the memory region is read/no-write/execute 
				// and it starts with a mach_header that looks like dyld.

                found = IsDyldAtAddress(task, cputype, thisRegion, info.protection);
				if ( found ) {
					*dyldAddrPtr = thisRegion;
				} else {
					thisRegion += thisRegionSize;
				}
			}				
		} while ( (err == 0) && ! found );
	}
	
	return err;
}

static int FindTaskDyldWithNonNativeRetry(task_t task, cpu_type_t cputype, QTMAddr *dyldAddrPtr)
    // A wrapper around FindTaskDyld that handles a nasty edge case.  Namely, 
    // if the client asks for any dyld and the target task is being run using 
    // Rosetta, try to find the PowerPC dyld first and then, if that fails, 
    // look for any dyld.  This gives an explicit priority to the PowerPC dyld 
    // for non-native tasks.  Without this, you run into problems where a client 
    // doesn't know the CPU type of the target task (because they haven't connected 
    // to it yet), tries to connect up, connects up the wrong dyld, and it all 
    // goes south.
{
    int         err;
    
	assert(task != MACH_PORT_NULL);
	assert(dyldAddrPtr != NULL);

    if ( (cputype == CPU_TYPE_ANY) && ! QTMTaskIsNative(task) ) {
        err = FindTaskDyld(task, CPU_TYPE_POWERPC, dyldAddrPtr);
        if (err != 0) {
            if ( LogDyldSearch() ) {
                fprintf(stderr, "FindTaskDyldWithNonNativeRetry: Failed to find PowerPC dyld; retrying for any dyld.\n");
            }
            err = FindTaskDyld(task, cputype, dyldAddrPtr);
        }
    } else {
        err = FindTaskDyld(task, cputype, dyldAddrPtr);
    }
    
    return err;
}

#pragma mark ***** High-Level APIs

extern const char * QMOImageGetLibraryID(QMOImageRef qmoImage)
	// See comment in header.
{
	assert(QMOImageIsValid(qmoImage));

    if ( ! qmoImage->imageIDCached ) {
        const struct dylib_command * imageIDCommand;
        
        imageIDCommand = (const struct dylib_command *) QMOImageFindLoadCommandByID(
            qmoImage,
            LC_ID_DYLIB
        );
        if (imageIDCommand != NULL) {
            qmoImage->imageID = ((const char *) imageIDCommand) + QMOImageToLocalUInt32(qmoImage, imageIDCommand->dylib.name.offset);
        }
        
        qmoImage->imageIDCached = true;
    }
    
    return qmoImage->imageID;
}

extern int QMOImageIterateSymbols(QMOImageRef qmoImage, QMOSymbolIteratorProc callback, void *iteratorRefCon)
	// See comment in header.
{
	int								err;
	const struct symtab_command *	symTabCmd;
	uint32_t						linkEditSegIndex;
	const char *					linkEditSeg;
	uint32_t						symCount;
	uint32_t						symIndex;
	QTMAddr							linkEditFileOffset;
	const char *					stringBase;
	const struct nlist *			symBase;
	const struct nlist_64 *			symBase64;
	uint32_t						nameStringOffset;
	const char *					name;
	bool							stop;

	assert(QMOImageIsValid(qmoImage));
    assert(callback != NULL);
	
    // Do some preparation.  Get the LC_SYMTAB command and map the __LINKEDIT segment.
    
	err = 0;
	symTabCmd = (const struct symtab_command *) QMOImageFindLoadCommandByID(qmoImage, LC_SYMTAB);
	if (symTabCmd == NULL) {
		err = EINVAL;
	}
	if (err == 0) {
		err = GetSegmentIndexByName(qmoImage, "__LINKEDIT", &linkEditSegIndex);
	}
	if (err == 0) {
		err = QMOImageMapSegmentByIndex(qmoImage, linkEditSegIndex, &linkEditSeg);
	}
	
    // Rummage through the these two things to find the symbol list.
    
	if (err == 0) {
		symCount = QMOImageToLocalUInt32(qmoImage, symTabCmd->nsyms);
		
        // The seg fields of the segments array have already been swapped, so we 
        // don't need to swap them here.

        linkEditFileOffset = qmoImage->segments[linkEditSegIndex].seg.fileoff;

		stringBase = linkEditSeg + QMOImageToLocalUInt32(qmoImage, symTabCmd->stroff) - linkEditFileOffset;
		symBase    = (const struct nlist *) (linkEditSeg + QMOImageToLocalUInt32(qmoImage, symTabCmd->symoff) - linkEditFileOffset);

		stop = false;
		
		if ( qmoImage->is64Bit ) {
			symBase64 = (const struct nlist_64 *) symBase;
			
			for (symIndex = 0; symIndex < symCount; symIndex++) {
				nameStringOffset = QMOImageToLocalUInt32(qmoImage, symBase64[symIndex].n_un.n_strx);
				if (nameStringOffset == 0) {
					name = "";
				} else {
					name = stringBase + nameStringOffset;
				}

				err = callback(
					qmoImage,
					name,
					QMOImageToLocalUInt8( qmoImage, symBase64[symIndex].n_type),
					QMOImageToLocalUInt8( qmoImage, symBase64[symIndex].n_sect),
					QMOImageToLocalUInt16(qmoImage, symBase64[symIndex].n_desc),
					QMOImageToLocalUInt64(qmoImage, symBase64[symIndex].n_value),
					iteratorRefCon,
					&stop
				);
				
				if ( (err != 0) || stop ) {
					break;
				}
			}
		} else {
			for (symIndex = 0; symIndex < symCount; symIndex++) {
				nameStringOffset = QMOImageToLocalUInt32(qmoImage, symBase[symIndex].n_un.n_strx);
				if (nameStringOffset == 0) {
					name = "";
				} else {
					name = stringBase + nameStringOffset;
				}

				err = callback(
					qmoImage,
					name,
					QMOImageToLocalUInt8( qmoImage, symBase[symIndex].n_type),
					QMOImageToLocalUInt8( qmoImage, symBase[symIndex].n_sect),
					QMOImageToLocalUInt16(qmoImage, symBase[symIndex].n_desc),
					QMOImageToLocalUInt32(qmoImage, symBase[symIndex].n_value),
					iteratorRefCon,
					&stop
				);

				if ( (err != 0) || stop ) {
					break;
				}
			}
		}
	}
	
	return err;
}

// SymbolByNameIteratorContext is pointed to be the iteratorRefCon in SymbolByNameIterator.

struct SymbolByNameIteratorContext {
	const char *	symName;
	QTMAddr			symValue;
	bool			found;
};
typedef struct SymbolByNameIteratorContext SymbolByNameIteratorContext;

static int SymbolByNameIterator(
	QMOImageRef		qmoImage, 
	const char *	name,
	uint8_t			type,
	uint8_t			sect,
	uint16_t		desc,
	QTMAddr			value,
	void *			iteratorRefCon,
	bool *			stopPtr
)
    // The QMOImageIterateSymbols callback for QMOImageLookupSymbol.
{
    #pragma unused(sect, desc)
	int								err;
	SymbolByNameIteratorContext *	context;

	assert(QMOImageIsValid(qmoImage));
    assert(name != NULL);
    assert( stopPtr != NULL);
    assert(*stopPtr == false);

	err = 0;

    // Check it's not a debugging symbol.
    
	if ( ! (type & N_STAB) ) {
		context = (SymbolByNameIteratorContext *) iteratorRefCon;
		
        // See if the name matches.
        
		if ( strcmp(name, context->symName) == 0 ) {
        
            // Handle the symbol differently depending on its type.
            
			switch (type & N_TYPE) {
				case N_ABS:
					context->symValue = value;
					context->found = true;
					*stopPtr = true;
					break;
				case N_SECT:
					context->symValue = value + qmoImage->slide;
					context->found = true;
					*stopPtr = true;
					break;
				case N_INDR:
					// *** should handle this, but it's hard
					break;
			}
		}
	}
	return err;
}

extern int QMOImageLookupSymbol(QMOImageRef qmoImage, const char *symName, QTMAddr *valuePtr)
	// See comment in header.
{
	int							err;
	SymbolByNameIteratorContext context;
	
	assert(QMOImageIsValid(qmoImage));
    assert(symName != NULL);
    assert(valuePtr != NULL);

	context.symName  = symName;
	context.symValue = 0;
	context.found = false;
	err = QMOImageIterateSymbols(qmoImage, SymbolByNameIterator, &context);
	if (err == 0) {
		if (context.found) {
			*valuePtr = context.symValue;
		} else {
			err = ESRCH;
		}
	}
	return err;
}

