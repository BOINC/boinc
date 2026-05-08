// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include "stdafx.h"
#include "win_util.h"
#include "terminate.h"

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
    LONG      Priority;
    LONG      BasePriority;
    ULONG          ContextSwitchCount;
    LONG           State;
    LONG           WaitReason;
} SYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES {
    ULONG          NextEntryDelta;
    ULONG          ThreadCount;
    ULONG          Reserved1[6];
    LARGE_INTEGER  CreateTime;
    LARGE_INTEGER  UserTime;
    LARGE_INTEGER  KernelTime;
    UNICODE_STRING ProcessName;
    LONG      BasePriority;
    ULONG pad1;
    ULONG          ProcessId;
    ULONG pad2;
    ULONG          InheritedFromProcessId;
    ULONG pad3, pad4, pad5;
    ULONG          HandleCount;
    ULONG          Reserved2[2];
    VM_COUNTERS    VmCounters;
    IO_COUNTERS    IoCounters;
    SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;


// NtQuerySystemInformation
typedef NTSTATUS(WINAPI* tNTQSI)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
    );

typedef struct _BOINC_PROCESS {
    DWORD               dwProcessId;
    DWORD               dwParentProcessId;
    tstring             strProcessName;
} BOINC_PROCESS;


static PVOID diagnostics_get_process_information() {
    constexpr auto NT_SUCCESS = [](auto Status) {
        return Status >= 0;
        };

    constexpr auto STATUS_INFO_LENGTH_MISMATCH =
        static_cast<NTSTATUS>(0xC0000004L);

    constexpr ULONG SystemProcessAndThreadInformation = 5;

    auto hNTDllLib = GetModuleHandle(_T("ntdll.dll"));
    if (hNTDllLib == nullptr) {
        return nullptr;
    }

    auto pNTQSI = reinterpret_cast<tNTQSI>(
        GetProcAddress(hNTDllLib, "NtQuerySystemInformation"));
    if (pNTQSI == nullptr) {
        return nullptr;
    }

    ULONG cbBuffer = 128 * 1024; // 128k initial buffer
    auto Status = STATUS_INFO_LENGTH_MISMATCH;
    PVOID pBuffer = nullptr;
    auto hHeap = GetProcessHeap();
    if (hHeap == nullptr) {
        return nullptr;
    }

    do {
        pBuffer = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbBuffer);
        if (pBuffer == nullptr) {
            return nullptr;
        }

        Status = pNTQSI(SystemProcessAndThreadInformation,
            pBuffer, cbBuffer, &cbBuffer);

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            HeapFree(hHeap, 0, pBuffer);
            cbBuffer *= 2;
        }
        else if (!NT_SUCCESS(Status)) {
            HeapFree(hHeap, 0, pBuffer);
        }
    } while (Status == STATUS_INFO_LENGTH_MISMATCH);

    return pBuffer;
}


static std::vector<BOINC_PROCESS> diagnostics_update_process_list() {
    // Get a snapshot of the process and thread information.
    auto pBuffer = diagnostics_get_process_information();
    if (pBuffer == nullptr) {
        return {};
    }

    // Lets start walking the structures to find the good stuff.
    auto pProcesses = reinterpret_cast<PSYSTEM_PROCESSES>(pBuffer);
    std::vector<BOINC_PROCESS> ps;
    do {
        if (pProcesses->ProcessId) {
            BOINC_PROCESS pi;
            pi.dwProcessId = pProcesses->ProcessId;
            pi.dwParentProcessId = pProcesses->InheritedFromProcessId;
            pi.strProcessName = pProcesses->ProcessName.Buffer;
            ps.emplace_back(pi);
        }

        // Move to the next structure if one exists
        if (!pProcesses->NextEntryDelta) {
            break;
        }
        pProcesses = reinterpret_cast<PSYSTEM_PROCESSES>(((
            reinterpret_cast<LPBYTE>(pProcesses)) +
            pProcesses->NextEntryDelta));
    } while (pProcesses);

    // Release resources
    if (pBuffer) {
        HeapFree(GetProcessHeap(), 0, pBuffer);
    }

    return ps;
}

void TerminateProcess::TerminateProcessEx(
    const tstring& strProcessName, bool bRecursive) {
    constexpr auto downcase_string = [](auto str) {
        std::transform(str.cbegin(), str.cend(), str.begin(),
            [](auto c) {
                return static_cast<wchar_t>(std::tolower(c));
            });
        return str;
        };

    // Get a list of currently executing processes.
    const auto ps = diagnostics_update_process_list();

    std::vector<BOINC_PROCESS> tps;
    // Find our root process that we are supposed to terminate and
    //   terminate it.
    const auto strProcessNameLower = downcase_string(strProcessName);
    for (const auto& p : ps) {
        if (downcase_string(p.strProcessName) == strProcessNameLower) {
            if (TerminateProcessById(p.dwProcessId)) {
                if (bRecursive) {
                    tps.emplace_back(p);
                }
            }
        }
    }

    if (!bRecursive) {
        return;
    }

    // Terminate all child processes
    for (const auto& tp : tps) {
        for (const auto& p : ps) {
            if (tp.dwProcessId == p.dwParentProcessId) {
                if (TerminateProcessById(p.dwProcessId)) {
                    tps.emplace_back(p);
                }
            }
        }
    }
}
