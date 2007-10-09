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

#include "stdafx.h"
#include "terminate.h"


// NtQuerySystemInformation
typedef NTSTATUS (WINAPI *tNTQSI)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);


// Provide a structure to store process measurements at the time of a
//   crash.
typedef struct _BOINC_PROCESS {
    DWORD               dwProcessId;
    DWORD               dwParentProcessId;
    tstring             strProcessName;
} BOINC_PROCESS, *PBOINC_PROCESS;


int diagnostics_get_process_information(PVOID* ppBuffer, PULONG pcbBuffer) {
    int      retval = 0;
    NTSTATUS Status = STATUS_INFO_LENGTH_MISMATCH;
    HANDLE   hHeap  = GetProcessHeap();
    HMODULE  hNTDllLib = NULL;
    tNTQSI   pNTQSI = NULL;

    hNTDllLib = GetModuleHandle(_T("ntdll.dll"));
    pNTQSI = (tNTQSI)GetProcAddress(hNTDllLib, "NtQuerySystemInformation");

    do {
        *ppBuffer = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *pcbBuffer);
        if (ppBuffer == NULL) {
            retval = ERROR_NOT_ENOUGH_MEMORY;
        }

        Status = pNTQSI(
            SystemProcessAndThreadInformation,
            *ppBuffer,
            *pcbBuffer,
            pcbBuffer
        );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            HeapFree(hHeap, NULL, *ppBuffer);
            *pcbBuffer *= 2;
        } else if (!NT_SUCCESS(Status)) {
            HeapFree(hHeap, NULL, *ppBuffer);
            retval = Status;
        }
    } while (Status == STATUS_INFO_LENGTH_MISMATCH);

    return retval;
}


int diagnostics_update_process_list( std::vector<BOINC_PROCESS>& ps ) {
    ULONG                   cbBuffer = 128*1024;    // 128k initial buffer
    PVOID                   pBuffer = NULL;
    PSYSTEM_PROCESSES       pProcesses = NULL;

    // Clear out the vector
    ps.clear();

    // Get a snapshot of the process and thread information.
    diagnostics_get_process_information(&pBuffer, &cbBuffer);

    // Lets start walking the structures to find the good stuff.
    pProcesses = (PSYSTEM_PROCESSES)pBuffer;
    do {

        if (pProcesses->ProcessId) {
            BOINC_PROCESS pi;
            pi.dwProcessId = pProcesses->ProcessId;
            pi.dwParentProcessId = pProcesses->InheritedFromProcessId;
            pi.strProcessName = pProcesses->ProcessName.Buffer;
            ps.push_back(pi);
        }

        // Move to the next structure if one exists
        if (!pProcesses->NextEntryDelta) {
            break;
        }
        pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses) + pProcesses->NextEntryDelta);
    } while (pProcesses);

    // Release resources
    if (pBuffer) HeapFree(GetProcessHeap(), NULL, pBuffer);

    return 0;
}


tstring downcase_string(tstring& orig) {
    tstring retval = orig;
    for (size_t i=0; i < retval.length(); i++) {
        retval[i] = tolower(retval[i]);
    }
    return retval;
}


BOOL TerminateProcessById( DWORD dwProcessID ) {
    HANDLE hProcess;
    BOOL bRetVal = FALSE;

    hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, dwProcessID );

    if (hProcess) {
        bRetVal = TerminateProcess(hProcess, 1);
    }

    CloseHandle( hProcess );

    return bRetVal;
}


BOOL TerminateProcessEx( tstring& strProcessName ) {
	unsigned int i,j;
    std::vector<BOINC_PROCESS> ps;
    std::vector<BOINC_PROCESS> tps;

    // Get a list of currently executing processes.
    diagnostics_update_process_list(ps);


    // Find our root process that we are supposed to terminate and
    //   terminate it.
	for (i=0; i < ps.size(); i++) {
		BOINC_PROCESS& p = ps[i];
        if (downcase_string(p.strProcessName) == downcase_string(strProcessName)) {
            if (TerminateProcessById(p.dwProcessId)) {
                tps.push_back(p);
            }
        }
	}


    // Terminate all child processes
	for (i=0; i < tps.size(); i++) {
		BOINC_PROCESS tp = tps[i];
	    for (j=0; j < ps.size(); j++) {
		    BOINC_PROCESS p = ps[j];
            if (tp.dwProcessId == p.dwParentProcessId) {
                if (TerminateProcessById(p.dwProcessId)) {
                    tps.push_back(p);
                }
            }
	    }
	}

    return TRUE;
}

