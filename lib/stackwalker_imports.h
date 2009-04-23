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

#ifndef __STACKWALKER_IMPORTS_H__
#define __STACKWALKER_IMPORTS_H__

#define gle (GetLastError())
#define TTBUFLEN                8096 // for a temp buffer (2^13)

#if defined(__MINGW32__) || defined(__CYGWIN32__)

#define MAX_SYM_NAME            2000

#define SYMOPT_LOAD_ANYTHING            0x00000040
#define SYMOPT_IGNORE_CVREC             0x00000080
#define SYMOPT_NO_UNQUALIFIED_LOADS     0x00000100
#define SYMOPT_FAIL_CRITICAL_ERRORS     0x00000200
#define SYMOPT_EXACT_SYMBOLS            0x00000400
#define SYMOPT_ALLOW_ABSOLUTE_SYMBOLS   0x00000800
#define SYMOPT_IGNORE_NT_SYMPATH        0x00001000
#define SYMOPT_INCLUDE_32BIT_MODULES    0x00002000
#define SYMOPT_PUBLICS_ONLY             0x00004000
#define SYMOPT_NO_PUBLICS               0x00008000
#define SYMOPT_AUTO_PUBLICS             0x00010000
#define SYMOPT_NO_IMAGE_SEARCH          0x00020000
#define SYMOPT_SECURE                   0x00040000
#define SYMOPT_NO_PROMPTS               0x00080000
#define SYMOPT_DEBUG                    0x80000000

#define SSRVOPT_CALLBACK            0x0001
#define SSRVOPT_DWORD               0x0002
#define SSRVOPT_DWORDPTR            0x0004
#define SSRVOPT_GUIDPTR             0x0008
#define SSRVOPT_OLDGUIDPTR          0x0010
#define SSRVOPT_UNATTENDED          0x0020
#define SSRVOPT_NOCOPY              0x0040
#define SSRVOPT_PARENTWIN           0x0080
#define SSRVOPT_PARAMTYPE           0x0100
#define SSRVOPT_SECURE              0x0200
#define SSRVOPT_TRACE               0x0400
#define SSRVOPT_SETCONTEXT          0x0800
#define SSRVOPT_PROXY               0x1000
#define SSRVOPT_DOWNSTREAM_STORE    0x2000
#define SSRVOPT_RESET               ((ULONG_PTR)-1)

#define SSRVACTION_TRACE        1
#define SSRVACTION_QUERYCANCEL  2
#define SSRVACTION_EVENT        3

#define CBA_READ_MEMORY                         0x00000006
#define CBA_DEFERRED_SYMBOL_LOAD_CANCEL         0x00000007
#define CBA_SET_OPTIONS                         0x00000008
#define CBA_EVENT                               0x00000010
#define CBA_DEFERRED_SYMBOL_LOAD_PARTIAL        0x00000020
#define CBA_DEBUG_INFO                          0x10000000

enum {
    sevInfo = 0,
    sevProblem,
    sevAttn,
    sevFatal,
    sevMax
};

typedef enum {
    SymDia = 7,
    SymVirtual,
    NumSymTypes
} SYM_TYPE_EX;

typedef PCSTR PCTSTR;

typedef struct _IMAGEHLP_CBA_EVENT {
    DWORD severity;                                     
    DWORD code;                                         
    PCHAR desc;                                         
    PVOID object;                                  
} IMAGEHLP_CBA_EVENT, *PIMAGEHLP_CBA_EVENT;

typedef struct _ADDRESS64 {
    DWORD64       Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS64, *LPADDRESS64;

typedef struct _KDHELP64 {
    DWORD64 Thread;
    DWORD   ThCallbackStack;
    DWORD   ThCallbackBStore;
    DWORD   NextCallback;
    DWORD   FramePointer;
    DWORD64 KiCallUserMode;
    DWORD64 KeUserCallbackDispatcher;
    DWORD64 SystemRangeStart;
    DWORD64 Reserved[8];
} KDHELP64, *PKDHELP64;

typedef struct _IMAGEHLP_LINE64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_LINE64)
    PVOID                       Key;                    // internal
    DWORD                       LineNumber;             // line number in file
    PCHAR                       FileName;               // full filename
    DWORD64                     Address;                // first instruction of line
} IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;

typedef struct _IMAGEHLP_MODULE64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
    DWORD64                     BaseOfImage;            // base load address of module
    DWORD                       ImageSize;              // virtual size of the loaded module
    DWORD                       TimeDateStamp;          // date/time stamp from pe header
    DWORD                       CheckSum;               // checksum from the pe header
    DWORD                       NumSyms;                // number of symbols in the symbol table
    SYM_TYPE                    SymType;                // type of symbols loaded
    CHAR                        ModuleName[32];         // module name
    CHAR                        ImageName[256];         // image name
    CHAR                        LoadedImageName[256];   // symbol file name
} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64;

typedef struct _SYMBOL_INFO {
    ULONG       SizeOfStruct;
    ULONG       TypeIndex;        // Type Index of symbol
    ULONG64     Reserved[2];
    ULONG       info;
    ULONG       Size;
    ULONG64     ModBase;          // Base Address of module comtaining this symbol
    ULONG       Flags;
    ULONG64     Value;            // Value of symbol, ValuePresent should be 1
    ULONG64     Address;          // Address of symbol including base address of module
    ULONG       Register;         // register holding value or pointer to value
    ULONG       Scope;            // scope of the symbol
    ULONG       Tag;              // pdb classification
    ULONG       NameLen;          // Actual length of name
    ULONG       MaxNameLen;
    CHAR        Name[1];          // Name of symbol
} SYMBOL_INFO, *PSYMBOL_INFO;

typedef struct _STACKFRAME64 {
    ADDRESS64   AddrPC;               // program counter
    ADDRESS64   AddrReturn;           // return address
    ADDRESS64   AddrFrame;            // frame pointer
    ADDRESS64   AddrStack;            // stack pointer
    ADDRESS64   AddrBStore;           // backing store pointer
    PVOID       FuncTableEntry;       // pointer to pdata/fpo or NULL
    DWORD64     Params[4];            // possible arguments to the function
    BOOL        Far;                  // WOW far call
    BOOL        Virtual;              // is this a virtual frame?
    DWORD64     Reserved[3];
    KDHELP64    KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;

typedef BOOL
(CALLBACK *PSYM_ENUMMODULES_CALLBACK64)(
    LPCSTR ModuleName,
    DWORD64 BaseOfDll,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYMBOL_REGISTERED_CALLBACK64)(
    HANDLE  hProcess,
    ULONG   ActionCode,
    ULONG64 CallbackData,
    ULONG64 UserContext
    );

typedef
BOOL
(CALLBACK *PREAD_PROCESS_MEMORY_ROUTINE64)(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead
    );

typedef
PVOID
(CALLBACK *PFUNCTION_TABLE_ACCESS_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 AddrBase
    );

typedef
DWORD64
(CALLBACK *PGET_MODULE_BASE_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 Address
    );

typedef
DWORD64
(CALLBACK *PTRANSLATE_ADDRESS_ROUTINE64)(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 lpaddr
    );

#endif


// ImagehlpApiVersion()
typedef LPAPI_VERSION (__stdcall *tIAV)(
    VOID
);

// SymCleanup()
typedef BOOL (__stdcall *tSC)(
    IN HANDLE hProcess
);

// SymEnumerateModules64()
typedef BOOL (__stdcall *tSEM)(
    IN HANDLE hProcess,
    IN PSYM_ENUMMODULES_CALLBACK64 EnumModulesCallback,
    IN PVOID UserContext
);

// SymFunctionTableAccess64()
typedef PVOID (__stdcall *tSFTA)( 
    IN HANDLE hProcess,
    IN DWORD64 AddrBase
);

// SymGetLineFromAddr64()
typedef BOOL (__stdcall *tSGLFA)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr,
    OUT PDWORD pdwDisplacement,
    OUT PIMAGEHLP_LINE64 Line
);

// SymGetModuleBase64()
typedef DWORD64 (__stdcall *tSGMB)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr
);

// SymGetModuleInfo64()
typedef BOOL (__stdcall *tSGMI)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr,
    OUT PIMAGEHLP_MODULE64 ModuleInfo
);

// SymGetOptions()
typedef DWORD (__stdcall *tSGO)(
    VOID
);

// SymGetSearchPath()
typedef BOOL (__stdcall *tSGSP)(
    IN HANDLE hProcess,
    OUT PTSTR SearchPath,
    IN DWORD SearchPathLength
);

// SymFromAddr()
typedef BOOL (__stdcall *tSFA)(
    IN HANDLE hProcess,
    IN DWORD64 dwAddr,
    OUT PDWORD64 pdwDisplacement,
    OUT PSYMBOL_INFO Symbol
);

// SymInitialize()
typedef BOOL (__stdcall *tSI)(
    IN HANDLE hProcess,
    IN PCSTR UserSearchPath,
    IN BOOL fInvadeProcess
);

// SymLoadModuleEx()
typedef DWORD64 (__stdcall *tSLM)(
    IN HANDLE hProcess,
    IN HANDLE hFile,
    IN PCSTR ImageName,
    IN PCSTR ModuleName,
    IN DWORD64 BaseOfDll,
    IN DWORD SizeOfDll
);

// SymRegisterCallback64()
typedef BOOL (__stdcall *tSRC)(
    IN HANDLE hProcess,
    PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction,
    ULONG64 UserContext
);

// SymSetOptions()
typedef DWORD (__stdcall *tSSO)(
    IN DWORD SymOptions
);

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

// UnDecorateSymbolName()
typedef DWORD (__stdcall *tUDSN)(
    PCSTR DecoratedName,
    PSTR  UnDecoratedName,
    DWORD UndecoratedLength,
    DWORD Flags
);

// SymbolServerSetOptions
typedef BOOL (__stdcall *tSSSO)(
    UINT_PTR options,
    ULONG64 data
);

// SetDllDirectory
typedef BOOL (__stdcall *tSDD)(
    LPCSTR lpPathName
);


// GetFileVersionInfoSize 
typedef BOOL (__stdcall *tGFVIS)(
    LPCSTR lptstrFilename,
    LPDWORD lpdwHandle
);

// GetFileVersionInfo 
typedef BOOL (__stdcall *tGFVI)(
    LPCSTR lptstrFilename,
    DWORD dwHandle,
    DWORD dwLen,
    LPVOID lpData
);

// VerQueryValue 
typedef BOOL (__stdcall *tVQV)(
    const LPVOID pBlock,
    LPCSTR lpSubBlock,
    LPVOID *lplpBuffer,
    PUINT puLen
);


#ifndef SYMOPT_NO_PROMPTS
#define SYMOPT_NO_PROMPTS               0x00080000
#endif

#ifndef SSRVACTION_EVENT
#define SSRVACTION_EVENT                3
#endif

#ifndef SSRVOPT_PROXY
#define SSRVOPT_PROXY                   0x00001000
#endif


#endif
