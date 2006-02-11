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
/*
 *  mac_backtrace.C
 *
 */
 
/* This is a rudimentary backtrace generator for boinc project applications.  
*
* It is adapted from Apple Developer Technical Support Sample Code 
*   MoreisBetter / MoreDebugging / MoreBacktraceTest
*  The symbols it displays are not always clean.  
*
* This code handles Mac OS X 10.3.x through 10.4.2.  It may require some 
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
*  Flags in backtrace:
*    F this frame pointer is bad
*    P this PC is bad
*    S this frame is a signal handler
*
*/


#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <mach-o/arch.h>

#include <stdio.h>
#include <unistd.h>     // for getpid()
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mac_backtrace.h"
#include "MoreBacktrace.h"
#include "MoreAddrToSym.h"

extern void * _sigtramp;

enum {
        kFrameCount = 200
};

#define SKIPFRAME 4 /* Number frames overhead for signal handler and backtrace */

static void PrintNameOfThisApp(void);
static void PrintOSVersion(char *minorVersion);
static int OutputFrames(const MoreBTFrame *frameArray, unsigned long frameCount, unsigned char lookupSymbolNames);

void PrintBacktrace(void) {
        int                 err;
        MoreBTFrame         frames[kFrameCount];
        unsigned long       frameCount;
        unsigned long       validFrames;
        char                OSMinorVersion;

        PrintNameOfThisApp();
        PrintOSVersion(&OSMinorVersion);

        frameCount = sizeof(frames) / sizeof(*frames);
        err = MoreBacktraceMachSelf(0, 0, frames, frameCount, &validFrames);
        if (err == 0) {
                if (validFrames > frameCount) {
                        validFrames = frameCount;
                }
                err = OutputFrames(frames, validFrames, true);
        }
        fflush(stderr);
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


static void PrintNameOfThisApp() {
    FILE *f;
    char buf[64], nameBuf[1024];
    pid_t myPID = getpid();
    int i;
    const NXArchInfo *localArch;
    
    nameBuf[0] = 0;    // in case of failure
    
    sprintf(buf, "ps -p %d -c -o command", myPID);
    f = popen(buf,  "r");
    if (!f)
        return;
    PersistentFGets(nameBuf, sizeof(nameBuf), f);  // Skip over line of column headings
    nameBuf[0] = 0;
    PersistentFGets(nameBuf, sizeof(nameBuf), f);  // Get the UNIX command which ran us
    fclose(f);

    for (i=strlen(nameBuf)-1; i>=0; --i) {
        if (nameBuf[i] <= ' ')
            nameBuf[i] = 0;  // Strip off trailing spaces, newlines, etc.
        else
            break;
    }
    
    if (nameBuf[0])
        fprintf(stderr, "\nCrashed executable name: %s\n", nameBuf);

#ifdef BOINC_VERSION_STRING
    fprintf(stderr, "built using BOINC library version %s\n", BOINC_VERSION_STRING);
#endif

    localArch = NXGetLocalArchInfo();
    fprintf(stderr, "Machine type %s\n", localArch->description);

}


// This is an alternative to using Gestalt(gestaltSystemVersion,..) so 
// we don't need the Carbon Framework
static void PrintOSVersion(char *OSMinorVersion) {
    char buf[1024], *p1 = NULL, *p2 = NULL, *p3;
    FILE *f;
    int n;
    
    f = fopen("/System/Library/CoreServices/SystemVersion.plist", "r");
    if (!f)
        return;
        
    n = fread(buf, 1, sizeof(buf)-1, f);
    buf[n] = '\0';
    p1 = strstr(buf, "<key>ProductUserVisibleVersion</key>");
    if (p1) {
        p1 = strstr(p1, "<string>") + 8;
        p2 = strstr(p1, "</string>");
        if (p2) {
            // Extract the minor system version number character
            p3 = strchr(p2, '.');
            *OSMinorVersion = *(p3+1);    // Pass minor version number back to caller
            // Now print the full OS version string
            fputs("System version: Macintosh OS ", stderr);
            while (p1 < p2) {
                fputc(*p1++, stderr);
            }
        }
    }
    
    if (p2) {
        p2 = NULL;
        p1 = strstr(buf, "<key>ProductBuildVersion</key>");
        if (p1) {
            p1 = strstr(p1, "<string>") + 8;
            p2 = strstr(p1, "</string>");
            if (p2) {
                fputs(" build ", stderr);
                while (p1 < p2) {
                    fputc(*p1++, stderr);
                }
            }
        }
    }
    
    fputc('\n', stderr);

    fclose(f);
}


static int OutputFrames(const MoreBTFrame *frameArray, unsigned long frameCount, unsigned char lookupSymbolNames) {
        // Output a textual description of frameCount frames from frameArray.
        // we look up the symbol names of the PCs of each of the frames.
        // This assumes we are not running a 64-bit application

        int                     err;
        unsigned long		frameIndex, skipframe = 0;
        MoreAToSSymInfo         symbol;
        MoreAToSAddr            address;

        err = 0;
        
        if ((frameCount >= SKIPFRAME) && (frameArray[SKIPFRAME-1].flags & kMoreBTSignalHandlerMask))
            skipframe = SKIPFRAME;
        
        fputs("Stack Frame backtrace:\n #  Flags Frame Addr  Caller PC   Return Address Symbol\n"
                        "===  ===  ==========  ==========  =====================\n", stderr);
        
        for (frameIndex = skipframe; frameIndex < frameCount; frameIndex++) {
                
                fprintf(stderr, "%3ld  %c%c%c  0x%08llx  0x%08llx  ", 
                                 frameIndex - skipframe + 1,
                                 (frameArray[frameIndex].flags & kMoreBTFrameBadMask)      ? 'F' : '-',
                                 (frameArray[frameIndex].flags & kMoreBTPCBadMask)         ? 'P' : '-',
                                 (frameArray[frameIndex].flags & kMoreBTSignalHandlerMask) ? 'S' : '-',
                                 frameArray[frameIndex].fp, 
                                 frameArray[frameIndex].pc);
                                                         
                if (frameArray[frameIndex].flags & kMoreBTPCBadMask) {
                        address = 0;
                } else {
                        address = (MoreAToSAddr) frameArray[frameIndex].pc;
                }

                symbol.symbolName = NULL;
                symbol.symbolType = kMoreAToSNoSymbol;
                symbol.symbolOffset = 0;
                
                err = MoreAToSCopySymbolNamesUsingDyld(1, &address, &symbol);
                
                if (symbol.symbolName) {
                    if (symbol.symbolName[0]) {
                        fprintf(stderr, "%s + 0x%llx", 
                                        symbol.symbolName, symbol.symbolOffset);
                        free( (void *) symbol.symbolName);
                    }
                }

                fputs("\n", stderr);
        }
        
        return err;
}
