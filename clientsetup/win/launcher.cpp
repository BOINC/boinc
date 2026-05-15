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
    LUID luid;

    if (LookupPrivilegeValue(nullptr, lpszPrivilege.data(), &luid)) {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = dwAttributes;

        if (!AdjustTokenPrivileges(hToken, FALSE, &tp,
            sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        return S_OK;
    }
    else {
        return HRESULT_FROM_WIN32(GetLastError());
    }
}

// returns HRESULT of the operation on
// SE_CREATE_GLOBAL_NAME (="SeCreateGlobalPrivilege")
static HRESULT ReducePrivilegesForMediumIL(HANDLE hToken) {
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

    return hr;
}

static HRESULT GetProcessIL(DWORD dwProcessId, LPDWORD pdwProcessIL) {
    if (!pdwProcessIL) {
        return E_INVALIDARG;
    }

    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (!hProcess) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hProcessDeleter(hProcess);

    HANDLE hToken = nullptr;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hTokenDeleter(hToken);

    PTOKEN_MANDATORY_LABEL pTIL = nullptr;
    DWORD dwSize = 0;
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, nullptr, 0, &dwSize)
        && GetLastError() == ERROR_INSUFFICIENT_BUFFER && dwSize) {
        pTIL = reinterpret_cast<PTOKEN_MANDATORY_LABEL>(
            HeapAlloc(GetProcessHeap(), 0, dwSize));
    }
    if (!pTIL) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_process_heap pTILDeleter(pTIL);

    if (!GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize,
        &dwSize)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const auto lpb = GetSidSubAuthorityCount(pTIL->Label.Sid);
    if (!lpb) {
        return E_UNEXPECTED;
    }

    *pdwProcessIL = *GetSidSubAuthority(pTIL->Label.Sid, *lpb - 1);
    return S_OK;
}

static HRESULT CreateProcessWithIL(std::wstring_view szCmdLine) {
    HANDLE hNewToken;
    DWORD dwEnableVirtualization = 0;

    DWORD dwExplorerID = 0;
    auto hwndShell = ::FindWindow(_T("Progman"), nullptr);
    if (!hwndShell) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (GetWindowThreadProcessId(hwndShell, &dwExplorerID) == 0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD dwExplorerIL = 0;
    if (GetProcessIL(dwExplorerID, &dwExplorerIL) != S_OK) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (dwExplorerIL != SECURITY_MANDATORY_MEDIUM_RID &&
        dwExplorerIL != SECURITY_MANDATORY_HIGH_RID) {
        return E_FAIL;
    }

    DWORD dwCurIL = 0;
    if (GetProcessIL(GetCurrentProcessId(), &dwCurIL) != S_OK) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (dwCurIL != SECURITY_MANDATORY_MEDIUM_RID &&
        dwCurIL != SECURITY_MANDATORY_HIGH_RID) {
        return E_FAIL;
    }

    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwExplorerID);
    if (!hProcess) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hProcessDeleter(hProcess);

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hTokenDeleter(hToken);

    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, nullptr,
        SecurityImpersonation, TokenPrimary, &hNewToken)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wil::unique_handle hNewTokenDeleter(hNewToken);

    HANDLE hProcessToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
        &hProcessToken)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else {
        wil::unique_handle hProcessTokenDeleter(hProcessToken);
        const auto hr = SetPrivilege(hProcessToken, SE_INCREASE_QUOTA_NAME);
        if (hr != S_OK) {
            return hr;
        }
    }

    const auto hr = ReducePrivilegesForMediumIL(hNewToken);
    if (hr != S_OK) {
        return hr;
    }

    if (!SetTokenInformation(hNewToken, TokenVirtualizationEnabled,
        &dwEnableVirtualization, sizeof(DWORD))) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    PROCESS_INFORMATION ProcInfo = { 0 };
    STARTUPINFO StartupInfo = { 0 };
    if (!CreateProcessWithTokenW(hNewToken, 0, nullptr,
        const_cast<wchar_t*>(szCmdLine.data()), 0, nullptr, nullptr,
        &StartupInfo, &ProcInfo)) {
        if (!CreateProcessAsUser(hNewToken, nullptr,
            const_cast<wchar_t*>(szCmdLine.data()), nullptr, nullptr, FALSE,
            NORMAL_PRIORITY_CLASS, nullptr, nullptr, &StartupInfo, &ProcInfo
        )) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);

    return S_OK;
}

HRESULT Launcher::CreateProcessWithExplorerIL(std::wstring_view szCmdLine) {
    if (CreateProcessWithIL(szCmdLine) == S_OK) {
        return S_OK;
    }

    PROCESS_INFORMATION ProcInfo = { 0 };
    STARTUPINFO StartupInfo = { 0 };
    if (!CreateProcess(nullptr, const_cast<wchar_t*>(szCmdLine.data()),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &StartupInfo,
        &ProcInfo)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    CloseHandle(ProcInfo.hThread);
    CloseHandle(ProcInfo.hProcess);

    return S_OK;
}
