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
 *  QBacktrace.c
 *
 */
 
/* This is part of a backtrace generator for boinc project applications.  
*
* Adapted from Apple Developer Technical Support Sample Code QCrashReport
*
* This code handles Mac OS X 10.3.x through 10.4.9.  It may require some 
* adjustment for future OS versions; see the discussion of _sigtramp and 
* PowerPC Signal Stack Frames below.
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
    File:       QBacktrace.c

    Contains:   Code for generating backtraces.

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

$Log: QBacktrace.c,v $
Revision 1.2  2007/03/02 13:00:08         
Quieten some warnings.

Revision 1.1  2007/03/02 12:19:49         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "QBacktrace.h"

// Mac OS Interfaces

#include <TargetConditionals.h>
#include <AvailabilityMacros.h>

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <mach-o/arch.h>

#if defined(__cplusplus)
	extern "C" {
#endif

// Some extra Mach interfaces that we don't need in the public header.
// Again, we need C++ guards.

// We want both PowerPC and Intel thread state information.
// By default, the system only gives us the one that's appropriate 
// for our machine.  So we include both here.

#include <mach/ppc/thread_status.h>
#include <mach/i386/thread_status.h>

#if defined(__cplusplus)
	}
#endif

#include "QSymbols.h"

/////////////////////////////////////////////////////////////////

// A new architecture will require substantial changes to this file.

#if ! (TARGET_CPU_PPC || TARGET_CPU_PPC64 || TARGET_CPU_X86 || TARGET_CPU_X86_64)
	#error QBacktrace: What architecture?
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** Architecture Specification

// This is an extra flag for the flags field of the QBTFrame structure.
// It is true if the symbol field of that structure was allocated by 
// QBacktrace and should be disposed by QBacktraceDisposeSymbols.

enum {
    kQBTFrameSymbolNeedsDisposeMask = 0x0100
};

typedef struct QBTContext QBTContext;
	// forward declaration

// Architecture Callbacks -- Called by the core to do architecture-specific tasks.

typedef int  (*QBTHandleLeafProc)(QBTContext *context, QTMAddr *pcPtr, QTMAddr *fpPtr);
	// This callback is called by the core to start a backtrace. 
	// It should extract the first PC and frame from the thread state 
	// in the context and return them to the core.  Also, if the 
	// routine detects a frameless leaf routine, it should add a 
	// dummy frame for that routine (by calling AddFrame).
	//
	// On entry, context will be a valid context (as determined by QBTContextIsValid).
	// On entry, pcPtr will not be NULL.
	// On entry, fpPtr will not be NULL.
	// Returns an errno-style error code.
	// On success, *pcPtr must be the PC of the first non-leaf frame.
	// On success, *fpPtr must be the frame pointer of the first non-leaf frame.

typedef bool (*QBTValidPCProc)(QBTContext *context, QTMAddr pc);
	// This callback is called by the core to check whether a PC address 
	// is valid.  This is architecture-specific; for example, on PowerPC a 
	// PC value must be a multiple of 4, whereas an Intel an instruction 
	// can start at any address.
	//
	// At a minimum, an implementation is expected to check the PC's alignment 
	// and read the instruction at the PC.
	//
	// IMPORTANT:
	// The core code assumes that (QTMAddr) -1 is never a valid PC. 
	// If that isn't true for your architecture, you'll need to eliminate 
	// that assumption from the core.
	// 
	// On entry, context will be a valid context (as determined by QBTContextIsValid).
	// On entry, pc can be any value.
	// Returns true if the PC looks reasonably valid, false otherwise.

typedef int  (*QBTGetFrameNextPCProc)(QBTContext *context, QTMAddr thisFrame, QTMAddr nextFrame, QTMAddr *nextPCPtr);
	// This callback is called by the core to get the PC associated with 
	// the next frame.  This is necessary because different architectures 
	// store the PC in different places.  Specifically, PowerPC stores 
	// the PC of a frame in that frame, whereas Intel stores the PC of 
	// a frame in the previous frame (that is, the PC value in the frame 
	// is actually a return address).
	//
	// On entry, context will be a valid context (as determined by QBTContextIsValid).
	// On entry, thisFrame will be a valid frame.
	// On entry, nextFrame will be the valid frame following thisFrame.
	// On entry, nextPCPtr will not be NULL.
	// Returns an errno-style error code.
	// On success, *nextPCPtr must be the PC associated with nextFrame.

typedef int  (*QBTCrossSignalFrameProc)(QBTContext *context, QTMAddr thisFrame, QTMAddr *nextPCPtr, QTMAddr *nextFramePtr);
	// This callback is called by the core when it detects a cross signal 
	// frame and wants to cross that frame in an architecture-specific 
	// manner.  The code gets a pointer to the cross-signal handler frame 
	// and is expected to return the PC and frame pointer of the first 
	// non-leaf frame on the other side.  Furthermore, it must detect if 
	// the first frame on the other side is a leaf frame and add a 
	// dummy frame for that routine (by calling AddFrame) before returning.
	//
	// An implementation does not have to check the validity of the 
	// returned PC and frame.  The core will do that for you.
	// 
	// On entry, context will be a valid context (as determined by QBTContextIsValid).
	// On entry, thisFrame will be a valid cross-signal handler frame.
	// On entry, nextPCPtr will not be NULL.
	// On entry, nextFramePtr will not be NULL.
	// Returns an errno-style error code.
	// On success, *nextPCPtr must be the PC of the next non-leaf frame.
	// On success, *nextFramePtr must be the PC of the next non-leaf frame.

// Architecture Specification Structure -- Aggregates all of the information 
// associated with a particular architecture.

struct QBTArchInfo {
    const char *                name;                       // just for debugging

	// Information to identify the architecture
	
    cpu_type_t					cputype;					// per NXGetLocalArchInfo in <mach-o/arch.h>
    cpu_subtype_t				cpusubtype;					// per NXGetLocalArchInfo in <mach-o/arch.h>, 0 for wildcard
	bool						is64Bit;

	// Misc information about the architecture
	
	QTMAddr                     frameAlignMask;				// mask to detect frame misalignment
															// FP & frameAlignMask must be 0 for a valid frame
	// Architecture-specific backtrace callbacks
	
	QBTHandleLeafProc           handleLeaf;					// described in detail above
	QBTValidPCProc              validPC;					// described in detail above
	QBTGetFrameNextPCProc       getFrameNextPC;				// described in detail above
	QBTCrossSignalFrameProc     crossSignalFrame;			// described in detail above
	
	// Specification of how to call thread_get_state
	
	thread_state_flavor_t		stateFlavor;
	mach_msg_type_number_t		stateCount;
};
typedef struct QBTArchInfo QBTArchInfo;

static const QBTArchInfo kArchitectures[];
	// forward declaration

static const QBTArchInfo * GetTaskArch(QMOImageRef qmoImage)
	// Returns a pointer to the architecture associated with the specific 
	// task, or NULL if it's an architecture we don't know about. 
{
	const QBTArchInfo *     result;
    const QBTArchInfo *     thisArch;
    bool                    is64Bit;
    cpu_type_t              cputype;
    cpu_subtype_t           cpusubtype;
	
    assert(qmoImage != NULL);
    
	result = NULL;

    // Get the architecture characteristics from the image.
    
    is64Bit    = QMOImageIs64Bit(qmoImage);
    cputype    = QMOImageGetCPUType(qmoImage);
    cpusubtype = QMOImageGetCPUSubType(qmoImage);

    // Look through the architecture array for an architecture that matches the 
    // target architecture.  Also, we prefer architectures with an exact CPU 
    // subtype match, but we'll accept those with a 0 CPU subtype.
    
    thisArch = &kArchitectures[0];
    while ( (thisArch->cputype != 0) && (result == NULL) ) {
        if (   (thisArch->cputype == cputype)
            && ((thisArch->cpusubtype == 0) || (thisArch->cpusubtype == cpusubtype))
            && (thisArch->is64Bit == is64Bit) 
           ) {
            result = thisArch;
        } else {
            thisArch += 1;
        }
    }

	return result;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Backtrace Core

// Memory Read Callback

typedef int (*QBTReadBytesProc)(QBTContext *context, QTMAddr src, void *dst, size_t size);
	// This function pointer is called by the core backtrace code 
	// when it needs to read memory.  The callback should do a safe 
	// read of size bytes from src into the buffer specified by 
	// dst.  By "safe" we mean that the routine should return an error 
	// if the read can't be done (typically because src is a pointer to 
	// unmapped memory).  It does not need to do any word size adjustment 
    // or byte swapping.
	//
	// On entry, context will be a valid context (as determined by QBTContextIsValid).
	// On entry, src can be any value.
	// On entry, dst will not be NULL.
	// On entry, size will be greater than 0.
	// Returns an errno-style error code.
	// On success, the routine has copied size bytes of data from the address src 
	// in the remote task to the address dst in the local task.
	// On error, the value at dst is unspecified.
	//
	// Note:
	// In previous versions of QBacktrace, I supported alternative ways to 
	// read bytes from the target.  For example, I could use the Carbon 
	// exception handler mechanism <CoreServices/MachineExceptions.h> to read 
	// data from the current task in a safe fashion.  This was useful because 
	// it would work on both Mac OS X and traditional Mac OS.  Now that I 
	// require Mac OS X and Mach-O, I can just use the Mach routines to do 
	// my reading.  However, I've left the abstraction layer in place, 
	// Just In Case (tm).

struct QBTContext {

	// Internal parameters that are set up by the caller 
	// of the core backtrace code.
	
	const QBTArchInfo *		arch;                   // architecture specific info
	task_t					task;                   // target task
    QSymbolsRef             symRef;                 // symbols object for target task
    bool                    createdSymRef;          // true if we created the symbols object
	thread_state_flavor_t	threadStateFlavor;		// flavor of thread state
	const void *            threadState;			// architecture-specific current thread state
														// for example, on Intel this is x86_thread_state32_t
	size_t					threadStateSize;		// size of threadState
	QBTReadBytesProc		readBytes;				// described in detail above

	// Stuff worked out internally by MachInitContextFromTask.
	
	bool					swapBytes;				// true if target and current task have different byte orders
	QTMAddr					sigTrampLowerBound;		// bounds of _sigtramp in target task
	QTMAddr					sigTrampUpperBound;
	
	// Parameters from client.
	
	QTMAddr                 stackBottom;            // bounds of stack in target task
	QTMAddr                 stackTop;
	QBTFrame *              frameArray;				// array contents filled out by core
	size_t			        frameArrayCount;        // size of frameArray
	size_t			        frameCountOut;			// returned by core
};

#if ! defined(NDEBUG)

    // Because QBTContextIsValid is only referenced by assert macros, and these 
    // are disabled by NDEBUG, we have to conditionalise this code based on 
    // NDEBUG lest we get a "defined but not used" warning.

    static bool QBTContextIsValid(const QBTContext *context)
    {
        return (context != NULL)
            && (context->arch != NULL)
            && (context->task != MACH_PORT_NULL)
            && (context->symRef != NULL)
            && ( (context->threadState == NULL) == (context->threadStateSize == 0) )
            && (context->readBytes != NULL)
            && (context->sigTrampLowerBound != 0)
            && (context->sigTrampLowerBound < context->sigTrampUpperBound)
            && (context->stackBottom <= context->stackTop)
            && ((context->frameArrayCount == 0) || (context->frameArray != NULL));
    }

#endif

static int BacktraceAdorn(QBTContext *context)
    // Add symbols to all of the backtrace frames referenced by the context. 
    // Uses the 'bulk' symbols-to-address routine, in the hope that this will 
    // be faster one day.  Also uses the kQBTFrameSymbolNeedsDisposeMask flag 
    // to tag each frame to indicate whether the client needs to dispose it.
    //
    // Note that failing to find symbol information for a particular address 
    // will not cause this routine to fail.  Rather, the only causes of failure 
    // are really bad things, like running out of memory.
{
    int                 err;
    size_t              frameCount;
    size_t              frameIndex;
    QTMAddr *           addrs;
    QSymSymbolInfo *    infos;
    
    assert(context != NULL);
    assert(context->symRef != NULL);
    assert(context->frameArray != NULL);
    assert(context->frameArrayCount != 0);
    assert(context->frameCountOut != 0);

    // Only do the frames that actually exist.
    
    frameCount = context->frameCountOut;
    if (frameCount > context->frameArrayCount) {
        frameCount = context->frameArrayCount;
    }
    assert(frameCount > 0);         // because of our pre-conditions
    
    // Allocate arrays for call QSymGetSymbolsForAddresses.
    
    addrs = (QTMAddr *)        calloc(frameCount, sizeof(*addrs));
    infos = (QSymSymbolInfo *) calloc(frameCount, sizeof(*infos));
    err = 0;
    if ((addrs == NULL) || (infos == NULL)) {
        err = ENOMEM;
    }

    // Set up the addrs array and call QSymGetSymbolsForAddresses.
    
    if (err == 0) {
        for (frameIndex = 0; frameIndex < frameCount; frameIndex++) {
            addrs[frameIndex] = context->frameArray[frameIndex].pc;
        }
        
        err = QSymGetSymbolsForAddresses(
            context->symRef,
            frameCount,
            addrs,
            infos
        );
    }
    
    // Place the symbol information into the output frames.
    //
    // For error handling to work, you must not error past this point.  Otherwise 
    // we break the post-condition that the user doesn't need to do anything on 
    // error.
    
    if (err == 0) {
        for (frameIndex = 0; frameIndex < frameCount; frameIndex++) {
            if (infos[frameIndex].symbolType != kQSymNoSymbol) {
                if (context->createdSymRef) {
                    const char *    libName;
                    
                    // Within this block, we ignore any failures; they just leave 
                    // the relevant string NULL, which the dispose routine can 
                    // handle.
                    
                    context->frameArray[frameIndex].symbol  = strdup(infos[frameIndex].symbolName);
                    
                    libName = QMOImageGetFilePath(infos[frameIndex].symbolImage);
                    if (libName != NULL) {
                        context->frameArray[frameIndex].library = strdup(libName);
                    }

                    // Tell the dispose routine that we need to look at this frame.
                    
                    context->frameArray[frameIndex].flags |= kQBTFrameSymbolNeedsDisposeMask;
                } else {
                    assert(infos[frameIndex].symbolImage != NULL);
                    context->frameArray[frameIndex].symbol  = infos[frameIndex].symbolName;
                    context->frameArray[frameIndex].library = QMOImageGetFilePath(infos[frameIndex].symbolImage);
                }
                context->frameArray[frameIndex].offset = infos[frameIndex].symbolOffset;
            }
        }
    }
    
    // Clean up.
    
    free(addrs);
    free(infos);
    
    return err;
}

static int ReadAddr(QBTContext *context, QTMAddr addr, QTMAddr *valuePtr)
	// Reads an address (that is, a pointer) from the target task, 
	// returning an error if the memory is unmapped.
	//
	// On entry, context will be a valid context (as determined by QBTContextIsValid).
	// On entry, addr can be any value.
	// On entry, valuePtr must not be NULL.
	// Returns an errno-style error code.
	// On success, *valuePtr will be the value of the pointer stored at addr in 
	// the target task.
{
	int			err;
	QTMAddr     value;
	
	assert(QBTContextIsValid(context));
	assert(valuePtr != NULL);
	
	if (context->arch->is64Bit) {

		// Read directly into value, and then swap all 8 bytes.
		
		err = context->readBytes(context, addr, &value, sizeof(value));
		if (err == 0) {
			if (context->swapBytes) {
				value = OSSwapInt64(value);
			}
		}
	} else {
		uint32_t	tmpAddr;
		
		// Read into a temporary address, swap 4 bytes, then copy that 
		// into value.  tmpAddr is unsigned, so we zero fill the top 
		// 32 bits.
		
		err = context->readBytes(context, addr, &tmpAddr, sizeof(tmpAddr));
		if (err == 0) {
			if (context->swapBytes) {
				tmpAddr = OSSwapInt32(tmpAddr);
			}
			value = tmpAddr;
		}
	}
	if (err == 0) {
		*valuePtr = value;
	}
	
	return err;
}

static void AddFrame(QBTContext *context, QTMAddr pc, QTMAddr fp, QBTFlags flags)
	// Adds a frame to the end of the output array with the 
	// value specified by pc, fp, and flags.
	//
	// On entry, context will be a valid context (as determined by QBTContextIsValid).
{
	// Only actually output the frame if the client supplied an array 
	// and we we haven't filled it up yet.
	
	assert(QBTContextIsValid(context));
	
	if ( (context->frameArray != NULL) && (context->frameCountOut < context->frameArrayCount) ) {
		QBTFrame *	frameOutPtr;

		frameOutPtr = &context->frameArray[context->frameCountOut];
		frameOutPtr->pc    = pc;
		frameOutPtr->fp    = fp;
		frameOutPtr->flags = flags;
	}
	
	// Always increment the frame count.
	
	context->frameCountOut += 1;	
}

static int BacktraceCore(QBTContext *context, size_t *frameCountPtr)
	// The core backtrace code.  This routine is called by all of the various 
	// exported routines.  It implements the core backtrace functionality. 
	// All of the parameters to this routine are contained within 
	// the context.  This routine traces back through the stack (using the 
	// readBytes callback in the context to actually read memory) creating 
	// a backtrace.
{
	int			err;
	QTMAddr		thisPC;
	QTMAddr		thisFrame;
	QTMAddr		lowerBound;
	QTMAddr		upperBound;
	bool		stopNow;
	
	assert(QBTContextIsValid(context));

    // Check that the contents of the frame array, if any, are all zero.
    // MachInitContextFromTask did this for us.  We need this to be done 
    // to maintain our post-condition, and it would be wasteful to do it twice.
    
    #if ! defined(NDEBUG)
        if (context->frameArray != NULL) {
            size_t          byteCount;
            size_t          byteIndex;
            const char *    byteBase;
            
            byteCount = context->frameArrayCount * sizeof(*context->frameArray);
            byteBase  = (const char *) context->frameArray;
            for (byteIndex = 0; byteIndex < byteCount; byteIndex++) {
                assert(byteBase[byteIndex] == 0);
            }
        }
    #endif
	
	lowerBound = context->stackBottom;
	upperBound = context->stackTop;
	if (upperBound == 0) {
		if (context->arch->is64Bit) {
			// This actually generates a theoretical off-by-one error (a fp of 
			// 0xFFFFFFFF FFFFFFFF is falsely considered invalid), but that's 
			// not a problem in practice.
			upperBound = (QTMAddr)0xFFFFFFFFFFFFFFFFLL;
		} else {
			upperBound = (QTMAddr)0x0000000100000000LL;
		}
	}
	
	// If you supply bounds, they must make sense.
	
	assert(upperBound >= lowerBound);

	// Handle any leaf frames, and also return to us the initial 
	// PC and FP.  A failure here is worthy of being reported as a total 
    // failure of the routine, so we allow err to propagate.

	assert(context->frameCountOut == 0);			// set up by memset in MachInitContextFromTask
	err = context->arch->handleLeaf(context, &thisPC, &thisFrame);
	
	// Handle the normal frames.
	
	if (err == 0) {
		stopNow = false;
		do {
			QBTFrame *  frameOutPtr;
			QBTFrame	tmpFrameOut;
			QTMAddr 	nextFrame;
			QTMAddr 	nextPC;
			
			// Output to a tmpFrameOut unless the client has supplied 
			// a buffer and there's sufficient space left in it.
			//
			// IMPORTANT:
			// You can't just add the frame information (possibly by calling 
			// AddFrame) at the end of this loop, because the crossSignalFrame 
			// callback may add its own frame, and we have to make sure that 
			// this frame is allocated before that one.
			
			if ( (context->frameArray != NULL) && (context->frameCountOut < context->frameArrayCount) ) {
				frameOutPtr = &context->frameArray[context->frameCountOut];
			} else {
				frameOutPtr = &tmpFrameOut;
			}

			// Record this entry.
			
			frameOutPtr->pc    = thisPC;
			frameOutPtr->fp    = thisFrame;
			frameOutPtr->flags = 0;
			
			// Now set the flags to indicate the validity of specific information. 
			
			// Check the validity of the PC.  Don't set the err here; a bad PC value 
			// does not cause us to quit the backtrace.
			
			if ( ! context->arch->validPC(context, thisPC) ) {
				frameOutPtr->flags |= kQBTPCBadMask;
			} else {
				// On PowerPC I used to report the address of the call, 
				// rather than the return address.  That was easy: I just 
				// decremented the returned PC by 4.  However, this is 
				// much harder on Intel, where instructions are of variable 
				// length.  So, I decided to do what Apple's tools do, 
				// and just report the return address.
			}
			
			// Check the validity of the frame pointer.  A bad frame pointer *does* 
			// cause us to stop tracing.
			
			if (	(thisFrame == 0) 
				 || (thisFrame & context->arch->frameAlignMask) 
				 || (thisFrame < lowerBound) 
				 || (thisFrame >= upperBound) 
			   ) {
				frameOutPtr->flags |= kQBTFrameBadMask;
				stopNow = true;
			}

			if ( ! stopNow ) {

				// Move to the next frame, either by crossing a signal handler frame 
				// or by the usual mechanism.
				
				if (	!(frameOutPtr->flags & kQBTPCBadMask) 
					  && ( thisPC >= context->sigTrampLowerBound ) 
					  && ( thisPC <  context->sigTrampUpperBound ) 
				   ) {

					// If this frame is running in _sigtramp, get nextPC and nextFrame 
					// by delving into the signal handler stack block.
                    
                    // While developing the various per-architecture cross signal 
                    // frame handlers, there are many cases where I want to look at 
                    // the stack in detail.  In these circumstances, running under 
                    // the debugger can be difficult because, for signals like 
                    // SIGSEGV, GDB will catch the signal and not let you continue 
                    // into the handler.  So I typically stop in the signal handler 
                    // and then attach with GDB.  This code let's me do it without 
                    // have to recompile.

#if 0       // Added for BOINC
                    #if !defined(NDEBUG)
                        {
                            static bool     sInited;
                            static bool     sStop;
                            const char *    envVar;
                            
                            if ( ! sInited ) {
                                envVar = getenv("QBACKTRACE_STOP_FOR_SIGNALS");
                                sStop = ( (envVar != NULL) && (atoi(envVar) != 0) );
                                sInited = true;
                            }
                            if (sStop) {
                                fprintf(stderr, "BacktraceCore: Waiting for debugger, pid = %ld\n", (long) getpid());
                                pause();
                            }
                        }
                    #endif
#endif
					frameOutPtr->flags |= kQBTSignalHandlerMask;
					err = context->arch->crossSignalFrame(context, thisFrame, &nextPC, &nextFrame);
                    
                    // If we get an error crossing the signal frame, we just stop. 
                    // The trace up to this point is probably OK.
                    
                    if (err != 0) {
                        stopNow = true;
                        err = 0;
                    }
				} else {
				
					// Read the next frame pointer.  A failure here causes us to quit 
					// backtracing.  Note that we set kQBTFrameBadMask in frameOutPtr 
					// because, if we can't read the contents of the frame pointer, the 
					// frame pointer itself must be bad.
					
					err = ReadAddr(context, thisFrame, &nextFrame);
					if (err != 0) {
						frameOutPtr->flags |= kQBTFrameBadMask;
						nextFrame = (QTMAddr) -1;
                        
                        stopNow = true;
                        err = 0;
					}
					
					// Also get the PC of the next frame, or set it to dummy value if 
					// there is no next frame or we can't get the PC from that frame.

					if (	(frameOutPtr->flags & kQBTFrameBadMask) 
						 || (context->arch->getFrameNextPC(context, thisFrame, nextFrame, &nextPC) != 0) 
					   ) {
						nextPC = (QTMAddr) -1;		// an odd value, to trigger above check on next iteration
					}
				}

				// Set up for the next iteration.
				
				if ( (err == 0) && ! stopNow ) {
                                        context->frameCountOut += 1;
                                        
					lowerBound = thisFrame;
					thisPC     = nextPC;
					thisFrame  = nextFrame;
				}
			}
		} while ( (err == 0) && ! stopNow );
	}

    // Adorn the backtrace with symbol information.
    
    if ( (err == 0) && (context->frameArray != NULL) && (context->frameArrayCount != 0) && (context->frameCountOut != 0) ) {
        err = BacktraceAdorn(context);
    }

    // Clean up.
    
    if (err != 0) {
        QBacktraceDisposeSymbols(context->frameArray, context->frameArrayCount);
        if (context->frameArray != NULL) {
            memset(context->frameArray, 0, context->frameArrayCount * sizeof(*context->frameArray));
        }
        context->frameCountOut = 0;
    }
    *frameCountPtr = context->frameCountOut;

	assert(QBTContextIsValid(context));
	
	return err;
}

#pragma mark ***** Mach Infrastructure

static int MachReadBytes(QBTContext *context, QTMAddr src, void *dst, size_t size)
	// A memory read callback for Mach.  This simply calls through to our 
    // QTaskMemory abstraction layer, which in turns calls mach_vm_read_overwrite.
	//
	// See the description of QBTReadBytesProc for information about 
	// the parameters.
{
	assert(QBTContextIsValid(context));
	assert(dst != NULL);
	assert(size > 0);

	return QTMRead(context->task, src, size, dst);
}

static void MachTermContext(QBTContext *context)
    // Clean up after a MachInitContext.  It's safe to call this even 
    // if MachInitContext fails.
{
    assert(context != NULL);
    
    if (context->createdSymRef) {
        QSymDestroy(context->symRef);
    }
}

static int MachInitContextFromTask(
	QBTContext *			context,
	task_t					task,
    cpu_type_t              cputype,
	QTMAddr                 stackBottom, 
	QTMAddr                 stackTop,
    QSymbolsRef             symRef,
	QBTFrame *              frameArray, 
	size_t					frameArrayCount
)
    // Initialise the backtrace context from numerous parameters, mostly supplied 
    // by the client.  Even if this fails, it's safe to call MachTermContext.
{
    int             err;
    QMOImageRef     qmoImage = 0;
    
    assert(context != NULL);
    assert(task != MACH_PORT_NULL);
	assert( ((stackBottom == 0) && (stackBottom == stackTop)) || (stackBottom < stackTop) );
	assert( (frameArrayCount == 0) || (frameArray != NULL) );
    
    // Clear the context first, to make it safe to call MachTermContext.
    
	memset(context, 0, sizeof(*context));

    // Stuff that can't fail
    
    // Zap the input array; the BacktraceCore requires this to make error 
    // handling easier.
    
    if (frameArray != NULL) {
        memset(frameArray, 0, frameArrayCount * sizeof(*frameArray));
    }
    
    // General stuff
    
	context->stackBottom     = stackBottom;
	context->stackTop        = stackTop;
	context->frameArray      = frameArray;
	context->frameArrayCount = frameArrayCount;

    // Platform specific stuff
    
	context->readBytes       = MachReadBytes;
	context->task            = task;

    // Stuff that might fail.
    
    // Create a symbols object for the task, if necessary, and use that to get 
    // some basic information about it.
    
    err = 0;
    if (symRef == NULL) {
        context->createdSymRef = true;
        err = QSymCreateFromTask(context->task, (context->task != mach_task_self()), cputype, &context->symRef);
    } else {
        context->symRef = symRef;
    }
    if (err == 0) {
        qmoImage = QSymGetExecutableImage(context->symRef);
        if (qmoImage == NULL) {
            err = EINVAL;
        }
    }
    if (err == 0) {
        context->swapBytes = QMOImageIsByteSwapped(qmoImage);

        context->arch = GetTaskArch(qmoImage);
        if (context->arch == NULL) {
            err = EINVAL;
        }
    }
    
    // Determine the address of _sigtramp.
    
    if (err == 0) {
        QSymSymbolInfo symInfo;
        QSymSymbolInfo nextSymInfo;
        
        err = QSymGetAddressForSymbol(context->symRef, "/usr/lib/libSystem.B.dylib", "__sigtramp", &symInfo);
        if (err == 0) {
            context->sigTrampLowerBound = symInfo.symbolValue;
        }
        if (err == 0) {
            err = QSymGetNextSymbol(context->symRef, &symInfo, &nextSymInfo);
            if (err == 0) {
                context->sigTrampUpperBound = nextSymInfo.symbolValue;
            } else {
                // If QSymGetNextSymbol fails, just take a guess.
                
                context->sigTrampUpperBound = context->sigTrampLowerBound + 256;
                err = 0;
            }
        }
    }

	assert( (err != 0) || QBTContextIsValid(context) );

    return err;
}

#pragma mark ***** CPU Specific

#pragma mark - PowerPC

/*	PowerPC Stack Frame Basics
	--------------------------
	
	
						Offset	Size	Purpose
						------	----	-------
	low memory
	fp == sp == r1 ->	0		X		pointer to next frame
						X		X		place to save CR
						2X		X		place to save LR
						3X		2X		reserved
						5X		X		place to save TOC (CFM only)
	high memory
	
						where X is the address size (4 bytes for 32-bits, 
						8 bytes for 64-bits)
					
	To get from one frame to the next, you have to indirect an offset 
	of 0.  To extract the PC from a frame (which, notably, is the 
	address of the code running in that frame, not a return address), you 
	have to indirect an offset of 2X bytes (8 or 16).
	
	There's enough commonality between 32- and 64-bit PowerPC architectures 
	that it's easy to handle them both with the same code.
*/

static bool PowerPCIsSystemCall(QBTContext *context, QTMAddr pc)
	// Using the PC from the thread state, walk back through 
	// the code stream for 3 instructions looking for a "sc" instruction. 
	// If we find one, it's almost certain that we're in a system call 
	// frameless leaf routine.
{
	int			err;
	bool		isSystemCall;
	int			count;
	uint8_t		inst[4];
	
	isSystemCall = false;
	count = 0;
	do {
		err = context->readBytes(context, pc, &inst, sizeof(inst));
		if (err == 0) {
			isSystemCall = (inst[0] == 0x44)		// PPC "sc" instruction
			            && (inst[1] == 0x00)		// PPC instructions are always big 
						&& (inst[2] == 0x00)		// endian, so we compare it byte at 
						&& (inst[3] == 0x02);		// time for endian neutrality
						
		}
		if ( (err == 0) && ! isSystemCall ) {
			count += 1;
			pc -= sizeof(inst);
		}
	} while ( (err == 0) && ! isSystemCall && (count < 3) );
	err = 0;
	
	return isSystemCall;
}

static int PowerPCHandleLeaf(QBTContext *context, QTMAddr *pcPtr, QTMAddr *framePtr)
	// This is the handleLeaf routine for the PowerPC 
	// architecture.  See the description of QBTHandleLeafProc 
	// for a detailed discussion of its parameters.
	//
	// The top most frame may be in a weird state because of the 
	// possible variations in the routine prologue.  There are a 
	// variety of combinations, such as:
	//
	// 1. a normal routine, with its return address stored in 
	//    its caller's stack frame
	//
	// 2. a system call routine, which is a leaf routine with 
	//    no frame and the return address is in LR
	//
	// 3. a leaf routine with no frame, where the return address 
	//    is in LR
	//
	// 4. a leaf routine with no frame that accesses a global, where 
	//    the return address is in r0
	//
	// 5. a normal routine that was stopped midway through 
	//    constructing its prolog, where the return address is 
	//    typically in r0
	//
	// Of these, 1 and 2 are most common, and they're the cases I 
	// handle.  General support for all of the cases requires the 
	// ability to accurately determine the start of the routine 
	// which is not something that I can do with my current 
	// infrastructure.
	//
	// Note that don't handle any cases where the return address is 
	// in r0, although r0 is available as part of the threadState 
	// if I need it in the future.
{
#ifdef __LP64__
    return EINVAL;
#else
	int		err;
	QTMAddr	pc;
	QTMAddr	lr;
    QTMAddr r1;
	
	// Get the pc and lr from the thread state.
	
    err = 0;
    switch (context->threadStateFlavor) {
        case PPC_THREAD_STATE:
            pc = ((const ppc_thread_state_t *) context->threadState)->srr0;
            lr = ((const ppc_thread_state_t *) context->threadState)->lr;
            r1 = ((const ppc_thread_state_t *) context->threadState)->r1;
            break;
        case PPC_THREAD_STATE64:
            pc = ((const ppc_thread_state64_t *) context->threadState)->srr0;
            lr = ((const ppc_thread_state64_t *) context->threadState)->lr;
            r1 = ((const ppc_thread_state64_t *) context->threadState)->r1;
            break;

        default:
            err = EINVAL;
            break;
    }

	// If we find that we're in a system call frameless leaf routine, 
	// add a dummy stack frame (with no frame, because the frame actually 
	// belows to frameArray[1]).

    if (err == 0) {
        if ( PowerPCIsSystemCall(context, pc) ) {

            AddFrame(context, pc, 0, kQBTFrameBadMask);

            pc = lr;
        }

        // Pass the initial pc and frame back to the caller.
        
        *pcPtr    = pc;
        *framePtr = r1;
    }

	return err;
#endif
}

static bool  PowerPCValidPC(QBTContext *context, QTMAddr pc)
	// This is the validPC routine for the PowerPC 
	// architecture.  See the description of 
	// QBTValidPCProc for a detailed discussion 
	// of its parameters.
	//
	// PowerPC instructions must be word aligned.  Also, I check that 
	// it's possible to read the instruction.  I don't do anything 
	// clever like check that the resulting value is a valid instruction.
{
	uint32_t	junkInst;
	
	return ((pc & 0x03) == 0) && (context->readBytes(context, pc, &junkInst, sizeof(junkInst)) == 0);
}

static int PowerPCGetFrameNextPC(QBTContext *context, QTMAddr thisFrame, QTMAddr nextFrame, QTMAddr *nextPCPtr)
	// This is the getFrameNextPC routine for the PowerPC 
	// architecture.  See the description of 
	// QBTGetFrameNextPCProc for a detailed discussion 
	// of its parameters.
{
    #pragma unused(thisFrame)
    #pragma unused(nextFrame)
	QTMAddr	offset;
	
	if ( context->arch->is64Bit ) {
		offset = 16;
	} else {
		offset = 8;
	}
	
	return ReadAddr(context, nextFrame + offset, nextPCPtr);
}

/*	PowerPC Signal Stack Frames
	---------------------------
	In the current Mac OS X architecture, there is no guaranteed reliable 
	way to backtrace a PowerPC signal stack frame.  The problem is that the 
	kernel pushes a variable amount of data on to the stack when it invokes the 
	user space signal trampoline (_sigtramp), and the only handle to the 
	information about how much data was pushed is passed in a register 
	parameter to _sigtramp.  _sigtramp stashes that value away in a 
	non-volatile register.  So, when _sigtramp calls the user-supplied 
	signal handler, there's no way to work out where that register 
	ends up being saved.
	
	Thus, we devolve into guesswork.  It turns out that the offset from 
	the stack of the kernel data to the information we need (the place 
	where the interrupted thread's registers were stored) is a (relatively) 
	constant for any given system release.  So, we can just simply add the 
	appropriate offset to the frame pointer and grab the data we need.
	
	On recent systems (10.3 and later) this fails if the signal handle 
	requests 'dual contexts', that is, it requests both 32- and 64-bit 
	PowerPC registers.  In that case, the size of the pushed data changes, 
	and that affects the relative alignment of the data and the stack 
	pointer, and things break.  I don't know of any way to work around 
	this <rdar://problem/4411774>.
	
	Finally, these constant vary from release to release. 
	This code handles the significant cases that I know about (Mac OS X 10.1.x 
	and earlier, Mac OS X 10.2, and Mac OS 10.3 and later), but there's no 
	guarantee that this offset won't change again in the future.

	When the kernel invokes the user space signal trampoline, it pushes 
	the following items on to the stack.
	
	Mac OS X 10.1.x
	---------------
					Size	Purpose
					----	-------
	low memory
					0x030   bytes for C linkage
					0x040 	bytes for saving PowerPC parameters
					0x0c0	ppc_saved_state
					0x110	ppc_float_state
					0x018	struct sigcontext
					0x0e0	red zone
	high memory
					The previous frame's SP is at offset 0x00C within 
					ppc_saved_state, which makes it equal to 
					0x030 + 0x040 + 0x00C, or 0x07C.  The offset to 
					the previous PC (0x84) follows from that.
				                   			 
	Mac OS X 10.2.x
	---------------
					Size	Purpose
					----	-------
	low memory
					0x030   bytes for C linkage
					0x040 	bytes for saving PowerPC parameters
					0x008	alignment padding
					0x408   struct mcontext, comprised of:
								 0x020 ppc_exception_state_t
								 0x0A0 ppc_thread_state_t
								 0x108 ppc_float_state_t
								 0x240 ppc_vector_state_t
					0x040	siginfo_t
					0x020	ucontext
					0x0e0	red zone
	high memory	
					The previous frame's SP is at offset 0x00C within 
					ppc_thread_state_t, which it equal to 
					0x030 + 0x040 + 0x008 + 0x020 + 0x00C, or 0x0A4. 
					The offsets to the previous PC and LR (0x98 and 0x128) 
					follow from that.

	Mac OS X 10.3.x and 10.4.x
	--------------------------
                    
                    Size (process/hardware/requested)               Purpose
                    ---------------------------------               -------
					32/32/x     32/64/32    32/64/64    64/64/x
					------- 	--------	--------    -----
	low memory
					align16     align16     align16     align32		alignment
					0x030       0x030       0x030       0x030		bytes for C linkage
					0x040       0x040       0x040       0x040		bytes for saving PowerPC parameters
					0x008       0x000       0x008       0x018       pad
					0x040       0x040       0x040       0x068		[user_]siginfo_t
					0x020       0x020       0x020       0x038		ucontext64
					0x408       0x408       -           -           mcontext
				    -           0x498       0x498       0x498		mcontext64
				    align16     align16     align16     align32		alignment
                    ?           ?           ?           ?           pad
					0x0e0       0x0e0       0x0e0       0x140		red zone
	high memory	
					Some things to note about the above diagram:
                    
                    o The number and type of mcontexts depends on:
                      - whether the process is 32- or 64-bit
                      - whether the process is running on 32- or 64-bit hardware
                      - whether the process requests 64-bit registers when it 
                        installs its signal handler (the SA_64REGSET flag)
                    
                    o This, in turns, affects the amount of pad inserted to 
                      get the proper alignment.

					o For a 64-bit process, the kernel aligns the stack to a 
					  32 byte boundary, even though the runtime architecture 
					  only requires a 16 byte boundary.
					  
					o The final alignment is done last, but the pad that it 
					  creates is effectively created between the parameter save 
					  area and the [user_]siginfo_t because the C linkage area 
					  and param save areas are both defined to be a fixed offset 
					  from the frame pointer.
					
                    o This means that we have to use some heuristics to find the 
                      amount of pad.  These heuristics generally involve looking 
                      into the mcontext[64] to see if the registers in there 
                      match the registers in the [user_]siginfo_t.  However, 
                      this is further complicated by the fact that the mcontext 
                      might be an mcontext64, even on a 32-bit process.
                      
                    o The pad size is highly constrained.  This is because the 
                      kernel aligns the frame as part of setting up the red 
                      zone and again to set up the linkage.  Thus, the only cause 
                      of misalignment is the amount of data between the two alignments 
                      operations.  For a 64-bit process, there's no variation, 
                      so the pad size is always 0x018.  For a 32-bit system, 
                      there's also no variability; the pad size is always 0x008.  
                      The issues arise for 32-bit code on a 64-bit system, 
                      where the pad size can be either 0x000 or 0x000.
*/

static bool PPCCheckFrameStyle(
    QBTContext *context, 
    QTMAddr     nextFrame, 
    QTMAddr     sigInfoFPOffset,
    QTMAddr     sigInfoSize,
    QTMAddr     ucontextSize,
    QTMAddr     padSize,
    QTMAddr     mcontextFPOffset
)
    // Returns true if it seems that we're looking at the frame in the right way.  
    // That is, for the signal crossing frame at nextFrame, return true if it 
    // can be interpreted correctly the specified padSize and mcontextFPOffset.
    //
    // To check whether we're interpreting the frame correctly, we check that:
    //
    // 1. the uc_mcontext64 field of the ucontext points to the mcontext[64]
    // 2. the FP we get from the mcontext[64] matches the FP we get from the 
    //    sigInfo (stored in the pad[0] field at sigInfoFPOffset)
{
    int         err;
    bool        result;
    QTMAddr     sigInfo;
    QTMAddr     ucontext;
    QTMAddr     mcontext;
    QTMAddr     mcontextFromUContext;
    QTMAddr     preSignalFP;
    QTMAddr     preSignalFPFromMContext;

    assert(context != NULL);
    assert( ! context->arch->is64Bit);
    
    result = false;
    
    // Get the sigInfo, ucontext and mcontext addresses.  This is, of course, 
    // all provisional, based on the padSize.
    
    sigInfo  = nextFrame + 0x030 + 0x040 + padSize;
    ucontext = sigInfo + sigInfoSize;
    mcontext = ucontext + ucontextSize;

    // Read the uc_mcontext64 field from the ucontext.
    
    err = ReadAddr(context, ucontext + 0x01c, &mcontextFromUContext);
    
    // Read the FP from the pad[0] of the siginfo_t.
    
    if (err == 0) {
        err = ReadAddr(context, sigInfo + sigInfoFPOffset, &preSignalFP);
    }
    
    // Read the FP from the mcontext[64].
    
    if (err == 0) {
		err = ReadAddr(context, mcontext + mcontextFPOffset, &preSignalFPFromMContext);
    }
    
    // Return true if the mcontext is where we expected and the FPs match.
    
    if (err == 0) {
        result = (mcontext == mcontextFromUContext)
              && (preSignalFP == preSignalFPFromMContext);
    }
    
    return result;
}

static int PowerPCCrossSignalFrame(QBTContext *context, QTMAddr thisFrame, QTMAddr *nextPCPtr, QTMAddr *nextFramePtr)
	// This is the crossSignalFrame routine for the PowerPC 
	// architecture.  See the description of QBTCrossSignalFrameProc 
	// for a detailed discussion of its parameters.
{
	int         err;
    int         major;
	QTMAddr     nextFrame;
    QTMAddr     padSize = 0;
    QTMAddr     sigInfoSize = 0;
    QTMAddr     sigInfoFPOffset = 0;
    QTMAddr     ucontextSize = 0;
    QTMAddr     mcontextPCOffset = 0;
    QTMAddr     mcontextFPOffset = 0;
    QTMAddr     mcontextLROffset = 0;
    QTMAddr     sigInfo;
    QTMAddr     ucontext;
    QTMAddr     mcontext;
    QTMAddr     preSignalFP;
    QTMAddr     preSignalFPFromMContext;
    QTMAddr     preSignalPCFromMContext;
    QTMAddr     preSignalLRFromMContext;

    // The code depends on the version of the OS.  *sigh*
    
    err = QTMGetDarwinOSRelease(&major, NULL, NULL);

    // Read the address of the frame below the _sigtramp frame, because 
    // that where all the action is.

    if (err == 0) {
        err = ReadAddr(context, thisFrame, &nextFrame);
    }
	
    // Sniff the frame to see which type it is:
    
    if (err == 0) {
        if (context->arch->is64Bit) {
            // For 64-bit processes, everything is easy.
            
            padSize          = 0x018;
            sigInfoSize      = 0x068;
            sigInfoFPOffset  = 0x030;
            ucontextSize     = 0x038;
            mcontextPCOffset = 0x020 + 0x000;
            mcontextFPOffset = 0x020 + 0x018;
            mcontextLROffset = 0x020 + 0x11c;
        } else if (major < kQTMDarwinOSMajorForMacOSX103) {
            #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_3
                #warning QBacktrace has not been tested properly on pre-10.3 systems.
            #endif
            /*
                I used to have code (that was a lot more naive) that supported 
                pre-10.3 systems.  I've abandoned these systems for my new code.
                However, I've left the following information around in case I 
                ever contemplate resurrecting the support for older systems.
                
                Some random numbers from the old pre-10.3 code:
                
                o 10.0 through 10.1.x
                    offsetToPC = 0x84;
                    offsetToFP = 0x7c;
                    offsetToLR = 0;            
                o 10.2.x
                    offsetToPC = 0x98;
                    offsetToFP = 0xa4;
                    offsetToLR = 0x128;
                  but G5-based 10.2.x systems are more like 10.3 (probably).
            */
            assert(false);
            err = ENOTSUP;
        } else {
            // For 32-bit processes on 10.3 and later, things are much trickier.
            
            // padSize needs to be worked out
            sigInfoSize      = 0x040;
            sigInfoFPOffset  = 0x024;
            ucontextSize     = 0x020;
            // mcontextPCOffset needs to be worked out
            // mcontextFPOffset needs to be worked out
            // mcontextLROffset needs to be worked out
            
            // Try all three frame style...
            
            // 1. Start with the 32/64/64 case.
            
            padSize = 8;
            // mcontext64 offsets
            mcontextPCOffset = 0x020 + 0x004;
            mcontextFPOffset = 0x020 + 0x01c;
            mcontextLROffset = 0x020 + 0x120;
            
            // IMPORTANT:
            // The above offsets are 4 larger than the equivalent offsets for a 
            // 64-bit process.  That's because we'll read them using ReadAddr, 
            // and ReadAddr will only read a 32-bits word on a 32-bit process.
            // Thus, we have to make sure it reads the bottom 32-bits of the 
            // mcontext64 field, which means we add 4 to the offset.
            
            if ( ! PPCCheckFrameStyle(context, nextFrame, sigInfoFPOffset, sigInfoSize, ucontextSize, padSize, mcontextFPOffset) ) {
                // 2. OK, it's not that, let's try 32/32/x.
                
                // mcontext offsets
                mcontextPCOffset = 0x020 + 0x000;
                mcontextFPOffset = 0x020 + 0x00c;
                mcontextLROffset = 0x020 + 0x090;

                if ( ! PPCCheckFrameStyle(context, nextFrame, sigInfoFPOffset, sigInfoSize, ucontextSize, padSize, mcontextFPOffset) ) {
                    // 3. OK, third time is a charm.  Let's try 32/64/32.

                    padSize = 0;
                    
                    if ( ! PPCCheckFrameStyle(context, nextFrame, sigInfoFPOffset, sigInfoSize, ucontextSize, padSize, mcontextFPOffset) ) {
                        err = EINVAL;
                    }
                }
            }
        }
    }
    
    // Grab the pre-signal FP from the siginfo_t and the pre-signal FP, PC and LR 
    // from the mcontext and use them to set up the results for the client.

	mcontext = 0;		// quieten a warning
    if (err == 0) {
        sigInfo  = nextFrame + 0x030 + 0x040 + padSize;
        ucontext = sigInfo + sigInfoSize;
        mcontext = ucontext + ucontextSize;

		err = ReadAddr(context, sigInfo + sigInfoFPOffset, &preSignalFP);
    }
    if (err == 0) {
		err = ReadAddr(context, mcontext + mcontextFPOffset, &preSignalFPFromMContext);
    }
    if (err == 0) {
		err = ReadAddr(context, mcontext + mcontextPCOffset, &preSignalPCFromMContext);
    }
    if (err == 0) {
		err = ReadAddr(context, mcontext + mcontextLROffset, &preSignalLRFromMContext);
    }
    if (err == 0) {
        assert(preSignalFPFromMContext == preSignalFP);
        
        *nextFramePtr = preSignalFP;
        *nextPCPtr    = preSignalPCFromMContext;
    }
    	
	// If the PC is a system call, add a dummy leaf for that PC 
	// and then get the next frame's PC from LR.
	
	if ( (err == 0) && PowerPCIsSystemCall(context, *nextPCPtr) ) {
		AddFrame(context, *nextPCPtr, 0, kQBTFrameBadMask);
		
        *nextPCPtr = preSignalLRFromMContext;
	}
	
	return err;
}

#pragma mark - Intel
#if TARGET_CPU_X86 || TARGET_CPU_X86_64

/*	Intel Stack Frame Basics
	------------------------
	
                        Offset	Size	Purpose
                        ------	----	-------
	low memory
	sp == ESP/RSP ->    -??     ??		general work area
                        -??		??		local variables
	fp == EBP/RBP ->	0		X		pointer to next frame
                        X		X		return address
                        -??		??		parameters
	high memory

                    where X is the address size (4 bytes for 32-bits, 
                    8 bytes for 64-bits)

	The stack frame on Intel is remarkably traditional.  Two registers 
	are used to manage the stack: ESP/RSP points to the bottom of the stack, 
	and EBP/RBP points to the stack frame itself.  The memory at offset 0 
	off the frame stores the address of the next stack frame.  The memory at 
	offset X stores the saved PC for the next stack frame (that is, the return 
	address for this stack frame).
*/

static bool IntelIsSystemCall(QBTContext *context, QTMAddr pc)
	// Using the PC from the thread state, look back in the code 
	// stream to see if the previous bytes look something like a 
	// system call.  This is a heuristic rather than solid design. 
	// Because Intel instructions are of variable length, there's no 
	// guarantee that these bytes are part of some other instruction. 
	// Still, it works most of the time.
	//
	// For 64-bit, all systems calls are done via SYSCALL.  Nice!
    //
    // For 32-bit, we need to look for two instructions:
	//
	// o INT 81 -- used for Mach system calls
	// o sysenter -- used by BSD system calls
	// 
	// We detect INT 81 simply by looking for its bytes.  It's no 
	// so easy to detect sysenter, because the PC we get is an 
	// address in the specific system call, which actually calls 
	// another routine (_sysenter_trap) to do the sysenter.  
	// We look for the CALL disp32 instruction and, if we see, 
	// work out the address that it calls.  We then get the 
	// instructions from that address.  If that looks like a 
	// sysenter, we're probably looking at a system call.
{
	int	err;
	bool		isSystemCall;
	uint8_t		buf[5];
	uint32_t	sysEnterOffset;

	isSystemCall = false;
	err = context->readBytes(context, pc - sizeof(buf), buf, sizeof(buf));
	if (err == 0) {
        if (context->arch->is64Bit) {
            isSystemCall = ( buf[3] == 0x0f && buf [4] == 0x05 );           // syscall
        } else {
            isSystemCall = ( buf[3] == 0xcd && buf[4] == 0x81);				// INT 81
            
            if ( ! isSystemCall && (buf[0] == 0xe8) ) {						// CALL disp32
                // Get the disp32.
                
                sysEnterOffset = (buf[1] | (buf[2] << 8) | (buf[3] << 16) | (buf[4] << 24));

                // Read the instructions at that offset from the PC and see if they're 
                // the standard _sysenter_trap code.
                //
                // It's a happy coincidence that the size of the _sysenter_trap code is 
                // 5 bytes, which is also the size of the buffer that I have lying around 
                // to read the instructions in front of the PC.  The upshot is that I can 
                // reuse buf rather than needing a second one.
                
                err = context->readBytes(context, pc + sysEnterOffset, buf, sizeof(buf));
                if (err == 0) {
                    isSystemCall = (buf[0] == 0x5a)								// pop      %edx
                                && (buf[1] == 0x89)	&& (buf[2] == 0xe1)			// mov      %esp,%ecx
                                && (buf[3] == 0x0f) && (buf[4] == 0x34);		// sysenter
                }
            }
        }
	}
	return isSystemCall;
}

static int IntelHandleLeaf(QBTContext *context, QTMAddr *pcPtr, QTMAddr *framePtr)
	// This is the handleLeaf routine for the Intel architecture.  See the 
    // description of QBTHandleLeafProc for a detailed discussion of its 
    // parameters.
	// 
	// I don't have the experience or the time to fully analyse 
	// the leaf routine problem for Intel.  Rather, I just implemented 
	// a simple system call check, much like I did on PowerPC.  This 
	// seems to be effective in the cases that I care about.
{
	int		err;
	QTMAddr	pc;
	QTMAddr	sp;
	QTMAddr	fp;
	
    err = 0;
    switch (context->threadStateFlavor) {
#ifdef __LP64__
        case x86_THREAD_STATE64:

            pc = ((const x86_thread_state64_t *) context->threadState)->__rip;
            sp = ((const x86_thread_state64_t *) context->threadState)->__rsp;
            fp = ((const x86_thread_state64_t *) context->threadState)->__rbp;
            break;
        case x86_THREAD_STATE32:
            pc = ((const x86_thread_state32_t *) context->threadState)->__eip;
            sp = ((const x86_thread_state32_t *) context->threadState)->__esp;
            fp = ((const x86_thread_state32_t *) context->threadState)->__ebp;
            break;
#else
        case x86_THREAD_STATE64:

            pc = ((const x86_thread_state64_t *) context->threadState)->rip;
            sp = ((const x86_thread_state64_t *) context->threadState)->rsp;
            fp = ((const x86_thread_state64_t *) context->threadState)->rbp;
            break;
        case x86_THREAD_STATE32:
            pc = ((const x86_thread_state32_t *) context->threadState)->eip;
            sp = ((const x86_thread_state32_t *) context->threadState)->esp;
            fp = ((const x86_thread_state32_t *) context->threadState)->ebp;
            break;
#endif
        default:
            err = EINVAL;
    }

	// If the PC is a system call, add a dummy leaf for that PC 
	// and then get the next frame's PC from the top of stack.

	if (err == 0) {
        if ( IntelIsSystemCall(context, pc) ) {
            AddFrame(context, pc, 0, kQBTFrameBadMask);

            err = ReadAddr(context, sp, &pc);
        }
        if (err == 0) {
            *pcPtr    = pc;
            *framePtr = fp;
        }
    }
	
	return err;
}

static bool  IntelValidPC(QBTContext *context, QTMAddr pc)
	// This is the validPC routine for the Intel 
	// architecture.  See the description of 
	// QBTValidPCProc for a detailed discussion 
	// of its parameters.
	//
	// Intel instructions are not aligned in any way.  All, I can do 
	// is check for known bad values ((QTMAddr) -1 is used as a 
	// known bad value by the core) and check that I can read at least 
	// byte of instruction from the address.
{
	uint8_t	junkInst;
	
	return (pc != (QTMAddr) -1) && (context->readBytes(context, pc, &junkInst, sizeof(junkInst)) == 0);
}

static int IntelGetFrameNextPC(QBTContext *context, QTMAddr thisFrame, QTMAddr nextFrame, QTMAddr *nextPCPtr)
	// This is the getFrameNextPC routine for the Intel architecture.  See the 
    // description of QBTGetFrameNextPCProc for a detailed discussion of its 
	// parameters.
	//
	// This is very easy on Intel, because it's the return address, which is at a 
    // fixed offset in the frame.
{
    #pragma unused(nextFrame)
	QTMAddr	offset;
	
	if ( context->arch->is64Bit ) {
		offset = 8;
	} else {
		offset = 4;
	}
	
	return ReadAddr(context, thisFrame + offset, nextPCPtr);
}

#endif

#if TARGET_CPU_X86

/*	Intel 32-Bit Signal Stack Frames
	--------------------------------
	Cross signal stack frames is much more reliable on Intel.  The parameters 
	to _sigtramp are stored on the stack, and you can reliably pick them up 
	from there.

					Size	Purpose
					----	-------
	low memory
	
	frame  ->		0x004	pre-signal frame pointer
					0x018	struct sigframe32
					0x020?	pad
					0x258	struct mcontext
								0x00c	x86_exception_state32_t
								0x040	x86_thread_state32_t
								0x20c	x86_float_state32_t
					0x040	siginfo_t
					0x020	struct ucontext
	high memory
	
	As for other architectures, the kernel aligns the stack such that the catcher 
    field of the (struct sigframe32) is aligned on a 16 byte boundary.  This means 
    that there's a variable amount of pad between the (struct sigframe32) and the 
    other fields.  However, for x86 this isn't a problem because the 
    (struct sigframe32) contains pointers to the other structures that we need.
    
    Another thing to note is that SA_64REGSET flag isn't significant on x86.  
    A 32-bit process will always get 32-bit structures and a 64-bit process will 
    always get 64-bit structures.  This makes things somewhat easier than on 
    PowerPC.
    
    The three values we need to get are:
    
    o pre-signal FP -- This is easy.  It's always pointed to by the current FP.
    
    o pre-signal PC -- This is hard because the obvious place that it's stored 
      (the si_addr field of the siginfo_t) isn't reliable.  Specifically, for 
      SIGBUS and SIGSEGV, this field holds the faulting address, not the faulting 
      instruction's address.  *sigh*  So, we get this value by following the 
      (struct sigframe32) to the (struct ucontext) to the (struct mcontext) to 
      the (x86_thread_state32_t) to the ebp field.  *phew*
    
    o pre-signal SP -- The kernel stores this in the pad[0] field of the siginfo_t, 
      so we use that.  We also check it against the esp from the thread state, 
      just 'cause that's easy.
    
	The sinfo field of the sigframe32 structure is at offset 0x10 and the uctx 
    field is at offset 0x14.  Once you account for the pre-signal frame pointer 
    that's pushed on to the stack by _sigtramp, you need to go 0x14 bytes up the 
    frame to get the sinfo field, which is a pointer to a siginfo_t structure.  
    The kernel places the pre-signal PC and SP in fields in that structure (si_addr 
    and pad[0], offset 0x18 and 0x24 respectively). 
    
	Finally, if we detect a frameless leaf routine past the signal frame, 
	we extract its return address from the top of stack.
*/

static int Intel32CrossSignalFrame(QBTContext *context, QTMAddr thisFrame, QTMAddr *nextPCPtr, QTMAddr *nextFramePtr)
	// This is the crossSignalFrame routine for the Intel 
	// architecture.  See the description of 
	// QBTCrossSignalFrameProc for a detailed discussion 
	// of its parameters.
{
	int         err;
    QTMAddr     sigFrame;
	QTMAddr     sigInfo;
	QTMAddr     ucontext;
    QTMAddr     mcontext;
    QTMAddr     threadState;
	QTMAddr     preSignalFP;
	QTMAddr     preSignalSP;
    QTMAddr     preSignalFPFromMContext;
    QTMAddr     preSignalSPFromMContext;
    QTMAddr     preSignalPCFromMContext;
    
    sigFrame = thisFrame + 4;
    
	// Get the pre-signal FP by simply reading from the frame pointer. 
	// Because of the way __sigtramp works, this ends up being correct.
	
    err = ReadAddr(context, thisFrame, &preSignalFP);
	
	// Get the siginfo_t pointer from the parameters to _sigtramp 
	// (the sinfo field of sigframe32).
	
	if (err == 0) {
        err = ReadAddr(context, sigFrame + 0x10, &sigInfo);
	}
	
    // Get the pre-signal SP from the pad[0] field of the siginfo_t.

    if (err == 0) {
		err = ReadAddr(context, sigInfo + 0x24, &preSignalSP);
    }

	// Get the ucontext address from the parameters to _sigtramp 
    // (the uctx field of the sigframe32).
	
	if (err == 0) {
		err = ReadAddr(context, sigFrame + 0x14, &ucontext);
	}
    
    // Get the mcontext address from the uc_mcontext field of the ucontext.
    
    if (err == 0) {
		err = ReadAddr(context, ucontext + 0x1c, &mcontext);
    }
    
    // Get the address of the ss field of the mcontext.  This is the 
    // x86_thread_state_32_t.  Then extract the ebp, esp, and eip fields from 
    // that.
    
    if (err == 0) {
        threadState = mcontext + 0x0c;
        
        err = ReadAddr(context, threadState + 0x18, &preSignalFPFromMContext);
        if (err == 0) {
            err = ReadAddr(context, threadState + 0x1c, &preSignalSPFromMContext);
        }
        if (err == 0) {
            err = ReadAddr(context, threadState + 0x28, &preSignalPCFromMContext);
        }
    }
    if (err == 0) {
        assert(preSignalFPFromMContext == preSignalFP);
        assert(preSignalSPFromMContext == preSignalSP);
        
        *nextFramePtr = preSignalFP;
        *nextPCPtr    = preSignalPCFromMContext;
    }
    
	// Finally, if we detect a leaf routine, add a dummy frame for it 
	// and then get the pre-signal SP and, assuming that the top word on 
    // the stack is a return address, use it for the next PC.
	
	if ( (err == 0) && IntelIsSystemCall(context, *nextPCPtr) ) {
		AddFrame(context, *nextPCPtr, 0, kQBTFrameBadMask);
		
        err = ReadAddr(context, preSignalSP, nextPCPtr);
	}
	
	return err;
}

#endif

#if TARGET_CPU_X86_64

/*
	Intel 64-Bit Signal Stack Frames
	--------------------------------
    Like PowerPC, x86-64 passes most parameters in registers.  This makes it 
    hard to cross signal stack frames successfully.  However, like PowerPC, some 
    heuristics get us there most of the time.
    
					Size	Purpose
					----	-------
	low memory
	
	frame  ->		0x008	pre-signal frame pointer
                    0x008   space for return address (unused)
					0x014?  alignment padding (typically 0x014 or 0x01c)
					0x2c4	struct mcontext64
								0x010	x86_exception_state64_t
								0x0A8	x86_thread_state64_t
								0x20c	x86_float_state64_t
					0x068	user_siginfo_t
					0x038	struct user_ucontext64
                    0x080   red zone
	high memory

	Things to note about the above:
	
	o The kernel places the pre-signal SP in the pad[0] field (offset 0x30) of 
      the user_siginfo_t structure.
    
    o Most of the time the kernel puts the pre-signal PC in the si_addr field
      of the same structure.  However, for SIGBUS and SIGSEGV this is actually 
      the address of the fault itself, so that doesn't help us.
	
	o The kernel aligns the stack such that the start of the alignment padding 
      is on a 16 byte boundary.  This means that there's a variable amount of pad 
      (from 0x10 through 0x1c) between the current frame and the user_siginfo_t.  
      Argh!

    o The reason why this alignment padding is typically either 0x014 or 0x01c is 
      that the (struct mcontext64) is 0x2c4 bytes.  So, if the stack was reasonably 
      aligned (that is, to an 8 byte boundary) when the signal occurs, by the time 
      we've deducted space for all our junk (0x080 + 0x038 + 0x068 + 0x2c4) we've 
      pushed the alignment to a 4 byte boundary.  To bring it back, we have to 
      pad to with either 0x014 or 0x01c.
    
      However, rather than just guess, I actually try all possible alignments 
      (0x000 through 0x01c) and see which one makes sense.

    Another thing to note is that SA_64REGSET flag isn't significant on x86.  
    A 32-bit process will always get 32-bit structures and a 64-bit process will 
    always get 64-bit structures.  This makes things somewhat easier than on 
    PowerPC.

	Finally, if we detect a frameless leaf routine past the signal frame, 
	we extract its return address from the top of stack.
*/

static int Intel64CrossSignalFrame(QBTContext *context, QTMAddr thisFrame, QTMAddr *nextPCPtr, QTMAddr *nextFramePtr)
	// This is the crossSignalFrame routine for the 64-bit Intel  
	// architecture.  See the description of 
	// QBTCrossSignalFrameProc for a detailed discussion 
	// of its parameters.
{
	int         err;
	QTMAddr     sigInfo;
    QTMAddr     ucontext;
    QTMAddr     mcontext;
    QTMAddr     threadState;
	QTMAddr     preSignalFP;
    QTMAddr     preSignalSP;
	QTMAddr     preSignalFPFromMContext;
    QTMAddr     preSignalSPFromMContext;
    QTMAddr     align;
    
	// Get the previous frame by simply reading from the frame pointer. 
	// Because of the way things work, this ends up being correct.

    err = ReadAddr(context, thisFrame, &preSignalFP);
    
    if (err == 0) {        
        // Now try various alignments to see which one yields a valid result.
        
        for (align = 0x010; align < 0x020; align += 4) {
            // Use the 'Hail Mary (tm)' algorithm to get a pointer to the user_siginfo_t 
            // and the (struct user_ucontext64) that immediately follows it.
            
            sigInfo = thisFrame + 0x008 + 0x08 + align + 0x2c4;
            ucontext = sigInfo + 0x068;
            
            // Now check whether it makes sense...
            
            // Get the previous SP from the pad[0] field of the user_siginfo_t.
            
            err = ReadAddr(context, sigInfo + 0x030, &preSignalSP);
            
            // Get the uc_mcontext64 field of the (struct user_ucontext64) that 
            // immediately follows the user_siginfo_t.  This results in a pointer 
            // to our (struct mcontext64).
            
            if (err == 0) {
                err = ReadAddr(context, ucontext + 0x030, &mcontext);
            }
            
            // Calculate the start of the x86_thread_state64_t structure at a fixed 
            // offset from the start of the (struct mcontext64) and get the 
            // pre-signal FP and SP from that.
            
            if (err == 0) {
                threadState = mcontext + 0x10;
                err = ReadAddr(context, threadState + 6 * 0x08, &preSignalFPFromMContext);
            }
            if (err == 0) {
                err = ReadAddr(context, threadState + 7 * 0x08, &preSignalSPFromMContext);
            }
            
            if ( (err == 0) 
              && (preSignalFP == preSignalFPFromMContext) 
              && (preSignalSP == preSignalSPFromMContext) ) {
                break;
            }
        }
        
        // At this point we throw away any error status from the above code.

        err = 0;
        
        // If none of the alignments worked, just take a guess.  This is 
        // sufficienly bad that we want to know about it in the debug version.
        
        if (align == 0x020) {
            assert(false);
            align = 0x014;
        }
    }

    // Get the address of the sigInfo using the alignment calculated above 
    // and then use it to set the result.
    
    if (err == 0) {
        // This value is /always/ right, so we copy it out to the client immediately.
        
        *nextFramePtr = preSignalFP;

        sigInfo  = thisFrame + 0x008 + 0x08 + align + 0x2c4;
        ucontext = sigInfo + 0x068;

        // Get the pre-signal SP and mcontext as before.
        
		err = ReadAddr(context, sigInfo + 0x030, &preSignalSP);
        if (err == 0) {
            err = ReadAddr(context, ucontext + 0x030, &mcontext);
        }
        
        // And from that the thread state and the previous PC.
        
        if (err == 0) {
            threadState = mcontext + 0x10;
            err = ReadAddr(context, threadState + 16 * 0x08, nextPCPtr);
        }
	}
	
	// Finally, if we detect a leaf routine, add a dummy frame for it 
	// and then get the pre-signal SP and, assuming that the top word on 
    // the stack is a return address, use it for the next PC.
	
	if ( (err == 0) && IntelIsSystemCall(context, *nextPCPtr) ) {
		AddFrame(context, *nextPCPtr, 0, kQBTFrameBadMask);

        err = ReadAddr(context, preSignalSP, nextPCPtr);
	}
	
	return err;
}

#endif

// kArchitectures is an array of all the architectures we support.  
// Things to notes:
//
// o GetTaskArch processes this in a forward direction.  If you 
//   list a more-specific architecture, you should list it before 
//   the less-specific one.
//
// o The table is terminated by a NULL architecture, signified by 
//   a 0 in the cputype field.
//
// See the comments near QBTArchInfo for a detailed description of 
// each field.

static const QBTArchInfo kArchitectures[] = {
	{	// PowerPC
        "ppc",                      // name
		CPU_TYPE_POWERPC,			// cputype
		0,							// subcputype
		false,						// is64Bit
		15,							// frameAlignMask
		PowerPCHandleLeaf,			// handleLeaf
		PowerPCValidPC,				// validPC
		PowerPCGetFrameNextPC,		// getFrameNextPC
		PowerPCCrossSignalFrame,	// crossSignalFrame
		PPC_THREAD_STATE,			// stateFlavor
		PPC_THREAD_STATE_COUNT		// stateCount
	},
#if 0       // Not supported by BOINC
	{	// PowerPC64
        "ppc64",                    // name
		CPU_TYPE_POWERPC64,			// cputype
		0,							// subcputype
		true,						// is64Bit
		15,							// frameAlignMask
		PowerPCHandleLeaf,			// handleLeaf
		PowerPCValidPC,				// validPC
		PowerPCGetFrameNextPC,		// getFrameNextPC
		PowerPCCrossSignalFrame,	// crossSignalFrame
		PPC_THREAD_STATE64,			// stateFlavor
		PPC_THREAD_STATE64_COUNT	// stateCount
	},
#endif

#if TARGET_CPU_X86
	{	// Intel
        "x86",                      // name
		CPU_TYPE_X86,				// cputype
		0,							// subcputype
		false,						// is64Bit
		3,							// frameAlignMask
									// Apple's x86 ABI requires that the stack be 16 byte aligned, 
									// but it says nothing about the frame.  It turns out that the 
									// frame is typically 8 byte aligned, but I can't find any 
							// documentation that requires that, so I'm only checking 4 byte 
									// alignment.
		IntelHandleLeaf,			// handleLeaf
		IntelValidPC,				// validPC
		IntelGetFrameNextPC,		// getFrameNextPC
		Intel32CrossSignalFrame,    // crossSignalFrame
		x86_THREAD_STATE32,			// stateFlavor
		x86_THREAD_STATE32_COUNT    // stateCount
	},
#endif

#if TARGET_CPU_X86_64
	{	// x86-64
        "x86-64",                   // name
		CPU_TYPE_X86_64,			// cputype
		0,							// subcputype
		true,						// is64Bit
		3,							// frameAlignMask
									// Apple's x86 ABI requires that the stack be 16 byte aligned, 
									// but it says nothing about the frame.  It turns out that the 
									// frame is typically 8 byte aligned, but I can't find any 
									// documentation that requires that, so I'm only checking 4 byte 
									// alignment.
		IntelHandleLeaf,			// handleLeaf
		IntelValidPC,				// validPC
		IntelGetFrameNextPC,		// getFrameNextPC
		Intel64CrossSignalFrame,	// crossSignalFrame
		x86_THREAD_STATE64,			// stateFlavor
		x86_THREAD_STATE64_COUNT    // stateCount
	},
#endif

	{ /* null terminator */
	}
};

#pragma mark ***** Public Interface

#if ! defined(NDEBUG)

    static bool FrameArrayNeedsDispose(const QBTFrame *frameArray, size_t frameArrayCount)
        // We want to make sure that we don't leak symbol strings if we fail 
        // with an error.  So, for debugging, we check that the output frame 
        // array has no strings that need disposing.
    {
        bool        needsDispose;
        size_t      frameIndex;
        
        needsDispose = false;
        for (frameIndex = 0; frameIndex < frameArrayCount; frameIndex++) {
            if (frameArray[frameIndex].flags & kQBTFrameSymbolNeedsDisposeMask) {
                needsDispose = true;
            }
        }
        
        return needsDispose;
    }

#endif

extern int QBacktraceMachSelf(
    QSymbolsRef symRef,
	QBTFrame *	frameArray, 
	size_t		frameArrayCount, 
	size_t *	frameCountPtr
)
	// See comments in header.
{
	int                     err;
    QBTContext              context;
    uintptr_t               stackBottom;
    uintptr_t               stackTop;
    uintptr_t               stackPtr;
    thread_state_flavor_t   stateFlavor;
    void *                  state;
    size_t                  stateSize;

	assert( (frameArrayCount == 0) || (frameArray != NULL) );
	assert( frameCountPtr != NULL );

    state = NULL;
    stateSize = 0;

    // Ask pthreads for our stack bounds.
    
    stackTop    = (uintptr_t) pthread_get_stackaddr_np( pthread_self() );
    stackBottom =  stackTop - pthread_get_stacksize_np( pthread_self() );
    stackPtr    = (uintptr_t) &context;

    // Due to a bug <rdar://problem/5007494>, the above calls can return bogus 
    // results.  Specifically, you get bogus results for the main thread on 
    // pre-Leopard systems when running 64-bit code.
    //
    // So, if the results are clearly bogus (because a known good stack address 
    // (the address of one of our locals) is out of bounds), we just ignore 
    // the values we set up and use zeroes instead.
    
    if ( (stackPtr < stackBottom) || (stackPtr >= stackTop) ) {
        stackBottom = 0;
        stackTop    = 0;
    }

	assert( ((stackBottom == 0) && (stackBottom == stackTop)) || (stackBottom < stackTop) );

	// Get the thread state for the current thread.

    err = QBTCreateThreadStateSelf(&stateFlavor, &state, &stateSize);
    
	// Do the backtrace.
	
	if (err == 0) {
        err = QBacktraceMachThreadState(
            mach_task_self(),
            stateFlavor,
            state,
            stateSize,
            QMOGetLocalCPUType(),
            symRef,
            stackBottom,
            stackTop,
            frameArray,
            frameArrayCount,
            frameCountPtr
        );
	}
	
	// Clean up.
	
    free(state);

    // Post condition

    assert( (err == 0) || ! FrameArrayNeedsDispose(frameArray, frameArrayCount) );
    assert( (err == 0) == (*frameCountPtr != 0) );
	
	return err;
}

extern int QBacktraceMachThread(
	task_t		task, 
	thread_t	thread,
    cpu_type_t  cputype,
    QSymbolsRef symRef,
	QTMAddr		stackBottom, 
	QTMAddr		stackTop,
	QBTFrame *	frameArray, 
	size_t		frameArrayCount, 
	size_t *	frameCountPtr
)
	// See comments in header.
{
    int                     err;
    thread_state_flavor_t   stateFlavor;
    void *                  state;
    size_t                  stateSize;
    
	assert(task != MACH_PORT_NULL);
	assert(thread != MACH_PORT_NULL);
	assert( ((stackBottom == 0) && (stackBottom == stackTop)) || (stackBottom < stackTop) );
	assert( (frameArrayCount == 0) || (frameArray != NULL) );
	assert( frameCountPtr != NULL );

    state = NULL;
    stateSize = 0;
    
	// Get the thread state for the target thread.

    if (thread == mach_thread_self()) {
        assert(task == mach_task_self());
        
        err = QBTCreateThreadStateSelf(
            &stateFlavor,
            &state,
            &stateSize
        );
    } else {
        err = QBTCreateThreadState(
            task,
            thread,
            cputype,
            symRef,
            &stateFlavor,
            &state,
            &stateSize
        );
    }
    
	// Do the backtrace.

    if (err == 0) {
        err = QBacktraceMachThreadState(
            task,
            stateFlavor,
            state,
            stateSize,
            cputype,
            symRef,
            stackBottom,
            stackTop,
            frameArray,
            frameArrayCount,
            frameCountPtr
        );
    }
    
	// Clean up.
	
    free(state);

    // Post condition

    assert( (err == 0) || ! FrameArrayNeedsDispose(frameArray, frameArrayCount) );
    assert( (err == 0) == (*frameCountPtr != 0) );

    return err;
}

extern int QBacktraceMachThreadState(
	task_t                  task, 
	thread_state_flavor_t	stateFlavor,
	const void *			state,
	size_t					stateSize,
    cpu_type_t              cputype,
    QSymbolsRef				symRef,
	QTMAddr					stackBottom, 
	QTMAddr					stackTop,
	QBTFrame *				frameArray, 
	size_t					frameArrayCount, 
	size_t *				frameCountPtr
)
	// See comments in header.
{
	int             err;
    QBTContext      context;

	assert(task != MACH_PORT_NULL);
	assert(state != NULL);
	assert(stateSize != 0);
	assert( ((stackBottom == 0) && (stackBottom == stackTop)) || (stackBottom < stackTop) );
	assert( (frameArrayCount == 0) || (frameArray != NULL) );
	assert( frameCountPtr != NULL );

    // Initialise the context from the task.
    
    err = MachInitContextFromTask(
        &context, 
        task, 
        cputype, 
        stackBottom, 
        stackTop, 
        symRef, 
        frameArray, 
        frameArrayCount
    );
    
    // Initialise the thread state from the input arguments.
    
    if (err == 0) {
        context.threadStateFlavor = stateFlavor;
        context.threadState       = state;
        context.threadStateSize   = stateSize;
    }
	
	// Do the backtrace.
	
	if (err == 0) {
        err = BacktraceCore(&context, frameCountPtr);
	}
	
	// Clean up.
	
    MachTermContext(&context);

    // Post condition

    assert( (err == 0) || ! FrameArrayNeedsDispose(frameArray, frameArrayCount) );
    assert( (err == 0) == (*frameCountPtr != 0) );
	
	return err;
}

extern void QBacktraceDisposeSymbols(QBTFrame frameArray[], size_t frameCount)
    // See comments in header.
{
    size_t  frameIndex;
    
    if (frameArray != NULL) {
        for (frameIndex = 0; frameIndex < frameCount; frameIndex++) {
            if (frameArray[frameIndex].flags & kQBTFrameSymbolNeedsDisposeMask) {
                free( (void *) frameArray[frameIndex].symbol );
                free( (void *) frameArray[frameIndex].library );
            }
            frameArray[frameIndex].symbol = NULL;
        }
    }
}

extern int QBTCreateThreadState(
	task_t                  task, 
	thread_t                thread,
    cpu_type_t              cputype,
    QSymbolsRef             symRef,
    thread_state_flavor_t * stateFlavorPtr,
    void **                 statePtr,
    size_t *                stateSizePtr
)
    // See comments in header.
{
    int                     err;
    kern_return_t           kr;
    void *                  state;
    QMOImageRef             dyld;
    bool                    didCreateDyld;
    const QBTArchInfo *     arch = NULL;

    assert(task != MACH_PORT_NULL);
    assert(thread != MACH_PORT_NULL);
    // cputype can be anything
    // symRef may be NULL
    assert(stateFlavorPtr != NULL);
    assert( statePtr != NULL);
    assert(*statePtr == NULL);
    assert( stateSizePtr != NULL);
    assert(*stateSizePtr == 0);

    state = NULL;
    dyld = NULL;
    didCreateDyld = false;
    
    // If the client supplied us with a sym, use it to get dyld.  Otherwise 
    // get dyld directly from the QMachOImage module.
    
    if (symRef == NULL) {
        err = QMOImageCreateFromTaskDyld(task, cputype, &dyld);
        didCreateDyld = true;
    } else {
        dyld = QSymGetDyldImage(symRef);
        err = 0;
    }
    
    // Use the dyld to get the architecture.
    
    if (err == 0) {
        arch = GetTaskArch(dyld);
        if (arch == NULL) {
            err = EINVAL;
        }
    }
    
    // Use the parameters from the architecture to allocate and initialise the 
    // thread state buffer.
    
    if (err == 0) {
        state = calloc(arch->stateCount, sizeof(integer_t));
        if (state == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        mach_msg_type_number_t  stateCount;

        stateCount = arch->stateCount;
        kr = thread_get_state(thread, arch->stateFlavor, (thread_state_t) state, &stateCount);
        err = QTMErrnoFromMachError(kr);
    }
    if (err == 0) {
        *statePtr = state;
        state = NULL;           // so it's not freed below
        *stateFlavorPtr = arch->stateFlavor;
        *stateSizePtr   = arch->stateCount * sizeof(integer_t);
    }
    
    // Clean up.
    
    free(state);
    if (didCreateDyld) {
        QMOImageDestroy(dyld);
    }
    
    assert( (err == 0) == (*statePtr != NULL) );
    assert( (err == 0) == (*stateSizePtr != 0) );

    return err;
}

extern int QBTCreateThreadStateSelf(
    thread_state_flavor_t * stateFlavorPtr,
    void **                 statePtr,
    size_t *                stateSizePtr
)
    // See comments in header.
    //
    // I used to do this with a bundle of assembly language gunk, but now I take 
    // advantage of GCC's built-in functions.
{
    int         err;
    void *      pc;
    void *      fp;
    thread_state_flavor_t   flavor;
    
    assert(stateFlavorPtr != NULL);
    assert( statePtr != NULL);
    assert(*statePtr == NULL);
    assert( stateSizePtr != NULL);
    assert(*stateSizePtr == 0);
    
    // Use GCC intrinsics to get the information we need.
    
    pc = __builtin_return_address(0);
    fp = __builtin_frame_address(1);

    // Use CPU-specific code to allocate and initialise a thread state buffer.
    
    // *** Could do the allocation using information from the kArchitectures
    // array, but GetTaskArch needs an image object, which we don't have handy.
    
    #if TARGET_CPU_PPC
        ppc_thread_state_t *    state;
        
        flavor = PPC_THREAD_STATE;
        state = (ppc_thread_state_t *) calloc(1, sizeof(*state));
        if (state != NULL) {
            state->srr0 = (uintptr_t) pc;
            state->r1   = (uintptr_t) fp;
        }
    #elif TARGET_CPU_PPC64
        ppc_thread_state64_t *  state;
        
        flavor = PPC_THREAD_STATE64;
        state = (ppc_thread_state64_t *) calloc(1, sizeof(*state));
        if (state != NULL) {
            state->srr0 = (uintptr_t) pc;
            state->r1    = (uintptr_t) fp;
        }
    #elif TARGET_CPU_X86
        x86_thread_state32_t *  state;
        
        flavor = x86_THREAD_STATE32;
        state = (x86_thread_state32_t *) calloc(1, sizeof(*state));
        if (state != NULL) {
            state->eip = (uintptr_t) pc;
            state->ebp = (uintptr_t) fp;
        }
    #elif TARGET_CPU_X86_64
        x86_thread_state64_t *  state;
        
        flavor = x86_THREAD_STATE64;
        state = (x86_thread_state64_t *) calloc(1, sizeof(*state));
        if (state != NULL) {
#ifdef __LP64__
            state->__rip = (uintptr_t) pc;
            state->__rbp = (uintptr_t) fp;
#else
            state->rip = (uintptr_t) pc;
            state->rbp = (uintptr_t) fp;
#endif
        }
    #else
        #error What architecture?
    #endif

    // Pass the information back to our client.
    
    if (state == NULL) {
        err = ENOMEM;
    } else {
        *stateFlavorPtr = flavor;
        *statePtr       = state;
        *stateSizePtr   = sizeof(*state);
        err = 0;
    }

    assert( (err == 0) == (*statePtr != NULL) );
    assert( (err == 0) == (*stateSizePtr != 0) );

    return err;
}

