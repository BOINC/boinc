// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
#include "boinc_api.h"

#define RESIDICON -16455

char MacPListData[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "
"\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
"<plist version=\"1.0\">\n<dict>\n"
"\t<key>NSUIElement</key>\n\t<string>1</string>\n</dict>\n</plist>\n";


/* checks for an OS error, reports the line number and returns */
#define CHECK_OSERR(e) {\
  if (e) {\
    fprintf(stderr,"MacOS Error %d occured in Mac_Lib.c line %d\n",e,__LINE__);\
    return(e); } }

// Adds ther specified resource to the file given as an argument.
int setMacRsrcForFile(char *filename, char *rsrcData, long rsrcSize, 
                            OSType rsrcType, int rsrcID, StringPtr rsrcName) {
    OSErr oserr;        /* stores an OS error code */
    FSSpec fsspec;      /* FileSpec */
    FSRef fsref;        /* File Reference */
    FInfo finfo;        /* File Info */
    short rref;         /* Resource Reference */
    Handle hand;
    int retry;

    /* get finder spec for this file */
    CHECK_OSERR(FSPathMakeRef((StringPtr)filename, &fsref, NULL));
    CHECK_OSERR(FSGetCatalogInfo(&fsref, NULL, NULL, NULL, &fsspec, NULL));

    /* Open the resource fork for writing, create it if it does not exist.
        On a dual-processor system, the other cpu may have the resource fork 
        open for writing, so if we fail we wait and retry.
    */
    for (retry=0;retry<5;retry++) {
        rref = FSpOpenResFile(&fsspec, fsRdWrPerm);
        oserr = ResError();
        if (oserr == eofErr) { /* EOF, resource fork/file not found */
            // If we set file type and signature to non-NULL, it makes OS mistakenly
            // identify file as a classic application instead of a UNIX executable.
            FSpCreateResFile(&fsspec, 0, 0, smRoman);
            oserr = ResError();
            if (oserr == noErr) {
                rref = FSpOpenResFile(&fsspec, fsRdWrPerm);
                oserr = ResError();
            }
        }
        if (oserr == noErr)
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
        CHECK_OSERR(FSpGetFInfo(&fsspec, &finfo));
        finfo.fdFlags |= kHasCustomIcon;
        CHECK_OSERR( FSpSetFInfo(&fsspec, &finfo));
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
    char buf[64], *c;
    pid_t myPID = getpid();
    int i;
    
    *pathBuf = 0;    // in case of failure
    
    sprintf(buf, "ps -p %d -o command", myPID);
    f = popen(buf,  "r");
    if (!f)
        return;
    PersistentFGets(pathBuf, bufSize, f);  // Skip over line of column headings
    PersistentFGets(pathBuf, bufSize, f);  // Get the UNIX command which ran us
    fclose(f);

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
// identifies the applciation to the OS as an NSUIElement, so 
// that the application does not show in the Dock and it has no 
// menu bar.
int setMacPList() {
    char path[1024], resolvedPath[1024];;
    StringPtr rsrcName = (StringPtr)"\pApplication PList";

    getPathToThisApp(path, sizeof(path));
    if (path[0] == 0)
        return -1; // Should never happen
    
    setMacRsrcForFile(path, MacPListData, sizeof(MacPListData), 'plst', 0, rsrcName);
    boinc_resolve_filename(path, resolvedPath, sizeof(resolvedPath));
    return(setMacRsrcForFile(resolvedPath, MacPListData, sizeof(MacPListData), 'plst', 0, rsrcName));
}


// Adds icns resource to the file given as an argument.
// If the file is a soft link, also adds icns resource to the resolved flle. 
// Typically called from a main() with argv[0] to attach resources to itself */
int setMacIcon(char *filename, char *iconData, long iconSize) {
    char path[1024];
    StringPtr rsrcName = (StringPtr)"\pApplication icons";

    setMacRsrcForFile(filename, iconData, iconSize, 'icns', RESIDICON, rsrcName);
    boinc_resolve_filename(filename, path, sizeof(path));
    return(setMacRsrcForFile(path, iconData, iconSize, 'icns', RESIDICON, rsrcName));
}
const char *BOINC_RCSID_112a101526="$Id$";
