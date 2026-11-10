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

#include "boinccas.h"
#include <openssl/evp.h>
#include "win_util.h"

class CALaunchBOINCWSLInstaller : public BOINCCABase {
public:
    virtual ~CALaunchBOINCWSLInstaller() = default;
    explicit CALaunchBOINCWSLInstaller(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CALaunchBOINCWSLInstaller"),
            _T("Launching BOINC WSL Installer")) {
    }
private:
    UINT OnExecution() override final {
        tstring strInstallDirectory;

        auto uiReturnValue =
            GetProperty(_T("INSTALLDIR"), strInstallDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strInstallDirectory.empty()) {
            return ERROR_INSTALL_FAILURE;
        }

        auto strBuffer = strInstallDirectory +
            tstring(_T("\\boinc-buda-runner-wsl-installer.exe"));

        tstring strCheckSum;
        uiReturnValue =
            GetProperty(_T("boinc_buda_runner_wsl_installer_checksum"),
            strCheckSum);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strCheckSum.empty()) {
            return ERROR_INSTALL_FAILURE;
        }

        //verify the installer executable's checksum before launching it
        std::ifstream file(strBuffer, std::ios::binary);
        if (!file.is_open()) {
            return ERROR_INSTALL_FAILURE;
        }

        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(
            EVP_MD_CTX_new(), EVP_MD_CTX_free);
        if (!ctx) {
            return ERROR_INSTALL_FAILURE;
        }

        if (EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1) {
            return ERROR_INSTALL_FAILURE;
        }
        constexpr auto bufferSize = 8192u;
        std::array<char, bufferSize> buffer;
        while (file.read(buffer.data(), bufferSize)) {
            if (EVP_DigestUpdate(ctx.get(), buffer.data(),
                file.gcount()) != 1) {
                return ERROR_INSTALL_FAILURE;
            }
        }
        if (EVP_DigestUpdate(ctx.get(), buffer.data(), file.gcount()) != 1) {
            return ERROR_INSTALL_FAILURE;
        }
        std::array<unsigned char, EVP_MAX_MD_SIZE> hash;
        unsigned int hashSize = 0;
        if (EVP_DigestFinal_ex(ctx.get(), hash.data(), &hashSize) != 1) {
            return ERROR_INSTALL_FAILURE;
        }
        std::ostringstream hashStringStream;
        for (size_t i = 0; i < hashSize; ++i) {
            hashStringStream << std::hex << std::setw(2) << std::setfill('0')
                << std::nouppercase << static_cast<int>(hash[i]);
        }
        auto calculatedHash = _T("sha256:") +
            boinc_ascii_to_wide(hashStringStream.str());

        if (calculatedHash != strCheckSum) {
            return ERROR_INSTALL_FAILURE;
        }

        // Surround the installer path with quotes in case there are spaces in the path
        strBuffer = tstring(_T("\"")) + strBuffer + tstring(_T("\""));
        PROCESS_INFORMATION ProcInfo = { 0 };
        STARTUPINFO StartupInfo = { 0 };
        // create process with the elevated permissions
        // inherited from the installer process
        if (!CreateProcess(nullptr, const_cast<wchar_t*>(strBuffer.data()),
            nullptr, nullptr, FALSE, 0, nullptr, nullptr, &StartupInfo,
            &ProcInfo)) {
            return ERROR_INSTALL_FAILURE;
        }

        // Wait until the installer process exits before returning from this custom action
        WaitForSingleObject(ProcInfo.hProcess, INFINITE);

        CloseHandle(ProcInfo.hThread);
        CloseHandle(ProcInfo.hProcess);

        return ERROR_SUCCESS;
    }
};

UINT __stdcall LaunchBOINCWSLInstaller(MSIHANDLE hInstall) {
    return CALaunchBOINCWSLInstaller(hInstall).Execute();
}
