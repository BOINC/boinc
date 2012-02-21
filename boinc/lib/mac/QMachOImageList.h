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
 *  QMachOImageList.h
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
    File:       QMachOImageList.h

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

$Log: QMachOImageList.h,v $
Revision 1.1  2007/03/02 12:20:25         
First checked in.


*/

#ifndef _QMACHOIMAGELIST_H
#define _QMACHOIMAGELIST_H

/////////////////////////////////////////////////////////////////

#include "QMachOImage.h"

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
    extern "C" {
#endif

/*!
    @header         QMachOImageList.h
    
    @abstract       Gets all Mach-O images within a process.

    @discussion     This module provides routines to get all of the Mach-O images 
                    from a process.  The intricacies of the implementation are covered 
                    by the comments in the source.  The interface is, however, 
                    quite clean.  There are two functions, one to get a list of 
                    all of the images in the current process (QMOImageListFromSelf), 
                    and another to get a list of the images in an arbitrary process.

                    This module assumes that the state of the process you're 
                    investigating is stable.  That is, when you ask the module to 
                    create the process's image list, you need to guarantee that 
                    the dyld state of the process does not change while this module 
                    is looking at it.  The best way to prevent this from happening 
                    is to suspend the process while you're accessing it.  The Mach 
                    routine task_suspend is very useful in this situation.
*/

/*!
    @function       QMOImageListFromSelf
    
    @abstract       Returns a list of Mach-O images in the current process.
    
    @discussion     Just like QMOImageListForTask except that it returns the 
                    image list for the current task.

                    The implementation of this routine is much less scary than 
                    that of QMOImageListForTask.

    @param imageArray
                    May be NULL only if imageArrayCount is 0.  If not NULL, 
                    it must point to an array of imageArrayCount elements. 
                    On success, the array holds references to up to imageArrayCount 
                    image objects.  On error, each element of the array will 
                    be NULL.

    @param imageArrayCount
                    The size of the array pointed to by imageArray.  May be 
                    zero.
                    
    @param imageArrayCountPtr
                    Must not be NULL.  On entry, the value of *imageArrayCountPtr 
                    is ignored.  On success, *imageArrayCountPtr is the number of 
                    images in the target process.  On error, *imageArrayCountPtr is 
                    undefined.
                    
                    IMPORTANT: The value returned in *imageArrayCountPtr may be 
                    larger than imageArrayCount.  That means that the imageArray 
                    was not big enough to hold all of the images in the target 
                    process.  You can use the value in *imageArrayCountPtr to 
                    grow the array and thus get all of the images.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QMOImageListFromSelf(
    QMOImageRef *   imageArray, 
    size_t          imageArrayCount, 
    size_t *        imageCountPtr
);

/*!
    @function       QMOImageListFromTask
    
    @abstract       Returns a list of Mach-O images for an arbitrary process.
    
    @discussion     Returns a list of Mach-O images for the specified process.
        
    @param task     Must be the name of a valid send right for the task control 
                    port of the task whose images you want; mach_task_self is 
                    just fine.
                    
                    If you do pass in mach_task_self, this routine acts like you'd 
                    called QMOImageListFromSelf.  This automatically enables some 
                    nice optimisations.
                    
    @param cputype  The CPU type of the dynamic linker from which you want to get 
                    the image list.  Typically you would pass CPU_TYPE_ANY to use 
                    the first dynamic linker that's discovered.  See 
                    QMOImageCreateFromTaskDyld for a detailed discussion of this 
                    value.

    @param imageArray
                    May be NULL only if imageArrayCount is 0.  If not NULL, 
                    it must point to an array of imageArrayCount elements. 
                    On success, the array holds references to up to imageArrayCount 
                    image objects.  On error, each element of the array will 
                    be NULL.

    @param imageArrayCount
                    The size of the array pointed to by imageArray.  May be 
                    zero.
                    
    @param imageArrayCountPtr
                    Must not be NULL.  On entry, the value of *imageArrayCountPtr 
                    is ignored.  On success, *imageArrayCountPtr is the number of 
                    images in the target process.  On error, *imageArrayCountPtr is 
                    undefined.
                    
                    IMPORTANT: The value returned in *imageArrayCountPtr may be 
                    larger than imageArrayCount.  That means that the imageArray 
                    was not big enough to hold all of the images in the target 
                    process.  You can use the value in *imageArrayCountPtr to 
                    grow the array and thus get all of the images.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QMOImageListFromTask(
    task_t          task, 
    cpu_type_t      cputype, 
    QMOImageRef *   imageArray, 
    size_t          imageArrayCount, 
    size_t *        imageCountPtr
);

/*!
    @function       QMOImageListDestroy
    
    @abstract       Destroys an array of image objects.
    
    @discussion     Destroys an array of image objects.  Calls QMOImageDestroy 
                    for each element of the array.  Because QMOImageDestroy does 
                    nothing if its parameter is NULL, it's safe for elements of 
                    the array to be NULL.

    @param imageArray
                    May be NULL only if imageArrayCount is 0.  If not NULL, it 
                    must point to an array of imageArrayCount elements.

    @param imageArrayCount
                    The size of the array pointed to by imageArray.  May be 
                    zero.
*/
extern void QMOImageListDestroy(QMOImageRef *imageArray, size_t imageArrayCount);

#ifdef __cplusplus
}
#endif

#endif
