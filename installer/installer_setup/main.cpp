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
#include <sstream>
#include <fstream>
#include <chrono>
#include <cwchar>

#include <openssl/evp.h>
#include <openssl/md5.h>

#include "installer_setup.h"
#include "launcher.h"
#include "version.h"

constexpr std::string_view boinc_buda_runner_wsl_installer_checksum =
    "cdf702d6b41cf105bc1b507e36964841bfab3363480a3bd135640eb63ccd0f2b";

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void Log(std::string_view message) {
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

    std::ofstream ofs;
};

static void Log(const std::string& message) {
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

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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

static std::string computeMD5(const void* buffer, size_t size) {
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

static std::string computeSHA256(const std::filesystem::path& filePath) {
    Log("Computing SHA256 hash for file: " + filePath.string());
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        Log("Failed to open file for SHA256 computation.");
        return {};
    }
    unsigned char sha256Digest[EVP_MAX_MD_SIZE];
    unsigned int digestLength = 0;
    auto* mdContext = EVP_MD_CTX_new();
    if (mdContext == nullptr) {
        Log("Failed to create SHA256 context.");
        return {};
    }
    if (EVP_DigestInit_ex(mdContext, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log("Failed to initialize SHA256 context.");
        return {};
    }
    constexpr size_t bufferSize = 8192;
    char buffer[bufferSize];
    while (file.read(buffer, bufferSize)) {
        if (EVP_DigestUpdate(mdContext, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdContext);
            Log("Failed to update SHA256 context.");
            return {};
        }
    }
    // Handle any remaining bytes
    if (file.gcount() > 0) {
        if (EVP_DigestUpdate(mdContext, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdContext);
            Log("Failed to update SHA256 context with remaining bytes.");
            return {};
        }
    }
    if (EVP_DigestFinal_ex(mdContext, sha256Digest, &digestLength) != 1) {
        EVP_MD_CTX_free(mdContext);
        Log("Failed to finalize SHA256 digest.");
        return {};
    }
    EVP_MD_CTX_free(mdContext);
    std::ostringstream oss;
    for (unsigned int i = 0; i < digestLength; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') <<
            std::nouppercase << static_cast<int>(sha256Digest[i]);
    }
    Log("SHA256 hash computed successfully: " + oss.str());
    return oss.str();
}

static std::string getRegistryValue(const std::string& strName) {
    Log("Attempting to read registry value: " + strName);
    HKEY hkSetupHive;
    auto lReturnValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup",
        0, KEY_READ, &hkSetupHive);
    if (lReturnValue != ERROR_SUCCESS) {
        Log("Failed to open registry key for reading: " + strName);
        return {};
    }
    wil::unique_hkey hKeyDeleter(hkSetupHive);

    DWORD dwSize = 0;
    lReturnValue = RegQueryValueEx(hkSetupHive, strName.data(), nullptr,
        nullptr, nullptr, &dwSize);
    if (lReturnValue != ERROR_SUCCESS) {
        Log("Failed to query registry key for reading: " + strName);
        return {};
    }

    std::string strValue;
    strValue.resize(dwSize / sizeof(char));
    lReturnValue = RegQueryValueEx(hkSetupHive, strName.data(), nullptr,
        nullptr, reinterpret_cast<LPBYTE>(strValue.data()), &dwSize);

    if (lReturnValue != ERROR_SUCCESS) {
        Log("Failed to read registry value: " + strName);
        return {};
    }

    if (!strValue.empty() && strValue.back() == '\0') {
        strValue.pop_back();
    }

    Log("Successfully read from the registry key '" + strName +
        "' with a value of '" + strValue + "'");
    return strValue;
}

static bool ExtractResourceAndExecute(UINT ResourceID,
    const std::string& OutputFileName, const std::string& CmdParameters) {
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
            MessageBox(nullptr, "Failed to get the Windows directory!", "Error",
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
                MessageBox(nullptr, "Failed to create output directory!", "Error",
                    MB_ICONERROR);
                return false;
            }
        }

        const auto filePath = (outputDir / OutputFileName).string();
        auto hFile = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE,
            0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
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

        const auto bMode = CmdParameters == "" ? SW_SHOWNORMAL : SW_HIDE;
        char self[MAX_PATH];
        auto cmd = CmdParameters;
        if (GetModuleFileName(nullptr, self, MAX_PATH) != 0) {
            cmd += " SETUPEXENAME=\"" +
                std::filesystem::path(self).filename().string() + "\"";
        }

        SHELLEXECUTEINFO sei = {};
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd = nullptr;
        sei.lpVerb = "open";
        sei.lpFile = filePath.c_str();
        sei.lpParameters = cmd.c_str();
        sei.lpDirectory = nullptr;
        sei.nShow = bMode;
        sei.hInstApp = nullptr;
        if (!ShellExecuteEx(&sei)) {
            Log("Failed to execute installer");
            MessageBox(nullptr, "Failed to execute the installer!", "Error",
                MB_ICONERROR);
            return false;
        }
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);

        Log("Installer executed successfully: " + filePath);
        return true;
    }
    catch (const std::exception& ex) {
        Log("Exception occurred while extracting resource: " +
            std::string(ex.what()));
        MessageBox(nullptr, "Failed to extract resource!", ex.what(),
            MB_ICONERROR);
        return false;
    }
}

static void ShowWindow(HINSTANCE hInstance, int nCmdShow) {
    hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));
    if (hBitmap) {
        const auto CLASS_NAME = "SplashScreen";
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        if (!RegisterClass(&wc)) {
            return;
        }

        auto hwnd = CreateWindowEx(
            WS_EX_TOPMOST, CLASS_NAME, "Splash Screen", WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance,
            nullptr);

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
    constexpr std::string_view wslInstallerName =
        "boinc-buda-runner-wsl-installer.exe";

    const auto wslInstallerPath = installPath / wslInstallerName;
    if (!std::filesystem::exists(wslInstallerPath)) {
        Log("File '" + wslInstallerPath.string() +
            "' is missing, skipping installation");
        return -2;
    }

    const auto hash = computeSHA256(wslInstallerPath);
    if (hash.empty()) {
        Log("Failed to get a hash for the file '" + wslInstallerPath.string() +
            "', skipping installation");
        return -2;
    }
    if (hash != boinc_buda_runner_wsl_installer_checksum) {
        Log("A hash for the file '" + wslInstallerPath.string() +
            "' is wrong, skipping installation");
        return -2;
    }

    const auto wslInstallerPathStr = wslInstallerPath.string();
    SHELLEXECUTEINFO sei = {};
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = nullptr;
    sei.lpVerb = nullptr;
    sei.lpFile = wslInstallerPathStr.c_str();
    sei.lpParameters = mode == SW_SHOWNORMAL ? "-nu" : "-q";
    sei.lpDirectory = nullptr;
    sei.nShow = mode;
    sei.hInstApp = nullptr;
    if (!ShellExecuteEx(&sei)) {
        Log("Failed to execute installer");
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
    std::string args = "";
    bool adminInstallation = false;
    bool noWsl = false;
    if (nArgs > 1) {
        for (int i = 1; i < nArgs; i++) {
            if (!args.empty()) args += ' ';
            args += WideToUtf8(szArglist[i]);
            if (wcscmp(szArglist[i], L"/a") == 0) {
                adminInstallation = true;
            }
            if (wcscmp(szArglist[i], L"OVERRIDE_INSTALLWSLIMAGEINSTALLER=0") == 0) {
                noWsl = true;
            }
        }
    }
    LocalFree(szArglist);

    if (args.empty()) {
        ShowWindow(hInstance, nCmdShow);
    }
    // install BOINC
    if (!ExtractResourceAndExecute(IDB_MSI, "BOINC.msi", args)) {
        return -1;
    }

    if (adminInstallation) {
        Log("Admin installation, skipping WSL installation");
        return 0;
    }

    if (noWsl) {
        Log("OVERRIDE_INSTALLWSLIMAGEINSTALLER=0, skipping WSL installation");
        return 0;
    }

    const auto runWslInstaller = getRegistryValue("INSTALLWSLIMAGEINSTALLER");
    if (runWslInstaller != "1") {
        Log("'INSTALLWSLIMAGEINSTALLER' is [" + runWslInstaller +
            "], skipping installation");
        // we don't run wsl installer that means that we don't need
        // to run boinc either, so we return here
        return 0;
    }

    const auto runBOINC = getRegistryValue("LAUNCHPROGRAM");
    if (runBOINC != "1") {
        Log("'LAUNCHPROGRAM' is [" + runBOINC +
            "], not launching BOINC");
    }

    const auto installDir = getRegistryValue("INSTALLDIR");
    if (installDir.empty()) {
        Log("'INSTALLDIR' value is empty, skipping installation");
        return -3;
    }

    const auto installPath = std::filesystem::path(installDir);

    // install WSL and return
    int wslInstallResult = 0;
    if (runWslInstaller == "1") {
        wslInstallResult = InstallWSL(args.empty() ? SW_SHOWNORMAL : SW_HIDE, installPath);
    }

    if (runBOINC == "1") {
        const auto manager =
            "\"" + (installPath / "boincmgr.exe").string() + "\"";
        if (Launcher::CreateProcessWithExplorerIL(manager) != S_OK) {
            Log("Failed to run '" + manager + "', exiting");
            return -4;
        }
        const auto tray =
            "\"" + (installPath / "boinctray.exe").string() + "\"";
        if (Launcher::CreateProcessWithExplorerIL(tray) != S_OK) {
            Log("Failed to run '" + tray + "', exiting");
            return -5;
        }
    }

    return wslInstallResult;
}
