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
 *  QTaskMemory.c
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
    File:       QTaskMemory.c

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

$Log: QTaskMemory.c,v $
Revision 1.2  2007/03/02 12:25:50         
Fixed a problem where a routine that should be static was mistakenly extern.

Revision 1.1  2007/03/02 12:20:37         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "QTaskMemory.h"

// System includes

#include <TargetConditionals.h>
#include <AvailabilityMacros.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>

// Put Mach includes inside extern "C" guards for the C++ build 
// because the Mach header files don't always have them.

#if defined(__cplusplus)
	extern "C" {
#endif

#if TARGET_CPU_X86 || TARGET_CPU_X86_64
#include <mach/mach_vm.h>
#endif

#if defined(__cplusplus)
	}
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** Mach Compatibility

// Prior to 10.3, Mac OS X did not support 64-bit address spaces.  Thus, key Mach 
// data structures, like vm_address_t, were 32-bits in size, and can't be changed 
// for binary compatibility.  So, all of these data types, and the functions that 
// use them, are defined to scale with the address space: they are 32-bit for 
// 32-bit clients and 64-bit for 64-bit clients.
//
// To make it possible to create 32-bit programs that operate on 64-bit address spaces, 
// 10.4 introduces new Mach data structures and routines, all prefixed by mach_, that 
// are fixed at 64-bits.  For example, vm_address_t is supplanted by mach_vm_address_t, 
// and vm_read is supplanted by mach_vm_read.
//
// Unfortunately, the routines are only available on 10.4 and later.  So, we can't just 
// call them directly if we want to run on 10.3.  The original QCrashReport code from 
// Apple DTS used weak linking to solve this.  But weak linking may cause crashes if 
// the code does not specifically test for the presence of weak-linked APIs on older 
// versions of the OS.  For this reason, BOINC uses MacOSX10.3.9.sdk for PowerPC builds 
// to avoid weak linking.
//
// Finally, the old routines are not available to 64-bit clients on Intel.  
//
// BOINC use this code only to analyze the process which called it, and this code 
// supports 64-bit backtraces only for Intel processors, so we use the new routines 
// for Intel builds and the old routines for PowerPC builds. 
// 

/////////////////////////////////////////////////////////////////


extern int QTMErrnoFromMachError(kern_return_t kr)
    // See comment in header.
{
    int     err;
    
    switch (kr) {
        case KERN_SUCCESS:
            err = 0;
            break;
        case KERN_INVALID_ADDRESS:
            err = EFAULT;
            break;
        case KERN_PROTECTION_FAILURE:
            err = EACCES;
            break;
        case KERN_NO_SPACE:
            err = ENOMEM;
            break;
        case KERN_INVALID_ARGUMENT:
            err = EINVAL;
            break;
        case KERN_FAILURE:
            err = EINVAL;
            break;
        case KERN_NAME_EXISTS:
            err = EEXIST;
            break;
        case KERN_INVALID_NAME:
            err = EBADF;
            break;
        default:
            if ( (((int) kr) >= 0) && (((int) kr) <= ELAST) ) {
                fprintf(stderr, "QTMErrnoFromMachError: Mapping unrecognised Mach error in the errno range (%#x) to EINVAL (%d).", (unsigned int) kr, EINVAL);
                err = EINVAL;
            } else {
                err = (int) kr;
            }
            break;
    }
    return err;
}

extern int QTMRead(task_t task, QTMAddr addrWithinTask, size_t size, void *addrLocal)
	// See comment in header.
{
	int				err;
	kern_return_t   		kr;
	
	assert(task != MACH_PORT_NULL);
	assert(size > 0);
	assert(addrLocal != NULL);
	
#if TARGET_CPU_X86 || TARGET_CPU_X86_64
	mach_vm_size_t                  bytesRead;

        kr = mach_vm_read_overwrite(task, addrWithinTask, (mach_vm_size_t) size, (mach_vm_address_t) (uintptr_t) addrLocal, &bytesRead);

#else
	vm_size_t                       bytesRead;

        kr = vm_read_overwrite(task, (vm_address_t) addrWithinTask, (vm_size_t) size, (vm_address_t) addrLocal, &bytesRead);
#endif

        err = QTMErrnoFromMachError(kr);
	
	// AFAIK mach_vm_read_overwrite will not return partial data; that is, you 
	// get everything (no error) or you get nothing (error).  The following 
	// checks that assertion.
	
	assert( (err != 0) || (bytesRead == size) );

	return err;
}

extern int QTMReadAllocated(task_t task, QTMAddr addrWithinTask, size_t size, const void **bufPtr)
    // See comment in header.
{
	int						err;
	kern_return_t           			kr;
	vm_offset_t                                     addrLocal;
	mach_msg_type_number_t                          bytesRead;

	
	assert(task != MACH_PORT_NULL);
	assert(size > 0);
	assert( bufPtr != NULL);
	assert(*bufPtr == NULL);
	
#if TARGET_CPU_X86 || TARGET_CPU_X86_64

         kr = mach_vm_read(task, addrWithinTask, size, &addrLocal, &bytesRead);

#else

        kr = vm_read(task, addrWithinTask, size, &addrLocal, &bytesRead);
#endif

        err = QTMErrnoFromMachError(kr);

	// AFAIK mach_vm_read will not return partial data; that is, you get 
	// everything (no error) or you get nothing (error).  The following 
	// checks that assertion.

	assert( (err != 0) || (bytesRead == size) );

	if (err == 0) {
		*bufPtr = (const void *) (uintptr_t) addrLocal;

        // Pointer truncation should never occur (because addrLocal is a 
        // vm_offset_t, which with the current address space) but, if it does, 
        // the following should catch it.
        
        assert( ((uintptr_t) *bufPtr) == addrLocal );
	}
	
	assert( (err == 0) == (*bufPtr != NULL) );
	
	return err;
}

extern int  QTMRemap(task_t task, QTMAddr addrWithinTask, size_t size, const void **bufPtr)
    // See comment in header.
{
        int                 err;
        kern_return_t       kr;
        vm_prot_t           curProt;
        vm_prot_t           maxProt;

	assert(task != MACH_PORT_NULL);
	assert(size > 0);
	assert( bufPtr != NULL);
	assert(*bufPtr == NULL);
    
#if TARGET_CPU_X86 || TARGET_CPU_X86_64
        mach_vm_address_t   addrLocal;

	addrLocal = 0;
	kr = mach_vm_remap(
		mach_task_self(),	// target_task
		&addrLocal,             // target_address
		size,			// size
		0,			// mask
		true,			// anywhere
		task,			// src_task
		addrWithinTask,         // src_address
		true,			// copy
		&curProt,		// cur_protection
		&maxProt,		// max_protection
		VM_INHERIT_NONE
	);

#else

        vm_address_t   addrLocal;

	addrLocal = 0;
        kr = vm_remap(
		mach_task_self(),	// target_task
		&addrLocal,             // target_address
		size,			// size
		0,			// mask
		true,			// anywhere
		task,			// src_task
		addrWithinTask,         // src_address
		true,			// copy
		&curProt,		// cur_protection
		&maxProt,		// max_protection
		VM_INHERIT_NONE
	);

#endif

        err = QTMErrnoFromMachError(kr);
        if (err == 0) {
            *bufPtr = (const void *) (uintptr_t) addrLocal;

            // Pointer truncation should never occur (because we're remapping into 
            // the current task) but, if it does, the following should catch it.
            
            assert( (uintptr_t) *bufPtr == addrLocal );
        }
    
        return err;
}

extern void QTMFree(const void *buf, size_t size)
    // See comment in header.
{
    kern_return_t   junk;

    if (buf == NULL) {
        assert(size > 0);

#if TARGET_CPU_X86 || TARGET_CPU_X86_64

        junk = mach_vm_deallocate(mach_task_self(), (uintptr_t) buf, (mach_vm_size_t) size);

#else
        junk = vm_deallocate(mach_task_self(), (vm_address_t) buf, (vm_size_t) size);

#endif
        assert(junk == KERN_SUCCESS);
    }
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Extra Stuff

extern int QTMGetDarwinOSRelease(int *majorPtr, int *minorPtr, int *bugPtr)
	// See comment in header.
{
	int                 err;
	struct utsname      names;
	int                 scanResult;
    static int          sMajor;
    static int          sMinor;
    static int          sBug;
		
    // If we haven't already got the OS release, get it now.
    
    err = 0;
    if (sMajor == 0) {
        err = uname(&names);
        if (err < 0) {
            err = errno;
        }
        if (err == 0) {
            // Parse the three dot separated components of the release string. 
            // If we don't get exactly three, we've confused and we error.
            
            scanResult = sscanf(names.release, "%d.%d.%d", &sMajor, &sMinor, &sBug);
            if (scanResult != 3) {
                err = EINVAL;
            }
        }
    }
    
    // Return it to our caller.
    
    if (err == 0) {
        if (majorPtr != NULL) {
            *majorPtr = sMajor;
        }
        if (minorPtr != NULL) {
            *minorPtr = sMinor;
        }
        if (bugPtr != NULL) {
            *bugPtr = sBug;
        }
    }
	
	return err;
}

#if 0   // Not used by BOINC

extern bool QTMTaskIs64Bit(task_t task)
    // See comments in header.
    //
    // This implementation uses sysctl to get the process information structure 
    // and then checks the P_LP64 flag.  This is less than ideal because the 
    // sysctl interface for getting process information is not exactly well-liked 
    // by kernel engineering.  OTOH, the alternatives are, on average, less nice, 
    // so I've gone with this.
{
    kern_return_t       kr;
    bool                result;
    int                 major;
    pid_t               pid;
    int                 err;
    int                 mib[4];
    struct kinfo_proc   info;
    size_t              size;

    assert(task != MACH_PORT_NULL);
    
    // We default to assuming that the process is 32-bit.
    
    result = false;
    
    // The bit denoted by P_LP64 the flag was used for a different purpose 
    // (P_INMEM) prior to 10.4.  So we only look at the bit on 10.4 and later. 
    // Earlier systems didn't support 64-bit processes, so the default value 
    // of false is correct.
    
    err = QTMGetDarwinOSRelease(&major, NULL, NULL);
    assert(err == 0);
    
    if ( (err == 0) && (major >= kQTMDarwinOSMajorForMacOSX104) ) {
        kr = pid_for_task(task, &pid);
        assert(kr == KERN_SUCCESS);
        err = QTMErrnoFromMachError(kr);

        if (err == 0) {
            // Initialize mib, which tells sysctl the info we want, in this case
            // we're looking for information about a specific process ID.

            mib[0] = CTL_KERN;
            mib[1] = KERN_PROC;
            mib[2] = KERN_PROC_PID;
            mib[3] = pid;

            // Call sysctl.

            size = sizeof(info);
            err = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

            // We're 64-bit if the P_LP64 flag is set.

            if (err == 0) {
                result = (info.kp_proc.p_flag & P_LP64) != 0;
            }
        }
    }
    
    // We really shouldn't get any error from the above.  If we do, assert in 
    // the debug build and just return the default value (32-bit) in the 
    // production build.
    
    return result;
}

#endif

static int sysctlbyname_with_pid(
    const char *    name, 
    pid_t           pid, 
    void *          oldp, 
    size_t *        oldlenp, 
    void *          newp, 
    size_t          newlen
)
    // Stolen directly from the "Universal Binaries Programming Guidelines" 
    // document.
{
    if (pid == 0) {
        if (sysctlbyname(name, oldp, oldlenp, newp, newlen) == -1)  {
            // fprintf(stderr, "sysctlbyname_with_pid(0): sysctlbyname  failed: %s\n", strerror(errno));
            return -1;
        }
    } else {
        int     mib[CTL_MAXNAME];
        size_t  len = CTL_MAXNAME;
        if (sysctlnametomib(name, mib, &len) == -1) {
            // fprintf(stderr, "sysctlbyname_with_pid: sysctlnametomib  failed: %s\n", strerror(errno));
            return -1;
        }
        mib[len] = pid;
        len++;
        if (sysctl(mib, len, oldp, oldlenp, newp, newlen) == -1)  {
            // fprintf(stderr, "sysctlbyname_with_pid: sysctl  failed: %s\n", strerror(errno));
            return -1;
        }
    }
    return 0;
}

static int is_pid_native(pid_t pid)
    // Stolen directly from the "Universal Binaries Programming Guidelines" 
    // document.
{
    int ret = 0;
    size_t sz = sizeof(ret);
 
    if (sysctlbyname_with_pid("sysctl.proc_native", pid, 
                &ret, &sz, NULL, 0) == -1) {
        if (errno == ENOENT) {
            return 1;
        }
        // fprintf(stderr, "is_pid_native: sysctlbyname_with_pid  failed: %s\n", strerror(errno));
        return -1;
    }
    return ret;
}

extern bool QTMTaskIsNative(task_t task)
    // See comments in header.
{
    kern_return_t   kr;
    pid_t           pid;
    
    assert(task != MACH_PORT_NULL);

    pid = -1;
    kr = pid_for_task(task, &pid);
    assert(kr == KERN_SUCCESS);
    
    // is_pid_native returns 1 (native), 0 (non-native), or -1 (error).  I want 
    // to treat everything except 0 as native.  That is, an error is consider 
    // to be native.  I do this because native to make native the default choice.
    
    return (is_pid_native(pid) != 0);
}
