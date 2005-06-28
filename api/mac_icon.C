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

/* checks for an OS error, reports the line number and returns */
#define CHECK_OSERR(e) {\
  if (e) {\
    fprintf(stderr,"MacOS Error %d occured in Mac_Lib.c line %d\n",e,__LINE__);\
    return(e); } }

// Adds icns resource to the file given as an argument.
int setMacIconForFile(char *filename, char *iconData, long iconSize) {
    OSErr oserr;        /* stores an OS error code */
    FSSpec fsspec;      /* FileSpec */
    FSRef fsref;        /* File Reference */
    FInfo finfo;        /* File Info */
    short rref;         /* Resource Reference */
    Handle hand;

    /* get finder spec for this file */
    CHECK_OSERR(FSPathMakeRef((StringPtr)filename, &fsref, NULL));
    CHECK_OSERR(FSGetCatalogInfo(&fsref, NULL, NULL, NULL, &fsspec, NULL));

    /* open the resource fork, create it if it does not exist */
    rref = FSpOpenResFile(&fsspec, fsWrPerm);
    oserr = ResError();
    if (oserr == -39) { /* EOF, resource fork/file not found */
        // If we set file type and signature to non-NULL, it makes OS mistakenly
        // identify file as a classic application instead of a UNIX executable.
        FSpCreateResFile(&fsspec, 0, 0, smRoman);
        CHECK_OSERR(ResError());
        rref = FSpOpenResFile(&fsspec, fsWrPerm);
        CHECK_OSERR(ResError());
    } else {
        CHECK_OSERR(oserr);
    }

    /* attach icns resource */
    if (!GetResource('icns', RESIDICON)) { /* resource not found */
        CHECK_OSERR(PtrToHand(iconData, &hand, iconSize));
        AddResource(hand, 'icns', RESIDICON, "\pApplication icons");
        CHECK_OSERR(ResError());
    }

    /* add this to the file on disk */
    CloseResFile(rref);
    CHECK_OSERR(ResError());

    /* set custom icon flag */
    CHECK_OSERR(FSpGetFInfo(&fsspec, &finfo));
    finfo.fdFlags |= kHasCustomIcon;
    CHECK_OSERR( FSpSetFInfo(&fsspec, &finfo));
    
    return(0);
}


// Adds icns resource to the file given as an argument.
// If the file is a soft link, also adds icns resource to the resolved flle. 
// Typically called from a main() with argv[0] to attach resources to itself */
int setMacIcon(char *filename, char *iconData, long iconSize) {
    char path[256];

    setMacIconForFile(filename, iconData, iconSize);
    boinc_resolve_filename(filename, path, sizeof(path));
    return(setMacIconForFile(path, iconData, iconSize));
}
