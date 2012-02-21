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
 *  QSymbols.h
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
    File:       QSymbols.h

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

$Log: QSymbols.h,v $
Revision 1.1  2007/03/02 12:20:33         
First checked in.


*/

#ifndef _QTASKSYMBOLS_H
#define _QTASKSYMBOLS_H

/////////////////////////////////////////////////////////////////

// Mac OS Interfaces

#include <stdint.h>
#include <stdlib.h>

// Put <mach/mach.h> inside extern "C" guards for the C++ build 
// because the Mach header files don't always have them.

#if defined(__cplusplus)
	extern "C" {
#endif

#include <mach/mach.h>

#if defined(__cplusplus)
	}
#endif

// Our interfaces

#include "QMachOImage.h"

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/*!
    @header         QTaskSymbols.h
    
    @abstract       Code for symbol to address conversion, and vice versa.

    @discussion     This module provides an abstraction layer for mapping symbols 
                    to addresses, and vice versa, within a particular process.  To 
                    map symbols within a process, first create a symbols object 
                    (QTaskSymRef) from either the current process (QSymCreateFromSelf) 
                    or an arbitrary process (QSymCreateFromTask).
                    
                    You can then call (QSymGetAddressForSymbol) to map a symbol to an 
                    address or (QSymGetSymbolForAddress) to map an address to a symbol.  
                    There are also bulk versions of these calls 
                    (QSymGetAddressesForSymbols and QSymGetSymbolsForAddresses) to 
                    allow for future performance optimisations.  The QSymGetNextSymbol 
                    routine lets you find the symbol that immediately follows 
                    another symbol; this is useful is you want to find the extent
                    of a symbol.
                    
                    Finally, call QSymDestroy to destroy the symbols object.
                    
                    IMPORTANT: This module has not been performance tuned.  
                    Specifically, it uses a bunch of algorithms that are linear 
                    with the number of images within the task, and linear with 
                    the number of symbols within the images.  If you need to map 
                    lots of symbols, you should probably performance tune the 
                    implementation.
                    
                    This module assumes that the state of the process you're 
                    investigating is stable.  That is, when you create a symbols 
                    object for a process, you need to guarantee that the state 
                    of the process does not change while that symbols object exists.  
                    The best way to prevent this from happening is to suspend the 
                    process while you're accessing it.  You can automatically 
                    suspend the task for the lifetime of the symbols object by 
                    pass true to the suspend parameter of QSymCreateFromTask.
                    
                    Many routines in this module (for example, QSymGetAddressForSymbol) 
                    return you pointers to structures that are embedded within the 
                    symbols object.  Such pointers are only valid for the lifetime 
                    of the symbols object itself.  Once you've destroyed the image 
                    object (by calling QSymDestroy), you can no longer rely 
                    on the validity of these pointers.
                    
                    This module works in terms of symbol names seen by the linker. 
                    Keep in mind the following:
                    
                      o The C compiler adds a leading underscore to all C symbols.
                        For example, the C routine "getpid" because is seen by the 
                        linker as "_getpid".
                    
                      o The C++ compiler mangles names extensively.

                    While this module is currently implemented entirely in terms of 
                    Mach-O, the interface is not Mach-O specific; you could enhance 
                    the module to support other types of symbols (for example, 
                    PowerPC traceback tables).
*/

/*!
    @typedef        QSymbolsRef
    
    @abstract       A reference to the symbols for a process.
    
    @discussion     This type is opaque; to create, destroy, or access it, you must 
                    use the routines in this module.
*/
typedef struct QSymbols * QSymbolsRef;

/*!
    @functiongroup  Create and Destroy
*/
#pragma mark ***** Create and Destroy

/*!
    @function       QSymCreateFromTask
    
    @abstract       Creates a symbols object for an arbitrary process.
    
    @discussion     Creates a symbols object for the specified procses.

                    Once you're done with the object, call QSymDestroy to 
                    destroy it.

    @param task     Must be the name of a valid send right for the task control 
                    port of the process to inspect; mach_task_self is just fine.
                    
                    If you do pass in mach_task_self, this routine automatically 
                    enables some nice optimisations.

    @param suspend  If true, the target task is suspended while the symbols object 
                    exists.
                    
                    Must not be true if task is mach_task_self.
                    
    @param cputype  The CPU type of the dynamic linker from which you want to get 
                    the symbols.  Typically you would pass CPU_TYPE_ANY to use 
                    the first dynamic linker that's discovered.  See 
                    QMOImageCreateFromTaskDyld for a detailed discussion of this 
                    value.

    @param symRefPtr
                    On entry, symRefPtr must not be NULL and *symRefPtr must 
                    be NULL.  On success, *symRefPtr will be a reference to the 
                    symbols object that's been created.  On error, *symRefPtr 
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QSymCreateFromTask(
    task_t              task,
    bool                suspend,
    cpu_type_t          cputype,
    QSymbolsRef *       symRefPtr
);

/*!
    @function       QSymCreateFromSelf
    
    @abstract       Creates a symbols object for the current process.
    
    @discussion     Creates a symbols object for the current procses.  This is 
                    equivalent to calling QSymCreateFromTask, passing it 
                    mach_task_self and the current CPU type.

                    Once you're done with the object, call QSymDestroy to 
                    destroy it.

    @param symRefPtr
                    On entry, symRefPtr must not be NULL and *symRefPtr must 
                    be NULL.  On success, *symRefPtr will be a reference to the 
                    symbols object that's been created.  On error, *symRefPtr 
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QSymCreateFromSelf(
    QSymbolsRef *       symRefPtr
);

/*!
    @function       QSymDestroy
    
    @abstract       Destroys a symbols object.
    
    @discussion     Destroys the supplied symbols object.
                    
    @param qmoImage The symbols object to destroy.  If this is NULL, the routine 
                    does nothing.
*/
extern void QSymDestroy(QSymbolsRef symRef);

/*!
    @functiongroup  Basic Accessors
*/
#pragma mark ***** Basic Accessors

/*!
    @function       QSymGetDyldImage
    
    @abstract       Gets the dyld image object for the process.
    
    @discussion     Gets the dyld image object for the process which the symbols 
                    object is inspecting.

    @param symRef   A valid symbols object.

    @result         A reference to the image object.  It's possible for this to 
                    be NULL (although that would be /very/ weird).
*/
extern QMOImageRef QSymGetDyldImage(QSymbolsRef symRef);

/*!
    @function       QSymGetExecutableImage
    
    @abstract       Gets the executable image object for the process.
    
    @discussion     Gets the executable image object for the process which the symbols 
                    object is inspecting.

    @param symRef   A valid symbols object.

    @result         A reference to the image object.  It's possible for this to 
                    be NULL (although that would be /very/ weird).
*/
extern QMOImageRef QSymGetExecutableImage(QSymbolsRef symRef);

/*!
    @function       QSymGetImages
    
    @abstract       Gets all of the image objects for the process.
    
    @discussion     Gets all of the image objects for the process.
    
                    This routine can never fail; the image array is created when 
                    the symbols object is created, and any failures would 
                    have caused creation to fail.
                    
                    IMPORTANT: Do not free the array that this returns, or indeed 
                    any of the image objects contained in the array.  These all 
                    belongs to the symbols object and will be destroyed when 
                    the symbols object is destroyed.

    @param symRef   A valid symbols object.

    @param imageArrayPtr
                    A pointer to an image object array pointer.  On entry,  
                    imageArrayPtr must not be NULL and *imageArrayPtr is 
                    ignored.  On return, *imageArrayPtr will be a pointer 
                    to *imageCountPtr QMOImageRef objects.

    @param imageCountPtr
                    A pointer to an image object count.  On entry, imageCountPtr 
                    must not be NULL and *imageCountPtr is ignored.  On return, 
                    *imageCountPtr is the size of the image array whose pointer 
                    is returned in *imageArrayPtr.
*/
extern void QSymGetImages(
    QSymbolsRef     symRef, 
    QMOImageRef **  imageArrayPtr, 
    size_t *        imageCountPtr
);

/*!
    @functiongroup  Symbol to Address
*/
#pragma mark ***** Symbol to Address

/*!
    @enum           QSymSymbolType
    
    @abstract       Identifies the type of symbol found.
    
    @discussion     This module currently works entirely in terms of Mach-O, so 
                    this enumeration only contains values for Mach-O symbols. 
                    If I extended the module to handle other types of symbols 
                    (for example, PowerPC traceback tables), I would extend this 
                    enumeration to identify those symbol types.

    @constant kQSymNoSymbol
                    No symbol was found.  This is useful for bulk routines 
                    (like QSymGetAddressesForSymbols), where failing to find one 
                    symbol does not cause the entire routine to fail.

    @constant kQSymDyldPublicSymbol
                    Found an exported Mach-O symbol.

    @constant kQSymDyldPrivateSymbol
                    Found a private Mach-O symbol.
*/
enum QSymSymbolType {
	kQSymNoSymbol = 0,
	kQSymDyldPublicSymbol,
	kQSymDyldPrivateSymbol
};
typedef enum QSymSymbolType QSymSymbolType;

/*!
    @struct         QSymSymbolInfo
    
    @abstract       Information about a particular symbol.
    
    @discussion     This structure is used to return information about a particular 
                    symbol.
                    
                    The pointers in a QSymSymbolInfo structure are valid until you 
                    destroy the symbols object that you passed to the routine that 
                    returned the structure.

    @field symbolType
                    The type of the symbol.  If this is kQSymNoSymbol, the remaining 
                    fields are meaningless.

    @field symbolImage
                    A reference to the image object that contains the symbol.  
                    
                    For symbols of type kQSymDyldPublicSymbol and kQSymDyldPrivateSymbol, 
                    this will not be NULL.

    @field symbolName
                    The name of the symbol.  This is the name seen as the linker 
                    so, for example, the name of a C function would include the 
                    leading underscore added by the C compiler.

                    For symbols of whose type is not kQSymNoSymbol, this will not 
                    be NULL.

    @field symbolValue
                    The address of the symbol whose name is symbolName within the 
                    process for which you created the symbols object.

    @field symbolOffset
                    For the symbol-to-address mapping routines, this will always be 
                    zero.
                    
                    For the address-to-symbol mapping routines, this contains the 
                    offset of the address from the start of the symbol.
*/
struct QSymSymbolInfo {
    QSymSymbolType      symbolType;
    QMOImageRef         symbolImage;
	const char *        symbolName;
	QTMAddr             symbolValue;        // address of symbol named by symbolName
    QTMOffset           symbolOffset;
};
typedef struct QSymSymbolInfo QSymSymbolInfo;

/*!
    @function       QSymGetAddressForSymbol
    
    @abstract       Gets the address for a symbol.
    
    @discussion     Gets the address for a symbol.  The symbol is specified by 
                    both a library name and a symbol name.  If the library name 
                    is NULL, the routine searches all Mach-O images within the process 
                    for a symbol with the specified name.  The order that the images 
                    are search is not specified.  This simulates dyld's flat 
                    namespace concept.
                    
                    If the library name is not NULL, it should contain the name 
                    of a shared library.  In that case, the routine searches 
                    just that shared library for the symbol.  This simulates dyld's 
                    two-level namespace.
                    
                    A library name is equivalent to the name given in the LC_ID_DYLIB 
                    load command of the library.  For example, the library name of 
                    "/usr/lib/libz.1.2.3.dylib" is "/usr/lib/libz.1.dylib".  You can 
                    get the library name of using <x-man-page://1/otool> with the 
                    "-l" option, looking through the output for the LC_ID_DYLIB 
                    command.  For example:
                    
                    $ otool -l /usr/lib/libz.1.2.3.dylib | grep -A 5 LC_ID_DYLIB
                              cmd LC_ID_DYLIB
                          cmdsize 48
                             name /usr/lib/libz.1.dylib (offset 24)
                       time stamp 1170684920 Mon Feb  5 14:15:20 2007
                          current version 1.2.3
                    compatibility version 1.0.0
                    
                    This will find absolute and section-based symbols (those of 
                    type N_ABS and N_SECT) that aren't debugger symbols (that is, 
                    N_TYPE is not set).
                    
    @param symRef   A valid symbols object.

    @param libraryName
                    The name of the library to search, or NULL to search all images.

    @param symbolName
                    The name of the symbol to search for.  If this is a C symbol, 
                    you must include the leading underscore that's inserted by 
                    the compiler for all C symbols.

    @param symbolInfo
                    On success, this structured is filled out with information 
                    about the found symbol.
                    
                    On failure, the symbolType field will be kQSymNoSymbol.

    @result         An errno-style error code per QTMErrnoFromMachError.  If the 
                    symbol is not found, this will be ESRCH.
*/
extern int QSymGetAddressForSymbol(
    QSymbolsRef         symRef,
    const char *        libraryName,
    const char *        symbolName,
    QSymSymbolInfo *    symbolInfo
);

/*!
    @function       QSymGetAddressesForSymbols
    
    @abstract       Maps multiple symbols to their addresses.
    
    @discussion     This is a bulk version of QSymGetAddressForSymbol.  Using it, 
                    you can map multiple symbols to their addresses in one 
                    function call.
                    
                    This routine is equivalent to calling QSymGetAddressForSymbol 
                    count times, each time passing corresponding entries of the 
                    libraryNames, symbolNames and symbolInfos arrays.
    
                    IMPORTANT: Failing to map a symbol to an address will not 
                    cause this routine to fail.  Rather, the corresponding entry 
                    in symbolInfos will have a symbol type of kQSymNoSymbol.

    @param symRef   A valid symbols object.

    @param count    The number of entries in the libraryNames, symbolNames, 
                    and symbolInfos arrays.

    @param libraryNames
                    An array of library names.  Each element is equivalent to the 
                    libraryName parameter of QSymGetAddressForSymbol.
                    
                    This may be NULL, in which case it's treated as an array of 
                    count items, each of which is NULL.

    @param symbolNames
                    The array of symbol names to search for.  Each element is 
                    equivalent to the symbolName parameter of QSymGetAddressForSymbol.

    @param symbolInfos
                    An array of symbol information structures.  Each element is 
                    equivalent to the symbolInfo parameter of QSymGetAddressForSymbol. 

                    If the mapping failed for a particular element, the symbolType 
                    field will be kQSymNoSymbol

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QSymGetAddressesForSymbols(
    QSymbolsRef         symRef,
    size_t              count,
    const char *        libraryNames[],
    const char *        symbolNames[],
    QSymSymbolInfo      symbolInfos[]
);

/*!
    @function       QSymGetNextSymbol
    
    @abstract       Returns the symbol that immediately follows a specified symbol.
    
    @discussion     Returns the symbol that immediately follows a specified symbol. 
                    
                    IMPORTANT: This finds the next symbol within a particular 
                    image.  That is, on success, the symbolImage field of the 
                    next symbol will be equal to the symbolImage field of the 
                    original symbol.

    @param symRef   A valid symbols object.

    @param symbol   The symbol for which you wish to find the next symbol.  Typically 
                    you pass in the results of QSymGetAddressForSymbol.  Regardless, 
                    the symbolType field must not be kQSymNoSymbol and the 
                    symbolType, symbolImage, symbolName and symbolValue fields must 
                    be valid.

    @param nextSymbol
                    On success, this describes the symbol that immediately follows 
                    symbol.

    @result         An errno-style error code per QTMErrnoFromMachError.  If there  
                    is no next symbol, this will be ESRCH.
*/
extern int QSymGetNextSymbol(
    QSymbolsRef             symRef,
    const QSymSymbolInfo *  symbol,
    QSymSymbolInfo *        nextSymbol
);

/*!
    @functiongroup  Address to Symbol
*/
#pragma mark ***** Address to Symbol

/*!
    @function       QSymGetImageForAddress
    
    @abstract       Gets the image that contains the specified address.
    
    @discussion     Returns the image that contains the specified address and, 
					optionally, the section within that image.
					
					IMPORTANT: The current implementation only considers sections 
                    in the "__TEXT" and "__DATA" segments.

    @param symRef   A valid symbols object.

    @param addr     The address to search for; this is within the process specified 
					when you created the symbols object.

    @param qmoImagePtr
					Must not be NULL.
					
					On success, *qmoImagePtr is set to the image object that has 
					a section that contains the address.  This image object is 
					owned by the symbols object and will be destroyed when the 
					symbols object is destroy; you must not destroy it yourself.  
					On failure, the value in *qmoImagePtr is unspecified.

    @param sectIndexPtr
					If NULL, this parameter is ignored.
					
					If not NULL then, on success, *sectIndexPtr will contain 
					the zero-based section index of the section containing the 
					address (within the image returned in *qmoImagePtr).  On 
					failure, the value in *sectIndexPtr is unspecified.

    @result         An errno-style error code per QTMErrnoFromMachError.  If no 
                    image is found, this will be ESRCH.
*/
extern int QSymGetImageForAddress(
	QSymbolsRef		symRef, 
	QTMAddr			addr,
	QMOImageRef *	qmoImagePtr, 
	uint32_t *		sectIndexPtr
);

/*!
    @function       QSymGetSymbolForAddress
    
    @abstract       Maps an address to a symbol.
    
    @discussion     Maps an address (within the process specified when you created 
                    the symbols object) to a symbol (within that process).  This 
                    first tries to find the section containing the symbol; if 
                    that succeeds, it looks through all of the symbol in that 
                    section looking for the one that's least less than the specified 
                    address.
                    
                    IMPORTANT: The current implementation only considers sections 
                    in the "__TEXT" and "__DATA" segments.

    @param symRef   A valid symbols object.

    @param addr     The address to map; this is within the process specified when 
                    you created the symbols object

    @param symbolInfo
                    On success, this structured is filled out with information 
                    about the found symbol.  The symbolOffset field will contain 
                    the offset of from the start of the symbol (that is, the 
                    value in symbolValue) to the address.
                    
                    On failure, the symbolType field will be kQSymNoSymbol.

    @result         An errno-style error code per QTMErrnoFromMachError.  If no 
                    symbol is found, this will be ESRCH.
*/
extern int QSymGetSymbolForAddress(
    QSymbolsRef         symRef,
    QTMAddr             addr,
    QSymSymbolInfo *    symbolInfo
);

/*!
    @function       QSymGetSymbolsForAddresses
    
    @abstract       Map multiple addresses to their symbols.
    
    @discussion     This is a bulk version of QSymGetSymbolForAddress.  Using it, 
                    you can map multiple aaddress to their symbols in one 
                    function call.
                    
                    This routine is equivalent to calling QSymGetSymbolForAddress 
                    count times, each time passing corresponding entries of the 
                    addrs and symbolInfos arrays.
    
                    IMPORTANT: Failing to map an address to a symbol will not 
                    cause this routine to fail.  Rather, the corresponding entry 
                    in symbolInfos will have a symbol type of kQSymNoSymbol.

    @param symRef   A valid symbols object.

    @param count    The number of entries in the addrs and symbolInfos arrays.

    @param symbolNames
                    The array of addresses to map.  Each element is equivalent 
                    to the addr parameter of QSymGetSymbolForAddress.

    @param symbolInfos
                    An array of symbol information structures.  Each element is 
                    equivalent to the symbolInfo parameter of QSymGetSymbolForAddress. 

                    If the mapping failed for a particular element, the symbolType 
                    field will be kQSymNoSymbol

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QSymGetSymbolsForAddresses(
    QSymbolsRef         symRef,
    size_t              count,
    QTMAddr             addrs[],
    QSymSymbolInfo      symbolInfos[]
);

#ifdef __cplusplus
}
#endif

#endif
