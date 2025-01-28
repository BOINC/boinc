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

// THIS CODE IS OBSOLETE! A better way to add an icon to science applications
// (or any Mac executable, including command-line tools) is:
// [1] Select the application in the Finder
// [2] Get Info (command-I) for the selected application
// [3] Click on the file's current icon in the top left corner of the Get Info window
// [4] Paste (command-v) the desired icon.
//
// To copy an icon from a different file:
// [a] Get Info for the source file
// [b] Click on the file's  icon in the top left corner of the Get Info window
// [c] Copy (command-c) the icon.


/* Mac-specific code to display custom icon for science application (optional)
   adapted from code written by Bernd Machenschalk.  Used with permission of the
   Einstein@home project.

    To use this code:
    1. Create a *.icns file using "/Developer/Applications/utilities/Icon Composer.app"
    2. Convert the *.icns file to an app_icon.h file as follows: in Terminal, run
      "MakeAppIcon_h <source_file> <dest_file>".  (The MakeAppIcon_h command-line utility
      is built by the Mac boinc XCode project.)
    3. In the science application's main(), #include "app_icon.h" and call:
        setMacIcon(argv[0], MacAppIconData, sizeof(MacAppIconData));
    4. Science application must link with Carbon.framework.
*/

#include <Carbon/Carbon.h>
#include <sys/stat.h>

#include "boinc_api.h"
#include "filesys.h"
#include "common_defs.h"

#define RESIDICON -16455

char MacPListData[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "
"\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
"<plist version=\"1.0\">\n<dict>\n"
"\t<key>NSUIElement</key>\n\t<string>1</string>\n</dict>\n</plist>\n";


/* checks for an OS error, reports the line number and returns */
#define CHECK_OSERR(e) {\
  if (e) {\
    fprintf(stderr,"MacOS Error %d occurred in %s line %d\n",e,__FILE__,__LINE__);\
    return(e); } }

// Adds the specified resource to the file given as an argument.
int setMacRsrcForFile(char *filename, char *rsrcData, long rsrcSize,
                            OSType rsrcType, int rsrcID, ConstStringPtr rsrcName) {
    OSErr oserr;                    /* stores an OS error code */
//    FSSpec fsspec;                  /* FileSpec */
    HFSUniStr255 forkName;          /* Unicode name of resource fork "RESOURCE_FORK" */
    FSRef fsref;                    /* File Reference */
    FSCatalogInfo catalogInfo;      /* For setting custom icon bit in Finder Info */
    short rref;                     /* Resource Reference */
    Handle hand;
    int retry;

    /* get finder spec for this file */
    CHECK_OSERR((int)FSPathMakeRef((StringPtr)filename, &fsref, NULL));
//    CHECK_OSERR(FSGetCatalogInfo(&fsref, nil, NULL, NULL, &fsspec, NULL));

    /* Open the resource fork for writing, create it if it does not exist.
        On a dual-processor system, the other cpu may have the resource fork
        open for writing, so if we fail we wait and retry.
    */
    for (retry=0;retry<5;retry++) {
//        rref = FSpOpenResFile(&fsspec, fsRdWrPerm);
        rref = FSOpenResFile(&fsref, fsRdWrPerm);
        oserr = ResError();
        if (oserr == eofErr) { /* EOF, resource fork/file not found */
            // If we set file type and signature to non-NULL, it makes OS mistakenly
            // identify file as a classic application instead of a UNIX executable.
//            FSpCreateResFile(&fsref, 0, 0, smRoman);
            oserr = FSGetResourceForkName(&forkName);
            if (oserr == noErr) {
                oserr = FSCreateResourceFork(&fsref, forkName.length, forkName.unicode, 0);
            }
            oserr = ResError();
            if (oserr == noErr) {
//                rref = FSpOpenResFile(&fsspec, fsRdWrPerm);
                rref = FSOpenResFile(&fsref, fsRdWrPerm);
                oserr = ResError();
            }
        }
        // We may not have permissions to set resources in debug runs
        if ((oserr == noErr) || (oserr == wrPermErr) || (oserr == permErr))
            break;
        sleep (1);
    };

    if (oserr)
        return oserr; // give up after 5 seconds

    /* add the resource if not already present */
    if (!GetResource(rsrcType, rsrcID)) { /* if resource not found */
        oserr = PtrToHand(rsrcData, &hand, rsrcSize);
        if (!oserr)
            AddResource(hand, rsrcType, rsrcID, rsrcName);
    }

    /* add this to the file on disk */
    CloseResFile(rref);
    CHECK_OSERR(ResError());

    if (rsrcType == 'icns') {
        /* set custom icon flag */
        CHECK_OSERR(FSGetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL));
        ((FileInfo *)&catalogInfo.finderInfo)->finderFlags |= kHasCustomIcon;
        CHECK_OSERR( FSSetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo));
    }
    return(0);
}


static char * PersistentFGets(char *buf, size_t buflen, FILE *f) {
    char *p = buf;
    size_t len = buflen;
    size_t datalen = 0;

    *buf = '\0';
    while (datalen < (buflen - 1)) {
        fgets(p, len, f);
        if (feof(f)) break;
        if (ferror(f) && (errno != EINTR)) break;
        if (strchr(buf, '\n')) break;
        datalen = strlen(buf);
        p = buf + datalen;
        len -= datalen;
    }
    return (buf[0] ? buf : NULL);
}


void getPathToThisApp(char* pathBuf, size_t bufSize) {
    FILE *f;
    char buf[MAXPATHLEN], *c;
    pid_t myPID = getpid();
    int i;
    struct stat stat_buf;

    strlcpy(pathBuf, GRAPHICS_APP_FILENAME, bufSize);
    if (!stat(pathBuf, &stat_buf)) {
       // stat() returns zero on success
       return;
    }

    *pathBuf = 0;    // in case of failure

    // Before launching this project application, the BOINC client set the
    // current directory to the slot directory which contains this application
    // (or the soft-link to it.)  So all we need for the path to this
    // application is the file name.  We use the -c option so ps strips off
    // any command-line arguments for us.
    snprintf(buf, sizeof(buf), "ps -wcp %d -o command=", myPID);
    f = popen(buf,  "r");
    if (!f)
        return;
    PersistentFGets(pathBuf, bufSize, f);  // Skip over line of column headings
    PersistentFGets(pathBuf, bufSize, f);  // Get the UNIX command which ran us
    pclose(f);

    c = strstr(pathBuf, " -");
    if (c)
        *c = 0;     // Strip off any command-line arguments

    for (i=strlen(pathBuf)-1; i>=0; --i) {
        if (pathBuf[i] <= ' ')
            pathBuf[i] = 0;  // Strip off trailing spaces, newlines, etc.
        else
            break;
    }
}

// Adds plst resource 0 to the file given as an argument.  This
// identifies the application to the OS as an NSUIElement, so
// that the application does not show in the Dock and it has no
// menu bar.
int setMacPList() {
    int rc;
    char path[1024], resolvedPath[1024];
    const char plistStr[] = "Application PList";
    CFStringRef plistCFStr = CFStringCreateWithCString(kCFAllocatorDefault, plistStr, kCFStringEncodingMacRoman);
    ConstStringPtr rsrcName = CFStringGetPascalStringPtr(plistCFStr, kCFStringEncodingMacRoman);
    if (!rsrcName) {
        return -2;
    }

    // If resource already exists, don't call getPathToThisApp()
    // which leaves a zombie process.
    if (GetResource('plst', 0)) {
        return 0;
    }

    getPathToThisApp(path, sizeof(path));
    if (path[0] == 0)
        return -1; // Should never happen

    setMacRsrcForFile(path, MacPListData, sizeof(MacPListData), 'plst', 0, rsrcName);
    boinc_resolve_filename(path, resolvedPath, sizeof(resolvedPath));
    rc = setMacRsrcForFile(resolvedPath, MacPListData, sizeof(MacPListData), 'plst', 0, rsrcName);

    CFRelease(plistCFStr);
    return rc;
}


// Adds icns resource to the file given as an argument.
// If the file is a soft link, also adds icns resource to the resolved flle.
// Typically called from a main() with argv[0] to attach resources to itself */
int setMacIcon(char *filename, char *iconData, long iconSize) {
    int rc;
    char path[1024];
    const char iconStr[] = "Application icons";
    CFStringRef iconCFStr = CFStringCreateWithCString(kCFAllocatorDefault, iconStr, kCFStringEncodingMacRoman);
    ConstStringPtr rsrcName = CFStringGetPascalStringPtr(iconCFStr, kCFStringEncodingMacRoman);  // FIXME: How to release this?
    if (!rsrcName) {
        return -2;
    }

    setMacRsrcForFile(filename, iconData, iconSize, 'icns', RESIDICON, rsrcName);
    boinc_resolve_filename(filename, path, sizeof(path));
    rc = setMacRsrcForFile(path, iconData, iconSize, 'icns', RESIDICON, rsrcName);

    CFRelease(iconCFStr);
    return rc;
}
