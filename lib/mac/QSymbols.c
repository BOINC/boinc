// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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
 *  QSymbols.c
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
    File:       QSymbols.c

    Contains:   Code for symbols to addresses and vice versa.

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

$Log: QSymbols.c,v $
Revision 1.2  2007/03/02 13:00:51
Fix an error handling bug in the creation code.

Revision 1.1  2007/03/02 12:20:29
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our Prototypes
#include "config.h"
#include "QSymbols.h"

// Mac OS Interfaces

//#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef assert
#undef __assert
#define	assert(e)	((void)0)

#include <mach-o/nlist.h>
#include <mach-o/arch.h>

// Project interfaces

#include "QMachOImage.h"
#include "QMachOImageList.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Performance Notes

/*
    I'm currently not happy with the performance characteristics of this module.
    There are /way/ too many linear searches for my liking.  However, there are a
    number of reasons I implemented it this way.  In no particular order, these are:

      o In general I believe in the principle of "make it work, then make it
        go fast".  To that end, I've specifically designed the module to allow
        for future optimisation.

      o The current performance isn't too bad.  While there's lots of linear
        algorithms, the data structures that we're operating on are all cached
        in client-side memory, so there's very little IPC cost.

      o If and when I write the optimised code, it will be nice to have the
        simple code around for testing.  That is, I'll be able to automatically
        test the new code by comparing its results to the old code.

      o This is sample code.  I've already spent way too much time on this, and
        writing the optimised version of the code would consume even more time.
        optimising

      o The overall goal of this project is to provide infrastructure for crash
        reporting; crash reporting is not, in general, time sensitive.

    The way I'd make this go fast are as follows:

      o Don't do anything more on QSymbolsRef creation.  Rather, create the
        'go faster' data structures when the first address-to-symbol or
        symbol-to-address operation is done.  This is because the type of
        data structure you want is different for each operation, and you don't
        want to spend the time creating the data structures for both types
        if the client is only going to use one.

      o The address-to-symbol optimising data structure would consist of a
        sorted array of section starts.  This would be created lazily when the
        first address-to-symbol operation is done.  You could binary search
        this to find the section containing the address.  Then, for each section,
        you would (lazily) create an array of symbols sorted by their start
        address.  So you could binary search that to find the best symbol.

        This would make QSymGetSymbolForAddress O(log2 n) + O(log2 m), where
        n is the number of sections and m is the number of symbols within a
        section.

      o The symbol-to-address optimising data structure would start with a
        hash table that maps library names to image objects.  For each image
        object, there would be another hash table to map symbols to addresses.

        This would make a QSymGetAddressForSymbol call to look up a symbol with
        a particular library O(1).  It would make a flat namespace lookup O(n),
        where n is the number of images.  You could further reduce that to O(1)
        by creating a global hash table for all symbols from all libraries.

      o These two data structures would also support an O(1) implementation of
        QSymGetNextSymbol.
*/

/////////////////////////////////////////////////////////////////

// In the QSymbols data structure, we record the index of the image for dyld
// and the main executable.  We initialise these indexes to kQSymBadImageIndex so
// we can tell whether we've found the corresponding image.

enum {
    kQSymBadImageIndex = (size_t) -1
};

// QMOImage represents all of the symbols with a process.  It's the backing for
// the exported QSymbolsRef type.

struct QSymbols {
    task_t          task;               // target task
    bool            didSuspend;         // true if we suspended the target task
    size_t          imageCount;         // size of images array
    QMOImageRef *   images;             // pointer to images array
    size_t          execIndex;          // index of main executable in images array
    size_t          dyldIndex;          // index of dyld in images array
};
typedef struct QSymbols QSymbols;

#if 0 //! defined(NDEBUG)

    static bool QSymIsValid(QSymbolsRef symRef)
        // Returns true if symRef references a valid QSymbols data structure.
    {
        bool        valid;
        size_t      imageIndex;

        valid = (symRef != NULL)
             && (symRef->imageCount > 0)
             && (symRef->images != NULL)
             && ( (symRef->execIndex == kQSymBadImageIndex) || (symRef->execIndex < symRef->imageCount) )
             && ( (symRef->dyldIndex == kQSymBadImageIndex) || (symRef->dyldIndex < symRef->imageCount) );
        if (valid) {
            for (imageIndex = 0; imageIndex < symRef->imageCount; imageIndex++) {
                if ( symRef->images[imageIndex] == NULL ) {
                    valid = false;
                }
            }
        }

        return valid;
    }

#endif

extern void QSymDestroy(QSymbolsRef symRef)
    // See comment in header.
{
    kern_return_t   krJunk __attribute__((unused));
    size_t          imageIndex;

    if (symRef != NULL) {
        if (symRef->images != NULL) {
            for (imageIndex = 0; imageIndex < symRef->imageCount; imageIndex++) {
                QMOImageDestroy(symRef->images[imageIndex]);
            }
            free(symRef->images);
        }
        if (symRef->didSuspend) {
            krJunk = task_resume(symRef->task);
            assert(krJunk == KERN_SUCCESS);
        }
        free(symRef);
    }
}

extern int QSymCreateFromTask(
    task_t              task,
    bool                suspend,
    cpu_type_t          cputype,
    QSymbolsRef *       symRefPtr
)
    // See comment in header.
{
    int             err;
    kern_return_t   kr;
    QSymbolsRef     symRef;
    size_t          imageIndex;

    assert(task != MACH_PORT_NULL);
    assert( symRefPtr != NULL);
    assert(*symRefPtr == NULL);

	symRef = NULL;

    // Basic initialisation.  This includes getting the image list from the task.

    err = 0;
    if ( (task == mach_task_self()) && suspend ) {
        err = EINVAL;
    }
    if (err == 0) {
        symRef = (QSymbolsRef) calloc(1, sizeof(*symRef));
        if (symRef == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        symRef->task      = task;
        symRef->execIndex = kQSymBadImageIndex;
        symRef->dyldIndex = kQSymBadImageIndex;

        if (suspend) {
            kr = task_suspend(symRef->task);
            err = QTMErrnoFromMachError(kr);

            symRef->didSuspend = (err == 0);
        }
    }
    if (err == 0) {
        err = QMOImageListFromTask(task, cputype, NULL, 0, &symRef->imageCount);
    }
    if (err == 0) {
        symRef->images = (QMOImageRef *) calloc(symRef->imageCount, sizeof(*symRef->images));
        if (symRef->images == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        err = QMOImageListFromTask(task, cputype, symRef->images, symRef->imageCount, &symRef->imageCount);
    }

    // Search for dyld and the main executable.

    if (err == 0) {
        for (imageIndex = 0; imageIndex < symRef->imageCount; imageIndex++) {
            int32_t fileType;

            fileType = QMOImageGetFileType(symRef->images[imageIndex]);
            switch (fileType) {
                case MH_EXECUTE:
                    assert(symRef->execIndex == kQSymBadImageIndex);
                    symRef->execIndex = imageIndex;
                case MH_DYLINKER:
                    assert(symRef->dyldIndex == kQSymBadImageIndex);
                    symRef->dyldIndex = imageIndex;
                    break;
                default:
                    // do nothing
                    break;
            }
        }
    }

    // Clean up.

    if (err != 0) {
        QSymDestroy(symRef);
        symRef = NULL;
    }
    *symRefPtr = symRef;

    assert( (err == 0) == (*symRefPtr != NULL) );

    return err;
}

extern int QSymCreateFromSelf(
    QSymbolsRef *       symRefPtr
)
    // See comment in header.
{
    // All of the infrastructure that QSymCreateFromTask uses (specifically,
    // QMOImageListFromTask and QMOImageCreateFromTask) has a short circuit
    // implementation when task is mach_task_self.  So we can just call that
    // routine with mach_task_self and be assured that we'll get the simple
    // implementation.

    // We have to pass in the local CPU type (from QMOGetLocalCPUType), not
    // CPU_TYPE_ANY.  Otherwise, if we're being run using Rosetta, there's a chance
    // we might find the native dyld rather than the PowerPC one.  If that happens,
    // we end up looking at the wrong list of Mach-O images, and things go south
    // quickly.

    return QSymCreateFromTask(mach_task_self(), false, QMOGetLocalCPUType(), symRefPtr);
}

extern QMOImageRef QSymGetDyldImage(QSymbolsRef symRef)
    // See comment in header.
{
    QMOImageRef result;

    assert( QSymIsValid(symRef) );

    result = NULL;
    if (symRef->dyldIndex != kQSymBadImageIndex) {
        result = symRef->images[symRef->dyldIndex];
    }
    return result;
}

extern QMOImageRef QSymGetExecutableImage(QSymbolsRef symRef)
    // See comment in header.
{
    QMOImageRef result;

    assert( QSymIsValid(symRef) );

    result = NULL;
    if (symRef->execIndex != kQSymBadImageIndex) {
        result = symRef->images[symRef->execIndex];
    }
    return result;
}

extern void QSymGetImages(
    QSymbolsRef     symRef,
    QMOImageRef **  imageArrayPtr,
    size_t *        imageCountPtr
)
    // See comment in header.
{
    assert( QSymIsValid(symRef) );
    assert(imageArrayPtr != NULL);
    assert(imageCountPtr != NULL);

    *imageArrayPtr = symRef->images;
    *imageCountPtr = symRef->imageCount;
}

static int SymbolToAddressCallback(
	QMOImageRef		qmoImage,
	const char *	name,
	uint8_t			type,
	uint8_t			sect,
	uint16_t		desc,
	QTMAddr			value,
	void *			iteratorRefCon,
	bool *			stopPtr
)
    // The QMOImageIterateSymbols callback for QSymGetAddressForSymbol.  See
    // See QMOSymbolIteratorProc for a description of the parameters.
    //
    // iteratorRefCon is a pointer to a QSymSymbolInfo structure.  Initially,
    // only the symbolName field is valid.  If this routine finds a matching
    // symbol, it sets up the symbolType, symbolName, symbolImage, and symbolValue
    // fields.
    //
    // IMPORTANT: We set symbolInfo->symbolName to "name" because the initial
    // value of symbolName comes from the client, so the lifetime of the referenced
    // memory is undefined.  However, our "name" input parameter comes from the
    // QMOImageRef object, so it persists until that object is destroy, which
    // only happens when the QSymbolsRef object is destroyed.
{
    #pragma unused(sect, desc)
	int             err;
	QSymSymbolInfo * symbolInfo;

	assert(qmoImage != NULL);
    assert(name != NULL);
    assert( stopPtr != NULL);
    assert(*stopPtr == false);

	err = 0;

    // Ignore debugger symbols.

	if ( ! (type & N_STAB) ) {
		symbolInfo = (QSymSymbolInfo *) iteratorRefCon;

        // Do the names match.

		if ( strcmp(name, symbolInfo->symbolName) == 0 ) {

            // If so, check that it's one of our supported types and, if so,
            // set up the output structure.

			switch (type & N_TYPE) {
				case N_ABS:
                    symbolInfo->symbolType   = (type & N_EXT) ? kQSymDyldPublicSymbol : kQSymDyldPrivateSymbol;
                    symbolInfo->symbolImage  = qmoImage;
                    symbolInfo->symbolName   = name;
					symbolInfo->symbolValue  = value;
					*stopPtr = true;
					break;
				case N_SECT:
                    symbolInfo->symbolType   = (type & N_EXT) ? kQSymDyldPublicSymbol : kQSymDyldPrivateSymbol;
                    symbolInfo->symbolImage  = qmoImage;
                    symbolInfo->symbolName   = name;
					symbolInfo->symbolValue  = value + QMOImageGetSlide(qmoImage);
					*stopPtr = true;
					break;
			}
		}
	}
	return err;
}

extern char *   QSymCreateLibraryNameWithSuffix(const char *libraryName, const char *suffix);
    // I need to export this for the benefit of the test program.  So it isn't
    // in a header, but it is "extern".

extern char *   QSymCreateLibraryNameWithSuffix(const char *libraryName, const char *suffix)
    // Add the suffix (for example, "_debug") to the library name (for example,
    // "/usr/lib/libSystem.B_debug.dylib").  Place the suffix before the extension
    // if there is one.
{
    const char *    nameStart;
    const char *    lastDot;
    size_t          libraryNameLen;
    const char *    extension;
    char *          result;

    assert(libraryName != NULL);
    assert(suffix != NULL);

    nameStart = strrchr(libraryName, '/');
    if (nameStart == NULL) {
        nameStart = libraryName;
    } else {
        nameStart += 1;
    }
    lastDot   = strrchr(nameStart, '.');

    // Default to just concatenating the suffix.

    libraryNameLen = strlen(libraryName);
    extension      = "";

    // If there's an extensiont, break the string at the dot and place the suffix
    // between the two.

    if (lastDot != NULL) {
        libraryNameLen = lastDot - libraryName;
        extension      = lastDot;
    }

    result = NULL;
    asprintf(&result, "%.*s%s%s", (int) libraryNameLen, libraryName, suffix, extension);
    return result;
}

extern int QSymGetAddressForSymbol(
    QSymbolsRef         symRef,
    const char *        libraryName,
    const char *        symbolName,
    QSymSymbolInfo *    symbolInfo
)
    // See comment in header.
{
    int         err;
    size_t      imageIndex;
    size_t      imageIndexStart;
    size_t      imageIndexLimit;

    assert( QSymIsValid(symRef) );
    // libraryName may be NULL
    assert(symbolName != NULL);
    assert(symbolInfo != NULL);

    // First find the library to search, if any.  We first try with no suffix,
    // then with the _debug and _profile suffixes.

    err = 0;
    imageIndexStart = 0;
    imageIndexLimit = symRef->imageCount;
    if (libraryName != NULL) {
        static const char * kSuffixes[] = { "", "_debug", "_profile", NULL };
        size_t suffixIndex;

        suffixIndex = 0;
        do {
            char *      libraryNameWithSuffix;

            libraryNameWithSuffix = QSymCreateLibraryNameWithSuffix(libraryName, kSuffixes[suffixIndex]);
            if (libraryNameWithSuffix == NULL) {
                err = ENOMEM;
            }

            if (err == 0) {
                for (imageIndex = 0; imageIndex < symRef->imageCount; imageIndex++) {
                    const char *    thisImageName;

                    thisImageName = QMOImageGetLibraryID(symRef->images[imageIndex]);
                    if ( (thisImageName != NULL) && (strcmp(thisImageName, libraryNameWithSuffix) == 0) ) {
                        break;
                    }
                }
                if (imageIndex == symRef->imageCount) {
                    err = ESRCH;
                } else {
                    imageIndexStart = imageIndex;
                    imageIndexLimit = imageIndex + 1;
                }
            }

            free(libraryNameWithSuffix);

            suffixIndex += 1;
        } while ( (err == ESRCH) && (kSuffixes[suffixIndex] != NULL) );
    }

    // Within the range of libraries specified by imageIndexStart and imageIndexLimit,
    // search for the symbol.

    if (err == 0) {
        memset(symbolInfo, 0, sizeof(*symbolInfo));
        symbolInfo->symbolType  = kQSymNoSymbol;
        symbolInfo->symbolName  = symbolName;

        for (imageIndex = imageIndexStart; imageIndex < imageIndexLimit; imageIndex++) {
            err = QMOImageIterateSymbols(symRef->images[imageIndex], SymbolToAddressCallback, symbolInfo);
            if ( (err != 0) || (symbolInfo->symbolType != kQSymNoSymbol) ) {
                break;
            }
        }
        if ( (err == 0) && (symbolInfo->symbolType == kQSymNoSymbol) ) {
            err = ESRCH;
        }
    }

    // Clean up if there's any failure.

    if (err != 0) {
        memset(symbolInfo, 0, sizeof(*symbolInfo));
        symbolInfo->symbolType  = kQSymNoSymbol;
    }

    // Post-conditions

    assert( (err == 0) == (symbolInfo->symbolType != kQSymNoSymbol) );
    assert( (symbolInfo->symbolType != kQSymNoSymbol) == (symbolInfo->symbolImage != NULL) );
    assert( (symbolInfo->symbolType != kQSymNoSymbol) == (symbolInfo->symbolName  != NULL) );
    assert(symbolInfo->symbolOffset == 0);      // for sym -> addr lookup, offset always zero

    return err;
}

extern int QSymGetAddressesForSymbols(
    QSymbolsRef         symRef,
    size_t              count,
    const char *        libraryNames[],
    const char *        symbolNames[],
    QSymSymbolInfo      symbolInfos[]
)
    // See comment in header.
{
    int         err;
    size_t      symbolIndex;

    assert( QSymIsValid(symRef) );
    assert(count > 0);
    // libraryNames may be NULL
    assert(symbolNames != NULL);
    for (symbolIndex = 0; symbolIndex < count; symbolIndex++) {
        assert(symbolNames[symbolIndex] != NULL);
    }
    assert(symbolInfos != NULL);

    // The current implementation is very naive.  I could definitely make this
    // faster, but see my "Performance Notes" comment at the top of this file.

    err = 0;
    for (symbolIndex = 0; symbolIndex < count; symbolIndex++) {
        const char *    thisLib;

        if (libraryNames == NULL) {
            thisLib = NULL;
        } else {
            thisLib = libraryNames[symbolIndex];
        }
        (void) QSymGetAddressForSymbol(symRef, thisLib, symbolNames[symbolIndex], &symbolInfos[symbolIndex]);
    }

    return err;
}

extern int QSymGetImageForAddress(
	QSymbolsRef		symRef,
	QTMAddr			addr,
	QMOImageRef *	qmoImagePtr,
	uint32_t *		sectIndexPtr
)
    // See comment in header.
{
	int					err;
	int                 junk __attribute__((unused));
	size_t              imageIndex;
	QTMAddr             slide;
	uint32_t            sectCount;
	uint32_t            sectIndex;
	struct section_64	sect;
	bool                found;

	assert( QSymIsValid(symRef) );
	assert( qmoImagePtr != NULL );
	// sectIndexPtr may be NULL

	// Iterate through the libraries looking for the section that contains the address.

    found = false;
    for (imageIndex = 0; imageIndex < symRef->imageCount; imageIndex++) {
        slide = QMOImageGetSlide(symRef->images[imageIndex]);

        sectCount = QMOImageGetSectionCount(symRef->images[imageIndex]);

        for (sectIndex = 0; sectIndex < sectCount; sectIndex++) {
            junk = QMOImageGetSectionByIndex(symRef->images[imageIndex], sectIndex, &sect);
            assert(junk == 0);

            if ( (strcmp(sect.segname, "__TEXT") == 0) || (strcmp(sect.segname, "__DATA") == 0) ) {
                found = (addr >= (sect.addr + slide)) && (addr < (sect.addr + slide + sect.size));
                if ( found ) {
					*qmoImagePtr = symRef->images[imageIndex];
					if (sectIndexPtr != NULL) {
						*sectIndexPtr = sectIndex;
					}
					found = true;
                    break;
                }
            }
        }
        if ( found ) {
            break;
        }
    }
	if (found) {
		err = 0;
	} else {
		err = ESRCH;
	}

	assert( (err != 0) || (*qmoImagePtr != NULL) );

	return err;
}

// AddressToSymbolContext is the parameter block passed to AddressToSymbolCallback
// via its iteratorRefCon.

struct AddressToSymbolContext {
    QTMAddr         requiredAddr;
    uint8_t         requiredSect;
    const char *    currentSymbol;
    uint8_t         currentSymbolType;
    QTMAddr         currentSymbolValue;
    QTMOffset       currentOffset;
};
typedef struct AddressToSymbolContext AddressToSymbolContext;

static int AddressToSymbolCallback(
	QMOImageRef		qmoImage,
	const char *	name,
	uint8_t			type,
	uint8_t			sect,
	uint16_t		desc,
	QTMAddr			value,
	void *			iteratorRefCon,
	bool *			stopPtr
)
    // The QMOImageIterateSymbols callback for QSymGetSymbolForAddress.  See
    // See QMOSymbolIteratorProc for a description of the parameters.
    //
    // iteratorRefCon is a pointer to a AddressToSymbolContext structure.  Initially,
    // the requiredSect, requiredAddr, and currentOffset fields are valid.  As
    // this routine finds better matching symbols, it fills out currentSymbol,
    // currentSymbolType, and currentSymbolValue, and adjusts currentOffset.
    // In this way, currentOffset slowly decreases until, after all symbols
    // have been considered, AddressToSymbolContext contains the best match.
{
    #pragma unused(desc)
	int                         err;
	AddressToSymbolContext *    iterContext;
    QTMAddr                     symValue;
    QTMOffset                   symOffset;

	assert(qmoImage != NULL);
    assert(name != NULL);
    assert( stopPtr != NULL);
    assert(*stopPtr == false);

	err = 0;

    // Ignore debugger symbols.

	if ( ! (type & N_STAB) ) {
		iterContext = (AddressToSymbolContext *) iteratorRefCon;

        // Check that it's a section-based symbol within our section.

        if ( ((type & N_TYPE) == N_SECT) && (sect == iterContext->requiredSect) ) {

            // Check that it's a better match than our current symbol.

            symValue  = value + QMOImageGetSlide(qmoImage);
            symOffset = (iterContext->requiredAddr - symValue);
            if ( (symValue <= iterContext->requiredAddr) && (symOffset < iterContext->currentOffset) ) {

                // If it is, record the name of the symbol and its offset.

                iterContext->currentSymbol      = name;
                iterContext->currentSymbolType  = type;
                iterContext->currentSymbolValue = symValue;
                iterContext->currentOffset      = symOffset;
            }
        }
	}
	return err;
}

extern int QSymGetSymbolForAddress(
    QSymbolsRef         symRef,
    QTMAddr             addr,
    QSymSymbolInfo *    symbolInfo
)
    // See comment in header.
{
    int                 err;
	QMOImageRef			qmoImage;
    uint32_t            sectIndex;
    struct section_64   sect;

    assert( QSymIsValid(symRef) );
    assert(symbolInfo != NULL);

    memset(symbolInfo, 0, sizeof(*symbolInfo));
    symbolInfo->symbolType = kQSymNoSymbol;

	// Find the section and then get its information.

    err = QSymGetImageForAddress(symRef, addr, &qmoImage, &sectIndex);
	if (err == 0) {
		err = QMOImageGetSectionByIndex(qmoImage, sectIndex, &sect);
	}

    // Within that image, iterate through the symbols that are relative to the
    // section we found and that closest match the address.

    if (err == 0) {
        AddressToSymbolContext  iterContext;

        iterContext.requiredAddr  = addr;
        iterContext.requiredSect  = sectIndex + 1;  // sect is one-based in QMOImageIterateSymbols callback
        iterContext.currentSymbol      = NULL;
        iterContext.currentSymbolType  = 0;
        iterContext.currentSymbolValue = 0;
        iterContext.currentOffset      = sect.size;

        err = QMOImageIterateSymbols(qmoImage, AddressToSymbolCallback, &iterContext);

        if (err == 0) {
            if (iterContext.currentSymbol == NULL) {
                err = ESRCH;
            } else {
                symbolInfo->symbolType   = (iterContext.currentSymbolType & N_EXT) ? kQSymDyldPublicSymbol : kQSymDyldPrivateSymbol;
                symbolInfo->symbolImage  = qmoImage;
                symbolInfo->symbolName   = iterContext.currentSymbol;
                symbolInfo->symbolValue  = iterContext.currentSymbolValue;
                symbolInfo->symbolOffset = iterContext.currentOffset;
            }
        }
    }

    assert( (err == 0) == (symbolInfo->symbolType != kQSymNoSymbol) );
    assert( (symbolInfo->symbolType != kQSymNoSymbol) == (symbolInfo->symbolImage != NULL) );
    assert( (symbolInfo->symbolType != kQSymNoSymbol) == (symbolInfo->symbolName  != NULL) );

    return err;
}

extern int QSymGetSymbolsForAddresses(
    QSymbolsRef         symRef,
    size_t              count,
    QTMAddr             addrs[],
    QSymSymbolInfo      symbolInfos[]
)
    // See comment in header.
{
    int     err;
    size_t  addrIndex;

    assert( QSymIsValid(symRef) );
    assert(count > 0);
    assert(addrs != NULL);
    assert(symbolInfos != NULL);

    // The current implementation is very naive.  I could definitely make this
    // faster, but see my "Performance Notes" comment at the top of this file.

    err = 0;
    for (addrIndex = 0; addrIndex < count; addrIndex++) {
        (void) QSymGetSymbolForAddress(symRef, addrs[addrIndex], &symbolInfos[addrIndex]);
    }
    return err;
}

// NextSymbolContext is the parameter block passed to NextSymbolCallback
// via its iteratorRefCon.

struct NextSymbolContext {
    QTMAddr         requiredAddr;
    const char *    currentSymbol;
    uint8_t         currentSymbolType;
    QTMAddr         currentSymbolValue;
    QTMOffset       currentOffset;
};
typedef struct NextSymbolContext NextSymbolContext;

static int NextSymbolCallback(
	QMOImageRef		qmoImage,
	const char *	name,
	uint8_t			type,
	uint8_t			sect,
	uint16_t		desc,
	QTMAddr			value,
	void *			iteratorRefCon,
	bool *			stopPtr
)
    // The QMOImageIterateSymbols callback for QSymGetSymbolForAddress.  See
    // See QMOSymbolIteratorProc for a description of the parameters.
    //
    // iteratorRefCon is a pointer to a NextSymbolContext structure.  Initially,
    // the requiredAddr and currentOffset fields are valid.  As this routine
    // finds better matching symbols, it fills out currentSymbol,
    // currentSymbolType, and currentSymbolValue, and adjusts currentOffset.
    // In this way, currentOffset slowly decreases until, after all symbols
    // have been considered, NextSymbolContext contains the symbol immediately
    // following the one at requiredAddr.
{
    #pragma unused(sect, desc)
	int                 err;
	NextSymbolContext * iterContext;
    QTMAddr             symValue;
    QTMOffset           symOffset;

	assert(qmoImage != NULL);
    assert(name != NULL);
    assert( stopPtr != NULL);
    assert(*stopPtr == false);

	err = 0;

    // Ignore debugger symbols.

	if ( ! (type & N_STAB) ) {
		iterContext = (NextSymbolContext *) iteratorRefCon;

        // Check that it's a section-based symbol.  Don't want to get confused by
        // undefined symbols (N_UNDF) or absolute symbols (N_ABS) or any other
        // symbol type for that matter.

        if (((type & N_TYPE) == N_SECT)) {

            // Check that it's a better match than our current symbol.

            symValue  = value + QMOImageGetSlide(qmoImage);
            symOffset = (symValue - iterContext->requiredAddr);
            if ( (symValue > iterContext->requiredAddr) && (symOffset < iterContext->currentOffset) ) {

                // If it is, record the name of the symbol and its offset.

                iterContext->currentSymbol      = name;
                iterContext->currentSymbolType  = type;
                iterContext->currentSymbolValue = symValue;
                iterContext->currentOffset      = symOffset;
            }
        }
	}
	return err;
}

extern int QSymGetNextSymbol(
    QSymbolsRef             symRef,
    const QSymSymbolInfo *  symbol,
    QSymSymbolInfo *        nextSymbol
)
    // See comment in header.
{
    int                 err;
    NextSymbolContext   iterContext;

    assert( QSymIsValid(symRef) );
    assert(symbol->symbolType  != kQSymNoSymbol);
    assert(symbol->symbolImage != NULL);
    assert(symbol->symbolName  != NULL);
    // I use symbol->symbolValue but I can't think of any meaningful validity checks.

    // Iterate all of the symbols in the image looking for the one that's closet
    // to symbol->symbolValue.  Set currentOffset to a very large number so that
    // any symbol greater than symbol->symbolValue is considered closer.

    iterContext.requiredAddr       = symbol->symbolValue;
    iterContext.currentSymbol      = NULL;
    iterContext.currentSymbolType  = 0;
    iterContext.currentSymbolValue = 0;
    iterContext.currentOffset      = (QTMAddr) -1;

    err = QMOImageIterateSymbols(symbol->symbolImage, NextSymbolCallback, &iterContext);
    if (err == 0) {
        if (iterContext.currentSymbol == NULL) {
            err = ESRCH;
        } else {
            nextSymbol->symbolType   = (iterContext.currentSymbolType & N_EXT) ? kQSymDyldPublicSymbol : kQSymDyldPrivateSymbol;
            nextSymbol->symbolImage  = symbol->symbolImage;
            nextSymbol->symbolName   = iterContext.currentSymbol;
            nextSymbol->symbolValue  = iterContext.currentSymbolValue;
            nextSymbol->symbolOffset = iterContext.currentOffset;
        }
    }

    assert( (err == 0) == (nextSymbol->symbolType != kQSymNoSymbol) );

    return err;
}
