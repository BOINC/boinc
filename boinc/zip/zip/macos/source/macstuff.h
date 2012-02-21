#ifndef _MACSTUFF_H
#define _MACSTUFF_H       1

/*
These Functions were originally part of More Files version 1.4.8

More Files fixes many of the broken or underfunctional
parts of the file system.

More Files

A collection of File Manager and related routines

by Jim Luther (Apple Macintosh Developer Technical Support Emeritus)
with significant code contributions by Nitin Ganatra
(Apple Macintosh Developer Technical Support Emeritus)
Copyright  1992-1998 Apple Computer, Inc.
Portions copyright  1995 Jim Luther
All rights reserved.

The Package "More Files" is distributed under the following
license terms:

         "You may incorporate this sample code into your
          applications without restriction, though the
          sample code has been provided "AS IS" and the
          responsibility for its operation is 100% yours.
          However, what you are not permitted to do is to
          redistribute the source as "DSC Sample Code" after
          having made changes. If you're going to
          redistribute the source, we require that you make
          it clear in the source that the code was descended
          from Apple Sample Code, but that you've made
          changes."


The following changes are made by Info-ZIP:

- The only changes are made by pasting the functions
  (mostly found in MoreFilesExtras.c / MoreFiles.c)
  directly into macstuff.c / macstuff.h and slightly
  reformatting the text (replacement of TABs by spaces,
  removal/replacement of non-ASCII characters).
  The code itself is NOT changed.

This file has been modified by Info-ZIP for use in MacZip.
This file is NOT part of the original package More Files.

More Files can be found on the MetroWerks CD and Developer CD from
Apple. You can also download the latest version from:

    http://members.aol.com/JumpLong/#MoreFiles

Jim Luther's Home-page:
    http://members.aol.com/JumpLong/


*/


#define __MACOSSEVENFIVEONEORLATER 1
#define __MACOSSEVENFIVEORLATER    1
#define __MACOSSEVENORLATER        1

#include <Errors.h>
#include <Files.h>


/*
 * Like the MoreFiles routines these fix problems in the standard
 * Mac calls.
 */

int     FSpLocationFromPath (int length,const char *path, FSSpecPtr theSpec);
OSErr   FSpPathFromLocation (FSSpecPtr theSpec,int *length, Handle *fullPath);

#define hasDesktopMgr(volParms) (((volParms).vMAttrib & (1L << bHasDesktopMgr)) != 0)

/*
 * The following routines are utility functions.  They are exported
 * here because they are needed and they are not officially supported,
 * however.  The first set are from the MoreFiles package.
 */
int             FSpGetDefaultDir (FSSpecPtr theSpec);
int             FSpSetDefaultDir (FSSpecPtr dirSpec);
pascal  OSErr   FSpGetDirectoryID(const FSSpec *spec,long *theDirID,
                                  Boolean *isDirectory);
pascal  short   FSpOpenResFileCompat(const FSSpec *spec, SignedByte permission);
pascal  void    FSpCreateResFileCompat(const FSSpec *spec,OSType creator,
                                       OSType fileType,
                                       ScriptCode scriptTag);
OSErr           FSpFindFolder (short vRefNum, OSType folderType,
                               Boolean createFolder,
                               FSSpec *spec);

/*****************************************************************************/

pascal  OSErr   GetVolumeInfoNoName(ConstStr255Param pathname,
                                    short vRefNum,
                                    HParmBlkPtr pb);
/*   Call PBHGetVInfoSync ignoring returned name.
    GetVolumeInfoNoName uses pathname and vRefNum to call PBHGetVInfoSync
    in cases where the returned volume name is not needed by the caller.
    The pathname and vRefNum parameters are not touched, and the pb
    parameter is initialized by PBHGetVInfoSync except that ioNamePtr in
    the parameter block is always returned as NULL (since it might point
    to GetVolumeInfoNoName's local variable tempPathname).

    I noticed using this code in several places, so here it is once.
    This reduces the code size of MoreFiles.

    pathName    input:  Pointer to a full pathname or nil.  If you pass in a
                        partial pathname, it is ignored. A full pathname to a
                        volume must end with a colon character (:).
    vRefNum     input:  Volume specification (volume reference number, working
                        directory number, drive number, or 0).
    pb          input:  A pointer to HParamBlockRec.
                output: The parameter block as filled in by PBHGetVInfoSync
                        except that ioNamePtr will always be NULL.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        paramErr            -50     No default volume, or pb was NULL
*/


/*****************************************************************************/

pascal  OSErr   GetFilenameFromPathname(ConstStr255Param pathname,
                                        Str255 filename);
/*   Get the object name from the end of a full or partial pathname.
    The GetFilenameFromPathname function gets the file (or directory) name
    from the end of a full or partial pathname. Returns notAFileErr if the
    pathname is nil, the pathname is empty, or the pathname cannot refer to
    a filename (with a noErr result, the pathname could still refer to a
    directory).

    pathname    input:  A full or partial pathname.
    filename    output: The file (or directory) name.

    Result Codes
        noErr               0       No error
        notAFileErr         -1302   The pathname is nil, the pathname
                                    is empty, or the pathname cannot refer
                                    to a filename

    __________

    See also:   GetObjectLocation.
*/



/*****************************************************************************/

pascal  OSErr   FSMakeFSSpecCompat(short vRefNum, long dirID,
                                   ConstStr255Param fileName,
                                   FSSpec *spec);
/*   Initialize a FSSpec record.
    The FSMakeFSSpecCompat function fills in the fields of an FSSpec record.
    If the file system can't create the FSSpec, then the compatibility code
    creates a FSSpec that is exactly like an FSSpec except that spec.name
    for a file may not have the same capitalization as the file's catalog
    entry on the disk volume. That is because fileName is parsed to get the
    name instead of getting the name back from the file system. This works
    fine with System 6 where FSMakeSpec isn't available.

    vRefNum     input:  Volume specification.
    dirID       input:  Directory ID.
    fileName    input:  Pointer to object name, or nil when dirID specifies
                        a directory that's the object.
    spec        output: A file system specification to be filled in by
                        FSMakeFSSpecCompat.

    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume doesnt exist
        fnfErr              -43     File or directory does not exist
                                    (FSSpec is still valid)
*/


#if !SystemSevenOrLater
static  Boolean FSHasFSSpecCalls(void);

static  Boolean QTHasFSSpecCalls(void);
#endif  /* !SystemSevenOrLater */



/*****************************************************************************/

pascal  OSErr   GetObjectLocation(short vRefNum,
                                  long dirID,
                                  ConstStr255Param pathname,
                                  short *realVRefNum,
                                  long *realParID,
                                  Str255 realName,
                                  Boolean *isDirectory);
/*   Get a file system object's location.
    The GetObjectLocation function gets a file system object's location -
    that is, its real volume reference number, real parent directory ID,
    and name. While we're at it, determine if the object is a file or directory.
    If GetObjectLocation returns fnfErr, then the location information
    returned is valid, but it describes an object that doesn't exist.
    You can use the location information for another operation, such as
    creating a file or directory.

    vRefNum     input:  Volume specification.
    dirID       input:  Directory ID.
    pathname    input:  Pointer to object name, or nil when dirID specifies
                        a directory that's the object.
    realVRefNum output: The real volume reference number.
    realParID   output: The parent directory ID of the specified object.
    realName    output: The name of the specified object (the case of the
                        object name may not be the same as the object's
                        catalog entry on disk - since the Macintosh file
                        system is not case sensitive, it shouldn't matter).
    isDirectory output: True if object is a directory; false if object
                        is a file.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        notAFileErr         -1302   The pathname is nil, the pathname
                                    is empty, or the pathname cannot refer
                                    to a filename
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    See also:   FSMakeFSSpecCompat
*/

pascal  OSErr   FSpGetDirectoryID(const FSSpec *spec, long *theDirID,
                                  Boolean *isDirectory);

pascal  OSErr   GetDirectoryID(short vRefNum,long dirID,ConstStr255Param name,
                               long *theDirID,Boolean *isDirectory);



/*****************************************************************************/

pascal  OSErr GetCatInfoNoName(short vRefNum,
                               long dirID,
                               ConstStr255Param name,
                               CInfoPBPtr pb);
/*   Call PBGetCatInfoSync ignoring returned name.
    GetCatInfoNoName uses vRefNum, dirID and name to call PBGetCatInfoSync
    in cases where the returned object is not needed by the caller.
    The vRefNum, dirID and name parameters are not touched, and the pb
    parameter is initialized by PBGetCatInfoSync except that ioNamePtr in
    the parameter block is always returned as NULL (since it might point
    to GetCatInfoNoName's local variable tempName).

    I noticed using this code in several places, so here it is once.
    This reduces the code size of MoreFiles.

    vRefNum         input:  Volume specification.
    dirID           input:  Directory ID.
    name            input:  Pointer to object name, or nil when dirID
                            specifies a directory that's the object.
    pb              input:  A pointer to CInfoPBRec.
                    output: The parameter block as filled in by
                            PBGetCatInfoSync except that ioNamePtr will
                            always be NULL.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

*/


/*****************************************************************************/


pascal  OSErr   DetermineVRefNum(ConstStr255Param pathname,
                                 short vRefNum,
                                 short *realVRefNum);
/*   Determine the real volume reference number.
    The DetermineVRefNum function determines the volume reference number of
    a volume from a pathname, a volume specification, or a combination
    of the two.
    WARNING: Volume names on the Macintosh are *not* unique -- Multiple
    mounted volumes can have the same name. For this reason, the use of a
    volume name or full pathname to identify a specific volume may not
    produce the results you expect.  If more than one volume has the same
    name and a volume name or full pathname is used, the File Manager
    currently uses the first volume it finds with a matching name in the
    volume queue.

    pathName    input:  Pointer to a full pathname or nil.  If you pass in a
                        partial pathname, it is ignored. A full pathname to a
                        volume must end with a colon character (:).
    vRefNum     input:  Volume specification (volume reference number, working
                        directory number, drive number, or 0).
    realVRefNum output: The real volume reference number.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        paramErr            -50     No default volume
*/


/*****************************************************************************/

pascal  OSErr   FSpGetFullPath(const FSSpec *spec,
                               short *fullPathLength,
                               Handle *fullPath);
/*   Get a full pathname to a volume, directory or file.
    The GetFullPath function builds a full pathname to the specified
    object. The full pathname is returned in the newly created handle
    fullPath and the length of the full pathname is returned in
    fullPathLength. Your program is responsible for disposing of the
    fullPath handle.

    spec            input:  An FSSpec record specifying the object.
    fullPathLength  output: The number of characters in the full pathname.
                            If the function fails to create a full pathname,
                            it sets fullPathLength to 0.
    fullPath        output: A handle to the newly created full pathname
                            buffer. If the function fails to create a
                            full pathname, it sets fullPath to NULL.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File or directory does not exist
        paramErr            -50     No default volume
        memFullErr          -108    Not enough memory
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    See also:   GetFullPath
*/

/*****************************************************************************/

pascal OSErr FSpLocationFromFullPath(short fullPathLength,
                                     const void *fullPath,
                                     FSSpec *spec);
/*   Get a FSSpec from a full pathname.
    The FSpLocationFromFullPath function returns a FSSpec to the object
    specified by full pathname. This function requires the Alias Manager.

    fullPathLength  input:  The number of characters in the full pathname
                            of the target.
    fullPath        input:  A pointer to a buffer that contains the full
                            pathname of the target. The full pathname
                            starts with the name of the volume, includes
                            all of the directory names in the path to the
                            target, and ends with the target name.
    spec            output: An FSSpec record specifying the object.

    Result Codes
        noErr               0       No error
        nsvErr              -35     The volume is not mounted
        fnfErr              -43     Target not found, but volume and parent
                                    directory found
        paramErr            -50     Parameter error
        usrCanceledErr      -128    The user canceled the operation

    __________

    See also:   LocationFromFullPath
*/


/*****************************************************************************/

pascal  OSErr   GetFullPath(short vRefNum,
                            long dirID,
                            ConstStr255Param name,
                            short *fullPathLength,
                            Handle *fullPath);
/*   Get a full pathname to a volume, directory or file.
    The GetFullPath function builds a full pathname to the specified
    object. The full pathname is returned in the newly created handle
    fullPath and the length of the full pathname is returned in
    fullPathLength. Your program is responsible for disposing of the
    fullPath handle.

    Note that a full pathname can be made to a file/directory that does not
    yet exist if all directories up to that file/directory exist. In this case,
    GetFullPath will return a fnfErr.

    vRefNum         input:  Volume specification.
    dirID           input:  Directory ID.
    name            input:  Pointer to object name, or nil when dirID
                            specifies a directory that's the object.
    fullPathLength  output: The number of characters in the full pathname.
                            If the function fails to create a full
                            pathname, it sets fullPathLength to 0.
    fullPath        output: A handle to the newly created full pathname
                            buffer. If the function fails to create a
                            full pathname, it sets fullPath to NULL.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File or directory does not exist (fullPath
                                    and fullPathLength are still valid)
        paramErr            -50     No default volume
        memFullErr          -108    Not enough memory
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    See also:   FSpGetFullPath
*/



/*****************************************************************************/

pascal  OSErr   ChangeCreatorType(short vRefNum,
                                  long dirID,
                                  ConstStr255Param name,
                                  OSType creator,
                                  OSType fileType);
/*   Change the creator or file type of a file.
    The ChangeCreatorType function changes the creator or file type of a file.

    vRefNum     input:  Volume specification.
    dirID       input:  Directory ID.
    name        input:  The name of the file.
    creator     input:  The new creator type or 0x00000000 to leave
                        the creator type alone.
    fileType    input:  The new file type or 0x00000000 to leave the
                        file type alone.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        fLckdErr            -45     File is locked
        vLckdErr            -46     Volume is locked or read-only
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        notAFileErr         -1302   Name was not a file
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    See also:   FSpChangeCreatorType
*/

/*****************************************************************************/

pascal  OSErr   FSpChangeCreatorType(const FSSpec *spec,
                                     OSType creator,
                                     OSType fileType);
/*   Change the creator or file type of a file.
    The FSpChangeCreatorType function changes the creator or file type of a file.

    spec        input:  An FSSpec record specifying the file.
    creator     input:  The new creator type or 0x00000000 to leave
                        the creator type alone.
    fileType    input:  The new file type or 0x00000000 to leave the
                        file type alone.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        fLckdErr            -45     File is locked
        vLckdErr            -46     Volume is locked or read-only
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        notAFileErr         -1302   Name was not a file
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    See also:   ChangeCreatorType
*/


/*****************************************************************************/

pascal  OSErr   BumpDate(short vRefNum,
                         long dirID,
                         ConstStr255Param name);
/*   Update the modification date of a file or directory.
    The BumpDate function changes the modification date of a file or
    directory to the current date/time.  If the modification date is already
    equal to the current date/time, then add one second to the
    modification date.

    vRefNum input:  Volume specification.
    dirID   input:  Directory ID.
    name    input:  Pointer to object name, or nil when dirID specifies
                    a directory that's the object.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        fLckdErr            -45     File is locked
        vLckdErr            -46     Volume is locked or read-only
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    See also:   FSpBumpDate
*/

/*****************************************************************************/

pascal  OSErr   FSpBumpDate(const FSSpec *spec);
/*   Update the modification date of a file or directory.
    The FSpBumpDate function changes the modification date of a file or
    directory to the current date/time.  If the modification date is already
    equal to the current date/time, then add one second to the
    modification date.

    spec    input:  An FSSpec record specifying the object.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        fLckdErr            -45     File is locked
        vLckdErr            -46     Volume is locked or read-only
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    See also:   BumpDate
*/

/*****************************************************************************/

pascal  OSErr   OnLine(FSSpecPtr volumes,
                       short reqVolCount,
                       short *actVolCount,
                       short *volIndex);
/*   Return the list of volumes currently mounted.
    The OnLine function returns the list of volumes currently mounted in
    an array of FSSpec records.

    A noErr result indicates that the volumes array was filled
    (actVolCount == reqVolCount) and there may be additional volumes
    mounted. A nsvErr result indicates that the end of the volume list
    was found and actVolCount volumes were actually found this time.

    volumes     input:  Pointer to array of FSSpec where the volume list
                        is returned.
    reqVolCount input:  Maximum number of volumes to return (the number of
                        elements in the volumes array).
    actVolCount output: The number of volumes actually returned.
    volIndex    input:  The current volume index position. Set to 1 to
                        start with the first volume.
                output: The volume index position to get the next volume.
                        Pass this value the next time you call OnLine to
                        start where you left off.

    Result Codes
        noErr               0       No error, but there are more volumes
                                    to list
        nsvErr              -35     No more volumes to be listed
        paramErr            -50     volIndex was <= 0
*/

/*****************************************************************************/

pascal  OSErr   DTGetComment(short vRefNum,
                             long dirID,
                             ConstStr255Param name,
                             Str255 comment);
/*   Get a file or directory's Finder comment field (if any).
    The DTGetComment function gets a file or directory's Finder comment
    field (if any) from the Desktop Manager or if the Desktop Manager is
    not available, from the Finder's Desktop file.

    IMPORTANT NOTE: Inside Macintosh says that comments are up to
    200 characters. While that may be correct for the HFS file system's
    Desktop Manager, other file systems (such as Apple Photo Access) return
    up to 255 characters. Make sure the comment buffer is a Str255 or you'll
    regret it.

    vRefNum input:  Volume specification.
    dirID   input:  Directory ID.
    name    input:  Pointer to object name, or nil when dirID
                    specifies a directory that's the object.
    comment output: A Str255 where the comment is to be returned.

    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        fnfErr              -43     File not found
        paramErr            -50     Volume doesn't support this function
        rfNumErr            -51     Reference number invalid
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        desktopDamagedErr   -1305   The desktop database has become corrupted -
                                    the Finder will fix this, but if your
                                    application is not running with the
                                    Finder, use PBDTReset or PBDTDelete
        afpItemNotFound     -5012   Information not found

    __________

    Also see:   DTCopyComment, FSpDTCopyComment, DTSetComment, FSpDTSetComment,
                FSpDTGetComment
*/

/*****************************************************************************/

pascal  OSErr   FSpDTGetComment(const FSSpec *spec,
                                Str255 comment);
/*   Get a file or directory's Finder comment field (if any).
    The FSpDTGetComment function gets a file or directory's Finder comment
    field (if any) from the Desktop Manager or if the Desktop Manager is
    not available, from the Finder's Desktop file.

    IMPORTANT NOTE: Inside Macintosh says that comments are up to
    200 characters. While that may be correct for the HFS file system's
    Desktop Manager, other file systems (such as Apple Photo Access) return
    up to 255 characters. Make sure the comment buffer is a Str255 or you'll
    regret it.

    spec    input:  An FSSpec record specifying the file or directory.
    comment output: A Str255 where the comment is to be returned.

    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        fnfErr              -43     File not found
        paramErr            -50     Volume doesn't support this function
        rfNumErr            -51     Reference number invalid
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        desktopDamagedErr   -1305   The desktop database has become corrupted -
                                    the Finder will fix this, but if your
                                    application is not running with the
                                    Finder, use PBDTReset or PBDTDelete
        afpItemNotFound     -5012   Information not found

    __________

    Also see:   DTCopyComment, FSpDTCopyComment, DTSetComment, FSpDTSetComment,
                DTGetComment
*/

/*****************************************************************************/

pascal  OSErr   DTOpen(ConstStr255Param volName,
                       short vRefNum,
                       short *dtRefNum,
                       Boolean *newDTDatabase);
/*   Open a volume's desktop database and return the desktop database refNum.
    The DTOpen function opens a volume's desktop database. It returns
    the reference number of the desktop database and indicates if the
    desktop database was created as a result of this call (if it was created,
    then it is empty).

    volName         input:  A pointer to the name of a mounted volume
                            or nil.
    vRefNum         input:  Volume specification.
    dtRefNum        output: The reference number of Desktop Manager's
                            desktop database on the specified volume.
    newDTDatabase   output: true if the desktop database was created as a
                            result of this call and thus empty.
                            false if the desktop database was already created,
                            or if it could not be determined if it was already
                            created.

    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        paramErr            -50     Volume doesn't support this function
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        desktopDamagedErr   -1305   The desktop database has become corrupted -
                                    the Finder will fix this, but if your
                                    application is not running with the
                                    Finder, use PBDTReset or PBDTDelete
*/

/*****************************************************************************/

pascal  OSErr   HGetVolParms(ConstStr255Param volName,
                             short vRefNum,
                             GetVolParmsInfoBuffer *volParmsInfo,
                             long *infoSize);
/*   Determine the characteristics of a volume.
    The HGetVolParms function returns information about the characteristics
    of a volume. A result of paramErr usually just means the volume doesn't
    support PBHGetVolParms and the feature you were going to check
    for isn't available.

    volName         input:  A pointer to the name of a mounted volume
                            or nil.
    vRefNum         input:  Volume specification.
    volParmsInfo    input:  Pointer to GetVolParmsInfoBuffer where the
                            volume attributes information is returned.
                    output: Atributes information.
    infoSize        input:  Size of buffer pointed to by volParmsInfo.
                    output: Size of data actually returned.

    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        paramErr            -50     Volume doesn't support this function

    __________

    Also see the macros for checking attribute bits in MoreFilesExtras.h
*/

/*****************************************************************************/

pascal  OSErr   DeleteDirectoryContents(short vRefNum,
                                        long dirID,
                                        ConstStr255Param name);
/*   Delete the contents of a directory.
    The DeleteDirectoryContents function deletes the contents of a directory.
    All files and subdirectories in the specified directory are deleted.
    If a locked file or directory is encountered, it is unlocked and then
    deleted.  If any unexpected errors are encountered,
    DeleteDirectoryContents quits and returns to the caller.

    vRefNum input:  Volume specification.
    dirID   input:  Directory ID.
    name    input:  Pointer to directory name, or nil when dirID specifies
                    a directory that's the object.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        wPrErr              -44     Hardware volume lock
        fLckdErr            -45     File is locked
        vLckdErr            -46     Software volume lock
        fBsyErr             -47     File busy, directory not empty, or working
                                    directory control block open
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    Also see:   DeleteDirectory
*/

/*****************************************************************************/

pascal  OSErr   DeleteDirectory(short vRefNum,
                                long dirID,
                                ConstStr255Param name);
/*   Delete a directory and its contents.
    The DeleteDirectory function deletes a directory and its contents.
    All files and subdirectories in the specified directory are deleted.
    If a locked file or directory is encountered, it is unlocked and then
    deleted.  After deleting the directories contents, the directory is
    deleted. If any unexpected errors are encountered, DeleteDirectory
    quits and returns to the caller.

    vRefNum input:  Volume specification.
    dirID   input:  Directory ID.
    name    input:  Pointer to directory name, or nil when dirID specifies
                    a directory that's the object.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        wPrErr              -44     Hardware volume lock
        fLckdErr            -45     File is locked
        vLckdErr            -46     Software volume lock
        fBsyErr             -47     File busy, directory not empty, or working
                                    directory control block open
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    Also see:   DeleteDirectoryContents
*/

/*****************************************************************************/

pascal  OSErr   DTSetComment(short vRefNum,
                             long dirID,
                             ConstStr255Param name,
                             ConstStr255Param comment);
/*   Set a file or directory's Finder comment field.
    The DTSetComment function sets a file or directory's Finder comment
    field. The volume must support the Desktop Manager because you only
    have read access to the Desktop file.

    vRefNum input:  Volume specification.
    dirID   input:  Directory ID.
    name    input:  Pointer to object name, or nil when dirID
                    specifies a directory that's the object.
    comment input:  The comment to add. Comments are limited to 200 characters;
                    longer comments are truncated.

    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        fnfErr              -43     File or directory doesnt exist
        paramErr            -50     Volume doesn't support this function
        wPrErr              -44     Volume is locked through hardware
        vLckdErr            -46     Volume is locked through software
        rfNumErr            -51     Reference number invalid
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        desktopDamagedErr   -1305   The desktop database has become corrupted -
                                    the Finder will fix this, but if your
                                    application is not running with the
                                    Finder, use PBDTReset or PBDTDelete

    __________

    Also see:   DTCopyComment, FSpDTCopyComment, FSpDTSetComment, DTGetComment,
                FSpDTGetComment
*/

/*****************************************************************************/

pascal  OSErr   FSpDTSetComment(const FSSpec *spec,
                                ConstStr255Param comment);
/*   Set a file or directory's Finder comment field.
    The FSpDTSetComment function sets a file or directory's Finder comment
    field. The volume must support the Desktop Manager because you only
    have read access to the Desktop file.

    spec    input:  An FSSpec record specifying the file or directory.
    comment input:  The comment to add. Comments are limited to 200 characters;
                    longer comments are truncated.

    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        fnfErr              -43     File or directory doesnt exist
        wPrErr              -44     Volume is locked through hardware
        vLckdErr            -46     Volume is locked through software
        rfNumErr            -51     Reference number invalid
        paramErr            -50     Volume doesn't support this function
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        desktopDamagedErr   -1305   The desktop database has become corrupted -
                                    the Finder will fix this, but if your
                                    application is not running with the
                                    Finder, use PBDTReset or PBDTDelete

    __________

    Also see:   DTCopyComment, FSpDTCopyComment, DTSetComment, DTGetComment,
                FSpDTGetComment
*/

/*****************************************************************************/

pascal  OSErr   XGetVInfo(short volReference,
                          StringPtr volName,
                          short *vRefNum,
                          UnsignedWide *freeBytes,
                          UnsignedWide *totalBytes);
/*   Get extended information about a mounted volume.
    The XGetVInfo function returns the name, volume reference number,
    available space (in bytes), and total space (in bytes) for the
    specified volume. You can specify the volume by providing its drive
    number, volume reference number, or 0 for the default volume.
    This routine is compatible with volumes up to 2 terabytes.

    volReference    input:  The drive number, volume reference number,
                            or 0 for the default volume.
    volName         input:  A pointer to a buffer (minimum Str27) where
                            the volume name is to be returned or must
                            be nil.
                    output: The volume name.
    vRefNum         output: The volume reference number.
    freeBytes       output: The number of free bytes on the volume.
                            freeBytes is an UnsignedWide value.
    totalBytes      output: The total number of bytes on the volume.
                            totalBytes is an UnsignedWide value.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        paramErr            -50     No default volume

    __________

    Also see:   HGetVInfo
*/

/*****************************************************************************/

pascal  OSErr   HGetVInfo(short volReference,
                          StringPtr volName,
                          short *vRefNum,
                          unsigned long *freeBytes,
                          unsigned long *totalBytes);
/*   Get information about a mounted volume.
    The HGetVInfo function returns the name, volume reference number,
    available space (in bytes), and total space (in bytes) for the
    specified volume. You can specify the volume by providing its drive
    number, volume reference number, or 0 for the default volume.
    This routine is compatible with volumes up to 4 gigabytes.

    volReference    input:  The drive number, volume reference number,
                            or 0 for the default volume.
    volName         input:  A pointer to a buffer (minimum Str27) where
                            the volume name is to be returned or must
                            be nil.
                    output: The volume name.
    vRefNum         output: The volume reference number.
    freeBytes       output: The number of free bytes on the volume.
                            freeBytes is an unsigned long value.
    totalBytes      output: The total number of bytes on the volume.
                            totalBytes is an unsigned long value.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        paramErr            -50     No default volume

    __________

    Also see:   XGetVInfo
*/

/*****************************************************************************/

pascal  OSErr   GetDirName(short vRefNum,
                           long dirID,
                           Str31 name);
/*   Get the name of a directory from its directory ID.
    The GetDirName function gets the name of a directory from its
    directory ID.

    vRefNum     input:  Volume specification.
    dirID       input:  Directory ID.
    name        output: Points to a Str31 where the directory name is to be
                        returned.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        paramErr            -50     No default volume or
                                    name parameter was NULL
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname
*/


/*****************************************************************************/

pascal  OSErr   GetVolFileSystemID(ConstStr255Param pathname,
                                   short vRefNum,
                                   short *fileSystemID);
/*   Get a volume's file system ID.
    The GetVolFileSystemID function returned the file system ID of
    a mounted volume. The file system ID identifies the file system
    that handles requests to a particular volume. Here's a partial list
    of file system ID numbers (only Apple's file systems are listed):
        FSID    File System
        -----   -----------------------------------------------------
        $0000   Macintosh HFS or MFS
        $0100   ProDOS File System
        $0101   PowerTalk Mail Enclosures
        $4147   ISO 9660 File Access (through Foreign File Access)
        $4242   High Sierra File Access (through Foreign File Access)
        $464D   QuickTake File System (through Foreign File Access)
        $4953   Macintosh PC Exchange (MS-DOS)
        $4A48   Audio CD Access (through Foreign File Access)
        $4D4B   Apple Photo Access (through Foreign File Access)

    See the Technical Note "FL 35 - Determining Which File System
    Is Active" and the "Guide to the File System Manager" for more
    information.

    pathName        input:  Pointer to a full pathname or nil.  If you pass
                            in a partial pathname, it is ignored. A full
                            pathname to a volume must contain at least
                            one colon character (:) and must not start with
                            a colon character.
    vRefNum         input:  Volume specification (volume reference number,
                            working directory number, drive number, or 0).
    fileSystemID    output: The volume's file system ID.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        paramErr            -50     No default volume, or pb was NULL
*/

/*****************************************************************************/

pascal  OSErr GetDInfo(short vRefNum,
                       long dirID,
                       ConstStr255Param name,
                       DInfo *fndrInfo);
/*   Get the finder information for a directory.
    The GetDInfo function gets the finder information for a directory.

    vRefNum         input:  Volume specification.
    dirID           input:  Directory ID.
    name            input:  Pointer to object name, or nil when dirID
                            specifies a directory that's the object.
    fndrInfo        output: If the object is a directory, then its DInfo.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    Also see:   FSpGetDInfo, FSpGetFInfoCompat
*/

/*****************************************************************************/

pascal  OSErr FSpGetDInfo(const FSSpec *spec,
                          DInfo *fndrInfo);
/*   Get the finder information for a directory.
    The FSpGetDInfo function gets the finder information for a directory.

    spec        input:  An FSSpec record specifying the directory.
    fndrInfo    output: If the object is a directory, then its DInfo.

    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname

    __________

    Also see:   FSpGetFInfoCompat, GetDInfo
*/


#endif    /*  _MACSTUFF_H  */
