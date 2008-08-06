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
 *  QTaskMemory.h
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
    File:       QTaskMemory.h

    Contains:   Task memory access abstraction.

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

$Log: QTaskMemory.h,v $
Revision 1.1  2007/03/02 12:20:40         
First checked in.


*/

#ifndef _QTASKMEMORY_H
#define _QTASKMEMORY_H

/////////////////////////////////////////////////////////////////

#include <stdbool.h>

// Put <mach/mach.h> inside extern "C" guards for the C++ build 
// because the Mach header files don't always have them.

#if defined(__cplusplus)
	extern "C" {
#endif

#include <mach/mach.h>

#if defined(__cplusplus)
	}
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
    extern "C" {
#endif

/*!
    @header         QTaskMemory.h
    
    @abstract       Abstraction layer for reading another process's memory.

    @discussion     This module lets you read memory from another task.  It's implemented 
                    using Mach APIs that are designed for that very job.  The purpose of 
                    this module is to make those routines easier to call, handling the 
                    various nasty compatibility problems that arise.  See the comments 
                    inside "QTaskMemory.c" for the gory details.
                    
                    When using this module, keep in mind that addresses within a remote 
                    task are represented by an unsigned 64-bit integer (QTMAddr).  This 
                    is necessary because it lets a 32-bit tool examine a 64-bit task.
                    -- Changed for BOINC: See comments in QTaskMemory.c

                    Because of oddities in the underlying infrastructure (specifically, 
                    mach_vm_read does odd things if you ask for a size of zero -- it 
                    returns KERN_SUCCESS, addrLocal == 0, bytesRead == 0), we don't 
                    allow clients to request a size of zero.
*/

/*!
    @functiongroup  Main Interface
*/
/////////////////////////////////////////////////////////////////
#pragma mark ***** Main Interface

/*!
    @typedef        QTMAddr
    
    @abstract       Represents an address within a task.
    
    @discussion     This type represents an address within a task.  We can't just 
                    use use (void *) because we need to be able to represent 64-bit 
                    addresses, even if we're built 32-bit.
*/
#if TARGET_CPU_X86 || TARGET_CPU_X86_64
typedef mach_vm_address_t QTMAddr;
#else
typedef uint64_t QTMAddr;
#endif

/*!
    @typedef        QTMOffset
    
    @abstract       Represents a byte offset between two QTMAddrs.
    
    @discussion     This type represents a byte offset between two QTMAddrs.  We 
                    can't just use use ptrdiff_t or size_t because we need to be 
                    able to represent a 64-bit offset, even if we're built 32-bit.
*/
#if TARGET_CPU_X86 || TARGET_CPU_X86_64
typedef mach_vm_offset_t  QTMOffset;
#else
typedef uint64_t  QTMOffset;
#endif

/*!
    @function       QTMRead
    
    @abstract       Reads memory from a task.
    
    @discussion     Read size bytes from addrWithinTask of the task to the buffer 
                    specified by addrLocal.  There are no partial results; you 
                    either get size bytes of data or an error.

    @param task     Must be the name of a valid send right for the task control 
                    port of the task whose memory you want to read; mach_task_self 
                    is just fine.

    @param addrWithinTask
                    The address within that task of the memory you want to read.

    @param size     The number of bytes that you want to read; there must be this 
                    many bytes available in the buffer pointed to by addrLocal.  
                    It must not be zero.

    @param addrLocal
                    The address of the buffer into which you want to read the bytes.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int  QTMRead(task_t task, QTMAddr addrWithinTask, size_t size, void *addrLocal);

/*!
    @function       QTMReadAllocated
    
    @abstract       Allocates a buffer and reads memory from a task into it.
    
    @discussion     Read size bytes from addrWithinTask of the task to a newly 
                    allocated buffer, and return a pointer to that buffer in *bufPtr.  
                    The client is responsible for freeing the buffer by calling QTMFree.

    @param task     Must be the name of a valid send right for the task control 
                    port of the task whose memory you want to read; mach_task_self 
                    is just fine.

    @param addrWithinTask
                    The address within that task of the memory you want to read.

    @param size     The number of bytes that you want to read.  It must not be zero.

    @param bufPtr   On entry, bufPtr must not be NULL and *bufPtr must be NULL.
                    On success, *bufPtr will point to a buffer of at least size bytes.  
                    You must free the buffer by calling QTMFree.
                    On failure, *bufPtr will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.

*/
extern int  QTMReadAllocated(task_t task, QTMAddr addrWithinTask, size_t size, const void **bufPtr);

/*!
    @function       QTMRemap
    
    @abstract       Maps memory from a task into your address space.
    
    @discussion     Maps size bytes of memory from addrWithinTask to the current 
                    task and returns the address of the newly mapped data in *bufPtr.  
                    The client is responsible for freeing the buffer by calling 
                    QTMFree.

    @param task     Must be the name of a valid send right for the task control 
                    port of the task whose memory you want to map; mach_task_self 
                    is just fine.

    @param addrWithinTask
                    The address within that task of the memory you want to map.

    @param size     The number of bytes that you want to map.  It must not be zero.

    @param bufPtr   On entry, bufPtr must not be NULL and *bufPtr must be NULL.
                    On success, *bufPtr will point to a buffer of at least size bytes.  
                    You must free the buffer by calling QTMFree.
                    On failure, *bufPtr will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.

*/
extern int  QTMRemap(task_t task, QTMAddr addrWithinTask, size_t size, const void **bufPtr);
    
/*!
    @function       QTMFree
    
    @abstract       Free data returned by QTMReadAllocated or QTMRemap.
    
    @discussion     Free data returned by QTMReadAllocated or QTMRemap.

    @param buf      The buffer to free.  This may be NULL, in which case the routine 
                    does nothing.

    @param size     The size of the buffer to free.  If buf is NULL, this value 
                    is ignored.  If buf is not NULL, this must bee the same value 
                    as passed to the routine that allocated the memory (and hence 
                    must not be 0).
*/
extern void QTMFree(const void *buf, size_t size);

/*!
    @functiongroup  Utilities
*/
/////////////////////////////////////////////////////////////////
#pragma mark ***** Utilities

/*!
    @function       QTMErrnoFromMachError
    
    @abstract       Converts a Mach error code to a errno-style error code.
    
    @discussion     Various other modules call Mach directly for various reasons, 
                    and these need to translate Mach-style errors into errno-style 
                    errors.  This module already has a helper routine to do that 
                    job, so I export it.

                    IMPORTANT:
                    This routine can still return errors outside of the errno range.  
                    What it does is as follows:

                     1. If the Mach error is something obvious that we recognise, 
                        map it to a specific errno-style error.

                     2. Failing that, if the Mach error is outside of the errno 
                        range (and thus it's not ambiguous as to whether the error 
                        is a Mach error or an errno error), just return it.

                     3. For unrecognised Mach errors within the errno range, just 
                        return EINVAL.

                    The upshot is that if an error is outside of the errno range 
                    [0..ELAST], you should treat it as a Mach error.

    @param kr       A Mach error code.

    @result         An errno-style error code.
*/
extern int QTMErrnoFromMachError(kern_return_t kr);

/*!
    @enum           QTMDarwinOSMajor
    
    @abstract       Constants for Darwin OS release major values.
    
    @constant kQTMDarwinOSMajorForMacOSX102
                    The Darwin OS release major value for Mac OS X 10.2.x.
    
    @constant kQTMDarwinOSMajorForMacOSX103
                    The Darwin OS release major value for Mac OS X 10.3.x.
    
    @constant kQTMDarwinOSMajorForMacOSX104
                    The Darwin OS release major value for Mac OS X 10.4.x.
*/
enum QTMDarwinOSMajor {
    kQTMDarwinOSMajorForMacOSX102 = 6,
    kQTMDarwinOSMajorForMacOSX103 = 7,
    kQTMDarwinOSMajorForMacOSX104 = 8
};

/*!
    @function       QTMGetDarwinOSRelease
    
    @abstract       Returns the Darwin OS release numbers.
    
    @discussion     Returns the Darwin OS release numbers for the current 
                    system.  It gets these values by calling uname and parsing 
                    the results.  For example, the major/minor/bug values for 
                    Mac OS X 10.4.8 will be 8/8/x (where x is dependent on the 
                    particular release).
                    
                    This is useful in environments, like a 64-bit process running 
                    on Mac OS X 10.4.x, where you can't get gestaltSystemVersion.

    @param majorPtr May be NULL, in which case no value is return.  Otherwise, 
                    on entry, *majorPtr is ignored and, on success, *majorPtr 
                    will be set to the Darwin OS release major component.

    @param minorPtr May be NULL, in which case no value is return.  Otherwise, 
                    on entry, *minorPtr is ignored and, on success, *minorPtr 
                    will be set to the Darwin OS release minor component.

    @param bugPtr   May be NULL, in which case no value is return.  Otherwise, 
                    on entry, *bugPtr is ignored and, on success, *bugPtr 
                    will be set to the Darwin OS release bug fix component.

    @result         An errno-style error number.
*/
extern int QTMGetDarwinOSRelease(int *majorPtr, int *minorPtr, int *bugPtr);
	// Get the Darwin OS release using uname.  I can't use gestaltSystemVersion 
	// because it's part of CoreServices, and CoreServices is not available 
	// to 64-bit programs on Mac OS X 10.4.x.

/*!
    @function       QTMTaskIs64Bit
    
    @abstract       Determines whether a task is 64-bit.
    
    @discussion     Returns true if the specified task is 64-bit.

    @param task     Must be the name of a valid send right for the task control 
                    port of the task; mach_task_self should work just fine.

    @result         Returns true if the task is 64-bit and false otherwise.
                    
                    If something goes wrong, you get the default value of false.
*/
extern bool QTMTaskIs64Bit(task_t task);

/*!
    @function       QTMTaskIsNative
    
    @abstract       Determines whether a task is native.
    
    @discussion     Returns false if the specified task is being run using Rosetta.

    @param task     Must be the name of a valid send right for the task control 
                    port of the task; mach_task_self should work just fine.

    @result         Returns false if the task is being run by Rosetta and false 
                    otherwise.
                    
                    If something goes wrong, you get the default value of true.
*/
extern bool QTMTaskIsNative(task_t task);

    // I'm not ever going to try to explain how this works (-:
    //
    // Apple doesn't generally recommend, and hence doesn't officially document, 
    // the Mach API.  If you search the web for "vm_region", you'll find our 
    // unofficial documentation.

#ifdef __cplusplus
}
#endif

#endif
