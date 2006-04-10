// $Id$
//

/*////////////////////////////////////////////////////////////////////////////
 *  Project:
 *    Memory_and_Exception_Trace
 *
 * ///////////////////////////////////////////////////////////////////////////
 *	File:
 *		Stackwalker.cpp
 *
 *	Remarks:
 *    Dumps the stack of an thread if an exepction occurs
 *
 *	Author:
 *		Jochen Kalmbach, Germany
 *    (c) 2002-2003 (Freeware)
 *    http://www.codeproject.com/tools/leakfinder.asp
 *
 * License (The zlib/libpng License, http://www.opensource.org/licenses/zlib-license.php):
 *
 * Copyright (c) 2003 Jochen Kalmbach
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including
 * commercial applications, and to alter it and redistribute it freely, subject to
 * the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 *    that you wrote the original software. If you use this software in a product,
 *    an acknowledgment in the product documentation would be appreciated but is
 *    not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *//////////////////////////////////////////////////////////////////////////////


#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif


#include "version.h"
#include "dbghelp.h"
#include "stackwalker_win.h"


#define gle (GetLastError())
#define lenof(a) (sizeof(a) / sizeof((a)[0]))
#define IMGSYMLEN ( sizeof IMAGEHLP_SYMBOL )
#define TTBUFLEN 8096 // for a temp buffer (2^13)


// ImagehlpApiVersion()
typedef LPAPI_VERSION (__stdcall WINAPI *tIAV)(
    VOID
);
tIAV pIAV = NULL;

// SymCleanup()
typedef BOOL (__stdcall *tSC)(
    IN HANDLE hProcess
);
tSC pSC = NULL;

// SymEnumerateModules64()
typedef BOOL (__stdcall WINAPI *tSEM)(
    IN HANDLE hProcess,
    IN PSYM_ENUMMODULES_CALLBACK64 EnumModulesCallback,
    IN PVOID UserContext
);
tSEM pSEM = NULL;

// SymFunctionTableAccess64()
typedef PVOID (__stdcall *tSFTA)( 
    IN HANDLE hProcess,
    IN DWORD64 AddrBase
);
tSFTA pSFTA = NULL;

// SymGetLineFromAddr64()
typedef BOOL (__stdcall *tSGLFA)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr,
    OUT PDWORD pdwDisplacement,
    OUT PIMAGEHLP_LINE64 Line
);
tSGLFA pSGLFA = NULL;

// SymGetModuleBase64()
typedef DWORD64 (__stdcall *tSGMB)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr
);
tSGMB pSGMB = NULL;

// SymGetModuleInfo64()
typedef BOOL (__stdcall *tSGMI)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr,
    OUT PIMAGEHLP_MODULE64 ModuleInfo );
tSGMI pSGMI = NULL;

// SymGetOptions()
typedef DWORD (__stdcall *tSGO)(
    VOID
);
tSGO pSGO = NULL;

// SymGetSearchPath()
typedef BOOL (__stdcall *tSGSP)(
    IN HANDLE hProcess,
    OUT PTSTR SearchPath,
    IN DWORD SearchPathLength
);
tSGSP pSGSP = NULL;

// SymFromAddr()
typedef BOOL (__stdcall *tSFA)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr,
    OUT PDWORD64 pdwDisplacement,
    OUT PSYMBOL_INFO Symbol
);
tSFA pSFA = NULL;

// SymInitialize()
typedef BOOL (__stdcall *tSI)(
    IN HANDLE hProcess,
    IN PCTSTR UserSearchPath,
    IN BOOL fInvadeProcess
);
tSI pSI = NULL;

// SymLoadModuleEx()
typedef DWORD64 (__stdcall *tSLM)(
    IN HANDLE hProcess,
    IN HANDLE hFile,
    IN PCSTR ImageName,
    IN PCSTR ModuleName,
    IN DWORD64 BaseOfDll,
    IN DWORD SizeOfDll
);
tSLM pSLM = NULL;

// SymRegisterCallback64()
typedef BOOL (__stdcall *tSRC)(
    IN HANDLE hProcess,
    PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction,
    ULONG64 UserContext
);
tSRC pSRC = NULL;

// SymSetOptions()
typedef DWORD (__stdcall *tSSO)(
    IN DWORD SymOptions
);
tSSO pSSO = NULL;

// StackWalk()
typedef BOOL (__stdcall *tSW)(
    DWORD MachineType,
    HANDLE hProcess,
    HANDLE hThread,
    LPSTACKFRAME64 StackFrame,
    PVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
);
tSW pSW = NULL;

// UnDecorateSymbolName()
typedef DWORD (__stdcall WINAPI *tUDSN)(
    PCSTR DecoratedName,
    PSTR UnDecoratedName,
    DWORD UndecoratedLength,
    DWORD Flags
);
tUDSN pUDSN = NULL;

#ifndef SYMOPT_NO_PROMPTS
#define SYMOPT_NO_PROMPTS               0x00080000
#endif

// Forward definitions of functions:
static void ShowStackRM( HANDLE hThread, CONTEXT& c, HANDLE hProcess);

// Global data:
static BOOL g_bInitialized = FALSE;
static DWORD g_dwShowCount = 0;  // increase at every ShowStack-Call
static HINSTANCE g_hDbgHelpDll = NULL;
static HINSTANCE g_hSymSrvDll = NULL;
static HINSTANCE g_hSrcSrvDll = NULL;
static CRITICAL_SECTION g_csFileOpenClose = {0};


// ##########################################################################################
// ##########################################################################################
// ##########################################################################################
// ##########################################################################################


// Write Date/Time to specified file (will also work after 2038)
void DebuggerWriteDateTime() {
    TCHAR pszTemp[11], pszTemp2[11];

    _tstrdate( pszTemp );
    _tstrtime( pszTemp2 );

    _ftprintf(stderr, _T("%s %s"), pszTemp, pszTemp2 );
}


bool DebuggerLoadLibrary( 
    HINSTANCE* lphInstance, const std::string strBOINCLocation, const std::string strLibrary
)
{
    std::string strTargetLibrary;

    if (strBOINCLocation.empty()) {
        strTargetLibrary = strLibrary;
    } else {
        strTargetLibrary = strBOINCLocation + "\\" + strLibrary;
    }

    *lphInstance = LoadLibraryA( strTargetLibrary.c_str() );
    if ( *lphInstance == NULL )
    {
        strTargetLibrary = strLibrary;
        *lphInstance = LoadLibraryA( strTargetLibrary.c_str() );
        if ( *lphInstance == NULL )
        {
            _ftprintf( stderr, "LoadLibraryA( %s ): GetLastError = %lu\n", strLibrary.c_str(), gle );
            return false;
        }
    }
    return true;
}


BOOL CALLBACK SymRegisterCallbackProc(HANDLE hProcess, ULONG ActionCode, ULONG64 CallbackData, ULONG64 UserContext)
{
    BOOL bRetVal = FALSE;
    PIMAGEHLP_DEFERRED_SYMBOL_LOAD64 pModule;

    switch(ActionCode) {
        case CBA_DEBUG_INFO:
            _ftprintf(stderr, _T("DEBUG: %s\n"), (PCTSTR)CallbackData);
            bRetVal = TRUE;
            break;
        case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:
            pModule = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD64)CallbackData;
            _ftprintf(stderr, _T("ModLoad: "));
            _ftprintf(stderr, _T("%.8x "), pModule->BaseOfImage);
            _ftprintf(stderr, _T("%s "), pModule->FileName);
            _ftprintf(stderr, _T("\n"));
            bRetVal = TRUE;
            break;
    }

    fflush(stderr);
    return bRetVal;
}


BOOL CALLBACK SymEnumerateModulesProc(LPSTR ModuleName, DWORD64 BaseOfDll, PVOID UserContext)
{
    IMAGEHLP_MODULE64 Module;
    char type[MAX_SYM_NAME];

    memset( &Module, '\0', sizeof(IMAGEHLP_MODULE64) );
    Module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    memset( &type, '\0', sizeof type );

    if ( !pSGMI( GetCurrentProcess(), BaseOfDll, &Module ) )
    {
        _ftprintf(stderr, _T("SymGetModuleInfo(): GetLastError = %lu\n"), gle );
    }
    else
    { 
        switch ( Module.SymType )
        {
            case SymNone:
                strcpy( type, "-nosymbols-" );
                break;
            case SymCoff:
                strcpy( type, "COFF" );
                break;
            case SymCv:
                strcpy( type, "CV" );
                break;
            case SymPdb:
                strcpy( type, "PDB" );
                break;
            case SymExport:
                strcpy( type, "-exported-" );
                break;
            case SymDeferred:
                strcpy( type, "-deferred-" );
                break;
            case SymSym:
                strcpy( type, "SYM" );
                break;
            default:
                _snprintf( type, sizeof type, "symtype=%ld", (long) Module.SymType );
                break;
        }
    }

    _ftprintf(stderr, _T("ModLoad: "));
    _ftprintf(stderr, _T("%.8x ")              , Module.BaseOfImage);
    _ftprintf(stderr, _T("%.8x ")              , Module.ImageSize);
    _ftprintf(stderr, _T("%s ")                , Module.LoadedImageName);
    _ftprintf(stderr, _T("(%s Symbols Loaded)"), type);
    _ftprintf(stderr, _T("\n"));
    fflush(stderr);

    return TRUE;
}



int DebuggerInitialize( LPCSTR pszBOINCLocation, LPCSTR pszSymbolStore )
{
    using namespace std;

    if (g_bInitialized != FALSE)
        return 0;

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((OSVERSIONINFO*)&osvi);

    // If Win9x use the old dbghelp.dll (5.0.2195.1)
    if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId)
    {
        if (!DebuggerLoadLibrary(&g_hDbgHelpDll, pszBOINCLocation, "dbghelp95.dll")) {
            if (!DebuggerLoadLibrary(&g_hDbgHelpDll, pszBOINCLocation, "dbghelp.dll")) {
                g_bInitialized = FALSE;
                return 1;
            }
        }
    }
    else
    {
        if (!DebuggerLoadLibrary(&g_hDbgHelpDll, pszBOINCLocation, "dbghelp.dll")) {
            g_bInitialized = FALSE;
            return 1;
        }
        DebuggerLoadLibrary(&g_hSymSrvDll, pszBOINCLocation, "symsrv.dll");
        DebuggerLoadLibrary(&g_hSrcSrvDll, pszBOINCLocation, "srcsrv.dll");
    }

    pIAV = (tIAV) GetProcAddress( g_hDbgHelpDll, "ImagehlpApiVersion" );
    pSC = (tSC) GetProcAddress( g_hDbgHelpDll, "SymCleanup" );
    pSEM = (tSEM) GetProcAddress( g_hDbgHelpDll, "SymEnumerateModules64" );
    pSFTA = (tSFTA) GetProcAddress( g_hDbgHelpDll, "SymFunctionTableAccess64" );
    pSGLFA = (tSGLFA) GetProcAddress( g_hDbgHelpDll, "SymGetLineFromAddr64" );
    pSGMB = (tSGMB) GetProcAddress( g_hDbgHelpDll, "SymGetModuleBase64" );
    pSGMI = (tSGMI) GetProcAddress( g_hDbgHelpDll, "SymGetModuleInfo64" );
    pSGO = (tSGO) GetProcAddress( g_hDbgHelpDll, "SymGetOptions" );
    pSGSP = (tSGSP) GetProcAddress( g_hDbgHelpDll, "SymGetSearchPath" );
    pSFA = (tSFA) GetProcAddress( g_hDbgHelpDll, "SymFromAddr" );
    pSI = (tSI) GetProcAddress( g_hDbgHelpDll, "SymInitialize" );
    pSRC = (tSRC) GetProcAddress( g_hDbgHelpDll, "SymRegisterCallback64" );
    pSSO = (tSSO) GetProcAddress( g_hDbgHelpDll, "SymSetOptions" );
    pSW = (tSW) GetProcAddress( g_hDbgHelpDll, "StackWalk64" );
    pUDSN = (tUDSN) GetProcAddress( g_hDbgHelpDll, "UnDecorateSymbolName" );
    pSLM = (tSLM) GetProcAddress( g_hDbgHelpDll, "SymLoadModuleEx" );

    if ( pIAV == NULL || pSC == NULL || pSEM == NULL || pSFTA == NULL ||
         pSGMB == NULL || pSGMI == NULL || pSGO == NULL || pSFA == NULL ||
         pSI == NULL || pSRC == NULL || pSSO == NULL || pSW == NULL ||
         pUDSN == NULL || pSLM == NULL )
    {
        _ftprintf( stderr, "GetProcAddress(): some required function not found.\n" );
        FreeLibrary( g_hDbgHelpDll );
        g_bInitialized = FALSE;
        return 1;
    }

    g_bInitialized = TRUE;

    InitializeCriticalSection(&g_csFileOpenClose);
    EnterCriticalSection(&g_csFileOpenClose);

    CHAR* tt;
    CHAR* p;
    DWORD symOptions; // symbol handler settings
    string strLocalSymbolStore;
    string strSymbolSearchPath;

    static const basic_string<char>::size_type npos = -1;


    // NOTE: normally, the exe directory and the current directory should be taken
    // from the target process. The current dir would be gotten through injection
    // of a remote thread; the exe fir through either ToolHelp32 or PSAPI.

    tt = (CHAR*) malloc(sizeof(CHAR) * TTBUFLEN); // Get the temporary buffer
    if (!tt) return 1;  // not enough memory...

    // build symbol search path from:
    strLocalSymbolStore = "";
    strSymbolSearchPath = "";

    // current directory
    if ( GetCurrentDirectoryA( TTBUFLEN, tt ) )
        strSymbolSearchPath += tt + string( ";" );

    // dir with executable
    if ( GetModuleFileNameA( 0, tt, TTBUFLEN ) )
    {
        for ( p = tt + strlen( tt ) - 1; p >= tt; -- p )
        {
            // locate the rightmost path separator
            if ( *p == '\\' || *p == '/' || *p == ':' )
                break;
        }
        // if we found one, p is pointing at it; if not, tt only contains
        // an exe name (no path), and p points before its first byte
        if ( p != tt ) // path sep found?
        {
            if ( *p == ':' ) // we leave colons in place
                ++p;
            *p = '\0'; // eliminate the exe name and last path sep
            strSymbolSearchPath += tt + string( ";" );
        }
    }

    // environment variable _NT_SYMBOL_PATH
    if ( GetEnvironmentVariableA( "_NT_SYMBOL_PATH", tt, TTBUFLEN ) )
        strSymbolSearchPath += tt + string( ";" );
    // environment variable _NT_ALTERNATE_SYMBOL_PATH
    if ( GetEnvironmentVariableA( "_NT_ALTERNATE_SYMBOL_PATH", tt, TTBUFLEN ) )
        strSymbolSearchPath += tt + string( ";" );

    if ( GetTempPathA( TTBUFLEN, tt ) )
        strLocalSymbolStore += tt + string("symbols");

    // microsoft public symbol server
    if (npos == strSymbolSearchPath.find("http://msdl.microsoft.com/download/symbols")) {
        strSymbolSearchPath += 
            string( "srv*" ) + strLocalSymbolStore + 
            string( "*http://msdl.microsoft.com/download/symbols;" );
    }

    // project symbol server
    if ((npos == strSymbolSearchPath.find(pszSymbolStore)) && (0 < strlen(pszSymbolStore))) {
        strSymbolSearchPath += 
            string( "srv*" ) + strLocalSymbolStore + string( "*" ) +
            string( pszSymbolStore );
    }

    // boinc symbol server
    if (npos == strSymbolSearchPath.find("http://boinc.berkeley.edu/symstore")) {
        strSymbolSearchPath += 
            string( "srv*" ) + strLocalSymbolStore + 
            string( "*http://boinc.berkeley.edu/symstore;" );
    }

    if ( strSymbolSearchPath.size() > 0 ) // if we added anything, we have a trailing semicolon
        strSymbolSearchPath = strSymbolSearchPath.substr( 0, strSymbolSearchPath.size() - 1 );

    // init symbol handler stuff (SymInitialize())
    if ( ! pSI(GetCurrentProcess(), strSymbolSearchPath.c_str(), TRUE ) )
    {
        _ftprintf(stderr, _T("SymInitialize(): GetLastError = %lu\n"), gle );
        if (tt) free( tt );
        return 1;
    }

    // SymGetOptions()
    symOptions = pSGO();
    symOptions |= SYMOPT_DEBUG;
    symOptions |= SYMOPT_LOAD_LINES;
    symOptions |= SYMOPT_NO_PROMPTS;
    symOptions &= ~SYMOPT_UNDNAME;
    pSSO( symOptions ); // SymSetOptions()

    if (tt) 
        free( tt );

    LeaveCriticalSection(&g_csFileOpenClose);
    return 0;
}

int DebuggerDisplayDiagnostics()
{
    EnterCriticalSection(&g_csFileOpenClose);

    LPAPI_VERSION lpDV = NULL;
    TCHAR buf[TTBUFLEN];

    lpDV = pIAV();
    pSGSP(GetCurrentProcess(), buf, TTBUFLEN);

    _ftprintf( stderr, _T("\n\n"));
    _ftprintf( stderr, _T("BOINC Windows Debugger  Version %s\n"), BOINC_VERSION_STRING);
    _ftprintf( stderr, _T("\n"));
    _ftprintf( stderr, _T("Dump Timestamp    : "));
    DebuggerWriteDateTime();
    _ftprintf( stderr, _T("\n"));
    _ftprintf( stderr, _T("Debugger Engine   : %d.%d.%d.%d\n"), lpDV->MajorVersion, lpDV->MinorVersion, lpDV->Revision, lpDV->Reserved);
    _ftprintf( stderr, _T("Symbol Search Path: %s\n"), buf);
    _ftprintf( stderr, _T("\n\n"));

    if (!pSRC(GetCurrentProcess(), SymRegisterCallbackProc, NULL))
    {
        _ftprintf(stderr, _T("SymRegisterCallback64(): GetLastError = %lu\n"), gle );
    }

    if (!pSEM(GetCurrentProcess(), SymEnumerateModulesProc, NULL))
    {
        _ftprintf(stderr, _T("SymEnumerateModules64(): GetLastError = %lu\n"), gle );
    }

    _ftprintf( stderr, _T("\n"));

    LeaveCriticalSection(&g_csFileOpenClose);

    return 0;
}


// #################################################################################
// #################################################################################
// Here the Stackwalk-Part begins.
//   Some of the code is from an example from a book 
//   But I couldn´t find the reference anymore... sorry...
//   If someone knowns, please let me know...
// #################################################################################
// #################################################################################


// if you use C++ exception handling: install a translator function
// with set_se_translator(). In the context of that function (but *not*
// afterwards), you can either do your stack dump, or save the CONTEXT
// record as a local copy. Note that you must do the stack sump at the
// earliest opportunity, to avoid the interesting stackframes being gone
// by the time you do the dump.

// status: 
// - EXCEPTION_CONTINUE_SEARCH: exception wird weitergereicht
// - EXCEPTION_CONTINUE_EXECUTION: 
// - EXCEPTION_EXECUTE_HANDLER:
DWORD StackwalkFilter(EXCEPTION_POINTERS *ep, DWORD status)
{
  HANDLE hThread;

  DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
    GetCurrentProcess(), &hThread, 0, false, DUPLICATE_SAME_ACCESS );

  StackwalkThread( hThread, ep->ContextRecord);

  CloseHandle( hThread );

  return status;
}  // StackwalkFilter

void StackwalkThread(HANDLE hThread, CONTEXT* c)
{
  ShowStackRM( hThread, *c, GetCurrentProcess() );
}

static void ShowStackRM(HANDLE hThread, CONTEXT& Context, HANDLE hSWProcess)
{
    BOOL bRetVal = FALSE;

    int frameNum; // counts walked frames
    DWORD64 offsetFromSymbol; // tells us how far from the symbol we were
    DWORD offsetFromLine; // tells us how far from the line we were

    char undName[MAX_SYM_NAME]; // undecorated name
    IMAGEHLP_MODULE64 Module;
    IMAGEHLP_LINE64 Line;
    STACKFRAME64 StackFrame;

    ULONG64 SymbolBuffer[
        (sizeof(SYMBOL_INFO) +
        MAX_SYM_NAME*sizeof(TCHAR) +
        sizeof(ULONG64) - 1) /
        sizeof(ULONG64)
    ];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)SymbolBuffer;


    if (g_bInitialized == FALSE)
    {
        _ftprintf(stderr, _T("Stackwalker not initialized (or was not able to initialize)!\n"));
        return;
    }


    // Critical section begin...
    EnterCriticalSection(&g_csFileOpenClose);
    InterlockedIncrement((long*) &g_dwShowCount);  // erhöhe counter


    // Dump the Context data
    _ftprintf(stderr, 
        _T("eax=%.8x ebx=%.8x ecx=%.8x edx=%.8x esi=%.8x edi=%.8x\n"),
        Context.Eax, Context.Ebx, Context.Ecx, Context.Edx, Context.Esi, Context.Edi
    );
    _ftprintf(stderr, 
        _T("eip=%.8x esp=%.8x ebp=%.8x\n"),
        Context.Eip, Context.Esp, Context.Ebp
    );
    _ftprintf(stderr, 
        _T("cs=%.4x  ss=%.4x  ds=%.4x  es=%.4x  fs=%.4x  gs=%.4x             efl=%.8x\n\n"),
        Context.SegCs, Context.SegSs, Context.SegDs,  Context.SegEs,  Context.SegFs,  Context.SegGs, Context.EFlags
    );

    // Stack Header
    _ftprintf(stderr, _T("ChildEBP RetAddr  Args to Child\n"));
    fflush( stderr );


    // init STACKFRAME for first call
    // Notes: AddrModeFlat is just an assumption. I hate VDM debugging.
    // Notes: will have to be #ifdef-ed for Alphas; MIPSes are dead anyway,
    // and good riddance.
    memset( &StackFrame, '\0', sizeof(STACKFRAME64) );
    StackFrame.AddrPC.Offset = Context.Eip;
    StackFrame.AddrPC.Mode = AddrModeFlat;
    StackFrame.AddrFrame.Offset = Context.Ebp;
    StackFrame.AddrFrame.Mode = AddrModeFlat;

    memset( pSymbol, '\0', sizeof(SymbolBuffer) );
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    memset( &Line, '\0', sizeof(IMAGEHLP_LINE64) );
    Line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    memset( &Module, '\0', sizeof(IMAGEHLP_MODULE64) );
    Module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
  
    for ( frameNum = 0; ; ++frameNum )
    {
        // get next stack frame (StackWalk(), SymFunctionTableAccess(), SymGetModuleBase())
        // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
        // assume that either you are done, or that the stack is so hosed that the next
        // deeper frame could not be found.
        bRetVal = pSW(
            IMAGE_FILE_MACHINE_I386,
            hSWProcess,
            hThread,
            &StackFrame,
            &Context,
            NULL,
            (PFUNCTION_TABLE_ACCESS_ROUTINE64)pSFTA,
            (PGET_MODULE_BASE_ROUTINE64)pSGMB,
            NULL
        );

        if (!bRetVal)
            break;

        if ( StackFrame.AddrPC.Offset == 0 )
        {
            // Special case: If we are here, we have no valid callstack entry!
            _ftprintf(stderr, _T("(-nosymbols- PC == 0)\n"), g_dwShowCount);
        }
        else
        {
            // show procedure info (SymFromAddr())
            undName[0] = 0;
            offsetFromSymbol = 0;
            if ( !pSFA( hSWProcess, StackFrame.AddrPC.Offset, &offsetFromSymbol, pSymbol ) )
            {
                if ( gle != 487 )
                {
                    _ftprintf(stderr, _T("SymFromAddr(): GetLastError = '%lu' Address = '%.8x'\n"), gle, StackFrame.AddrPC.Offset);
                }
            }
            else
            {
                // UnDecorateSymbolName()
                pUDSN( pSymbol->Name, undName, MAX_SYM_NAME, UNDNAME_NAME_ONLY );
            }

            // show line number info (SymGetLineFromAddr())
            offsetFromLine = 0;
            if ( !pSGLFA( hSWProcess, StackFrame.AddrPC.Offset, &offsetFromLine, &Line ) )
            {
                if ( (gle != 487) && (frameNum > 0) )  // ignore error for first frame
                {
                    _ftprintf(stderr, _T("SymGetLineFromAddr(): GetLastError = '%lu' Address = '%.8x'\n"), gle, StackFrame.AddrPC.Offset);
                }
            }

            // show module info (SymGetModuleInfo())
            if ( !pSGMI( hSWProcess, StackFrame.AddrPC.Offset, &Module ) )
            {
                _ftprintf(stderr, _T("SymGetModuleInfo(): GetLastError = '%lu' Address = '%.8x'\n"), gle, StackFrame.AddrPC.Offset);
            }
        } // we seem to have a valid PC


        _ftprintf(stderr, "%.8x ", StackFrame.AddrFrame.Offset);
        _ftprintf(stderr, "%.8x ", StackFrame.AddrReturn.Offset);
        _ftprintf(stderr, "%.8x ", StackFrame.Params[0]);
        _ftprintf(stderr, "%.8x ", StackFrame.Params[1]);
        _ftprintf(stderr, "%.8x ", StackFrame.Params[2]);
        _ftprintf(stderr, "%s",    Module.ModuleName);
        _ftprintf(stderr, "!%s+",  undName);
        _ftprintf(stderr, "0x%x ", offsetFromLine);

        if (Line.LineNumber)
            _ftprintf(stderr, "(%s:%lu) ", Line.FileName, Line.LineNumber);

        if (StackFrame.FuncTableEntry) {
            // FPO Data
            PFPO_DATA pFPO = (PFPO_DATA)StackFrame.FuncTableEntry;
            switch(pFPO->cbFrame) {
                case FRAME_NONFPO:
                    _ftprintf(stderr, "FPO: [non-Fpo] ");
                    break;
                case FRAME_FPO:
                    _ftprintf(stderr, "FPO: [%d,%d,%d] ", pFPO->cdwParams, pFPO->cdwLocals, pFPO->cbRegs);
                    break;
                case FRAME_TRAP:
                    _ftprintf(stderr, "FPO: [%d,%d] TrapFrame @ 0x%.8x ", pFPO->cdwParams, pFPO->cdwLocals, pFPO->ulOffStart);
                    break;
                case FRAME_TSS:
                    _ftprintf(stderr, "FPO: TaskGate Segment: 0 ");
                    break;
            }
        }

        _ftprintf(stderr, "\n");
        fflush( stderr );


        // Zero out params so we have fresh parameters through the next interation
        StackFrame.Params[0] = NULL;
        StackFrame.Params[1] = NULL;
        StackFrame.Params[2] = NULL;
        StackFrame.Params[3] = NULL;


        // no return address means no deeper stackframe
        if ( StackFrame.AddrReturn.Offset == 0 )
        {
            // avoid misunderstandings in the printf() following the loop
            SetLastError( 0 );
            break;
        }
    } // for ( frameNum )

	switch(gle){
    case ERROR_SUCCESS:
        break;
    case ERROR_INVALID_ADDRESS:
        _ftprintf(stderr, _T("\nStackWalk(): ERROR_INVALID_ADDRESS (%lu) - Possible stack corruption.\n"), gle );
        break;
    case ERROR_NOACCESS:
        _ftprintf(stderr, _T("\nStackWalk(): ERROR_NOACCESS (%lu) - Possible stack corruption.\n"), gle );
    	break;
    default:
        _ftprintf(stderr, _T("\nStackWalk(): GetLastError = %lu\n"), gle );
        break;
	}

    fflush(stderr);

    LeaveCriticalSection(&g_csFileOpenClose);
}

const char *BOINC_RCSID_e8b4633192 = "$Id$";
