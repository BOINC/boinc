// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#ifndef BOINC_DIAGNOSTICS_WIN_H
#define BOINC_DIAGNOSTICS_WIN_H

#include "boinc_win.h"

#define STATUS_INFO_LENGTH_MISMATCH             ((NTSTATUS)0xC0000004L)

typedef LONG       NTSTATUS;

typedef LONG       KPRIORITY;

//MinGW-W64 defines this struct in its own header
#if !defined(HAVE_CLIENT_ID) && !defined(__MINGW32__) && _MSC_VER <= 1800
typedef struct _CLIENT_ID {
    HANDLE         UniqueProcess;
    HANDLE         UniqueThread;
} CLIENT_ID;
#endif

//MinGW-W64 defines this struct in its own header
#if !defined(HAVE_VM_COUNTERS) && !defined(__MINGW32__)
// https://learn.microsoft.com/windows/win32/api/winternl/nf-winternl-ntquerysysteminformation
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
    SIZE_T         PrivatePageCount;
} VM_COUNTERS;
#endif

//MinGW-W64 defines this struct in its own header
#if !defined(HAVE_SYSTEM_THREADS) && !defined(__MINGW32__)
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
#endif

#ifndef HAVE_SYSTEM_PROCESSES
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
	ULONG pad3;
#else
    ULONG          ProcessId;
    ULONG          InheritedFromProcessId;
#endif
    ULONG          HandleCount;
    ULONG          SessionId;
    PVOID          Reserved3;
    VM_COUNTERS    VmCounters;
    IO_COUNTERS    IoCounters;
    SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;
#endif

//MinGW-W64 defines this struct in its own header
#if !defined(HAVE_THREAD_STATE) && !defined(__MINGW32__)
typedef enum _THREAD_STATE {
    StateInitialized,
    StateReady,
    StateRunning,
    StateStandby,
    StateTerminated,
    StateWait,
    StateTransition
} THREAD_STATE, *PTHREAD_STATE;
#endif

#ifndef HAVE_THREAD_WAIT_REASON
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
#endif

// older mingw versions (before 2012-07-12) do not define this in winternl.h
#if defined(__MINGW32__)
#ifndef NT_SUCCESS
#define NT_SUCCESS(status)     ((NTSTATUS) (status) >= 0)
#endif
#endif

#endif
