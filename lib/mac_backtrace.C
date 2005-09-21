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
*  The symbols it displays are not always clean.  This code assumes 
*
* This code handles Mac OS X 10.2.x through 10.4.2.  It may require some 
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

#include <stdio.h>
#include <unistd.h>     // for getpid()
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mac_backtrace.h"

extern void * _sigtramp;

enum {
        kFrameCount = 200
};

static void PrintNameOfThisApp(void);
static void PrintOSVersion(char *minorVersion);
static int OutputFrames(const MacBTPPCFrame *frameArray, unsigned long frameCount, unsigned char lookupSymbolNames);


void PrintBacktrace(void) {
        int                 err;
        MacBTPPCFrame       frames[kFrameCount];
        unsigned long       frameCount;
        unsigned long       validFrames;
        char                OSMinorVersion;

        PrintNameOfThisApp();
        PrintOSVersion(&OSMinorVersion);        

        frameCount = sizeof(frames) / sizeof(*frames);
        err = MacBacktracePPCMachSelf(0, 0, frames, frameCount, &validFrames, OSMinorVersion);
        if (err == 0) {
                if (validFrames > frameCount) {
                        validFrames = frameCount;
                }
                err = OutputFrames(frames, validFrames, true);
        }
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
        fputc('\n', stderr);
    }
    
    fclose(f);
}

static void ReplaceSymbolIfBetter(MacAToSSymInfo *existingSymbol, MacAToSSymbolType symbolType, const char * symbolName, unsigned long symbolOffset) {
	unsigned char replace;
		
	if (existingSymbol->symbolType == kMacAToSNoSymbol) {
		replace = true;
	} else {
		replace = (symbolOffset < existingSymbol->symbolOffset);
	}
	
	if (replace) {
		existingSymbol->symbolType   = symbolType;
                strncpy(existingSymbol->symbolName, symbolName, sizeof(existingSymbol->symbolName)-1);
                existingSymbol->symbolName[sizeof(existingSymbol->symbolName)-1] = '\0';
		existingSymbol->symbolOffset = symbolOffset;
	}
}


// FindOwnerOfPC and GetFunctionName countesy of Ed Wynne.

static const struct mach_header *FindOwnerOfPC(unsigned int pc) {
        unsigned int			count,index,offset,cmdex;
        struct segment_command          *seg;
        struct load_command		*cmd;
        struct mach_header		*header;
        
        count = _dyld_image_count();
        for (index = 0;index < count;index += 1)
        {
                header = (struct mach_header*)_dyld_get_image_header(index);
                offset = _dyld_get_image_vmaddr_slide(index);
                cmd = (struct load_command*)((char*)header + sizeof(struct mach_header));
                for (cmdex = 0;cmdex < header->ncmds;cmdex += 1,cmd = (struct load_command*)((char*)cmd + cmd->cmdsize))
                {
                        switch(cmd->cmd)
                        {
                                case LC_SEGMENT:
                                        seg = (struct segment_command*)cmd;
                                        if ((pc >= (seg->vmaddr + offset)) && (pc < (seg->vmaddr + offset + seg->vmsize)))
                                                return header;
                                        break;
                        }
                }
        }
        
        return NULL;
}

static const char *GetFunctionName(unsigned int pc,unsigned int *offset, unsigned char *publicSymbol) {
        struct segment_command	*seg_linkedit = NULL;
        struct segment_command	*seg_text = NULL;
        struct symtab_command	*symtab = NULL;
        struct load_command		*cmd;
        const struct mach_header*header;
        unsigned int			vm_slide,file_slide;
        struct nlist			*sym,*symbase;
        char					*strings,*name;
        unsigned int			base,index;
        
        header = FindOwnerOfPC(pc);
        if (header != NULL)
        {
                cmd = (struct load_command*)((char*)header + sizeof(struct mach_header));
                for (index = 0;index < header->ncmds;index += 1,cmd = (struct load_command*)((char*)cmd + cmd->cmdsize))
                {
                        switch(cmd->cmd)
                        {
                                case LC_SEGMENT:
                                        if (!strcmp(((struct segment_command*)cmd)->segname,SEG_TEXT))
                                                seg_text = (struct segment_command*)cmd;
                                        else if (!strcmp(((struct segment_command*)cmd)->segname,SEG_LINKEDIT))
                                                seg_linkedit = (struct segment_command*)cmd;
                                        break;
                                
                                case LC_SYMTAB:
                                        symtab = (struct symtab_command*)cmd;
                                        break;
                        }
                }
                
                if ((seg_text == NULL) || (seg_linkedit == NULL) || (symtab == NULL))
                {
                        *offset = 0;
                        return NULL;
                }
                
                vm_slide = (unsigned long)header - (unsigned long)seg_text->vmaddr;
                file_slide = ((unsigned long)seg_linkedit->vmaddr - (unsigned long)seg_text->vmaddr) - seg_linkedit->fileoff;
                symbase = (struct nlist*)((unsigned long)header + (symtab->symoff + file_slide));
                strings = (char*)((unsigned long)header + (symtab->stroff + file_slide));
                
                // Look for a global symbol.
                for (index = 0,sym = symbase;index < symtab->nsyms;index += 1,sym += 1)
                {
                        if (sym->n_type != N_FUN)
                                continue;
                        
                        name = sym->n_un.n_strx ? (strings + sym->n_un.n_strx) : NULL;
                        base = sym->n_value + vm_slide;
                        
                        for (index += 1,sym += 1;index < symtab->nsyms;index += 1,sym += 1)
                                if (sym->n_type == N_FUN)
                                        break;
                        
                        if ((pc >= base) && (pc <= (base + sym->n_value)) && (name != NULL) && (strlen(name) > 0))
                        {
                                *offset = pc - base;
                                *publicSymbol = true;
                                return strdup(name);
                        }
                }
                
                // Look for a reasonably close private symbol.
                for (name = NULL,base = 0xFFFFFFFF,index = 0,sym = symbase;index < symtab->nsyms;index += 1,sym += 1)
                {
                        if ((sym->n_type & 0x0E) != 0x0E)
                                continue;
                        
                        if ((sym->n_value + vm_slide) > pc)
                                continue;
                        
                        if ((base != 0xFFFFFFFF) && ((pc - (sym->n_value + vm_slide)) >= (pc - base)))
                                continue;
                        
                        name = sym->n_un.n_strx ? (strings + sym->n_un.n_strx) : NULL;
                        base = sym->n_value + vm_slide;
                }
                
                *offset = pc - base;
                *publicSymbol = false;
                return (name != NULL) ? strdup(name) : NULL;
        }
        
        *offset = 0;
        return NULL;
}

static int AToSCopySymbolNameUsingDyld(MacAToSAddr address, MacAToSSymInfo *symbol) {
        const char * 		thisSymbol;
        char *                  colonPtr;
        unsigned int 		thisSymbolOffset;
        unsigned char      	thisSymbolPublic;
        MacAToSSymbolType	thisSymbolType;
        int                     err = 0;
        
        thisSymbol = NULL;
        if (address != NULL) {		// NULL is never a useful symbol
                thisSymbol = GetFunctionName( (unsigned int) address, &thisSymbolOffset, &thisSymbolPublic);
        }
        if (thisSymbol != NULL) {
                // Symbols are sometimes followed by a colon and other characters.
                // If there is a colon, replace it with a null terminator
                colonPtr = strchr(thisSymbol, ':');
                if (colonPtr)
                    *colonPtr = '\0';
                
                if (thisSymbolPublic) {
                        thisSymbolType = kMacAToSDyldPubliSymbol;
                } else {
                        thisSymbolType = kMacAToSDyldPrivateSymbol;
                }
                
                if (err == 0) {	
                        ReplaceSymbolIfBetter(symbol, thisSymbolType, thisSymbol, (unsigned long) thisSymbolOffset);
                }
        } else {
            symbol->symbolName[0] = '\0';
        }
        
        free( (void *) thisSymbol);
        
        return err;
}



static int OutputFrames(const MacBTPPCFrame *frameArray, unsigned long frameCount, unsigned char lookupSymbolNames) {
        // Output a textual description of frameCount frames from frameArray.
        // we look up the symbol names of the PCs of each of the frames.

        int                     err;
        unsigned long		frameIndex;
        MacAToSSymInfo          symbol;
        MacAToSAddr             address;

        err = 0;
        
        fputs("Stack Frame backtrace:\n #  Flags Frame Addr  Caller PC   Symbol\n"
                        "===  ===  ==========  ==========  ==========\n", stderr);
        
        for (frameIndex = 0; frameIndex < frameCount; frameIndex++) {
                
                fprintf(stderr, "%3ld  %c%c%c  0x%08lx  0x%08lx  ", 
                                 frameIndex,
                                 (frameArray[frameIndex].flags & kMacBTFrameBadMask)      ? 'F' : '-',
                                 (frameArray[frameIndex].flags & kMacBTPCBadMask)         ? 'P' : '-',
                                 (frameArray[frameIndex].flags & kMacBTSignalHandlerMask) ? 'S' : '-',
                                 frameArray[frameIndex].sp, 
                                 frameArray[frameIndex].pc);
                                                         
                if (frameArray[frameIndex].flags & kMacBTPCBadMask) {
                        address = NULL;
                } else {
                        address = (MacAToSAddr) frameArray[frameIndex].pc;
                }

                symbol.symbolName[0] = '\0';
                symbol.symbolType = kMacAToSNoSymbol;
                symbol.symbolOffset = 0;
                
                err = AToSCopySymbolNameUsingDyld(address, &symbol);
                
                if (symbol.symbolName[0]) {
                    fprintf(stderr, "%s + 0x%lx", 
                                    symbol.symbolName, symbol.symbolOffset);
                }

                fputs("\n", stderr);
        }
        
        return err;
}

#pragma mark ***** PowerPC Backtrace Core

/*	PowerPC Stack Frame Basics
	--------------------------
	
					Size	Purpose
					----	-------
	low memory ->	0x004	pointer to next frame
					0x004	place to save CR
					0x004	place to save LR
					0x008	reserved
	high memory ->	0x004	place to save TOC (CFM only)
					
					To get from one frame to the next, you have to indirect 
					through an offset of 0 (kMacBTPPCOffsetToSP).  To 
					extract the return address from a frame, you have to 
					indirect an offset of 8 (kMacBTPPCOffsetToLR).
*/

enum {
	kMacBTPPCOffsetToSP	= 0,
	kMacBTPPCOffsetToLR 	= 8
};

/*	PowerPC Signal Stack Frames
	---------------------------
	In the current Mac OS X architecture, there is no guaranteed reliable 
	way to backtrace a signal stack frame.  The problem is that the kernel 
	pushes a variable amount of data on to the stack when it invokes the 
	user space signal trampoline (_sigtramp), and the only handle to the 
	information about how much data was pushed is passed in a register 
	parameter to _sigtramp.  _sigtramp stashes that value away in a 
	non-volatile register.  So, when _sigtramp calls the user-supplied 
	signal handler, there's no way to work out where that register 
	ends up being saved.
	
	Thus, we devolve into guesswork.  It turns out that the offset from 
	the stack of the kernel data to the information we need (the place 
	where the interrupted thread's SP was stored) is a constant for any 
	given system release.  So, we can just simply add the appropriate 
	offset to the frame pointer and grab the data we need.
	
	The problem is that this constant varies from release to release. 
	This code handles Mac OS X 10.2.x through 10.4.x.  There's no 
	guarantee that this offset won't change again in the future.

	When the kernel invokes the user space signal trampoline, it pushes 
	the following items on to the stack.
	
	Mac OS X 10.2.x
	---------------
					Size	Purpose
					----	-------
	low memory ->	0x030   bytes for C linkage
					0x040 	bytes for saving PowerPC parameters
					0x008	alignment padding
					0x408   struct mcontext, comprised of:
								 0x020 ppc_exception_state_t
								 0x0A0 ppc_thread_state_t
								 0x108 ppc_float_state_t
								 0x240 ppc_vector_state_t
					0x040	siginfo_t
					0x020	ucontext
	high memory ->	0x0e0	red zone
	
					The previous frame's SP is at offset 0x00C within 
					ppc_thread_state_t, which makes 
					kMacBTPPCOffsetToSignalSPTenTwo equal to 
					0x030 + 0x040 + 0x008 + 0x020 + 0x00C, or 0x0A4.
*/

enum {
	kMacBTPPCOffsetToSignalSPTenOne = 0x07C,
	kMacBTPPCOffsetToSignalSPTenTwo = 0x0A4, 
        // We determined the offsets for OS 10.3 and 10.4 heuristically by 
        // examining the contents of the stack
        kMacBTPPCOffsetToSignalSPTenThree = 0x09C,
        // The same offset seems to work for OS 10.4 as for OS 10.3.  Other 
        // possible values for the offset in 10.4 are 0x0a8 and 0x0c8.  (In 
        // our tests on OS 10.4.2, the same correct frame pointer appeared 
        // at all three of these offsets.)
        kMacBTPPCOffsetToSignalSPTenFour = 0x09C
};

/*	PowerPC Signal Stack Frames (cont)
	----------------------------------
	The only remotely reliable way to detect a signal stack frame is to 
	look at the return address to see whether it points within the 
	_sigtramp routine.  I can find the address of this routine via 
	the dynamic linker, but I don't have an easy way to determine it's 
	length.  So I just guess!  Fortunately, this is rarely a problem.
        And here's the number I chose.  
*/

enum {
	kMacBTPPCSigTrampSize	= 256
};

typedef struct MacBTPPCContext MacBTPPCContext;

typedef int (*MacBTReadBytesProc)(MacBTPPCContext *context, MacBTPPCAddr src, void *dst, unsigned long size);
	// This function pointer is called by the core backtrace code 
	// when it needs to read memory.  The callback should do a safe 
	// read of size bytes from src into the buffer specified by 
	// dst.  By "safe" we mean that the routine should return an error 
	// if the read can't be done (typically because src is a pointer to 
	// unmapped memory).

// The MacBTPPCContext structure is used by the core backtrace code 
// to maintain its state.

struct MacBTPPCContext {

	// Internal parameters that are set up by the caller 
	// of the core backtrace code.
	
	unsigned long			offsetToSignalSP;
	MacBTPPCAddr			sigTrampLowerBound;
	MacBTPPCAddr			sigTrampUpperBound;
	MacBTReadBytesProc              readBytes;
	void *				refCon;
	
	// Parameters from client.
	
	MacBTPPCAddr	pc;
	MacBTPPCAddr	r0;			// see MacBTPPCCheckLeaf
	MacBTPPCAddr	sp;
	MacBTPPCAddr	lr;
	MacBTPPCAddr	stackBottom;
	MacBTPPCAddr	stackTop;
	MacBTPPCFrame   *frameArray;		// array contents filled out by core
	unsigned long 	frameArrayCount;
	unsigned long 	frameCountOut;		// returned by core
};

static int ReadPPCAddr(MacBTPPCContext *context, MacBTPPCAddr addr, MacBTPPCAddr *value) {
	// Reads a PowerPC address (ie a pointer) from the target task, 
	// returning an error if the memory is unmapped.

	return context->readBytes(context, addr, value, sizeof(*value));
}

static int ReadPPCInst(MacBTPPCContext *context, MacBTPPCAddr addr, MacBTPPCInst *value) {
	// Reads a PowerPC instruction from the target task, 
	// returning an error if the memory is unmapped.

	return context->readBytes(context, addr, value, sizeof(*value));
}

static int MacBTPPCCheckLeaf(MacBTPPCContext *context) {
	// The top most frame may be in a weird state because of the 
	// possible variations in the routine prologue.  There are a 
	// variety of combinations, such as:
	//
	// 1. a normal routine, with its return address stored in 
	//    its caller's stack frame
	//
	// 2. a system call routine, which is a leaf routine with 
	//    no frame and the return address is in LR
	//
	// 3. a leaf routine with no frame, where the return address 
	//    is in LR
	//
	// 4. a leaf routine with no frame that accesses a global, where 
	//    the return address is in R0
	//
	// 5. a normal routine that was stopped midway through 
	//    constructing its prolog, where the return address is 
	//    typically in R0
	//
	// Of these, 1 and 2 are most common, and they're the cases I 
	// handle.  General support for all of the cases requires the 
	// ability to accurately determine the start of the routine 
	// which is not something that I can do with my current 
	// infrastructure.
	//
	// Note that don't handle any cases where the return address is 
	// in R0, although I do have a variable for R0 in the context 
	// in case I add that handling in the future.

	int                     err;
	unsigned char           isSystemCall;
	MacBTPPCInst            inst;
	MacBTPPCInst            pc;
	int			count;

	// Using the PC from the top frame (frame[0]), walk back through 
	// the code stream for 3 instructions looking for a "sc" instruction. 
	// If we find one, it's almost certain that we're in a system call 
	// frameless leaf routine.

	isSystemCall = false;
	count = 0;
	pc = context->pc;
	do {
		err = ReadPPCInst(context, pc, &inst);
		if (err == 0) {
			isSystemCall = (inst == 0x44000002);			// PPC "sc" instruction
		}
		if ( (err == 0) && ! isSystemCall ) {
			count += 1;
			pc -= sizeof(MacBTPPCInst);
		}
	} while ( (err == 0) && ! isSystemCall && (count < 3) );
	err = 0;
	
	// If we find that we're in a system call frameless leaf routine, 
	// te add a dummy stack frame (with no frame, because the frame actually 
	// belows to frameArray[1]).

	if (isSystemCall) {
		if ( (context->frameArray != NULL) && (context->frameCountOut < context->frameArrayCount) ) {
			MacBTPPCFrame *	frameOutPtr;

			frameOutPtr = &context->frameArray[context->frameCountOut];
			frameOutPtr->pc    = context->pc;
			frameOutPtr->sp    = 0;
			frameOutPtr->flags = kMacBTFrameBadMask;
		}
		context->frameCountOut += 1;

		context->pc = context->lr;
	}

	return err;
}

static int MacBacktracePPCCore(MacBTPPCContext *context) {
	// The core backtrace code.  This routine is called by all of the various 
	// exported routines.  It implements the core backtrace functionality. 
	// All of the parameters to this routine are contained within 
	// the context.  This routine traces back through the stack (using the 
	// readBytes callback in the context to actually read memory) creating 
	// a backtrace.

	int                     err;
	MacBTPPCAddr            thisPC;
	MacBTPPCAddr            thisFrame;
	MacBTPPCAddr            lowerBound;
	MacBTPPCAddr            upperBound;
	unsigned char           stopNow;
	
	lowerBound = context->stackBottom;
	upperBound = context->stackTop;
	if (upperBound == 0) {
		upperBound = (MacBTPPCAddr) -1;
	}
	
	// Check the current PC and add a dummy frame if it points to 
	// a frameless leaf routine.
	
	context->frameCountOut = 0;
	err = MacBTPPCCheckLeaf(context);
	
	// Handle the normal frames.
	
	if (err == 0) {
		thisPC     = context->pc;
		thisFrame  = context->sp;
		
		stopNow = false;
		do {
			MacBTPPCFrame           * frameOutPtr;
			MacBTPPCFrame     	tmpFrameOut;
			MacBTPPCAddr 		nextFrame;
			MacBTPPCAddr 		nextPC;
			MacBTPPCInst		junkInst;
			
			// Output to a tmpFrameOut unless the client has supplied 
			// a buffer and there's sufficient space left in it.
			
			if ( (context->frameArray != NULL) && (context->frameCountOut < context->frameArrayCount) ) {
				frameOutPtr = &context->frameArray[context->frameCountOut];
			} else {
				frameOutPtr = &tmpFrameOut;
			}
			context->frameCountOut += 1;

			// Record this entry.
			
			frameOutPtr->pc    = thisPC;
			frameOutPtr->sp    = thisFrame;
			frameOutPtr->flags = 0;
			
			// Now set the flags to indicate the validity of specific information. 
			
			// Check the validity of the PC.  Don't set the err here; a bad PC value 
			// does not cause us to quit the backtrace.
			
			if ( (((int) thisPC) & 0x03) || (ReadPPCInst(context, thisPC, &junkInst) != 0) ) {
				frameOutPtr->flags |= kMacBTPCBadMask;
			}
                        			
			// Check the validity of the frame pointer.  A bad frame pointer *does* 
			// cause us to stop tracing.
			
			if ( (thisFrame == 0L) || (((int) thisFrame) & 0x03) || (thisFrame < lowerBound) || (thisFrame >= upperBound) ) {
				frameOutPtr->flags |= kMacBTFrameBadMask;
				stopNow = true;
			}

			if (err == 0 && ! stopNow) {
			
				// Read the next frame pointer.  Again, a failure here causes us to quit 
				// backtracing.  Note that we set kMacBTFrameBadMask in frameOutPtr 
				// because, if we can't read the contents of the frame pointer, the 
				// frame pointer itself must be bad.
				
				err = ReadPPCAddr(context, thisFrame + kMacBTPPCOffsetToSP, &nextFrame);
				if (err != 0) {
					frameOutPtr->flags |= kMacBTFrameBadMask;
					// No need to set stopNow because err != 0 will 
					// terminate loop.
				}

				// If the next frame pointer indicates that this frame was called 
				// as a signal handler, handle the discontinuity in the stack.
				
				if (err == 0) {
					// Extract the LR from the stack frame.    Note that we have to do 
					// this before we check for a signal frame because the PC of 
					// the frame that was interrupted by the signal is stored 
					// in this nextFrame, not in the one we'll get by delving 
					// into the signal handler stack block.
					
					if ( ReadPPCAddr(context, nextFrame + kMacBTPPCOffsetToLR, &nextPC) != 0 ) {
						nextPC = (MacBTPPCAddr) -1;		// an odd value, to trigger above check on next iteration
					}
					
					// delving into the signal handler stack block.
					
					if (      !(frameOutPtr->flags & kMacBTPCBadMask) 
							&& ( frameOutPtr->pc >= context->sigTrampLowerBound ) 
							&& ( frameOutPtr->pc <  context->sigTrampUpperBound ) ) {
						frameOutPtr->flags |= kMacBTSignalHandlerMask;
#if 0
                                                // This code allows us to examine the stack to find the correct 
                                                // value for offsetToSignalSP for new releases of OS X.  We 
                                                // don't use it in production
                                                for (long index=0;index<256;index+=4) {
                                                    MacBTPPCAddr value;
                                                    ReadPPCAddr(context, nextFrame + index, &value);
                                                    fprintf(stderr, "offset %lx: value = %lx\n", index, value);
                                                }
#endif
						err = ReadPPCAddr(context, nextFrame + context->offsetToSignalSP, &nextFrame);
					}
				}

				// Set up for the next iteration.
				
				if (err == 0) {
					lowerBound = thisFrame;
					thisPC     = nextPC;
					thisFrame  = nextFrame;
				}
			}
		} while ( (err == 0) && ! stopNow );
	}
	
	return err;
}

static int InitMacBTPPCContext(MacBTPPCContext *context,
                                    MacBTPPCAddr stackBottom, MacBTPPCAddr stackTop,
                                    MacBTPPCFrame *frameArray, unsigned long frameArrayCount, 
                                    char OSMinorVersion) {
	// Initialises a MacBTPPCContext to appropriate default values.
	int err;
		
	memset(context, 0, sizeof(context));
	
	// We don't check the input parameters here.  Instead the 
	// check is done by the backtrace core.
	
	context->stackBottom     = stackBottom;
	context->stackTop        = stackTop;
	context->frameArray      = frameArray;
	context->frameArrayCount = frameArrayCount;
	
	// Some system version specific parameters:
	//
	// o _sigtramp is irrelevant on traditional Mac OS.
	// o We don't support Mac OS X 10.0.x.
	// o offsetToSignalSP changed between 10.1.x and 10.2.x.
	
	err = 0;

        if (OSMinorVersion == '3')
            context->offsetToSignalSP = kMacBTPPCOffsetToSignalSPTenThree;
        else
            context->offsetToSignalSP = kMacBTPPCOffsetToSignalSPTenFour;
        
        context->sigTrampLowerBound = (MacBTPPCAddr) & _sigtramp;

        // We can't actually determine the size of _sigtramp with our current 
        // technology, so we just guess at the upper bound.
        context->sigTrampUpperBound = context->sigTrampLowerBound + kMacBTPPCSigTrampSize;
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Mach Interface

// The Mach interface works accesses all backtrace memory via 
// Mach VM calls, and thus there's the potential for it to execute 
// on a instruction set architecture other than the one being 
// backtraced.  Hence, there's no requirement for TARGET_CPU_PPC here.

static int MacBTReadBytesMach(MacBTPPCContext *context, MacBTPPCAddr src, void *dst, unsigned long size) {
        // A memory read callback for Mach (see MacBTReadBytesProc). 
        // This simply calls through to the Mach vm_read_overwrite 
        // primitive, which does exactly what we want.
        int             err;
        vm_size_t 	sizeRead;
        
        sizeRead = size;
        err = vm_read_overwrite( (thread_t) context->refCon, (vm_address_t) src, size, (vm_address_t) dst, &sizeRead);
        if ( (err == 0) && (sizeRead != size) ) {
                err = -1;
        }
        return err;
}

int MacBacktracePPCMach(task_t task, MacBTPPCAddr pc, MacBTPPCAddr sp,
                            MacBTPPCAddr stackBottom, MacBTPPCAddr stackTop,
                            MacBTPPCFrame *frameArray, unsigned long frameArrayCount, 
                            unsigned long *frameCount, char OSMinorVersion) {
        // See comments in header.
        int 			err;
        MacBTPPCContext 	context;
        
        err = InitMacBTPPCContext(&context, stackBottom, stackTop, frameArray, frameArrayCount, OSMinorVersion);
        if (err == 0) {
                context.pc = pc;
                context.sp = sp;
                context.readBytes = MacBTReadBytesMach;
                context.refCon    = (void *) task;
                
                err = MacBacktracePPCCore(&context);
        }
        if (frameCount != NULL) {
                *frameCount = context.frameCountOut;
        }
        return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Assembly Stuff

// The following is an inline assembly abstraction layer that isolates 
// the rest of the code from the specific compiler's flavour of assembly. 
// Note that this is not as clean as I'd like it to be.  Specifically, 
// MacBTPPCGetProgramCounter is a nice function definition, but 
// MacBTPPCGetStackPointer is a macro.  If I define MacBTPPCGetStackPointer 
// as a function, I have problems because GCC insists on building a 
// stack from in non-optimised builds but no stack frame in optimised 
// builds.  That makes things tricky.  So, instead, I use a macro that 
// expands inside the function itself.  This causes other issues.  
// For example, in MacBacktracePPCCarbonSelf I had to name the local 
// variable "mySP" and not "sp", because otherwise the CodeWarrior 
// inline assembler can't distinguish between the variable and the 
// register.  Also, mySP has to qualified as "register" for the sake 
// of the CodeWarrior inline assembler.  Fortunately, these changes 
// do not cause problems for GCC.

// Obviously the "Self" calls require TARGET_CPU_PPC.


#define MacBTPPCGetStackPointer(result)						\
        __asm__ volatile("mr		%0,r1" : "=r" (result));

static MacBTPPCAddr MacBTPPCGetProgramCounter(void) {
    MacBTPPCAddr result;
    __asm__ volatile("mflr		%0" : "=r" (result));
    return result;
}

int MacBacktracePPCMachSelf(MacBTPPCAddr stackBottom, MacBTPPCAddr stackTop,
                                MacBTPPCFrame *frameArray, unsigned long frameArrayCount, 
                                unsigned long *frameCount, char OSMinorVersion) {
        // See comments in header.
        register MacBTPPCAddr   mySP;
        MacBTPPCAddr            myPC;
        
        // For Mac information about these inline assembly routines, 
        // see the "***** Assembly Stuff" comment above.

        MacBTPPCGetStackPointer(mySP);
        myPC = MacBTPPCGetProgramCounter();
        
        return MacBacktracePPCMach(mach_task_self(), myPC, mySP, stackBottom, stackTop, frameArray, frameArrayCount, frameCount, OSMinorVersion);
}
