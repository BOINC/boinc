/***************************************************************************
 *  HeapCheck - a heap debugging library
 *  Copyright (C) 2001  Thanassis Tsiodras (ttsiod@softlab.ntua.gr)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **************************************************************************
 *
 * HeapCheck 1.2
 * =============
 *
 * A simple debugging allocator, designed to help heap bug-hunting.
 * Like most debugging heap allocators, it helps you find memory leaks.
 * It also helps you where others can't: in catching invalid accesses.
 * It uses the paging system to create inaccessible pages by the side
 * of your normal allocations. This way, accesses outside the heap-
 * allocated space cause an exception (even READ accesses). This is much 
 * better than heap debugging done with:
 *
 * 1. Visual C++ 6.0 Runtime Heap Debugging:
 *    This only checks for heap corruption (i.e. only WRITINGS outside 
 *    the allocations) and even then, it only catches writings in the
 *    NO_MANS_LAND_SIZE (default:4 bytes) on either side of the blocks.
 *    The detection of these errors is done whenever the user (or the system)
 *    calls _CrtCheckMemory(). HeapCheck catches READ accesses as well, 
 *    at the EXACT place they are made, and in a much larger
 *    block (a page in i386 systems is 4096 bytes).
 *
 * 2. BoundsChecker:
 *    This is a very good debugging tool, capable of catching almost 
 *    every bug. However, in order to perform all these checks, a lot
 *    of CPU cycles are used, especially in instrumentation mode and 
 *    maximum memory checking (some programs won't work correctly when 
 *    they run so slowly, e.g. protocol stacks). HeapCheck catches only 
 *    heap-related errors, but it uses the paging hardware to do the 
 *    checking. The net result is that the HeapCheck debug versions of 
 *    programs run at almost the same speed as normal debug versions.
 *
 * I hope (but can't guarantee) that it will help you in your heap
 * bug hunting. It has definitely helped me with my projects.
 * Many thanks for the original idea go to the creator of UNIX's
 * Electric Fence, Bruce Perens. Oh, and make sure you use the 
 * correct (Win2000) versions of the debugging libraries. I used:
 *
 *    dbghelp.dll   : 5.0.2195.1
 *    imagehlp.dll  : 5.0.2195.1
 *
 * Maybe SymGetLineFromAddr works with other combinations, but I used
 * these successfully under both NT 4.0sp6 and Win2000.
 *
 * Happy coding!
 * Thanassis Tsiodras
 * ttsiod@softlab.ntua.gr
 *
 **************************************************************************
 */

/* 
 * Remember, all of this code gets called only in _DEBUG builds!
 * Check swaps.h to see why...
 */

#ifdef _DEBUG

#include <windows.h>
#include <crtdbg.h>
#include <imagehlp.h>

/* Include prototypes for HeapCheck functions */
#include "HeapCheckPrototypes.h"

/*
 * It is possible to write code that bypasses the allocation mechanisms of  
 * the library, and allocates from the standard VC-heap. An example inside 
 * the runtime library is the use of an enhanced 'new' operator in 
 * iostrini.cpp (line 21) and cerrinit.cpp (line 21) where _new_crt is used.
 * _new_crt maps through a #define to a 'new' operator that takes extra
 * parameters. HeapCheck can't catch this call, so when the time comes for 
 * the deletion of these blocks, the library's delete operator doesn't find 
 * them in it's tables. It is capable though to understand that these are
 * VC-heap blocks, through the use of _CrtIsValidHeapPointer.
 *
 * If you #define NO_VC_HEAP_ERRS, the library won't complain for such 
 * situations. This is the default, but if you use inside your code direct 
 * calls to _malloc_dbg, _calloc_dbg, etc, you should disable this.
 */

#define NO_VC_HEAP_ERRS

/*
 * Modify this for Alpha, PowerPC, etc. It is the page size used by
 * the virtual memory hardware. For Intel architectures, this is 4096
 * bytes. For others, place a breakpoint in the call to HeapCheckStartup's
 * GetSystemInfo, and read the 'dwPageSize' field of the 'si' variable.
 */

#define PAGE_SIZE			4096

/*
 * Total concurrent allocations possible. If your program needs more,
 * you'll get an assertion telling you to increase this.
 */

#define MAX_ALLOCATIONS			16384

/*
 * Total heap available to the application. If your program needs more,
 * you'll get an assertion telling you to increase this.
 */

#define MAX_ALLOCATABLE_BLOCK		8*1048576

/*
 * Max depth of call stack looked up when our functions are called
 */

#define MAX_STACK_DEPTH			30

/*
 * Define PRINT_NUMERIC_EIP to get stack traces that boldly go
 * where no imagehlp.dll has gone before... Hope you have SoftIce...
 */

//#define PRINT_NUMERIC_EIP

/*
 * Variables
 */

static DWORD	dwAllocationGranularity = 0;

static PCHAR	pMemoryBase;
static DWORD	dwTotalPages;
static DWORD	dwFreePages;

static struct tag_allocations {
    DWORD	isFree;
    PCHAR	pData;
    DWORD	dwPages;
    DWORD	dwLength;
    DWORD	EIPs[MAX_STACK_DEPTH];
} allocations[MAX_ALLOCATIONS];

static BYTE pagesState[MAX_ALLOCATABLE_BLOCK/PAGE_SIZE];

static DWORD dwTotalPeakMemory = 0, dwTotalAllocated = 0;

static CRITICAL_SECTION section;

/*
 * Function prototypes.
 */

/*
 * Weird hack, in order to support 5 and 6 argument passing to _CrtDbgReport.
 * Unforunately, with VC 5.0/6.0, RPT macros go up until _RPT4!
 */

#ifndef _RPT5
#define _RPT5(rptno, msg, arg1, arg2, arg3, arg4, arg5) \
    do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3, arg4, arg5))) \
	_CrtDbgBreak(); } while (0)
#endif

#ifndef _RPT6
#define _RPT6(rptno, msg, arg1, arg2, arg3, arg4, arg5, arg6) \
    do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3, arg4, arg5, arg6))) \
	_CrtDbgBreak(); } while (0)
#endif

static void HeapCheckStartup(void);
static void HeapCheckShutDown(void);

/*
 * Simple allocator of pages that never frees. 
 */

static void *
FindConsecutiveFreePagesSimple(DWORD dwPages)
{
    return pMemoryBase + PAGE_SIZE*(dwTotalPages - dwFreePages);
}

/*
 * Page allocator that searches for free pages.
 * Can be improved with better algorithms, but it's good enough for
 * most projects...
 */

static void *
FindConsecutiveFreePages(DWORD dwPages)
{	
    DWORD i;
    PVOID pv;
    
    for(i=0; i<sizeof(pagesState); i++) {
	if (!pagesState[i]) {
	    // Found a free page. Make sure there's enough free pages after it...			
	    
	    PVOID test;
	    
	    // If you land here after an assertion, stop debugging
	    // and increase MAX_ALLOCATABLE_BLOCK
	    _ASSERTE ((i + dwPages - 1) < sizeof(pagesState));
	    
	    // Search for used-up page in our searching range...
	    test = memchr(&pagesState[i], 1, dwPages);
	    
	    if (!test)	// We found enough pages.
		break;			
	}
    }
    
    // If you land here after an assertion 'Retry', stop debugging
    // and increase MAX_ALLOCATABLE_BLOCK
    _ASSERTE(i<sizeof(pagesState));
    
    // Set 1 to these pages's slots to reflect NON-FREE state
    FillMemory(&pagesState[i], dwPages, 1);
    
    pv = pMemoryBase + i*PAGE_SIZE;
    return pv;
}

/*
 * Utility function for VC-heap blocks.
 * Should be using binary search, but hey...I'm lazy.
 *
 * NOT TESTED YET - Who cares about VC-blocks?
 */

static DWORD 
GetSize(PVOID pData)
{
    // Search for block length....
    DWORD dwTest;
    
    for(dwTest=0; dwTest<MAX_ALLOCATABLE_BLOCK; dwTest++)
	if (_CrtIsMemoryBlock(pData, dwTest, NULL, NULL, NULL))
	    break;
	
    if (dwTest == MAX_ALLOCATABLE_BLOCK)
	return -1;
    else
	return dwTest;
}

static void FillCallStack(
    DWORD	EIPs[])
{
    DWORD LevelOneFrame;    
    DWORD dwFrame = 0;
    
    __asm {
	push eax	
	push ebx
	mov eax, [ebp]   // return to the frame of the caller
	mov LevelOneFrame, eax
	pop ebx
	pop eax
    }    

    while(1) {
	DWORD _eip;

	if (IsBadReadPtr((VOID*)LevelOneFrame, 8))
	    break;
	if (LevelOneFrame & 3)
	    break;

	_eip = ((PULONG) LevelOneFrame)[1];
	if (!_eip)
	    break;

	// If you end up in this assertion, stop debugging and re-compile
	// with an increased value for the MAX_STACK_DEPTH constant.
	_ASSERTE(dwFrame < MAX_STACK_DEPTH);

	EIPs[dwFrame++] = _eip;
	LevelOneFrame = ((PULONG) LevelOneFrame)[0];
    }
    EIPs[dwFrame] = 0;
}

static void PrintCaller()
{
    DWORD LevelOneFrame;
    IMAGEHLP_LINE dbgLine;
    DWORD dwTemp;
    HANDLE hProcess = GetCurrentProcess();
    __asm {
	push eax	
	push ebx
	mov eax, [ebp]   // return to the frame of the caller
	mov LevelOneFrame, eax
	pop ebx
	pop eax
    }    

    while(1) {
	DWORD _eip;

	if (IsBadReadPtr((VOID*)LevelOneFrame, 8))
	    break;
	if (LevelOneFrame & 3)
	    break;

	_eip = ((PULONG) LevelOneFrame)[1];
	if (!_eip)
	    break;

	ZeroMemory(&dbgLine, sizeof(IMAGEHLP_LINE));
	dbgLine.SizeOfStruct = sizeof(IMAGEHLP_LINE);
	if(!SymGetLineFromAddr(hProcess, _eip, &dwTemp, &dbgLine)) {
	    //_RPT1(_CRT_WARN, "Called from EIP = %x\n", _eip);
	    break;
	} else {
	    _RPT3(
		_CRT_WARN,	    
		"\tCalled from line %d(+%d bytes) of %s\n",	    
		dbgLine.LineNumber,
		dwTemp,
		dbgLine.FileName);
	}

	LevelOneFrame = ((PULONG) LevelOneFrame)[0];
    }
}

/*
 * Post-allocation fence versions of malloc, calloc, realloc and free 
 */
void *
HeapCheckPostFenceMalloc(
    size_t blockSize)
{
    DWORD dwSlot;
    DWORD dwPages;
    DWORD dwOldProtection;
    PCHAR data = NULL;
    BOOL bResult = FALSE;
    LPVOID lpvResult = NULL;
    static DWORD dwFirstTime = 1;

    if (dwFirstTime) {
	dwFirstTime = 0;
	HeapCheckStartup();
    }

    if (!blockSize)
	return NULL;
    
    EnterCriticalSection(&section);

    for(dwSlot = 0; dwSlot<MAX_ALLOCATIONS; dwSlot++)
	if (allocations[dwSlot].isFree) {
	    allocations[dwSlot].isFree = 0;
	    break;
	}
	
    // If you end up in this assertion, stop debugging and re-compile
    // with an increased value for the MAX_ALLOCATIONS constant.
    _ASSERTE(dwSlot != MAX_ALLOCATIONS);
    
    // Calculate number of requires pages
    dwPages = ((blockSize - 1) / PAGE_SIZE) + 2;
    
    //_RPT2(_CRT_WARN, "Requested %7d bytes, granted %2d pages\n", blockSize, dwPages);
    
    // Make sure we have enough room
    if (dwFreePages < dwPages) {
	_RPT1(
	    _CRT_WARN, 
	    "Your application requires more free memory...\n"
	    "Increase MAX_ALLOCATABLE_BLOCK in %s.", __FILE__);
	_ASSERTE(dwFreePages >= dwPages);
    }
    
    data = (char*)FindConsecutiveFreePages(dwPages);
    
    // OK, now make data-pages available...
    lpvResult = VirtualAlloc(
	(LPVOID) data,
	(dwPages-1)*PAGE_SIZE,
	MEM_COMMIT,
	PAGE_READWRITE);
    _ASSERTE(lpvResult != NULL );
    
    VirtualProtect(
	(LPVOID) data,
	(dwPages-1)*PAGE_SIZE,
	PAGE_READWRITE,
	&dwOldProtection);
    
    // and fence-page untouchable!
    bResult = VirtualFree( 
	(LPVOID) (data + (dwPages-1)*PAGE_SIZE),
	PAGE_SIZE,
	MEM_DECOMMIT);
    _ASSERTE(bResult == TRUE);
       
    data += (PAGE_SIZE - (blockSize % PAGE_SIZE)) % PAGE_SIZE;
    
    dwFreePages -= dwPages;
    
    allocations[dwSlot].pData = data;
    allocations[dwSlot].dwPages = dwPages;
    allocations[dwSlot].dwLength = blockSize;
    
    FillCallStack(allocations[dwSlot].EIPs);
    
    // Cause problems to users who think malloc zeroes block memory...
    FillMemory(data, blockSize, 0xCD);
    
    dwTotalAllocated += blockSize;
    if (dwTotalPeakMemory<dwTotalAllocated)
	dwTotalPeakMemory = dwTotalAllocated;

    LeaveCriticalSection(&section);

    return (void *)data;
}

void *
HeapCheckPostFenceCalloc(
    size_t blockSize)
{
    PVOID *data = (PVOID*) HeapCheckPostFenceMalloc(blockSize);
    if (data)
	FillMemory(data, blockSize, 0);
    return data;
}


void *
HeapCheckPostFenceRealloc(
    void *ptr, 
    size_t size)
{
    void *tmpData;
    DWORD dw;
    
    EnterCriticalSection(&section);

    for(dw=0; dw<MAX_ALLOCATIONS; dw++)
	if(!allocations[dw].isFree)
	    if(allocations[dw].pData == ptr)
		break;

    if(dw == MAX_ALLOCATIONS) {
	_RPT0(
	    _CRT_WARN, 
	    "### ERROR ### Attempt to realloc unallocated block!...\n");
	PrintCaller();
	_ASSERTE(dw != MAX_ALLOCATIONS);
    }

    LeaveCriticalSection(&section);
    
    tmpData = HeapCheckPostFenceMalloc(size);
    if (!tmpData)
	return NULL;
    
    if (size < allocations[dw].dwLength)
	CopyMemory(tmpData, allocations[dw].pData, size);
    else
	CopyMemory(tmpData, allocations[dw].pData, allocations[dw].dwLength);
    
    HeapCheckPostFenceFree(ptr);
    
    return tmpData;
}

void
HeapCheckPostFenceFree(
    void *pData)
{
    PCHAR pTmp = (PCHAR) pData;
    DWORD dw;

    EnterCriticalSection(&section);

    for(dw=0; dw<MAX_ALLOCATIONS; dw++)
	if(!allocations[dw].isFree)
	    if(allocations[dw].pData == pData)
		break;
	    
    if(dw == MAX_ALLOCATIONS) {
	// This is a block not allocated from us...
	// Check first to see if it was allocated from
	// the normal VC heap through any direct calls
	// (brain-dead coding, that is)
	
	if (_CrtIsValidHeapPointer(pData)) {			
#ifndef NO_VC_HEAP_ERRS
	    char *origFileName;
	    DWORD origLineNo, origBlockSize;
	    
	    if (_CrtIsMemoryBlock(
		pData, 
		origBlockSize = GetSize(pData), 
		NULL,
		&origFileName, 
		&origLineNo))
	    {
		_RPT6(
		    _CRT_WARN, 
		    "Freeing VC-heap allocated block (%d bytes from line %d of %s)\n",		    
		    origBlockSize, origLineNo, origFileName);
		PrintCaller();
	    }
	    else {
		_RPT0(
		    _CRT_WARN, 
		    "Freeing unknown VC-heap allocated block\n");
		PrintCaller();
		_CrtDbgBreak();
	    }
#endif
	}
	else {
	    _RPT0(
		_CRT_WARN, 
		"### ERROR ### Attempt to free unallocated block!... \n");
	    PrintCaller();
	    _CrtDbgBreak();
	}
	LeaveCriticalSection(&section);
	return;
    }
    
    // Make data pages inaccessible, since they are freed!
    pTmp -= (((DWORD)pTmp) % PAGE_SIZE);
    VirtualFree(
	pTmp,
	(allocations[dw].dwPages - 1)*PAGE_SIZE,
	MEM_DECOMMIT);
    
    // Set these pages to 'available' again.
    ZeroMemory(
	&pagesState[ (pTmp - pMemoryBase)/PAGE_SIZE ],
	allocations[dw].dwPages);
    dwFreePages += allocations[dw].dwPages;
    
    dwTotalAllocated -= allocations[dw].dwLength;
    
    allocations[dw].isFree = 1;    
    allocations[dw].pData = NULL;
    allocations[dw].dwPages = 0;
    allocations[dw].dwLength = 0;
    allocations[dw].EIPs[0] = 0;

    LeaveCriticalSection(&section);
}

/*
 * Pre-allocation fence versions of malloc, calloc, realloc and free
 *
 */

void *
HeapCheckPreFenceMalloc(
    size_t blockSize)
{
    DWORD dwSlot;
    DWORD dwPages, dwOldProtection;
    PCHAR data = NULL;
    BOOL bResult = FALSE;
    LPVOID lpvResult = NULL;
    static DWORD dwFirstTime = 1;
    
    if (dwFirstTime) {
	dwFirstTime = 0;
	HeapCheckStartup();
    }
    
    if (!blockSize)
	return NULL;
    
    EnterCriticalSection(&section);

    for(dwSlot = 0; dwSlot<MAX_ALLOCATIONS; dwSlot++)
	if (allocations[dwSlot].isFree) {
	    allocations[dwSlot].isFree = 0;
	    break;
	}
	
    // If you end up in this assertion, stop debugging and re-compile
    // with an increased value for the MAX_ALLOCATIONS constant.
    _ASSERTE(dwSlot != MAX_ALLOCATIONS);
    
    // Calculate number of requires pages
    dwPages = ((blockSize - 1) / PAGE_SIZE) + 2;
    
    //_RPT2(_CRT_WARN, "Requested %7d bytes, granted %2d pages\n", blockSize, dwPages);
    
    // Make sure we have enough room
    if (dwFreePages < dwPages) {
	_RPT1(
	    _CRT_WARN, 
	    "Your application requires more free memory...\n"
	    "Change MAX_ALLOCATABLE_BLOCK in %s.", __FILE__);
	_ASSERTE(dwFreePages >= dwPages);
    }
    
    data = (char*) FindConsecutiveFreePages(dwPages);
    
    // OK, now make fence-page untouchable...
    bResult = VirtualFree(
	(LPVOID) data,
	PAGE_SIZE,
	MEM_DECOMMIT);
    _ASSERTE(bResult == TRUE);
       
    // and data-pages available!
    lpvResult = VirtualAlloc( 
	(LPVOID) (data+PAGE_SIZE),
	(dwPages-1)*PAGE_SIZE,
	MEM_COMMIT,
	PAGE_READWRITE);
    _ASSERTE(lpvResult != NULL );		
    
    VirtualProtect(
	(LPVOID) (data+PAGE_SIZE),
	(dwPages-1)*PAGE_SIZE,
	PAGE_READWRITE,
	&dwOldProtection);
    
    data += PAGE_SIZE;
    
    dwFreePages -= dwPages;
    
    allocations[dwSlot].pData = data;
    allocations[dwSlot].dwPages = dwPages;
    allocations[dwSlot].dwLength = blockSize;
    
    FillCallStack(allocations[dwSlot].EIPs);
    
    // Cause problems to users who think malloc zeroes block memory...
    FillMemory(data, blockSize, 0xCD);
    
    dwTotalAllocated += blockSize;
    if (dwTotalPeakMemory<dwTotalAllocated)
	dwTotalPeakMemory = dwTotalAllocated;

    LeaveCriticalSection(&section);

    return (void *)data;
}

void *
HeapCheckPreFenceCalloc(
    size_t blockSize)
{
    PVOID *data = (PVOID*) HeapCheckPreFenceMalloc(blockSize);
    if (data)
	FillMemory(data, blockSize, 0);
    return data;
}


void *
HeapCheckPreFenceRealloc(
    void *ptr, 
    size_t size)
{
    void *tmpData;
    DWORD dw;

    EnterCriticalSection(&section);
    
    for(dw=0; dw<MAX_ALLOCATIONS; dw++)
	if(!allocations[dw].isFree)
	    if(allocations[dw].pData == ptr)
		break;
    if(dw == MAX_ALLOCATIONS) {
	_RPT0(
	    _CRT_WARN, 
	    "### ERROR ### Attempt to realloc unallocated block!...\n");
	PrintCaller();
	_ASSERTE(dw != MAX_ALLOCATIONS);
    }

    LeaveCriticalSection(&section);
    
    tmpData = HeapCheckPreFenceMalloc(size);
    if (!tmpData)
	return NULL;
    
    if (size < allocations[dw].dwLength)
	CopyMemory(tmpData, allocations[dw].pData, size);
    else
	CopyMemory(tmpData, allocations[dw].pData, allocations[dw].dwLength);
    
    HeapCheckPreFenceFree(ptr);
    
    return tmpData;
}

void
HeapCheckPreFenceFree(
    void *pData)
{
    PCHAR pTmp;
    DWORD dw;

    EnterCriticalSection(&section);
    
    for(dw=0; dw<MAX_ALLOCATIONS; dw++)
	if(!allocations[dw].isFree)
	    if(allocations[dw].pData == pData)
		break;
	    
    if(dw == MAX_ALLOCATIONS) {
	// This is a block not allocated from us...
	// Check first to see if it was allocated from
	// the normal VC heap through any direct calls
	// (brain-dead coding, that is)
	
	if (_CrtIsValidHeapPointer(pData)) {
#ifndef NO_VC_HEAP_ERRS
	    char *origFileName;
	    DWORD origLineNo, origBlockSize;
	    
	    if (_CrtIsMemoryBlock(
		pData, 
		origBlockSize = GetSize(pData), 
		NULL, 
		&origFileName, 
		&origLineNo))
	    {
		_RPT6(
		    _CRT_WARN, 
		    "Freeing VC-heap allocated block (%d bytes from line %d of %s)\n",		    
		    origBlockSize, origLineNo, origFileName);
		PrintCaller();
	    }
	    else {
		_RPT0(
		    _CRT_WARN, 
		    "Freeing unknown VC-heap allocated block\n");		    
		PrintCaller();
		_CrtDbgBreak();
	    }
#endif
	}
	else {
	    _RPT0(
		_CRT_WARN, 
		"### ERROR ### Attempt to free unallocated block!... \n");
	    PrintCaller();
	    _CrtDbgBreak();
	}
	LeaveCriticalSection(&section);

	return;
    }
    
    // Make data pages inaccessible, since they are freed!	
    VirtualFree(
	pData,
	(allocations[dw].dwPages - 1)*PAGE_SIZE,
	MEM_DECOMMIT);
    
    // Set these pages to 'available' again.
    pTmp = (PCHAR) pData;
    pTmp -= PAGE_SIZE;
    ZeroMemory(
	&pagesState[ (pTmp - pMemoryBase)/PAGE_SIZE ],
	allocations[dw].dwPages);
    dwFreePages += allocations[dw].dwPages;
    
    dwTotalAllocated -= allocations[dw].dwLength;
    
    allocations[dw].isFree = 1;    
    allocations[dw].pData = NULL;
    allocations[dw].dwPages = 0;
    allocations[dw].dwLength = 0;
    allocations[dw].EIPs[0] = 0;

    LeaveCriticalSection(&section);
}


/*
 * Startup and shutdown functions
 */

static void
HeapCheckStartup()
{	
    SYSTEM_INFO si;
    DWORD dw;
    CHAR dbgPath[MAX_PATH], *pEnd;

    SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES );

    // Try to read the symbols from the path of the .exe file
    if (!GetModuleFileName(	
	NULL,
	dbgPath,
	sizeof(dbgPath))) {
	
	// if we failed, do your best
	if(!SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
	    _RPT0(
		_CRT_WARN,
		"HEAPCHECK: Won't be able to read file/line information...:(\n");
	}
    } else {
	pEnd = strrchr(dbgPath, '\\');
	if (!pEnd) {
	    // if we failed, do your best
	    if(!SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
		_RPT0(
		    _CRT_WARN,
		    "HEAPCHECK: Won't be able to read file/line information...:(\n");
	    }
	} else {
	    // 99% probability of success with file/line info...!
	    *pEnd = 0;
	    if(!SymInitialize(GetCurrentProcess(), dbgPath, TRUE)) {
		_RPT0(
		    _CRT_WARN,
		    "HEAPCHECK: Won't be able to read file/line information...:(\n");
	    }
	}
    }

    InitializeCriticalSection(&section);

    GetSystemInfo(&si);
    
    _ASSERTE(si.dwPageSize == PAGE_SIZE);
    dwAllocationGranularity = si.dwAllocationGranularity;
    
    _ASSERTE(dwAllocationGranularity <= MAX_ALLOCATABLE_BLOCK);
    
    pMemoryBase = (PCHAR) VirtualAlloc(
	NULL,					// Place memory base wherever
	MAX_ALLOCATABLE_BLOCK,	// Total heap memory available
	MEM_RESERVE,			// For now, just reserve it
	PAGE_NOACCESS);			// and make it no-touch.
    
    _ASSERTE(pMemoryBase != NULL);
    
    dwTotalPages = MAX_ALLOCATABLE_BLOCK / PAGE_SIZE;
    dwFreePages = dwTotalPages;
    
    if(atexit(HeapCheckShutDown)) {
	_RPT0(_CRT_WARN, "### WARNING ### Can't check for memory leaks automatically!\n");
	_RPT0(_CRT_WARN, "### WARNING ### Call HeapCheckShutDown at the end of your app,\n");
    }
    
    for(dw=0; dw<MAX_ALLOCATIONS; dw++)
	allocations[dw].isFree = 1;
    
    ZeroMemory(pagesState, sizeof(pagesState));
}

static void
HeapCheckShutDown()
{
    BOOL bSuccess;
    DWORD dw;
    HANDLE hProcess = GetCurrentProcess();

    EnterCriticalSection(&section);

    _RPT0(_CRT_WARN, "\n##################################\n");
    _RPT0(_CRT_WARN,   "#######  HeapCheck report   ######\n");
    _RPT0(_CRT_WARN,   "##################################\n\n");

    for(dw=0; dw<MAX_ALLOCATIONS; dw++) {
	if (!allocations[dw].isFree) {
	    
	    // We have to use IMAGEHLP.DLL. God help us...						
	    IMAGEHLP_LINE dbgLine;
	    DWORD dwTemp, dwCodePlaces = 0;

	    _RPT1(
		_CRT_WARN,
		"### WARNING ### Memory leak (%d bytes) found... Allocated:\n",
		allocations[dw].dwLength);

	    while(allocations[dw].EIPs[dwCodePlaces]) {
		    
		ZeroMemory(&dbgLine, sizeof(IMAGEHLP_LINE));
		dbgLine.SizeOfStruct = sizeof(IMAGEHLP_LINE);
		if(!SymGetLineFromAddr(
		    hProcess,
		    allocations[dw].EIPs[dwCodePlaces],
		    &dwTemp,
		    &dbgLine))
		{
#ifdef PRINT_NUMERIC_EIP
		    _RPT1(
			_CRT_WARN,
			"\tfrom EIP = %x\n",
			allocations[dw].EIPs[dwCodePlaces]);
#endif
		} else {
		    _RPT3(
			_CRT_WARN,
			"\tfrom line %d(+%d bytes) of %s\n",
			dbgLine.LineNumber,
			dwTemp,
			dbgLine.FileName);
		}
		dwCodePlaces++;

		// If you land here after an assertion, stop debugging
		// and increase MAX_STACK_DEPTH
		_ASSERTE(dwCodePlaces < MAX_STACK_DEPTH);
	    }
	}
    }
    
    /* Decommit the entire block. */ 
    bSuccess = VirtualFree( 
        pMemoryBase,			/* base address of block    */ 
        MAX_ALLOCATABLE_BLOCK,	/* bytes of committed pages */ 
        MEM_DECOMMIT);			/* decommit the pages       */ 
    _ASSERTE(bSuccess);
    
    /* Release the entire block. */ 
    if (bSuccess) 
	bSuccess = VirtualFree( 
        pMemoryBase,    /* base address of block     */ 
        0,              /* releases the entire block */ 
        MEM_RELEASE);   /* releases the pages        */ 
    _ASSERTE(bSuccess);
    
    _RPT1(_CRT_WARN, "HeapCheck Statistics:\n\tMaximum memory allocated = %d\n\n", dwTotalPeakMemory);    

    SymCleanup(hProcess);

    LeaveCriticalSection(&section);
    DeleteCriticalSection(&section);
}

#endif
