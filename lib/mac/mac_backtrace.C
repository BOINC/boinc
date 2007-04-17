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
 
/* This is part of a backtrace generator for boinc project applications.  
*
* Adapted from Apple Developer Technical Support Sample Code QCrashReport
*
* This code handles Mac OS X 10.3.x through 10.4.9.  It may require some 
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
* A very useful shell script to add symbols to a crash dump can be found at:
*  http://developer.apple.com/tools/xcode/symbolizingcrashdumps.html
* Pipe the output of the shell script through c++filt to demangle C++ symbols.
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
#include <time.h>

#include "QSymbols.h"
#include "QMachOImageList.h"
#include "QBacktrace.h"
#include "QCrashReport.h"
#include "mac_backtrace.h"

extern void * _sigtramp;

enum {
        kFrameCount = 200
};

#define SKIPFRAME 4 /* Number frames overhead for signal handler and backtrace */

static void GetNameOfThisApp(char *nameBuf, int buflen);
static void PrintOSVersion(char *minorVersion);

void PrintBacktrace(void) {
    int                     err;
    QCrashReportRef         crRef;

    char                        nameBuf[1024];
    const NXArchInfo            *localArch;
    char                        OSMinorVersion;
    time_t                      t;


    GetNameOfThisApp(nameBuf, sizeof(nameBuf));
    if (nameBuf[0])
        fprintf(stderr, "\nCrashed executable name: %s\n", nameBuf);

#ifdef BOINC_VERSION_STRING
    fprintf(stderr, "built using BOINC library version %s\n", BOINC_VERSION_STRING);
#endif

    localArch = NXGetLocalArchInfo();
    fprintf(stderr, "Machine type %s\n", localArch->description);

    PrintOSVersion(&OSMinorVersion);

    time(&t);
    fputs(asctime(localtime(&t)), stderr);
    
   err = QCRCreateFromSelf(&crRef);

    QCRPrintBacktraces(crRef, stderr);
    QCRPrintThreadState(crRef, stderr);
    QCRPrintImages(crRef, stderr);
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


static void GetNameOfThisApp(char *nameBuf, int buflen) {
    FILE *f;
    char buf[64];
    pid_t myPID = getpid();
    int i;
    
    nameBuf[0] = 0;    // in case of failure
    
    sprintf(buf, "ps -p %d -c -o command", myPID);
    f = popen(buf,  "r");
    if (!f)
        return;
    PersistentFGets(nameBuf, buflen, f);  // Skip over line of column headings
    nameBuf[0] = 0;
    PersistentFGets(nameBuf, buflen, f);  // Get the UNIX command which ran us
    fclose(f);

    for (i=strlen(nameBuf)-1; i>=0; --i) {
        if (nameBuf[i] <= ' ')
            nameBuf[i] = 0;  // Strip off trailing spaces, newlines, etc.
        else
            break;
    }
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
