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


#ifdef __ppc__
#include <Carbon/Carbon.h>
#endif

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
#include "filesys.h"

// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <dlfcn.h>

// Functions available only in OS 10.5 and later
    typedef int     (*backtraceProc)(void**,int);
    typedef char ** (*backtrace_symbolsProc)(void* const*,int);
#define CALL_STACK_SIZE 128

extern void * _sigtramp;

enum {
        kFrameCount = 200
};

#define SKIPFRAME 4 /* Number frames overhead for signal handler and backtrace */

static void PrintOSVersion(char *minorVersion);

void PrintBacktrace(void) {
    int                         err;
    QCrashReportRef             crRef = NULL;

    char                        nameBuf[256], pathToThisProcess[1024];
    const NXArchInfo            *localArch;
    char                        OSMinorVersion = '?';
    time_t                      t;
    char                        atosPipeBuf[1024], cppfiltPipeBuf[1024];
    char                        outBuf[1024], offsetBuf[32];
    char                        *sourceSymbol, *symbolEnd;
    char                        **symbols = NULL;
    void                        *callstack[CALL_STACK_SIZE];
    int                         frames, i;
    void                        *systemlib = NULL;
    FILE                        *atosPipe = NULL;
    FILE                        *cppfiltPipe = NULL;
    backtraceProc               myBacktraceProc = NULL;
    backtrace_symbolsProc       myBacktrace_symbolsProc = NULL;
    char                        saved_env[128], *env = NULL;
    bool                        atosExists = false, cppfiltExists = false;

#if 0
// To debug backtrace logic:
//  * Enable this block of code.
//  * Set a breakpoint at sleep(1) call, and wherever else you wish.
//  * Launch built development application from Finder.
//  * Get this application's pid from Activity Monitor.
//  * Attach Debugger to this application.
//  * Continue until you reach this breakpoint.
//  * Change wait variable to 0 (false).
// This is necessary because GDB intercepts signals even if you tell it 
// not to, so you must attach GDB after the signal handler is invoked.

    bool wait = true;
    
    while (wait) {
        fprintf(stderr, "waiting\n");
        sleep(1);
    }
#endif

    GetNameOfAndPathToThisProcess(nameBuf, sizeof(nameBuf), pathToThisProcess, sizeof(pathToThisProcess));
    
    if (nameBuf[0]) {
        fprintf(stderr, "\nCrashed executable name: %s\n", nameBuf);
    }
    
#ifdef BOINC_VERSION_STRING
    fprintf(stderr, "built using BOINC library version %s\n", BOINC_VERSION_STRING);
#endif

    localArch = NXGetLocalArchInfo();
    fprintf(stderr, "Machine type %s", localArch->description);
#ifdef __LP64__
    fprintf(stderr, " (64-bit executable)\n");
#else
    fprintf(stderr, " (32-bit executable)\n");
#endif

    PrintOSVersion(&OSMinorVersion);

    time(&t);
    fputs(asctime(localtime(&t)), stderr);
    fputc('\n', stderr);
    
    err = QCRCreateFromSelf(&crRef);

    if (OSMinorVersion == '5') {
#ifdef __ppc__
        fputs("BOINC backtrace under OS 10.5.x only shows exported (global) symbols\n", stderr);
        fputs("and may work poorly on a PowerPC Mac after a crash.  For a better\n", stderr);
        fputs("backtrace, run under OS 10.4.x.\n\n", stderr);
#else
        fputs("BOINC backtrace under OS 10.5.x only shows exported (global) symbols\n", stderr);
        fputs("and may not show the final location which caused a crash.  For a better\n", stderr);
        fputs("backtrace, either run under OS 10.4.x or run under OS 10.6.x or later.\n\n", stderr);
#endif
    }
    
    if (OSMinorVersion >= '5') {
        // Use new backtrace functions if available (only in OS 10.5 and later)
        systemlib = dlopen ("/usr/lib/libSystem.dylib", RTLD_NOW );
        if (systemlib) {
            myBacktraceProc = (backtraceProc)dlsym(systemlib, "backtrace");
         }
        if (! myBacktraceProc) {
            goto skipBackTrace;     // Should never happen
        }
        frames = myBacktraceProc(callstack, CALL_STACK_SIZE);
        myBacktrace_symbolsProc = (backtrace_symbolsProc)dlsym(systemlib, "backtrace_symbols");
        if (myBacktrace_symbolsProc) {
            symbols = myBacktrace_symbolsProc(callstack, frames);
        } else {
            goto skipBackTrace;     // Should never happen
        }
        
        atosExists = boinc_file_exists("/usr/bin/atos");
        cppfiltExists = boinc_file_exists("/usr/bin/atos");
        if (atosExists || cppfiltExists) {
            // The bidirectional popen only works if the NSUnbufferedIO environment 
            // variable is set, so we save and restore its current value.
            env = getenv("NSUnbufferedIO");
            if (env) {
                strlcpy(saved_env, env, sizeof(saved_env));
            }
            setenv("NSUnbufferedIO", "YES", 1);
        }
        
        if (atosExists) {
            // The backtrace_symbols() and backtrace_symbols() APIs are limited to 
            // external symbols only, so we also use the atos command-line utility  
            // which gives us debugging symbols when available.
            //
            // For some reason, using the -p option with the value from getpid() 
            // fails here but the -o option with a path does work.
#ifdef __x86_64__
            snprintf(atosPipeBuf, sizeof(atosPipeBuf), "/usr/bin/atos -o \"%s\" -arch x86_64", pathToThisProcess);
#elif defined (__i386__)
            snprintf(atosPipeBuf, sizeof(atosPipeBuf), "/usr/bin/atos -o \"%s\" -arch i386", pathToThisProcess);
#else
            snprintf(atosPipeBuf, sizeof(atosPipeBuf), "/usr/bin/atos -o \"%s\" -arch ppc", pathToThisProcess);
#endif

            atosPipe = popen(atosPipeBuf, "r+");
            if (atosPipe) {
                setbuf(atosPipe, 0);
            }
        }

        if (cppfiltExists) {
            cppfiltPipe = popen("/usr/bin/c++filt -s gnu-v3 -n", "r+");
            if (cppfiltPipe) {
                setbuf(cppfiltPipe, 0);
            }
        }
        
        for (i=0; i<frames; i++) {
            strlcpy(outBuf, symbols[i], sizeof(outBuf));
            if (cppfiltPipe) {
                sourceSymbol = strstr(outBuf, "0x");
                if (sourceSymbol) {
                    sourceSymbol = strchr(sourceSymbol, (int)'_');
                    if (sourceSymbol) {
                        strlcpy(cppfiltPipeBuf, sourceSymbol, sizeof(cppfiltPipeBuf)-1);
                        *sourceSymbol = '\0';
                        symbolEnd = strchr(cppfiltPipeBuf, (int)' ');
                        if (symbolEnd) {
                            strlcpy(offsetBuf, symbolEnd, sizeof(offsetBuf));
                            *symbolEnd = '\0';
                        }
                        fprintf(cppfiltPipe, "%s\n", cppfiltPipeBuf);
                        BT_PersistentFGets(cppfiltPipeBuf, sizeof(cppfiltPipeBuf), cppfiltPipe);
                        symbolEnd = strchr(cppfiltPipeBuf, (int)'\n');
                        if (symbolEnd) {
                            *symbolEnd = '\0';
                        }
                        strlcat(outBuf, cppfiltPipeBuf, sizeof(outBuf));
                        strlcat(outBuf, offsetBuf, sizeof(outBuf));
                    }
                }
            }
            
            if (atosPipe) {
                fprintf(atosPipe, "%#llx\n", (QTMAddr)callstack[i]);
                BT_PersistentFGets(atosPipeBuf, sizeof(atosPipeBuf), atosPipe);
                sourceSymbol = strstr(atosPipeBuf, "0x");
                if (!sourceSymbol) {        // If atos returned a symbol (not just a hex value)
                    sourceSymbol = strstr(outBuf, "0x");
                    if (sourceSymbol) sourceSymbol = strstr(sourceSymbol, " ");
                    if (sourceSymbol) *++sourceSymbol = '\0'; // Remove questionable symbol from backtrace_symbols()
                    strlcat(outBuf, " ", sizeof(outBuf));
                    strlcat(outBuf, atosPipeBuf, sizeof(outBuf));
                    symbolEnd = strchr(outBuf, (int)'\n');
                    if (symbolEnd) {
                        *symbolEnd = '\0';
                    }
                }
            }
            fprintf(stderr, "%s\n", outBuf);
        }

        if (atosPipe) {
            pclose(atosPipe);
        }
        
        if (cppfiltPipe) {
            pclose(cppfiltPipe);
        }

        if (atosExists || cppfiltExists) {
            if (env) {
                setenv("NSUnbufferedIO", saved_env, 1);
            } else {
                unsetenv("NSUnbufferedIO");
            }
        }
        
skipBackTrace:
        fprintf(stderr, "\n");
    } else {
    // Not OS 10.5.x
        QCRPrintBacktraces(crRef, stderr);
    }

    // make sure this much gets written to file in case future 
    // versions of OS break our crash dump code beyond this point.
    fflush(stderr);
    
    QCRPrintThreadState(crRef, stderr);
    QCRPrintImages(crRef, stderr);
}


char * BT_PersistentFGets(char *buf, size_t buflen, FILE *f) {
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


void GetNameOfAndPathToThisProcess(char *nameBuf, size_t nameBufLen, char* outbuf, size_t outBufLen) {
    FILE *f;
    char buf[256], *p, *q=NULL;
    pid_t aPID = getpid();
    size_t nameLen;

    *outbuf = '\0';
    *nameBuf = '\0';
    
    sprintf(buf, "ps -wo command -p %d", (int)aPID);
    f = popen(buf, "r");
    if (f == NULL)
        return;
    
    BT_PersistentFGets (outbuf, outBufLen, f);     // Discard header line
    BT_PersistentFGets (outbuf, outBufLen, f);     // Get the UNIX command which ran us
    pclose(f);

    sprintf(buf, "ps -p %d -c -o command", aPID);
    f = popen(buf,  "r");
    if (!f)
        return;
    BT_PersistentFGets(nameBuf, nameBufLen, f);    // Discard header line
    BT_PersistentFGets(nameBuf, nameBufLen, f);    // Get just the name of our application
    pclose(f);

    // Remove trailing newline if present
    p = strchr(nameBuf, '\n');
    if (p)
        *p = '\0';
    
    // Strip off any arguments
    p = outbuf;
    nameLen = strlen(nameBuf);
    // Find last instance of string nameBuf in string outbuf
    p = strnstr(p, nameBuf, outBufLen);
    while (p) {
        q = p;
        p = strnstr(q + nameLen, nameBuf, outBufLen - (q - nameBuf));
    }
    // Terminate the string immediately after path
    if (q) {
        q += nameLen;
        *q = '\0';
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
