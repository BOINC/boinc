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

// Only valid in the following environment: Intel platform, MS VC++ 5/6/7/8
#ifndef _X86_
#error Only INTEL envirnoments are supported!
#endif

// Make extern "C", so it will also work with normal C-Programs
#ifdef __cplusplus
extern "C" {
#endif

extern int DebuggerInitialize( LPCSTR pszBOINCLocation, LPCSTR pszSymbolStore );
extern int DebuggerDisplayDiagnostics();
extern DWORD StackwalkFilter( EXCEPTION_POINTERS* ep, DWORD status );
extern void StackwalkThread( HANDLE hThread, CONTEXT* c );

#ifdef __cplusplus
}
#endif

#endif  // __STACKWALKER_H__
