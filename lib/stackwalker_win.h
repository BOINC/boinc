// $Id$
//
/*////////////////////////////////////////////////////////////////////////////
 *  Project:
 *    Memory_and_Exception_Trace
 *
 * ///////////////////////////////////////////////////////////////////////////
 *  File:
 *    Stackwalker.h
 *
 *  Remarks:
 *
 *
 *  Note:
 *
 *
 *  Author:
 *    Jochen Kalmbach
 *
 *////////////////////////////////////////////////////////////////////////////

#ifndef __STACKWALKER_H__
#define __STACKWALKER_H__

#include "boinc_win.h"

// Only valid in the following environment: Intel platform, MS VC++ 5/6/7
#ifndef _X86_
#error Only INTEL envirnoments are supported!
#endif

// Only MS VC++ 5 to 8
#if (_MSC_VER < 1100) || (_MSC_VER > 1400)
#error Only MS VC++ 5/6/7/8 supported. Check if the '_CrtMemBlockHeader' has not changed with this compiler!
#endif

typedef enum eAllocCheckOutput
{
  ACOutput_Simple,
  ACOutput_Advanced,
  ACOutput_XML
};

// Make extern "C", so it will also work with normal C-Programs
#ifdef __cplusplus
extern "C" {
#endif

extern int InitStackWalk();
extern DWORD StackwalkFilter( EXCEPTION_POINTERS *ep, DWORD status, LPCTSTR pszLogFile);
extern void StackwalkThread( HANDLE hThread, CONTEXT* c, LPCTSTR pszLogFile);

#ifdef __cplusplus
}
#endif

#endif  // __STACKWALKER_H__
