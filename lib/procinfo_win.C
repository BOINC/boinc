#include "diagnostic_win.h"
#include "procinfo.h"

static int get_process_information(PVOID* ppBuffer, PULONG pcbBuffer) {
    int      retval = 0;
    NTSTATUS Status = STATUS_INFO_LENGTH_MISMATCH;
    HANDLE   hHeap  = GetProcessHeap();
    HMODULE  hNTDllLib = NULL;
    tNTQSI   pNTQSI = NULL;

    hNTDllLib = GetModuleHandle("ntdll.dll");
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

int get_procinfo_XP(vector<PROCINFO>& pi) {
    ULONG                   cbBuffer = 32*1024;    // 32k initial buffer
    PVOID                   pBuffer = NULL;
    PSYSTEM_PROCESSES       pProcesses = NULL;
    PSYSTEM_THREADS         pThread = NULL;
    HMODULE                 hKernel32Lib;
    tOT                     pOT = NULL;

    hKernel32Lib = GetModuleHandle("kernel32.dll");
    pOT = (tOT) GetProcAddress( hKernel32Lib, "OpenThread" );

    get_process_information(&pBuffer, &cbBuffer);
    pProcesses = (PSYSTEM_PROCESSES)pBuffer;
    while (pProcesses) {
        PROCINFO p;
        p.virtual_size = pProcesses->VmCounters.VirtualSize;
        p.working_set_size = pProcesses->VmCounters.WorkingSetSize;
        p.user_time = (double) pProcesses->UserTime.QuadPart;
        p.kernel_time = (double) pProcesses->KernelTime.QuadPart;
        pi.push_back(p);

        if (!pProcesses->NextEntryDelta) {
            break;
        }
        pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses) + pProcesses->NextEntryDelta);
    }

    if (pBuffer) HeapFree(GetProcessHeap(), NULL, pBuffer);
    return 0;
}
int get_procinfo_NT(vector<PROCINFO>& pi) {
    return 0;
}
int get_procinfo_9X(vector<PROCINFO>& pi) {
    return 0;
}

int get_procinfo(vector<PROCINFO>& pi) {
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    pi.clear();

    switch(osvi.dwPlatformId) {
    case VER_PLATFORM_WIN32_WINDOWS:
        // Win95, Win98, WinME
        return get_procinfo_9x(pi);
    case VER_PLATFORM_WIN32_NT:
        switch(osvi.dwMajorVersion) {
        case 4:
            // WinNT 4.0
            return get_procinfo_NT(pi);
        case 5:
            // Win2k, WinXP, Win2k3
            return get_procinfo_XP(pi);
        case 6:
            if (osvi.dwMinorVersion == 0) {
                // WinVista
                return get_procinfo_XP(pi);
            } else {
                return get_procinfo_9x(pi);
            }
        default:
            return get_procinfo_9x(pi);
        }
    }
    return get_procinfo_9x(pi);
}
