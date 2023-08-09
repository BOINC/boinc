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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef _BOINC_TERMINATE_
#define _BOINC_TERMINATE_

#define NT_SUCCESS(Status)                      ((NTSTATUS)(Status) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH             ((NTSTATUS)0xC0000004L)
#define SystemProcessAndThreadInformation       5

typedef LONG       NTSTATUS;

typedef LONG       KPRIORITY;

typedef struct _CLIENT_ID {
    DWORD          UniqueProcess;
    DWORD          UniqueThread;
} CLIENT_ID;

typedef struct _VM_COUNTERS {
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


// Prototypes
BOOL TerminateProcessEx( tstring& strProcessName, bool bRecursive = true );


#endif

