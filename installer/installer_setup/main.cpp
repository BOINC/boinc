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
#include <sstream>
#include <fstream>
#include <chrono>
#include <cwchar>

#include <openssl/evp.h>
#include <openssl/md5.h>

#include "installer_setup.h"
#include "version.h"

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void Log(const std::string& message) {
        if (!ofs.is_open()) {
            return;
        }
        char buf[128];
        const auto now = std::chrono::system_clock::now();
        const auto now_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_s(&tm, &now_c);
        std::strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y\t", &tm);
        ofs << buf << message << std::endl;
    }

private:
    Logger() {
        char tempPath[MAX_PATH];
        std::filesystem::path logPath;
        const auto len = GetTempPath(MAX_PATH, tempPath);
        if (len != 0 && len <= MAX_PATH) {
            logPath = tempPath;
        } else {
            logPath = std::filesystem::current_path();
        }
        logPath /= "boinc_installer.log";
        ofs.open(logPath, std::ios::app);
    }
    ~Logger() {
        if (ofs.is_open()) {
            ofs.close();
        }
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream ofs;
};

void Log(const std::string& message) {
    Logger::Instance().Log(message);
}

static std::string WideToUtf8(const wchar_t* w) {
    if (!w) return {};
    const auto len = static_cast<int>(wcslen(w));
    if (len == 0) return {};
    const auto size =
        WideCharToMultiByte(CP_UTF8, 0, w, len, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string out;
    out.resize(static_cast<size_t>(size));
    WideCharToMultiByte(
        CP_UTF8, 0, w, len, out.data(), size, nullptr, nullptr);
    return out;
}

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
    Log("Computing MD5 hash...");

    unsigned char md5Digest[MD5_DIGEST_LENGTH];
    auto* mdContext = EVP_MD_CTX_new();

    if (mdContext == nullptr) {
        Log("Failed to create MD5 context.");
        return {};
    }

    if (EVP_DigestInit_ex(mdContext, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log("Failed to initialize MD5 context.");
        return {};
    }

    if (EVP_DigestUpdate(mdContext, buffer, size) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log("Failed to update MD5 context.");
        return {};
    }

    if (EVP_DigestFinal_ex(mdContext, md5Digest, nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log("Failed to finalize MD5 digest.");
        return {};
    }

    EVP_MD_CTX_free(mdContext);

    std::ostringstream oss;
    for (auto i = 0u; i < MD5_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') <<
            static_cast<int>(md5Digest[i]);
    }

    Log("MD5 hash computed successfully: " + oss.str());
    return oss.str();
}


bool ExtractResourceAndExecute(UINT ResourceID, std::string OutputFileName,
    std::string CmdParameters)
{
    Log("Extracting resource and executing installer...");
    try {
        auto hResource = FindResource(nullptr, MAKEINTRESOURCE(ResourceID),
            "BINARY");
        if (hResource == nullptr) {
            Log("Failed to find resource.");
            return false;
        }

        auto hFileResource = LoadResource(nullptr, hResource);
        if (hFileResource == nullptr) {
            Log("Failed to load resource.");
            return false;
        }

        auto lpFile = LockResource(hFileResource);
        if (lpFile == nullptr) {
            Log("Failed to lock resource.");
            return false;
        }

        const auto dwSize = SizeofResource(nullptr, hResource);
        if (dwSize == 0) {
            Log("Resource size is zero.");
            return false;
        }

        char buffer[MAX_PATH];
        const auto result = GetWindowsDirectory(buffer, MAX_PATH);
        if (result == 0 || result > MAX_PATH) {
            Log("Failed to get Windows directory.");
            MessageBox(NULL, "Failed to get the Windows directory!", "Error",
                MB_ICONERROR);
            return false;
        }

        const std::filesystem::path windowsDir(buffer);
        const auto outputDir = windowsDir / "Downloaded Installations" /
            "BOINC" / BOINC_VERSION_STRING / computeMD5(lpFile, dwSize);
        if (!std::filesystem::exists(outputDir)) {
            Log("Creating output directory: " + outputDir.string());
            if (!std::filesystem::create_directories(outputDir)) {
                Log("Failed to create output directory: " +
                    outputDir.string());
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
            Log("Failed to create file mapping.");
            return false;
        }

        auto lpBaseAddress = MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);
        if (lpBaseAddress == nullptr) {
            Log("Failed to map view of file.");
            return false;
        }
        CopyMemory(lpBaseAddress, lpFile, dwSize);
        UnmapViewOfFile(lpBaseAddress);
        CloseHandle(hFilemap);
        CloseHandle(hFile);

        auto hInstance = ShellExecute(nullptr, "open",
            (outputDir / OutputFileName).string().c_str(),
            CmdParameters.c_str(), nullptr,
            CmdParameters == "" ? SW_SHOWNORMAL : SW_HIDE);
        if (reinterpret_cast<INT_PTR>(hInstance) <= 32) {
            Log("Failed to execute installer");
            MessageBox(NULL, "Failed to execute the installer!", "Error",
                MB_ICONERROR);
            return false;
        }

        Log("Installer executed successfully: " +
            (outputDir / OutputFileName).string());
        return true;
    }
    catch (const std::exception& ex) {
        Log("Exception occurred while extracting resource: " +
            std::string(ex.what()));
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
        for (int i = 1; i < nArgs; i++) {
            if (!args.empty()) args += ' ';
            args += WideToUtf8(szArglist[i]);
        }
    }
    LocalFree(szArglist);

    if (args == "") {
        ShowWindow(hInstance, nCmdShow);
    }
    ExtractResourceAndExecute(IDB_MSI, "BOINC.msi", args);
    return 0;
}
