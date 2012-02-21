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
 *  QMachOImageList.c
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
    File:       QMachOImageList.c

    Contains:   Code to get all Mach-O images within a task.

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

$Log: QMachOImageList.c,v $
Revision 1.1  2007/03/02 12:20:22         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "QMachOImageList.h"

// System interfaces

#include <TargetConditionals.h>

#include <assert.h>
#include <sys/param.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <mach-o/dyld.h>
#include <mach-o/loader.h>

// Scary Darwin-derived header file for dyld.

#include "dyld_gdb.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** New Technique

static int ImageListForTaskNew(
	task_t          task, 
	QMOImageRef     dyldImage, 
	QTMAddr         allImageInfoAddr, 
	QMOImageRef *	imageArray, 
	size_t          imageArraySize, 
	size_t *        imageCountPtr
)
	// The new interface is fairly straightforward: there's a single symbol 
	// ("dyld_all_image_infos", which has already been looked up by our caller, 
	// and whose address (in the remote task) is passed to us in allImageInfoAddr) 
    // that contains a simple data structure that describes all of the images.  
    // That data structure is defined by (struct dyld_all_image_infos) (see 
    // "dyld_gdb.h"); it, in turn, contains a pointer to an array of structures 
	// (struct dyld_image_info) for each image.
	//
	// Given the task, a dyld image, and the address of the 
	// "dyld_all_image_infos" structure within that task, this routine returns 
	// an array of images within that task (in the buffer specified by imageArray, 
	// if it's not NULL, and imageArraySize), and a count of those images 
    // (in *imageCountPtr). 
	//
	// On entry, task must not be MACH_PORT_NULL
	// On entry, dyldImage must not be NULL
	// On entry, allImageInfoAddr must be the address of "dyld_all_image_infos" 
	// within the task, which shouldn't be NULL
	// On entry, imageCountPtr must not be NULL
	// On entry, *imageCountPtr is ignored
	// On entry, if imageArray is NULL, imageArraySize must be 0
	// On entry, if imageArray is not NULL, imageArraySize must be the number of 
	// elements available in imageArray
	// On success, if imageArray is not NULL, up to imageArraySize image objects 
	// are returned in the array
	// On success, *imageCountPtr is the number of images in the task; this may 
	// be larger than imageArraySize, in which case the imageArray wasn't big 
	// enough and the returned value tells you how big it needs to be
{
	int							err;
	struct dyld_all_image_infos	allImageInfo;
	uint32_t					infoCount = 0;
	uint32_t					infoIndex;
	QTMAddr						infoArrayAddr;
	size_t						infoArraySize;
	const void *				infoArrayLocal;
	
	assert(task != MACH_PORT_NULL);
	assert(dyldImage != NULL);
	assert(allImageInfoAddr != 0);		// very unlikely for dyld_all_image_infos to be at 0
	assert( (imageArray != NULL) || (imageArraySize == 0) );
	assert(imageCountPtr != NULL);
	
	infoArrayLocal = NULL;
    infoArraySize  = 0;                 // quieten a warning

	// Read the value of the dyld_all_image_infos structure.
	
	err = QTMRead(task, allImageInfoAddr, sizeof(allImageInfo), &allImageInfo);

	// Bletch.  The layout of (struct dyld_all_image_infos) varies with the pointer 
	// size of the architecture.  As we can't guarantee that the remote architecture 
	// is compatible with our local pointer size, we have to be very careful about 
	// the fields that we access within dyld_all_image_infos.  It turns out that we 
	// only care about the "version", "infoArrayCount", and "infoArray" fields, and 
	// only the last one varies by pointer size, so this all works; but it's 
	// hardly nice.
	
	if ( (err == 0) && (QMOImageToLocalUInt32(dyldImage, allImageInfo.version) != 1) ) {
		err = ESHLIBVERS;
	}
	if (err == 0) {
		infoCount = QMOImageToLocalUInt32(dyldImage, allImageInfo.infoArrayCount);
		
		if ( QMOImageIs64Bit(dyldImage) ) {
			infoArrayAddr = QMOImageToLocalUInt64(dyldImage, *(uint64_t *) &allImageInfo.infoArray);
			infoArraySize = infoCount * sizeof(uint64_t) * 3;
		} else {
			infoArrayAddr = QMOImageToLocalUInt32(dyldImage, *(uint32_t *) &allImageInfo.infoArray);
			infoArraySize = infoCount * sizeof(uint32_t) * 3;
		}
		
		// Read in the array of per-image structures.
		
		err = QTMReadAllocated(task, infoArrayAddr, infoArraySize, &infoArrayLocal);
	}
	
	// Process the array of per-image structures, using the relevant information 
	// to create our output.  Again, the size of these fields vary per architecture.
	
	if (err == 0) {
		*imageCountPtr = infoCount;
		
		if (imageArray != NULL) {
            bool    is64Bit;
            
            is64Bit = QMOImageIs64Bit(dyldImage);
            for (infoIndex = 0; infoIndex < infoCount; infoIndex++) {
                if (infoIndex < imageArraySize) {
                    QTMAddr     machHeader;
                    QTMAddr     filePathAddr;
                    char        filePath[PATH_MAX];
                    
                    if (is64Bit) {
                        machHeader   = QMOImageToLocalUInt64(dyldImage, ((uint64_t *) infoArrayLocal)[infoIndex * 3] );
                        filePathAddr = QMOImageToLocalUInt64(dyldImage, ((uint64_t *) infoArrayLocal)[infoIndex * 3 + 1] );
                    } else {
                        machHeader   = QMOImageToLocalUInt32(dyldImage, ((uint32_t *) infoArrayLocal)[infoIndex * 3] );
                        filePathAddr = QMOImageToLocalUInt32(dyldImage, ((uint32_t *) infoArrayLocal)[infoIndex * 3 + 1] );
                    }
                    err = QTMRead(task, filePathAddr, sizeof(filePath), filePath);
                    if (err == 0) {
                        filePath[sizeof(filePath) - 1] = 0;         // guarantee NULL termination
                        err = QMOImageCreateFromTask(task, machHeader, filePath, &imageArray[infoIndex]);
                    }
                    if (err != 0) {
                        break;
                    }
                }
            }
		}
	}

	// Clean up.
	
    QTMFree(infoArrayLocal, infoArraySize);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Old Technique

static int ImageListForTaskOldWithNames(
	task_t			task, 
	QMOImageRef		dyldImage, 
	const char *	listHeadName, 
	const char *	elemCountName, 
	const char *	elemSizeName, 
	QMOImageRef *	imageArray, 
	size_t			imageArraySize, 
	size_t *		imageCountPtr
)
	// The old interface is quite strange.  dyld declares a global variable 
	// (whose name is given by listHeadName, for example, "library_images") 
	// that contains elemCount elements, each of elemSize bytes.  The values 
	// of elemCount and elemSize are exported in global variables, each 
	// of which is a uint32_t.  The names of these variables are also supplied 
	// as parameters to this routine (elemCountName and elemSizeName, for example, 
	// "gdb_nlibrary_images" and "gdb_library_image_size", respectively).
	// Finally, at the end of this block of elements is a count of the number 
	// of elements in the block ("nimages") followed by a pointer to the 
	// next block of elements ("next_images").
	//
	// Within an element, there are only three fields that interest us:
	//
	// o "valid"         (offset 12, size 4) -- Non-zero if this element is valid.
	// o "mh"            (offset  8, size 4) -- A pointer to the mach_header for this 
    //                                          image.
    // o "physical_name" (offset  0, size 4) -- A pointer to the file path for the
    //                                          on disk storage, if any.
	//
	// On entry, task must not be MACH_PORT_NULL
	// On entry, dyldImage must not be NULL
	// On entry, listHeadName, elemCountName, and elemSizeName must all not be NULL
	// On entry, imageCountPtr must not be NULL
	// On entry, *imageCountPtr is NOT ignored; rather, this routine assumes that 
	// imageArray already has *imageCountPtr elements filled in, and starts putting 
	// new elements at *imageCountPtr
	// On entry, if imageArray is NULL, imageArraySize must be 0
	// On entry, if imageArray is not NULL, imageArraySize must be the number of 
	// elements available in imageArray
	// On success, if imageArray is not NULL, up to imageArraySize image objects 
	// are returned in the array, starting at the position specified by the 
	// original value of *imageCountPtr
	// On success, *imageCountPtr is incremented by the number of images that were 
	// found; this may end up making it larger than imageArraySize, in which case the 
	// imageArray wasn't big enough and the returned value tells you how big 
	// it needs to be
{
	int						err;
	QTMAddr					listChunkAddr;
	QTMAddr					elemSizeAddr;
	QTMAddr					elemCountAddr;
	uint32_t				elemCount;
	uint32_t				elemSize;
	size_t					listChunkSize;
	enum {									// constants related to the array elements
		kValidOffset        = 12,				// offset to "valid" field
		kMHOffset           = 8,				// offset to "mh" field
        kPhysicalNameOffset = 0                 // offset to "physical_name" field
	};
	enum {									// constants related to data after array
		kNImagesOffset    = 0,					// offset to "nimages" field
		kNextImagesOffset = 4,					// offset to next_images field
		kBlockSuffixSize  = 8,					// total size of trailing block
	};

	// The old-style interface is not present in 64-bit tasks.
	
	assert( ! QMOImageIs64Bit(dyldImage) );
	
	assert(task != MACH_PORT_NULL);
	assert(dyldImage     != NULL);
	assert(listHeadName  != NULL);
	assert(elemCountName != NULL);
	assert(elemSizeName  != NULL);
	assert( (imageArray  != NULL) || (imageArraySize == 0) );
	assert(imageCountPtr != NULL);

	// Look up each of the symbols.
	
	err = QMOImageLookupSymbol(dyldImage, listHeadName, &listChunkAddr);
	if (err == 0) {
		err = QMOImageLookupSymbol(dyldImage, elemCountName, &elemCountAddr);
	}
	if (err == 0) {
		err = QMOImageLookupSymbol(dyldImage, elemSizeName, &elemSizeAddr);
	}
	
	// Read the elemCount and elemSize values.
	
	if (err == 0) {
		err = QTMRead(task, elemCountAddr, sizeof(elemCount), &elemCount);
	}
	if (err == 0) {
		err = QTMRead(task, elemSizeAddr, sizeof(elemSize), &elemSize);
	}
	
	// Read the image data, one chunk at a time.
	
	if (err == 0) {
		elemCount = QMOImageToLocalUInt32(dyldImage, elemCount);
		elemSize  = QMOImageToLocalUInt32(dyldImage, elemSize);
		listChunkSize = elemCount * elemSize + kBlockSuffixSize;

		do {
			const char *    listChunk;
			uint32_t        elemCountInThisBlock;
			uint32_t        elemIndex;
			
			listChunk = NULL;
			
			// Read a copy of this chunk of the list.
			
			err = QTMReadAllocated(task, listChunkAddr, listChunkSize, (const void **) &listChunk);
			if (err == 0) {
				elemCountInThisBlock = QMOImageToLocalUInt32(dyldImage, *(uint32_t *) (listChunk + elemCount * elemSize + kNImagesOffset) );
				
				// Process each element in the chunk.  If the element is valid, place 
				// the image into the output array (if there's size)
				
				for (elemIndex = 0; elemIndex < elemCountInThisBlock; elemIndex++) {
					if ( QMOImageToLocalUInt32(dyldImage, *(uint32_t *) (listChunk + elemIndex * elemSize + kValidOffset)) != 0 ) {
						if ( (imageArray != NULL) && (*imageCountPtr < imageArraySize) ) {
                            QTMAddr     machHeader;
                            QTMAddr     filePathAddr;
                            char        filePath[PATH_MAX];
                            
							machHeader   = QMOImageToLocalUInt32(dyldImage, *(uint32_t *) (listChunk + elemIndex * elemSize + kMHOffset          ));
                            filePathAddr = QMOImageToLocalUInt32(dyldImage, *(uint32_t *) (listChunk + elemIndex * elemSize + kPhysicalNameOffset));
                            err = QTMRead(task, filePathAddr, sizeof(filePath), filePath);
                            if (err == 0) {
                                filePath[sizeof(filePath) - 1] = 0;         // guarantee NULL termination
                                err = QMOImageCreateFromTask(task, machHeader, filePath, &imageArray[*imageCountPtr]);
                            }
						}
						*imageCountPtr += 1;
                        if (err != 0) {
                            break;
                        }
					}
				}
			}
			
			// Move on to the next chunk.
			
			if (err == 0) {
				listChunkAddr = QMOImageToLocalUInt32(dyldImage, *(uint32_t *) (listChunk + elemCount * elemSize + kNextImagesOffset) );
			}
			
            QTMFree(listChunk, listChunkSize);
		} while ( (err == 0) && (listChunkAddr != 0) );
	}
	
	return err;
}

static int ImageListForTaskOld(
	task_t          task, 
	QMOImageRef     dyldImage, 
	QMOImageRef *	imageArray, 
	size_t          imageArraySize, 
	size_t *        imageCountPtr
)
	// Returns information about the prepared Mach-O images in a task 
	// using the old (pre-10.4) interface.  See ImageListForTaskOldWithNames 
	// for a discussion of the mechanics of this.
	//
	// On entry, task must not be MACH_PORT_NULL
	// On entry, dyldImage must not be NULL
	// On entry, imageCountPtr must not be NULL
	// On entry, *imageCountPtr is ignored
	// On entry, if imageArray is NULL, imageArraySize must be 0
	// On entry, if imageArray is not NULL, imageArraySize must be the number of 
	// elements available in imageArray
	// On success, if imageArray is not NULL, up to imageArraySize image objects 
	// are returned in the array
	// On success, *imageCountPtr is the number of images in the task; this may 
	// be larger than imageArraySize, in which case the imageArray wasn't big 
	// enough and the returned value tells you how big it needs to be
{
	int	err;
	
	assert(task != MACH_PORT_NULL);
	assert(dyldImage != NULL);
	assert( (imageArray != NULL) || (imageArraySize == 0) );
	assert(imageCountPtr != NULL);
	
	// The old-style interface is only used for 32-bit.
	
	assert( ! QMOImageIs64Bit(dyldImage) );

	// The old interface makes a (strange) distinction between 
	// 'library' images and 'object' images.  To avoid unnecessary 
	// code duplication I've factored this out into a subroutine 
	// that I call twice.
	
	*imageCountPtr = 0;
	err = ImageListForTaskOldWithNames(
		task, 
		dyldImage, 
		"_library_images", 
		"_gdb_nlibrary_images", 
		"_gdb_library_image_size", 
		imageArray, 
		imageArraySize, 
		imageCountPtr
	);
	if (err == 0) {
		err = ImageListForTaskOldWithNames(
			task, 
			dyldImage, 
			"_object_images", 
			"_gdb_nobject_images", 
			"_gdb_object_image_size", 
			imageArray, 
			imageArraySize, 
			imageCountPtr
		);
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Exported Abstraction Layer

static int SortByMachHeader(const void *lhs, const void *rhs)
{
    QTMAddr lhsAddr;
    QTMAddr rhsAddr;
 
    lhsAddr = QMOImageGetMachHeaderOffset(*((QMOImageRef *) lhs));
    rhsAddr = QMOImageGetMachHeaderOffset(*((QMOImageRef *) rhs));

    // Can't do this using the standard (lhsAddr - rhsAddr) trick because the 
    // difference can be so big that it overflows an int, which is the type of 
    // the return value.
    
    if (lhsAddr < rhsAddr) {
        return  -1;
    } else if (lhsAddr > rhsAddr) {
        return  +1;
    } else {
        return 0;
    }
}

extern void QMOImageListDestroy(QMOImageRef *imageArray, size_t imageArrayCount)
	// See comments in header.
{
	uint32_t        imageIndex;

	assert( (imageArray != NULL) || (imageArrayCount == 0) );

    if (imageArray != NULL) {
        for (imageIndex = 0; imageIndex < imageArrayCount; imageIndex++) {
            QMOImageDestroy(imageArray[imageIndex]);
            imageArray[imageIndex] = NULL;
        }
    }
}

bool kQMachOImageListTestOldDyldOnNew = false;
	// By changing this variable you can test the old code when looking 
	// at a new dyld.  For compatibility reasons, the new dyld continues 
	// to support the old interface (at least for 32-bit PowerPC), so this 
	// makes it possible to test the old code on 10.4, which makes life a lot 
	// easier.
    //
    // This is exported for the benefit of the unit test.

bool kQMachOImageListTestSelfShortCircuit = true;
	// By changing this variable to false you can disable the short circuit that 
    // we apply when targetting the local task.
    //
    // This is exported for the benefit of the unit test.

extern int QMOImageListFromSelf(
    QMOImageRef *   imageArray, 
    size_t          imageArrayCount, 
    size_t *        imageCountPtr
)
	// See comments in header.
{
    int             err;
	uint32_t        imageIndex;

	assert( (imageArray != NULL) || (imageArrayCount == 0) );
	assert(imageCountPtr != NULL);
    
    // Blast the output array to NULL to aid in error recovery.
    
    if (imageArray != NULL) {
        memset(imageArray, 0, imageArrayCount * sizeof(*imageArray));
    }
    
    // Do the job.
    
    err = 0;
	*imageCountPtr = _dyld_image_count();
	for (imageIndex = 0; imageIndex < *imageCountPtr; imageIndex++) {
        if ( (imageArray != NULL) && (imageIndex < imageArrayCount) ) {
            const struct mach_header *  machHeader;
            const char *                filePath;

            machHeader = _dyld_get_image_header(imageIndex);
            filePath   = _dyld_get_image_name(imageIndex);
            // fprintf(stderr, "filePath = %s\n", filePath);

            err = QMOImageCreateFromLocalImage(machHeader, filePath, &imageArray[imageIndex]);
            if (err != 0) {
                break;
            }
        }
	}
    if ( (err == 0) && (imageArray != NULL) ) {
        qsort(imageArray, MIN(imageArrayCount, *imageCountPtr), sizeof(*imageArray), SortByMachHeader);
    }
    
    // On error, destroy any partial results.
    
    if (err != 0) {
        QMOImageListDestroy(imageArray, imageArrayCount);
    }
    
    return err;
}

extern int QMOImageListFromTask(
    task_t          task, 
    cpu_type_t      cputype, 
    QMOImageRef *   imageArray, 
    size_t          imageArrayCount, 
    size_t *        imageCountPtr
)
	// See comments in header.
{
	int			err;
	
	assert(task != MACH_PORT_NULL);
	assert( (imageArray != NULL) || (imageArrayCount == 0) );
	assert(imageCountPtr != NULL);
	
    if ( (task == mach_task_self()) && (cputype == QMOGetLocalCPUType()) && kQMachOImageListTestSelfShortCircuit ) {
        err = QMOImageListFromSelf(imageArray, imageArrayCount, imageCountPtr);
    } else {
        QMOImageRef	dyldImage;
        QTMAddr		allImageInfoAddr;

        // Blast the output array to NULL to aid in error recovery.
        
        if (imageArray != NULL) {
            memset(imageArray, 0, imageArrayCount * sizeof(*imageArray));
        }

        dyldImage = NULL;
        
        // The information about the prepared images within a task is published 
        // in various global variables exported by dyld.  So, to start off, we 
        // have to find the address of dyld in the remote task and then create 
        // an QMOImage for it.
        
        err = QMOImageCreateFromTaskDyld(task, cputype, &dyldImage);
        
        // Now we look for the dyld_all_image_infos variable exported by dyld. 
        // If it exists, we looking at a new dyld (10.4 and later), and we use 
        // the new mechanism for finding the prepared images.  If it's not present, 
        // we're looking at an old dyld and we use the old mechanism.
        
        if (err == 0) {
            err = QMOImageLookupSymbol(dyldImage, "_dyld_all_image_infos", &allImageInfoAddr);
            if ( ! kQMachOImageListTestOldDyldOnNew && (err == 0) ) {
                err = ImageListForTaskNew(task, dyldImage, allImageInfoAddr, imageArray, imageArrayCount, imageCountPtr);
            } else {
                err = ImageListForTaskOld(task, dyldImage, imageArray, imageArrayCount, imageCountPtr);
            }
        }
        if ( (err == 0) && (imageArray != NULL) ) {
            qsort(imageArray, MIN(imageArrayCount, *imageCountPtr), sizeof(*imageArray), SortByMachHeader);
        }

        // Clean up.
        
        QMOImageDestroy(dyldImage);

        // On error, destroy any partial results.
        
        if (err != 0) {
            QMOImageListDestroy(imageArray, imageArrayCount);
        }
    }
	
	return err;
}
