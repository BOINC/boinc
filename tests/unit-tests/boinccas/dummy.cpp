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

#include <windows.h>

#ifdef _GUI

LRESULT CALLBACK DummyWndProc(HWND hwnd, UINT msg, WPARAM wParam,
    LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    constexpr auto kClassName = "wxTaskBarExWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = DummyWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kClassName;

    if (!RegisterClass(&wc)) {
        return 1;
    }

    auto hwnd = CreateWindowEx(0, kClassName, "BOINCManagerSystray",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr,
        nullptr, hInstance, nullptr);

    if (!hwnd) {
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

#else

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

SERVICE_STATUS g_ServiceStatus = {};
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
HANDLE ghSvcStopEvent = nullptr;

void WINAPI ServiceCtrlHandler(DWORD ctrl) {
    if (ctrl == SERVICE_CONTROL_STOP) {
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        SetEvent(ghSvcStopEvent);
    }
}

void WINAPI ServiceMain(DWORD, LPTSTR*) {
    g_StatusHandle = RegisterServiceCtrlHandler("BOINC", ServiceCtrlHandler);

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    ghSvcStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (ghSvcStopEvent == nullptr) {
        g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    while (true) {
        WaitForSingleObject(ghSvcStopEvent, INFINITE);
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }
}

int main() {
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        { "MyService", ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return 0;
}

#endif
