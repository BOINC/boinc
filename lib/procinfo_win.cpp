// much of this code is public-domain
//
#include "boinc_win.h"
#include "error_numbers.h"
#include "diagnostics_win.h"
#include "str_util.h"
#include "str_replace.h"

#include "procinfo.h"

using std::vector;

// NtQuerySystemInformation
typedef NTSTATUS (WINAPI *tNTQSI)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

static int get_process_information(PVOID* ppBuffer, PULONG pcbBuffer) {
    NTSTATUS Status = STATUS_INFO_LENGTH_MISMATCH;
    HANDLE   hHeap  = GetProcessHeap();
    HMODULE  hNTDllLib = GetModuleHandle(_T("ntdll.dll"));
    tNTQSI   pNTQSI = (tNTQSI)GetProcAddress(hNTDllLib, "NtQuerySystemInformation");
    ULONG    cbBuffer = 0;

    while (1) {
        // Store the buffer size since it appears that somebody is monkeying around
        //   with the return values on some systems.
        cbBuffer = *pcbBuffer;

        *ppBuffer = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *pcbBuffer);
        if (*ppBuffer == NULL) {
            return ERR_MALLOC;
        }

        Status = pNTQSI(
            SystemProcessAndThreadInformation,
            *ppBuffer,
            *pcbBuffer,
            pcbBuffer
        );

        if (*pcbBuffer < cbBuffer) {
            // Somebody is trying to screw us up,
            // so set the value back to the cached size
            // so we can do something smart like increase the buffer size.
            *pcbBuffer = cbBuffer;
        }

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            HeapFree(hHeap, NULL, *ppBuffer);
            *pcbBuffer *= 2;
        } else if (!NT_SUCCESS(Status)) {
            HeapFree(hHeap, NULL, *ppBuffer);
            return ERR_GETRUSAGE;
		} else {
			return 0;
		}
    }
    return 0;	// never reached
}

// Note: the following will work on both NT and XP,
// because the NT process structure differs only at the end
//
int get_procinfo_XP(PROC_MAP& pm) {
    ULONG                   cbBuffer = 128*1024;    // 128k initial buffer
    PVOID                   pBuffer = NULL;
    PSYSTEM_PROCESSES       pProcesses = NULL;
    static DWORD            pid = 0;

    if (!pid) {
        pid = GetCurrentProcessId();
    }
#if 0
	printf("FILETIME: %d\n", sizeof(FILETIME));
	printf("LARGE_INTEGER: %d\n", sizeof(LARGE_INTEGER));
	printf("DWORD: %d\n", sizeof(DWORD));
	printf("UNICODE_STRING: %d\n", sizeof(UNICODE_STRING));
	printf("KPRIORITY: %d\n", sizeof(KPRIORITY));
	printf("ULONG: %d\n", sizeof(ULONG));
	printf("SIZE_T: %d\n", sizeof(SIZE_T));
#endif

    get_process_information(&pBuffer, &cbBuffer);
    pProcesses = (PSYSTEM_PROCESSES)pBuffer;
    while (pProcesses) {
        PROCINFO p;
        p.clear();
		p.id = pProcesses->ProcessId;
		p.parentid = pProcesses->InheritedFromProcessId;
        p.swap_size = pProcesses->VmCounters.PagefileUsage;
        p.working_set_size = pProcesses->VmCounters.WorkingSetSize;
		p.page_fault_count = pProcesses->VmCounters.PageFaultCount;
        p.user_time = ((double) pProcesses->UserTime.QuadPart)/1e7;
        p.kernel_time = ((double) pProcesses->KernelTime.QuadPart)/1e7;
		p.id = pProcesses->ProcessId;
		p.parentid = pProcesses->InheritedFromProcessId;
        p.is_low_priority = (pProcesses->BasePriority <= 4);
        WideCharToMultiByte(CP_ACP, 0,
            pProcesses->ProcessName.Buffer,
            pProcesses->ProcessName.Length,
            p.command,
            sizeof(p.command),
            NULL, NULL
        );
		p.is_boinc_app = (p.id == (int)pid) || (strcasestr(p.command, "boinc") != NULL);
        
#ifdef _GRIDREPUBLIC
        if (!strcmp(p.command, "gridrepublic.exe")) {
            p.is_boinc_app = true;
        }
#endif        
#ifdef _PROGRESSTHRUPROCESSORS
        if (!strcmp(p.command, "progressthruprocessors.exe")) {
            p.is_boinc_app = true;
        }
#endif        
        pm.insert(std::pair<int, PROCINFO>(p.id, p));
        if (!pProcesses->NextEntryDelta) {
            break;
        }
        pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses) + pProcesses->NextEntryDelta);
    }

    if (pBuffer) HeapFree(GetProcessHeap(), NULL, pBuffer);
    find_children(pm);
    return 0;
}

// get a list of all running processes.
//
int procinfo_setup(PROC_MAP& pm) {
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    switch(osvi.dwPlatformId) {
    case VER_PLATFORM_WIN32_WINDOWS:
        // Win95, Win98, WinME
        return 0;   // not supported
    case VER_PLATFORM_WIN32_NT:
        return get_procinfo_XP(pm);
    }
    return 0;
}
