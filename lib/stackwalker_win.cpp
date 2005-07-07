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

#ifdef _WIN32
#include "boinc_win.h"
#endif

// the form of the output file
static eAllocCheckOutput g_CallstackOutputType = ACOutput_Simple;

// Size of Callstack-trace in bytes (0x500 => appr. 5-9 functions, depending on parameter count for each function)
#define MAX_ESP_LEN_BUF 0x500

// MaxSize: 128 KByte (only for StackwalkFilter)
#define LOG_FILE_MAX_SIZE 1024*128

// #############################################################################################
#ifdef _IMAGEHLP_
#error "'imagehlp.h' should only included here, not before this point! Otherwise there are some problems!"
#endif
#pragma pack( push, before_imagehlp, 8 )
#include <imagehlp.h>
#pragma pack( pop, before_imagehlp )
#if API_VERSION_NUMBER < 7  // ImageHelp-Version is older.... so define it by mayself
// The following definition is only available with VC++ 6.0 or higher, so include it here
extern "C" {
//
// source file line data structure
//
typedef struct _IMAGEHLP_LINE
{
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_LINE)
    DWORD                       Key;                    // internal
    DWORD                       LineNumber;             // line number in file
    PCHAR                       FileName;               // full filename
    DWORD                       Address;                // first instruction of line
} IMAGEHLP_LINE, *PIMAGEHLP_LINE;
#define SYMOPT_LOAD_LINES        0x00000010
}  // extern "C"
#endif



// Forward definitions of functions:
static void ShowStackRM( HANDLE hThread, CONTEXT& c, FILE *fLogFile, PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryFunction, HANDLE hProcess);
static void ShowStack( HANDLE hThread, CONTEXT& c, FILE *fLogFile);

static void AllocHashOut(FILE*);
static ULONG AllocHashOutLeaks(FILE*);


// Global data:
static BOOL g_bInitialized = FALSE;
static HINSTANCE g_hImagehlpDll = NULL;

static DWORD g_dwShowCount = 0;  // increase at every ShowStack-Call
static CRITICAL_SECTION g_csFileOpenClose = {0};

// Globale Vars:
static TCHAR *g_pszAllocLogName = NULL;
static FILE *g_fFile = NULL;

// AllocCheckFileOpen
//  Checks if the log-file is already opened
//  if not, try to open file (append or create if not exists)
//  if open failed, redirect output to stderr
static void AllocCheckFileOpen(bool bAppend = true) {
  // is the File already open? If not open it...
  if (g_fFile == NULL)
    if (g_pszAllocLogName != NULL)
    {
      if (bAppend == false)
        g_fFile = _tfopen(g_pszAllocLogName, _T("w"));
      else
        g_fFile = _tfopen(g_pszAllocLogName, _T("a"));
    }
  if (g_fFile == NULL)
    g_fFile = stderr;
}

// Write Date/Time to specified file (will also work after 2038)
static void WriteDateTime(FILE *fFile, BOOL asXMLAttrs = FALSE) {
  TCHAR pszTemp[11], pszTemp2[11];

  if (fFile != NULL) {
    _tstrdate( pszTemp );
    _tstrtime( pszTemp2 );
    if (asXMLAttrs == FALSE)
      _ftprintf(fFile,  _T("%s %s"), pszTemp, pszTemp2 );  // also ok after year 2038 (asctime is NOT ok)
    else
      _ftprintf(fFile,  _T("date=\"%s\" time=\"%s\" "), pszTemp, pszTemp2 );  // also ok after year 2038 (asctime is NOT ok)
  }
}  // WriteDateTime


// ##########################################################################################
// ##########################################################################################
// ##########################################################################################
// ##########################################################################################

#define gle (GetLastError())
#define lenof(a) (sizeof(a) / sizeof((a)[0]))
#define MAXNAMELEN 1024 // max name length for found symbols
#define IMGSYMLEN ( sizeof IMAGEHLP_SYMBOL )
#define TTBUFLEN 8096 // for a temp buffer (2^13)



// SymCleanup()
typedef BOOL (__stdcall *tSC)( IN HANDLE hProcess );
tSC pSC = NULL;

// SymFunctionTableAccess()
typedef PVOID (__stdcall *tSFTA)( HANDLE hProcess, DWORD AddrBase );
tSFTA pSFTA = NULL;

// SymGetLineFromAddr()
typedef BOOL (__stdcall *tSGLFA)( IN HANDLE hProcess, IN DWORD dwAddr,
  OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE Line );
tSGLFA pSGLFA = NULL;

// SymGetModuleBase()
typedef DWORD (__stdcall *tSGMB)( IN HANDLE hProcess, IN DWORD dwAddr );
tSGMB pSGMB = NULL;

// SymGetModuleInfo()
typedef BOOL (__stdcall *tSGMI)( IN HANDLE hProcess, IN DWORD dwAddr, OUT PIMAGEHLP_MODULE ModuleInfo );
tSGMI pSGMI = NULL;

// SymGetOptions()
typedef DWORD (__stdcall *tSGO)( VOID );
tSGO pSGO = NULL;

// SymGetSymFromAddr()
typedef BOOL (__stdcall *tSGSFA)( IN HANDLE hProcess, IN DWORD dwAddr,
  OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_SYMBOL Symbol );
tSGSFA pSGSFA = NULL;

// SymInitialize()
typedef BOOL (__stdcall *tSI)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
tSI pSI = NULL;

// SymLoadModule()
typedef DWORD (__stdcall *tSLM)( IN HANDLE hProcess, IN HANDLE hFile,
  IN PSTR ImageName, IN PSTR ModuleName, IN DWORD BaseOfDll, IN DWORD SizeOfDll );
tSLM pSLM = NULL;

// SymSetOptions()
typedef DWORD (__stdcall *tSSO)( IN DWORD SymOptions );
tSSO pSSO = NULL;

// StackWalk()
typedef BOOL (__stdcall *tSW)( DWORD MachineType, HANDLE hProcess,
  HANDLE hThread, LPSTACKFRAME StackFrame, PVOID ContextRecord,
  PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
  PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
  PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
  PTRANSLATE_ADDRESS_ROUTINE TranslateAddress );
tSW pSW = NULL;

// UnDecorateSymbolName()
typedef DWORD (__stdcall WINAPI *tUDSN)( PCSTR DecoratedName, PSTR UnDecoratedName,
  DWORD UndecoratedLength, DWORD Flags );
tUDSN pUDSN = NULL;



struct ModuleEntry
{
  std::string imageName;
  std::string moduleName;
  DWORD baseAddress;
  DWORD size;
};
typedef std::vector< ModuleEntry > ModuleList;
typedef ModuleList::iterator ModuleListIter;

// **************************************** ToolHelp32 ************************
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE   0x00000008
#pragma pack( push, 8 )
typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;
#pragma pack( pop )



static bool GetModuleListTH32(ModuleList& modules, DWORD pid, FILE *fLogFile)
{
  // CreateToolhelp32Snapshot()
  typedef HANDLE (__stdcall *tCT32S)(DWORD dwFlags, DWORD th32ProcessID);
  // Module32First()
  typedef BOOL (__stdcall *tM32F)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
  // Module32Next()
  typedef BOOL (__stdcall *tM32N)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

  // try both dlls...
  const TCHAR *dllname[] = { _T("kernel32.dll"), _T("tlhelp32.dll") };
  HINSTANCE hToolhelp = NULL;
  tCT32S pCT32S = NULL;
  tM32F pM32F = NULL;
  tM32N pM32N = NULL;

  HANDLE hSnap;
  MODULEENTRY32 me;
  me.dwSize = sizeof(me);
  bool keepGoing;
  ModuleEntry e;
  int i;

  for (i = 0; i<lenof(dllname); i++ )
  {
    hToolhelp = LoadLibrary( dllname[i] );
    if (hToolhelp == NULL)
      continue;
    pCT32S = (tCT32S) GetProcAddress(hToolhelp, "CreateToolhelp32Snapshot");
    pM32F = (tM32F) GetProcAddress(hToolhelp, "Module32First");
    pM32N = (tM32N) GetProcAddress(hToolhelp, "Module32Next");
    if ( pCT32S != 0 && pM32F != 0 && pM32N != 0 )
      break; // found the functions!
    FreeLibrary(hToolhelp);
    hToolhelp = NULL;
  }

  if (hToolhelp == NULL)
    return false;

  hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
  if (hSnap == (HANDLE) -1)
    return false;

  keepGoing = !!pM32F( hSnap, &me );
  while (keepGoing)
  {
    e.imageName = me.szExePath;
    e.moduleName = me.szModule;
    e.baseAddress = (DWORD) me.modBaseAddr;
    e.size = me.modBaseSize;
    modules.push_back( e );
    keepGoing = !!pM32N( hSnap, &me );
  }

  CloseHandle(hSnap);
  FreeLibrary(hToolhelp);

  return modules.size() != 0;
}  // GetModuleListTH32


// **************************************** PSAPI ************************
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

static bool GetModuleListPSAPI(ModuleList &modules, DWORD pid, HANDLE hProcess, FILE *fLogFile)
{
  // EnumProcessModules()
  typedef BOOL (__stdcall *tEPM)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
  // GetModuleFileNameEx()
  typedef DWORD (__stdcall *tGMFNE)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
  // GetModuleBaseName()
  typedef DWORD (__stdcall *tGMBN)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
  // GetModuleInformation()
  typedef BOOL (__stdcall *tGMI)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

  HINSTANCE hPsapi;
  tEPM pEPM;
  tGMFNE pGMFNE;
  tGMBN pGMBN;
  tGMI pGMI;

  DWORD i;
  ModuleEntry e;
  DWORD cbNeeded;
  MODULEINFO mi;
  HMODULE *hMods = 0;
  char *tt = 0;

  hPsapi = LoadLibrary( _T("psapi.dll") );
  if ( hPsapi == 0 )
    return false;

  modules.clear();

  pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
  pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameExA" );
  pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
  pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
  if ( pEPM == 0 || pGMFNE == 0 || pGMBN == 0 || pGMI == 0 )
  {
    // we couldn´t find all functions
    FreeLibrary( hPsapi );
    return false;
  }

  hMods = (HMODULE*) malloc(sizeof(HMODULE) * (TTBUFLEN / sizeof HMODULE));
  tt = (char*) malloc(sizeof(char) * TTBUFLEN);

  if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
  {
    _ftprintf(fLogFile, _T("%lu: EPM failed, GetLastError = %lu\n"), g_dwShowCount, gle );
    goto cleanup;
  }

  if ( cbNeeded > TTBUFLEN )
  {
    _ftprintf(fLogFile, _T("%lu: More than %lu module handles. Huh?\n"), g_dwShowCount, lenof( hMods ) );
    goto cleanup;
  }

  for ( i = 0; i < cbNeeded / sizeof hMods[0]; i++ )
  {
    // base address, size
    pGMI(hProcess, hMods[i], &mi, sizeof mi );
    e.baseAddress = (DWORD) mi.lpBaseOfDll;
    e.size = mi.SizeOfImage;
    // image file name
    tt[0] = 0;
    pGMFNE(hProcess, hMods[i], tt, TTBUFLEN );
    e.imageName = tt;
    // module name
    tt[0] = 0;
    pGMBN(hProcess, hMods[i], tt, TTBUFLEN );
    e.moduleName = tt;

    modules.push_back(e);
  }

cleanup:
  if (hPsapi)
    FreeLibrary(hPsapi);
  free(tt);
  free(hMods);

  return modules.size() != 0;
}  // GetModuleListPSAPI


static bool GetModuleList(ModuleList& modules, DWORD pid, HANDLE hProcess, FILE *fLogFile)
{
  // first try toolhelp32
  if (GetModuleListTH32(modules, pid, fLogFile) )
    return true;
  // then try psapi
  return GetModuleListPSAPI(modules, pid, hProcess, fLogFile);
}  // GetModuleList


static void EnumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid, FILE *fLogFile )
{
  static ModuleList modules;
  static ModuleListIter it;
  char *img, *mod;

  // fill in module list
  GetModuleList(modules, pid, hProcess, fLogFile);

  for ( it = modules.begin(); it != modules.end(); ++ it )
  {
    // SymLoadModule() wants writeable strings
    img = strdup(it->imageName.c_str());
    mod = strdup(it->moduleName.c_str());

    pSLM( hProcess, 0, img, mod, it->baseAddress, it->size );

    free(img);
    free(mod);
    std::string s;
  }
}  // EnumAndLoadModuleSymbols

static int InitStackWalk(void)
{
  if (g_bInitialized != FALSE)
    return 0;  // already initialized

  // old:  we load imagehlp.dll dynamically because the NT4-version does not
  // old: offer all the functions that are in the NT5 lib
  // 02-12-19: Now we only support dbghelp.dll!
  //           To use it on NT you have to install the redistrubutable for DBGHELP.DLL
  g_hImagehlpDll = LoadLibrary( _T("dbghelp.dll") );
  if ( g_hImagehlpDll == NULL )
  {
    printf( "LoadLibrary( \"dbghelp.dll\" ): GetLastError = %lu\n", gle );
    g_bInitialized = FALSE;
    return 1;
  }

  pSC = (tSC) GetProcAddress( g_hImagehlpDll, "SymCleanup" );
  pSFTA = (tSFTA) GetProcAddress( g_hImagehlpDll, "SymFunctionTableAccess" );
  pSGLFA = (tSGLFA) GetProcAddress( g_hImagehlpDll, "SymGetLineFromAddr" );
  pSGMB = (tSGMB) GetProcAddress( g_hImagehlpDll, "SymGetModuleBase" );
  pSGMI = (tSGMI) GetProcAddress( g_hImagehlpDll, "SymGetModuleInfo" );
  pSGO = (tSGO) GetProcAddress( g_hImagehlpDll, "SymGetOptions" );
  pSGSFA = (tSGSFA) GetProcAddress( g_hImagehlpDll, "SymGetSymFromAddr" );
  pSI = (tSI) GetProcAddress( g_hImagehlpDll, "SymInitialize" );
  pSSO = (tSSO) GetProcAddress( g_hImagehlpDll, "SymSetOptions" );
  pSW = (tSW) GetProcAddress( g_hImagehlpDll, "StackWalk" );
  pUDSN = (tUDSN) GetProcAddress( g_hImagehlpDll, "UnDecorateSymbolName" );
  pSLM = (tSLM) GetProcAddress( g_hImagehlpDll, "SymLoadModule" );

  if ( pSC == NULL || pSFTA == NULL || pSGMB == NULL || pSGMI == NULL ||
    pSGO == NULL || pSGSFA == NULL || pSI == NULL || pSSO == NULL ||
    pSW == NULL || pUDSN == NULL || pSLM == NULL )
  {
    printf( "GetProcAddress(): some required function not found.\n" );
    FreeLibrary( g_hImagehlpDll );
    g_bInitialized = FALSE;
    return 1;
  }

  g_bInitialized = TRUE;
  InitializeCriticalSection(&g_csFileOpenClose);
  return 0;
}

static TCHAR s_szExceptionLogFileName[_MAX_PATH] = _T("\\exceptions.log");  // default
static BOOL s_bUnhandledExeptionFilterSet = FALSE;
static LONG __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
   LONG lRet;
   lRet = StackwalkFilter(pExPtrs, /*EXCEPTION_CONTINUE_SEARCH*/EXCEPTION_EXECUTE_HANDLER, NULL);
   TCHAR lString[500];
   _stprintf(lString,
      _T("*** Unhandled Exception!\n")
      _T("   ExpCode: 0x%8.8X\n")
      _T("   ExpFlags: %d\n")
      _T("   ExpAddress: 0x%8.8X\n")
      _T("   Please report!"),
      pExPtrs->ExceptionRecord->ExceptionCode,
      pExPtrs->ExceptionRecord->ExceptionFlags,
      (PVOID)pExPtrs->ExceptionRecord->ExceptionAddress);
   FatalAppExit(-1,lString);
   return lRet;
}


static TCHAR *GetExpectionCodeText(DWORD dwExceptionCode) {
  switch(dwExceptionCode) {
  case EXCEPTION_ACCESS_VIOLATION: return _T("ACCESS VIOLATION");
  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return _T("ARRAY BOUNDS EXCEEDED");
  case EXCEPTION_BREAKPOINT: return _T("BREAKPOINT");
  case EXCEPTION_DATATYPE_MISALIGNMENT: return _T("DATATYPE MISALIGNMENT");
  case EXCEPTION_FLT_DENORMAL_OPERAND: return _T("FLT DENORMAL OPERAND");
  case EXCEPTION_FLT_DIVIDE_BY_ZERO: return _T("FLT DIVIDE BY ZERO");
  case EXCEPTION_FLT_INEXACT_RESULT: return _T("FLT INEXACT RESULT");
  case EXCEPTION_FLT_INVALID_OPERATION: return _T("FLT INVALID OPERATION");
  case EXCEPTION_FLT_OVERFLOW: return _T("FLT OVERFLOW");
  case EXCEPTION_FLT_STACK_CHECK: return _T("FLT STACK CHECK");
  case EXCEPTION_FLT_UNDERFLOW: return _T("FLT UNDERFLOW");
  case EXCEPTION_ILLEGAL_INSTRUCTION: return _T("ILLEGAL INSTRUCTION");
  case EXCEPTION_IN_PAGE_ERROR: return _T("IN PAGE ERROR");
  case EXCEPTION_INT_DIVIDE_BY_ZERO: return _T("INT DIVIDE BY ZERO");
  case EXCEPTION_INT_OVERFLOW: return _T("INT OVERFLOW");
  case EXCEPTION_INVALID_DISPOSITION: return _T("INVALID DISPOSITION");
  case EXCEPTION_NONCONTINUABLE_EXCEPTION: return _T("NONCONTINUABLE EXCEPTION");
  case EXCEPTION_PRIV_INSTRUCTION: return _T("PRIV INSTRUCTION");
  case EXCEPTION_SINGLE_STEP: return _T("SINGLE STEP");
  case EXCEPTION_STACK_OVERFLOW: return _T("STACK OVERFLOW");
  case DBG_CONTROL_C : return _T("DBG CONTROL C ");
  default:
    return _T("<unkown exception>");
  }
}  // GetExpectionCodeText

// Function is not multi-threading safe, because of static char!
static TCHAR *GetAdditionalExpectionCodeText(PEXCEPTION_RECORD pExceptionRecord) {
  static TCHAR szTemp[100];

  switch(pExceptionRecord->ExceptionCode) {
  case EXCEPTION_ACCESS_VIOLATION:
    if (pExceptionRecord->NumberParameters == 2) {
      switch(pExceptionRecord->ExceptionInformation[0]) {
      case 0: // read attempt
        _stprintf(szTemp, _T(" read attempt to address 0x%8.8X "), pExceptionRecord->ExceptionInformation[1]);
        return szTemp;
      case 1: // write attempt
        _stprintf(szTemp, _T(" write attempt to address 0x%8.8X "), pExceptionRecord->ExceptionInformation[1]);
        return szTemp;
      default:
        return _T("");
      }
    }  // if (pExceptionRecord->NumberParameters == 2)
    return _T("");
  default:
    return _T("");
  }  // switch(pExceptionRecord->ExceptionCode)
}  // GetAdditionalExpectionCodeText

std::string SimpleXMLEncode(PCSTR szText)
{
  std::string szRet;

  for (size_t i=0; i<strlen(szText); i++)
  {
    switch(szText[i])
    {
    case '&':
      szRet.append("&amp;");
      break;
    case '<':
      szRet.append("&lt;");
      break;
    case '>':
      szRet.append("&gt;");
      break;
    case '"':
      szRet.append("&quot;");
      break;
    case '\'':
      szRet.append("&apos;");
      break;
    default:
      szRet += szText[i];
    }
  }
  return szRet;
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
DWORD StackwalkFilter( EXCEPTION_POINTERS *ep, DWORD status, LPCTSTR pszLogFile)
{
  HANDLE hThread;
  FILE *fFile = stderr;  // default to stderr

  if (pszLogFile != NULL) {  // a filename is provided
    // Open the logfile
    fFile = _tfopen(pszLogFile, _T("a"));
    if (fFile != NULL) {  // Is the file too big?
      long size;
      fseek(fFile, 0, SEEK_END);
      size = ftell(fFile);  // Get the size of the file
      if (size >= LOG_FILE_MAX_SIZE) {
        TCHAR *pszTemp = (TCHAR*) malloc(MAX_PATH);
        // It is too big...
        fclose(fFile);
        _tcscpy(pszTemp, pszLogFile);
        _tcscat(pszTemp, _T(".old"));
        _tremove(pszTemp);  // Remove an old file, if exists
        _trename(pszLogFile, pszTemp);  // rename the actual file
        fFile = _tfopen(pszLogFile, _T("w"));  // create a new file
        free(pszTemp);
      }
    }
  }  // if (pszLogFile != NULL) 
  if (fFile == NULL) {
    fFile = stderr;
  }

  // Write infos about the exception
  if (g_CallstackOutputType == ACOutput_XML)
  {
    _ftprintf(fFile, _T("<EXCEPTION code=\"%8.8X\" addr=\"%8.8X\" "), 
      ep->ExceptionRecord->ExceptionCode,
      (PVOID)ep->ExceptionRecord->ExceptionAddress);
    WriteDateTime(fFile, TRUE);
    _ftprintf(fFile, _T("code_desc=\"%s\" more_desc=\"%s\">\n"), GetExpectionCodeText(ep->ExceptionRecord->ExceptionCode),
      GetAdditionalExpectionCodeText(ep->ExceptionRecord));
  }

  DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
    GetCurrentProcess(), &hThread, 0, false, DUPLICATE_SAME_ACCESS );
  ShowStack( hThread, *(ep->ContextRecord), fFile);
  CloseHandle( hThread );

  if (g_CallstackOutputType == ACOutput_XML)
    _ftprintf(fFile, _T("</EXCEPTION>\n"));

  fclose(fFile);

  return status;
}  // StackwalkFilter

void ShowStack( HANDLE hThread, CONTEXT& c, LPCTSTR pszLogFile)
{
  FILE *fFile = stderr;  // default to stderr

  if (pszLogFile != NULL) {  // a filename is available
    // Open the logfile
    fFile = _tfopen(pszLogFile, _T("a"));
    if (fFile != NULL) {  // Is the file too big?
      long size;
      fseek(fFile, 0, SEEK_END);
      size = ftell(fFile);  // Get the size of the file
      if (size >= LOG_FILE_MAX_SIZE) {
        TCHAR *pszTemp = (TCHAR*) malloc(MAX_PATH);
        // It is too big...
        fclose(fFile);
        _tcscpy(pszTemp, pszLogFile);
        _tcscat(pszTemp, _T(".old"));
        _tremove(pszTemp);  // Remove an old file, if exists
        _trename(pszLogFile, pszTemp);  // rename the actual file
        fFile = _tfopen(pszLogFile, _T("w"));  // open new file
        free(pszTemp);
      }
    }
  }  // if (pszLogFile != NULL) 
  if (fFile == NULL) {
    fFile = stderr;
  }

  ShowStack( hThread, c, fFile);

  fclose(fFile);
}


static void ShowStack( HANDLE hThread, CONTEXT& c, FILE *fLogFile) {
  ShowStackRM(hThread, c, fLogFile, NULL, GetCurrentProcess());
}

static void ShowStackRM( HANDLE hThread, CONTEXT& c, FILE *fLogFile, PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryFunction, HANDLE hSWProcess) {
  // normally, call ImageNtHeader() and use machine info from PE header
  DWORD imageType = IMAGE_FILE_MACHINE_I386;
  HANDLE hProcess = GetCurrentProcess(); // hProcess normally comes from outside
  int frameNum; // counts walked frames
  DWORD offsetFromSymbol; // tells us how far from the symbol we were
  DWORD offsetFromLine; // tells us how far from the line we were
  DWORD symOptions; // symbol handler settings

  static IMAGEHLP_SYMBOL *pSym = NULL;
  char undName[MAXNAMELEN]; // undecorated name
  char undFullName[MAXNAMELEN]; // undecorated name with all shenanigans
  IMAGEHLP_MODULE Module;
  IMAGEHLP_LINE Line;
  BOOL bXMLTagWrote;

  std::string symSearchPath;

  static BOOL bFirstTime = TRUE;

  // If no logfile is present, outpur to "stderr"
  if (fLogFile == NULL) {
    fLogFile = stderr;
  }

  STACKFRAME s; // in/out stackframe
  memset( &s, '\0', sizeof s );

  if ( (g_bInitialized == FALSE) && (bFirstTime == TRUE) ) {
    InitStackWalk();
  }

  if (g_bInitialized == FALSE)
  {
    // Could not init!!!!
    bFirstTime = FALSE;
    _ftprintf(fLogFile, _T("%lu: Stackwalker not initialized (or was not able to initialize)!\n"), g_dwShowCount);
    return;
  }

// Critical section begin...
  EnterCriticalSection(&g_csFileOpenClose);

  InterlockedIncrement((long*) &g_dwShowCount);  // erhöhe counter


  // NOTE: normally, the exe directory and the current directory should be taken
  // from the target process. The current dir would be gotten through injection
  // of a remote thread; the exe fir through either ToolHelp32 or PSAPI.

  if (pSym == NULL) {
    pSym = (IMAGEHLP_SYMBOL *) malloc( IMGSYMLEN + MAXNAMELEN );
    if (!pSym) goto cleanup;  // not enough memory...
  }

  if (g_CallstackOutputType != ACOutput_XML)
  {
    _ftprintf(fLogFile, _T("%lu: "), g_dwShowCount);
    WriteDateTime(fLogFile);
    _ftprintf(fLogFile, _T("\n"));
  }


  if (bFirstTime) {

    CHAR *tt, *p;

    tt = (CHAR*) malloc(sizeof(CHAR) * TTBUFLEN); // Get the temporary buffer
    if (!tt) goto cleanup;  // not enough memory...

    // build symbol search path from:
    symSearchPath = "";
    // current directory
    if ( GetCurrentDirectoryA( TTBUFLEN, tt ) )
      symSearchPath += tt + std::string( ";" );
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
          ++ p;
        *p = '\0'; // eliminate the exe name and last path sep
        symSearchPath += tt + std::string( ";" );
      }
    }
    // environment variable _NT_SYMBOL_PATH
    if ( GetEnvironmentVariableA( "_NT_SYMBOL_PATH", tt, TTBUFLEN ) )
      symSearchPath += tt + std::string( ";" );
    // environment variable _NT_ALTERNATE_SYMBOL_PATH
    if ( GetEnvironmentVariableA( "_NT_ALTERNATE_SYMBOL_PATH", tt, TTBUFLEN ) )
      symSearchPath += tt + std::string( ";" );
    // environment variable SYSTEMROOT
    if ( GetEnvironmentVariableA( "SYSTEMROOT", tt, TTBUFLEN ) )
      symSearchPath += tt + std::string( ";" );



    if ( symSearchPath.size() > 0 ) // if we added anything, we have a trailing semicolon
      symSearchPath = symSearchPath.substr( 0, symSearchPath.size() - 1 );

    // why oh why does SymInitialize() want a writeable string?
    strncpy( tt, symSearchPath.c_str(), TTBUFLEN );
    tt[TTBUFLEN - 1] = '\0'; // if strncpy() overruns, it doesn't add the null terminator

    // init symbol handler stuff (SymInitialize())
    if ( ! pSI( hProcess, tt, false ) )
    {
      if (g_CallstackOutputType != ACOutput_XML)
        _ftprintf(fLogFile, _T("%lu: SymInitialize(): GetLastError = %lu\n"), g_dwShowCount, gle );
      if (tt) free( tt );
      goto cleanup;
    }

    // SymGetOptions()
    symOptions = pSGO();
    symOptions |= SYMOPT_LOAD_LINES;
    symOptions &= ~SYMOPT_UNDNAME;
    symOptions &= ~SYMOPT_DEFERRED_LOADS;
    pSSO( symOptions ); // SymSetOptions()

    // Enumerate modules and tell imagehlp.dll about them.
    // On NT, this is not necessary, but it won't hurt.
    EnumAndLoadModuleSymbols( hProcess, GetCurrentProcessId(), fLogFile );

    if (tt) 
      free( tt );
  }  // bFirstTime = TRUE
  bFirstTime = FALSE;

  // init STACKFRAME for first call
  // Notes: AddrModeFlat is just an assumption. I hate VDM debugging.
  // Notes: will have to be #ifdef-ed for Alphas; MIPSes are dead anyway,
  // and good riddance.
  s.AddrPC.Offset = c.Eip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.Ebp;
  s.AddrFrame.Mode = AddrModeFlat;

  memset( pSym, '\0', IMGSYMLEN + MAXNAMELEN );
  pSym->SizeOfStruct = IMGSYMLEN;
  pSym->MaxNameLength = MAXNAMELEN;

  memset( &Line, '\0', sizeof Line );
  Line.SizeOfStruct = sizeof Line;

  memset( &Module, '\0', sizeof Module );
  Module.SizeOfStruct = sizeof Module;

  for ( frameNum = 0; ; ++ frameNum )
  {
    // get next stack frame (StackWalk(), SymFunctionTableAccess(), SymGetModuleBase())
    // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
    // assume that either you are done, or that the stack is so hosed that the next
    // deeper frame could not be found.
    // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
    if ( ! pSW( imageType, hSWProcess, hThread, &s, NULL, ReadMemoryFunction, pSFTA, pSGMB, NULL ) )
      break;

    bXMLTagWrote = FALSE;

    if (g_CallstackOutputType == ACOutput_Advanced)
      _ftprintf(fLogFile, _T("\n%lu: %3d"), g_dwShowCount, frameNum);
    if ( s.AddrPC.Offset == 0 )
    {
      // Special case: If we are here, we have no valid callstack entry!
      switch(g_CallstackOutputType)
      {
      case ACOutput_Simple:
        _ftprintf(fLogFile, _T("%lu: (-nosymbols- PC == 0)\n"), g_dwShowCount);
        break;
      case ACOutput_Advanced:
        _ftprintf(fLogFile, _T("   (-nosymbols- PC == 0)\n"));
        break;
      case ACOutput_XML:
        // TODO: ....
        _ftprintf(fLogFile, _T("<STACKENTRY decl=\"(-nosymbols- PC == 0)\"/>\n"));
        break;
      }
    }
    else
    {
      // we seem to have a valid PC
      undName[0] = 0;
      undFullName[0] = 0;
      offsetFromSymbol = 0;
      // show procedure info (SymGetSymFromAddr())
      if ( ! pSGSFA( hProcess, s.AddrPC.Offset, &offsetFromSymbol, pSym ) )
      {
        if (g_CallstackOutputType == ACOutput_Advanced)
        {
          if ( gle != 487 )
            _ftprintf(fLogFile, _T("   SymGetSymFromAddr(): GetLastError = %lu\n"), gle );
          else
            _ftprintf(fLogFile, _T("\n"));
        }
      }
      else
      {
        // UnDecorateSymbolName()
        pUDSN( pSym->Name, undName, MAXNAMELEN, UNDNAME_NAME_ONLY );
        pUDSN( pSym->Name, undFullName, MAXNAMELEN, UNDNAME_COMPLETE );
        if (g_CallstackOutputType == ACOutput_Advanced)
        {
          if (strlen(undName) > 0)
            fprintf(fLogFile, "     %s %+ld bytes\n", undName, (long) offsetFromSymbol );
          else
          {
            fprintf(fLogFile, "     Sig:  %s %+ld bytes\n", pSym->Name, (long) offsetFromSymbol );
            strcpy(undName, pSym->Name);
          }
          fprintf(fLogFile, "%lu:     Decl: %s\n", g_dwShowCount, undFullName );
        }
      }
      //if (g_CallstackOutputType == ACOutput_XML)
      //  fprintf(fLogFile, "decl=\"%s\" decl_offset=\"%+ld\" ", SimpleXMLEncode(undName).c_str(), (long) offsetFromSymbol);

      // show line number info, NT5.0-method (SymGetLineFromAddr())
      offsetFromLine = 0;
      if ( pSGLFA != NULL )
      { // yes, we have SymGetLineFromAddr()
        if ( ! pSGLFA( hProcess, s.AddrPC.Offset, &offsetFromLine, &Line ) )
        {
          if ( (gle != 487) && (frameNum > 0) )  // ignore error for first frame
          {
            if (g_CallstackOutputType == ACOutput_XML)
            {
              _ftprintf(fLogFile, _T("<STACKENTRY "));
              bXMLTagWrote = TRUE;
              fprintf(fLogFile, "decl=\"%s\" decl_offset=\"%+ld\" ", SimpleXMLEncode(undName).c_str(), (long) offsetFromSymbol);
              _ftprintf(fLogFile, _T("srcfile=\"SymGetLineFromAddr(): GetLastError = %lu\" "), gle);
            }
            else
              _ftprintf(fLogFile, _T("%lu: SymGetLineFromAddr(): GetLastError = %lu\n"), g_dwShowCount, gle );
          }
        }
        else
        {
          switch(g_CallstackOutputType)
          {
          case ACOutput_Advanced:
            fprintf(fLogFile, "%lu:     Line: %s(%lu) %+ld bytes\n", g_dwShowCount,
              Line.FileName, Line.LineNumber, offsetFromLine );
            break;
          case ACOutput_Simple:
            fprintf(fLogFile, "%lu: %s(%lu) %+ld bytes (%s)\n", g_dwShowCount,
              Line.FileName, Line.LineNumber, offsetFromLine, undName);
            break;
          case ACOutput_XML:
            _ftprintf(fLogFile, _T("<STACKENTRY "));
            bXMLTagWrote = TRUE;
            fprintf(fLogFile, "decl=\"%s\" decl_offset=\"%+ld\" ", SimpleXMLEncode(undName).c_str(), (long) offsetFromSymbol);
            fprintf(fLogFile, "srcfile=\"%s\" line=\"%lu\" line_offset=\"%+ld\" ", 
              SimpleXMLEncode(Line.FileName).c_str(), Line.LineNumber, offsetFromLine, undName);
            break;
          }
        }
      } // yes, we have SymGetLineFromAddr()

      // show module info (SymGetModuleInfo())
      if ( (g_CallstackOutputType == ACOutput_Advanced) || (g_CallstackOutputType == ACOutput_XML) )
      {
        if ( ! pSGMI( hProcess, s.AddrPC.Offset, &Module ) )
        {
          if (g_CallstackOutputType == ACOutput_Advanced)
            _ftprintf(fLogFile, _T("%lu: SymGetModuleInfo): GetLastError = %lu\n"), g_dwShowCount, gle );
        }
        else
        { // got module info OK
          char ty[80];
          switch ( Module.SymType )
          {
          case SymNone:
            strcpy( ty, "-nosymbols-" );
            break;
          case SymCoff:
            strcpy( ty, "COFF" );
            break;
          case SymCv:
            strcpy( ty, "CV" );
            break;
          case SymPdb:
            strcpy( ty, "PDB" );
            break;
          case SymExport:
            strcpy( ty, "-exported-" );
            break;
          case SymDeferred:
            strcpy( ty, "-deferred-" );
            break;
          case SymSym:
            strcpy( ty, "SYM" );
            break;
          /* // TODO: #if API_VERSION_NUMBER >= 9 ?
          case SymDia:
            strcpy( ty, "DIA" );
            break;*/
          default:
            _snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
            break;
          }

          if (g_CallstackOutputType == ACOutput_XML)
          {
            // now, check if the XML-Entry is written...
            if (bXMLTagWrote == FALSE) 
            {
              _ftprintf(fLogFile, _T("<STACKENTRY "));
              bXMLTagWrote = TRUE;
              fprintf(fLogFile, "decl=\"%s\" decl_offset=\"%+ld\" ", SimpleXMLEncode(undName).c_str(), (long) offsetFromSymbol);
              _ftprintf(fLogFile, _T("srcfile=\"\" "));
              bXMLTagWrote = TRUE;
            }
          }

          if (g_CallstackOutputType == ACOutput_Advanced)
          {
            fprintf(fLogFile, "%lu:     Mod:  %s, base: %08lxh\n", g_dwShowCount,
              Module.ModuleName, Module.BaseOfImage );
            if (Module.SymType == SymNone) { // Gebe nur aus, wenn keine Symbole vorhanden sind!
              _ftprintf(fLogFile, _T("%lu:     Offset: 0x%8.8x\n"), g_dwShowCount, s.AddrPC.Offset);
              fprintf(fLogFile, "%lu:     Sym:  type: %s, file: %s\n", g_dwShowCount,
                ty, Module.LoadedImageName );
            }
          }
          else
          {
            // XML:
            if (bXMLTagWrote == TRUE)
              fprintf(fLogFile, "module=\"%s\" base=\"%08lx\" ", Module.ModuleName, Module.BaseOfImage);
          }
        } // got module info OK
      }
      if ( (g_CallstackOutputType == ACOutput_XML) && (bXMLTagWrote == TRUE) )
        _ftprintf(fLogFile, _T("/>\n"));  // terminate the XML node

    } // we seem to have a valid PC

    // no return address means no deeper stackframe
    if ( s.AddrReturn.Offset == 0 )
    {
      // avoid misunderstandings in the printf() following the loop
      SetLastError( 0 );
      break;
    }

  } // for ( frameNum )

  if ( (g_CallstackOutputType != ACOutput_XML) && (gle != 0) )
    _ftprintf(fLogFile, _T("\n%lu: StackWalk(): GetLastError = %lu\n"), g_dwShowCount, gle );

cleanup:
  //if (pSym) free( pSym );
  if (fLogFile) {
    _ftprintf(fLogFile, _T("\n\n"));
    if (g_dwShowCount % 1000)
      fflush(fLogFile);
  }

  LeaveCriticalSection(&g_csFileOpenClose);
// Critical section end...
}  // ShowStackRM

const char *BOINC_RCSID_e8b4633192 = "$Id$";
