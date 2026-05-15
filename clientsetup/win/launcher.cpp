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
#include "launcher.h"

static HRESULT SetPrivilege(HANDLE hToken, std::wstring_view lpszPrivilege,
    DWORD dwAttributes = SE_PRIVILEGE_ENABLED) {
    std::cout << __LINE__ << std::endl;
    LUID luid;

    if (LookupPrivilegeValue(nullptr, lpszPrivilege.data(), &luid)) {
        std::cout << __LINE__ << std::endl;
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = dwAttributes;

        std::cout << __LINE__ << std::endl;
        if (!AdjustTokenPrivileges(hToken, FALSE, &tp,
            sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
            std::cout << __LINE__ << std::endl;
            return HRESULT_FROM_WIN32(GetLastError());
        }
        std::cout << __LINE__ << std::endl;
        return S_OK;
    }
    else {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
}

// returns HRESULT of the operation on
// SE_CREATE_GLOBAL_NAME (="SeCreateGlobalPrivilege")
static HRESULT ReducePrivilegesForMediumIL(HANDLE hToken) {
    std::cout << __LINE__ << std::endl;
    const auto hr =
        SetPrivilege(hToken, SE_CREATE_GLOBAL_NAME, SE_PRIVILEGE_REMOVED);

    SetPrivilege(hToken, SE_BACKUP_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_CREATE_PAGEFILE_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_CREATE_SYMBOLIC_LINK_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_DEBUG_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_IMPERSONATE_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_INC_BASE_PRIORITY_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_INCREASE_QUOTA_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_LOAD_DRIVER_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_MANAGE_VOLUME_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_PROF_SINGLE_PROCESS_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_REMOTE_SHUTDOWN_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_RESTORE_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_SECURITY_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_SYSTEM_ENVIRONMENT_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_SYSTEM_PROFILE_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_SYSTEMTIME_NAME, SE_PRIVILEGE_REMOVED);
    SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, SE_PRIVILEGE_REMOVED);

    std::cout << __LINE__ << std::endl;
    return hr;
}

static HRESULT GetProcessIL(DWORD dwProcessId, LPDWORD pdwProcessIL) {
    std::cout << __LINE__ << std::endl;
    if (!pdwProcessIL) {
        std::cout << __LINE__ << std::endl;
        return E_INVALIDARG;
    }

    std::cout << __LINE__ << std::endl;
    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    std::cout << __LINE__ << std::endl;
    if (!hProcess) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hProcessDeleter(hProcess);
    std::cout << __LINE__ << std::endl;

    HANDLE hToken = nullptr;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hTokenDeleter(hToken);

    std::cout << __LINE__ << std::endl;
    PTOKEN_MANDATORY_LABEL pTIL = nullptr;
    DWORD dwSize = 0;
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, nullptr, 0, &dwSize)
        && GetLastError() == ERROR_INSUFFICIENT_BUFFER && dwSize) {
        std::cout << __LINE__ << std::endl;
        pTIL = reinterpret_cast<PTOKEN_MANDATORY_LABEL>(
            HeapAlloc(GetProcessHeap(), 0, dwSize));
        std::cout << __LINE__ << std::endl;
    }
    std::cout << __LINE__ << std::endl;
    if (!pTIL) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_process_heap pTILDeleter(pTIL);

    std::cout << __LINE__ << std::endl;
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize,
        &dwSize)) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::cout << __LINE__ << std::endl;
    const auto lpb = GetSidSubAuthorityCount(pTIL->Label.Sid);
    std::cout << __LINE__ << std::endl;
    if (!lpb) {
        std::cout << __LINE__ << std::endl;
        return E_UNEXPECTED;
    }

    std::cout << __LINE__ << std::endl;
    *pdwProcessIL = *GetSidSubAuthority(pTIL->Label.Sid, *lpb - 1);
    std::cout << __LINE__ << std::endl;
    return S_OK;
}

static HRESULT CreateProcessWithIL(std::wstring_view szCmdLine) {
    std::cout << __LINE__ << std::endl;
    HANDLE hNewToken;
    DWORD dwEnableVirtualization = 0;

    DWORD dwExplorerID = 0;
    std::cout << __LINE__ << std::endl;
    auto hwndShell = ::FindWindow(_T("Progman"), nullptr);
    std::cout << __LINE__ << std::endl;
    if (!hwndShell) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    std::cout << __LINE__ << std::endl;
    if (GetWindowThreadProcessId(hwndShell, &dwExplorerID) == 0) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    std::cout << __LINE__ << std::endl;

    DWORD dwExplorerIL = 0;
    std::cout << __LINE__ << std::endl;
    if (GetProcessIL(dwExplorerID, &dwExplorerIL) != S_OK) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    std::cout << __LINE__ << std::endl;
    if (dwExplorerIL != SECURITY_MANDATORY_MEDIUM_RID &&
        dwExplorerIL != SECURITY_MANDATORY_HIGH_RID) {
        std::cout << __LINE__ << std::endl;
        return E_FAIL;
    }

    DWORD dwCurIL = 0;
    std::cout << __LINE__ << std::endl;
    if (GetProcessIL(GetCurrentProcessId(), &dwCurIL) != S_OK) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    std::cout << __LINE__ << std::endl;
    if (dwCurIL != SECURITY_MANDATORY_MEDIUM_RID &&
        dwCurIL != SECURITY_MANDATORY_HIGH_RID) {
        std::cout << __LINE__ << std::endl;
        return E_FAIL;
    }

    std::cout << __LINE__ << std::endl;
    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwExplorerID);
    std::cout << __LINE__ << std::endl;
    if (!hProcess) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hProcessDeleter(hProcess);

    std::cout << __LINE__ << std::endl;
    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hTokenDeleter(hToken);

    std::cout << __LINE__ << std::endl;
    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, nullptr,
        SecurityImpersonation, TokenPrimary, &hNewToken)) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hNewTokenDeleter(hNewToken);

    std::cout << __LINE__ << std::endl;
    HANDLE hProcessToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
        &hProcessToken)) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else {
        std::cout << __LINE__ << std::endl;
        wil::unique_handle hProcessTokenDeleter(hProcessToken);
        const auto hr = SetPrivilege(hProcessToken, SE_INCREASE_QUOTA_NAME);
        std::cout << __LINE__ << std::endl;
        if (hr != S_OK) {
            std::cout << __LINE__ << std::endl;
            return hr;
        }
        std::cout << __LINE__ << std::endl;
        if (!CloseHandle(hProcessToken)) {
            std::cout << __LINE__ << std::endl;
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    std::cout << __LINE__ << std::endl;
    const auto hr = ReducePrivilegesForMediumIL(hNewToken);
    std::cout << __LINE__ << std::endl;
    if (hr != S_OK) {
        std::cout << __LINE__ << std::endl;
        return hr;
    }

    std::cout << __LINE__ << std::endl;
    if (!SetTokenInformation(hNewToken, TokenVirtualizationEnabled,
        &dwEnableVirtualization, sizeof(DWORD))) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::cout << __LINE__ << std::endl;
    PROCESS_INFORMATION ProcInfo = { 0 };
    STARTUPINFO StartupInfo = { 0 };
    std::cout << __LINE__ << std::endl;
    if (!CreateProcessWithTokenW(hNewToken, 0, nullptr,
        const_cast<wchar_t*>(szCmdLine.data()), 0, nullptr, nullptr,
        &StartupInfo, &ProcInfo)) {
        std::cout << __LINE__ << std::endl;
        if (!CreateProcessAsUser(hNewToken, nullptr,
            const_cast<wchar_t*>(szCmdLine.data()), nullptr, nullptr, FALSE,
            NORMAL_PRIORITY_CLASS, nullptr, nullptr, &StartupInfo, &ProcInfo
        )) {
            std::cout << __LINE__ << std::endl;
            return HRESULT_FROM_WIN32(GetLastError());
        }
        std::cout << __LINE__ << std::endl;
    }

    std::cout << __LINE__ << std::endl;
    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);

    std::cout << __LINE__ << std::endl;
    return S_OK;
}

HRESULT Launcher::CreateProcessWithExplorerIL(std::wstring_view szCmdLine) {
    std::cout << __LINE__ << std::endl;
    if (CreateProcessWithIL(szCmdLine) == S_OK) {
        std::cout << __LINE__ << std::endl;
        return S_OK;
    }

    std::cout << __LINE__ << std::endl;
    PROCESS_INFORMATION ProcInfo = { 0 };
    STARTUPINFO StartupInfo = { 0 };
    std::cout << __LINE__ << std::endl;
    if (!CreateProcess(nullptr, const_cast<wchar_t*>(szCmdLine.data()),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &StartupInfo,
        &ProcInfo)) {
        std::cout << __LINE__ << std::endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::cout << __LINE__ << std::endl;
    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);

    std::cout << __LINE__ << std::endl;
    return S_OK;
}
