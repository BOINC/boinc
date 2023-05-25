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
 *  QCrashReport.h
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
    File:       QCrashReport.h

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

$Log: QCrashReport.h,v $
Revision 1.1  2007/03/02 12:20:01
First checked in.


*/

#ifndef _QCRASHREPORT_H
#define _QCRASHREPORT_H

/////////////////////////////////////////////////////////////////

#include <stdio.h>

// Put <mach/mach.h> inside extern "C" guards for the C++ build
// because the Mach header files don't always have them.

#if defined(__cplusplus)
	extern "C" {
#endif

#include <mach/mach.h>

#if defined(__cplusplus)
	}
#endif

#include "QMachOImage.h"
#include "QBacktrace.h"

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/*!
    @functiongroup  Crash Report Object
*/
/////////////////////////////////////////////////////////////////
#pragma mark ***** Crash Report Object

/*!
    @header         QCrashReport.h

    @abstract       High-level crash report object.

    @discussion     This module provides a high-level interface for creating and
                    recording crash report information.  It encapsulates all of
                    the complexity of many lower-level modules, providing you with
                    a nice, clean interface for creating crash reports.

                    To create a create report object, just call QCRCreateFromTask or
                    QCRCreateFromSelf.  Use QCRDestroy to destroy the resulting
                    object.  Use QCRGetThreadCount and QCRGetThreadAtIndex to
                    iterate the threads of the crash report, and
                    QCRGetBacktraceAtIndex to interate the backtraces for those
                    threads.  Call QCRGetSymbols and QCRGetImages to get at the
                    underlying symbols object and image objects.  Finally,
                    QCRPrintBacktraces, QCRPrintThreadState and QCRPrintImages
                    provide a standard way to render crash reports to text.

                    This module assumes that the state of the process you're
                    investigating is stable.  That is, when you create a crash report
                    object for a process, you need to guarantee that the state
                    of the process does not change while that object exists.
                    The best way to prevent this from happening is to suspend the
                    process while you're accessing it.  You can automatically
                    suspend the task for the lifetime of the crash report object by
                    pass true to the suspend parameter of QCRCreateFromTask.

                    One special case of this applies to crash report objects that
                    target the current process (mach_task_self).  In this case,
                    it's not feasible to suspend the task, a fact that makes for
                    some interesting challenges.  This module copes with that
                    in two ways:

                    o When you get a backtrace for a thread, it implicitly gets
                      the thread's state.  It then caches that state and the
                      associated backtrace.  From that point on, you get
                      a consistent backtrace, although it may not be correlated
                      with the current state of the thread.

                    o An exception to this is for the current thread
                      (mach_thread_self).  Unless you explicitly set its state
                      (using QCRSetThreadStateAtIndex), the act of calling
                      QCRGetBacktraceAtIndex causes the module to throw away
                      the cached backtrace for the thread and generate a new one
                      based on the thread's current state.

                    This has some interesting consequences on the life time of
                    certain memory blocks returned by the module.  Specifically:

                    o thread state returned by QCRGetThreadStateAtIndex

                    o backtrace frames returned by QCRGetBacktraceAtIndex

                    In both cases, this memory persists until one of the following
                    happens:

                      - you destroy the crash report object
                      - you explicitly set the thread state using QCRSetThreadStateAtIndex
                      - it's associated with the current thread and you generate
                        a new backtrace for the thread by calling QCRGetBacktraceAtIndex

                    In addition, this module provides some helper routines for
                    launching a crash report tool (QCRExecuteCrashReportTool) and
                    for giving that tool access to the task control port for the
                    crashed task (QCRGetParentTask).
*/

/*!
    @typedef        QCrashReportRef

    @abstract       A reference to the crash report object.

    @discussion     This type is opaque; to create, destroy, or access it, you must
                    use the routines in this module.
*/
typedef struct QCrashReport *QCrashReportRef;

/*!
    @functiongroup  Create and Destroy
*/
#pragma mark ***** Create and Destroy

/*!
    @function       QCRCreateFromTask

    @abstract       Creates a crash report object for an arbitrary process.

    @discussion     Creates a crash report object for the specified process.

    @param task     Must be the name of a valid send right for the task control
                    port of the process to inspect; mach_task_self is just fine.

                    If you do pass in mach_task_self, this routine automatically
                    enables some nice optimisations.

    @param suspend  If true, the target task is suspended while the crash report
                    object exists.

                    Must not be true if task is mach_task_self.

    @param crashedThread
                    The name of a valid send right for the thread control port of
                    the thread that crashed, or MACH_PORT_NULL if you don't have
                    this information handy.  If you pass in MACH_PORT_NULL, you
                    can also set the crashed thread later by calling
                    QCRSetCrashedThreadIndex.

                    It is acceptable to pass mach_thread_self to this parameter,
                    but it may not do what you expect.  See the discussion
                    above for details.

                    This value primarily affects the text rendering routines;
                    see the discussion in QCRSetCrashedThreadIndex for details.

    @param cputype  The CPU type for which you are creating a crash report.
                    Typically you would pass CPU_TYPE_ANY to use the CPU type of
                    the first dynamic linker that's discovered.  See
                    QMOImageCreateFromTaskDyld for a detailed discussion of this
                    value.

    @param crRefPtr On entry, crRefPtr must not be NULL and *crRefPtr must
                    be NULL.  On success, *crRefPtr will be a reference to the
                    crash report object that's been created.  On error, *crRefPtr
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QCRCreateFromTask(
    task_t              task,
    bool                suspend,
    thread_t            crashedThread,
    cpu_type_t          cputype,
    QCrashReportRef *   crRefPtr
);

/*!
    @function       QCRCreateFromSelf

    @abstract       Creates a crash report object for the current process.

    @discussion     This is equivalent to calling QCRCreateFromTask with
                    mach_task_self, false, mach_thread_self, and QMOGetLocalCPUType
                    for the first four parameters.

    @param crRefPtr On entry, crRefPtr must not be NULL and *crRefPtr must
                    be NULL.  On success, *crRefPtr will be a reference to the
                    crash report object that's been created.  On error, *crRefPtr
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QCRCreateFromSelf(QCrashReportRef *crRefPtr);

/*!
    @function       QCRDestroy

    @abstract       Destroys a crash report object.

    @discussion     Destroys the supplied crash report object.

    @param qmoImage The crash report object to destroy.  If this is NULL, the routine
                    does nothing.
*/
extern void QCRDestroy(QCrashReportRef crRef);

/*!
    @functiongroup  Accessors
*/
#pragma mark ***** Accessors

/*!
    @function       QCRGetSymbols

    @abstract       Gets the symbols for this crash report.

    @discussion     Gets the symbols object for this crash report object.

    @param crRef    A valid crash report object.

    @result         A reference to the symbols object.  This will never be
                    NULL (it is created when the crash report object is created,
                    and any failures would have caused creation to fail).  It
                    exists until you destroy the crash report object.
*/
extern QSymbolsRef QCRGetSymbols(QCrashReportRef crRef);

/*!
    @function       QCRGetImages

    @abstract       Gets the images for this crash report.

    @discussion     Gets the array of image objects for this crash report object.

                    This routine can never fail; the image array is created when
                    the crash report object is created, and any failures would
                    have caused creation to fail.

                    IMPORTANT: Do not free the array that this returns, or indeed
                    any of the image objects contained in the array.  These all
                    belongs to the crash report object and will be destroyed when
                    the crash report object is destroyed.

    @param crRef    A valid crash report object.

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
extern void QCRGetImages(
    QCrashReportRef     crRef,
    QMOImageRef **      imageArrayPtr,
    size_t *            imageCountPtr
);

/*!
    @function       QCRGetThreadCount

    @abstract       Returns the number of threads in this crash report.

    @discussion     Return the number of threads running in the process for which
                    this crash report was created.

    @param crRef    A valid crash report object.

    @result         A count of the number of threads.
*/
extern size_t QCRGetThreadCount(QCrashReportRef crRef);

/*!
    @function       QCRGetThreadAtIndex

    @abstract       Returns the specified thread for a crash report.

    @discussion     Returns the thread specified by threadIndex for the crash
                    report.

    @param crRef    A valid crash report object.

    @param threadIndex
                    A zero-based index into the array of threads.

    @result         Returns the name of a send right for the thread control
                    port.  This right is owned by the crash report object;
                    you must not release it and it goes away when you destroy
                    the crash report object (although you may have other
                    references to the right).
*/
extern thread_t QCRGetThreadAtIndex(QCrashReportRef crRef, size_t threadIndex);

/*!
    @function       QCRGetThreadStateAtIndex

    @abstract       Gets the thread state of the specified thread.

    @discussion     Returns the state of the specified thread in the crash
                    report object.  The first time you call this function
                    (either explicitly, or implicitly by calling functions that
                    need the thread state), it takes a snapshot of the thread
                    state.  By the time the function returns, this snapshot is
                    valid only if the thread (or the entire task) is suspend.
                    Also, the lifetime of this snapshot is controlled by a number
                    of factors; see the detailed discussion of this topic,
                    above.

                    If you have explicitly set the thread state (by calling
                    QCRSetThreadStateAtIndex) you will get back the values you
                    set most recently.

    @param crRef    A valid crash report object.

    @param threadIndex
                    A zero-based index into the array of threads.

                    If this specifies the current thread (that is, QCRGetThreadAtIndex
                    for this index returns mach_thread_self), you probably won't
                    get useful results.  See the discussion above for details.

    @param cpuTypePtr
                    If NULL, this parameter is ignored.  Otherwise,
                    on entry, the value of *cpuTypePtr is ignored.  On success,
                    *cpuTypePtr will be the CPU type of the process that contains
                    the thread (for example, CPU_TYPE_X86).  This will be the same
                    for all threads within the process.  On error, the value in
                    *cpuTypePtr is undefined.

    @param threadStateFlavorPtr
                    If NULL, this parameter is ignored.  Otherwise,
                    on entry, the value of *threadStateFlavorPtr is ignored.
                    On success, *threadStateFlavorPtr will be the thread state
                    flavor of the thread's state (for example, x86_THREAD_STATE32).
                    On error, the value in *threadStateFlavorPtr is undefined.

    @param threadStatePtr
                    If NULL, this parameter is ignored.  Otherwise,
                    on entry, the value of *threadStatePtr is ignored.
                    On success, *threadStatePtr will contain a pointer to the
                    thread's state (for example, you could treat this as a pointer
                    to a x86_thread_state32_t structure).  On error, the value in
                    *threadStatePtr is undefined.

                    IMPORTANT: The lifetime of this pointer is controlled by
                    a number of factors.  See the detailed discussion of this
                    topic, above.

    @param threadStateSizePtr
                    If NULL, this parameter is ignored.  Otherwise,
                    on entry, the value of *threadStateSizePtr is ignored.
                    On success, *threadStateSizePtr will be the size, in bytes,
                    of thread's state (for example, sizeof(x86_thread_state32_t)).
                    On error, the value in *threadStateSizePtr is undefined.

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If threadIndex is out of range, this will be EINVAL.
*/
extern int QCRGetThreadStateAtIndex(
    QCrashReportRef         crRef,
    size_t                  threadIndex,
    cpu_type_t *            cpuTypePtr,
    thread_state_flavor_t * threadStateFlavorPtr,
    const void **           threadStatePtr,
    size_t *                threadStateSizePtr
);

/*!
    @function       QCRSetThreadStateAtIndex

    @abstract       Set the thread state for the specified thread.

    @discussion     Sets the thread state of the specified thread within the
                    the crash report.  This is useful if you have a better idea
                    of the thread's state than the default mechanism for getting
                    the thread state used by QCRGetThreadStateAtIndex (which gets
                    it by calling the Mach routine thread_get_state).  For example,
                    if you're running in a signal handler, you can extract the
                    interrupted thread's state from the signal handlers parameters.

                    If you set a thread's state by calling this routine, it
                    persists for the lifetime of the crash report object (unless
                    you override it by calling this routine again).

                    The memory buffer specified by threadState and threadStateSize
                    is copied by this routine; you do not need to maintain it
                    once the routine has returned.

    @param crRef    A valid crash report object.

    @param threadIndex
                    A zero-based index into the array of threads.

                    It is both acceptable and useful to specify the current
                    thread here (that is, pass in a value for which QCRGetThreadAtIndex
                    would return mach_thread_self).

    @param threadStateFlavor
                    Must be the thread state flavor of the new thread state
                    (for example, x86_THREAD_STATE32).

    @param threadState
                    Must be a pointer to the new thread state (for example, a
                    pointer to a x86_thread_state32_t structure).  NULL is not
                    acceptable.

    @param threadStateSize
                    The size, in bytes, of the new thread state pointed to by
                    threadState (for example, sizeof(x86_thread_state32_t)).  Due
                    to the nature of Mach thread states, this must not be zero and
                    must be an even multiple of sizeof(integer_t).

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If threadIndex is out of range, this will be EINVAL.
*/
extern int QCRSetThreadStateAtIndex(
    QCrashReportRef         crRef,
    size_t                  threadIndex,
    thread_state_flavor_t   threadStateFlavor,
    const void *            threadState,
    size_t                  threadStateSize
);

/*!
    @function       QCRGetBacktraceAtIndex

    @abstract       Returns a backtrace for the specified thread.

    @discussion     Returns a backtrace for the specified thread.  The first time
                    you call this function (except when specifying the current
                    thread, see below), it takes a snapshot of the thread's
                    backtrace.  For each subsequent call, you get back the
                    same backtrace.  This lifetime of this backtrace is
                    controlled by a number of factors; see the detailed
                    discussion of this topic, above.

                    This routine has a special case so that you can easily get a
                    meaningful backtrace for the current thread.  If you specify
                    the current thread (that is, you pass in a value for which
                    QCRGetThreadAtIndex would return mach_thread_self) and you
                    have not explicitly set the thread state (by calling
                    QCRSetThreadStateByIndex), this routine will return a backtrace
                    for the current thread at the current time.  This backtrace
                    will persist until you call this function again (or until any
                    of the other events that can clear the backtrace occurs; see
                    the detailed discussion above).

                    If you call this for anything except the current thread,
                    it implicitly snapshots the current thread state; this ensures
                    that the current thread state and the backtrace are in sync.

                    IMPORTANT: The lifetime of the array returned by this routine
                    is controlled by a number of factors.  See the detailed
                    discussion of this topic, above.

                    IMPORTANT: Do not free the array that this returns, or indeed
                    any of the items in the array.  These all belongs to the crash
                    report object and will be destroyed when the crash report
                    object is destroyed.

    @param crRef    A valid crash report object.

    @param threadIndex
                    A zero-based index into the array of threads.

    @param frameArrayPtr
                    Must not be NULL.  On entry, the value of *frameArrayPtr is ignored.
                    On success, *frameArrayPtr will contain a pointer to an array
                    of *frameCountPtr QBTFrame objects.  On error, the value in
                    *frameArrayPtr is undefined.

    @param frameCountPtr
                    Must not be NULL.  On entry, the value of *frameCountPtr is
                    ignored.  On success, *frameCountPtr will be the number of
                    valid elements in the array pointed to be *frameArrayPtr.
                    On error, the value in *frameCountPtr is undefined.

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If threadIndex is out of range, this will be EINVAL.
*/
extern int QCRGetBacktraceAtIndex(
    QCrashReportRef     crRef,
    size_t              threadIndex,
    const QBTFrame **   frameArrayPtr,
    size_t *            frameCountPtr
);

enum {
    kQCRNoThread = (size_t) -1
};

/*!
    @function       QCRGetCrashedThreadIndex

    @abstract       Returns the crashed thread.

    @discussion     Returns the index of the crashed thread.  Unless you've
                    explicitly set this (using QCRSetCrashedThreadIndex), this
                    value is determined as follows:

                    1. If you specified MACH_PORT_NULL when you created the crash
                       report object, the result will be kQCRNoThread.

                    2. Otherwise, the result will be the index of the thread that
                       matches the thread that you specified.

    @param crRef    A valid crash report object.

    @result         The index of the crashed thread, or kQCRNoThread if none
                    was specified.
*/
extern size_t QCRGetCrashedThreadIndex(QCrashReportRef crRef);

/*!
    @function       QCRSetCrashedThreadIndex

    @abstract       Sets the crashed thread.

    @discussion     Sets the index of the crashed thread.  This value can be
                    specified initially by passing a value to the crashedThread
                    parameter of QCRCreateFromTask.  However, you can use this
                    function to override this value at any time.

                    The information primarily affects the text rendering routines.
                    The crashed thread will be highlighted by QCRPrintBacktraces.
                    Also, QCRPrintThreadState prints the thread state of the
                    crashed thread.

    @param crRef    A valid crash report object.

    @param threadIndex
                    A zero-based index into the array of threads.

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If threadIndex is out of range, this will be EINVAL.
*/
extern int QCRSetCrashedThreadIndex(QCrashReportRef crRef, size_t threadIndex);

/*!
    @functiongroup  Text Rendering
*/
#pragma mark ***** Text Rendering

/*!
    @function       QCRPrintBacktraces

    @abstract       Prints backtraces of each thread in the crash report.

    @discussion     This function is designed to ape the backtraces section of
                    a CrashReporter crash log as much as possible.

    @param crRef    A valid crash report object.

    @param f        The file to print to.  If you want to print to something
                    other than a file, create a custom (FILE *) object using
                    <x-man-page://3/funopen>.
*/
extern void QCRPrintBacktraces(QCrashReportRef crRef, FILE *f);

/*!
    @function       QCRPrintThreadState

    @abstract       Prints the thread state of the crashed thread.

    @discussion     This function is designed to ape the thread state section of
                    a CrashReporter crash log as much as possible.

    @param crRef    A valid crash report object.

    @param f        The file to print to.  If you want to print to something
                    other than a file, create a custom (FILE *) object using
                    <x-man-page://3/funopen>.
*/
extern void QCRPrintThreadState(QCrashReportRef crRef, FILE *f);

/*!
    @function       QCRPrintImages

    @abstract       Prints the dyld images of the crashed thread.

    @discussion     This function is designed to ape the "Binary Images Description"
                    section of a CrashReporter crash log as much as possible.

    @param crRef    A valid crash report object.

    @param f        The file to print to.  If you want to print to something
                    other than a file, create a custom (FILE *) object using
                    <x-man-page://3/funopen>.
*/
extern void QCRPrintImages(QCrashReportRef crRef, FILE *f);

/*!
    @functiongroup  Tool Helpers
*/
/////////////////////////////////////////////////////////////////
#pragma mark ***** Tool Helpers

/*!
    @function       QCRExecuteCrashReportTool

    @abstract       Runs a crash report tool, giving it access to our task.

    @discussion     This function runs a crash report tool against the current
                    task in such a way that the crash report tool can get a
                    send right to the current process's task control port by
                    calling QCRGetParentTask.

                    IMPORTANT: This routine isn't thread safe.  To do its work
                    it must switch the current task's bootstrap port, fork/exec
                    the tool, and then switch the port back.  This can't be
                    done in a thread-safe way on current versions of Mac OS X.

    @param toolArgs An array of arguments for the tool.  The array must be
                    terminated by a NULL entry.  The first entry must contain
                    the path to the tool.

    @param pidPtr   If NULL, the tool runs synchronously.  That is, the routine
                    waits for the tool to exit before returning.  In that case,
                    toolStatusPtr must not be NULL.

                    If non-NULL, the tools runs asynchronously and the routine
                    returns the process ID of the tool so that you can wait
                    for it to complete.  On entry, *pidPtr must be -1.  On
                    success, *pidPtr will be the process ID of the running tool
                    (that is, not -1).  On error, *pidPtr will be -1.

    @param toolStatusPtr
                    If the tool is run synchronously, the routine returns the
                    tool's exit status here.  This is the value returned from
                    its main function (typically EXIT_SUCCESS or EXIT_FAILURE).

                    In the synchronous case, toolStatusPtr must not be NULL.
                    On entry, *toolStatusPtr is ignore.  On success,
                    *toolStatusPtr contains the tool's exit status.  On error,
                    the value of *toolStatusPtr is undefined.

                    In the asynchronous case, toolStatusPtr must be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QCRExecuteCrashReportTool(const char *toolArgs[], pid_t *pidPtr, int *toolStatusPtr);

/*!
    @function       QCRGetParentTask

    @abstract       Returns the task control port for the parent task.

    @discussion     For a crash report tool executed by QCRExecuteCrashReportTool,
                    this routine returns the name of a send right for the parent
                    process's task control port.  This allows you to interrogate
                    the parent process using routines like QCRCreateFromTask.

    @param taskPtr  Must not be NULL.  On entry, *taskPtr must be MACH_PORT_NULL.
                    On success, *taskPtr will be the name of a send right for the
                    parent process's task control port.  On error, *taskPtr will
                    be MACH_PORT_NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QCRGetParentTask(task_t *taskPtr);

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
