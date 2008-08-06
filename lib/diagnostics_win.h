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

#ifndef _BOINC_DIAGNOSTICS_WIN_
#define _BOINC_DIAGNOSTICS_WIN_

#define NT_SUCCESS(Status)                      ((NTSTATUS)(Status) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH             ((NTSTATUS)0xC0000004L)
#define SystemProcessAndThreadInformation       5

typedef LONG       NTSTATUS;

typedef LONG       KPRIORITY;

typedef struct _CLIENT_ID {
    DWORD          UniqueProcess;
    DWORD          UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING {
    USHORT         Length;
    USHORT         MaximumLength;
    PWSTR          Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
#ifdef _WIN64
// the following was inferred by painful reverse engineering
	SIZE_T		   PeakVirtualSize;	// not actually
    SIZE_T         PageFaultCount;
    SIZE_T         PeakWorkingSetSize;
    SIZE_T         WorkingSetSize;
    SIZE_T         QuotaPeakPagedPoolUsage;
    SIZE_T         QuotaPagedPoolUsage;
    SIZE_T         QuotaPeakNonPagedPoolUsage;
    SIZE_T         QuotaNonPagedPoolUsage;
    SIZE_T         PagefileUsage;
    SIZE_T         PeakPagefileUsage;
    SIZE_T         VirtualSize;		// not actually
#else
    SIZE_T         PeakVirtualSize;
    SIZE_T         VirtualSize;
    ULONG          PageFaultCount;
    SIZE_T         PeakWorkingSetSize;
    SIZE_T         WorkingSetSize;
    SIZE_T         QuotaPeakPagedPoolUsage;
    SIZE_T         QuotaPagedPoolUsage;
    SIZE_T         QuotaPeakNonPagedPoolUsage;
    SIZE_T         QuotaNonPagedPoolUsage;
    SIZE_T         PagefileUsage;
    SIZE_T         PeakPagefileUsage;
#endif
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER  KernelTime;
    LARGE_INTEGER  UserTime;
    LARGE_INTEGER  CreateTime;
    ULONG          WaitTime;
    PVOID          StartAddress;
    CLIENT_ID      ClientId;
    KPRIORITY      Priority;
    KPRIORITY      BasePriority;
    ULONG          ContextSwitchCount;
    LONG           State;
    LONG           WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES_NT4 {
    ULONG          NextEntryDelta;
    ULONG          ThreadCount;
    ULONG          Reserved1[6];
    LARGE_INTEGER  CreateTime;
    LARGE_INTEGER  UserTime;
    LARGE_INTEGER  KernelTime;
    UNICODE_STRING ProcessName;
    KPRIORITY      BasePriority;
    ULONG          ProcessId;
    ULONG          InheritedFromProcessId;
    ULONG          HandleCount;
    ULONG          Reserved2[2];
    VM_COUNTERS    VmCounters;
    SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES_NT4, *PSYSTEM_PROCESSES_NT4;

typedef struct _SYSTEM_PROCESSES {
    ULONG          NextEntryDelta;
    ULONG          ThreadCount;
    ULONG          Reserved1[6];
    LARGE_INTEGER  CreateTime;
    LARGE_INTEGER  UserTime;
    LARGE_INTEGER  KernelTime;
    UNICODE_STRING ProcessName;
    KPRIORITY      BasePriority;
#ifdef _WIN64
	ULONG pad1;
    ULONG          ProcessId;
	ULONG pad2;
    ULONG          InheritedFromProcessId;
	ULONG pad3, pad4, pad5;
#else
    ULONG          ProcessId;
    ULONG          InheritedFromProcessId;
#endif
    ULONG          HandleCount;
    ULONG          Reserved2[2];
    VM_COUNTERS    VmCounters;
    IO_COUNTERS    IoCounters;
    SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;

typedef enum _THREAD_STATE {
    ThreadStateInitialized,
    ThreadStateReady,
    ThreadStateRunning,
    ThreadStateStandby,
    ThreadStateTerminated,
    ThreadStateWaiting,
    ThreadStateTransition
} THREAD_STATE, *PTHREAD_STATE;

typedef enum _THREAD_WAIT_REASON {
    ThreadWaitReasonExecutive,
    ThreadWaitReasonFreePage,
    ThreadWaitReasonPageIn,
    ThreadWaitReasonPoolAllocation,
    ThreadWaitReasonDelayExecution,
    ThreadWaitReasonSuspended,
    ThreadWaitReasonUserRequest,
    ThreadWaitReasonWrExecutive,
    ThreadWaitReasonWrFreePage,
    ThreadWaitReasonWrPageIn,
    ThreadWaitReasonWrPoolAllocation,
    ThreadWaitReasonWrDelayExecution,
    ThreadWaitReasonWrSuspended,
    ThreadWaitReasonWrUserRequest,
    ThreadWaitReasonWrEventPairHigh,
    ThreadWaitReasonWrEventPairLow,
    ThreadWaitReasonWrLpcReceive,
    ThreadWaitReasonWrLpcReply,
    ThreadWaitReasonWrVirtualMemory,
    ThreadWaitReasonWrPageOut,
    ThreadWaitReasonMaximumWaitReason
} THREAD_WAIT_REASON;


// Delay Load Error Handling stuff
#ifndef _DELAY_IMP_VER

#define FACILITY_VISUALCPP  ((LONG)0x6d)
#define VcppException(sev,err)  ((sev) | (FACILITY_VISUALCPP<<16) | err)

typedef DWORD  RVA;

typedef struct ImgDelayDescr {
    DWORD           grAttrs;        // attributes
    RVA             rvaDLLName;     // RVA to dll name
    RVA             rvaHmod;        // RVA of module handle
    RVA             rvaIAT;         // RVA of the IAT
    RVA             rvaINT;         // RVA of the INT
    RVA             rvaBoundIAT;    // RVA of the optional bound IAT
    RVA             rvaUnloadIAT;   // RVA of optional copy of original IAT
    DWORD           dwTimeStamp;    // 0 if not bound,
                                    // O.W. date/time stamp of DLL bound to (Old BIND)
} ImgDelayDescr, *PImgDelayDescr;

typedef const ImgDelayDescr *PCImgDelayDescr;

typedef struct DelayLoadProc {
    BOOL                fImportByName;
    union {
        LPCSTR          szProcName;
        DWORD           dwOrdinal;
        };
} DelayLoadProc;

typedef struct DelayLoadInfo {
    DWORD               cb;         // size of structure
    PCImgDelayDescr     pidd;       // raw form of data (everything is there)
    FARPROC *           ppfn;       // points to address of function to load
    LPCSTR              szDll;      // name of dll
    DelayLoadProc       dlp;        // name or ordinal of procedure
    HMODULE             hmodCur;    // the hInstance of the library we have loaded
    FARPROC             pfnCur;     // the actual function that will be called
    DWORD               dwLastError;// error received (if an error notification)
} DelayLoadInfo, * PDelayLoadInfo;

#endif

#endif
