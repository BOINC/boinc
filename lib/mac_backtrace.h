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
 *  mac_backtrace.h
 *
 */
 
/* This is a rudimentary backtrace generator for boinc project applications.  
*
* It is adapted from Apple Developer Technical Support Sample Code 
*   MoreisBetter / MoreDebugging / MoreBacktraceTest
*  The symbols it displays are not always clean.
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
 
#ifndef _BOINC_BACKTRACE_
#define _BOINC_BACKTRACE_


#ifdef __cplusplus
extern "C" {
#endif

#include <mach/mach.h>

/*	Overview
	--------
	This module implements a number of PowerPC backtrace routines. 
	All of the routines are implemented in terms of a common core. 
	The code is structured in a very generic way.  For example, if 
	you were running on a version of Mach that support inter-machine 
	messaging, it would be feasible to do a backtrace of a PowerPC 
	program from program executing a completely different instruction 
	set architecture (ISA).
	
	Backtraces are inherently processor-specific.  As Mac OS X only 
	runs on PowerPC, I haven't attempted to make this code ISA 
	independent.  Even if I did this work, there would be no way 
	to test it, and I don't believe it shipping untested code.  
	Similarly, I don't have any way to test this with 64-bit PowerPC 
	code.
	
	If you're curious about how PowerPC stack frames work, check out 
	the comments in the implementation file.  The comments in the 
	header focus on how you use these routines.
*/

// These definitions isolate the backtrace algorithm from the specifics 
// of the instruction set architecture that it's being compiled for. 

typedef unsigned long MacBTPPCInst;
typedef unsigned long MacBTPPCAddr;

// The end result of a backtrace is an array of MacBTPPCFrame 
// structures.

struct MacBTPPCFrame {
	MacBTPPCAddr	sp;		// frame pointer for this function invocation
	MacBTPPCAddr	pc;		// PC for this function invocation
	unsigned long  	flags;		// various flags, see below
};
typedef struct MacBTPPCFrame MacBTPPCFrame;

enum {
	kMacBTFrameBadMask      = 0x0001,	// this frame pointer is bad
	kMacBTPCBadMask         = 0x0002,	// this PC is bad
	kMacBTSignalHandlerMask = 0x0004	// this frame is a signal handler
};

/*	Common Parameters
	-----------------
	All of the backtrace routines accept certain common parameters.
	
	  o pc and sp -- For non "Self" routines, these parameters supply 
		the initial program counter and stack pointer for the backtrace.
		
	  o stackBottom and stackTop -- These define the extent of the stack 
		which you are tracing.  If this information isn't handy, supply 
		0 for both.  Supplying meaningful values can reduce the number 
		of bogus frames reported if the stack is corrupt.
	
	  o frameArray and frameArrayCount -- These define an array of stack 
		frames that the routines fill out.  You can supply NULL and 0 
		(respectively) if you're not interested in getting the actual 
		frame data (typically you do this to get the count of the number 
		of frames via frameCount).  The routines do not fail if this 
		buffer is exhausted.  Instead they simply return as many frames 
		as they can and continue tracing, returning an accurate value 
		for frameCount.
		
	  o frameCount -- You can use this to get back an accurate count of 
		the number of frames in the stack.  If you're not interested 
		in this information, you can pass NULL.
*/

void PrintBacktrace(void);
int MacBacktracePPCMachSelf(MacBTPPCAddr stackBottom, MacBTPPCAddr stackTop,
                                MacBTPPCFrame *frameArray, unsigned long frameArrayCount, 
                                unsigned long *frameCount, char OSMinorVersion);

#if defined(__cplusplus)
}
#endif


typedef const void *MacAToSAddr;

enum {
	kMacAToSNoSymbol = 0,
	kMacAToSDyldPubliSymbol,			// supported
	kMacAToSDyldPrivateSymbol,			// supported
	kMacAToSCFMSymbol,				// not yet implemented
	kMacAToSTracebackTableSymbol			// not yet implemented
};
typedef signed long MacAToSSymbolType;

struct MacAToSSymInfo {
	MacAToSSymbolType	symbolType;
	char			symbolName[63];
	unsigned long		symbolOffset;
};
typedef struct MacAToSSymInfo MacAToSSymInfo;

#endif      // _BOINC_BACKTRACE_