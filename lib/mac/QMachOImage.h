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
 *  QMachOImage.h
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
    File:       QMachOImage.h

    Contains:   Mach-O image access.

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

$Log: QMachOImage.h,v $
Revision 1.1  2007/03/02 12:20:18         
First checked in.


*/

#ifndef _QMACHOIMAGE_H
#define _QMACHOIMAGE_H

/////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <mach-o/loader.h>

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
// Fixes for compiling on 10.3 and earlier                     //
/////////////////////////////////////////////////////////////////
#ifdef DARWIN_10_3
#define nlist_64 nlist
#define mach_header_64 mach_header
#define segment_command_64 segment_command
#define section_64 section
#define reserved3 reserved2
#define MH_MAGIC_64 0x0ffffff0
#define MH_CIGAM_64 0x00ffffff
#define LC_SEGMENT_64 0x00fffff9
#undef MH_CIGAM
#define MH_CIGAM 0xcefaedfe
#endif
/////////////////////////////////////////////////////////////////

#include "QTaskMemory.h"            // for the QTMAddr type

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
    extern "C" {
#endif

/*!
    @header         QMachOImage.h
    
    @abstract       Mach-O image access.

    @discussion     This module gives you read access to a Mach-O image, either 
                    on disk or in memory.  The module allows you to create an 
                    object (denoted by the QMOImageRef type) for a Mach-O image 
                    and then query that image object for various bits of information 
                    about an image.  For example, you can ask whether the image 
                    is 64-bit (QMOImageIs64Bit), how many segments it has 
                    (QMOImageGetSegmentCount), and what symbols it uses 
                    (QMOImageIterateSymbols).
                    
                    You can create an image object from:
                    
                    o from a Mach-O file on disk (QMOImageCreateFromFile)
                    o from a running image in the local process (QMOImageCreateFromLocalImage)
                    o from a running image in another process (QMOImageCreateFromTask)
                    o from the dynamic link in another process (QMOImageCreateFromTaskDyld)
                    
                    In the last three cases, the image is considered to be prepared.  
                    Prepared images work slightly differently from file-based (that 
                    is, non-prepared) ones.  For example, the image slide 
                    (QMOImageGetSlide) is always zero for a file-based image.
                    
                    Unless you know that the image is local, you have to be careful 
                    to access the object in an architecture independent way.  The 
                    image may have a different pointer size (QMOImageIs64Bit) or 
                    byte order (QMOImageIsByteSwapped).  Many image object accessors 
                    automatically take care of this for you.  For example, the 
                    QMOImageGetSegmentByName always returns segment information as 
                    in a 64-bit capable structure, even if the underlying image is 
                    32-bit, and the fields of that structure are always in your 
                    native byte order.  However, some routines (for example, 
                    QMOImageGetMachHeader) give you direct access to the underlying 
                    image; in that case, you have to be careful to access the image 
                    in an architecture neutral way.  There routines, like 
                    QMOImageToLocalUInt32, to help you do this.
                    
                    When you create a prepared image object, this module assumes 
                    that the state of the task in which the object resides is stable. 
                    That is, if you create an image object for an image within a 
                    task and the code in that task unloads the image, all bets are 
                    off.  The best way to prevent this from happening is to suspend 
                    the task while you're accessing it.  The Mach routine 
                    task_suspend is very useful in this situation.
                    
                    Some routines in this module (for example, QMOImageGetFilePath) 
                    return you pointers to structures that are embedded within the 
                    image object.  Such pointers are only valid for the lifetime 
                    of the image object itself.  Once you've destroyed the image 
                    object (by calling QMOImageDestroy), you can no longer rely 
                    on the validity of these pointers.
*/

/*!
    @typedef        QMOImageRef
    
    @abstract       A reference to a Mach-O image object.
    
    @discussion     This type is opaque; to create, destroy, or access it, you must 
                    use the routines in this module.
*/
typedef struct QMOImage * QMOImageRef;

/*!
    @functiongroup  Create and Destroy
*/
#pragma mark ***** Create and Destroy

/*!
    @function       QMOImageCreateFromFile
    
    @abstract       Creates an image object for a Mach-O file.
    
    @discussion     Create an image object based on the contents of the Mach-O file 
                    at filePath.  This handles both plain Mach-O files and fat files.  
                    If you want to specify a particular architecture within a fat 
                    file, pass the appropriate values to cputype and cpusubtype.
                    
                    Once you're done with the object, call QMOImageDestroy to 
                    destroy it.
                    
    @param filePath The BSD path to the Mach-O file.  This must not be NULL.
    
    @param cputype  Specifies, along with cpusubtype, the architecture of the 
                    Mach-O image you're looking for.
                    
                    You can pass CPU_TYPE_ANY, which says that you don't care 
                    what architecture you get.  In this case, the routine will 
                    try to find a Mach-O image that's most compatible with the 
                    current architecture but, if that fails, will return an 
                    arbitrary architecture.
                    
                    If you pass a value other than CPU_TYPE_ANY, you are guaranteed 
                    to get an architecture that's compatible with that CPU type 
                    or an error.
    
    @param cpusubtype
                    Specifies, along with cputype, the architecture of the 
                    Mach-O image you're looking for.  Pass 0 as a don't care 
                    value.
                    
                    You must pass 0 to this parameter if you pass CPU_TYPE_ANY to 
                    the cputype parameter.
                    
    @param qmoImagePtr
                    On entry, qmoImagePtr must not be NULL and *qmoImagePtr must 
                    be NULL.  On success, *qmoImagePtr will be a reference to the 
                    image object that's been created.  On error, *qmoImagePtr 
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QMOImageCreateFromFile(
    const char *    filePath, 
    cpu_type_t      cputype, 
    cpu_subtype_t   cpusubtype, 
    QMOImageRef *   qmoImagePtr
);



/*!
    @function       QMOImageCreateFromLocalImage
    
    @abstract       Creates an image object from a Mach-O image in the current process.
    
    @discussion     Creates an image object from a Mach-O image running in the current 
                    process.

                    Once you're done with the object, call QMOImageDestroy to 
                    destroy it.
                    
    @param machHeader
                    The address of the (struct mach_header) of the Mach-O image 
                    in the current process.  This /could/ be NULL, although its 
                    very unlikely.

                    If the current process is 64-bit, this parameter would actually 
                    point to (struct mach_header_64) not a (struct mach_header).  
                    The code automatically figures out whether the image is 64-bit 
                    or not.  However, I had to define the routine to take one or 
                    the other type, so I chose (struct mach_header).

    @param filePath The BSD path to the Mach-O file that backs this image. It's 
                    possible for an image not to have a backing file (if, for example, 
                    the image was prepared directly from memory), so it's OK to 
                    pass in NULL.
                    
                    If you do pass in NULL, the routine QMOImageGetFilePath will 
                    return NULL.

    @param qmoImagePtr
                    On entry, qmoImagePtr must not be NULL and *qmoImagePtr must 
                    be NULL.  On success, *qmoImagePtr will be a reference to the 
                    image object that's been created.  On error, *qmoImagePtr 
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QMOImageCreateFromLocalImage(
    const struct mach_header *  machHeader, 
    const char *                filePath,
    QMOImageRef *               qmoImagePtr
);

/*!
    @function       QMOImageCreateFromTask
    
    @abstract       Creates an image object from a Mach-O image in an arbitrary process.
    
    @discussion     Creates an image object from a Mach-O image running in an 
                    arbitrary process.

                    Once you're done with the object, call QMOImageDestroy to 
                    destroy it.
                    
    @param task     Must be the name of a valid send right for the task control 
                    port of the task in which the image resides; mach_task_self 
                    is just fine.
                    
                    If you do pass in mach_task_self, this routine acts like you'd 
                    create the image using QMOImageCreateFromLocalImage.  This 
                    automatically enables some nice optimisations.

    @param machHeader
                    The address of the (struct mach_header) (or 
                    (struct mach_header_64)) of the Mach-O image in the specified 
                    process.  This /could/ be NULL, although its very unlikely.

    @param filePath The BSD path to the Mach-O file that backs this image. It's 
                    possible for an image not to have a backing file (if, for example, 
                    the image was prepared directly from memory), so it's OK to 
                    pass in NULL.
                    
                    If you do pass in NULL, the routine QMOImageGetFilePath will 
                    return NULL.

    @param qmoImagePtr
                    On entry, qmoImagePtr must not be NULL and *qmoImagePtr must 
                    be NULL.  On success, *qmoImagePtr will be a reference to the 
                    image object that's been created.  On error, *qmoImagePtr 
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QMOImageCreateFromTask(
    task_t          task, 
    QTMAddr         machHeader, 
    const char *    filePath,
    QMOImageRef *   qmoImagePtr
);

/*!
    @function       QMOImageCreateFromTaskDyld
    
    @abstract       Creates an image object for the dynamic linker of a particular process.
    
    @discussion     Creates an image object for the dynamic linker of a particular 
                    process.  It's rare that you'd need to call this routine; it's 
                    mainly exported for the benefit of the QMachOImageList module.

                    IMPORTANT: Locating the dynamic linker within a remote process 
                    is a difficult problem, and it could take some time.
                    
                    Once you're done with the object, call QMOImageDestroy to 
                    destroy it.
                    
    @param task     Must be the name of a valid send right for the task control 
                    port of the task in which the image resides; mach_task_self 
                    is just fine.
                    
                    If you do pass in mach_task_self, this routine automatically 
                    enables some nice optimisations.

    @param cputype  The CPU type of the dynamic linker that you're looking for. 
                    In many cases, you can just pass in CPU_TYPE_ANY.  If you want 
                    to guarantee that you'll get the same CPU type as the current 
                    process, use the result of QMOGetLocalCPUType.
                    
                    This parameter is necessary beacuse it is possible for 
                    a process to be running more than one dynamic linker.  
                    The most common example is a process being run using 
                    Rosetta.  In this case, there's a native dynamic linker that's 
                    managing the native code in the process, and a PowerPC dynamic 
                    linker that's managing your code.
                    
                    To handle this case, this routine lets you specify the CPU 
                    type of the dynamic linker you're looking for.  In the 
                    typical case (where you're inspecting a native process), you 
                    can pass in CPU_TYPE_ANY.  However, if you know the process 
                    is being run using Rosetta, you can pass in a value of 
                    CPU_TYPE_X86 to look at the native dynamic linker or 
                    CPU_TYPE_POWERPC to look at the PowerPC one. 
                    
                    If you pass in CPU_TYPE_ANY, the behaviour depends on the 
                    process type.  For a native process, CPU_TYPE_ANY will cause  
                    the routine to use the first dynamic linker it finds.  However, 
                    for a Rosetta process, it will prefer the PowerPC dynamic 
                    linker, only returning the Intel dynamic linker if it can't 
                    find the PowerPC one.

    @param qmoImagePtr
                    On entry, qmoImagePtr must not be NULL and *qmoImagePtr must 
                    be NULL.  On success, *qmoImagePtr will be a reference to the 
                    image object that's been created.  On error, *qmoImagePtr 
                    will be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
*/
extern int QMOImageCreateFromTaskDyld(
    task_t          task, 
    cpu_type_t      cputype, 
    QMOImageRef *   qmoImagePtr
);

/*!
    @function       QMOImageDestroy
    
    @abstract       Destroys an image object.
    
    @discussion     Destroys the supplied image object.
                    
    @param qmoImage The image object to destroy.  If this is NULL, the routine 
                    does nothing.
*/
extern void QMOImageDestroy(QMOImageRef qmoImage);


/*!
    @functiongroup  Basic Accessors
*/
#pragma mark ***** Basic Accessors

/*!
    @function       QMOImageGetSlide
    
    @abstract       Gets the image's slide.
    
    @discussion     Gets the slide associated with an image.  In this case, 
                    slide has the traditional Mach-O definition, that it, it is 
                    the amount that you have to add to the virtual addresses stored 
                    in the Mach-O file to get to the actual virtual addresses 
                    being used by the prepared image.
                    
                    For a file-based image, the slide is always assumed to be 0.
                    
    @param qmoImage Must be a valid image object.

    @result         The object's slide.
*/
extern QTMAddr	QMOImageGetSlide(QMOImageRef qmoImage);

/*!
    @function       QMOImageIs64Bit
    
    @abstract       Reports whether the image is 64-bit.
    
    @discussion     Returns true if the image is a 64-bit image.  This information 
                    is based on the Mach-O image, not on the process in which the 
                    image is running (if any).  Current versions of Mac OS X 
                    don't let you mix 32- and 64-bit images within a process, 
                    so that distinction is irrelevant at the moment.
                    
    @param qmoImage Must be a valid image object.

    @result         Returns true if the image is 64-bit; returns false otherwise.
*/
extern bool		QMOImageIs64Bit(QMOImageRef qmoImage);
	
/*!
    @function       QMOImageIsByteSwapped
    
    @abstract       Reports whether the image is byte swapped.
    
    @discussion     Returns true if the image is byte swapped relative to the 
                    calling process.  For example, if you're running on a PowerPC 
                    and you create an image for file-based Intel executable, this 
                    will return true.
                    
                    In general, you shouldn't care whether the image is byte 
                    swapped.  In most cases, this module will automatically 
                    return data in native endian format.  If you're calling the 
                    few routines in this module that don't do that, you can use 
                    helper routines, like QMOImageToLocalUInt32, which 
                    automatically byte swap if necessary.
                    
    @param qmoImage Must be a valid image object.

    @result         Returns true if the image is byte swapped relative to the 
                    current process; returns false otherwise.
*/
extern bool		QMOImageIsByteSwapped(QMOImageRef qmoImage);

/*!
    @function       QMOImageGetMachHeaderOffset
    
    @abstract       Returns the address of the image's Mach-O header.
    
    @discussion     Returns the offset of the image's Mach-O header within its 
                    container.  For a prepared image, this is just the address 
                    of the Mach-O header.  For a file-based image, this is the 
                    offset within the file.
                    
    @param qmoImage Must be a valid image object.

    @result         The offset of the image's Mach-O header within its 
                    container.
*/
extern QTMAddr QMOImageGetMachHeaderOffset(QMOImageRef qmoImage);

/*!
    @function       QMOImageGetMachHeader
    
    @abstract       Returns a pointer to the image's Mach-O header.
    
    @discussion     Returns a pointer, in the current process's address space, to 
                    the image's Mach-O header.

                    IMPORTANT: The (struct mach_header *) may actually be a 
                    (struct mach_header_64 *) depending on the result of QMOImageIs64Bit.

                    IMPORTANT: The fields of the returned (struct mach_header *) 
                    may be byte swapped.  Use the QMOImageToLocalUIntX routines 
                    to access them.
                    
    @param qmoImage Must be a valid image object.

    @result         A pointer to the Mach-O header of the image.  If the image 
                    is local, this will point to the Mach-O header of the image 
                    in the current process.  If the image is remote or file-based, 
                    this will be a copy of the Mach-O header.
*/
extern const struct mach_header * QMOImageGetMachHeader(QMOImageRef qmoImage);

/*!
    @function       QMOImageGetFilePath
    
    @abstract       Returns a path to the Mach-O file containing the image.
    
    @discussion     There is no magic here.  The value you get back is just a 
                    copy of the value you passed in when you created the image.
                    
    @param qmoImage Must be a valid image object.

    @result         A pointer to a BSD path for the Mach-O file containing the 
                    image.  This may be NULL.
                    
                    Do not try to free this pointer.  It belongs to the image 
                    object and will be freed when you destroy the image object.
*/
extern const char * QMOImageGetFilePath(QMOImageRef qmoImage);

/*!
    @function       QMOImageGetFileType
    
    @abstract       Returns the Mach-O file type.
    
    @discussion     Returns the Mach-O file type per the filetype field of the 
                    (struct mach_header).  Typical values are MH_EXECUTE, 
                    MH_BUNDLE, and so on.
                    
    @param qmoImage Must be a valid image object.

    @result         The Mach-O file type (as native endian).
*/
extern uint32_t QMOImageGetFileType(QMOImageRef qmoImage);

/*!
    @function       QMOImageGetCPUType
    
    @abstract       Returns the Mach-O CPU type.
    
    @discussion     Returns the Mach-O CPU type per the cputype field of the 
                    (struct mach_header).  Typical values are CPU_TYPE_POWERPC, 
                    CPU_TYPE_X86, and so on.
                    
    @param qmoImage Must be a valid image object.

    @result         The Mach-O CPU type (as native endian).
*/
extern uint32_t QMOImageGetCPUType(QMOImageRef qmoImage);

/*!
    @function       QMOImageGetCPUSubType
    
    @abstract       Returns the Mach-O CPU subtype.
    
    @discussion     Returns the Mach-O CPU subtype per the cpusubtype field of the 
                    (struct mach_header).  Typical values are CPU_SUBTYPE_POWERPC_970, 
                    CPU_SUBTYPE_X86_ALL, and so on.
                    
    @param qmoImage Must be a valid image object.

    @result         The Mach-O CPU subtype (as native endian).
*/
extern uint32_t QMOImageGetCPUSubType(QMOImageRef qmoImage);

/*!
    @function       QMOImageFindLoadCommandByID
    
    @abstract       Finds a load command within an image.
    
    @discussion     Finds a load command within an image.  cmdID is the command 
                    that you're trying to find (for example, LC_UNIXTHREAD, 
                    LC_SYMTAB, and so on).  Returns the address of the first 
                    instance of that command ID, or NULL if that command is not 
                    found.

                    IMPORTANT:  The fields of the return (struct load_command *) 
                    may be byte swapped.  Use the QMOImageToLocalUIntX routines 
                    to access them.
                    
    @param qmoImage Must be a valid image object.

    @param cmdID    The load command ID to look for.  

    @result         A pointer to the load command.  This will be NULL if the 
                    command is not found.
*/
extern const struct load_command * QMOImageFindLoadCommandByID(
    QMOImageRef     qmoImage, 
    uint32_t        cmdID
);

/*!
    @function       QMOImageGetSegmentCount
    
    @abstract       Returns the number of segments in the image.
    
    @discussion     Returns the number of segments in the image.  This is a 
                    count of the number of LC_SEGMENT commands in the image.
                    
    @param qmoImage Must be a valid image object.

    @result         A count of the segments in the image.
*/
extern uint32_t	QMOImageGetSegmentCount(QMOImageRef qmoImage);
	
/*!
    @function       QMOImageGetSegmentByName
    
    @abstract       Returns information about the named segment.
    
    @discussion     Returns information about the named segment within the image. 
                    You can chose to get the segment index, or the segment fields, 
                    or both.
                    
    @param qmoImage Must be a valid image object.

    @param segName
                    The name of the segment to search for as ASCII.  A typical 
                    value might be "__TEXT".  Segment name comparison is case 
                    sensitive.  If there is more than one segment with this name, 
                    you get the first one.
                    
    @param segIndexPtr
                    On success, if segIndexPtr is not NULL, *segIndexPtr will be 
                    the zero-based index of the segment within the image.
                    
                    It is an error for both segIndexPtr and segPtr to be NULL.
                    
    @param segPtr   On success, if segPtr is not NULL, *segPtr will contain 
                    information about the segment.  The fields of this structure 
                    will be in native endian format.

                    The information is always returned in a (struct seg_command_64), 
                    even if the segment is in a 32-bit image (which would contain 
                    (struct seg_command)); all relevant 32-bit quantities are 
                    promoted to 64-bits in the result.
                    
                    The vmaddr field of this structure is the /unprepared/ value. 
                    If the image is prepared, there's no guarantee that the segment 
                    will be mapped at this address.  To get the address at which 
                    the segment is mapped, you must add the slide to this value.

                    It is an error for both segIndexPtr and segPtr to be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If the named segment is not found, the result is ESRCH.
*/
extern int		QMOImageGetSegmentByName(
    QMOImageRef                 qmoImage, 
    const char *                segName, 
    uint32_t *                  segIndexPtr, 
    struct segment_command_64 * segPtr
);

/*!
    @function       QMOImageGetSegmentByIndex
    
    @abstract       Returns information about a segment.
    
    @discussion     Returns information about the specified segment within the image. 
                    
    @param qmoImage Must be a valid image object.

    @param segIndex The zero-based index of the segment for which to return 
                    information. 

    @param segPtr   On entry, segPtr must not be NULL.  On success, *segPtr will 
                    contain information about the segment.  The fields of this 
                    structure will be in native endian format.

                    The information is always returned in a (struct seg_command_64), 
                    even if the segment is in a 32-bit image (which would contain 
                    (struct seg_command)); all relevant 32-bit quantities are 
                    promoted to 64-bits in the result.

                    The vmaddr field of this structure is the /unprepared/ value. 
                    If the image is prepared, there's no guarantee that the segment 
                    will be mapped at this address.  To get the address at which 
                    the segment is mapped, you must add the slide to this value.

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If segIndex is out of range, the result is EINVAL.
*/
extern int		QMOImageGetSegmentByIndex(
    QMOImageRef                 qmoImage, 
    uint32_t                    segIndex, 
    struct segment_command_64 * segPtr
);

/*!
    @function       QMOImageGetSectionCount
    
    @abstract       Returns the number of sections within the image.
    
    @discussion     Returns the number of sections within the image.  This is 
                    equal to the sum of the nsects fields of all the segment in 
                    the image.
                    
    @param qmoImage Must be a valid image object.

    @result         A count of the sections in the image.
*/
extern uint32_t	QMOImageGetSectionCount(QMOImageRef qmoImage);

/*!
    @function       QMOImageGetSectionByName
    
    @abstract       Returns information about the named section.
    
    @discussion     Returns information about the named section within the image. 
                    You can choose to search for the section by the section name, 
                    the segment name, or both.  If there is more than one section 
                    that matches the search criteria, you get the first.
                    
                    You can chose to request the section index, or the section 
                    fields, or both.
                    
    @param qmoImage Must be a valid image object.

    @param segName  The name of the segment in which the section must reside, in 
                    ASCII.  A typical value might be "__TEXT".  Segment name 
                    comparison is case sensitive.  If you specify NULL, the section 
                    specified by sectName can reside in any segment.
                    
                    It is an error for both segName and sectName to be NULL.

    @param sectName The name of the section, in ASCII.  A typical value might be 
                    "__text".  Section name comparison is case sensitive.  If you 
                    specify NULL, you get the first section within the segment 
                    specified by segName.

                    It is an error for both segName and sectName to be NULL.

    @param sectIndexPtr
                    On success, if sectIndexPtr is not NULL, *sectIndexPtr will be 
                    the zero-based index of the section within the image.
                    
                    It is an error for both sectIndexPtr and sectPtr to be NULL.
                    
    @param sectPtr  On success, if sectPtr is not NULL, *sectPtr will contain 
                    information about the section.  The fields of this structure 
                    will be in native endian format.

                    The information is always returned in a (struct section_64), 
                    even if the section is in a 32-bit image (which would contain 
                    (struct section)); all relevant 32-bit quantities are promoted 
                    to 64-bits in the result.
                    
                    The addr field of this structure is the /unprepared/ value. 
                    If the image is prepared, there's no guarantee that the section 
                    will be mapped at this address.  To get the address at which 
                    the section is mapped, you must add the slide to this value.

                    It is an error for both sectIndexPtr and sectPtr to be NULL.

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If the named section is not found, the result is ESRCH.
*/
extern int		QMOImageGetSectionByName(
    QMOImageRef                 qmoImage, 
    const char *                segName,
    const char *                sectName,
    uint32_t *                  sectIndexPtr,
    struct section_64 *         sectPtr
);

/*!
    @function       QMOImageGetSectionByIndex
    
    @abstract       Returns information about a section.
    
    @discussion     Returns information about the specified section within the image. 
                    
    @param qmoImage Must be a valid image object.

    @param sectIndex
                    The zero-based index of the section for which to return 
                    information. 

    @param sectPtr  On entry, sectPtr must not be NULL.  On success, *sectPtr will 
                    contain information about the section.  The fields of this 
                    structure will be in native endian format.

                    The information is always returned in a (struct section_64), 
                    even if the section is in a 32-bit image (which would contain 
                    (struct section)); all relevant 32-bit quantities are promoted 
                    to 64-bits in the result.

                    The addr field of this structure is the /unprepared/ value. 
                    If the image is prepared, there's no guarantee that the section 
                    will be mapped at this address.  To get the address at which 
                    the section is mapped, you must add the slide to this value.

    @result         An errno-style error code per QTMErrnoFromMachError.
                    If sectIndex is out of range, the result is EINVAL.
*/
extern int		QMOImageGetSectionByIndex(
    QMOImageRef                 qmoImage, 
    uint32_t                    sectIndex, 
    struct section_64 *         sectPtr
);

/*!
    @functiongroup  Utilities
*/
#pragma mark ***** Utilities

/*!
    @function       QMOGetLocalCPUType
    
    @abstract       Returns the CPU type of the current process.

    @discussion     Returns the CPU type of the current process (for example,
                    CPU_TYPE_X86, CPU_TYPE_POWERPC).
    
    @result         The CPU type of the current process.
*/
extern cpu_type_t QMOGetLocalCPUType(void);

/*!
    @function       QMOImageToLocalUInt8
    
    @abstract       Converts an 8-bit quantity from image format to native format.

    @discussion     Yes, I know that this routine is a NOP, but it makes the code 
                    more symmentric.
    
    @param qmoImage Must be a valid image object.

    @param value    The value to convert, in image format.

    @result         The value in native format.
*/
extern uint8_t	QMOImageToLocalUInt8( QMOImageRef qmoImage, uint8_t  value);

/*!
    @function       QMOImageToLocalUInt16
    
    @abstract       Converts a 16-bit quantity from image format to native format.
    
    @param qmoImage Must be a valid image object.

    @param value    The value to convert, in image format.

    @result         The value in native format.
*/
extern uint16_t QMOImageToLocalUInt16(QMOImageRef qmoImage, uint16_t value);

/*!
    @function       QMOImageToLocalUInt32
    
    @abstract       Converts a 32-bit quantity from image format to native format.
    
    @param qmoImage Must be a valid image object.

    @param value    The value to convert, in image format.

    @result         The value in native format.
*/
extern uint32_t QMOImageToLocalUInt32(QMOImageRef qmoImage, uint32_t value);

/*!
    @function       QMOImageToLocalUInt64
    
    @abstract       Converts a 64-bit quantity from image format to native format.
    
    @param qmoImage Must be a valid image object.

    @param value    The value to convert, in image format.

    @result         The value in native format.
*/
extern uint64_t QMOImageToLocalUInt64(QMOImageRef qmoImage, uint64_t value);

/*!
    @functiongroup  High-Level Routines
*/
#pragma mark ***** High-Level Routines

/*!
    @function       QMOImageGetLibraryID
    
    @abstract       Returns the library ID of an image.
    
    @discussion     Returns the library ID of an image, as contained in the 
                    LC_ID_DYLIB command within the image.  This is typically 
                    only present for shared libraries and frameworks (whose Mach-O 
                    file type is MH_DYLIB).
                    
    @param qmoImage Must be a valid image object.

    @result         A pointer to the library ID for the image.  This will be NULL 
                    if the image has no LC_ID_DYLIB command.
                    
                    Do not try to free this pointer.  It belongs to the image 
                    object and will be freed when you destroy the image object.
*/
extern const char * QMOImageGetLibraryID(QMOImageRef qmoImage);

/*!
    @function       QMOSymbolIteratorProc
    
    @abstract       Symbol callback for QMOImageIterateSymbols.
    
    @discussion     QMOImageIterateSymbols will call this callback for every symbol 
                    in the image.
                    
                    IMPORTANT: When I say every symbol, I mean /every/ symbol.  
                    This isn't called just for symbols exported by the image. 
                    It's also called for symbols that the image imports (these 
                    are of type N_SECT) and for all debugger symbols.  If you're 
                    just looking for exported symbols, you must filter out the 
                    cruft.
                    
                    The following pre-conditions are appropriate for your callback:
                    
                    assert(qmoImage != NULL);
                    assert(name != NULL);
                    assert( stopPtr != NULL);
                    assert(*stopPtr == false);
                    
    @param qmoImage This is the image object on which you called QMOImageIterateSymbols.

    @param name     The name of the symbol.  If you're looking for a C symbol, 
                    remember that the compiler prefixes all C symbols with a 
                    leading underscore.

    @param type     The symbol type; equivalent to the n_type field of (struct nlist). 
                    You can use the masks from <mach-o/nlist.h> to extract information 
                    from this value.  The most commonly used masks are:
                    
                    o N_STAB -- If this flag is set, the symbol is a debugging symbol.
                    o N_EXT  -- If this flag is set, the symbol is an external symbol.
                    o N_TYPE -- Use this mask to extract the symbol type (typical 
                      values are N_UNDF, N_ABS, and N_SECT).

    @param sect     The symbol section; equivalent to the n_sect field of (struct nlist).
                    For symbols of type N_SECT, this is the section in which the 
                    symbol resides.
                    
                    IMPORTANT: This is /one-based/, not zero-based.  Thus, if you 
                    want to pass this to QMOImageGetSectionByIndex, you have to 
                    subtract one.

    @param desc     Equivalent to the n_desc field of (struct nlist).  See 
                    <mach-o/nlist.h> and <mach-o/stabs.h> for details.

    @param value    The symbol value; equivalent to the n_value field of (struct nlist).

                    IMPORTANT: For symbols of type N_SECT, you must add the image's 
                    slide (as returned by QMOImageGetSlide) to the value to get the 
                    actual address of the symbol.

    @param iteratorRefCon
                    The iterator refCon passed to QMOImageIterateSymbols.

    @param stopPtr  Will not be NULL.  Set *stopPtr to true to stop iteration 
                    without triggering an error.

    @result         An errno-style error code.  Any value other than 0 will 
                    cause symbol iteration to stop and QMOImageIterateSymbols 
                    to return that error.
*/
typedef int (*QMOSymbolIteratorProc)(
	QMOImageRef		qmoImage, 
	const char *	name,
	uint8_t			type,
	uint8_t			sect,
	uint16_t		desc,
	QTMAddr			value,
	void *			iteratorRefCon,
	bool *			stopPtr
);

/*!
    @function       QMOImageIterateSymbols
    
    @abstract       Iterates through the symbols in an image.
    
    @discussion     Iterates through the symbols in an image, call your supplied 
                    callback for each symbol.

    @param qmoImage Must be a valid image object.

    @param callback A pointer to a QMOSymbolIteratorProc callback routine that's 
                    called for each symbol in the image.

    @param iteratorRefCon
                    A refCon for that callback.

    @result         An errno-style error code per QTMErrnoFromMachError.  If your 
                    callback returns an error, symbol iteration stops and the 
                    error code from your callback is returned here.
*/
extern int QMOImageIterateSymbols(
    QMOImageRef             qmoImage, 
    QMOSymbolIteratorProc   callback, 
    void *                  iteratorRefCon
);

/*!
    @function       QMOImageLookupSymbol
    
    @abstract       Looks up a symbol by name.
    
    @discussion     Looks up the named symbol and returns its value.  The current 
                    implementation will only find symbols of type N_ABS and N_SECT 
                    (that is, absolute symbols and symbols exported by this image).

    @param qmoImage Must be a valid image object.

    @param symName  The name of the symbol to look up.  If this is a C symbol, 
                    you must include the leading underscore that's inserted by 
                    the compiler for all C symbols.

    @param valuePtr On entry, valuePtr must not be NULL.  On success, *valuePtr 
                    will be the value of the symbol.  This /will/ account for 
                    any slide.

    @result         An errno-style error code per QTMErrnoFromMachError.  If the 
                    named symbol is not found, the result is ESRCH.
*/
extern int QMOImageLookupSymbol(
    QMOImageRef     qmoImage, 
    const char *    symName, 
    QTMAddr *       valuePtr
);

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
