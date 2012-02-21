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
 *  QBacktrace.h
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
    File:       QBacktrace.h

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

$Log: QBacktrace.h,v $
Revision 1.1  2007/03/02 12:19:53         
First checked in.


*/

#ifndef _QBACKTRACE_H
#define _QBACKTRACE_H

/////////////////////////////////////////////////////////////////

// System Interfaces

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

#include "QTaskMemory.h"
#include "QSymbols.h"

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////

/*!
    @header         QBacktrace.h
    
    @abstract       Comprehensive backtrace generation.

    @discussion     This module implements a number of backtrace routines: 
                    
                      o QBacktraceMachSelf does a backtrace of the current thread. 
                      o QBacktraceMachThread does a backtrace of an arbitrary thread 
                        within an arbitrary process.
                    
                    All of the routines are implemented in terms of a common core. 
                    The code is structured in a very generic way.  For example, 
                    on an Intel-based Macintosh computer, it's possible to 
                    backtrace a PowerPC program (run using Rosetta) from a Intel 
                    program, and vice versa.
                    
                    Backtraces are inherently processor-specific.  Internal to this 
                    module is a ISA layer than adapts the backtrace for various ISA. 
                    Currently it supports PowerPC (32-bit), PowerPC (64-bit), 
                    Intel (32-bit), and Intel (64-bit).
                    
                    If you're curious about how stack frames work for each ISA, 
                    check out the comments in the implementation file.  The comments 
                    in the header focus on how you use these routines.
                    
                    The core of this module is also (mostly) independent of the 
                    technology that you use to read the address space of the 
                    process that you're backtracing.  The current implementation 
                    uses Mach APIs to do this, but it is relatively simple to 
                    retarget it to use some other API (a previous version of the 
                    code read the memory directly and used the Carbon 
                    Exception Manager to catch any exceptions that this triggered). 
                    However, as we're currently quite wedded to Mach, I've only 
                    retained the Mach functionality.
                    
                    If you used a previous version of this code (known as 
                    MoreBacktrace), please note the following changes:
                    
                    o Everything has changed (-:
                    
                    I completely rewrote this code to support multiple architectures. 
                    By including support for 64-bit ISA, I was forced to 
                    eliminate my dependencies on CoreServices (which isn't available 
                    to 64-bit programs on Mac OS X 10.4.x), which means now I depend 
                    solely on the System framework.  Also, because I had to support 
                    Intel, which requires Mach-O, I decided to drop support for CFM. 
                    My theory is that anyone who wants to adopt the new version of 
                    this module is doing so because they're porting to Intel, and 
                    those folks have to leave their CFM build behind.
                    
                    The good news is that the new implementation is very similar 
                    in spirit to the old, and it should be very easy for you to 
                    adopt the new code.
                    
                    There are a number of approaches for using the routines exported 
                    by this module.
                    
                    o If you just want a simple backtrace, create an array of N 
                      QBTFrame structures and just pass that into the function. 
                      You'll get information about the N frames on the top of 
                      the stack.  Simple and easy.
                    
                    o If you want to get all of the frames, you can first call 
                      the routine with a NULL frame array.  This will return 
                      you the number of frames in the backtrace.  You can use 
                      that value to allocate a QBTFrame array with the appropriate 
                      number of elements and then call the function again to 
                      get the actual backtrace.
                    
                    o You can also use a hybrid of these approaches.  Start by 
                      allocating an array with N entries, where N is likely to be 
                      enough to accomodate a typical backtrace.  Then call the 
                      backtrace function.  If it indicates that you missed some 
                      elements, grow the array and call the backtrace function 
                      again.
                    
                    The backtrace functions generally don't fail with an error. 
                    In general, if the chain of frames on the stack runs off the 
                    rails, the backtrace function just returns a truncated backtrace. 
                    The two situations where the backtrace function do return an 
                    error are a) if it can't even start the backtrace, or 
                    b) if something wacky happens, like it can't allocate memory, 
                    or manipulate the target task, or find the target thread, and 
                    so on.
                    
                    If the backtrace function succeeds, you may need to dispose 
                    of the symbol and library strings that it allocated by calling 
                    QBacktraceDisposeSymbols.  This is necessary when all of the 
                    following are true:
                    
                    o The backtrace function succeeds.
                    
                    o You pass NULL to the symRef parameter.
                    
                    o You pass a non-zero value to the frameArrayCount parameter 
                      (and thus passed a non-NULL value to the frameArray 
                      parameter).
                    
                    To make things simpler, it's generally easiest to always 
                    call QBacktraceDisposeSymbols.  The module know which symbol 
                    and library strings it allocated, and won't try to free anything 
                    that it doesn't own.  The QBacktraceDisposeSymbols is very 
                    flexible about the parameters it accepts, which makes it easy 
                    for you to call in all circumstances.  See the function description 
                    for more details.

                    The lifetime of the symbol and library strings returned by the 
                    backtrace depends on the symRef parameter.  If this is NULL, the 
                    strings persist until you call QBacktraceDisposeSymbols.  If it 
                    is not NULL, the strings persist until you dispose of the symbols 
                    object itself.
                    
                    This module assumes that the state of the target task is stable; 
                    if the task is still running, you may experienc odd artifacts 
                    (as, for example, we take a backtrace of a thread then try 
                    to find its symbols, only to determine that the Mach-O image 
                    has been unloaded in the interim).  The best way to prevent this 
                    from happening is to suspend the task while you're accessing it.  
                    The Mach routine task_suspend is very useful in this situation.
*/

/*!
    @enum           QBTFlags
    
    @abstract       Flags for a frame within a backtrace.
    
    @discussion     These flags provide information about a specific frame in 
                    a backtrace.

                    Note: The code also uses this field to store some internal 
                    flags.  Do not be alarmed if you see bit other than the 
                    ones shown below set in a QBTFlags value.

    @constant kQBTFrameBadMask
                    The frame pointer of this frame is bad (for example, no 
                    frame could be found or the frame pointer is misaligned or 
                    outside of the stack or references unmapped memory).

                    IMPORTANT: This flag is set for the last frame in the 
                    backtrace (where we've run off the end of the stack), but it 
                    can also be set for intermediate frames (where we've detected 
                    a frameless leaf routine, either at the top of the stack or 
                    as part of crossing a signal frame).
                    
    @constant kQBTPCBadMask
                    The program counter of this frame is bad (for example, the 
                    PC is misaligned or references unmapped memory).
                    
    @constant kQBTSignalHandlerMask
                    This frame represents the invocation of a signal handler.
*/
typedef int QBTFlags;
#define kQBTFrameBadMask        0x0001
#define kQBTPCBadMask           0x0002
#define kQBTSignalHandlerMask   0x0004
#if 0
enum QBTFlags {
	kQBTFrameBadMask      = 0x0001,
	kQBTPCBadMask         = 0x0002,
	kQBTSignalHandlerMask = 0x0004
};
#endif

/*!
    @struct         QBTFrame
    
    @abstract       Describes a frame within a backtrace.
    
    @discussion     The end result of a backtrace is an array of QBTFrame 
                    structures describing a particular frame in the backtrace.  

                    IMPORTANT:  The PC points to the code that's using the frame.  
                    It is not the return address for that code.  On architectures 
                    where the frame holds the return address (Intel, but not 
                    PowerPC), we do the appropriate corrections.
                    
    @field pc       The PC for this function invocation.

    @field fp       The frame pointer for this function invocation.

    @field flags    Various flags; see QBTFlags above.

    @field symbol   Name of the symbol containing this PC.  May be NULL.
    
                    IMPORTANT: The lifetime of this string is controlled by 
                    various factors; see the discussion above for details.

    @field library  File path of the library containing this PC.  May be NULL.
    
                    IMPORTANT: The lifetime of this string is controlled by 
                    various factors; see the discussion above for details.

    @field offset   Offset from the symbol to the PC.  Only valid if symbol 
                    is not NULL.
*/
struct QBTFrame {
	QTMAddr         pc;
	QTMAddr         fp;
	QBTFlags        flags;
    const char *    symbol;
    const char *    library;
    QTMOffset       offset;
};
typedef struct QBTFrame QBTFrame;

/*!
    @function       QBacktraceMachSelf
    
    @abstract       Returns a backtrace of the current thread.
    
    @discussion     Returns a backtrace of the current thread in the array 
                    described by frameArray and frameArrayCount.  The number 
                    of valid frames is returned in *frameCountPtr; this may 
                    be larger than frameArrayCount.

                    IMPORTANT: You may need to call QBacktraceDisposeSymbols 
                    on the resulting frames.  See the detailed discussion of this 
                    above.

    @param symRef   A symbols object for doing symbol to address translation 
                    (and vice versa).  This may be NULL, in which case the routine 
                    will internally create the symbols object.
                    
                    IMPORTANT: This parameter affects the lifecycle of the strings 
                    returned in the frames array.  See the discussion above for details.

    @param frameArray
                    A pointer to an array of frame structures; this routine places 
                    the backtrace into this array with the first element being the 
                    most recent function invocation.
                    
                    On entry, frameArray must not be NULL unless frameArrayCount 
                    is zero.
                    
                    On entry, if frameArray is not NULL, the contents of the array 
                    are ignored.  
                    
                    On success, if frameArray is not NULL, then 
                    MIN(frameArrayCount, *frameCountPtr) elements of the array 
                    contain information.  You must clean up these elements by 
                    calling QBacktraceDisposeSymbols.
                    
                    On error, you do not need to clean up this array.

    @param frameArrayCount
                    The size of this array pointed to be frameArray.  If this 
                    is zero, frameArray may be NULL.

    @param frameCountPtr
                    Returns the number of frames in the backtrace.  On entry, 
                    frameCountPtr must not be NULL and *frameCountPtr is ignored. 
                    On success, *frameCountPtr contains the number frames in the 
                    backtrace; this will not be zero.  On error, *frameCountPtr 
                    will be zero.
                    
                    IMPORTANT: On success, *frameCountPtr may be greater than 
                    frameArrayCount, in which case the backtrace contains more 
                    frames than can be returned in frameArray.  If you want to 
                    get all of the frames, you can allocate a bigger frame array 
                    and call this function again.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QBacktraceMachSelf(
    QSymbolsRef symRef,
	QBTFrame *	frameArray, 
	size_t		frameArrayCount, 
	size_t *	frameCountPtr
);

/*!
    @function       QBacktraceMachThread
    
    @abstract       Returns a backtrace of an arbitrary thread in an arbitrary process.
    
    @discussion     Returns a backtrace of the specified thread in the specified 
                    process.  Places the backtrace in the array described by 
                    frameArray and frameArrayCount.  The number of valid frames 
                    is returned in *frameCountPtr; this may be larger than 
                    frameArrayCount.

                    IMPORTANT: You may need to call QBacktraceDisposeSymbols 
                    on the resulting frames.  See the detailed discussion of this 
                    above.

    @param task     Must be the name of a valid send right for the task control 
                    port of the process to inspect; mach_task_self is just fine.
                    
                    If you do pass in mach_task_self, this routine automatically 
                    enables some nice optimisations.

    @param thread   Must be the name of a valid send right for the thread control 
                    port of the thread to inspect.  Passing in mach_thread_self is  
                    just fine, although you'd probably be better off calling 
                    QBacktraceMachSelf instead.

    @param cputype  The CPU type of the dynamic linker from which you want to get 
                    the symbols.  Typically you would pass CPU_TYPE_ANY to use 
                    the first dynamic linker that's discovered.  See 
                    QMOImageCreateFromTaskDyld for a detailed discussion of this 
                    value.

    @param symRef   A symbols object for doing symbol to address translation 
                    (and vice versa).  This may be NULL, in which case the routine 
                    will internally create the symbols object.
                    
                    IMPORTANT: This parameter affects the lifecycle of the strings 
                    returned in the frames array.  See the discussion above for details.

    @param stackBottom
                    This parameter, along with stackTop, defines the extent of the 
                    stack you are tracing.  If this information isn't handy, supply 
                    0 for both parameters.  Supplying meaningful values can reduce 
                    the number of bogus frames reported if the stack is corrupt.

                    stackBottom and stackTop must both be zero, or stackBottom 
                    must be strictly less than stackTop.	

    @param stackTop See the discussion of stackBottom.

    @param frameArray
                    A pointer to an array of frame structures; this routine places 
                    the backtrace into this array with the first element being the 
                    most recent function invocation.
                    
                    On entry, frameArray must not be NULL unless frameArrayCount 
                    is zero.
                    
                    On entry, if frameArray is not NULL, the contents of the array 
                    are ignored.  
                    
                    On success, if frameArray is not NULL, then 
                    MIN(frameArrayCount, *frameCountPtr) elements of the array 
                    contain information.  You must clean up these elements by 
                    calling QBacktraceDisposeSymbols.

                    On error, you do not need to clean up this array.

    @param frameArrayCount
                    The size of this array pointed to be frameArray.  If this 
                    is zero, frameArray may be NULL.

    @param frameCountPtr
                    Returns the number of frames in the backtrace.  On entry, 
                    frameCountPtr must not be NULL and *frameCountPtr is ignored. 
                    On success, *frameCountPtr contains the number frames in the 
                    backtrace; this will not be zero.  On error, *frameCountPtr 
                    will be zero.
                    
                    IMPORTANT: On success, *frameCountPtr may be greater than 
                    frameArrayCount, in which case the backtrace contains more 
                    frames than can be returned in frameArray.  If you want to 
                    get all of the frames, you can allocate a bigger frame array 
                    and call this function again.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
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
);

/*!
    @function       QBacktraceMachThreadState
    
    @abstract       Returns a backtrace of a thread state within an arbitrary process.
    
    @discussion     Returns a backtrace of the specified thread state in the specified 
                    process.  Places the backtrace in the array described by 
                    frameArray and frameArrayCount.  The number of valid frames 
                    is returned in *frameCountPtr; this may be larger than 
                    frameArrayCount.

                    IMPORTANT: You may need to call QBacktraceDisposeSymbols 
                    on the resulting frames.  See the detailed discussion of this 
                    above.

    @param task     Must be the name of a valid send right for the task control 
                    port of the process to inspect; mach_task_self is just fine.
                    
                    If you do pass in mach_task_self, this routine automatically 
                    enables some nice optimisations.

    @param stateFlavor
                    The thread state flavor of the thread state.  For example, 
                    you might pass in PPC_THREAD_STATE or x86_THREAD_STATE64.

    @param state    A pointer to a block of memory containing the thread state.
                    For example, if stateFlavor is PPC_THREAD_STATE, this would 
                    point to a ppc_thread_state_t.

    @param stateSize
                    The size of the thread state pointed to by the state parameter.
                    For example, if stateFlavor is PPC_THREAD_STATE, you could 
                    pass in sizeof(ppc_thread_state_t).  Equivalently, you could 
                    pass in PPC_THREAD_STATE_COUNT * sizeof(integer_t).

    @param cputype  The CPU type of the dynamic linker from which you want to get 
                    the symbols.  Typically you would pass CPU_TYPE_ANY to use 
                    the first dynamic linker that's discovered.  See 
                    QMOImageCreateFromTaskDyld for a detailed discussion of this 
                    value.

    @param symRef   A symbols object for doing symbol to address translation 
                    (and vice versa).  This may be NULL, in which case the routine 
                    will internally create the symbols object.
                    
                    IMPORTANT: This parameter affects the lifecycle of the strings 
                    returned in the frames array.  See the discussion above for details.

    @param stackBottom
                    This parameter, along with stackTop, defines the extent of the 
                    stack you are tracing.  If this information isn't handy, supply 
                    0 for both parameters.  Supplying meaningful values can reduce 
                    the number of bogus frames reported if the stack is corrupt.

                    stackBottom and stackTop must both be zero, or stackBottom 
                    must be strictly less than stackTop.	

    @param stackTop See the discussion of stackBottom.

    @param frameArray
                    A pointer to an array of frame structures; this routine places 
                    the backtrace into this array with the first element being the 
                    most recent function invocation.
                    
                    On entry, frameArray must not be NULL unless frameArrayCount 
                    is zero.
                    
                    On entry, if frameArray is not NULL, the contents of the array 
                    are ignored.  
                    
                    On success, if frameArray is not NULL, then 
                    MIN(frameArrayCount, *frameCountPtr) elements of the array 
                    contain information.  You must clean up these elements by 
                    calling QBacktraceDisposeSymbols.

                    On error, you do not need to clean up this array.

    @param frameArrayCount
                    The size of this array pointed to be frameArray.  If this 
                    is zero, frameArray may be NULL.

    @param frameCountPtr
                    Returns the number of frames in the backtrace.  On entry, 
                    frameCountPtr must not be NULL and *frameCountPtr is ignored. 
                    On success, *frameCountPtr contains the number frames in the 
                    backtrace; this will not be zero.  On error, *frameCountPtr 
                    will be zero.
                    
                    IMPORTANT: On success, *frameCountPtr may be greater than 
                    frameArrayCount, in which case the backtrace contains more 
                    frames than can be returned in frameArray.  If you want to 
                    get all of the frames, you can allocate a bigger frame array 
                    and call this function again.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QBacktraceMachThreadState(
	task_t					task, 
	thread_state_flavor_t	stateFlavor,
	const void *			state,
	size_t					stateSize,
    cpu_type_t				cputype,
    QSymbolsRef				symRef,
	QTMAddr					stackBottom, 
	QTMAddr					stackTop,
	QBTFrame *				frameArray, 
	size_t					frameArrayCount, 
	size_t *				frameCountPtr
);

/*!
    @function       QBacktraceDisposeSymbols
    
    @abstract       Disposes of the symbol and library strings in a frame array.  
                    It is safe to call this function on a frame array that is all 
                    zeroes.  It is safe to call this function on a frame array 
                    that has been passed to any of the backtrace routines, 
                    regardless of whether they succeeded or not.
    
    @discussion     Disposes of the symbol and library strings in the specified 
                    frame array.

    @param frameArray
                    A pointer to an array of frames with frameCount elements. 
                    You may pass NULL, in which case the routine does nothing.

    @param frameCount
                    The size of the array pointed to be frameArray.
*/
extern void QBacktraceDisposeSymbols(QBTFrame frameArray[], size_t frameCount); 

/*!
    @function       QBTCreateThreadState
    
    @abstract       Gets the thread state of the specified thread.
    
    @discussion     Allocates a memory block and stores the thread state of the 
                    specified thread into it.
                    
                    To get this information, the routine must inspect the task 
                    to see what architecture it's running.  Doing this requires a 
                    reference to an image object within the task.  If you pass 
                    in a symbols object, the routine will get it from that.  
                    Otherwise, it will create its own temporary image object 
                    internally, and to do that it needs you to tell it what 
                    CPU type you're interested in.

    @param task     Must be the name of a valid send right for the task control 
                    port of the process containing the thread; mach_task_self is 
                    just fine.

    @param thread   Must be the name of a valid send right for the thread control 
                    port of the thread whose state you want.  Passing mach_thread_self 
                    will work, although it won't yield useful results (because 
                    the thread state will be for the thread as it entered the 
                    kernel in the thread_get_state call, and that state is no 
                    longer useful; specifically, the frames that the state refers 
                    to have already been destroyed).

    @param cputype  This parameter is only consulted if the symRef is parameter 
                    is NULL.  In that case, the routine has to create an image 
                    object for some Mach-O image inside the task and this value 
                    controls which dynamic linker it finds.  Typically you would 
                    pass CPU_TYPE_ANY to use the first dynamic linker that's discovered.  
                    See QMOImageCreateFromTaskDyld for a detailed discussion of this 
                    value.

    @param symRef   A symbols object from which the routine can get an image object 
                    for the dynamic linker within the task.  This may be NULL, 
                    in which case the routine will internally create its own 
                    temporary image object.

    @param stateFlavorPtr
                    Must not be NULL.  On success, *stateFlavorPtr will contain 
                    the thread state flavor for the thread state that's being 
                    returned.

    @param statePtr
                    Must not be NULL.  On entry, *statePtr must be NULL.  On 
                    success, *statePtr will contain a pointer to a newly allocated 
                    thread state block.  The caller is responsible for freeing 
                    that memory.
                    
                    On error, *statePtr will be NULL.

    @param stateSizePtr
                    Must not be NULL.  On success, *stateSizePtr will contain 
                    the size of the thread state that's being returned.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QBTCreateThreadState(
	task_t                  task, 
	thread_t                thread,
    cpu_type_t              cputype,
    QSymbolsRef             symRef,
    thread_state_flavor_t * stateFlavorPtr,
    void **                 statePtr,
    size_t *                stateSizePtr
);

/*!
    @function       QBTCreateThreadStateSelf
    
    @abstract       Gets the thread state of the current thread.
    
    @discussion     Allocates a memory block and stores the thread state of the 
                    current thread into it.  This state contains only the 
                    information needed to do a backtrace; this is architecture 
                    dependent, but it is typically the stack pointer, frame pointer 
                    and program counter.  These correspond to the frame of the 
                    caller of this routine.
                    
                    IMPORTANT: This routine has the no-inline attribute because, 
                    if you inlined it, you'd break its semantics.

    @param stateFlavorPtr
                    Must not be NULL.  On success, *stateFlavorPtr will contain 
                    the thread state flavor for the thread state that's being 
                    returned.

    @param statePtr
                    Must not be NULL.  On entry, *statePtr must be NULL.  On 
                    success, *statePtr will contain a pointer to a newly allocated 
                    thread state block.  The caller is responsible for freeing 
                    that memory.
                    
                    On error, *statePtr will be NULL.

    @param stateSizePtr
                    Must not be NULL.  On success, *stateSizePtr will contain 
                    the size of the thread state that's being returned.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QBTCreateThreadStateSelf(
    thread_state_flavor_t * stateFlavorPtr,
    void **                 statePtr,
    size_t *                stateSizePtr
)  __attribute__((noinline));

#ifdef __cplusplus
}
#endif

#endif
