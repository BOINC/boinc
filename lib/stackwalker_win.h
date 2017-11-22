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

#ifndef BOINC_STACKWALKER_WIN_H
#define BOINC_STACKWALKER_WIN_H

// Make extern "C", so it will also work with normal C-Programs
#ifdef __cplusplus
extern "C" {
#endif

extern int DebuggerInitialize( LPCSTR pszBOINCLocation, LPCSTR pszSymbolStore, BOOL bProxyEnabled, LPCSTR pszProxyServer );
extern int DebuggerDisplayDiagnostics();
extern DWORD StackwalkFilter( EXCEPTION_POINTERS* ep, DWORD status );
extern void StackwalkThread( HANDLE hThread, CONTEXT* c );

#ifdef __cplusplus
}
#endif

#endif
