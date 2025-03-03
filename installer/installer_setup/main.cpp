// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <codecvt>

#include <openssl/evp.h>
#include <openssl/md5.h>

#include "installer_setup.h"
#include "version.h"

HBITMAP hBitmap = NULL;

void LoadSplashImage(HDC hdc, HWND hwnd) {
    auto hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hBitmap);

    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    RECT rect;
    GetClientRect(hwnd, &rect);

    StretchBlt(hdc, 0, 0, rect.right, rect.bottom,
        hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

    DeleteDC(hdcMem);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        auto hdc = BeginPaint(hwnd, &ps);
        LoadSplashImage(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        DeleteObject(hBitmap);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

std::string computeMD5(const void* buffer, size_t size) {
    unsigned char md5Digest[MD5_DIGEST_LENGTH];
    auto* mdContext = EVP_MD_CTX_new();

    if (mdContext == nullptr) {
        return {};
    }

    if (EVP_DigestInit_ex(mdContext, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        return {};
    }

    if (EVP_DigestUpdate(mdContext, buffer, size) != 1) {
        EVP_MD_CTX_free(mdContext);
        return {};
    }

    if (EVP_DigestFinal_ex(mdContext, md5Digest, nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        return {};
    }

    EVP_MD_CTX_free(mdContext);

    std::ostringstream oss;
    for (auto i = 0u; i < MD5_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') <<
            static_cast<int>(md5Digest[i]);
    }

    return oss.str();
}


bool ExtractResourceAndExecute(UINT ResourceID, std::string OutputFileName,
    std::string CmdParameters)
{
    try {
        auto hResource = FindResource(nullptr, MAKEINTRESOURCE(ResourceID),
            "BINARY");
        if (hResource == nullptr) {
            return false;
        }

        auto hFileResource = LoadResource(nullptr, hResource);
        if (hFileResource == nullptr) {
            return false;
        }

        auto lpFile = LockResource(hFileResource);
        if (lpFile == nullptr) {
            return false;
        }

        const auto dwSize = SizeofResource(nullptr, hResource);
        if (dwSize == 0) {
            return false;
        }

        char buffer[MAX_PATH];
        const auto result = GetWindowsDirectory(buffer, MAX_PATH);
        if (result == 0 || result > MAX_PATH) {
            MessageBox(NULL, "Failed to get the Windows directory!", "Error",
                MB_ICONERROR);
            return false;
        }

        const std::filesystem::path windowsDir(buffer);
        const auto outputDir = windowsDir / "Downloaded Installations" /
            "BOINC" / BOINC_VERSION_STRING / computeMD5(lpFile, dwSize);
        if (!std::filesystem::exists(outputDir)) {
            if (!std::filesystem::create_directories(outputDir)) {
                MessageBox(NULL, "Failed to create output directory!", "Error",
                    MB_ICONERROR);
                return false;
            }
        }

        auto hFile = CreateFile((outputDir / OutputFileName).string().c_str(),
            GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        auto hFilemap = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0,
            dwSize, nullptr);
        if (hFilemap == nullptr) {
            return false;
        }

        auto lpBaseAddress = MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);
        if (lpBaseAddress == nullptr) {
            return false;
        }
        CopyMemory(lpBaseAddress, lpFile, dwSize);
        UnmapViewOfFile(lpBaseAddress);
        CloseHandle(hFilemap);
        CloseHandle(hFile);

        const auto hInstance = ShellExecute(nullptr, "open",
            (outputDir / OutputFileName).string().c_str(),
            CmdParameters.c_str(), nullptr,
            CmdParameters == "" ? SW_SHOWNORMAL : SW_HIDE);
        if (reinterpret_cast<int>(hInstance) <= 32) {
            MessageBox(NULL, "Failed to execute the installer!", "Error",
                MB_ICONERROR);
            return false;
        }

        return true;
    }
    catch (const std::exception& ex) {
        MessageBox(NULL, "Failed to extract resource!", ex.what(),
            MB_ICONERROR);
        return false;
    }
    return false;
}

void ShowWindow(HINSTANCE hInstance, int nCmdShow) {
    hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));
    if (hBitmap) {
        const auto CLASS_NAME = "SplashScreen";
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

        if (!RegisterClass(&wc)) {
            return;
        }

        auto hwnd = CreateWindowEx(
            WS_EX_TOPMOST, CLASS_NAME, "Splash Screen", WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance,
            NULL);

        if (!hwnd) {
            return;
        }

        RECT rect;
        GetWindowRect(hwnd, &rect);
        const auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const auto x = (screenWidth - 400) / 2;
        const auto y = (screenHeight - 300) / 2;
        SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, 400, 300,
            SWP_SHOWWINDOW);

        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);

    LPWSTR* szArglist;
    int nArgs;
    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (szArglist == NULL) {
        return 0;
    }
    std::string args = "";
    if (nArgs > 1) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        for (int i = 1; i < nArgs; i++) {
            args += converter.to_bytes(szArglist[i]) + " ";
        }
    }
    LocalFree(szArglist);

    if (args == "") {
        ShowWindow(hInstance, nCmdShow);
    }
    ExtractResourceAndExecute(IDB_MSI, "BOINC.msi", args);
    return 0;
}
