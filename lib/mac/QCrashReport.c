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
 *  QCrashReport.c
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
    File:       QCrashReport.c

    Contains:   Code for generating crash reports.

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

$Log: QCrashReport.c,v $
Revision 1.1  2007/03/02 12:19:57         
First checked in.


*/

/////////////////////////////////////////////////////////////////

#include "config.h"
#include "QCrashReport.h"

// System interfaces

#include <TargetConditionals.h>

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

// Put Mach includes inside extern "C" guards for the C++ build 
// because the Mach header files don't always have them.

#if defined(__cplusplus)
	extern "C" {
#endif

#include <servers/bootstrap.h>

// We want both PowerPC and Intel thread state information.
// By default, the system only gives us the one that's appropriate 
// for our machine.  So we include both here.

#include <mach/ppc/thread_status.h>
#include <mach/i386/thread_status.h>

#if defined(__cplusplus)
	}
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** Crash Report Object

// Each crash report maintains an array of QCRThread records, one per thread 
// in the target task.

struct QCRThread {
    thread_t                thread;             // name of send right for thread control port

    bool                    stateOverridden;    // true if thread state was explicitly set via QCRSetThreadStateAtIndex
    thread_state_flavor_t   stateFlavor;        // flavor of thread state
    void *                  state;              // thread state, may be NULL if not yet been fetched
    size_t                  stateSize;          // size of thread state
    
    QBTFrame *              frames;             // backtrace associated with thread state, may be NULL if not yet fetched
    size_t                  frameCount;         // number of elements of frames array
};
typedef struct QCRThread QCRThread;

// QCrashReport represents the crash report itself.  It's the backing for 
// the exported QCrashReportRef type.

struct QCrashReport {
    task_t                  task;               // target task
    cpu_type_t              requestedCPUType;   // taken from cputype parameter of QCRCreateFromTask
    cpu_type_t              actualCPUType;      // CPU type from main executable of target task
    QSymbolsRef             symRef;             // symbols object for target task
    QMOImageRef             executable;         // main executable of target task

    size_t                  threadCount;        // number of elements of threads array
    QCRThread *             threads;            // threads array, one element per thread

    size_t                  crashedThreadIndex; // index of crashed thread, kQCRNoThread
};
typedef struct QCrashReport QCrashReport;

#if ! defined(NDEBUG)

    static bool QCRIsValid(QCrashReportRef crRef)
        // Returns true if crRef looks like a valid crash report object.
    {
        bool    success;
        size_t  threadIndex;
        
        success = true
            && (crRef != NULL)
            && (crRef->task != MACH_PORT_NULL)
            && (crRef->actualCPUType != CPU_TYPE_ANY)
            && (crRef->symRef != NULL)
            && (crRef->executable != NULL)
            && (crRef->threadCount > 0)
            && (crRef->threads != NULL)
            && ((crRef->crashedThreadIndex == kQCRNoThread) || (crRef->crashedThreadIndex < crRef->threadCount))
            ;
        if (success) {
            for (threadIndex = 0; threadIndex < crRef->threadCount; threadIndex++) {
                const QCRThread *   thisThread;

                // Note: A frame count of 0 can happen with a NULL backtrace (we 
                // haven't attempted the backtrace yet) or with a non-NULL backtrace 
                // (we did it and it returned zero frames).
                
                thisThread = &crRef->threads[threadIndex];
                success = true
                    && (thisThread->thread != MACH_PORT_NULL)
                    && ( (thisThread->state == NULL) == (thisThread->stateSize == 0) )
                    && ((thisThread->frameCount == 0) || (thisThread->frames != NULL))
                    ;
            }
        }
        return success;
    }

#endif

#pragma mark - Create and Destroy

extern int QCRCreateFromTask(
    task_t              task, 
    bool                suspend, 
    thread_t            crashedThread, 
    cpu_type_t          cputype, 
    QCrashReportRef *   crRefPtr
)
    // See comment in header.
    //
    // This does less than you might think.  A lot of the work, like getting the 
    // state and backtrace for each thread, is done lazily.  The goal is to make 
    // it relatively cheap to create a crash report object, only paying the 
    // penality for things like getting the backtrace if you actually need them.
{
    int                     err;
    kern_return_t           kr;
    kern_return_t           krJunk;
    QCrashReportRef         crRef;
    size_t                  threadIndex;
    
    assert(task != MACH_PORT_NULL);
    // crashedThread may be MACH_PORT_NULL
    assert( crRefPtr != NULL);
    assert(*crRefPtr == NULL);
    
    // Allocate the memory and start filling it out.
    
    err = 0;
    crRef = (QCrashReportRef) calloc(1, sizeof(*crRef));
    if (crRef == NULL) {
        err = ENOMEM;
    }
    if (err == 0) {
        crRef->task               = task;
        crRef->crashedThreadIndex = kQCRNoThread;
        crRef->requestedCPUType   = cputype;
        
        err = QSymCreateFromTask(task, suspend, crRef->requestedCPUType, &crRef->symRef);
    }
    if (err == 0) {
        crRef->executable = QSymGetExecutableImage(crRef->symRef);
        if (crRef->executable == NULL) {
            err = EINVAL;
        }
        if (err == 0) {
            crRef->actualCPUType = QMOImageGetCPUType(crRef->executable);
            // If we didn't get the CPU type we requested, that's very weird.
            assert( (crRef->requestedCPUType == CPU_TYPE_ANY) || (crRef->requestedCPUType == crRef->actualCPUType) );
        }
    }
    
    // Fill out the threads array.
    
    if (err == 0) {
        thread_act_array_t      threadsTmp;
        mach_msg_type_number_t  threadCountTmp;
        
        threadsTmp = NULL;
        
        kr = task_threads(crRef->task, &threadsTmp, &threadCountTmp);
        err = QTMErrnoFromMachError(kr);
        
        if (err == 0) {
            crRef->threadCount = threadCountTmp;
            
            crRef->threads = (QCRThread *) calloc(crRef->threadCount, sizeof(*crRef->threads));
            if (crRef->threads == NULL) {
                err = ENOMEM;
            }
            
            // Regardless of whether the calloc succeeded, we need to run the 
            // following loop.  It either records the thread send rights we got 
            // back, or disposes of them.  We need this silliness because we 
            // can't get a thread count (to calloc the array) without also 
            // getting rights at the same time, and we have to make sure that 
            // those rights get cleaned up if calloc fais.

            for (threadIndex = 0; threadIndex < crRef->threadCount; threadIndex++) {
                if (crRef->threads == NULL) {
                    krJunk = mach_port_deallocate(mach_task_self(), threadsTmp[threadIndex]);
                    assert(krJunk == KERN_SUCCESS);
                } else {
                    crRef->threads[threadIndex].thread = threadsTmp[threadIndex];
                }
            }
        }
        
        if (threadsTmp != NULL) {
            krJunk = vm_deallocate(mach_task_self(), (vm_address_t) threadsTmp, sizeof(*threadsTmp) * threadCountTmp);
            assert(krJunk == KERN_SUCCESS);
        }
    }

    // Calculate index of crashed thread.  It's an error if we fail to 
    // find the thread in our thread list.

    if ( (err == 0) && (crashedThread != MACH_PORT_NULL) ) {
        err = EINVAL;
        for (threadIndex = 0; threadIndex < crRef->threadCount; threadIndex++) {
            if (crRef->threads[threadIndex].thread == crashedThread) {
                err = QCRSetCrashedThreadIndex(crRef, threadIndex);
                break;
            }
        }
    }
    
    // Clean up.
	    
    if (err != 0) {
        QCRDestroy(crRef);
        crRef = NULL;
    }
    *crRefPtr = crRef;
    
    assert( (err == 0) == (*crRefPtr != NULL) );
    assert( (*crRefPtr == NULL) || QCRIsValid(crRef) );
    
    return err;
}

extern int QCRCreateFromSelf(QCrashReportRef *crRefPtr)
    // See comment in header.
{
    assert( crRefPtr != NULL);
    assert(*crRefPtr == NULL);

    return QCRCreateFromTask(mach_task_self(), false, mach_thread_self(), QMOGetLocalCPUType(), crRefPtr);
}

extern void QCRDestroy(QCrashReportRef crRef)
    // See comment in header.
{
    kern_return_t   krJunk;
    size_t          threadIndex;
    
    if (crRef != NULL) {
        if (crRef->threads != NULL) {
            for (threadIndex = 0; threadIndex < crRef->threadCount; threadIndex++) {
                QCRThread * thisThread;
                
                thisThread = &crRef->threads[threadIndex];
                
                if (thisThread->thread != MACH_PORT_NULL) {
                    krJunk = mach_port_deallocate(mach_task_self(), thisThread->thread);
                    assert(krJunk == KERN_SUCCESS);
                }
                
                free(thisThread->state);
                free(thisThread->frames);
            }
        }
        QSymDestroy(crRef->symRef);
        free(crRef);
    }
}

#pragma mark - Accessors

extern QSymbolsRef QCRGetSymbols(QCrashReportRef crRef)
    // See comment in header.
{
    assert( QCRIsValid(crRef) );
    return crRef->symRef;
}

extern size_t QCRGetThreadCount(QCrashReportRef crRef)
    // See comment in header.
{
    assert( QCRIsValid(crRef) );
    return crRef->threadCount;
}

extern thread_t QCRGetThreadAtIndex(QCrashReportRef crRef, size_t threadIndex)
    // See comment in header.
{
    assert( QCRIsValid(crRef) );
    return crRef->threads[threadIndex].thread;
}

extern void QCRGetImages(
    QCrashReportRef     crRef, 
    QMOImageRef **      imageArrayPtr,
    size_t *            imageCountPtr
)
    // See comment in header.
{
    assert( QCRIsValid(crRef) );
    assert(imageArrayPtr != NULL);
    assert(imageCountPtr != NULL);
    
    QSymGetImages(crRef->symRef, imageArrayPtr, imageCountPtr);
}

static void ClearBacktraceAtIndex(
    QCrashReportRef     crRef, 
    size_t              threadIndex
)
    // For a given thread, clean its backtrace.  This reset the thread to the 
    // point before a backtrace was ever taken.
{
    assert( QCRIsValid(crRef) );
    assert( threadIndex < crRef->threadCount );

    free(crRef->threads[threadIndex].frames);
    crRef->threads[threadIndex].frames = NULL;
    crRef->threads[threadIndex].frameCount = 0;

    assert( QCRIsValid(crRef) );
}

static void ClearThreadStateAndBacktraceAtIndex(
    QCrashReportRef     crRef, 
    size_t              threadIndex
)
    // For a given thread, clean its thread state.  This reset the thread to the 
    // point before a thread state was ever fetched.  This also clears the 
    // backtrace because the backtrace was based on the old thread state.
{
    assert( QCRIsValid(crRef) );
    assert( threadIndex < crRef->threadCount );

    // Any backtrace based on the thread state is now going to be bogus, so 
    // clear it out.
    
    ClearBacktraceAtIndex(crRef, threadIndex);
    
    free(crRef->threads[threadIndex].state);
    crRef->threads[threadIndex].stateFlavor = 0;
    crRef->threads[threadIndex].state = NULL;
    crRef->threads[threadIndex].stateSize = 0;

    assert( QCRIsValid(crRef) );
}

extern int QCRGetThreadStateAtIndex(
    QCrashReportRef         crRef,
    size_t                  threadIndex,
    cpu_type_t *            cpuTypePtr,
    thread_state_flavor_t * threadStateFlavorPtr,
    const void **           threadStatePtr,
    size_t *                threadStateSizePtr
)
    // See comment in header.
{
    int             err;

    assert( QCRIsValid(crRef) );
    // Any combination of cpuTypePtr, threadStateFlavorPtr, threadStatePtr, and 
    // threadStateSizePtr may be NULL.  This is useful because it allows other 
    // parts of the module to call QCRGetThreadStateAtIndex to populate the cache 
    // without having to provide any dummy parameters.
    
    if (threadIndex >= crRef->threadCount) {
        err = EINVAL;
    } else {
        // If we haven't already cached a thread's state, go get one now.  This code 
        // is carefully written so as to not commit any results to the QCRThread 
        // structure until we're guaranteed success.
        
        err = 0;
        
        if (crRef->threads[threadIndex].state == NULL) {
            assert(crRef->threads[threadIndex].stateSize == 0);

            err = QBTCreateThreadState(
                crRef->task,
                crRef->threads[threadIndex].thread,
                crRef->actualCPUType,
                crRef->symRef,
                &crRef->threads[threadIndex].stateFlavor,
                &crRef->threads[threadIndex].state,
                &crRef->threads[threadIndex].stateSize
            );
            
            // QBTCreateThreadState is required to either error (and leave these 
            // fields alone) or succeed (and set them up).
            
            assert( (err == 0) == (crRef->threads[threadIndex].state  != NULL) );
            assert( (err == 0) == (crRef->threads[threadIndex].stateSize != 0) );
        }
            
        // Regardless of whether the above succeeded or failed, the QCRThread 
        // should still be valid.  The QCRIsValid checks that.
        
        assert( QCRIsValid(crRef) );
        
        if (err == 0) {
            if (cpuTypePtr != NULL) {
                *cpuTypePtr = crRef->actualCPUType;
            }
            if (threadStateFlavorPtr != NULL) {
                *threadStateFlavorPtr = crRef->threads[threadIndex].stateFlavor;
            }
            if (threadStatePtr != NULL) {
                *threadStatePtr       = crRef->threads[threadIndex].state;
            }
            if (threadStateSizePtr != NULL) {
                *threadStateSizePtr   = crRef->threads[threadIndex].stateSize;
            }
        }
    }

    assert( QCRIsValid(crRef) );
    
    return err;
}

extern int QCRSetThreadStateAtIndex(
    QCrashReportRef         crRef,
    size_t                  threadIndex,
    thread_state_flavor_t   threadStateFlavor,
    const void *            threadState,
    size_t                  threadStateSize
)
    // See comment in header.
{
    int     err;
    void *  state;
    
    assert( QCRIsValid(crRef) );
    assert(threadState != NULL);
    assert(threadStateSize > 0);
    assert((threadStateSize % sizeof(integer_t)) == 0);
    
	state = NULL;
	
    err = 0;
	if (threadIndex >= crRef->threadCount) {
		err = EINVAL;
	}
    
    // Allocate a buffer to hold our copy of the thread state.
    
	if (err == 0) {
		state = malloc(threadStateSize);
		if (state == NULL) {
			err = ENOMEM;
		} else {
            memcpy(state, threadState, threadStateSize);
        }
	}
    
    // Replace the thread's current state, if any, with the copy.
    
    if (err == 0) {
		// Clear out the cached thread state for the thread, and any associated 
        // backtrace.  That way the backtrace will be recalculated based on the 
		// new thread state.

        ClearThreadStateAndBacktraceAtIndex(crRef, threadIndex);
        
        assert(crRef->threads[threadIndex].state == NULL);
        crRef->threads[threadIndex].stateOverridden = true;
        crRef->threads[threadIndex].stateFlavor     = threadStateFlavor;
        crRef->threads[threadIndex].state           = state;
        crRef->threads[threadIndex].stateSize       = threadStateSize;
    }

    assert( QCRIsValid(crRef) );

    return err;
}

extern int QCRGetBacktraceAtIndex(
    QCrashReportRef     crRef, 
    size_t              threadIndex, 
    const QBTFrame **   frameArrayPtr, 
    size_t *            frameCountPtr
)
    // See comment in header.
{
    int     err;
    
    assert( QCRIsValid(crRef) );
    assert(frameArrayPtr != NULL);
    assert(frameCountPtr != NULL);
    
    err = 0;
    if (threadIndex >= crRef->threadCount) {
        err = EINVAL;
    }
    
    // If we're been asked to get a backtrace of the current thread and the 
    // thread state hasn't been explicitly set, clear any cached copy of the 
    // thread state and the backtrace, and then get the thread state right now. 
    // This ensures that we always return the current backtrace if we're 
    // called by the current thread for the current thread.
    
    if ( (err == 0) 
      && (crRef->threads[threadIndex].thread == mach_thread_self()) 
      && ! crRef->threads[threadIndex].stateOverridden ) {
        ClearThreadStateAndBacktraceAtIndex(crRef, threadIndex);
        
        assert(crRef->threads[threadIndex].state == NULL);
        
        err = QBTCreateThreadStateSelf(
            &crRef->threads[threadIndex].stateFlavor,
            &crRef->threads[threadIndex].state,
            &crRef->threads[threadIndex].stateSize
        );
    }
    
    // Get the thread state (if we haven't already).
    
    if (err == 0) {
        err = QCRGetThreadStateAtIndex(crRef, threadIndex, NULL, NULL, NULL, NULL);
    }

    // If we haven't already got a backtrace for this thread, go get one.
    
    if ( (err == 0) && (crRef->threads[threadIndex].frames == NULL) ) {
        bool        done;
        int         retries;
        QBTFrame *  frames;
        size_t      frameToAllocate;
        size_t      frameCount;

        // Start with 50 frames, which is pretty generous (QBTFrame structures are 
        // small) but certainly not always going to work.
        
        retries = 0;
        done = false;
        frameToAllocate = 50;
        do {
            frames = (QBTFrame *) calloc(frameToAllocate, sizeof(QBTFrame));
            if (frames == NULL) {
                err = ENOMEM;
            }
            if (err == 0) {
                err = QBacktraceMachThreadState(
                    crRef->task,
                    crRef->threads[threadIndex].stateFlavor,
                    crRef->threads[threadIndex].state,
                    crRef->threads[threadIndex].stateSize,
                    crRef->actualCPUType,
                    crRef->symRef,
                    0,
                    0,
                    frames,
                    frameToAllocate,
                    &frameCount
                );
            }
            if (err == 0) {
                if (frameCount <= frameToAllocate) {
                    assert(crRef->threads[threadIndex].frames == NULL);
                    crRef->threads[threadIndex].frames     = frames;
                    crRef->threads[threadIndex].frameCount = frameCount;
                    frames = NULL;      // to prevent it being freed
                    done = true;
                } else {
                    frameToAllocate = frameCount;
                    
                    retries += 1;
                    if (retries > 10) {
                        assert(false);
                        err = EINVAL;
                    }
                }
            }
            // We don't need to call QBacktraceDisposeSymbols because we passed 
            // a symbols object into QBacktraceMachThreadState.  Thus, the symbol 
            // strings are owned by the symbols object, and we don't need to clean 
            // them up here.
            free(frames);
        } while ( (err == 0) && ! done );
    }

    // Return the backtrace (either one we've cached from a previous call, or one 
    // we just got) to the caller
    
    if (err == 0) {
        *frameArrayPtr = crRef->threads[threadIndex].frames;
        *frameCountPtr = crRef->threads[threadIndex].frameCount;
    }
    
    assert( (err != 0) || (*frameArrayPtr != NULL) );
    assert( QCRIsValid(crRef) );
    
    return err;
}

extern size_t QCRGetCrashedThreadIndex(QCrashReportRef crRef)
    // See comment in header.
{
    assert( QCRIsValid(crRef) );
    
    return crRef->crashedThreadIndex;
}

extern int QCRSetCrashedThreadIndex(QCrashReportRef crRef, size_t threadIndex)
    // See comment in header.
{
    int err;
    
    assert( QCRIsValid(crRef) );

    err = 0;
    if ( (threadIndex != kQCRNoThread) && (threadIndex >= crRef->threadCount) ) {
        err = EINVAL;
    }
    if ( (err == 0) && (threadIndex != crRef->crashedThreadIndex) ) {
        crRef->crashedThreadIndex = threadIndex;
    }
    
    assert( QCRIsValid(crRef) );
    
    return err;
}

#pragma mark - Text Rendering

extern void QCRPrintBacktraces(QCrashReportRef crRef, FILE *f)
    // See comment in header.
{
    int             err;
    bool            is64Bit;
    size_t          threadIndex;
    const QBTFrame *frames;
    size_t          frameCount;
    size_t          frameIndex;
    const char *    library;
    char            libraryName[32];
    const char *    symbol;
    char            offsetStr[32];
    QTMAddr         pc;
    size_t          startframe = 0;     // Added for BOINC
   
    assert( QCRIsValid(crRef) );
    assert( f != NULL );
    
    is64Bit = QMOImageIs64Bit(crRef->executable);

    for (threadIndex = 0; threadIndex < crRef->threadCount; threadIndex++) {
        fprintf(f, "Thread %zd%s:\n", threadIndex, (threadIndex == crRef->crashedThreadIndex) ? " Crashed" : "");

        err = QCRGetBacktraceAtIndex(crRef, threadIndex, &frames, &frameCount);
        if (err == 0) {

        // If this is the thread that crashed, skip frames after the one that generated the signal. (Added for BOINC)
        startframe = 0;
        
        if (threadIndex == crRef->crashedThreadIndex)
            for (frameIndex = 0; frameIndex < frameCount; frameIndex++) {
                if ((frames[frameIndex-1].flags & kQBTSignalHandlerMask))
                    startframe = frameIndex;
            }
            
            for (frameIndex = startframe; frameIndex < frameCount; frameIndex++) {

                // The way that CrashReporter prints the library name is pretty 
                // wacky, and would require me to use functions that aren't 
                // available to 64-bit code on 10.4.x.  So I've adopted a simple 
                // solution of just printing the path to the library, trimmed 
                // to fit the field.  I trim at the front because the interesting 
                // stuff (most notably, the framework name) is at the end.
                //
                // Note that the field with is 30 and 27 is the field width 
                // minus the three dots that mark the truncation.
                
                library = frames[frameIndex].library;
                if (library == NULL) {
                    libraryName[0] = 0;
                } else {
                    if (strlen(library) > 30) {
                        snprintf(libraryName, sizeof(libraryName), "...%s", library + strlen(library) - 27);
                    } else {
                        strlcpy(libraryName, library, sizeof(libraryName));
                    }
                }
                
                // OTOH, it's easy to emulate the symbol rendering.  Note that 
                // I deliberately include the leading underscore.  The fact 
                // that CrashReporter strips it out is a poor decision IMHO.
                
                if (frames[frameIndex].symbol == NULL) {
                    symbol = "";
                    offsetStr[0] = 0;
                } else {
                    symbol = frames[frameIndex].symbol;
                    snprintf(offsetStr, sizeof(offsetStr), " + %llu", (unsigned long long) frames[frameIndex].offset);
                }

				pc = frames[frameIndex].pc;
				if ( ! is64Bit ) {
					// Without this, a value of ((QTMAddr) -1), which is generated 
					// by the backtrace code, will cause us to print an 16 digit 
					// hex number.
					pc = pc & 0x00000000FFFFFFFFLL;
				}
                fprintf(
                    f, 
                    "%3d %-30s %#0*llx %s%s\n", 
                    (int) (frameIndex - startframe),            // startframe added for BOINC
                    libraryName,
                    is64Bit ? 18 : 10,
                    pc,
                    symbol,
                    offsetStr
                );
            }
        }
        
        fprintf(f, "\n");
    }
}

static void PrintPowerPCThreadState(
    QCrashReportRef crRef, 
    const char *    threadID, 
    FILE *          f
)
    // Prints a PowerPC 32-bit thread state based on the thread state of the crashed 
    // thread.
    //
    // I'm really not happy with this.  For a start, there's a whole bunch code  
    // that's shared between the various CPU architecture printing routines that 
    // could be factored out.  However, doing that nicely would require me to 
    // strike a difficult balance between flexibility and complexity.
    // 
    // Secondly, it would be nice if this routine was explicitly passed the 
    // thread state so that it could work on things other than the crashed 
    // thread.  And that has implications for the API that wraps this up 
    // (QCRPrintThreadState).
    //
    // Hmmm, what to do.  Nothing at the moment.  Also spent way too much time 
    // on this code.
{
    const ppc_thread_state_t *      state;
    const unsigned int *            regBase;
    int                             reg;
    char                            regName[32];
    
    assert( QCRIsValid(crRef) );
    assert(crRef->crashedThreadIndex != kQCRNoThread);
    assert( crRef->threads[crRef->crashedThreadIndex].state != NULL );
    assert( crRef->threads[crRef->crashedThreadIndex].stateFlavor == PPC_THREAD_STATE );
    
    if (crRef->threads[crRef->crashedThreadIndex].stateSize == (PPC_THREAD_STATE_COUNT * sizeof(integer_t))) {
        state = (const ppc_thread_state_t *) crRef->threads[crRef->crashedThreadIndex].state;
/*
Thread 0 crashed with PPC Thread State:
  srr0: 0x01be8f98 srr1: 0x0200f030                vrsave: 0x00000000
   xer: 0x20000000   lr: 0x01be49bc  ctr: 0x00000000   mq: 0x00000000
    r0: 0x01be49b4   r1: 0xbffff630   r2: 0x0188e250   r3: 0xffffffff
    r4: 0x00000052   r5: 0x00000004   r6: 0x00000000   r7: 0xbffff7cf
    r8: 0xbffffcc7   r9: 0x0000000b  r10: 0xa0000d84  r11: 0xa00041f4
   r12: 0x0188e3a4  r13: 0x00000000  r14: 0x00000000  r15: 0x00000000
   r16: 0x00000000  r17: 0x00000000  r18: 0x00000000  r19: 0x00000000
   r20: 0x00ca0275  r21: 0xbffffa00  r22: 0xbffffcce  r23: 0xbffffccc
   r24: 0xbffffcc4  r25: 0xbffffb20  r26: 0xbffffcc4  r27: 0xbffffcc4
   r28: 0x0188d650  r29: 0x01be92d8  r30: 0xbffff900  r31: 0x01be92e0
*/
        fprintf(f, "%s crashed with PPC Thread State:\n", threadID);
#ifndef __LP64__
        fprintf(f, "  srr0: 0x%08x srr1: 0x%08x                vrsave: 0x%08x\n", state->srr0, state->srr1, state->vrsave);
        fprintf(f, "   xer: 0x%08x   lr: 0x%08x  ctr: 0x%08x   mq: 0x%08x\n", state->xer, state->lr, state->ctr, state->mq);

        regBase = (const unsigned int *) &state->r0;
#else
        fprintf(f, "  srr0: 0x%08x srr1: 0x%08x                vrsave: 0x%08x\n", state->__srr0, state->__srr1, state->__vrsave);
        fprintf(f, "   xer: 0x%08x   lr: 0x%08x  ctr: 0x%08x   mq: 0x%08x\n", state->__xer, state->__lr, state->__ctr, state->__mq);

        regBase = (const unsigned int *) &state->__r0;
#endif
        for (reg = 0; reg < 32; reg++) {
            if ((reg % 4) == 0) {
                fprintf(f, " ");
            }
            snprintf(regName, sizeof(regName), "r%d", reg);
            fprintf(f, "%4s: 0x%08x", regName, regBase[reg]);
            
            if ((reg % 4) == 3) {
                fprintf(f, "\n");
            }
        }
        fprintf(f, "\n");
    }
}

static void PrintPowerPC64ThreadState(
    QCrashReportRef crRef, 
    const char *    threadID, 
    FILE *          f
)
    // Prints a PowerPC 64-bit thread state based on the thread state of the 
    // crashed thread.
    //
    // See PrintPowerPCThreadState for comments about the overall approach.
{
    const ppc_thread_state64_t *    state;
    const unsigned long long *      regBase;
    int                             reg;
    char                            regName[32];
    
    assert( QCRIsValid(crRef) );
    assert(crRef->crashedThreadIndex != kQCRNoThread);
    assert( crRef->threads[crRef->crashedThreadIndex].state != NULL );
    assert( crRef->threads[crRef->crashedThreadIndex].stateFlavor == PPC_THREAD_STATE64 );
    
    if (crRef->threads[crRef->crashedThreadIndex].stateSize == (PPC_THREAD_STATE64_COUNT * sizeof(integer_t))) {
        state = (const ppc_thread_state64_t *) crRef->threads[crRef->crashedThreadIndex].state;

/*
Thread 0 crashed with PPC Thread State 64:
  srr0: 0x0000000000000000 srr1: 0x000000004000d030                        vrsave: 0x0000000000000000
    cr: 0x44022282          xer: 0x0000000020000004   lr: 0x000000009000b15c  ctr: 0x000000009000b200
    r0: 0x00000000ffffffe1   r1: 0x00000000bfffeb10   r2: 0x00000000a073cdec   r3: 0x0000000010004005
    r4: 0x0000000003000006   r5: 0x0000000000000000   r6: 0x0000000000000450   r7: 0x0000000000001203
    r8: 0x0000000000000000   r9: 0x0000000000000000  r10: 0x0000000000000003  r11: 0x00000000a0006a2c
   r12: 0x000000009000b200  r13: 0x0000000000000000  r14: 0x0000000000000001  r15: 0x0000000000000001
   r16: 0x0000000000000000  r17: 0x0000000000000000  r18: 0x000000000000430f  r19: 0x0000000000000000
   r20: 0x00000000101a6e8a  r21: 0x00000000f8bd9d7f  r22: 0x0000000000310808  r23: 0x00000000bfffebe0
   r24: 0x0000000000000450  r25: 0x0000000000001203  r26: 0x0000000000000000  r27: 0x0000000000000000
   r28: 0x0000000000000000  r29: 0x0000000003000006  r30: 0x0000000003000006  r31: 0x000000009075cdec
*/

        fprintf(f, "%s crashed with PPC Thread State:\n", threadID);
#ifndef __LP64__
        fprintf(f, "  srr0: 0x%016llx srr1: 0x%016llx                        vrsave: 0x%016x\n", state->srr0, state->srr1, state->vrsave);
        fprintf(f, "    cr: 0x%08x          xer: 0x%016llx   lr: 0x%016llx  ctr: 0x%016llx\n", state->cr, state->xer, state->lr, state->ctr);

        regBase = (const unsigned long long *) &state->r0;
#else
        fprintf(f, "  srr0: 0x%016llx srr1: 0x%016llx                        vrsave: 0x%016x\n", state->__srr0, state->__srr1, state->__vrsave);
        fprintf(f, "    cr: 0x%08x          xer: 0x%016llx   lr: 0x%016llx  ctr: 0x%016llx\n", state->__cr, state->__xer, state->__lr, state->__ctr);

        regBase = (const unsigned long long *) &state->__r0;
#endif
        for (reg = 0; reg < 32; reg++) {
            if ((reg % 4) == 0) {
                fprintf(f, " ");
            }
            snprintf(regName, sizeof(regName), "r%d", reg);
            fprintf(f, "%4s: 0x%016llx", regName, regBase[reg]);
            
            if ((reg % 4) == 3) {
                fprintf(f, "\n");
            }
        }
        fprintf(f, "\n");
    }
}

#if TARGET_CPU_X86 || TARGET_CPU_X86_64

static void PrintX86ThreadState(
    QCrashReportRef crRef, 
    const char *    threadID, 
    FILE *          f
)
    // Prints a x86 32-bit thread state based on the thread state of the 
    // crashed thread.
    //
    // See PrintPowerPCThreadState for comments about the overall approach.
{
    const x86_thread_state32_t *    state;
    const unsigned int *            regBase;
    int                             reg;
    static const char * kRegNames[16] = {
        "eax", "ebx", "ecx", "edx",
        "edi", "esi", "ebp", "esp",
        "ss",  "efl", "eip", "cs", 
        "ds",  "es",  "fs",  "gs"
    };
    
    assert( QCRIsValid(crRef) );
    assert(crRef->crashedThreadIndex != kQCRNoThread);
    assert( crRef->threads[crRef->crashedThreadIndex].state != NULL );
    assert( crRef->threads[crRef->crashedThreadIndex].stateFlavor == x86_THREAD_STATE32 );
    
    if (crRef->threads[crRef->crashedThreadIndex].stateSize == (x86_THREAD_STATE32_COUNT * sizeof(integer_t))) {
        state = (const x86_thread_state32_t *) crRef->threads[crRef->crashedThreadIndex].state;

/*
Thread 0 crashed with X86 Thread State (32-bit):
  eax: 0x00000000    ebx: 0x908156b2 ecx: 0xbfffee2c edx: 0x00000000
  edi: 0x0031efa0    esi: 0x00000000 ebp: 0xbfffee98 esp: 0xbfffee70
   ss: 0x0000001f    efl: 0x00010282 eip: 0x908156b5  cs: 0x00000017
   ds: 0x0000001f     es: 0x0000001f  fs: 0x00000000  gs: 0x00000037
*/
        fprintf(f, "%s crashed with X86 Thread State (32-bit):\n", threadID);
#ifndef __LP64__
        regBase = (const unsigned int *) &state->eax;
#else
        regBase = (const unsigned int *) &state->__eax;
#endif
        for (reg = 0; reg < 16; reg++) {
            if ((reg % 4) == 0) {
                fprintf(f, " ");
            }
            fprintf(f, "%4s: 0x%08x", kRegNames[reg], regBase[reg]);
            
            if ((reg % 4) == 3) {
                fprintf(f, "\n");
            }
        }
        fprintf(f, "\n");
    }
}

static void PrintX86_64ThreadState(
    QCrashReportRef crRef, 
    const char *    threadID, 
    FILE *          f
)
    // Prints a x86 64-bit thread state based on the thread state of the 
    // crashed thread.
    //
    // See PrintPowerPCThreadState for comments about the overall approach.
{
    const x86_thread_state64_t *    state;
    const unsigned long long *      regBase;
    int                             reg;
    static const char * kRegNames[18] = {
        "rax", "rbx", "rcx", "rdx",
        "rdi", "rsi", "rbp", "rsp",
        "r8",  "r9",  "r10", "r11",
        "r12", "r13", "r14", "r15",
        "rip", "rfl" 
    };
    
    assert( QCRIsValid(crRef) );
    assert(crRef->crashedThreadIndex != kQCRNoThread);
    assert( crRef->threads[crRef->crashedThreadIndex].state != NULL );
    assert( crRef->threads[crRef->crashedThreadIndex].stateFlavor == x86_THREAD_STATE64 );
    
    if (crRef->threads[crRef->crashedThreadIndex].stateSize == (x86_THREAD_STATE64_COUNT * sizeof(integer_t))) {
        state = (const x86_thread_state64_t *) crRef->threads[crRef->crashedThreadIndex].state;
/*
Unknown thread crashed with X86 Thread State (64-bit):
  rax: 0x0000000000000000  rbx: 0x00007fff5fbffb98  rcx: 0x000000000000003a  rdx: 0x0000000000000000
  rdi: 0x0000000000000002  rsi: 0x00007fff5fbfedd0  rbp: 0x00007fff5fbffab0  rsp: 0x00007fff5fbff5f0
   r8: 0x0000000000000e03   r9: 0x0000000000000000  r10: 0x0000000000000000  r11: 0x0000000000000246
  r12: 0x00007fff5fbffb78  r13: 0x00007fff5fbffc18  r14: 0x0000000000000003  r15: 0x0000000000000000
  rip: 0x000000010000f1fe  rfl: 0x0000000000010202
*/
        fprintf(f, "%s crashed with X86 Thread State (64-bit):\n", threadID);
#ifndef __LP64__
        regBase = (const unsigned long long *) &state->rax;
#else
        regBase = (const unsigned long long *) &state->__rax;
#endif
        for (reg = 0; reg < 18; reg++) {
            fprintf(f, "%5s: 0x%08llx", kRegNames[reg], regBase[reg]);
            
            if ((reg % 4) == 3) {
                fprintf(f, "\n");
            }
        }
        fprintf(f, "\n");
        fprintf(f, "\n");
    }
}

#endif

extern void QCRPrintThreadState(QCrashReportRef crRef, FILE *f)
    // See comment in header.
{
    int     err;
    char    threadID[32];
    
    assert( QCRIsValid(crRef) );
    assert( f != NULL );
    
    // Get the thread state, if we haven't already.
    
    err = QCRGetThreadStateAtIndex(crRef, crRef->crashedThreadIndex, NULL, NULL, NULL, NULL);
    if (err == 0) {
        snprintf(threadID, sizeof(threadID), "Thread %zu", crRef->crashedThreadIndex);

        // Dispatch to the printing routine.
        //
        // Each CPU type has its own thread state flavor namespace, although it's 
        // shared by the 32- and 64-bit variants.
        switch (crRef->actualCPUType) {
            case CPU_TYPE_POWERPC:
#if 0       // BOINC does not support 64-bit PowerPC
            case CPU_TYPE_POWERPC64:
#endif
                switch (crRef->threads[crRef->crashedThreadIndex].stateFlavor) {
                    case PPC_THREAD_STATE:
                        PrintPowerPCThreadState(crRef, threadID, f);
                        break;
                    case PPC_THREAD_STATE64:
                        PrintPowerPC64ThreadState(crRef, threadID, f);
                        break;
                    default:
                        assert(false);
                        break;
                }
                break;
#if TARGET_CPU_X86 || TARGET_CPU_X86_64
            case CPU_TYPE_X86:
            case CPU_TYPE_X86_64:
                switch (crRef->threads[crRef->crashedThreadIndex].stateFlavor) {
                    case x86_THREAD_STATE32:
                        PrintX86ThreadState(crRef, threadID, f);
                        break;
                    case x86_THREAD_STATE64:
                        PrintX86_64ThreadState(crRef, threadID, f);
                        break;
                    default:
                        assert(false);
                        break;
                }
                break;
#endif
            default:
                assert(false);
                break;
        }
    }
}

extern void QCRPrintImages(QCrashReportRef crRef, FILE *f)
    // See comment in header.
{
    int                         err;
    int                         width;
    QMOImageRef *               images;
    size_t                      imageCount;
    size_t                      imageIndex;
    struct segment_command_64   seg;
    const char *                filePath;
    QTMAddr                     imageStart;
    QTMAddr                     imageEnd;

    assert( QCRIsValid(crRef) );
    assert( f != NULL );

    fprintf(f, "Binary Images Description:\n");

    width = QMOImageIs64Bit(crRef->executable) ? 18 : 10;

    QCRGetImages(crRef, &images, &imageCount);
    for (imageIndex = 0; imageIndex < imageCount; imageIndex++) {
        QMOImageRef thisImage;
        
        thisImage = images[imageIndex];
        
        err = QMOImageGetSegmentByName(thisImage, "__TEXT", NULL, &seg);
        assert(err == 0);
        
        if (err == 0) {
            filePath = QMOImageGetFilePath(thisImage);
            if (filePath == NULL) {
                filePath = "";
            }
            
            // Currently we just print the path to the library.  CrashReporter 
            // has all sorts of wacky code to print the bundle ID and version 
            // numbers.  This isn't easy to replicate in code that can only 
            // link with libSystem.
            
            imageStart = seg.vmaddr + QMOImageGetSlide(thisImage);
            imageEnd   = imageStart + seg.vmsize - 1;
            fprintf(f, "%#*llx - %#*llx %s\n", width, imageStart, width, imageEnd, filePath);
        }
    }
    fprintf(f, "\n");
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Tool Helpers

#if 0       // Not used by BOINC

// Declare the standard C environ variable.  I never understood why this 
// global isn't declared in some public prototype.

extern char **environ;

static int ForkExecWithBootstrap(const char *toolArgs[], mach_port_t newBootstrap, pid_t *childPIDPtr)
    // Runs a process with the specified bootstrap namespace.  toolArgs is a 
    // NULL terminated list of argument for the tool.  The first entry must be 
    // the path to the tool.  newBootstrap is the name of a send right to the 
    // new bootstrap namespace.  childPIDPtr must not be NULL.  On entry, 
    // *childPIDPtr is ignored.  On success, *childPIDPtr is the process ID 
    // of the new process.  On error, *childPIDPtr is -1.
    //
    // IMPORTANT: This routine is not thread safe.  On current systems the only 
    // reliable way to set the bootstrap port of a child process is for the 
    // process to inherit it.  Thus, this routine works by setting the current 
    // process's bootstrap, forking the child, and then resetting the current 
    // process's bootstrap back to its previous value.  If some other thread 
    // calls fork while we're doing this, it's child is going to run in the 
    // wrong namespace.  Go team!
    //
    // You can solve this problem using posix_spawn, but it's not available on 
    // current systems.
{
    int             err;
    kern_return_t   kr;
    kern_return_t   junk;
    mach_port_t     oldBootstrap;
    pid_t           childPID;
    
    assert(toolArgs != NULL);
    assert(toolArgs[0] != NULL);
    assert(newBootstrap != MACH_PORT_NULL);
    assert( childPIDPtr != NULL);
    
    oldBootstrap = MACH_PORT_NULL;
    childPID = -1;
    
    // Get the old bootstrap and set the new bootstrap.
    
    kr = task_get_bootstrap_port(mach_task_self(), &oldBootstrap);
    if (kr == KERN_SUCCESS) {
        kr = task_set_bootstrap_port(mach_task_self(), newBootstrap);
    }
    
    // Do the standard fork/exec dance.
    
    if (kr == KERN_SUCCESS) {
        childPID = fork();
        switch (childPID) {
            case -1:
                err = errno;
                break;
            case 0:
                (void) execve(toolArgs[0], (char **) toolArgs, environ);
                _exit(EXIT_FAILURE);
                break;
            default:
                // parent execution continues below
                err = 0;
                break;
        }
        
        // In the parent, restore the bootstrap.
        
        junk = task_set_bootstrap_port(mach_task_self(), oldBootstrap);
        assert(junk == KERN_SUCCESS);
    } else {
        err = QTMErrnoFromMachError(kr);
    }
    
    // Clean up.
    
    *childPIDPtr = childPID;
    if (oldBootstrap != MACH_PORT_NULL) {
        junk = mach_port_deallocate(mach_task_self(), oldBootstrap);
        assert(junk == KERN_SUCCESS);
    }
    
    assert( (err == 0) == (*childPIDPtr != -1) );

    return err;
}

#pragma mark - On task_for_pid and Task Control Ports

/*
    On task_for_pid and Task Control Ports
    --------------------------------------
    When writing an external crash reporting tool, you really need to be able to 
    get at the memory, threads, and so on, of the target process.  The only way 
    to do this on Mac OS X is to have a send right to the process's task 
    control port.  And the best way to get that is to use a routine called 
    task_for_pid.
    
    task_for_pid returns the name of a send right for the task control port for 
    the process with the specified process ID.  This is fabulously useful for 
    debugging tools, like any form of external crash report.  But, needless to 
    say, it represents somewhat of a security concern.  Once you have access 
    to a task's control port, you can do anything you like to the task 
    (most notably, modify its memory).  Therefore it's important that task_for_pid 
    be appropriately protected.
    
    Initially (starting with Mac OS X 10.0), task_for_pid's security policy was 
    pretty simple.  To summarise, the old policy was that task_for_pid would 
    succeed if:
    
    o if the caller is running as root (EUID 0)

    o otherwise if the caller is running as the same UID as the target (and the 
      target's EUID matches its RUID, that is, it's not a setuid binary)
    
    Starting with the release of the Intel-based Macintosh computers, task_for_pid 
    security has been tightened up.  The new policy is that task_for_pid suceeds:
    
    o if the caller was running as root (EUID 0)
    
    o if the pid is that of the caller
    
    o if the caller is running as the same UID as the target and the target's 
      EUID matches its RUID, that is, it's not a setuid binary AND the caller 
      is in group "procmod" or "procview"
    
    btw The long term goal of having both "procmod" and "procview" is that 
    task_for_pid would return a send right for the task control port only to those 
    processes in "procmod"; the callers in "procview" would only get a send right 
    to a task inspection port.  This distinction is not currently implemented.
      
    This new policy will be adopted by all systems (that is, PowerPC-based systems 
    as well as Intel-based systems) in a future version of Mac OS X.
    
    This policy change is a pain for code, like this, that wants to manipulate 
    other processes.  If you want to get the task control port for a task, your 
    options are much more limited:
    
    1. run as ROOT (EUID 0) -- This is a bad idea for security reasons.
    
    2. change the policy -- You can actually change the task_for_pid policy via 
       sysctl.  This is also a bad idea.  The policy was changed changed for a 
       reason, and it's not your place to subvert that change.
    
    3. get into group "procmod" or "procview" -- This is the solution used by 
       Apple's tools, but it is tricky for third party developers in practise.  
       You have to make your executable set-group-id to one of these groups, and 
       that requires you to have some sort of installer to 'bless' the executable.
       
    4. get the task to give you a send right to its task control port -- As with 
       any other other Mach right, you can send a send right for a task control 
       port between tasks as part of a Mach message.
    
    Given the above, only option 4 is a reasonable solution.  However, it does result 
    in a chicken and egg problem.  How do you initially start communication with 
    the task?  The solution to this is to use the Mach bootstrap service.  This 
    service lets you register a port with a specific name, and other processes 
    within the same bootstrap context can look up that port using that name.

    However, publishing your task control port via the bootstrap service is not 
    wise.  Anyone could look up the port and start reading and writing your tasks 
    memory.  Of course, this is what you want, except that you want to restrict 
    it to just your crash report tool.
    
    You can solve that problem by creating a new bootstrap namespace (also known 
    as a subset) and registering your task control port in that namespace.  
    If you run your crash reporting tool in that namespace, it can access your 
    task control port.  Moreover, only processes running in that namespace can 
    access it.  That's the approach taken by the following code.
*/

extern int QCRExecuteCrashReportTool(const char *toolArgs[], pid_t *pidPtr, int *toolStatusPtr)
    // See comment in header.
{
    int             err;
    kern_return_t   kr;
    kern_return_t   junk;
    mach_port_t     inheritedNamespace;
    mach_port_t     privateNamespace;
    pid_t           childPID;
    pid_t           waitResult;
    int             status;
    
    assert(toolArgs != NULL);
    assert(toolArgs[0] != NULL);
    // pidPtr may be NULL
    assert( (pidPtr == NULL) || (*pidPtr == -1) );
    assert( (pidPtr == NULL) == (toolStatusPtr != NULL) );
    
    inheritedNamespace = MACH_PORT_NULL;
    privateNamespace   = MACH_PORT_NULL;
    
    // Get the old bootstarp.
    
    kr = task_get_bootstrap_port(mach_task_self(), &inheritedNamespace);
    
    // Create a private namespace based on it.  It's this that we're going to 
    // give to the child.
    
    if (kr == KERN_SUCCESS) {
        kr = bootstrap_subset(inheritedNamespace, mach_task_self(), &privateNamespace);
    }
    
    // Register our task control port in the private namespace.  Only someone 
    // with access to privateNamespace can look up our task control port.  And 
    // we're only going to give that namespace to our child.
    //
    // Note that we don't have to worry about name uniqueness, because we just 
    // created the namespace and it doesn't have any entries.  If someone in 
    // the parent namespace (inheritedNamespace) has registered this name, 
    // our registration masks it.
    
    if (kr == KERN_SUCCESS) {
        // Don't try this at home kiddies.  It gives anyone that inherits this 
        // private namespace completely access to your process.
        kr = bootstrap_register(privateNamespace, "QCrashReport", mach_task_self());
    }
    err = QTMErrnoFromMachError(kr);
    
    // Run the tool with the private namespace.
    
    if (err == 0) {
        err = ForkExecWithBootstrap(toolArgs, privateNamespace, &childPID);
    }
    
    // Clean up.  If we're running asynchronously, just pass the PID back to the 
    // caller.  If we're running synchronously, wait for the child to quit.
    
    if (err == 0) {
        if (pidPtr != NULL) {
            // Client is going to manage the child from here on.
            
            *pidPtr = childPID;
        } else {
            // Client wants use to wait for the child to exit.
            
            do {
                waitResult = waitpid(childPID, &status, 0);
                if (waitResult < 0) {
                    err = errno;
                }
            } while (err == EINTR);
            
            if (err == 0) {
                if ( WIFEXITED(status) ) {
                    *toolStatusPtr = WEXITSTATUS(status);
                } else {
                    err = EINVAL;
                }
            }
        }
    }

    // Clean up.  Note that we can dispose of our reference to privateNamespace. 
    // The namespace will persist because the child has a send right for it. 
    // It will, however, be deactivated if we quit.

    if (inheritedNamespace != MACH_PORT_NULL) {
        junk = mach_port_deallocate(mach_task_self(), inheritedNamespace);
        assert(junk == KERN_SUCCESS);
    }
    if (privateNamespace != MACH_PORT_NULL) {
        junk = mach_port_deallocate(mach_task_self(), privateNamespace);
        assert(junk == KERN_SUCCESS);
    }
    
    assert( (pidPtr == NULL) || ( (err == 0) == (*pidPtr != -1) ) );

    return err;
}

extern int QCRGetParentTask(task_t *taskPtr)
    // See comment in header.
{
    int             err;
    kern_return_t   kr;
    kern_return_t   junk;
    mach_port_t     bootstrap;
    
    assert( taskPtr != NULL);
    assert(*taskPtr == MACH_PORT_NULL);

    bootstrap = MACH_PORT_NULL;
    
    // Look up the parent's task control port by name in our bootstrap namespace. 
    // Easy peasy.
    
    kr = task_get_bootstrap_port(mach_task_self(), &bootstrap);
    if (kr == KERN_SUCCESS) {
        kr = bootstrap_look_up(bootstrap, "QCrashReport", taskPtr);
    }
    err = QTMErrnoFromMachError(kr);
    
    if (bootstrap != MACH_PORT_NULL) {
        junk = mach_port_deallocate(mach_task_self(), bootstrap);
        assert(junk == KERN_SUCCESS);
    }
    
    if (false) {
        fprintf(stderr, "Process %ld waiting for debugger.\n", (long) getpid());
        pause();
    }

    return err;
}

#endif      // Not used by BOINC
