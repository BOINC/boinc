/*////////////////////////////////////////////////////////////////////////////
 *  Project:
 *    Memory_and_Exception_Trace
 *
 * ///////////////////////////////////////////////////////////////////////////
 *	File:
 *		Stackwalker.cpp
 *
 *	Remarks:
 *    Dumps memory leaks (unreleased allocations) for CRT-Allocs and COM-Allocs
 *    Dumps the stack of an thread if an exepction occurs
 *
 *  Known bugs:
 *    - If the allocation-RequestID wrap, then allocations will get lost...
 *
 *	Author:
 *		Jochen Kalmbach, Germany
 *
 *//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <crtdbg.h>
#include <tchar.h>

#include "Stackwalker.h"

// If the following is defined, only the used memories are stored in the hash-table. 
// If the memory is freed, it will be removed from the hash-table (to reduce memory)
// Consequences: At DeInitAllocHook, only Leaks will be reported
#define HASH_ENTRY_REMOVE_AT_FREE


// 0 = Do not write any output during runtime-alloc-call
// 1 = Write only the alloc action (malloc, realloc, free)
// 2 = Write alloc action and callstack only for malloc/realloc
// 3 = Write alloc action and callstack for all actions
static ULONG g_ulShowStackAtAlloc = 0;

// the form of the output file
static eAllocCheckOutput g_CallstackOutputType = ACOutput_Simple;


// Size of Hash-Table
#define ALLOC_HASH_ENTRIES 1024


// Size of Callstack-trace in bytes (0x500 => appr. 5-9 functions, depending on parameter count for each function)
#define MAX_ESP_LEN_BUF 0x500


// Normally we can ignore allocations from the Runtime-System
#define IGNORE_CRT_ALLOC

// MaxSize: 128 KByte (only for StackwalkFilter)
#define LOG_FILE_MAX_SIZE 1024*128

// If the following is defined, then COM-Leaks will also be tracked
#define WITH_IMALLOC_SPY

// #############################################################################################
#ifdef WITH_IMALLOC_SPY
//forwards:
void IMallocHashInsert(void *pData, CONTEXT &Context, size_t nDataSize);
BOOL IMallocHashRemove(void *pData);

// IMallocSpy-Interface
class CMallocSpy : public IMallocSpy
{
public:
  CMallocSpy(void) {
    m_cbRequest = 0;
  }
  ~CMallocSpy(void) {
  }
  // IUnknown methods
  STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppUnk) {
    HRESULT hr = S_OK;
    if (IsEqualIID(riid, IID_IUnknown)) {
        *ppUnk = (IUnknown *) this;
    }
    else if (IsEqualIID(riid, IID_IMallocSpy)) {
        *ppUnk =  (IMalloc *) this;
    }
    else {
        *ppUnk = NULL;
        hr =  E_NOINTERFACE;
    }
    AddRef();
    return hr;
  }
  STDMETHOD_(ULONG, AddRef) (void) {
    return InterlockedIncrement(&m_cRef);
  }
  STDMETHOD_(ULONG, Release) (void) {
    LONG cRef;
    cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
      delete this;
    }
    return cRef;
  }
  // IMallocSpy methods
  STDMETHOD_(ULONG, PreAlloc) (ULONG cbRequest) {
    m_cbRequest = cbRequest;
    return cbRequest;
  }
  STDMETHOD_(void *, PostAlloc) (void *pActual) {
    HANDLE hThread;
    if (DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
      GetCurrentProcess(), &hThread, 0, false, DUPLICATE_SAME_ACCESS ) != 0) {
      // Ok
      CONTEXT c;
      memset( &c, '\0', sizeof c );
      c.ContextFlags = CONTEXT_FULL;
      if ( GetThreadContext( hThread, &c )  != 0) {
        // Ok
        IMallocHashInsert(pActual, c, m_cbRequest);
      }
      CloseHandle(hThread);
    }
    return pActual;
  }
  STDMETHOD_(void *, PreFree) (void *pRequest, BOOL fSpyed) {
    IMallocHashRemove(pRequest);
    return pRequest;
  }
  STDMETHOD_(void, PostFree) (BOOL fSpyed) {
    return;
  }
  STDMETHOD_(ULONG, PreRealloc) (void *pRequest, ULONG cbRequest,
    void **ppNewRequest, BOOL fSpyed) {
    IMallocHashRemove(pRequest);
    m_cbRequest = cbRequest;
    return cbRequest;
  }
  STDMETHOD_(void *, PostRealloc) (void *pActual, BOOL fSpyed) {
    HANDLE hThread;
    if (DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
      GetCurrentProcess(), &hThread, 0, false, DUPLICATE_SAME_ACCESS ) != 0) {
      // Ok
      CONTEXT c;
      memset( &c, '\0', sizeof c );
      c.ContextFlags = CONTEXT_FULL;
      if ( GetThreadContext( hThread, &c )  != 0) {
        // Ok
        IMallocHashInsert(pActual, c, m_cbRequest);
      }
      CloseHandle(hThread);
    }
    return pActual;
  }
  STDMETHOD_(void *, PreGetSize) (void *pRequest, BOOL fSpyed) {
    return pRequest;
  }
  STDMETHOD_(ULONG, PostGetSize) (ULONG cbActual, BOOL fSpyed) {
    return cbActual;
  }
  STDMETHOD_(void *, PreDidAlloc) (void *pRequest, BOOL fSpyed) {
    return pRequest;
  }
  STDMETHOD_(BOOL, PostDidAlloc) (void *pRequest, BOOL fSpyed, BOOL fActual) {
    return fActual;
  }
  STDMETHOD_(void, PreHeapMinimize) (void) {
    return;
  }
  STDMETHOD_(void, PostHeapMinimize) (void) {
    return;
  }
private:
  LONG    m_cRef;
  ULONG m_cbRequest;
};
#endif

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



// Globale Vars:
static TCHAR *g_pszAllocLogName = NULL;
static FILE *g_fFile = NULL;

// AllocCheckFileOpen
//  Checks if the log-file is already opened
//  if not, try to open file (append or create if not exists)
//  if open failed, redirect output to stdout
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
    g_fFile = stdout;
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


/*******************************************************************************
 * Hash-Tabelle
 *******************************************************************************/
// Memory for the EIP-Address (is used by the ShowStack-method)
#define MAX_EIP_LEN_BUF 4

#define ALLOC_ENTRY_NOT_FOUND 0xFFFFFFFF

typedef struct AllocHashEntryType {
  long                       lRequestID;    // RequestID from CRT (if 0, then this entry is empty)
  size_t                     nDataSize;     // Size of the allocated memory
  char                       cRemovedFlag;  // 0 => memory was not yet released
  struct AllocHashEntryType  *Next;
  // Callstack for EIP
  DWORD                      dwEIPOffset;
  DWORD                      dwEIPLen;
  char                       pcEIPAddr[MAX_EIP_LEN_BUF];
  // Callstack for ESP
  DWORD                      dwESPOffset;
  DWORD                      dwESPLen;
  char                       pcESPAddr[MAX_ESP_LEN_BUF];
} AllocHashEntryType;

static AllocHashEntryType AllocHashTable[ALLOC_HASH_ENTRIES];
static ULONG AllocHashEntries = 0;
static ULONG AllocHashCollisions = 0;
static ULONG AllocHashFreed = 0;
static ULONG AllocHashMaxUsed = 0; // maximal number of concurrent entries
static ULONG AllocHashCurrentCount = 0;

static ULONG AllocHashMaxCollisions = 0;
static ULONG AllocHashCurrentCollisions = 0;

// ##########################################################################################
#ifdef WITH_IMALLOC_SPY
// eigene Tabelle für die IMallocs:
typedef struct IMallocHashEntryType {
  void                       *pData;    // Key-Word
  size_t                     nDataSize;     // größe des Datenblocks (optional)
  char                       cRemovedFlag;  // 0 => nicht wurde noch nicht freigegeben
  struct IMallocHashEntryType  *Next;
  // Callstack für EIP
  DWORD                      dwEIPOffset;
  DWORD                      dwEIPLen;
  char                       pcEIPAddr[MAX_EIP_LEN_BUF];
  // Callstack für ESP
  DWORD                      dwESPOffset;
  DWORD                      dwESPLen;
  char                       pcESPAddr[MAX_ESP_LEN_BUF];
} IMallocHashEntryType;

static IMallocHashEntryType IMallocHashTable[ALLOC_HASH_ENTRIES];

static ULONG IMallocHashEntries = 0;
static ULONG IMallocHashCollisions = 0;
static ULONG IMallocHashFreed = 0;
static ULONG IMallocHashMaxUsed = 0; // maximal number of concurrent entries
static ULONG IMallocHashCurrentCount = 0;

static ULONG IMallocHashMaxCollisions = 0;
static ULONG IMallocHashCurrentCollisions = 0;


//static void AllocHashOut(FILE*);
static ULONG IMallocHashOutLeaks(FILE*);

// AllocHashFunction
//   Die eigentliche Hash-Funktion (hier ganz simpel)
static ULONG IMallocHashFunction(void *pData) {
  ULONG ulTemp;
  DWORD dwPointer = (DWORD) pData;

  // relativ simpler Mechanismus für die Hash-Funktion,
  // mir ist nur nix besseres eingefallen...
  ulTemp = dwPointer % ALLOC_HASH_ENTRIES;

  _ASSERTE( (ulTemp >= 0) && (ulTemp < ALLOC_HASH_ENTRIES) );

  return ulTemp;
}  // AllocHashFunction

// IMallocHashInsert
//   pData: Key-Word (Pointer to address)
//   pContext:   Context-Record, for retrieving Callstack (EIP and EBP is only needed)
//   nDataSize:  How many bytes
void IMallocHashInsert(void *pData, CONTEXT &Context, size_t nDataSize) {
  ULONG HashIdx;
  IMallocHashEntryType *pHashEntry;

  // ermittle Statistische Werte
  IMallocHashEntries++;
  IMallocHashCurrentCount++;
  if (IMallocHashCurrentCount > IMallocHashMaxUsed)
    IMallocHashMaxUsed = IMallocHashCurrentCount;

  // ermittle den Hash-Wert
  HashIdx = IMallocHashFunction(pData);

  // Eintrag darf nicht größer als die Hash-Tabelle sein
  _ASSERTE(HashIdx < ALLOC_HASH_ENTRIES);

  pHashEntry = &IMallocHashTable[HashIdx];
  if (pHashEntry->pData == 0) {
    // es ist noch kein Eintrag da
  }
  else {
    //Statistische Daten:
    IMallocHashCollisions++;
    IMallocHashCurrentCollisions++;
    if (IMallocHashCurrentCollisions > IMallocHashMaxCollisions)
      IMallocHashMaxCollisions = IMallocHashCurrentCollisions;

    // Eintrag ist schon belegt, verkette die Einträge
    // wenn dies oft vorkommt, sollte man entweder die Tabelle vergrößern oder eine
    // andere Hash-Funktion wählen
    while(pHashEntry->Next != NULL) {
      pHashEntry = pHashEntry->Next;
    }

    pHashEntry->Next = (IMallocHashEntryType*) _calloc_dbg(sizeof(IMallocHashEntryType), 1, _CRT_BLOCK, __FILE__, __LINE__);
    pHashEntry = pHashEntry->Next;

  }
  pHashEntry->pData = pData;  // Key-Word
  pHashEntry->nDataSize = nDataSize;
  pHashEntry->Next = NULL;
  // Get EIP and save it in the record
  pHashEntry->dwEIPOffset = Context.Eip;
  if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID) Context.Eip, &(pHashEntry->pcEIPAddr), MAX_EIP_LEN_BUF, &(pHashEntry->dwEIPLen)) == 0) {
    // Could not read memory... remove everything...
    memset(pHashEntry->pcEIPAddr, 0, MAX_EIP_LEN_BUF);
    pHashEntry->dwEIPLen = 0;
    pHashEntry->dwEIPOffset = 0;
  }

  // Get ESP and save it in the record
  pHashEntry->dwESPOffset = Context.Ebp;
  if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID) Context.Ebp, &(pHashEntry->pcESPAddr), MAX_ESP_LEN_BUF, &(pHashEntry->dwESPLen)) == 0) {
    // Could not read memory... remove everything...
    memset(pHashEntry->pcESPAddr, 0, MAX_ESP_LEN_BUF);
    pHashEntry->dwESPLen = 0;
    pHashEntry->dwESPOffset = 0;

    // Check if I tried to read too much...
    if (GetLastError() == ERROR_PARTIAL_COPY)
    {
      // ask how many I can read:
      MEMORY_BASIC_INFORMATION MemBuffer;
      DWORD dwRet = VirtualQuery((LPCVOID) Context.Ebp, &MemBuffer, sizeof(MemBuffer));
      if (dwRet > 0)
      {
        // calculate the length
        DWORD len = ((DWORD) MemBuffer.BaseAddress + MemBuffer.RegionSize) - Context.Ebp;
        if ( (len > 0) && (len < MAX_ESP_LEN_BUF) )
        {
          // try to read it again (with the shorter length)
          if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID) Context.Ebp, &(pHashEntry->pcESPAddr), len, &(pHashEntry->dwESPLen)) == 0)
          {
            // ok, now everything goes wrong... remove it...
            memset(pHashEntry->pcESPAddr, 0, MAX_ESP_LEN_BUF);
            pHashEntry->dwESPLen = 0;
            pHashEntry->dwESPOffset = 0;
          }
          else
          {
            pHashEntry->dwESPOffset = Context.Ebp;
          }
        }
      } // VirtualQuery was successfully
    }  // ERROR_PARTIAL_COPY
  }
}

// IMallocHashFind
//   Wird ALLOC_ENTRY_NOT_FOUND zurückgegeben, so wurde der Key nicht 
//   gefunden, ansonsten wird ein Zeiger auf den Hash-Eintrag zurückgegeben
//   ACHTUNG: In einem preemptiven Tasking-System kann hier nicht 
//            garantiert werden, ob der Zeiger noch gültig ist, wenn er 
//            zurückgegeben wird, da er von einem anderen Thread schon
//            freigegeben sein könnte. 
//            Die synchronisation muß eine Ebene höher erfolgen
static IMallocHashEntryType *IMallocHashFind(void *pData) {
  ULONG HashIdx;
  IMallocHashEntryType *pHashEntry;

  // ermittle den Hash-Wert
  HashIdx = IMallocHashFunction(pData);

  // Eintrag darf nicht größer als die Hash-Tabelle sein
  _ASSERTE(HashIdx < ALLOC_HASH_ENTRIES);

  pHashEntry = &IMallocHashTable[HashIdx];
  while(pHashEntry != NULL) {
    if (pHashEntry->pData == pData) {
      return pHashEntry;
    }
    pHashEntry = pHashEntry->Next;
  }

  // wenn hier angelangt, dann wurde der Eintrag nicht gefunden!
  return (IMallocHashEntryType*) ALLOC_ENTRY_NOT_FOUND;
}  // AllocHashFind

// IMallocHashRemove
//   Return: FALSE (0) : Key wurde gefunden und entfernt/markiert
//           TRUE (!=0): Key wurde nicht gefunden!
BOOL IMallocHashRemove(void *pData) {
  ULONG HashIdx;
  IMallocHashEntryType *pHashEntry, *pHashEntryLast;

  // ermittle den Hash-Wert
  HashIdx = IMallocHashFunction(pData);

  // Eintrag darf nicht größer als die Hash-Tabelle sein
  _ASSERTE(HashIdx < ALLOC_HASH_ENTRIES);

  pHashEntryLast = NULL;
  pHashEntry = &IMallocHashTable[HashIdx];
  while(pHashEntry != NULL) {
    if (pHashEntry->pData == pData) {
#ifdef HASH_ENTRY_REMOVE_AT_FREE
      IMallocHashFreed++;
      IMallocHashCurrentCount--;
      // gebe den Speicher frei
      if (pHashEntryLast == NULL) {
        // Es ist ein Eintrag direkt in der Tabelle
        if (pHashEntry->Next == NULL) {
          // Es ist der letze Eintrag lösche also die Tabelle
          memset(&IMallocHashTable[HashIdx], 0, sizeof(IMallocHashTable[HashIdx]));
        }
        else {
          // Es sind noch Einträge verkettet, überschreibe einfach den nicht mehr gebrauchten...
          *pHashEntry = *(pHashEntry->Next);
        }
        return TRUE;
      }
      else {
        // ich bin in einem dynamischen Bereich
        // dies war eine kollisions, zähle also wieder zurück:
        IMallocHashCurrentCollisions--;
        pHashEntryLast->Next = pHashEntry->Next;
        _free_dbg(pHashEntry, _CRT_BLOCK);
        return TRUE;
      }
#else
      // erhöhe nur den Removed counter und behalte das Object im Speicher
      pHashEntry->cRemovedFlag++;
      return TRUE;  // erfolgreich
#endif
    }
    pHashEntryLast = pHashEntry;
    pHashEntry = pHashEntry->Next;
  }

  // wenn hier angelangt, dann wurde der Eintrag nicht gefunden!
  return FALSE;
}



//   Callback-Funtion for StackWalk für meine CallStack-Ausgabe aus der Hash-Tabelle
#if API_VERSION_NUMBER >= 9
static BOOL __stdcall ReadProcMemoryFromIMallocHash(HANDLE pData, DWORD lpBaseAddress, PVOID lpBuffer, DWORD nSize, PDWORD lpNumberOfBytesRead) {
#else
static BOOL __stdcall ReadProcMemoryFromIMallocHash(HANDLE pData, LPCVOID lpBaseAddress, PVOID lpBuffer, DWORD nSize, PDWORD lpNumberOfBytesRead) {
#endif
  // Versuche die hRequestID zu finden
  IMallocHashEntryType *pHashEntry;
  *lpNumberOfBytesRead = 0;

  pHashEntry = IMallocHashFind((PVOID) pData);
  if (pHashEntry == (IMallocHashEntryType*) ALLOC_ENTRY_NOT_FOUND) {
    // nicht gefunden, somit kann ich den Speicher nicht lesen
    *lpNumberOfBytesRead = 0;
    return FALSE;
  }
  if ( ((DWORD) lpBaseAddress >= pHashEntry->dwESPOffset) && ((DWORD) lpBaseAddress <= (pHashEntry->dwESPOffset+pHashEntry->dwESPLen)) ) {
    // Speicher liegt im ESP:
    // Errechne den Offset
    DWORD dwOffset = (DWORD) lpBaseAddress - pHashEntry->dwESPOffset;
    DWORD dwSize = __min(nSize, MAX_ESP_LEN_BUF-dwOffset);
    memcpy(lpBuffer, &(pHashEntry->pcESPAddr[dwOffset]), dwSize);
    *lpNumberOfBytesRead = dwSize;
    if (dwSize != nSize)
      return FALSE;
  }

  if ( ((DWORD) lpBaseAddress >= pHashEntry->dwEIPOffset) && ((DWORD) lpBaseAddress <= (pHashEntry->dwEIPOffset+pHashEntry->dwEIPLen)) ) {
    // Speicher liegt im EIP:
    // Errechne den Offset
    DWORD dwOffset = (DWORD) lpBaseAddress - pHashEntry->dwEIPOffset;
    DWORD dwSize = __min(nSize, MAX_ESP_LEN_BUF-dwOffset);
    memcpy(lpBuffer, &(pHashEntry->pcEIPAddr[dwOffset]), dwSize);
    *lpNumberOfBytesRead = dwSize;
    if (dwSize != nSize)
      return FALSE;
  }
  
  if (*lpNumberOfBytesRead == 0)  // Der Speicher konnte nicht gefunden werden
    return FALSE;

  return TRUE;
}
// AllocHashOutLeaks
// Gibt allen Speicher aus, der noch nicht wieder freigegeben wurde
//   Returns the number of bytes, that are not freed (leaks)
ULONG IMallocHashOutLeaks(FILE *fFile) {
  ULONG ulTemp;
  IMallocHashEntryType *pHashEntry;
  ULONG ulCount = 0;
  ULONG ulLeaksByte = 0;

  // Gehe jeden Eintrag durch und gebe ihn aus
  for(ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++) {
    pHashEntry = &IMallocHashTable[ulTemp];
    if (pHashEntry->pData != 0) {
      while(pHashEntry != NULL) {
        // gebe die Zeile aus
        if ( (pHashEntry->cRemovedFlag <= 0) || (pHashEntry->cRemovedFlag > 1) ) {
          ulCount++;
          if (g_CallstackOutputType == ACOutput_XML)
            _ftprintf(fFile, _T("<LEAK requestID=\"%u\" size=\"%u\">\n"), pHashEntry->pData, pHashEntry->nDataSize);
          else
            _ftprintf(fFile, _T("Pointer (RequestID): %12i, Removed: %i, Size: %12i\n"), pHashEntry->pData, pHashEntry->cRemovedFlag, pHashEntry->nDataSize);
          CONTEXT c;
          memset( &c, '\0', sizeof c );
          c.Eip = pHashEntry->dwEIPOffset;
          c.Ebp = pHashEntry->dwESPOffset;
          ShowStackRM( NULL, c, fFile, &ReadProcMemoryFromIMallocHash, (HANDLE) pHashEntry->pData);
          // Zähle zusammen wieviel Byte noch nicht freigegeben wurden
          if (pHashEntry->nDataSize > 0)
            ulLeaksByte += pHashEntry->nDataSize;
          else
            ulLeaksByte++;  // Wenn zwar Speicher allokiert wurde, dieser aber 0 Bytes lang war, so reserviere für diesen zumindest 1 Byte

          if (g_CallstackOutputType == ACOutput_XML)
            _ftprintf(fFile, _T("</LEAK>\n"));  // terminate the xml-node
        }
        pHashEntry = pHashEntry->Next;
      }
    }
  }
  if (g_CallstackOutputType != ACOutput_XML)
    _ftprintf(fFile, _T("\n**** Number of leaks: %i\n"), ulCount);
  return ulLeaksByte;
}  // AllocHashOutLeaks
#endif


static void AllocHashInit(void) {

  memset(AllocHashTable, 0, sizeof(AllocHashTable));
  AllocHashEntries = 0;
  AllocHashCollisions = 0;
  AllocHashFreed = 0;
  AllocHashCurrentCount = 0;
  AllocHashMaxUsed = 0;

  AllocHashMaxCollisions = 0;
  AllocHashCurrentCollisions = 0;

#ifdef WITH_IMALLOC_SPY
  memset(IMallocHashTable, 0, sizeof(IMallocHashTable));
  IMallocHashEntries = 0;
  IMallocHashCollisions = 0;
  IMallocHashFreed = 0;
  IMallocHashCurrentCount = 0;
  IMallocHashMaxUsed = 0;

  IMallocHashMaxCollisions = 0;
  IMallocHashCurrentCollisions = 0;
#endif
  return;
}  // AllocHashInit


// AllocHashDeinit
// Returns the number of bytes, that are not freed (leaks)
static ULONG AllocHashDeinit(void) {
  ULONG ulRet = 0;
  bool bAppend = g_CallstackOutputType != ACOutput_XML;
  AllocCheckFileOpen(bAppend);  // open global log-file

  if (g_CallstackOutputType == ACOutput_XML)
  {
    _ftprintf(g_fFile, _T("<MEMREPORT "));
    WriteDateTime(g_fFile, TRUE);
    _ftprintf(g_fFile, _T(">\n"));
  }
  else
  {
    _ftprintf(g_fFile, _T("\n##### Memory Report ########################################\n"));
    WriteDateTime(g_fFile);
    _ftprintf(g_fFile, _T("\n"));
  }

#ifndef HASH_ENTRY_REMOVE_AT_FREE
  // output the used memory
  if (g_CallstackOutputType != ACOutput_XML)
    _ftprintf(g_fFile, _T("##### Memory used: #########################################\n"));
  AllocHashOut(g_fFile);
#endif

  // output the Memoty leaks
  if (g_CallstackOutputType != ACOutput_XML)
    _ftprintf(g_fFile, _T("\n##### Leaks: ###############################################\n"));
  ulRet = AllocHashOutLeaks(g_fFile);

  if (g_CallstackOutputType == ACOutput_Advanced)
  {
    // output some statistics from the hash-table
    _ftprintf(g_fFile, _T("***** Hash-Table statistics:\n"));
    _ftprintf(g_fFile, _T("      Table-Size:     %i\n"), ALLOC_HASH_ENTRIES);
    _ftprintf(g_fFile, _T("      Inserts:        %i\n"), AllocHashEntries);
    _ftprintf(g_fFile, _T("      Freed:          %i\n"), AllocHashFreed);
    _ftprintf(g_fFile, _T("      Sum Collisions: %i\n"), AllocHashCollisions);
    _ftprintf(g_fFile, _T("\n"));
    _ftprintf(g_fFile, _T("      Max used:       %i\n"), AllocHashMaxUsed);
    _ftprintf(g_fFile, _T("      Max Collisions: %i\n"), AllocHashMaxCollisions);
  }

  // Free Hash-Table
  ULONG ulTemp;
  AllocHashEntryType *pHashEntry, *pHashEntryOld;

  // Now, free my own memory
  for(ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++) {
    pHashEntry = &AllocHashTable[ulTemp];
    while(pHashEntry != NULL) {
      pHashEntryOld = pHashEntry;
      pHashEntry = pHashEntry->Next;
      if (pHashEntryOld != &AllocHashTable[ulTemp]) {
        // now free the dynamically allocated memory
        free(pHashEntryOld);
      }
    }  // while
  }  // for
  // empty the hash-table
  memset(AllocHashTable, 0, sizeof(AllocHashTable));

#ifdef WITH_IMALLOC_SPY
  // output the Memoty leaks
  if (g_CallstackOutputType != ACOutput_XML)
    _ftprintf(g_fFile, _T("\n##### COM-Leaks: ###############################################\n"));
  ulRet = IMallocHashOutLeaks(g_fFile);

  if (g_CallstackOutputType == ACOutput_Advanced)
  {
    // output some statistics from the hash-table
    _ftprintf(g_fFile, _T("***** Hash-Table statistics:\n"));
    _ftprintf(g_fFile, _T("      Table-Size:     %i\n"), ALLOC_HASH_ENTRIES);
    _ftprintf(g_fFile, _T("      Inserts:        %i\n"), IMallocHashEntries);
    _ftprintf(g_fFile, _T("      Freed:          %i\n"), IMallocHashFreed);
    _ftprintf(g_fFile, _T("      Sum Collisions: %i\n"), IMallocHashCollisions);
    _ftprintf(g_fFile, _T("\n"));
    _ftprintf(g_fFile, _T("      Max used:       %i\n"), IMallocHashMaxUsed);
    _ftprintf(g_fFile, _T("      Max Collisions: %i\n"), IMallocHashMaxCollisions);
  }

  // Free Hash-Table
  //ULONG ulTemp;
  IMallocHashEntryType *pIMHashEntry, *pIMHashEntryOld;

  // Gehe jeden Eintrag durch und gebe ihn frei
  for(ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++) {
    pIMHashEntry = &IMallocHashTable[ulTemp];
    while(pHashEntry != NULL) {
      pIMHashEntryOld = pIMHashEntry;
      pIMHashEntry = pIMHashEntry->Next;
      if (pIMHashEntryOld != &IMallocHashTable[ulTemp]) {
        // es ist dynamischer Speicher, gebe ihn also frei:
        _free_dbg(pIMHashEntryOld, _CRT_BLOCK);
      }
    }  // while
  }  // for
  // Lösche die gesamte Hash-Tabelle
  memset(IMallocHashTable, 0, sizeof(IMallocHashTable));
#endif


  if (g_CallstackOutputType == ACOutput_XML)
    _ftprintf(g_fFile, _T("</MEMREPORT>\n"));

  return ulRet;
}  // AllocHashDeinit

// AllocHashFunction
// The has-function (very simple)
static inline ULONG AllocHashFunction(long lRequestID) {
  // I couldn´t find any better and faster
  return lRequestID % ALLOC_HASH_ENTRIES;
}  // AllocHashFunction

// AllocHashInsert
//   lRequestID: Key-Word (RequestID from AllocHook)
//   pContext:   Context-Record, for retrieving Callstack (EIP and EBP is only needed)
//   nDataSize:  How many bytes
static void AllocHashInsert(long lRequestID, CONTEXT &Context, size_t nDataSize) {
  ULONG HashIdx;
  AllocHashEntryType *pHashEntry;

  // change statistical data
  AllocHashEntries++;
  AllocHashCurrentCount++;
  if (AllocHashCurrentCount > AllocHashMaxUsed)
    AllocHashMaxUsed = AllocHashCurrentCount;

  // generate hash-value
  HashIdx = AllocHashFunction(lRequestID);

  pHashEntry = &AllocHashTable[HashIdx];
  if (pHashEntry->lRequestID == 0) {
    // Entry is empty...
  }
  else {
    // Entry is not empy! make a list of entries for this hash value...
    // change statistical data
    // if this happens often, you should increase the hah size or change the heash-function; 
    // to fasten the allocation time
    AllocHashCollisions++;
    AllocHashCurrentCollisions++;
    if (AllocHashCurrentCollisions > AllocHashMaxCollisions)
      AllocHashMaxCollisions = AllocHashCurrentCollisions;

    while(pHashEntry->Next != NULL) {
      pHashEntry = pHashEntry->Next;
    }

    pHashEntry->Next = (AllocHashEntryType*) calloc(sizeof(AllocHashEntryType), 1);
    pHashEntry = pHashEntry->Next;

  }
  pHashEntry->lRequestID = lRequestID;  // Key-Word
  pHashEntry->nDataSize = nDataSize;
  pHashEntry->Next = NULL;
  // Get EIP and save it in the record
  pHashEntry->dwEIPOffset = Context.Eip;
  if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID) Context.Eip, &(pHashEntry->pcEIPAddr), MAX_EIP_LEN_BUF, &(pHashEntry->dwEIPLen)) == 0) {
    // Could not read memory... remove everything...
    memset(pHashEntry->pcEIPAddr, 0, MAX_EIP_LEN_BUF);
    pHashEntry->dwEIPLen = 0;
    pHashEntry->dwEIPOffset = 0;
  }

  // Get ESP and save it in the record
  pHashEntry->dwESPOffset = Context.Ebp;
  if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID) Context.Ebp, &(pHashEntry->pcESPAddr), MAX_ESP_LEN_BUF, &(pHashEntry->dwESPLen)) == 0) {
    // Could not read memory... remove everything...
    memset(pHashEntry->pcESPAddr, 0, MAX_ESP_LEN_BUF);
    pHashEntry->dwESPLen = 0;
    pHashEntry->dwESPOffset = 0;

    // Check if I tried to read too much...
    if (GetLastError() == ERROR_PARTIAL_COPY)
    {
      // ask how many I can read:
      MEMORY_BASIC_INFORMATION MemBuffer;
      DWORD dwRet = VirtualQuery((LPCVOID) Context.Ebp, &MemBuffer, sizeof(MemBuffer));
      if (dwRet > 0)
      {
        // calculate the length
        DWORD len = ((DWORD) MemBuffer.BaseAddress + MemBuffer.RegionSize) - Context.Ebp;
        if ( (len > 0) && (len < MAX_ESP_LEN_BUF) )
        {
          // try to read it again (with the shorter length)
          if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID) Context.Ebp, &(pHashEntry->pcESPAddr), len, &(pHashEntry->dwESPLen)) == 0)
          {
            // ok, now everything goes wrong... remove it...
            memset(pHashEntry->pcESPAddr, 0, MAX_ESP_LEN_BUF);
            pHashEntry->dwESPLen = 0;
            pHashEntry->dwESPOffset = 0;
          }
          else
          {
            pHashEntry->dwESPOffset = Context.Ebp;
          }
        }
      } // VirtualQuery was successfully
    }  // ERROR_PARTIAL_COPY
  }
}

// AllocHashFind
//   If ALLOC_ENTRY_NOT_FOUND is returned, the Key was not found!
//   If the Key was found, a pointer to the entry is returned
static AllocHashEntryType *AllocHashFind(long lRequestID) {
  ULONG HashIdx;
  AllocHashEntryType *pHashEntry;

  // get the Hash-Value
  HashIdx = AllocHashFunction(lRequestID);

  // Just do some simple checks:
  _ASSERTE(HashIdx < ALLOC_HASH_ENTRIES);

  pHashEntry = &AllocHashTable[HashIdx];
  while(pHashEntry != NULL) {
    if (pHashEntry->lRequestID == lRequestID) {
      return pHashEntry;
    }
    pHashEntry = pHashEntry->Next;
  }

  // entry was not found!
  return (AllocHashEntryType*) ALLOC_ENTRY_NOT_FOUND;
}  // AllocHashFind

// AllocHashRemove
//   Return: FALSE (0) : Key was found and removed/marked
//           TRUE (!=0): Key was not found
static BOOL AllocHashRemove(long lRequestID) {
  ULONG HashIdx;
  AllocHashEntryType *pHashEntry, *pHashEntryLast;

  // get the Hash-Value
  HashIdx = AllocHashFunction(lRequestID);

  // Just do some simple checks:
  _ASSERTE(HashIdx < ALLOC_HASH_ENTRIES);

  pHashEntryLast = NULL;
  pHashEntry = &AllocHashTable[HashIdx];
  while(pHashEntry != NULL) {
    if (pHashEntry->lRequestID == lRequestID) {
#ifdef HASH_ENTRY_REMOVE_AT_FREE
      AllocHashFreed++;
      AllocHashCurrentCount--;
      // release my memory
      if (pHashEntryLast == NULL) {
        // It is an entry in the table, so do not release this memory
        if (pHashEntry->Next == NULL) {
          // It was the last entry, so empty the table entry
          memset(&AllocHashTable[HashIdx], 0, sizeof(AllocHashTable[HashIdx]));
        }
        else {
          // There are some more entries, so shorten the list
          *pHashEntry = *(pHashEntry->Next);
          // TODO: Do I need to free the memory here !? I think I should do this...
        }
        return TRUE;
      }
      else {
        // now, I am in an dynamic allocated entry
        // it was a collision, so decrease the current collision count
        AllocHashCurrentCollisions--;
        pHashEntryLast->Next = pHashEntry->Next;
        free(pHashEntry);
        return TRUE;
      }
#else
      // increase the Remove-Count and let the objet stay in memory
      pHashEntry->cRemovedFlag++;
      return TRUE;
#endif
    }
    pHashEntryLast = pHashEntry;
    pHashEntry = pHashEntry->Next;
  }

  // if we are here, we could not find the RequestID
  return FALSE;
}

// ReadProcMemoryFromHash
//   Callback-Funtion for StackWalk for my own CallStack from the Hash-Table-Entries
#if API_VERSION_NUMBER >= 9
static BOOL __stdcall ReadProcMemoryFromHash(HANDLE hRequestID, DWORD lpBaseAddress, PVOID lpBuffer, DWORD nSize, PDWORD lpNumberOfBytesRead) {
#else
static BOOL __stdcall ReadProcMemoryFromHash(HANDLE hRequestID, LPCVOID lpBaseAddress, PVOID lpBuffer, DWORD nSize, PDWORD lpNumberOfBytesRead) {
#endif
  // Try to find the RequestID
  AllocHashEntryType *pHashEntry;
  *lpNumberOfBytesRead = 0;

  pHashEntry = AllocHashFind((LONG) hRequestID);
  if (pHashEntry == (AllocHashEntryType*) ALLOC_ENTRY_NOT_FOUND) {
    // Not found, so I cannot return any memory
    *lpNumberOfBytesRead = 0;
    return FALSE;
  }
  if ( ((DWORD) lpBaseAddress >= pHashEntry->dwESPOffset) && ((DWORD) lpBaseAddress <= (pHashEntry->dwESPOffset+pHashEntry->dwESPLen)) ) {
    // Memory is located in ESP:
    // Calculate the offset
    DWORD dwOffset = (DWORD) lpBaseAddress - pHashEntry->dwESPOffset;
    DWORD dwSize = __min(nSize, MAX_ESP_LEN_BUF-dwOffset);
    memcpy(lpBuffer, &(pHashEntry->pcESPAddr[dwOffset]), dwSize);
    *lpNumberOfBytesRead = dwSize;
    if (dwSize != nSize)
      return FALSE;
  }

  if ( ((DWORD) lpBaseAddress >= pHashEntry->dwEIPOffset) && ((DWORD) lpBaseAddress <= (pHashEntry->dwEIPOffset+pHashEntry->dwEIPLen)) ) {
    // Memory is located in EIP:
    // Calculate the offset
    DWORD dwOffset = (DWORD) lpBaseAddress - pHashEntry->dwEIPOffset;
    DWORD dwSize = __min(nSize, MAX_ESP_LEN_BUF-dwOffset);
    memcpy(lpBuffer, &(pHashEntry->pcEIPAddr[dwOffset]), dwSize);
    *lpNumberOfBytesRead = dwSize;
    if (dwSize != nSize)
      return FALSE;
  }
  
  if (*lpNumberOfBytesRead == 0)  // Memory could not be found
    return FALSE;

  return TRUE;
}

// AllocHashOutLeaks
// Write all Memory (with callstack) which was not freed yet
//   Returns the number of bytes, that are not freed (leaks)
ULONG AllocHashOutLeaks(FILE *fFile) {
  ULONG ulTemp;
  AllocHashEntryType *pHashEntry;
  ULONG ulCount = 0;
  ULONG ulLeaksByte = 0;

  // Move throu every entry
  for(ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++) {
    pHashEntry = &AllocHashTable[ulTemp];
    if (pHashEntry->lRequestID != 0) {
      while(pHashEntry != NULL) {
        if ( (pHashEntry->cRemovedFlag <= 0) || (pHashEntry->cRemovedFlag > 1) ) {
          ulCount++;
          if (g_CallstackOutputType == ACOutput_XML)
            _ftprintf(fFile, _T("<LEAK requestID=\"%u\" size=\"%u\">\n"), pHashEntry->lRequestID, pHashEntry->nDataSize);
          else
            _ftprintf(fFile, _T("RequestID: %12i, Removed: %i, Size: %12i\n"), pHashEntry->lRequestID, pHashEntry->cRemovedFlag, pHashEntry->nDataSize);
          CONTEXT c;
          memset( &c, '\0', sizeof c );
          c.Eip = pHashEntry->dwEIPOffset;
          c.Ebp = pHashEntry->dwESPOffset;
          ShowStackRM( NULL, c, fFile, &ReadProcMemoryFromHash, (HANDLE) pHashEntry->lRequestID);
          // Count the number of leaky bytes
          if (pHashEntry->nDataSize > 0)
            ulLeaksByte += pHashEntry->nDataSize;
          else
            ulLeaksByte++;  // If memory was allocated with zero bytes, then just increase the counter 1

          if (g_CallstackOutputType == ACOutput_XML)
            _ftprintf(fFile, _T("</LEAK>\n"));  // terminate the xml-node
        }
        pHashEntry = pHashEntry->Next;
      }
    }
  }
  if (g_CallstackOutputType != ACOutput_XML)
    _ftprintf(fFile, _T("\n**** Number of leaks: %i\n"), ulCount);
  return ulLeaksByte;
}  // AllocHashOutLeaks

// Write all used memory to a file
void AllocHashOut(FILE *fFile) {
  ULONG ulTemp;
  AllocHashEntryType *pHashEntry;

  for(ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++) {
    pHashEntry = &AllocHashTable[ulTemp];
    if (pHashEntry->lRequestID != 0) {
      while(pHashEntry != NULL) {
        if (g_CallstackOutputType == ACOutput_XML)
          _ftprintf(fFile, _T("<MEMUSED requestID=\"%u\" size=\"%u\"\n"), pHashEntry->lRequestID, pHashEntry->nDataSize);
        else
          _ftprintf(fFile, _T("RequestID: %12i, Removed: %i, Size: %12i\n"), pHashEntry->lRequestID, pHashEntry->cRemovedFlag, pHashEntry->nDataSize);
        pHashEntry = pHashEntry->Next;
      }
    }
  }
}  // AllocHashOut

/*******************************************************************************
 * Ende der Hash-Tabelle
 *******************************************************************************/


// The follwoing is copied from dbgint.h:
// <CRT_INTERNALS>
/*
 * For diagnostic purpose, blocks are allocated with extra information and
 * stored in a doubly-linked list.  This makes all blocks registered with
 * how big they are, when they were allocated, and what they are used for.
 */

#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
        struct _CrtMemBlockHeader * pBlockHeaderNext;
        struct _CrtMemBlockHeader * pBlockHeaderPrev;
        char *                      szFileName;
        int                         nLine;
#ifdef _WIN64
        /* These items are reversed on Win64 to eliminate gaps in the struct
         * and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
         * maintained in the debug heap.
         */
        int                         nBlockUse;
        size_t                      nDataSize;
#else  /* _WIN64 */
        size_t                      nDataSize;
        int                         nBlockUse;
#endif  /* _WIN64 */
        long                        lRequest;
        unsigned char               gap[nNoMansLandSize];
        /* followed by:
         *  unsigned char           data[nDataSize];
         *  unsigned char           anotherGap[nNoMansLandSize];
         */
} _CrtMemBlockHeader;
#define pbData(pblock) ((unsigned char *)((_CrtMemBlockHeader *)pblock + 1))
#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)
 
// </CRT_INTERNALS>




// Global data:
static BOOL g_bInitialized = FALSE;
static HINSTANCE g_hImagehlpDll = NULL;

static DWORD g_dwShowCount = 0;  // increase at every ShowStack-Call
static CRITICAL_SECTION g_csFileOpenClose = {0};

// Is used for syncronising call to MyAllocHook (to prevent reentrant calls)
static LONG g_lMallocCalled = 0;

static _CRT_ALLOC_HOOK pfnOldCrtAllocHook = NULL;

// Deaktivate AllocHook, by increasing the Syncronisation-Counter
static void DeactivateMallocStackwalker(void) {
  InterlockedIncrement(&g_lMallocCalled);
}


// MyAllocHook is Single-Threaded, that means the the calls are serialized in the calling function!
// Special case for VC 5
#if _MSC_VER <= 1100 
static int MyAllocHook(int nAllocType, void *pvData, 
      size_t nSize, int nBlockUse, long lRequest, 
      const char * szFileName, int nLine ) {
#else
static int MyAllocHook(int nAllocType, void *pvData, 
      size_t nSize, int nBlockUse, long lRequest, 
      const unsigned char * szFileName, int nLine ) {
#endif
  static TCHAR *operation[] = { _T(""), _T("ALLOCATIONG"), _T("RE-ALLOCATING"), _T("FREEING") };
  static TCHAR *blockType[] = { _T("Free"), _T("Normal"), _T("CRT"), _T("Ignore"), _T("Client") };

#ifdef IGNORE_CRT_ALLOC
  if (_BLOCK_TYPE(nBlockUse) == _CRT_BLOCK)  // Ignore internal C runtime library allocations
    return TRUE;
#endif
  extern int _crtDbgFlag;
  if  ( ((_CRTDBG_ALLOC_MEM_DF & _crtDbgFlag) == 0) && ( (nAllocType == _HOOK_ALLOC) || (nAllocType == _HOOK_REALLOC) ) )
  {
    // Someone has disabled that the runtime should log this allocation
    // so we do not log this allocation
    if (pfnOldCrtAllocHook != NULL)
      pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    return TRUE;
  }

  // Prevent from reentrat calls
  if (InterlockedIncrement(&g_lMallocCalled) > 1) { // I was already called
    InterlockedDecrement(&g_lMallocCalled);
    // call the previous alloc hook
    if (pfnOldCrtAllocHook != NULL)
      pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    return TRUE;
  }

  if (g_ulShowStackAtAlloc > 0) {
    AllocCheckFileOpen();  // Open logfile
  }

   _ASSERT( (nAllocType == _HOOK_ALLOC) || (nAllocType == _HOOK_REALLOC) || (nAllocType == _HOOK_FREE) );
   _ASSERT( ( _BLOCK_TYPE(nBlockUse) >= 0 ) && ( _BLOCK_TYPE(nBlockUse) < 5 ) );

  if (nAllocType == _HOOK_FREE) { // freeing
    // Try to get the header information
    if (_CrtIsValidHeapPointer(pvData)) {  // it is a valid Heap-Pointer
      // get the ID
      _CrtMemBlockHeader *pHead;
      // get a pointer to memory block header
      pHead = pHdr(pvData);
      nSize = pHead->nDataSize;
      lRequest = pHead->lRequest; // This is the ID!

      if (pHead->nBlockUse == _IGNORE_BLOCK)
      {
        InterlockedDecrement(&g_lMallocCalled);
        if (pfnOldCrtAllocHook != NULL)
          pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
        return TRUE;
      }
    }
  }

  if (g_ulShowStackAtAlloc > 0) {
    _ftprintf( g_fFile, _T("##### Memory operation: %s a %d-byte '%s' block (# %ld)"),
      operation[nAllocType], nSize, blockType[_BLOCK_TYPE(nBlockUse)], lRequest );
    if ( pvData != NULL )
      _ftprintf( g_fFile, _T(" at 0x%X"), pvData );
    _ftprintf(g_fFile, _T("\n"));
  }

  if (nAllocType == _HOOK_FREE) { // freeing:
    if (lRequest != 0) {  // RequestID was found
      BOOL bRet;
      // Try to find the RequestID in the Hash-Table, mark it that it was freed
      bRet = AllocHashRemove(lRequest);
      if(g_ulShowStackAtAlloc > 0) {
        if (bRet == FALSE) {
          // RequestID not found!
          _ftprintf(g_fFile, _T("###### RequestID not found in hash table for FREEING (%i)!\n"), lRequest);
        }
      } // g_ulShowStackAtAlloc > 0
    }
    else {
      if(g_ulShowStackAtAlloc > 0) {
      // No valid RequestID found, display error
      _ftprintf(g_fFile, _T("###### No valid RequestID for FREEING! (0x%X)\n"), pvData);
 
      }
    }
  }  // freeing

  if (nAllocType == _HOOK_REALLOC) { // re-allocating
    // Try to get the header information
    if (_CrtIsValidHeapPointer(pvData)) {  // it is a valid Heap-Pointer
      BOOL bRet;
      LONG lReallocRequest;
      // get the ID
      _CrtMemBlockHeader *pHead;
      // get a pointer to memory block header
      pHead = pHdr(pvData);
      // Try to find the RequestID in the Hash-Table, mark it that it was freed
      lReallocRequest = pHead->lRequest;
      bRet = AllocHashRemove(lReallocRequest);
      if (g_ulShowStackAtAlloc > 0) {
        if (bRet == FALSE) {
          // RequestID not found!
          _ftprintf(g_fFile, _T("###### RequestID not found in hash table for RE-ALLOCATING (%i)!\n"), lReallocRequest);
        }
        else {
          _ftprintf(g_fFile, _T("##### Implicit freeing because of re-allocation (# old: %ld, new: %ld)\n"), lReallocRequest, lRequest);
        }
      }  // g_ulShowStackAtAlloc > 0
    }  // ValidHeapPointer
  }  // re-allocating

  if ( (g_ulShowStackAtAlloc < 3) && (nAllocType == _HOOK_FREE) ) {
    InterlockedDecrement(&g_lMallocCalled);
    // call the previous alloc hook
    if (pfnOldCrtAllocHook != NULL)
      pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    return TRUE;
  }

  // Get the context DIESES of this Thread
  HANDLE hThread;
  if (DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
    GetCurrentProcess(), &hThread, 0, false, DUPLICATE_SAME_ACCESS ) == 0) {
      // Something was wrong...
      _ftprintf(g_fFile, _T("###### Could not call 'DuplicateHandle' successfully\n"));
      InterlockedDecrement(&g_lMallocCalled);
      // call the previous alloc hook
      if (pfnOldCrtAllocHook != NULL)
        pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
      return TRUE;
  }

  CONTEXT c;
  memset( &c, '\0', sizeof c );
  c.ContextFlags = CONTEXT_FULL;

  // init CONTEXT record so we know where to start the stackwalk
  if ( GetThreadContext( hThread, &c )  == 0) {
    if(g_ulShowStackAtAlloc > 1) {
      _ftprintf(g_fFile, _T("###### Could not call 'GetThreadContext' successfully\n"));
    }
    InterlockedDecrement(&g_lMallocCalled);
    // call the previous alloc hook
    if (pfnOldCrtAllocHook != NULL)
      pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    CloseHandle(hThread);
    return TRUE; // could not get context
  }

  if(g_ulShowStackAtAlloc > 1) {
    if(g_ulShowStackAtAlloc > 2) {
      // output the callstack
      ShowStack( hThread, c, g_fFile);
    }
    else {
      // Output only (re)allocs
      if (nAllocType != _HOOK_FREE) {
        ShowStack( hThread, c, g_fFile);
      }
    }
  }  // g_ulShowStackAtAlloc > 1
  CloseHandle( hThread );

  // Only isert in the Hash-Table if it is not a "freeing"
  if (nAllocType != _HOOK_FREE) {
    if(lRequest != 0) // Always a valid RequestID should be provided (see comments in the header)
      AllocHashInsert(lRequest, c, nSize);
  }

  InterlockedDecrement(&g_lMallocCalled);
  // call the previous alloc hook
  if (pfnOldCrtAllocHook != NULL)
    pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
  return TRUE; // allow the memory operation to proceed
}




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
  HINSTANCE hToolhelp;
  tCT32S pCT32S;
  tM32F pM32F;
  tM32N pM32N;

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

// This function if NOT multi-threading capable
// It should only be called from the main-Function!
int InitAllocCheckWN(eAllocCheckOutput eOutput, LPCTSTR pszFileName, ULONG ulShowStackAtAlloc) {
  if (g_bInitialized) {
    return 2;  // already initialized!
  }
  if (ulShowStackAtAlloc <= 3)
    g_ulShowStackAtAlloc = ulShowStackAtAlloc;
  else
    g_ulShowStackAtAlloc = 0;

  if (pszFileName != NULL) 
    g_pszAllocLogName = _tcsdup(pszFileName);
  else
    g_pszAllocLogName = NULL;

  g_CallstackOutputType = eOutput;

#ifdef _DEBUG
  AllocHashInit();

#ifdef WITH_IMALLOC_SPY
  HRESULT hr;
  // erzeuge mein malloc-Spy object
  LPMALLOCSPY pMallocSpy = new CMallocSpy(); // wird später durch Release freigegeben
  if (pMallocSpy != NULL)
  {
    // CoInitilize(); // ??? Ist dies notwendig ?
    hr = CoRegisterMallocSpy(pMallocSpy);
    if FAILED(hr)
    {
      _tprintf(_T("\nCoRegisterMallocSpay failed with %.8x"), hr);
    }
  }
#endif

  // save the previous alloc hook
  pfnOldCrtAllocHook = _CrtSetAllocHook(MyAllocHook);
#endif

  return InitStackWalk();
}  // InitAllocCheckWN

static TCHAR s_szExceptionLogFileName[_MAX_PATH] = _T("\\exceptions.log");  // default
static BOOL s_bUnhandledExeptionFilterSet = FALSE;
static LONG __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
   LONG lRet;
   lRet = StackwalkFilter(pExPtrs, /*EXCEPTION_CONTINUE_SEARCH*/EXCEPTION_EXECUTE_HANDLER, s_szExceptionLogFileName);
   TCHAR lString[500];
   _stprintf(lString,
      _T("*** Unhandled Exception!\n")
      _T("   ExpCode: 0x%8.8X\n")
      _T("   ExpFlags: %d\n")
      _T("   ExpAddress: 0x%8.8X\n")
      _T("   Please report!"),
      pExPtrs->ExceptionRecord->ExceptionCode,
      pExPtrs->ExceptionRecord->ExceptionFlags,
      pExPtrs->ExceptionRecord->ExceptionAddress);
   FatalAppExit(-1,lString);
   return lRet;
}

int InitAllocCheck(eAllocCheckOutput eOutput, BOOL bSetUnhandledExeptionFilter, ULONG ulShowStackAtAlloc)  // will create the filename by itself
{
  TCHAR szModName[_MAX_PATH];
  if (GetModuleFileName(NULL, szModName, sizeof(szModName)/sizeof(TCHAR)) != 0)
  {
    _tcscpy(s_szExceptionLogFileName, szModName);
    if (eOutput == ACOutput_XML)
      _tcscat(s_szExceptionLogFileName, _T(".exp.xml"));
    else
      _tcscat(s_szExceptionLogFileName, _T(".exp.log"));

    if (eOutput == ACOutput_XML)
      _tcscat(szModName, _T(".mem.xml-leaks"));
    else
      _tcscat(szModName, _T(".mem.log"));
  }
  else
  {
    if (eOutput == ACOutput_XML)
      _tcscpy(szModName, _T("\\mem-leaks.xml-leaks"));  // default
    else
      _tcscpy(szModName, _T("\\mem-leaks.log"));  // default
  }

  if (bSetUnhandledExeptionFilter != FALSE)
  {
    // set global exception handler (for handling all unhandled exceptions)
    SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
    s_bUnhandledExeptionFilterSet = TRUE;
  }

  return InitAllocCheckWN(eOutput, szModName, ulShowStackAtAlloc);
}


// This function if NOT multi-threading capable
// It should only be called from the main-Function!
//   Returns the number of bytes that are not freed (leaks)
ULONG DeInitAllocCheck(void) {
  ULONG ulRet = 0;
  if (g_bInitialized) {

#ifdef _DEBUG
    InterlockedIncrement(&g_lMallocCalled); // No deactivate MyAllocHook, because StackWalker will allocate some memory)
    ulRet = AllocHashDeinit();  // output the not freed memory
    // remove the hook and set the old one
    _CrtSetAllocHook(pfnOldCrtAllocHook);

#ifdef WITH_IMALLOC_SPY
    CoRevokeMallocSpy();
#endif

#endif

    EnterCriticalSection(&g_csFileOpenClose);  // wait until a running stack dump was created
    g_bInitialized = FALSE;

    // de-init symbol handler etc. (SymCleanup())
    if (pSC != NULL)
      pSC( GetCurrentProcess() );
    FreeLibrary( g_hImagehlpDll );

    LeaveCriticalSection(&g_csFileOpenClose);
    if (g_pszAllocLogName != NULL) {
      free(g_pszAllocLogName);
      g_pszAllocLogName = NULL;
    }
    if (g_fFile != NULL) {
       fclose(g_fFile);
       g_fFile = NULL;
    }

    DeleteCriticalSection(&g_csFileOpenClose);
    InterlockedDecrement(&g_lMallocCalled);
  }

  if (s_bUnhandledExeptionFilterSet != TRUE)
  {
    SetUnhandledExceptionFilter(NULL);
    s_bUnhandledExeptionFilterSet = FALSE;
  }
  return ulRet;
}  // DeInitAllocCheck




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
  FILE *fFile = stdout;  // default to stdout

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
    fFile = stdout;
  }

  // Write infos about the exception
  if (g_CallstackOutputType == ACOutput_XML)
  {
    _ftprintf(fFile, _T("<EXCEPTION code=\"%8.8X\" addr=\"%8.8X\" "), 
      ep->ExceptionRecord->ExceptionCode,
      ep->ExceptionRecord->ExceptionAddress);
    WriteDateTime(fFile, TRUE);
    _ftprintf(fFile, _T("code_desc=\"%s\" more_desc=\"%s\">\n"), GetExpectionCodeText(ep->ExceptionRecord->ExceptionCode),
      GetAdditionalExpectionCodeText(ep->ExceptionRecord));
  }
  else
  {
    _ftprintf(fFile, _T("######## EXCEPTION: 0x%8.8X at address: 0x%8.8X"), 
      ep->ExceptionRecord->ExceptionCode,
      ep->ExceptionRecord->ExceptionAddress);
    _ftprintf(fFile, _T(": %s %s\n"), GetExpectionCodeText(ep->ExceptionRecord->ExceptionCode),
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
  FILE *fFile = stdout;  // default to stdout

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
    fFile = stdout;
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

  static bFirstTime = TRUE;

  // If no logfile is present, outpur to "stdout"
  if (fLogFile == NULL) {
    fLogFile = stdout;
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
