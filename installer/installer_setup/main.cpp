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

#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cwchar>
#include <format>
#include <tchar.h>

#include <openssl/evp.h>
#include <openssl/md5.h>

#include <wil/resource.h>

typedef std::wstring_view tstring_view;
typedef std::wstring tstring;

#include "installer_setup.h"
#include "launcher.h"
#include "version.h"
#include "win_util.h"

constexpr tstring_view boinc_buda_runner_wsl_installer_checksum =
    _T("b45f2c8630ddd26ed9a1887f8c2da0cda6141b305081d97903e133024b2dd8c1");

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void Log(tstring_view message) {
        if (!ofs.is_open()) {
            return;
        }
        ofs << std::format(_T("{:%a %b %d %H:%M:%S %Y}\t{}\n"),
            std::chrono::system_clock::now(), message
        );
    }

private:
    Logger() {
        wchar_t tempPath[MAX_PATH];
        std::filesystem::path logPath;
        const auto len = GetTempPath(MAX_PATH, tempPath);
        if (len != 0 && len <= MAX_PATH) {
            logPath = tempPath;
        }
        else {
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

    std::wofstream ofs;
};

static void Log(tstring_view message) {
    Logger::Instance().Log(message);
}

HBITMAP hBitmap = nullptr;

static void LoadSplashImage(HDC hdc, HWND hwnd) {
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

static LRESULT CALLBACK WndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam
) {
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

static tstring computeMD5(const void* buffer, size_t size) {
    Log(_T("Computing MD5 hash..."));

    unsigned char md5Digest[MD5_DIGEST_LENGTH];
    auto* mdContext = EVP_MD_CTX_new();

    if (mdContext == nullptr) {
        Log(_T("Failed to create MD5 context."));
        return {};
    }

    if (EVP_DigestInit_ex(mdContext, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log(_T("Failed to initialize MD5 context."));
        return {};
    }

    if (EVP_DigestUpdate(mdContext, buffer, size) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log(_T("Failed to update MD5 context."));
        return {};
    }

    if (EVP_DigestFinal_ex(mdContext, md5Digest, nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log(_T("Failed to finalize MD5 digest."));
        return {};
    }

    EVP_MD_CTX_free(mdContext);

    tstring md5_str;
    for (auto i : md5Digest) {
        md5_str += std::format(_T("{:02x}"), i);
    }

    Log(std::format(_T("MD5 hash computed successfully: {}"), md5_str));
    return md5_str;
}

static tstring computeSHA256(const std::filesystem::path& filePath) {
    Log(std::format(_T("Computing SHA256 hash for file: {}"),
        filePath.wstring())
    );
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        Log(_T("Failed to open file for SHA256 computation."));
        return {};
    }
    unsigned char sha256Digest[EVP_MAX_MD_SIZE];
    unsigned int digestLength = 0;
    auto* mdContext = EVP_MD_CTX_new();
    if (mdContext == nullptr) {
        Log(_T("Failed to create SHA256 context."));
        return {};
    }
    if (EVP_DigestInit_ex(mdContext, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log(_T("Failed to initialize SHA256 context."));
        return {};
    }
    constexpr size_t bufferSize = 8192;
    char buffer[bufferSize];
    while (file.read(buffer, bufferSize)) {
        if (EVP_DigestUpdate(mdContext, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdContext);
            Log(_T("Failed to update SHA256 context."));
            return {};
        }
    }
    // Handle any remaining bytes
    if (file.gcount() > 0) {
        if (EVP_DigestUpdate(mdContext, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdContext);
            Log(_T("Failed to update SHA256 context with remaining bytes."));
            return {};
        }
    }
    if (EVP_DigestFinal_ex(mdContext, sha256Digest, &digestLength) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log(_T("Failed to finalize SHA256 digest."));
        return {};
    }
    EVP_MD_CTX_free(mdContext);

    tstring sha256_str;
    for (unsigned int i = 0; i < digestLength; ++i) {
        sha256_str += std::format(_T("{:02x}"), sha256Digest[i]);
    }
    Log(std::format(_T("SHA256 hash computed successfully: {}"), sha256_str));
    return sha256_str;
}

static tstring getRegistryValue(tstring_view strName) {
    Log(std::format(_T("Attempting to read registry value: {}"), strName));
    HKEY hkSetupHive;
    auto lReturnValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),
        0, KEY_READ, &hkSetupHive);
    if (lReturnValue != ERROR_SUCCESS) {
        Log(
            std::format(_T("Failed to open registry key for reading: {}"),
                strName
            )
        );
        return {};
    }
    wil::unique_hkey hKeyDeleter(hkSetupHive);

    DWORD dwSize = 0;
    lReturnValue = RegQueryValueEx(hkSetupHive, strName.data(), nullptr,
        nullptr, nullptr, &dwSize);
    if (lReturnValue != ERROR_SUCCESS) {
        Log(
            std::format(_T("Failed to query registry key for reading: {}"),
                strName
            )
        );
        return {};
    }

    tstring strValue;
    strValue.resize(dwSize / sizeof(wchar_t));
    lReturnValue = RegQueryValueEx(hkSetupHive, strName.data(), nullptr,
        nullptr, reinterpret_cast<LPBYTE>(strValue.data()), &dwSize);

    if (lReturnValue != ERROR_SUCCESS) {
        Log(std::format(_T("Failed to read registry value: {}"), strName));
        return {};
    }

    if (!strValue.empty() && strValue.back() == '\0') {
        strValue.pop_back();
    }

    Log(
        std::format(
            _T("Successfully read from the registry key '{}' ")
            _T("with a value of '{}'"), strName, strValue
        )
    );
    return strValue;
}

static bool ExtractResourceAndExecute(UINT ResourceID,
    tstring_view OutputFileName, tstring_view CmdParameters) {
    Log(_T("Extracting resource and executing installer..."));
    try {
        auto hResource = FindResource(nullptr, MAKEINTRESOURCE(ResourceID),
            _T("BINARY"));
        if (hResource == nullptr) {
            Log(_T("Failed to find resource."));
            return false;
        }

        auto hFileResource = LoadResource(nullptr, hResource);
        if (hFileResource == nullptr) {
            Log(_T("Failed to load resource."));
            return false;
        }

        auto lpFile = LockResource(hFileResource);
        if (lpFile == nullptr) {
            Log(_T("Failed to lock resource."));
            return false;
        }

        const auto dwSize = SizeofResource(nullptr, hResource);
        if (dwSize == 0) {
            Log(_T("Resource size is zero."));
            return false;
        }

        wchar_t buffer[MAX_PATH];
        const auto result = GetWindowsDirectory(buffer, MAX_PATH);
        if (result == 0 || result > MAX_PATH) {
            Log(_T("Failed to get Windows directory."));
            MessageBox(
                nullptr,
                _T("Failed to get the Windows directory!"),
                _T("Error"),
                MB_ICONERROR
            );
            return false;
        }

        const std::filesystem::path windowsDir(buffer);
        const auto outputDir = windowsDir / _T("Downloaded Installations") /
            _T("BOINC") / BOINC_VERSION_STRING / computeMD5(lpFile, dwSize);
        if (!std::filesystem::exists(outputDir)) {
            Log(_T("Creating output directory: ") + outputDir.wstring());
            if (!std::filesystem::create_directories(outputDir)) {
                Log(_T("Failed to create output directory: ") +
                    outputDir.wstring());
                MessageBox(
                    nullptr,
                    _T("Failed to create output directory!"),
                    _T("Error"),
                    MB_ICONERROR);
                return false;
            }
        }

        const auto filePath = (outputDir / OutputFileName).wstring();
        auto hFile = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE,
            0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        auto hFilemap = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0,
            dwSize, nullptr);
        if (hFilemap == nullptr) {
            Log(_T("Failed to create file mapping."));
            return false;
        }

        auto lpBaseAddress = MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);
        if (lpBaseAddress == nullptr) {
            Log(_T("Failed to map view of file."));
            return false;
        }
        CopyMemory(lpBaseAddress, lpFile, dwSize);
        UnmapViewOfFile(lpBaseAddress);
        CloseHandle(hFilemap);
        CloseHandle(hFile);

        const auto bMode = CmdParameters == _T("") ? SW_SHOWNORMAL : SW_HIDE;
        wchar_t self[MAX_PATH];
        auto cmd = tstring(CmdParameters);
        if (GetModuleFileName(nullptr, self, MAX_PATH) != 0) {
            cmd += std::format(_T(" SETUPEXENAME=\"{}\""),
                std::filesystem::path(self).filename().wstring()
            );
        }

        SHELLEXECUTEINFO sei = {};
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd = nullptr;
        sei.lpVerb = _T("open");
        sei.lpFile = filePath.c_str();
        sei.lpParameters = cmd.data();
        sei.lpDirectory = nullptr;
        sei.nShow = bMode;
        sei.hInstApp = nullptr;
        if (!ShellExecuteEx(&sei)) {
            Log(_T("Failed to execute installer"));
            MessageBox(nullptr,
                _T("Failed to execute the installer!"),
                _T("Error"),
                MB_ICONERROR
            );
            return false;
        }
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);

        Log(std::format(_T("Installer executed successfully: {}"), filePath));
        return true;
    }
    catch (const std::exception& ex) {
        const auto ex_what =
            std::format(_T("{}"), boinc_ascii_to_wide(ex.what()));

        Log(std::format(_T("Exception occurred while extracting resource: {}"),
            ex_what)
        );

        MessageBox(nullptr,
            _T("Failed to extract resource!"),
            ex_what.data(),
            MB_ICONERROR
        );
        return false;
    }
}

static void ShowWindow(HINSTANCE hInstance, int nCmdShow) {
    hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));
    if (hBitmap) {
        const auto CLASS_NAME = _T("SplashScreen");
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

        if (!RegisterClass(&wc)) {
            return;
        }

        auto hwnd = CreateWindowEx(
            WS_EX_TOPMOST, CLASS_NAME, _T("BOINC Installer"), WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr,
            hInstance,
            nullptr
        );

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

static int InstallWSL(int mode, const std::filesystem::path& installPath) {
    constexpr tstring_view wslInstallerName =
        _T("boinc-buda-runner-wsl-installer.exe");

    const auto wslInstallerPath = installPath / wslInstallerName;
    if (!std::filesystem::exists(wslInstallerPath)) {
        Log(std::format(_T("File '{}' is missing, skipping installation"),
            wslInstallerPath.wstring())
        );
        return -2;
    }

    const auto hash = computeSHA256(wslInstallerPath);
    if (hash.empty()) {
        Log(std::format(
            _T(
                "Failed to get a hash for the file '{}', skipping installation"
            ),
            wslInstallerPath.wstring())
        );
        return -2;
    }
    if (hash != boinc_buda_runner_wsl_installer_checksum) {
        Log(std::format(
            _T("A hash for the file '{}' is wrong, skipping installation"),
            wslInstallerPath.wstring())
        );
        return -2;
    }

    const auto wslInstallerPathStr = wslInstallerPath.wstring();
    SHELLEXECUTEINFO sei = {};
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = nullptr;
    sei.lpVerb = nullptr;
    sei.lpFile = wslInstallerPathStr.c_str();
    sei.lpParameters = mode == SW_SHOWNORMAL ? _T("-nu") : _T("-q");
    sei.lpDirectory = nullptr;
    sei.nShow = mode;
    sei.hInstApp = nullptr;
    if (!ShellExecuteEx(&sei)) {
        Log(_T("Failed to execute installer"));
        return -2;
    }
    WaitForSingleObject(sei.hProcess, INFINITE);
    CloseHandle(sei.hProcess);

    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);

    int nArgs;
    auto* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (szArglist == nullptr) {
        return 0;
    }
    tstring args = _T("");
    bool adminInstallation = false;
    bool noWsl = false;
    if (nArgs > 1) {
        for (int i = 1; i < nArgs; i++) {
            if (!args.empty()) args += ' ';
            args += szArglist[i];
            if (wcscmp(szArglist[i], _T("/a")) == 0) {
                adminInstallation = true;
            }
            if (wcscmp(szArglist[i],
                _T("OVERRIDE_INSTALLWSLIMAGEINSTALLER=0")) == 0
                ) {
                noWsl = true;
            }
        }
    }
    LocalFree(szArglist);

    if (args.empty()) {
        ShowWindow(hInstance, nCmdShow);
    }
    // install BOINC
    if (!ExtractResourceAndExecute(IDB_MSI, _T("BOINC.msi"), args)) {
        return -1;
    }

    if (adminInstallation) {
        Log(_T("Admin installation, skipping WSL installation"));
        return 0;
    }

    if (noWsl) {
        Log(_T(
            "OVERRIDE_INSTALLWSLIMAGEINSTALLER=0, skipping WSL installation"
        ));
        return 0;
    }

    const auto runWslInstaller =
        getRegistryValue(_T("INSTALLWSLIMAGEINSTALLER"));
    if (runWslInstaller != _T("1")) {
        Log(std::format(
            _T("'INSTALLWSLIMAGEINSTALLER' is [{}], skipping installation"),
            runWslInstaller)
        );
        // we don't run wsl installer that means that we don't need
        // to run boinc either, so we return here
        return 0;
    }

    const auto runBOINC = getRegistryValue(_T("LAUNCHPROGRAM"));
    if (runBOINC != _T("1")) {
        Log(std::format(
            _T("'LAUNCHPROGRAM' is [{}], not launching BOINC"),
            runBOINC)
        );
    }

    const auto installDir = getRegistryValue(_T("INSTALLDIR"));
    if (installDir.empty()) {
        Log(_T("'INSTALLDIR' value is empty, skipping installation"));
        return -3;
    }

    const auto installPath = std::filesystem::path(installDir);

    // install WSL and return
    int wslInstallResult = 0;
    if (runWslInstaller == _T("1")) {
        wslInstallResult =
            InstallWSL(args.empty() ? SW_SHOWNORMAL : SW_HIDE, installPath);
    }

    if (runBOINC == _T("1")) {
        const auto manager =
            std::format(
                _T("\"{}\""), (installPath / _T("boincmgr.exe")).wstring()
            );
        if (Launcher::CreateProcessWithExplorerIL(manager) != S_OK) {
            Log(std::format(_T("Failed to run '{}', exiting"), manager));
            return -4;
        }
        const auto tray =
            std::format(
                _T("\"{}\""), (installPath / _T("boinctray.exe")).wstring()
            );
        if (Launcher::CreateProcessWithExplorerIL(tray) != S_OK) {
            Log(std::format(_T("Failed to run '{}', exiting"), tray));
            return -5;
        }
    }

    return wslInstallResult;
}
